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
#include "lusan/data/sm/StateMachineData.hpp"

#include <QSet>

namespace
{
    //!< Collects the layout owner IDs and the state names of a state subtree: the state,
    //!< its transitions, and recursively its painted substates and their transitions.
    void collectSubtree(const SMStateEntry& state, QList<uint32_t>& owners, QSet<QString>& names)
    {
        owners.append(state.getId());
        names.insert(state.getName());
        for (const SMTransitionEntry* transition : state.getTransitions().getElements())
        {
            owners.append(transition->getId());
        }

        const SMStateData* nested = state.getNestedStates();
        if (nested != nullptr)
        {
            for (const SMStateEntry* child : nested->getElements())
            {
                collectSubtree(*child, owners, names);
            }
        }
    }

    /**
     * \brief   One transition of a surviving state that targets a deleted state.
     **/
    struct IncomingRef
    {
        SMTransitionData*   list;   //!< The owning state's transition list.
        uint32_t            id;     //!< The transition's element ID.
    };

    //!< Walks every state outside the deleted subtree and collects the transitions whose
    //!< target name belongs to the subtree.
    void collectIncoming(SMStateData& level, uint32_t skipStateId, const QSet<QString>& names, QList<IncomingRef>& incoming)
    {
        for (SMStateEntry* state : level.getElements())
        {
            if (state->getId() == skipStateId)
            {
                continue;
            }

            for (SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if (transition->isExternal() && names.contains(transition->getTo()))
                {
                    incoming.append(IncomingRef{ &state->getTransitions(), transition->getId() });
                }
            }

            if (state->hasNestedStates())
            {
                collectIncoming(*state->getNestedStates(), skipStateId, names, incoming);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// SMCreateStateCommand
//////////////////////////////////////////////////////////////////////////

SMCreateStateCommand::SMCreateStateCommand(  StateMachineData& data, DocModelNotifier& notifier
                                           , SMStateData& level, const QString& name, SMStateEntry::eStateKind kind
                                           , const QRectF& geometry, const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCompositeCommand(data, notifier, text, parent)
    , mState            (new SMStateEntry(0, name, kind, &level))
{
    // Children run in order on redo: the insertion allocates the ID the node child reads.
    new TDocAddCommand<SMStateEntry*, DocumentElem>(notifier, level, mState, eDocElementKind::State, text, this);
    new SMAttachNodeCommand(data, notifier, *mState, geometry, text, this);
}

uint32_t SMCreateStateCommand::getStateId() const
{
    return mState->getId();
}

//////////////////////////////////////////////////////////////////////////
// SMRenameStateCommand
//////////////////////////////////////////////////////////////////////////

SMRenameStateCommand::SMRenameStateCommand(  StateMachineData& data, DocModelNotifier& notifier
                                           , uint32_t stateId, const QString& newName
                                           , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mId       (stateId)
    , mNew      (newName)
    , mOld      ( )
{
}

void SMRenameStateCommand::apply(const QString& name, const QString& previous)
{
    SMStateEntry* state = data().findStateById(mId);
    if (state != nullptr)
    {
        state->setName(name);
        notifier().notifyNameChanged(mId, previous, name);
    }
}

void SMRenameStateCommand::redo()
{
    if (mCaptured == false)
    {
        const SMStateEntry* state = data().findStateById(mId);
        mOld = (state != nullptr ? state->getName() : QString());
        mCaptured = true;
    }

    apply(mNew, mOld);
}

void SMRenameStateCommand::undo()
{
    apply(mOld, mNew);
}

//////////////////////////////////////////////////////////////////////////
// SMRemoveStateCommand
//////////////////////////////////////////////////////////////////////////

SMRemoveStateCommand::SMRemoveStateCommand(StateMachineData& data, DocModelNotifier& notifier, SMStateData& level, uint32_t stateId, const QString& text, QUndoCommand* parent)
    : SMCompositeCommand(data, notifier, text, parent)
{
    QList<uint32_t> owners;
    QSet<QString>   names;
    SMStateEntry** slot = level.findElement(stateId);
    if ((slot != nullptr) && (*slot != nullptr))
    {
        collectSubtree(**slot, owners, names);
    }

    QList<IncomingRef> incoming;
    collectIncoming(data.getStates(), stateId, names, incoming);
    for (const IncomingRef& ref : incoming)
    {
        owners.append(ref.id);
    }

    // Children run forward on redo (layout, incoming transitions, state) and reverse on undo.
    new SMRemoveLayoutCommand(data, notifier, owners, text, this);
    for (const IncomingRef& ref : incoming)
    {
        new TDocRemoveCommand<SMTransitionEntry*, DocumentElem>(notifier, *ref.list, ref.id, eDocElementKind::Transition, text, this);
    }

    new TDocRemoveCommand<SMStateEntry*, DocumentElem>(notifier, level, stateId, eDocElementKind::State, text, this);
}
