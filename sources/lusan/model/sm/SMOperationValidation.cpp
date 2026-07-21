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
 *  \file        lusan/model/sm/SMOperationValidation.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operation argument-mapping validation (headless).
 *
 ************************************************************************/

#include "lusan/model/sm/SMOperationValidation.hpp"

#include "lusan/data/common/MethodBase.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include <QSet>
#include <QString>

namespace
{
    using eOp = SMOperationBase::eOperation;

    //!< The declared callee of an action/event operation, and its stored argument list, or a null
    //!< callee when the operation is neither an action nor an event, or its callee does not resolve.
    const MethodBase* calleeOf(const StateMachineData& data, SMOperationBase* op, const QList<SMArgumentEntry>*& args)
    {
        args = nullptr;
        if (op->getOperationType() == eOp::ActionCall)
        {
            SMActionCall* call = static_cast<SMActionCall*>(op);
            args = &call->getArguments();
            for (const SMMethodEntry* m : data.getMethods().getElements())
            {
                if ((m != nullptr) && (m->getName() == call->getAction()))
                {
                    return m;
                }
            }
        }
        else if (op->getOperationType() == eOp::EventSend)
        {
            SMEventSend* send = static_cast<SMEventSend*>(op);
            args = &send->getArguments();
            for (const SMEventEntry* e : data.getEvents().getElements())
            {
                if ((e != nullptr) && (e->getName() == send->getEvent()))
                {
                    return e;
                }
            }
        }

        return nullptr;
    }
}

SMOperationValidation::eSeverity SMOperationValidation::listSeverity(const StateMachineData& data, const SMOperationList& list)
{
    // Only Ok and Error are produced today; the Warn tier is reserved. Error is terminal, so once
    // a fault is found the scan need not rank -- it can only stay Error.
    eSeverity worst = eSeverity::Ok;

    for (SMOperationBase* op : list.getOperations())
    {
        const QList<SMArgumentEntry>* args = nullptr;
        const MethodBase* callee = calleeOf(data, op, args);
        if ((callee == nullptr) || (args == nullptr))
        {
            continue;   // unresolved callee (the picker shows it blank): not a mapping fault here.
        }

        QSet<QString> formalNames;
        for (const MethodParameter& formal : callee->getElements())
        {
            formalNames.insert(formal.getName());

            // A required formal (no default) with no stored argument is an incomplete mapping.
            if (formal.hasDefault() == false)
            {
                bool mapped = false;
                for (const SMArgumentEntry& arg : *args)
                {
                    if (arg.getName() == formal.getName())
                    {
                        mapped = true;
                        break;
                    }
                }

                if (mapped == false)
                {
                    worst = eSeverity::Error;
                }
            }
        }

        // A stored argument for a formal that no longer exists is an orphan (value kept, red row).
        for (const SMArgumentEntry& arg : *args)
        {
            if (formalNames.contains(arg.getName()) == false)
            {
                worst = eSeverity::Error;
            }
        }
    }

    return worst;
}

SMOperationValidation::eSeverity SMOperationValidation::transitionSeverity(const StateMachineData& data, uint32_t transitionId)
{
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    return (transition != nullptr) ? listSeverity(data, transition->getOperations()) : eSeverity::Ok;
}

SMOperationValidation::eSeverity SMOperationValidation::stateSeverity(const StateMachineData& data, uint32_t stateId)
{
    const SMStateEntry* state = data.findStateById(stateId);
    if (state == nullptr)
    {
        return eSeverity::Ok;
    }

    const eSeverity entry = listSeverity(data, state->getEntryList());
    const eSeverity exit  = listSeverity(data, state->getExitList());
    return (static_cast<int>(entry) >= static_cast<int>(exit)) ? entry : exit;
}
