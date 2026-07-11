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
 *  \file        lusan/model/sm/SMLayoutCommands.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM layout mutation commands.
 *
 ************************************************************************/

#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
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

    // Geometry only - color/expanded state belong to their own edits.
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
// SMAttachEdgeCommand
//////////////////////////////////////////////////////////////////////////

SMAttachEdgeCommand::SMAttachEdgeCommand(  StateMachineData& data, DocModelNotifier& notifier
                                         , const SMTransitionEntry& transition, const SMLayoutEdge& geometry
                                         , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand   (data, notifier, text, parent)
    , mTransition (transition)
    , mGeometry   (geometry)
{
}

void SMAttachEdgeCommand::redo()
{
    const uint32_t owner = mTransition.getId();
    SMLayoutEdge* edge = data().getLayout().findEdge(owner);
    if (edge == nullptr)
    {
        edge = &data().getLayout().addEdge(owner);
    }

    *edge = mGeometry;
    edge->owner = owner;
    notifier().notifyLayoutChanged(QList<uint32_t>{ owner });
}

void SMAttachEdgeCommand::undo()
{
    const uint32_t owner = mTransition.getId();
    data().getLayout().removeOwned(QList<uint32_t>{ owner });
    notifier().notifyLayoutChanged(QList<uint32_t>{ owner });
}

//////////////////////////////////////////////////////////////////////////
// SMSetEdgeGeometryCommand
//////////////////////////////////////////////////////////////////////////

SMSetEdgeGeometryCommand::SMSetEdgeGeometryCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                   , uint32_t owner, uint32_t gestureId, const SMLayoutEdge& geometry
                                                   , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mOwner    (owner)
    , mGesture  (gestureId)
    , mNew      (geometry)
{
    mNew.owner = owner;
}

void SMSetEdgeGeometryCommand::applyTo(const SMLayoutEdge& geometry, bool present)
{
    if (present)
    {
        SMLayoutEdge* edge = data().getLayout().findEdge(mOwner);
        if (edge == nullptr)
        {
            edge = &data().getLayout().addEdge(mOwner);
        }

        *edge = geometry;
        edge->owner = mOwner;
    }
    else
    {
        data().getLayout().removeOwned(QList<uint32_t>{ mOwner });
    }

    notifier().notifyLayoutChanged(QList<uint32_t>{ mOwner });
}

void SMSetEdgeGeometryCommand::redo()
{
    if (mCaptured == false)
    {
        const SMLayoutEdge* edge = data().getLayout().findEdge(mOwner);
        mHadOld = (edge != nullptr);
        mOld    = (edge != nullptr ? *edge : mNew);
        mCaptured = true;
    }

    applyTo(mNew, true);
}

void SMSetEdgeGeometryCommand::undo()
{
    applyTo(mOld, mHadOld);
}

int SMSetEdgeGeometryCommand::id() const
{
    return CMD_ID;
}

bool SMSetEdgeGeometryCommand::mergeWith(const QUndoCommand* other)
{
    const SMSetEdgeGeometryCommand* set = static_cast<const SMSetEdgeGeometryCommand*>(other);
    if ((set->mOwner != mOwner) || (set->mGesture != mGesture))
    {
        return false;
    }

    mNew = set->mNew;   // absorb the latest geometry; the original mOld is kept
    return true;
}

//////////////////////////////////////////////////////////////////////////
// SMSetViewCommand
//////////////////////////////////////////////////////////////////////////

SMSetViewCommand::SMSetViewCommand(  StateMachineData& data, DocModelNotifier& notifier
                                   , uint32_t owner, uint32_t gestureId, const SMLayoutView& view
                                   , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mOwner    (owner)
    , mGesture  (gestureId)
    , mNew      (view)
{
    mNew.owner = owner;
}

void SMSetViewCommand::applyTo(const SMLayoutView& view, bool present)
{
    if (present)
    {
        SMLayoutView* entry = data().getLayout().findView(mOwner);
        if (entry == nullptr)
        {
            entry = &data().getLayout().addView(mOwner);
        }

        *entry = view;
        entry->owner = mOwner;
    }
    else
    {
        data().getLayout().removeView(mOwner);
    }

    notifier().notifyLayoutChanged(QList<uint32_t>{ mOwner });
}

void SMSetViewCommand::redo()
{
    if (mCaptured == false)
    {
        const SMLayoutView* entry = data().getLayout().findView(mOwner);
        mHadOld = (entry != nullptr);
        mOld    = (entry != nullptr ? *entry : mNew);
        mCaptured = true;
    }

    applyTo(mNew, true);
}

void SMSetViewCommand::undo()
{
    applyTo(mOld, mHadOld);
}

int SMSetViewCommand::id() const
{
    return CMD_ID;
}

bool SMSetViewCommand::mergeWith(const QUndoCommand* other)
{
    const SMSetViewCommand* set = static_cast<const SMSetViewCommand*>(other);
    if ((set->mOwner != mOwner) || (set->mGesture != mGesture))
    {
        return false;
    }

    mNew = set->mNew;   // absorb the latest viewport; the original mOld is kept
    return true;
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
// SMAutoPlaceNodesCommand
//////////////////////////////////////////////////////////////////////////

SMAutoPlaceNodesCommand::SMAutoPlaceNodesCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                 , const QList<SMLayoutNode>& nodes
                                                 , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mNodes    (nodes)
    , mIds      ( )
{
    for (const SMLayoutNode& node : mNodes)
    {
        mIds.append(node.owner);
    }
}

void SMAutoPlaceNodesCommand::redo()
{
    SMLayoutData& layout = data().getLayout();
    for (const SMLayoutNode& node : mNodes)
    {
        SMLayoutNode* entry = layout.findNode(node.owner);
        if (entry == nullptr)
        {
            entry = &layout.addNode(node.owner);
        }

        *entry = node;
    }

    notifier().notifyLayoutChanged(mIds);
}

void SMAutoPlaceNodesCommand::undo()
{
    SMLayoutData& layout = data().getLayout();
    for (const uint32_t owner : mIds)
    {
        layout.removeNode(owner);
    }

    notifier().notifyLayoutChanged(mIds);
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

        // A deleted composite's sublevel is one of the removed IDs; its notes go with it.
        // A note bound to a deleted state/transition (its owner) goes with it too.
        for (const SMLayoutNote& note : layout.getNotes())
        {
            if (mIds.contains(note.level) || ((note.owner != 0) && mIds.contains(note.owner)))
            {
                mNotes.append(note);
            }
        }

        mCaptured = true;
    }

    layout.removeOwned(mIds);
    for (const SMLayoutNote& note : mNotes)
    {
        layout.removeNote(note.id);
    }

    notifier().notifyLayoutChanged(mIds);
    for (const SMLayoutNote& note : mNotes)
    {
        notifier().notifyElementRemoved(note.id, eDocElementKind::Note);
    }
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

    for (const SMLayoutNote& note : mNotes)
    {
        layout.restoreNote(note);
    }

    notifier().notifyLayoutChanged(mIds);
    for (const SMLayoutNote& note : mNotes)
    {
        notifier().notifyElementAdded(note.id, eDocElementKind::Note);
    }
}

//////////////////////////////////////////////////////////////////////////
// SMSetGridSizeCommand
//////////////////////////////////////////////////////////////////////////

SMSetGridSizeCommand::SMSetGridSizeCommand(StateMachineData& data, DocModelNotifier& notifier, int gridSize, const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mNew      (gridSize)
{
}

void SMSetGridSizeCommand::redo()
{
    mOld = data().getLayout().getGridSize();
    data().getLayout().setGridSize(mNew);
    notifier().notifyLayoutChanged(QList<uint32_t>());
}

void SMSetGridSizeCommand::undo()
{
    data().getLayout().setGridSize(mOld);
    notifier().notifyLayoutChanged(QList<uint32_t>());
}

//////////////////////////////////////////////////////////////////////////
// SMSetGridVisibleCommand
//////////////////////////////////////////////////////////////////////////

SMSetGridVisibleCommand::SMSetGridVisibleCommand(StateMachineData& data, DocModelNotifier& notifier, bool visible, const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mNew      (visible)
{
}

void SMSetGridVisibleCommand::redo()
{
    mOld = data().getLayout().isGridVisible();
    data().getLayout().setGridVisible(mNew);
    notifier().notifyLayoutChanged(QList<uint32_t>());
}

void SMSetGridVisibleCommand::undo()
{
    data().getLayout().setGridVisible(mOld);
    notifier().notifyLayoutChanged(QList<uint32_t>());
}

//////////////////////////////////////////////////////////////////////////
// SMSetNodeColorCommand
//////////////////////////////////////////////////////////////////////////

SMSetNodeColorCommand::SMSetNodeColorCommand(  StateMachineData& data, DocModelNotifier& notifier
                                             , uint32_t owner, const QString& color
                                             , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mOwner    (owner)
    , mNew      (color)
{
}

void SMSetNodeColorCommand::redo()
{
    SMLayoutNode* node = data().getLayout().findNode(mOwner);
    if (node == nullptr)
    {
        node = &data().getLayout().addNode(mOwner);
    }

    mOld = node->color;
    node->color = mNew;
    notifier().notifyLayoutChanged(QList<uint32_t>{ mOwner });
}

void SMSetNodeColorCommand::undo()
{
    SMLayoutNode* node = data().getLayout().findNode(mOwner);
    if (node != nullptr)
    {
        node->color = mOld;
        notifier().notifyLayoutChanged(QList<uint32_t>{ mOwner });
    }
}

//////////////////////////////////////////////////////////////////////////
// SMSetEdgeColorCommand
//////////////////////////////////////////////////////////////////////////

SMSetEdgeColorCommand::SMSetEdgeColorCommand(  StateMachineData& data, DocModelNotifier& notifier
                                             , uint32_t owner, const QString& color
                                             , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mOwner    (owner)
    , mNew      (color)
{
}

void SMSetEdgeColorCommand::redo()
{
    SMLayoutEdge* edge = data().getLayout().findEdge(mOwner);
    if (edge == nullptr)
    {
        edge = &data().getLayout().addEdge(mOwner);
    }

    mOld = edge->color;
    edge->color = mNew;
    notifier().notifyLayoutChanged(QList<uint32_t>{ mOwner });
}

void SMSetEdgeColorCommand::undo()
{
    SMLayoutEdge* edge = data().getLayout().findEdge(mOwner);
    if (edge != nullptr)
    {
        edge->color = mOld;
        notifier().notifyLayoutChanged(QList<uint32_t>{ mOwner });
    }
}

//////////////////////////////////////////////////////////////////////////
// SMAddNoteCommand
//////////////////////////////////////////////////////////////////////////

SMAddNoteCommand::SMAddNoteCommand(  StateMachineData& data, DocModelNotifier& notifier
                                    , uint32_t level, const QRectF& geometry, const QString& text
                                    , const QString& undoText, QUndoCommand* parent /*= nullptr*/)
    : SMAddNoteCommand(data, notifier, level, 0u, geometry, text, undoText, parent)
{
}

SMAddNoteCommand::SMAddNoteCommand(  StateMachineData& data, DocModelNotifier& notifier
                                    , uint32_t level, uint32_t owner, const QRectF& geometry, const QString& text
                                    , const QString& undoText, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, undoText, parent)
{
    mNote.level  = level;
    mNote.owner  = owner;
    mNote.x      = geometry.x();
    mNote.y      = geometry.y();
    mNote.width  = geometry.width();
    mNote.height = geometry.height();
    mNote.text   = text;
}

void SMAddNoteCommand::redo()
{
    if (mAllocated == false)
    {
        SMLayoutNote& note = data().getLayout().addNote(mNote.level, mNote.owner);
        mNote.id = note.id;
        note = mNote;
        mAllocated = true;
    }
    else
    {
        data().getLayout().restoreNote(mNote);
    }

    notifier().notifyElementAdded(mNote.id, eDocElementKind::Note);
}

void SMAddNoteCommand::undo()
{
    data().getLayout().removeNote(mNote.id);
    notifier().notifyElementRemoved(mNote.id, eDocElementKind::Note);
}

//////////////////////////////////////////////////////////////////////////
// SMMoveNoteCommand
//////////////////////////////////////////////////////////////////////////

SMMoveNoteCommand::SMMoveNoteCommand(  StateMachineData& data, DocModelNotifier& notifier
                                     , uint32_t noteId, uint32_t gestureId
                                     , double x, double y, double width, double height
                                     , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand   (data, notifier, text, parent)
    , mNoteId     (noteId)
    , mGesture    (gestureId)
    , mNewX       (x)
    , mNewY       (y)
    , mNewWidth   (width)
    , mNewHeight  (height)
{
}

void SMMoveNoteCommand::applyTo(double x, double y, double width, double height)
{
    SMLayoutNote* note = data().getLayout().findNote(mNoteId);
    if (note != nullptr)
    {
        note->x      = x;
        note->y      = y;
        note->width  = width;
        note->height = height;
        notifier().notifyElementChanged(mNoteId, eDocElementKind::Note);
    }
}

void SMMoveNoteCommand::redo()
{
    if (mCaptured == false)
    {
        const SMLayoutNote* note = data().getLayout().findNote(mNoteId);
        if (note != nullptr)
        {
            mOldX = note->x; mOldY = note->y; mOldWidth = note->width; mOldHeight = note->height;
        }

        mCaptured = true;
    }

    applyTo(mNewX, mNewY, mNewWidth, mNewHeight);
}

void SMMoveNoteCommand::undo()
{
    applyTo(mOldX, mOldY, mOldWidth, mOldHeight);
}

int SMMoveNoteCommand::id() const
{
    return CMD_ID;
}

bool SMMoveNoteCommand::mergeWith(const QUndoCommand* other)
{
    const SMMoveNoteCommand* move = static_cast<const SMMoveNoteCommand*>(other);
    if ((move->mNoteId != mNoteId) || (move->mGesture != mGesture))
    {
        return false;
    }

    mNewX = move->mNewX; mNewY = move->mNewY; mNewWidth = move->mNewWidth; mNewHeight = move->mNewHeight;
    return true;
}

//////////////////////////////////////////////////////////////////////////
// SMSetNoteTextCommand
//////////////////////////////////////////////////////////////////////////

SMSetNoteTextCommand::SMSetNoteTextCommand(  StateMachineData& data, DocModelNotifier& notifier
                                            , uint32_t noteId, const QString& text
                                            , const QString& undoText, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, undoText, parent)
    , mNoteId   (noteId)
    , mNew      (text)
{
}

void SMSetNoteTextCommand::redo()
{
    SMLayoutNote* note = data().getLayout().findNote(mNoteId);
    if (note != nullptr)
    {
        mOld = note->text;
        note->text = mNew;
        notifier().notifyElementChanged(mNoteId, eDocElementKind::Note);
    }
}

void SMSetNoteTextCommand::undo()
{
    SMLayoutNote* note = data().getLayout().findNote(mNoteId);
    if (note != nullptr)
    {
        note->text = mOld;
        notifier().notifyElementChanged(mNoteId, eDocElementKind::Note);
    }
}

//////////////////////////////////////////////////////////////////////////
// SMSetNoteColorCommand
//////////////////////////////////////////////////////////////////////////

SMSetNoteColorCommand::SMSetNoteColorCommand(  StateMachineData& data, DocModelNotifier& notifier
                                              , uint32_t noteId, const QString& color
                                              , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mNoteId   (noteId)
    , mNew      (color)
{
}

void SMSetNoteColorCommand::redo()
{
    SMLayoutNote* note = data().getLayout().findNote(mNoteId);
    if (note != nullptr)
    {
        mOld = note->color;
        note->color = mNew;
        notifier().notifyElementChanged(mNoteId, eDocElementKind::Note);
    }
}

void SMSetNoteColorCommand::undo()
{
    SMLayoutNote* note = data().getLayout().findNote(mNoteId);
    if (note != nullptr)
    {
        note->color = mOld;
        notifier().notifyElementChanged(mNoteId, eDocElementKind::Note);
    }
}

//////////////////////////////////////////////////////////////////////////
// SMRemoveNoteCommand
//////////////////////////////////////////////////////////////////////////

SMRemoveNoteCommand::SMRemoveNoteCommand(  StateMachineData& data, DocModelNotifier& notifier
                                          , uint32_t noteId, const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand (data, notifier, text, parent)
    , mNoteId   (noteId)
{
}

void SMRemoveNoteCommand::redo()
{
    const SMLayoutNote* note = data().getLayout().findNote(mNoteId);
    if (note != nullptr)
    {
        mCaptured = *note;
        data().getLayout().removeNote(mNoteId);
        notifier().notifyElementRemoved(mNoteId, eDocElementKind::Note);
    }
}

void SMRemoveNoteCommand::undo()
{
    data().getLayout().restoreNote(mCaptured);
    notifier().notifyElementAdded(mNoteId, eDocElementKind::Note);
}
