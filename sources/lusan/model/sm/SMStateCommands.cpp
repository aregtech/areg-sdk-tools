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
 *  \file        lusan/model/sm/SMStateCommands.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM state add/delete commands.
 *
 ************************************************************************/

#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include <QSet>
#include <memory>
#include <utility>

namespace
{
    //!< Collects the layout owner IDs and the state IDs of a state subtree: the state,
    //!< its transitions, and recursively its painted substates and their transitions.
    void collectSubtree(const SMStateEntry& state, QList<uint32_t>& owners, QSet<uint32_t>& stateIds)
    {
        owners.append(state.getId());
        stateIds.insert(state.getId());
        for (const SMTransitionEntry* transition : state.getTransitions().getElements())
        {
            owners.append(transition->getId());
        }

        const SMStateData* nested = state.getNestedStates();
        if (nested != nullptr)
        {
            for (const SMStateEntry* child : nested->getElements())
            {
                collectSubtree(*child, owners, stateIds);
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

    //!< Walks every state outside the removed set and collects the transitions whose target id
    //!< belongs to it. A state in the removed set is skipped whole: its transitions go with it.
    void collectIncoming(SMStateData& level, const QSet<uint32_t>& removedIds, QList<IncomingRef>& incoming)
    {
        for (SMStateEntry* state : level.getElements())
        {
            if (removedIds.contains(state->getId()))
            {
                continue;
            }

            for (SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if (transition->hasTarget() && removedIds.contains(transition->getToId()))
                {
                    incoming.append(IncomingRef{ &state->getTransitions(), transition->getId() });
                }
            }

            if (state->hasNestedStates())
            {
                collectIncoming(*state->getNestedStates(), removedIds, incoming);
            }
        }
    }

    /**
     * \brief   The transitions of surviving states that target the removed subtree, with their ids
     *          appended to \p owners so their layout goes with them.
     * \param   data        The document root; the walk starts at its root level.
     * \param   removedIds  The state IDs disappearing in this command.
     * \param   owners      Receives the doomed transitions' IDs, for the layout child.
     **/
    QList<IncomingRef> collectIncomingRemovals(StateMachineData& data, const QSet<uint32_t>& removedIds, QList<uint32_t>& owners)
    {
        QList<IncomingRef> incoming;
        collectIncoming(data.getStates(), removedIds, incoming);
        for (const IncomingRef& ref : incoming)
        {
            owners.append(ref.id);
        }

        return incoming;
    }

    //!< Creates one removal child per collected transition. Kept apart from the sweep because the
    //!< layout child has to be constructed first (children redo in construction order).
    void appendIncomingRemovals(DocModelNotifier& notifier, const QList<IncomingRef>& incoming, const QString& text, QUndoCommand* parent)
    {
        for (const IncomingRef& ref : incoming)
        {
            new TDocRemoveCommand<SMTransitionEntry*, DocumentElem>(notifier, *ref.list, ref.id, eDocElementKind::Transition, text, parent);
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
        // Transitions reference their target by id, so a rename touches only the name and every
        // connection survives. The canvas repaints the affected edge labels on the notification.
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
// SMSetHistoryCommand
//////////////////////////////////////////////////////////////////////////

SMSetHistoryCommand::SMSetHistoryCommand(  StateMachineData& data, DocModelNotifier& notifier
                                         , uint32_t stateId, SMStateEntry::eHistory history
                                         , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mId       (stateId)
    , mNew      (history)
    , mOld      (SMStateEntry::eHistory::None)
{
}

void SMSetHistoryCommand::apply(SMStateEntry::eHistory history)
{
    SMStateEntry* state = data().findStateById(mId);
    if (state != nullptr)
    {
        state->setHistory(history);
        notifier().notifyElementChanged(mId, eDocElementKind::State);
    }
}

void SMSetHistoryCommand::redo()
{
    if (mCaptured == false)
    {
        const SMStateEntry* state = data().findStateById(mId);
        mOld = (state != nullptr ? state->getHistory() : SMStateEntry::eHistory::None);
        mCaptured = true;
    }

    apply(mNew);
}

void SMSetHistoryCommand::undo()
{
    apply(mOld);
}

//////////////////////////////////////////////////////////////////////////
// SMSetSubmachineCommand
//////////////////////////////////////////////////////////////////////////

SMSetSubmachineCommand::SMSetSubmachineCommand(  StateMachineData& data, DocModelNotifier& notifier
                                               , uint32_t stateId, const QString& alias
                                               , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mId       (stateId)
    , mNew      (alias)
{
    const SMStateEntry* state = data.findStateById(stateId);
    mEffective = (state != nullptr)
              && (state->hasNestedStates() == false)
              && (state->getKind() == SMStateEntry::eStateKind::Normal)
              && (state->getSubmachine() != alias);
}

bool SMSetSubmachineCommand::isEffective() const
{
    return mEffective;
}

void SMSetSubmachineCommand::redo()
{
    SMStateEntry* state = (mEffective ? data().findStateById(mId) : nullptr);
    if (state == nullptr)
    {
        return;
    }

    if (mCaptured == false)
    {
        mOldAlias   = state->getSubmachine();
        mOldFinal   = state->getOnFinal();
        mOldHistory = state->getHistory();
        mCaptured   = true;
    }

    state->setSubmachine(mNew);
    if (mNew.isEmpty())
    {
        state->setOnFinal(QString());
        state->setHistory(SMStateEntry::eHistory::None);
    }

    notifier().notifyElementChanged(mId, eDocElementKind::State);
    notifyImportUsage(mOldAlias, mNew);
}

void SMSetSubmachineCommand::notifyImportUsage(const QString& before, const QString& after)
{
    // Whether an import is used is a fact about the states that point at it, so the two imports
    // on either side of this change are told as well. Without it the import list keeps showing
    // the state it had before, and only a change to an import itself would correct it.
    const auto tell = [this](const QString& alias)
    {
        if (alias.isEmpty())
        {
            return;
        }

        const IncludeEntry* import = data().findImportByAlias(alias);
        if (import != nullptr)
        {
            notifier().notifyElementChanged(import->getId(), eDocElementKind::Import);
        }
    };

    tell(before);
    if (after != before)
    {
        tell(after);
    }
}

void SMSetSubmachineCommand::undo()
{
    SMStateEntry* state = (mEffective ? data().findStateById(mId) : nullptr);
    if (state == nullptr)
    {
        return;
    }

    state->setSubmachine(mOldAlias);
    state->setOnFinal(mOldFinal);
    state->setHistory(mOldHistory);
    notifier().notifyElementChanged(mId, eDocElementKind::State);
    notifyImportUsage(mNew, mOldAlias);
}

//////////////////////////////////////////////////////////////////////////
// SMSetOnFinalCommand
//////////////////////////////////////////////////////////////////////////

SMSetOnFinalCommand::SMSetOnFinalCommand(  StateMachineData& data, DocModelNotifier& notifier
                                         , uint32_t stateId, const QString& eventName
                                         , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mId       (stateId)
    , mNew      (eventName)
{
}

void SMSetOnFinalCommand::apply(const QString& eventName)
{
    SMStateEntry* state = data().findStateById(mId);
    if (state != nullptr)
    {
        state->setOnFinal(eventName);
        notifier().notifyElementChanged(mId, eDocElementKind::State);
    }
}

void SMSetOnFinalCommand::redo()
{
    if (mCaptured == false)
    {
        const SMStateEntry* state = data().findStateById(mId);
        mOld = (state != nullptr ? state->getOnFinal() : QString());
        mCaptured = true;
    }

    apply(mNew);
}

void SMSetOnFinalCommand::undo()
{
    apply(mOld);
}

//////////////////////////////////////////////////////////////////////////
// SMConvertToCompositeCommand
//////////////////////////////////////////////////////////////////////////

namespace
{
    /**
     * \class   SMAttachNestedCommand
     * \brief   Moves the ownership of a composite state's nested StateList between the state and
     *          this command, so undo and redo reuse the same list and every id inside it survives.
     *          The constructor picks the direction: attaching makes a state composite, detaching
     *          stops it being one.
     **/
    class SMAttachNestedCommand : public SMCommand
    {
    public:
        enum class eMode
        {
              Attach    //!< Redo attaches the list to the state; undo takes it back.
            , Detach    //!< Redo takes the state's list; undo puts it back.
        };

        SMAttachNestedCommand(  StateMachineData& data, DocModelNotifier& notifier
                              , uint32_t stateId, SMStateData* nested
                              , const QString& text, QUndoCommand* parent
                              , eMode mode = eMode::Attach)
            : SMCommand (data, notifier, text, parent)
            , mStateId  (stateId)
            , mNested   (nested)
            , mMode     (mode)
        {
        }

        void redo() override
        {
            (mMode == eMode::Attach) ? attach() : detach();
        }

        void undo() override
        {
            (mMode == eMode::Attach) ? detach() : attach();
        }

    private:
        void attach()
        {
            SMStateEntry* state = data().findStateById(mStateId);
            if (state != nullptr)
            {
                state->attachNestedStates(mNested.release());
                notifier().notifyElementChanged(mStateId, eDocElementKind::State);
            }
        }

        void detach()
        {
            SMStateEntry* state = data().findStateById(mStateId);
            if (state != nullptr)
            {
                mNested.reset(state->takeNestedStates());
                notifier().notifyElementChanged(mStateId, eDocElementKind::State);
            }
        }

    private:
        uint32_t                        mStateId;   //!< The composite state's ID.
        std::unique_ptr<SMStateData>    mNested;    //!< Owned while the list is off the state.
        eMode                           mMode;
    };
}

SMConvertToCompositeCommand::SMConvertToCompositeCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                         , uint32_t stateId, const QString& startName
                                                         , const QRectF& startGeometry, const QString& text
                                                         , QUndoCommand* parent /*= nullptr*/)
    : SMCompositeCommand(data, notifier, text, parent)
    , mStart            (nullptr)
{
    SMStateEntry* state = data.findStateById(stateId);
    const bool convertible = (state != nullptr)
            && (state->getKind() == SMStateEntry::eStateKind::Normal)
            && (state->isImportedSubmachine() == false)
            && (state->hasNestedStates() == false);
    if (convertible == false)
    {
        return;
    }

    // The list is parented to the state up front so the Start insertion allocates its ID
    // from the document counter even before the attach child has run.
    SMStateData* nested = new SMStateData(state);
    mStart = new SMStateEntry(0, startName, SMStateEntry::eStateKind::Start, nested);

    // Children run in order on redo: attach the list, insert Start (allocates its ID),
    // then create Start's Node layout entry from that ID.
    new SMAttachNestedCommand(data, notifier, stateId, nested, text, this);
    new TDocAddCommand<SMStateEntry*, DocumentElem>(notifier, *nested, mStart, eDocElementKind::State, text, this);
    new SMAttachNodeCommand(data, notifier, *mStart, startGeometry, text, this);
}

bool SMConvertToCompositeCommand::isEffective() const
{
    return (mStart != nullptr);
}

uint32_t SMConvertToCompositeCommand::getStartId() const
{
    return (mStart != nullptr ? mStart->getId() : 0u);
}

//////////////////////////////////////////////////////////////////////////
// SMRemoveStateCommand
//////////////////////////////////////////////////////////////////////////

SMRemoveStateCommand::SMRemoveStateCommand(StateMachineData& data, DocModelNotifier& notifier, SMStateData& level, uint32_t stateId, const QString& text, QUndoCommand* parent)
    : SMCompositeCommand(data, notifier, text, parent)
{
    QList<uint32_t> owners;
    QSet<uint32_t>  stateIds;
    SMStateEntry** slot = level.findElement(stateId);
    if ((slot != nullptr) && (*slot != nullptr))
    {
        collectSubtree(**slot, owners, stateIds);
    }

    const QList<IncomingRef> incoming = collectIncomingRemovals(data, stateIds, owners);

    // Children run forward on redo (layout, incoming transitions, state) and reverse on undo, so
    // the layout is restored last -- after the elements it belongs to are back.
    new SMRemoveLayoutCommand(data, notifier, owners, text, this);
    appendIncomingRemovals(notifier, incoming, text, this);
    new TDocRemoveCommand<SMStateEntry*, DocumentElem>(notifier, level, stateId, eDocElementKind::State, text, this);
}

//////////////////////////////////////////////////////////////////////////
// SMRemoveCompositeCommand
//////////////////////////////////////////////////////////////////////////

SMRemoveCompositeCommand::SMRemoveCompositeCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                   , uint32_t stateId, const QString& text
                                                   , QUndoCommand* parent /*= nullptr*/)
    : SMCompositeCommand(data, notifier, text, parent)
{
    SMStateEntry* state = data.findStateById(stateId);
    if ((state == nullptr) || (state->hasNestedStates() == false))
    {
        return;
    }

    // The host survives, so it is not part of the removed set: only what is inside its nested
    // list goes, at every level.
    QList<uint32_t> owners;
    QSet<uint32_t>  stateIds;
    for (const SMStateEntry* child : state->getNestedStates()->getElements())
    {
        collectSubtree(*child, owners, stateIds);
    }

    const QList<IncomingRef> incoming = collectIncomingRemovals(data, stateIds, owners);
    mEffective   = true;
    mStates      = stateIds.size();
    mTransitions = incoming.size();

    // Same child order as a state delete: layout first so undo restores it last, once the
    // elements it describes are back.
    new SMRemoveLayoutCommand(data, notifier, owners, text, this);
    appendIncomingRemovals(notifier, incoming, text, this);
    // History and OnFinal describe a composition that is about to stop existing; leaving them behind raises.
    if (state->getHistory() != SMStateEntry::eHistory::None)
    {
        new SMSetHistoryCommand(data, notifier, stateId, SMStateEntry::eHistory::None, text, this);
    }

    if (state->getOnFinal().isEmpty() == false)
    {
        new SMSetOnFinalCommand(data, notifier, stateId, QString(), text, this);
    }

    new SMAttachNestedCommand(data, notifier, stateId, nullptr, text, this, SMAttachNestedCommand::eMode::Detach);
}

bool SMRemoveCompositeCommand::isEffective() const
{
    return mEffective;
}

int SMRemoveCompositeCommand::removedStateCount() const
{
    return mStates;
}

int SMRemoveCompositeCommand::removedTransitionCount() const
{
    return mTransitions;
}
