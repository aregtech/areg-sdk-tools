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
 *  \file        lusan/model/sm/SMTransitionCommands.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition create/delete/reconnect commands.
 *
 ************************************************************************/

#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

//////////////////////////////////////////////////////////////////////////
// SMCreateTransitionCommand
//////////////////////////////////////////////////////////////////////////

SMCreateTransitionCommand::SMCreateTransitionCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                     , SMStateEntry& source, SMTransitionEntry::eStimulusKind kind
                                                     , const QString& stimulus, uint32_t targetId
                                                     , const QList<QPointF>& edgePoints
                                                     , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCompositeCommand(data, notifier, text, parent)
    , mTransition       (new SMTransitionEntry(0, kind, stimulus, &source.getTransitions()))
{
    if (targetId != 0)
    {
        mTransition->setToId(targetId);
    }

    // The add runs first on redo and allocates the ID the edge child reads.
    new TDocAddCommand<SMTransitionEntry*, DocumentElem>(notifier, source.getTransitions(), mTransition, eDocElementKind::Transition, text, this);
    if ((targetId != 0) && (edgePoints.isEmpty() == false))
    {
        SMLayoutEdge geometry;
        geometry.shape  = SMLayoutEdge::eShape::Line;
        geometry.points = edgePoints;
        new SMAttachEdgeCommand(data, notifier, *mTransition, geometry, text, this);
    }
}

uint32_t SMCreateTransitionCommand::getTransitionId() const
{
    return mTransition->getId();
}

//////////////////////////////////////////////////////////////////////////
// SMRemoveTransitionCommand
//////////////////////////////////////////////////////////////////////////

SMRemoveTransitionCommand::SMRemoveTransitionCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                     , SMStateEntry& source, uint32_t transitionId
                                                     , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCompositeCommand(data, notifier, text, parent)
{
    new SMRemoveLayoutCommand(data, notifier, QList<uint32_t>{ transitionId }, text, this);
    new TDocRemoveCommand<SMTransitionEntry*, DocumentElem>(notifier, source.getTransitions(), transitionId, eDocElementKind::Transition, text, this);
}

//////////////////////////////////////////////////////////////////////////
// SMSetTransitionTargetCommand
//////////////////////////////////////////////////////////////////////////

SMSetTransitionTargetCommand::SMSetTransitionTargetCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                           , uint32_t transitionId, uint32_t targetId
                                                           , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand     (data, notifier, text, parent)
    , mId           (transitionId)
    , mNewTarget    (targetId)
{
}

void SMSetTransitionTargetCommand::apply(uint32_t targetId)
{
    SMTransitionEntry* transition = data().findTransitionById(mId);
    if (transition != nullptr)
    {
        if (targetId != 0)
        {
            transition->setToId(targetId);
        }
        else
        {
            transition->clearTo();
        }

        notifier().notifyElementChanged(mId, eDocElementKind::Transition);
    }
}

void SMSetTransitionTargetCommand::redo()
{
    if (mCaptured == false)
    {
        const SMTransitionEntry* transition = data().findTransitionById(mId);
        mOldTarget = (transition != nullptr ? transition->getToId() : 0);
        mCaptured  = true;
    }

    apply(mNewTarget);
}

void SMSetTransitionTargetCommand::undo()
{
    apply(mOldTarget);
}

//////////////////////////////////////////////////////////////////////////
// SMSetStimulusCommand
//////////////////////////////////////////////////////////////////////////

SMSetStimulusCommand::SMSetStimulusCommand(  StateMachineData& data, DocModelNotifier& notifier
                                           , uint32_t transitionId, SMTransitionEntry::eStimulusKind kind
                                           , const QString& stimulus, const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mId       (transitionId)
    , mNewKind  (kind)
    , mNewName  (stimulus)
{
}

void SMSetStimulusCommand::apply(SMTransitionEntry::eStimulusKind kind, const QString& stimulus)
{
    SMTransitionEntry* transition = data().findTransitionById(mId);
    if (transition != nullptr)
    {
        transition->setStimulusKind(kind);
        transition->setStimulus(stimulus);
        notifier().notifyElementChanged(mId, eDocElementKind::Transition);
    }
}

void SMSetStimulusCommand::redo()
{
    if (mCaptured == false)
    {
        const SMTransitionEntry* transition = data().findTransitionById(mId);
        if (transition != nullptr)
        {
            mOldKind = transition->getStimulusKind();
            mOldName = transition->getStimulus();
        }

        mCaptured = true;
    }

    apply(mNewKind, mNewName);
}

void SMSetStimulusCommand::undo()
{
    apply(mOldKind, mOldName);
}

//////////////////////////////////////////////////////////////////////////
// SMReparentTransitionCommand
//////////////////////////////////////////////////////////////////////////

SMReparentTransitionCommand::SMReparentTransitionCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                         , SMStateEntry& oldSource, SMStateEntry& newSource
                                                         , uint32_t transitionId, const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mOldList  (oldSource.getTransitions())
    , mNewList  (newSource.getTransitions())
    , mOldId    (transitionId)
{
}

void SMReparentTransitionCommand::redo()
{
    if (mNewId == 0)
    {
        mOldIndex = mOldList.findIndex(mOldId);
        const SMLayoutEdge* edge = data().getLayout().findEdge(mOldId);
        mHadEdge = (edge != nullptr);
        mEdge    = (edge != nullptr ? *edge : SMLayoutEdge());
        mNewId   = mNewList.getNextId();
    }

    SMTransitionEntry* moved = nullptr;
    if (mOldList.removeElement(mOldId, &moved) == false)
    {
        return;
    }

    data().getLayout().removeOwned(QList<uint32_t>{ mOldId });
    if (mHadEdge)
    {
        SMLayoutEdge& edge = data().getLayout().addEdge(mNewId);
        edge = mEdge;
        edge.owner = mNewId;
    }

    moved->setParent(&mNewList);
    moved->setId(mNewId);
    mNewList.addElement(moved, false);

    notifier().notifyElementRemoved(mOldId, eDocElementKind::Transition);
    notifier().notifyElementAdded(mNewId, eDocElementKind::Transition);
    notifier().notifyLayoutChanged(QList<uint32_t>{ mOldId, mNewId });
}

void SMReparentTransitionCommand::undo()
{
    SMTransitionEntry* moved = nullptr;
    if (mNewList.removeElement(mNewId, &moved) == false)
    {
        return;
    }

    data().getLayout().removeOwned(QList<uint32_t>{ mNewId });
    if (mHadEdge)
    {
        SMLayoutEdge& edge = data().getLayout().addEdge(mOldId);
        edge = mEdge;
        edge.owner = mOldId;
    }

    moved->setParent(&mOldList);
    moved->setId(mOldId);
    mOldList.insertElement(mOldIndex < 0 ? mOldList.getElementCount() : mOldIndex, moved, false);

    notifier().notifyElementRemoved(mNewId, eDocElementKind::Transition);
    notifier().notifyElementAdded(mOldId, eDocElementKind::Transition);
    notifier().notifyLayoutChanged(QList<uint32_t>{ mOldId, mNewId });
}
