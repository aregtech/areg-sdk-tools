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
 *  \file        lusan/model/sm/SMLayoutCommands.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM layout mutation commands.
 *
 ************************************************************************/

#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

//////////////////////////////////////////////////////////////////////////
// SMMoveNodeCommand
//////////////////////////////////////////////////////////////////////////

uint32_t SMMoveNodeCommand::takeNextGesture()
{
    static uint32_t _gesture{ 0u };
    return ++_gesture;
}

SMMoveNodeCommand::SMMoveNodeCommand(  StateMachineData& data, DocModelNotifier& notifier
                                     , uint32_t owner, uint32_t gestureId
                                     , double x, double y, double width, double height
                                     , const QString& text, QUndoCommand* parent)
    : SMCommand (data, notifier, text, parent)
    , mOwner    (owner)
    , mGesture  (gestureId)
{
    mNew.owner  = owner;
    mNew.x      = x;
    mNew.y      = y;
    mNew.width  = width;
    mNew.height = height;
}

void SMMoveNodeCommand::applyTo(const SMLayoutNode& geometry)
{
    SMLayoutNode* node = data().getLayout().findNode(mOwner);
    if (node == nullptr)
    {
        node = &data().getLayout().addNode(mOwner);
    }

    // Geometry only — color/expanded state belong to their own edits.
    node->x      = geometry.x;
    node->y      = geometry.y;
    node->width  = geometry.width;
    node->height = geometry.height;

    notifier().notifyLayoutChanged(QList<uint32_t>{ mOwner });
}

void SMMoveNodeCommand::redo()
{
    if (mCaptured == false)
    {
        SMLayoutNode* node = data().getLayout().findNode(mOwner);
        mOld = (node != nullptr ? *node : mNew);
        mCaptured = true;
    }

    applyTo(mNew);
}

void SMMoveNodeCommand::undo()
{
    applyTo(mOld);
}

int SMMoveNodeCommand::id() const
{
    return CMD_ID;
}

bool SMMoveNodeCommand::mergeWith(const QUndoCommand* other)
{
    const SMMoveNodeCommand* move = static_cast<const SMMoveNodeCommand*>(other);
    if ((move->mOwner != mOwner) || (move->mGesture != mGesture))
    {
        return false;
    }

    mNew = move->mNew;   // absorb the latest geometry; the original mOld is kept
    return true;
}

//////////////////////////////////////////////////////////////////////////
// SMAttachNodeCommand
//////////////////////////////////////////////////////////////////////////

SMAttachNodeCommand::SMAttachNodeCommand(  StateMachineData& data, DocModelNotifier& notifier
                                         , const SMStateEntry& state, const QRectF& geometry
                                         , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mState    (state)
    , mGeometry (geometry)
{
}

void SMAttachNodeCommand::redo()
{
    const uint32_t owner = mState.getId();
    SMLayoutNode* node = data().getLayout().findNode(owner);
    if (node == nullptr)
    {
        node = &data().getLayout().addNode(owner);
    }

    node->x      = mGeometry.x();
    node->y      = mGeometry.y();
    node->width  = mGeometry.width();
    node->height = mGeometry.height();
    notifier().notifyLayoutChanged(QList<uint32_t>{ owner });
}

void SMAttachNodeCommand::undo()
{
    const uint32_t owner = mState.getId();
    data().getLayout().removeOwned(QList<uint32_t>{ owner });
    notifier().notifyLayoutChanged(QList<uint32_t>{ owner });
}

//////////////////////////////////////////////////////////////////////////
// SMSetNodeExpandedCommand
//////////////////////////////////////////////////////////////////////////

SMSetNodeExpandedCommand::SMSetNodeExpandedCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                   , uint32_t owner, bool expanded
                                                   , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mOwner    (owner)
    , mNew      (expanded)
{
}

void SMSetNodeExpandedCommand::apply(bool expanded)
{
    SMLayoutNode* node = data().getLayout().findNode(mOwner);
    if (node != nullptr)
    {
        node->hasExpanded = true;
        node->expanded    = expanded;
        notifier().notifyLayoutChanged(QList<uint32_t>{ mOwner });
    }
}

void SMSetNodeExpandedCommand::redo()
{
    if (mCaptured == false)
    {
        const SMLayoutNode* node = data().getLayout().findNode(mOwner);
        mOldHas      = (node != nullptr) && node->hasExpanded;
        mOldExpanded = (node != nullptr) ? node->expanded : true;
        mCaptured    = true;
    }

    apply(mNew);
}

void SMSetNodeExpandedCommand::undo()
{
    SMLayoutNode* node = data().getLayout().findNode(mOwner);
    if (node != nullptr)
    {
        node->hasExpanded = mOldHas;
        node->expanded    = mOldExpanded;
        notifier().notifyLayoutChanged(QList<uint32_t>{ mOwner });
    }
}

//////////////////////////////////////////////////////////////////////////
// SMRemoveLayoutCommand
//////////////////////////////////////////////////////////////////////////

SMRemoveLayoutCommand::SMRemoveLayoutCommand(StateMachineData& data, DocModelNotifier& notifier, const QList<uint32_t>& ownerIds, const QString& text, QUndoCommand* parent)
    : SMCommand (data, notifier, text, parent)
    , mIds      (ownerIds)
{
}

void SMRemoveLayoutCommand::redo()
{
    SMLayoutData& layout = data().getLayout();
    if (mCaptured == false)
    {
        for (const SMLayoutView& view : layout.getViews())
        {
            if (mIds.contains(view.owner))
            {
                mViews.append(view);
            }
        }

        for (const SMLayoutNode& node : layout.getNodes())
        {
            if (mIds.contains(node.owner))
            {
                mNodes.append(node);
            }
        }

        for (const SMLayoutEdge& edge : layout.getEdges())
        {
            if (mIds.contains(edge.owner))
            {
                mEdges.append(edge);
            }
        }

        mCaptured = true;
    }

    layout.removeOwned(mIds);
    notifier().notifyLayoutChanged(mIds);
}

void SMRemoveLayoutCommand::undo()
{
    SMLayoutData& layout = data().getLayout();
    for (const SMLayoutView& view : mViews)
    {
        layout.addView(view.owner) = view;
    }

    for (const SMLayoutNode& node : mNodes)
    {
        layout.addNode(node.owner) = node;
    }

    for (const SMLayoutEdge& edge : mEdges)
    {
        layout.addEdge(edge.owner) = edge;
    }

    notifier().notifyLayoutChanged(mIds);
}
