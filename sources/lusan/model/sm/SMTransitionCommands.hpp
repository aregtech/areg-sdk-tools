#ifndef LUSAN_MODEL_SM_SMTRANSITIONCOMMANDS_HPP
#define LUSAN_MODEL_SM_SMTRANSITIONCOMMANDS_HPP
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
 *  \file        lusan/model/sm/SMTransitionCommands.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition create/delete/reconnect commands.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/sm/SMCommand.hpp"
#include "lusan/data/sm/SMLayoutData.hpp"
#include "lusan/data/sm/SMTransition.hpp"

#include <QList>
#include <QPointF>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMStateEntry;

/**
 * \class   SMCreateTransitionCommand
 * \brief   Creates a transition on its source state as one undo step: appends the entry to
 *          the source state's transition list and, when it has a target, creates its Edge
 *          layout at the drop geometry. Internal transitions carry no edge.
 *          The transition's ID is allocated by the insertion; read it back with
 *          getTransitionId() after the push.
 **/
class SMCreateTransitionCommand : public SMCompositeCommand
{
public:
    SMCreateTransitionCommand(  StateMachineData& data, DocModelNotifier& notifier
                              , SMStateEntry& source, SMTransitionEntry::eStimulusKind kind
                              , const QString& stimulus, uint32_t targetId
                              , const QList<QPointF>& edgePoints
                              , const QString& text, QUndoCommand* parent = nullptr
                              , SMTransitionEntry::eTransitionKind transKind = SMTransitionEntry::eTransitionKind::External);

    /**
     * \brief   The created transition's element ID; valid after the first redo (push).
     **/
    uint32_t getTransitionId() const;

private:
    SMTransitionEntry*  mTransition;    //!< The created entry (owned by the add child / container).
};

/**
 * \class   SMRemoveTransitionCommand
 * \brief   Deletes a transition as one undo step: its Edge layout and the entry itself
 *          (with its captured conditions and operations), restored exactly on undo.
 **/
class SMRemoveTransitionCommand : public SMCompositeCommand
{
public:
    SMRemoveTransitionCommand(  StateMachineData& data, DocModelNotifier& notifier
                              , SMStateEntry& source, uint32_t transitionId
                              , const QString& text, QUndoCommand* parent = nullptr);
};

/**
 * \class   SMSetTransitionKindCommand
 * \brief   Sets what a transition IS -- \ref SMTransitionEntry::eTransitionKind. Switching to
 *          Internal drops the target (an internal transition has none by definition) and undo
 *          puts it back, so the round trip through the combo never loses where the edge pointed.
 **/
