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
 *          the source state's transition list and, for an external transition, creates its
 *          Edge layout at the drop geometry. Internal transitions (no target) carry no edge.
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
                              , const QString& text, QUndoCommand* parent = nullptr);

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
 * \class   SMSetTransitionTargetCommand
 * \brief   Sets or clears a transition's target state (`To`). A target ID of 0 makes the
 *          transition internal; a non-zero one makes it external (target reconnection).
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

#endif  // LUSAN_MODEL_SM_SMTRANSITIONCOMMANDS_HPP
