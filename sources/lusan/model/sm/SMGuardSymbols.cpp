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
 *  \file        lusan/model/sm/SMGuardSymbols.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard symbol resolution (name <-> document ID).
 *
 ************************************************************************/

#include "lusan/model/sm/SMGuardSymbols.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

namespace
{
    //!< The payload method (trigger or event) whose parameters are in scope for a transition.
    const MethodBase* stimulusPayload(const StateMachineData& data, uint32_t transitionId)
    {
        const SMTransitionEntry* transition = data.findTransitionById(transitionId);
        if ((transition == nullptr) || (transition->getStimulusKind() == SMTransitionEntry::eStimulusKind::Timer))
        {
            return nullptr;
        }

        if (transition->getStimulusKind() == SMTransitionEntry::eStimulusKind::Trigger)
        {
            return data.getMethods().findMethod(transition->getStimulus());
        }

        for (const SMEventEntry* event : data.getEvents().getElements())
        {
            if ((event != nullptr) && (event->getName() == transition->getStimulus()))
            {
                return event;
            }
        }

        return nullptr;
    }
}

uint32_t SMGuardSymbols::attributeId(const StateMachineData& data, const QString& name)
{
    for (const SMAttributeEntry& attr : data.getAttributes().getElements())
    {
        if (attr.getName() == name)
        {
            return attr.getId();
        }
    }

    return 0u;
}

uint32_t SMGuardSymbols::constantId(const StateMachineData& data, const QString& name)
{
    for (const ConstantEntry& constant : data.getConstants().getElements())
    {
        if (constant.getName() == name)
        {
            return constant.getId();
        }
    }

    return 0u;
}

uint32_t SMGuardSymbols::paramId(const StateMachineData& data, uint32_t transitionId, const QString& name)
{
    const MethodBase* payload = stimulusPayload(data, transitionId);
    if (payload != nullptr)
    {
        for (const MethodParameter& param : payload->getElements())
        {
            if (param.getName() == name)
            {
                return param.getId();
            }
        }
    }

    return 0u;
}

const SMMethodEntry* SMGuardSymbols::conditionMethod(const StateMachineData& data, const QString& name)
{
    const SMMethodEntry* method = data.getMethods().findMethod(name);
    return ((method != nullptr) && method->isCondition()) ? method : nullptr;
}

QStringList SMGuardSymbols::paramNames(const StateMachineData& data, uint32_t transitionId)
{
    QStringList names;
    const MethodBase* payload = stimulusPayload(data, transitionId);
    if (payload != nullptr)
    {
        for (const MethodParameter& param : payload->getElements())
        {
            names.append(param.getName());
        }
    }

    return names;
}

QStringList SMGuardSymbols::paramTypes(const StateMachineData& data, uint32_t transitionId)
{
    QStringList types;
    const MethodBase* payload = stimulusPayload(data, transitionId);
    if (payload != nullptr)
    {
        for (const MethodParameter& param : payload->getElements())
        {
            types.append(param.getType());
        }
    }

    return types;
}

QString SMGuardSymbols::attributeName(const StateMachineData& data, uint32_t id)
{
    for (const SMAttributeEntry& attr : data.getAttributes().getElements())
    {
        if (attr.getId() == id)
        {
            return attr.getName();
        }
    }

    return QString();
}

QString SMGuardSymbols::constantName(const StateMachineData& data, uint32_t id)
{
    for (const ConstantEntry& constant : data.getConstants().getElements())
    {
        if (constant.getId() == id)
        {
            return constant.getName();
        }
    }

    return QString();
}

QString SMGuardSymbols::paramName(const StateMachineData& data, uint32_t transitionId, uint32_t id)
{
    const MethodBase* payload = stimulusPayload(data, transitionId);
    if (payload != nullptr)
    {
        for (const MethodParameter& param : payload->getElements())
        {
            if (param.getId() == id)
            {
                return param.getName();
            }
        }
    }

    return QString();
}

const SMMethodEntry* SMGuardSymbols::method(const StateMachineData& data, uint32_t id)
{
    return data.getMethods().findMethod(id);
}