class SMSetTransitionKindCommand : public SMCommand
{
public:
    SMSetTransitionKindCommand(  StateMachineData& data, DocModelNotifier& notifier
                               , uint32_t transitionId, SMTransitionEntry::eTransitionKind kind
                               , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void apply(SMTransitionEntry::eTransitionKind kind, uint32_t targetId);

private:
    uint32_t                            mId;                //!< The transition's ID.
    SMTransitionEntry::eTransitionKind  mNewKind;
    SMTransitionEntry::eTransitionKind  mOldKind { SMTransitionEntry::eTransitionKind::External };
    uint32_t                            mOldTarget { 0 };   //!< The target Internal drops, restored on undo.
    bool                                mCaptured { false };
};

/**
 * \class   SMSetTransitionTargetCommand
 * \brief   Sets or clears a transition's target state (`To`). A target ID of 0 leaves the
 *          transition unconnected -- it does NOT change the kind; a non-zero one (re)connects it.
 **/
class SMSetTransitionTargetCommand : public SMCommand
{
public:
    SMSetTransitionTargetCommand(  StateMachineData& data, DocModelNotifier& notifier
                                 , uint32_t transitionId, uint32_t targetId
                                 , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void apply(uint32_t targetId);

private:
    uint32_t    mId;                    //!< The transition's ID.
    uint32_t    mNewTarget;             //!< The new target state ID (0 = internal).
    uint32_t    mOldTarget { 0 };       //!< The previous target state ID, captured on first redo.
    bool        mCaptured { false };
};

/**
 * \class   SMSetStimulusCommand
 * \brief   Sets a transition's stimulus (kind + name) over the shared registries.
 **/
class SMSetStimulusCommand : public SMCommand
{
public:
    SMSetStimulusCommand(  StateMachineData& data, DocModelNotifier& notifier
                         , uint32_t transitionId, SMTransitionEntry::eStimulusKind kind
                         , const QString& stimulus, const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void apply(SMTransitionEntry::eStimulusKind kind, const QString& stimulus);

private:
    uint32_t                            mId;        //!< The transition's ID.
    SMTransitionEntry::eStimulusKind    mNewKind;
    QString                             mNewName;
    SMTransitionEntry::eStimulusKind    mOldKind { SMTransitionEntry::eStimulusKind::Trigger };
    QString                             mOldName;
    bool                                mCaptured { false };
};

/**
 * \class   SMReparentTransitionCommand
 * \brief   Moves a transition to a different source state (begin-endpoint reconnection),
 *          preserving its conditions and operations. The container reassigns the entry's ID
 *          when its owner changes; the new ID is allocated once on the first redo and reused
 *          afterwards, and the transition's Edge layout is re-keyed to it — so redo restores
 *          a stable ID and undo puts the entry back under its original ID and position.
 **/
class SMReparentTransitionCommand : public SMCommand
{
public:
    SMReparentTransitionCommand(  StateMachineData& data, DocModelNotifier& notifier
                                , SMStateEntry& oldSource, SMStateEntry& newSource
                                , uint32_t transitionId, const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    SMTransitionData&   mOldList;       //!< The transition list it leaves.
    SMTransitionData&   mNewList;       //!< The transition list it joins.
    uint32_t            mOldId;         //!< The ID in the old list.
    uint32_t            mNewId { 0 };   //!< The ID in the new list (allocated on first redo).
    int                 mOldIndex { -1 };//!< The recorded position in the old list.
    SMLayoutEdge        mEdge;          //!< The captured Edge layout re-keyed across the move.
    bool                mHadEdge { false };
};

/**
 * \class   SMMoveTransitionCommand
 * \brief   Moves a transition to another position in its source state's list -- which IS its
 *          priority, since document order decides which of several transitions on one stimulus
 *          runs (see \ref SMTransitionData and the shadowing rule in SMValidator).
 *
 *          It moves the ENTRY and leaves every ID alone. The generic \ref TDocReorderCommand
 *          cannot serve here: it swaps positions through `TEDataContainer::swapElements`, which
 *          exchanges the two entries' IDs as well, so the layout `Edge` keyed by transition ID,
 *          the guard's parameter scope and any queued command would follow the number onto the
 *          wrong transition. \ref TEDataContainer::moveElement is the ID-preserving primitive.
 *
 *          A move is its own inverse in the obvious way -- undo moves it back -- and it notifies
 *          `listReordered(stateId, Transition)`, which the canvas already answers by rebuilding
 *          the state bodies and refreshing the edges.
 **/
class SMMoveTransitionCommand : public SMCommand
{
public:
    SMMoveTransitionCommand(  StateMachineData& data, DocModelNotifier& notifier
                            , SMStateEntry& owner, uint32_t transitionId, int newIndex
                            , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void apply(int from, int to);

private:
    SMTransitionData&   mList;          //!< The owning state's transition list.
    uint32_t            mStateId;       //!< The owning state, for the notification.
    uint32_t            mId;            //!< The transition being moved.
    int                 mNewIndex;      //!< Where it goes.
    int                 mOldIndex { -1 };//!< Where it came from, captured on the first redo.
    bool                mCaptured { false };
};

#endif  // LUSAN_MODEL_SM_SMTRANSITIONCOMMANDS_HPP
