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

    //!< The declaration of a stimulus parameter anywhere in the document (stale-ID lookup).
    const MethodParameter* findParamAnywhere(const StateMachineData& data, uint32_t paramId)
    {
        for (const SMMethodEntry* method : data.getMethods().getElements())
        {
            if (method == nullptr)
            {
                continue;
            }

            for (const MethodParameter& param : method->getElements())
            {
                if (param.getId() == paramId)
                {
                    return &param;
                }
            }
        }

        for (const SMEventEntry* event : data.getEvents().getElements())
        {
            if (event == nullptr)
            {
                continue;
            }

            for (const MethodParameter& param : event->getElements())
            {
                if (param.getId() == paramId)
                {
                    return &param;
                }
            }
        }

        return nullptr;
    }

    //!< What the message calls the predicate being checked. A stop condition is not a guard, and
    //!< a finding that says "guard" about a `do/` activity sends the reader to the wrong tab.
    QString noun(const SMGuardRef& target)
    {
        return (target.getOwner() == SMGuardRef::eOwner::DoActivity)
                    ? QStringLiteral("Do stop condition")
                    : QStringLiteral("guard");
    }

    void checkNode(const StateMachineData& data, const SMGuardRef& target, const QString& location
                  , const SMGuardNode& node, QStringList& shadowed, QList<Finding>& findings)
    {
        // The stimulus parameter scope of the checked predicate: its own, for a transition; none
        // at all for a `do/` activity, which ticks on a timer with no stimulus in hand.
        const uint32_t transitionId = target.getScopeId();
        const QString what = noun(target);
        switch (node.getKind())
        {
        case eKind::Attr:
            if (SMGuardSymbols::attributeName(data, node.getSymbolId()).isEmpty())
            {
                findings.append({ eSeverity::Error, eFinding::BrokenRef, target, location
                                , QStringLiteral("%1 references a deleted attribute (id %2)").arg(what).arg(node.getSymbolId())
                                , node.getSymbolId() });
            }
            break;

        case eKind::Const:
            if (SMGuardSymbols::constantName(data, node.getSymbolId()).isEmpty())
            {
                findings.append({ eSeverity::Error, eFinding::BrokenRef, target, location
                                , QStringLiteral("%1 references a deleted constant (id %2)").arg(what).arg(node.getSymbolId())
                                , node.getSymbolId() });
            }
            break;

        case eKind::Call:
        {
            const SMMethodEntry* method = SMGuardSymbols::method(data, node.getSymbolId());
            if ((method == nullptr) || (method->isCondition() == false))
            {
                findings.append({ eSeverity::Error, eFinding::BrokenRef, target, location
                                , QStringLiteral("%1 calls a deleted condition method (id %2)").arg(what).arg(node.getSymbolId())
                                , node.getSymbolId() });
            }
            break;
        }

        case eKind::Param:
        {
            const QString name = SMGuardSymbols::paramName(data, transitionId, node.getSymbolId());
            if (name.isEmpty())
            {
                // Stale after a stimulus change: a same-name-same-type parameter of the
                // new stimulus is the auto-re-bind case -- info, not an error.
                const MethodParameter* stale = findParamAnywhere(data, node.getSymbolId());
                bool rebindable = false;
                if (stale != nullptr)
                {
                    const QStringList names = SMGuardSymbols::paramNames(data, transitionId);
                    const QStringList types = SMGuardSymbols::paramTypes(data, transitionId);
                    const int index = static_cast<int>(names.indexOf(stale->getName()));
                    rebindable = (index >= 0) && (index < types.size()) && (types.at(index) == stale->getType());
                }

                if (rebindable)
                {
                    findings.append({ eSeverity::Info, eFinding::ParamRebind, target, location
                                    , QStringLiteral("parameter '%1' re-binds to the new stimulus by name and type, so commit the guard again").arg(stale->getName())
                                    , node.getSymbolId() });
                }
                else
                {
                    findings.append({ eSeverity::Error, eFinding::BrokenRef, target, location
                                    , QStringLiteral("%1 references a parameter the stimulus no longer has (id %2)").arg(what).arg(node.getSymbolId())
                                    , node.getSymbolId() });
                }
            }
            else
            {
                // Shadowing, kept quiet: one warning per shadowed name per guard.
                const bool hidesAttr  = (SMGuardSymbols::attributeId(data, name) != 0u);
                const bool hidesConst = (SMGuardSymbols::constantId(data, name) != 0u);
                if ((hidesAttr || hidesConst) && (shadowed.contains(name) == false))
                {
                    shadowed.append(name);
                    findings.append({ eSeverity::Warning, eFinding::Shadowing, target, location
                                    , QStringLiteral("'%1' is the stimulus parameter and hides %2 '%1'")
                                          .arg(name, hidesAttr ? QStringLiteral("attribute") : QStringLiteral("constant"))
                                    , node.getSymbolId() });
                }
            }
            break;
        }

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
            checkNode(data, target, location, *child, shadowed, findings);
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
                            , QStringLiteral("%1 is still a draft: %2. Code generation refuses it").arg(noun(target), elide(guard.getDraftText()))
                            , 0u });   // a draft is the whole predicate, not one element of it
            return;
        }

        if (guard.getTree() != nullptr)
        {
            QStringList shadowed;
            checkNode(data, target, location, *guard.getTree(), shadowed, findings);
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

    //!< The `<Until>` of a state's `DoList`. A stop condition on a state with no activity is not
    //!< checked: there is no timer for it to stop, so there is nothing there to be wrong about.
    void checkDoActivity(const StateMachineData& data, const SMStateEntry& state, QList<Finding>& findings)
    {
        if (state.getDoList().isEmpty())
        {
            return;
        }

        checkGuard(data, SMGuardRef::doActivity(state.getId())
                  , state.getName() + QStringLiteral(" : do/"), state.getDoUntil(), findings);
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

            if (all || (only == SMGuardRef::doActivity(state->getId())))
            {
                checkDoActivity(data, *state, findings);
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
    case eKind::Shadowing:
        return QObject::tr("A stimulus parameter has the same name as an attribute or constant, and hides it inside this guard. Rename one of them to say which is meant.");
    case eKind::RawFragment:
        return QObject::tr("A verbatim C++ fragment is emitted as written and is never parsed or checked. Listed so every unchecked fragment in the document is accounted for.");
    case eKind::BrokenRef:
        return QObject::tr("The guard references a declaration that no longer exists or has left its scope. Re-bind the reference or remove it.");
    case eKind::ParamRebind:
        return QObject::tr("The stimulus changed, but a parameter of the same name and type exists on the new one, so the reference can be re-bound as it stands.");
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

QList<SMGuardValidation::Finding> SMGuardValidation::validateDoActivity(const StateMachineData& data, uint32_t stateId)
{
    QList<Finding> findings;
    if (stateId != 0u)
    {
        checkLevel(data, data.getStates(), SMGuardRef::doActivity(stateId), findings);
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
