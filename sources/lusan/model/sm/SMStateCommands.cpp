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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/sm/SMStateCommands.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM state add/delete commands.
 *
 ************************************************************************/

#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/data/sm/SMTransition.hpp"

namespace
{
    //!< Collects the layout owner IDs of a state subtree: the state, its transitions, and
    //!< recursively its painted substates and their transitions.
    void collectSubtreeOwners(const SMStateEntry& state, QList<uint32_t>& owners)
    {
        owners.append(state.getId());
        for (const SMTransitionEntry* transition : state.getTransitions().getElements())
        {
            owners.append(transition->getId());
        }

        const SMStateData* nested = state.getNestedStates();
        if (nested != nullptr)
        {
            for (const SMStateEntry* child : nested->getElements())
            {
                collectSubtreeOwners(*child, owners);
            }
        }
    }
}

SMRemoveStateCommand::SMRemoveStateCommand(StateMachineData& data, DocModelNotifier& notifier, SMStateData& level, uint32_t stateId, const QString& text, QUndoCommand* parent)
    : SMCompositeCommand(data, notifier, text, parent)
{
    QList<uint32_t> owners;
    SMStateEntry** slot = level.findElement(stateId);
    if ((slot != nullptr) && (*slot != nullptr))
    {
        collectSubtreeOwners(**slot, owners);
    }

    // Children run forward on redo (layout then state) and reverse on undo.
    new SMRemoveLayoutCommand(data, notifier, owners, text, this);
    new TDocRemoveCommand<SMStateEntry*, DocumentElem>(notifier, level, stateId, eDocElementKind::State, text, this);
}
