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
 *  \brief       Lusan application, FSM guard document validation (v7 B12 / S15).
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

    void checkNode(const StateMachineData& data, uint32_t transitionId, const QString& location
                  , const SMGuardNode& node, QStringList& shadowed, QList<Finding>& findings)
    {
        switch (node.getKind())
        {
        case eKind::Attr:
            if (SMGuardSymbols::attributeName(data, node.getSymbolId()).isEmpty())
            {
                findings.append({ eSeverity::Error, eFinding::BrokenRef, transitionId, location
                                , QStringLiteral("guard references a deleted attribute (id %1)").arg(node.getSymbolId()) });
            }
            break;

        case eKind::Const:
            if (SMGuardSymbols::constantName(data, node.getSymbolId()).isEmpty())
            {
                findings.append({ eSeverity::Error, eFinding::BrokenRef, transitionId, location
                                , QStringLiteral("guard references a deleted constant (id %1)").arg(node.getSymbolId()) });
            }
            break;

        case eKind::Call:
        {
            const SMMethodEntry* method = SMGuardSymbols::method(data, node.getSymbolId());
            if ((method == nullptr) || (method->isCondition() == false))
            {
                findings.append({ eSeverity::Error, eFinding::BrokenRef, transitionId, location
                                , QStringLiteral("guard calls a deleted condition method (id %1)").arg(node.getSymbolId()) });
            }
            break;
        }

        case eKind::Param:
        {
            const QString name = SMGuardSymbols::paramName(data, transitionId, node.getSymbolId());
            if (name.isEmpty())
            {
                // Stale after a stimulus change: a same-name-same-type parameter of the
                // new stimulus is the v6-3.3 auto-re-bind case -- info, not an error.
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
                    findings.append({ eSeverity::Info, eFinding::ParamRebind, transitionId, location
                                    , QStringLiteral("parameter '%1' re-binds to the new stimulus by name and type -- re-commit the guard").arg(stale->getName()) });
                }
                else
                {
                    findings.append({ eSeverity::Error, eFinding::BrokenRef, transitionId, location
                                    , QStringLiteral("guard references a parameter the stimulus no longer has (id %1)").arg(node.getSymbolId()) });
                }
            }
            else
            {
                // D3 shadowing, kept quiet: one warning per shadowed name per guard.
                const bool hidesAttr  = (SMGuardSymbols::attributeId(data, name) != 0u);
                const bool hidesConst = (SMGuardSymbols::constantId(data, name) != 0u);
                if ((hidesAttr || hidesConst) && (shadowed.contains(name) == false))
                {
                    shadowed.append(name);
                    findings.append({ eSeverity::Warning, eFinding::Shadowing, transitionId, location
                                    , QStringLiteral("'%1' is the stimulus parameter and hides %2 '%1'")
                                          .arg(name, hidesAttr ? QStringLiteral("attribute") : QStringLiteral("constant")) });
                }
            }
            break;
        }

        case eKind::Raw:
            // The D4 audit: every verbatim fragment is listed -- never silent.
            findings.append({ eSeverity::Info, eFinding::RawFragment, transitionId, location
                            , QStringLiteral("raw C++ fragment: %1").arg(elide(node.getText())) });
            break;

        default:
            break;
        }

        for (const SMGuardNode* child : node.getChildren())
        {
            checkNode(data, transitionId, location, *child, shadowed, findings);
        }
    }

    void checkTransition(const StateMachineData& data, const SMStateEntry& state
                        , const SMTransitionEntry& transition, QList<Finding>& findings)
    {
        const SMGuard& guard = transition.getGuard();
        if (guard.isEmpty())
        {
            return;
        }

        QString location = state.getName() + QStringLiteral(" : ") + transition.getStimulus();
        if (transition.isExternal())
        {
            location += QStringLiteral(" -> ") + transition.getTo();
        }

        if (guard.isDraft())
        {
            findings.append({ eSeverity::Error, eFinding::Draft, transition.getId(), location
                            , QStringLiteral("guard is a draft: %1 -- generation refuses").arg(elide(guard.getDraftText())) });
            return;
        }

        if (guard.getTree() != nullptr)
        {
            QStringList shadowed;
            checkNode(data, transition.getId(), location, *guard.getTree(), shadowed, findings);
        }
    }

    void checkLevel(const StateMachineData& data, const SMStateData& level, uint32_t onlyTransition
                   , QList<Finding>& findings)
    {
        for (const SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
            {
                continue;
            }

            for (const SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if ((transition != nullptr)
                    && ((onlyTransition == 0u) || (transition->getId() == onlyTransition)))
                {
                    checkTransition(data, *state, *transition, findings);
                }
            }

            if (state->hasNestedStates())
            {
                checkLevel(data, *state->getNestedStates(), onlyTransition, findings);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// SMGuardValidation
//////////////////////////////////////////////////////////////////////////

QList<SMGuardValidation::Finding> SMGuardValidation::validate(const StateMachineData& data)
{
    QList<Finding> findings;
    checkLevel(data, data.getStates(), 0u, findings);
    return findings;
}

QList<SMGuardValidation::Finding> SMGuardValidation::validateTransition(const StateMachineData& data, uint32_t transitionId)
{
    QList<Finding> findings;
    if (transitionId != 0u)
    {
        checkLevel(data, data.getStates(), transitionId, findings);
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
