/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/sm/SMGuardValidation.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard document validation.
 *
 ************************************************************************/

#include "lusan/model/sm/SMGuardValidation.hpp"

#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"

#include <QObject>

namespace
{
    using eKind     = SMGuardNode::eKind;
    using eSeverity = SMGuardValidation::eSeverity;
    using eFinding  = SMGuardValidation::eKind;
    using Finding   = SMGuardValidation::Finding;

    QString elide(const QString& text, int max = 48)
    {
        const QString one = text.simplified();
        return (one.length() > max) ? (one.left(max - 3) + QStringLiteral("...")) : one;
    }

    //!< How a broken reference names its lost target: the advisory name last recorded on the node
    //!< (which survives save and load), or the raw id when the document never recorded a name.
    QString refDisplay(const SMGuardNode& node)
    {
        const QString cached = node.getCacheName();
        return cached.isEmpty() ? QStringLiteral("(id %1)").arg(node.getSymbolId())
                                : QStringLiteral("'%1'").arg(cached);
    }


    void checkNode(const StateMachineData& data, const SMGuardRef& target, const QString& location
                  , const SMGuardNode& node, QList<Finding>& findings)
    {
        const uint32_t transitionId = target.getScopeId();
        const QString what = QStringLiteral("guard");
        switch (node.getKind())
        {
        case eKind::Attr:
            if (SMGuardSymbols::attributeName(data, node.getSymbolId()).isEmpty())
            {
                findings.append({ eSeverity::Error, eFinding::BrokenRef, target, location
                                , QStringLiteral("%1 references a deleted attribute %2").arg(what, refDisplay(node))
                                , node.getSymbolId() });
            }
            break;

        case eKind::Const:
            if (SMGuardSymbols::constantName(data, node.getSymbolId()).isEmpty())
            {
                findings.append({ eSeverity::Error, eFinding::BrokenRef, target, location
                                , QStringLiteral("%1 references a deleted constant %2").arg(what, refDisplay(node))
                                , node.getSymbolId() });
            }
            break;

        case eKind::Call:
        {
            const SMMethodEntry* method = SMGuardSymbols::method(data, node.getSymbolId());
            if ((method == nullptr) || (method->isCondition() == false))
            {
                findings.append({ eSeverity::Error, eFinding::BrokenRef, target, location
                                , QStringLiteral("%1 calls a deleted condition method %2").arg(what, refDisplay(node))
                                , node.getSymbolId() });
            }
            break;
        }

        case eKind::Param:
            if (SMGuardSymbols::paramName(data, transitionId, node.getSymbolId()).isEmpty())
            {
                // The stimulus no longer has this parameter. A stimulus change that leaves a
                // same-name-same-type parameter re-binds at the command, so anything still stale
                // here has no match and is a broken reference.
                findings.append({ eSeverity::Error, eFinding::BrokenRef, target, location
                                , QStringLiteral("%1 references a parameter the stimulus no longer has %2").arg(what, refDisplay(node))
                                , node.getSymbolId() });
            }
            break;

        case eKind::Raw:
            // The audit: every verbatim fragment is listed -- never silent.
            findings.append({ eSeverity::Info, eFinding::RawFragment, target, location
                            , QStringLiteral("raw C++ fragment: %1").arg(elide(node.getText()))
                            , 0u });   // raw text names no declaration -- there is nothing to key on
            break;

        default:
            break;
        }

        for (const SMGuardNode* child : node.getChildren())
        {
            checkNode(data, target, location, *child, findings);
        }
    }

    //!< The one check every predicate gets, whichever element carries it.
    void checkGuard(const StateMachineData& data, const SMGuardRef& target, const QString& location
                   , const SMGuard& guard, QList<Finding>& findings)
    {
        if (guard.isEmpty())
        {
            return;
        }

        if (guard.isDraft())
        {
            findings.append({ eSeverity::Error, eFinding::Draft, target, location
                            , QStringLiteral("guard is still a draft: %1. Code generation refuses it").arg(elide(guard.getDraftText()))
                            , 0u });   // a draft is the whole predicate, not one element of it
            return;
        }

        if (guard.getTree() != nullptr)
        {
            checkNode(data, target, location, *guard.getTree(), findings);
        }
    }

    void checkTransition(const StateMachineData& data, const SMStateEntry& state
                        , const SMTransitionEntry& transition, QList<Finding>& findings)
    {
        QString location = state.getName() + QStringLiteral(" : ") + transition.getStimulus();
        if (transition.hasTarget())
        {
            location += QStringLiteral(" -> ") + transition.getTargetName();
        }

        checkGuard(data, SMGuardRef(transition.getId()), location, transition.getGuard(), findings);
    }

    /**
     * \brief   Walks a level. \p only addresses ONE predicate when it is valid -- the per-element
     *          queries the canvas and the hover card make -- and an invalid (default) ref means
     *          the whole document, which is the validation panel's run.
     **/
    void checkLevel(const StateMachineData& data, const SMStateData& level, const SMGuardRef& only
                   , QList<Finding>& findings)
    {
        const bool all = (only.isValid() == false);
        for (const SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
            {
                continue;
            }

            for (const SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if ((transition != nullptr) && (all || (only == SMGuardRef(transition->getId()))))
                {
                    checkTransition(data, *state, *transition, findings);
                }
            }

            if (state->hasNestedStates())
            {
                checkLevel(data, *state->getNestedStates(), only, findings);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// SMGuardValidation
//////////////////////////////////////////////////////////////////////////

QString SMGuardValidation::describe(SMGuardValidation::eKind kind)
{
    switch (kind)
    {
    case eKind::Draft:
        return QObject::tr("The guard text was never committed, so there is no parsed condition to generate. Commit it or clear it.");
    case eKind::RawFragment:
        return QObject::tr("A verbatim C++ fragment is emitted as written and is never parsed or checked. Listed so every unchecked fragment in the document is accounted for.");
    case eKind::BrokenRef:
        return QObject::tr("The guard references a declaration that no longer exists or has left its scope. Re-bind the reference or remove it.");
    default:
        return QString();
    }
}

QList<SMGuardValidation::Finding> SMGuardValidation::validate(const StateMachineData& data)
{
    QList<Finding> findings;
    checkLevel(data, data.getStates(), SMGuardRef(), findings);
    return findings;
}

QList<SMGuardValidation::Finding> SMGuardValidation::validateTransition(const StateMachineData& data, uint32_t transitionId)
{
    QList<Finding> findings;
    if (transitionId != 0u)
    {
        checkLevel(data, data.getStates(), SMGuardRef(transitionId), findings);
    }

    return findings;
}

bool SMGuardValidation::worstSeverity(const StateMachineData& data, uint32_t transitionId, eSeverity& worst)
{
    const QList<Finding> findings = validateTransition(data, transitionId);
    if (findings.isEmpty())
    {
        return false;
    }

    worst = eSeverity::Info;
    for (const Finding& finding : findings)
    {
        if (static_cast<int>(finding.severity) > static_cast<int>(worst))
        {
            worst = finding.severity;
        }
    }

    return true;
}
