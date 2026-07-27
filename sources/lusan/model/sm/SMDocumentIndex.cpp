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
 *  \file        lusan/model/sm/SMDocumentIndex.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM document lookup service (declarations and stimulus scope).
 *
 ************************************************************************/

#include "lusan/model/sm/SMDocumentIndex.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

//////////////////////////////////////////////////////////////////////////
// SMDocumentIndex::ParamScope implementation
//////////////////////////////////////////////////////////////////////////

SMDocumentIndex::ParamScope::ParamScope(const MethodBase* payload)
    : mPayload  (payload)
{
}

const QList<MethodParameter>& SMDocumentIndex::ParamScope::parameters() const
{
    static const QList<MethodParameter> _empty;
    return (mPayload != nullptr) ? mPayload->getElements() : _empty;
}

const MethodParameter* SMDocumentIndex::ParamScope::byName(const QString& name) const
{
    for (const MethodParameter& param : parameters())
    {
        if (param.getName() == name)
        {
            return &param;
        }
    }

    return nullptr;
}

const MethodParameter* SMDocumentIndex::ParamScope::byId(uint32_t id) const
{
    for (const MethodParameter& param : parameters())
    {
        if (param.getId() == id)
        {
            return &param;
        }
    }

    return nullptr;
}

QStringList SMDocumentIndex::ParamScope::names() const
{
    QStringList result;
    for (const MethodParameter& param : parameters())
    {
        result.append(param.getName());
    }

    return result;
}

QStringList SMDocumentIndex::ParamScope::types() const
{
    QStringList result;
    for (const MethodParameter& param : parameters())
    {
        result.append(param.getType());
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////
// SMDocumentIndex implementation
//////////////////////////////////////////////////////////////////////////

SMDocumentIndex::SMDocumentIndex(const StateMachineData& data)
    : mData         (data)
    , mByKind       ( )
    , mStimuli      ( )
    , mStimuliReady (false)
    , mTransitions  ( )
    , mScopes       ( )
{
}

const SMMethodEntry* SMDocumentIndex::method(const QString& name) const
{
    return mData.getMethods().findMethod(name);
}

const SMMethodEntry* SMDocumentIndex::method(const QString& name, SMMethodEntry::eMethodType kind) const
{
    return mData.getMethods().findMethod(name, kind);
}

const SMEventEntry* SMDocumentIndex::event(const QString& name) const
{
    return mData.getEvents().findEvent(name);
}

const SMTimerEntry* SMDocumentIndex::timer(const QString& name) const
{
    return mData.getTimers().findElement(name);
}

const SMAttributeEntry* SMDocumentIndex::attribute(const QString& name) const
{
    return mData.getAttributes().findElement(name);
}

const ConstantEntry* SMDocumentIndex::constant(const QString& name) const
{
    return mData.getConstants().findElement(name);
}

const SMMethodEntry* SMDocumentIndex::method(uint32_t id) const
{
    return mData.getMethods().findMethod(id);
}

const SMEventEntry* SMDocumentIndex::event(uint32_t id) const
{
    return mData.getEvents().findEvent(id);
}

const SMTimerEntry* SMDocumentIndex::timer(uint32_t id) const
{
    return mData.getTimers().findElement(id);
}

const SMAttributeEntry* SMDocumentIndex::attribute(uint32_t id) const
{
    return mData.getAttributes().findElement(id);
}

const ConstantEntry* SMDocumentIndex::constant(uint32_t id) const
{
    return mData.getConstants().findElement(id);
}

QList<const SMMethodEntry*> SMDocumentIndex::methodsOf(SMMethodEntry::eMethodType kind) const
{
    const int key = static_cast<int>(kind);
    const auto cached = mByKind.constFind(key);
    if (cached != mByKind.constEnd())
    {
        return cached.value();
    }

    QList<const SMMethodEntry*> group;
    for (const SMMethodEntry* entry : mData.getMethods().getElements())
    {
        if ((entry != nullptr) && (entry->getMethodType() == kind))
        {
            group.append(entry);
        }
    }

    mByKind.insert(key, group);
    return group;
}

QList<SMDocumentIndex::Stimulus> SMDocumentIndex::stimuli() const
{
    if (mStimuliReady)
    {
        return mStimuli;
    }

    for (const SMMethodEntry* entry : mData.getMethods().getElements())
    {
        if ((entry != nullptr) && entry->isTrigger())
        {
            mStimuli.append(Stimulus{ SMTransitionEntry::eStimulusKind::Trigger, entry->getName() });
        }
    }

    for (const SMEventEntry* entry : mData.getEvents().getElements())
    {
        if (entry != nullptr)
        {
            mStimuli.append(Stimulus{ SMTransitionEntry::eStimulusKind::Event, entry->getName() });
        }
    }

    for (const SMTimerEntry& entry : mData.getTimers().getElements())
    {
        mStimuli.append(Stimulus{ SMTransitionEntry::eStimulusKind::Timer, entry.getName() });
    }

    mStimuliReady = true;
    return mStimuli;
}

const SMTransitionEntry* SMDocumentIndex::transition(uint32_t transitionId) const
{
    const auto cached = mTransitions.constFind(transitionId);
    if (cached != mTransitions.constEnd())
    {
        return cached.value();
    }

    const SMTransitionEntry* found = mData.findTransitionById(transitionId);
    mTransitions.insert(transitionId, found);
    return found;
}

SMDocumentIndex::ParamScope SMDocumentIndex::paramScope(uint32_t transitionId) const
{
    const auto cached = mScopes.constFind(transitionId);
    if (cached != mScopes.constEnd())
    {
        return ParamScope(cached.value());
    }

    const MethodBase* payload = nullptr;
    const SMTransitionEntry* entry = transition(transitionId);
    if (entry != nullptr)
    {
        // A trigger is looked up by bare name on purpose: the stimulus name space is shared, so
        // the entry that carries the name is the one the transition fires on. A timer expiry
        // has no payload at all and leaves the scope empty.
        switch (entry->getStimulusKind())
        {
        case SMTransitionEntry::eStimulusKind::Trigger:
            payload = method(entry->getStimulus());
            break;

        case SMTransitionEntry::eStimulusKind::Event:
            payload = event(entry->getStimulus());
            break;

        default:
            break;
        }
    }

    mScopes.insert(transitionId, payload);
    return ParamScope(payload);
}
