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
 *  \file        lusan/model/sm/SMGoToDef.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM go-to-declaration resolver.
 *
 ************************************************************************/

#include "lusan/model/sm/SMGoToDef.hpp"

#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include <QObject>

namespace
{
    using eTarget = SMReferences::eTarget;

    //!< Resolves a referenced declaration name to its document ID for the given kind; 0 if none.
    uint32_t resolveId(const StateMachineData& data, eTarget kind, const QString& name)
    {
        switch (kind)
        {
        case eTarget::Trigger:
        case eTarget::Action:
        case eTarget::Condition:
        {
            const SMMethodEntry* method = data.getMethods().findMethod(name);
            return (method != nullptr) ? method->getId() : 0u;
        }
        case eTarget::Event:
        {
            const SMEventEntry* event = data.getEvents().findEvent(name);
            return (event != nullptr) ? event->getId() : 0u;
        }
        case eTarget::Timer:
        {
            const SMTimerEntry* timer = data.getTimers().findElement(name);
            return (timer != nullptr) ? timer->getId() : 0u;
        }
        case eTarget::Attribute:
        {
            const SMAttributeEntry* attribute = data.getAttributes().findElement(name);
            return (attribute != nullptr) ? attribute->getId() : 0u;
        }
        case eTarget::Constant:
        {
            const ConstantEntry* constant = data.getConstants().findElement(name);
            return (constant != nullptr) ? constant->getId() : 0u;
        }
        default:
            return 0u;
        }
    }

    //!< The current declaration name for a resolved ID (guard symbols carry IDs, not names).
    QString resolveName(const StateMachineData& data, eTarget kind, uint32_t declId)
    {
        switch (kind)
        {
        case eTarget::Condition:
        {
            const SMMethodEntry* method = data.getMethods().findMethod(declId);
            return (method != nullptr) ? method->getName() : QString();
        }
        case eTarget::Attribute:
        {
            const SMAttributeEntry* attribute = data.getAttributes().findElement(declId);
            return (attribute != nullptr) ? attribute->getName() : QString();
        }
        case eTarget::Constant:
        {
            const ConstantEntry* constant = data.getConstants().findElement(declId);
            return (constant != nullptr) ? constant->getName() : QString();
        }
        default:
            return QString();
        }
    }

    //!< Appends the guard-bound declarations (condition / attribute / constant) of a guard subtree.
    void collectGuardRefs(const StateMachineData& data, const SMGuardNode* node, QList<SMGoToDef::Target>& out)
    {
        if (node == nullptr)
            return;

        eTarget kind = eTarget::Condition;
        bool isRef = true;
        switch (node->getKind())
        {
        case SMGuardNode::eKind::Call:  kind = eTarget::Condition; break;
        case SMGuardNode::eKind::Attr:  kind = eTarget::Attribute; break;
        case SMGuardNode::eKind::Const: kind = eTarget::Constant;  break;
        default:                        isRef = false; break;   // Param and structural nodes: no page declaration.
        }

        if (isRef)
        {
            const uint32_t declId = node->getSymbolId();
            if (declId != 0u)
                out.append({ kind, declId, resolveName(data, kind, declId) });
        }

        for (const SMGuardNode* child : node->getChildren())
            collectGuardRefs(data, child, out);
    }
}

QList<SMGoToDef::Target> SMGoToDef::collect(const StateMachineData& data, uint32_t elementId, bool isState)
{
    QList<Target> collected;

    // Name-based references (stimulus, action calls, attribute sets, timer/event ops, arguments)
    // from the headless walker, each resolved to its declaration ID.
    for (const SMReferences::Ref& ref : SMReferences::definitionsOf(data, elementId, isState))
    {
        const uint32_t declId = resolveId(data, ref.target, ref.name);
        if (declId != 0u)
            collected.append({ ref.target, declId, ref.name });
    }

    // Guard-bound references live only on a transition and are keyed by ID; the name-based walker
    // does not see them, so union them the way SMWhereUsed does (guard knowledge stays here).
    if (isState == false)
    {
        const SMTransitionEntry* transition = data.findTransitionById(elementId);
        if (transition != nullptr)
            collectGuardRefs(data, transition->getGuard().getTree(), collected);
    }

    // De-duplicate by (kind, ID): a declaration referenced both in the guard and in an operation
    // argument must appear once. Small N (references of one element), so a linear scan is fine.
    QList<Target> unique;
    for (const Target& target : collected)
    {
        bool seen = false;
        for (const Target& kept : unique)
        {
            if ((kept.kind == target.kind) && (kept.declId == target.declId))
            {
                seen = true;
                break;
            }
        }

        if (seen == false)
            unique.append(target);
    }

    return unique;
}

SMGoToDef::Target SMGoToDef::stimulusOf(const StateMachineData& data, uint32_t transitionId)
{
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if (transition == nullptr)
    {
        return { eTarget::Trigger, 0u, QString() };
    }

    const QString name = transition->getStimulus();
    eTarget kind = eTarget::Trigger;
    switch (transition->getStimulusKind())
    {
    case SMTransitionEntry::eStimulusKind::Trigger: kind = eTarget::Trigger; break;
    case SMTransitionEntry::eStimulusKind::Event:   kind = eTarget::Event;   break;
    case SMTransitionEntry::eStimulusKind::Timer:   kind = eTarget::Timer;   break;
    default:                                        break;
    }

    return { kind, resolveId(data, kind, name), name };
}

QList<SMGoToDef::Target> SMGoToDef::resolveRefs(const StateMachineData& data, const QList<SMReferences::Ref>& refs)
{
    QList<Target> unique;
    for (const SMReferences::Ref& ref : refs)
    {
        const uint32_t declId = resolveId(data, ref.target, ref.name);
        if (declId == 0u)
        {
            continue;   // A dangling name resolves to no declaration: nowhere to navigate.
        }

        bool seen = false;
        for (const Target& kept : unique)
        {
            if ((kept.kind == ref.target) && (kept.declId == declId))
            {
                seen = true;
                break;
            }
        }

        if (seen == false)
        {
            unique.append({ ref.target, declId, ref.name });
        }
    }

    return unique;
}

QString SMGoToDef::kindWord(SMReferences::eTarget kind)
{
    switch (kind)
    {
    case SMReferences::eTarget::State:      return QObject::tr("state");
    case SMReferences::eTarget::Trigger:    return QObject::tr("trigger");
    case SMReferences::eTarget::Action:     return QObject::tr("action");
    case SMReferences::eTarget::Condition:  return QObject::tr("condition");
    case SMReferences::eTarget::Event:      return QObject::tr("event");
    case SMReferences::eTarget::Timer:      return QObject::tr("timer");
    case SMReferences::eTarget::Attribute:  return QObject::tr("attribute");
    case SMReferences::eTarget::Constant:   return QObject::tr("constant");
    default:                                return QString();
    }
}
