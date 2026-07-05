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
#include "lusan/data/sm/StateMachineData.hpp"

//////////////////////////////////////////////////////////////////////////
// SMMoveNodeCommand
//////////////////////////////////////////////////////////////////////////

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

void SMMoveNodeCommand::redo(void)
{
    if (mCaptured == false)
    {
        SMLayoutNode* node = data().getLayout().findNode(mOwner);
        mOld = (node != nullptr ? *node : mNew);
        mCaptured = true;
    }

    applyTo(mNew);
}

void SMMoveNodeCommand::undo(void)
{
    applyTo(mOld);
}

int SMMoveNodeCommand::id(void) const
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
// SMRemoveLayoutCommand
//////////////////////////////////////////////////////////////////////////

SMRemoveLayoutCommand::SMRemoveLayoutCommand(StateMachineData& data, DocModelNotifier& notifier, const QList<uint32_t>& ownerIds, const QString& text, QUndoCommand* parent)
    : SMCommand (data, notifier, text, parent)
    , mIds      (ownerIds)
{
}

void SMRemoveLayoutCommand::redo(void)
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

void SMRemoveLayoutCommand::undo(void)
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
