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
 *  \file        lusan/view/sm/SMScene.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas scene of one machine level.
 *
 ************************************************************************/

#include "lusan/view/sm/SMScene.hpp"

#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMCanvasItem.hpp"
#include "lusan/view/sm/SMEdgeItem.hpp"
#include "lusan/view/sm/SMNoteItem.hpp"
#include "lusan/view/sm/SMStateItem.hpp"
#include "lusan/view/sm/SMSubmachinePeek.hpp"

#include "lusan/data/sm/SMLayoutData.hpp"
#include "lusan/data/sm/SMState.hpp"

#include <QCoreApplication>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPainter>
#include <QPalette>
#include <QSet>
#include <QVarLengthArray>

#include <algorithm>
#include <cmath>

namespace
{
    bool hasInlineEditorFocus(const QGraphicsScene& scene)
    {
        return (qgraphicsitem_cast<QGraphicsProxyWidget*>(scene.focusItem()) != nullptr);
    }
}

SMScene::SMScene(StateMachineModel& model, uint32_t levelId, QObject* parent /*= nullptr*/)
    : QGraphicsScene(parent)
    , mModel        (model)
    , mLevelId      (levelId)
    , mItems        ( )
    , mTool         (createCanvasTool(NESMDesign::eCanvasTool::Select, *this))
    , mToolSticky   (false)
    , mToolModifiers(Qt::NoModifier)
    , mGridSize     (NESMDesign::GridSizeDefault)
    , mGridVisible  (true)
    , mGridStyle    (NESMDesign::eGridStyle::Lines)
    , mGridDotSize  (NESMDesign::GridDotSizeDefault)
    , mSnapToGrid   (true)
    , mMouseDrag    (false)
    , mDragLeader   (nullptr)
    , mDragLeaderPos( )
    , mDragOrigins  ( )
    , mSyncSelection(false)
    , mPeek         (nullptr)
{
    const double half{ NESMDesign::SceneExtent / 2.0 };
    setSceneRect(-half, -half, NESMDesign::SceneExtent, NESMDesign::SceneExtent);
    setItemIndexMethod(QGraphicsScene::BspTreeIndex);

    connect(this, &QGraphicsScene::selectionChanged, this, &SMScene::onSceneSelectionChanged);
    connect(&mModel.getSelectionModel(), &SMSelectionModel::signalSelectionChanged, this, &SMScene::onModelSelectionChanged);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementAdded, this, &SMScene::onElementAdded);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMScene::onElementRemoved);
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMScene::onElementChanged);
    connect(&notifier, &DocModelNotifier::listReordered, this, &SMScene::onListReordered);
    connect(&notifier, &DocModelNotifier::nameChanged, this, &SMScene::onNameChanged);
    connect(&notifier, &DocModelNotifier::layoutChanged, this, &SMScene::onLayoutChanged);

    // Live name mirroring: typing in the Properties panel name field paints onto the canvas
    // box in real time (the reverse of SMStateItem publishing while its inline editor is open).
    connect(&mModel, &StateMachineModel::signalStateNamePreview, this, [this](uint32_t stateId, const QString& text)
    {
        if (SMStateItem* item = stateItem(stateId))
        {
            item->setNamePreview(text);
        }
    });

    populateFromModel();
}

SMScene::~SMScene()
{
    // The base destructor deletes the items; deleting a selected item emits
    // selectionChanged, which must not re-enter our slots on a half-destroyed scene.
    disconnect(this, nullptr, this, nullptr);
    mModel.getNotifier().disconnect(this);
    mModel.getSelectionModel().disconnect(this);

    // Items unregister from the scene hash while being destroyed; drop the hash first.
    mItems.clear();
}

void SMScene::setGridSize(int gridSize)
{
    gridSize = std::max(gridSize, NESMDesign::GridSizeMin);
    if (gridSize != mGridSize)
    {
        mGridSize = gridSize;
        invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
        emit signalGridChanged();
    }
}

void SMScene::setGridVisible(bool visible)
{
    if (visible != mGridVisible)
    {
        mGridVisible = visible;
        invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
        emit signalGridChanged();
    }
}

void SMScene::setGridStyle(NESMDesign::eGridStyle style)
{
    if (style != mGridStyle)
    {
        mGridStyle = style;
        invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
        emit signalGridChanged();
    }
}

void SMScene::setGridDotSize(int dotSize)
{
    dotSize = std::clamp(dotSize, NESMDesign::GridDotSizeMin, NESMDesign::GridDotSizeMax);
    if (dotSize != mGridDotSize)
    {
        mGridDotSize = dotSize;
        if (mGridVisible && (mGridStyle == NESMDesign::eGridStyle::Dots))
        {
            invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
        }

        emit signalGridChanged();
    }
}

void SMScene::setSnapToGrid(bool snap)
{
    if (snap != mSnapToGrid)
    {
        mSnapToGrid = snap;
        emit signalGridChanged();
    }
}

QPointF SMScene::snappedPosition(const QPointF& position) const
{
    return (mSnapToGrid ? NESMDesign::snapPoint(position, mGridSize) : position);
}

void SMScene::setDragLeader(const QGraphicsItem& item)
{
    mDragLeader    = &item;
    mDragLeaderPos = item.pos();
}

QPointF SMScene::snapDragPosition(const QGraphicsItem& item, const QPointF& position)
{
    if (isInteractiveSnap() == false)
    {
        return position;
    }

    // The first position change of the gesture reaches an item before the drag has moved it, so
    // its own position is where it started from.
    auto origin = mDragOrigins.find(&item);
    if (origin == mDragOrigins.end())
    {
        origin = mDragOrigins.insert(&item, item.pos());
    }

    // The step is the same for every item of the selection; snap it once, against the item the
    // drag started on. That item -- or a lone dragged item, which is its own reference -- still
    // lands on the grid, and the rest of the selection travels the identical distance instead of
    // each one rounding to its own nearest cell (issue #550 bug 1).
    const QPointF reference = ((mDragLeader != nullptr) ? mDragLeaderPos : *origin);
    const QPointF step      = position - *origin;
    return (*origin + (NESMDesign::snapPoint(reference + step, mGridSize) - reference));
}

void SMScene::setActiveTool(NESMDesign::eCanvasTool tool, bool sticky /*= false*/)
{
    if ((mTool != nullptr) && (mTool->getKind() == tool))
    {
        mToolSticky = sticky;
        return;
    }

    std::unique_ptr<SMCanvasTool> created{ createCanvasTool(tool, *this) };
    if (created == nullptr)
    {
        created = createCanvasTool(NESMDesign::eCanvasTool::Select, *this);
        sticky  = false;
    }

    if (mTool != nullptr)
    {
        mTool->cancelGesture();
    }

    // Arming a tool is a mode switch, so an open in-place editor ends here -- before the tool
    // change is announced. An editor left open owns a proxy widget whose cursor QGraphicsView
    // remembers and restores onto the viewport when the pointer leaves it; that restored copy
    // is captured from before the tool armed, so it masks the crosshair the tool then sets.
    // Ending the editors first puts that restore ahead of applyToolCursor() (issue 8).
    closeInlineEditors();

    // A tool may switch tools from inside its own event handler; keep the replaced
    // object alive until the next switch so its call frame stays valid.
    mRetiredTool = std::move(mTool);
    mTool        = std::move(created);
    mToolSticky  = sticky;
    mTool->activate();
    emit signalToolChanged(mTool->getKind());
}

void SMScene::cancelActiveGesture()
{
    if (mTool != nullptr)
    {
        mTool->cancelGesture();
    }

    setActiveTool(NESMDesign::eCanvasTool::Select);
}

void SMScene::finishToolGesture()
{
    // Ctrl held through the gesture repeats the tool: the arming stays for the next gesture
    // only, so the first plain click ends the run and drops back to Select (issue #541).
    const bool repeat = mToolSticky || mToolModifiers.testFlag(Qt::ControlModifier);
    if ((repeat == false) && (getActiveTool() != NESMDesign::eCanvasTool::Select))
    {
        setActiveTool(NESMDesign::eCanvasTool::Select);
    }
}

QRectF SMScene::contentBounds() const
{
    return itemsBoundingRect();
}

void SMScene::selectAll()
{
    mSyncSelection = true;
    for (QGraphicsItem* item : items())
    {
        if (item->flags().testFlag(QGraphicsItem::ItemIsSelectable))
        {
            item->setSelected(true);
        }
    }

    mSyncSelection = false;
    onSceneSelectionChanged();
}

void SMScene::registerCanvasItem(SMCanvasItem& item)
{
    mItems.insert(item.getElementId(), &item);
}

void SMScene::unregisterCanvasItem(SMCanvasItem& item)
{
    if (mItems.value(item.getElementId(), nullptr) == &item)
    {
        mItems.remove(item.getElementId());
    }

    // An item that leaves the scene mid-gesture takes its drag bookkeeping with it, so a later
    // allocation at the same address cannot inherit a start position that was never its own.
    mDragOrigins.remove(&item);
    if (mDragLeader == &item)
    {
        mDragLeader = nullptr;
    }
}

void SMScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    const QPalette palette{ (views().isEmpty() == false) ? views().first()->palette() : QPalette() };
    painter->fillRect(rect, NESMDesign::canvasBackground(palette));

    if (mGridVisible == false)
    {
        return;
    }

    // Fade the grid with the view scale; skip it entirely when the cells collapse.
    const double cellPixels = painter->worldTransform().m11() * static_cast<double>(mGridSize);
    if (cellPixels < NESMDesign::GridHidePixels)
    {
        return;
    }

    const double range   = NESMDesign::GridFullPixels - NESMDesign::GridHidePixels;
    const double opacity = std::min(1.0, (cellPixels - NESMDesign::GridHidePixels) / range);

    const qreal grid = static_cast<qreal>(mGridSize);
    const qreal left = std::floor(rect.left() / grid) * grid;
    const qreal top  = std::floor(rect.top() / grid) * grid;

    if (mGridStyle == NESMDesign::eGridStyle::Dots)
    {
        QVarLengthArray<QPointF, 1024> dots;
        for (qreal x = left; x <= rect.right(); x += grid)
        {
            for (qreal y = top; y <= rect.bottom(); y += grid)
            {
                dots.append(QPointF(x, y));
            }
        }

        QPen pen{ NESMDesign::gridDotColor(palette, opacity) };
        pen.setCosmetic(true);
        pen.setWidthF(static_cast<qreal>(mGridDotSize));
        pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
        painter->drawPoints(dots.constData(), dots.size());
        return;
    }

    QVarLengthArray<QLineF, 256> lines;
    for (qreal x = left; x <= rect.right(); x += grid)
    {
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    }

    for (qreal y = top; y <= rect.bottom(); y += grid)
    {
        lines.append(QLineF(rect.left(), y, rect.right(), y));
    }

    QPen pen{ NESMDesign::gridColor(palette, opacity) };
    pen.setCosmetic(true);
    pen.setWidthF(1.0);
    painter->setPen(pen);
    painter->drawLines(lines.constData(), lines.size());
}

void SMScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    mToolModifiers = event->modifiers();
    if (event->button() == Qt::LeftButton)
    {
        mMouseDrag = true;
        // A new gesture: the previous one's reference item and start positions are stale.
        mDragLeader = nullptr;
        mDragOrigins.clear();

        // Select-tool border drag: a press in a state's border band starts a transition --
        // unless a selected edge's grab handle lies under the cursor: endpoint and
        // waypoint drags on the edge win over the border band.
        if ((mTool != nullptr) && (mTool->getKind() == NESMDesign::eCanvasTool::Select))
        {
            bool onEdgeHandle = false;
            for (SMEdgeItem* edge : selectedEdgeItems())
            {
                if (edge->hitsHandle(event->scenePos()))
                {
                    onEdgeHandle = true;
                    break;
                }
            }

            SMStateItem* source = (onEdgeHandle ? nullptr : stateAt(event->scenePos()));
            if ((source != nullptr) && source->isBorderDragZone(event->scenePos()))
            {
                setActiveTool(NESMDesign::eCanvasTool::AddTransition);
                SMTransitionTool* tool = dynamic_cast<SMTransitionTool*>(mTool.get());
                if (tool != nullptr)
                {
                    tool->beginDragFrom(source->getElementId(), event->scenePos());
                    event->accept();
                    return;
                }
            }
        }
    }

    if ((mTool == nullptr) || (mTool->mousePress(event) == false))
    {
        QGraphicsScene::mousePressEvent(event);
    }
}

void SMScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    mToolModifiers = event->modifiers();
    if ((mTool == nullptr) || (mTool->mouseMove(event) == false))
    {
        QGraphicsScene::mouseMoveEvent(event);
    }
}

void SMScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    mToolModifiers = event->modifiers();
    const bool handled = (mTool != nullptr) && mTool->mouseRelease(event);
    if (handled == false)
    {
        QGraphicsScene::mouseReleaseEvent(event);
    }

    if (event->button() == Qt::LeftButton)
    {
        mMouseDrag  = false;
        mDragLeader = nullptr;
        mDragOrigins.clear();
    }
}

void SMScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    mToolModifiers = event->modifiers();
    if (event->modifiers().testFlag(Qt::AltModifier))
    {
        emit signalGoToParent();
        event->accept();
        return;
    }

    if ((mTool == nullptr) || (mTool->mouseDoubleClick(event) == false))
    {
        QGraphicsScene::mouseDoubleClickEvent(event);
    }
}

void SMScene::keyPressEvent(QKeyEvent* event)
{
    if (hasInlineEditorFocus(*this))
    {
        // A canvas-owned key must never win while a proxy-backed inline editor is active:
        // the focused editor (rename/note) owns the entire key stream until it closes.
        QGraphicsScene::keyPressEvent(event);
        return;
    }

    if ((mTool != nullptr) && mTool->keyPress(event))
    {
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Escape:
        cancelActiveGesture();
        event->accept();
        return;

    case Qt::Key_F2:
        startRenameOfSelection();
        event->accept();
        return;

    case Qt::Key_Return:
    case Qt::Key_Enter:
    {
        const QList<SMStateItem*> selection{ selectedStateItems() };
        if (selection.size() == 1)
        {
            requestEnterSubmachine(selection.first()->getElementId());
            event->accept();
            return;
        }
        break;
    }

    case Qt::Key_Backspace:
        emit signalGoToParent();
        event->accept();
        return;

    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
    {
        const int dx = (event->key() == Qt::Key_Left) ? -1 : (event->key() == Qt::Key_Right) ? 1 : 0;
        const int dy = (event->key() == Qt::Key_Up)   ? -1 : (event->key() == Qt::Key_Down)  ? 1 : 0;
        const bool coarse = event->modifiers().testFlag(Qt::ControlModifier);
        const bool pixel  = event->modifiers().testFlag(Qt::ShiftModifier);
        // An active transition waypoint wins the arrow keys; otherwise nudge the box selection.
        if (nudgeSelectedEdgePoint(dx, dy, coarse, pixel) || nudgeSelection(dx, dy, pixel))
        {
            event->accept();
            return;
        }

        break;
    }

    default:
        break;
    }

    QGraphicsScene::keyPressEvent(event);
}

void SMScene::onSceneSelectionChanged()
{
    if (mSyncSelection)
    {
        return;
    }

    QList<uint32_t> selected;
    for (QGraphicsItem* item : selectedItems())
    {
        const SMCanvasItem* canvasItem = dynamic_cast<const SMCanvasItem*>(item);
        if (canvasItem != nullptr)
        {
            selected.append(canvasItem->getElementId());
        }
    }

    mSyncSelection = true;
    mModel.getSelectionModel().setSelection(selected);
    mSyncSelection = false;
    updateConnHighlights();
}

void SMScene::onModelSelectionChanged(const QList<uint32_t>& selected)
{
    if (mSyncSelection)
    {
        return;
    }

    mSyncSelection = true;
    clearSelection();
    for (uint32_t id : selected)
    {
        SMCanvasItem* item = findCanvasItem(id);
        if (item != nullptr)
        {
            item->setSelected(true);
        }
    }

    mSyncSelection = false;
    updateConnHighlights();
}

void SMScene::onElementAdded(uint32_t id, eDocElementKind kind)
{
    if (kind == eDocElementKind::State)
    {
        if (isOnThisLevel(id))
        {
            createStateItem(id);
        }
        else
        {
            refreshCompositeBoxes();    // a nested-level change affects a miniature here
        }
    }

    if (kind == eDocElementKind::Transition)
    {
        createEdgeItem(id);
        // A new internal transition adds a body row; refresh the owner box.
        refreshStateBodies();
    }

    if (kind == eDocElementKind::Note)
    {
        createNoteItem(id);     // free note only; owned notes update their owner's badge
        refreshNoteBadges();
    }

    if (kind == eDocElementKind::Operation)
    {
        // An operation is summarized in its owner state box (entry/exit) or on its transition
        // edge (transition operations) -- refresh both, the notifier does not say which owner.
        refreshStateBodies();
        refreshEdges();
    }

    if (kind == eDocElementKind::Method)
    {
        // A method's signature is rendered as a trigger stimulus on transition edges and, via the
        // operations that call it, summarized in state bodies. A parameter add emits the parameter's
        // id (not the method's) under the Method kind, so re-read both surfaces unconditionally.
        refreshEdges();
        refreshStateBodies();
    }

    if ((kind == eDocElementKind::State) || (kind == eDocElementKind::Transition))
    {
        updateConnHighlights();
    }
}

void SMScene::onElementRemoved(uint32_t id, eDocElementKind kind)
{
    if ((kind == eDocElementKind::State) || (kind == eDocElementKind::Transition) || (kind == eDocElementKind::Note))
    {
        // The destructor removes the item from the scene and the ID hash.
        SMCanvasItem* item = findCanvasItem(id);
        const bool foreign = (item == nullptr);
        delete item;
        if (kind == eDocElementKind::Transition)
        {
            refreshStateBodies();
        }
        else if ((kind == eDocElementKind::State) && foreign)
        {
            refreshCompositeBoxes();    // a nested-level change affects a miniature here
        }

        if (kind != eDocElementKind::Note)
        {
            updateConnHighlights();
        }
        else
        {
            refreshNoteBadges();    // an owned note left its state/transition
        }
    }
    else if (kind == eDocElementKind::Operation)
    {
        refreshStateBodies();       // an operation left its owner state box
        refreshEdges();             // or its transition edge summary
    }
    else if (kind == eDocElementKind::Method)
    {
        // A removed parameter shortens the trigger stimulus signature on edges and the called-method
        // summaries in state bodies; the notifier carries the parameter's id, so refresh both.
        refreshEdges();
        refreshStateBodies();
    }
}

void SMScene::onElementChanged(uint32_t id, eDocElementKind kind)
{
    if (kind == eDocElementKind::Transition)
    {
        const SMTransitionEntry* transition = mModel.getData().findTransitionById(id);
        SMEdgeItem* edge = edgeItem(id);
        if ((transition != nullptr) && transition->hasTarget())
        {
            if (edge != nullptr)
            {
                edge->updateFromModel();
            }
            else
            {
                createEdgeItem(id);     // gained a target
            }
        }
        else if (edge != nullptr)
        {
            delete edge;                // became internal, lost its target, or is gone
        }

        refreshStateBodies();
        updateConnHighlights();
        return;
    }

    if (kind == eDocElementKind::Operation)
    {
        // An operation has no item of its own: its owner state box shows its summary (entry/exit),
        // and a transition's operations read on its edge -- refresh both.
        refreshStateBodies();
        refreshEdges();
        return;
    }

    if (kind == eDocElementKind::Method)
    {
        // A method rename, or a parameter rename/retype/default change, alters the trigger stimulus
        // signature shown on edges and the called-method summaries in state bodies. The changed id is
        // the method's (never a canvas item's), so re-read both surfaces rather than a single item.
        refreshEdges();
        refreshStateBodies();
        return;
    }

    SMCanvasItem* item = findCanvasItem(id);
    if (item != nullptr)
    {
        item->updateFromModel();
    }
    else if (kind == eDocElementKind::Note)
    {
        // An owned note's text/color changed: it has no item of its own, so re-read the
        // owner items whose badge reflects it.
        refreshNoteBadges();
    }
}

void SMScene::onListReordered(uint32_t /*ownerId*/, eDocElementKind kind)
{
    if (kind == eDocElementKind::Transition)
    {
        // Transition IDs are position-keyed, so a reorder can reassign what each edge shows.
        for (SMCanvasItem* item : std::as_const(mItems))
        {
            SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(item);
            if (edge != nullptr)
            {
                edge->updateFromModel();
            }
        }

        refreshStateBodies();
        updateConnHighlights();
    }
    else if (kind == eDocElementKind::Operation)
    {
        refreshStateBodies();       // execution order changed; refresh the summarized rows
        refreshEdges();             // and the transition edge summary
    }
    else if (kind == eDocElementKind::Method)
    {
        // Reordering a method's parameters reorders the trigger stimulus signature on edges and the
        // called-method summaries in state bodies; refresh both.
        refreshEdges();
        refreshStateBodies();
    }
}

void SMScene::onNameChanged(uint32_t id, const QString& /*oldName*/, const QString& /*newName*/)
{
    SMCanvasItem* item = findCanvasItem(id);
    if (item != nullptr)
    {
        item->updateFromModel();
    }

    // An edge label resolves its target's name live, so only the edges touching the renamed
    // state can change. Refreshing all of them made a rename cost O(level size) -- 200 ms on a
    // 200-node level (SM-27 responsiveness gate).
    for (SMCanvasItem* edgeItem : std::as_const(mItems))
    {
        SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(edgeItem);
        if ((edge != nullptr) && ((edge->getSourceId() == id) || (edge->getTargetId() == id)))
        {
            edge->updateFromModel();
        }
    }

    updateConnHighlights();
}

void SMScene::onLayoutChanged(const QList<uint32_t>& ownerIds)
{
    bool foreign{ false };
    for (uint32_t id : ownerIds)
    {
        SMCanvasItem* item = findCanvasItem(id);
        if (item != nullptr)
        {
            item->updateFromModel();
        }
        else
        {
            foreign = true;
        }

        // A state box move/resize re-anchors its connected edges. The geometry came from the
        // document, so the anchors are re-measured against the box it now describes.
        if (stateItem(id) != nullptr)
        {
            updateEdgesForState(id, true);
        }
    }

    if (foreign)
    {
        refreshCompositeBoxes();    // a nested node moved: the owner's miniature is stale
    }
}

QList<SMStateItem*> SMScene::selectedStateItems() const
{
    QList<SMStateItem*> result;
    for (QGraphicsItem* item : selectedItems())
    {
        SMStateItem* stateItem = dynamic_cast<SMStateItem*>(item);
        if (stateItem != nullptr)
        {
            result.append(stateItem);
        }
    }

    return result;
}

QList<SMEdgeItem*> SMScene::selectedEdgeItems() const
{
    QList<SMEdgeItem*> result;
    for (QGraphicsItem* item : selectedItems())
    {
        SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(item);
        if (edge != nullptr)
        {
            result.append(edge);
        }
    }

    return result;
}

QList<SMNoteItem*> SMScene::selectedNoteItems() const
{
    QList<SMNoteItem*> result;
    for (QGraphicsItem* item : selectedItems())
    {
        SMNoteItem* note = dynamic_cast<SMNoteItem*>(item);
        if (note != nullptr)
        {
            result.append(note);
        }
    }

    return result;
}

void SMScene::reconnectTransitionTarget(uint32_t transitionId, uint32_t targetStateId, const SMLayoutEdge& geometry)
{
    StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if (transition == nullptr)
    {
        return;
    }

    const SMStateEntry* target = (targetStateId != 0 ? data.findStateById(targetStateId) : nullptr);
    if ((target == nullptr) || (target->getId() == transition->getToId()))
    {
        return;
    }

    // A Start state is a source only (no incoming): never retarget a transition onto it. The edge
    // item already vetoes this drop with a hint; this is the backstop for any other caller.
    if (target->getKind() == SMStateEntry::eStateKind::Start)
    {
        return;
    }

    // One undo step: persist the drop geometry (the end anchor at the release position, the label
    // reset) and retarget the transition. Geometry first, so the retarget's edge refresh reads the
    // final anchor and there is no flash back to the old endpoint.
    const QString text = QCoreApplication::translate("SMScene", "Reconnect transition");
    SMCompositeCommand* command = new SMCompositeCommand(data, mModel.getNotifier(), text);
    new SMSetEdgeGeometryCommand(data, mModel.getNotifier(), transitionId, SMMoveNodeCommand::takeNextGesture(), geometry, text, command);
    new SMSetTransitionTargetCommand(data, mModel.getNotifier(), transitionId, target->getId(), text, command);
    mModel.getUndoStack().push(command);
}

void SMScene::reparentTransition(uint32_t transitionId, uint32_t newSourceStateId, const SMLayoutEdge& geometry)
{
    if (newSourceStateId == 0)
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    SMStateEntry* newSource = data.findStateById(newSourceStateId);
    SMStateEntry* oldSource = data.findTransitionOwner(transitionId);
    if ((newSource == nullptr) || (oldSource == nullptr) || (newSource == oldSource))
    {
        return;
    }

    // A Final state is a target only (no outgoing): never reparent a transition onto it. The edge
    // item already vetoes this drop with a hint; this is the backstop for any other caller.
    if (newSource->getKind() == SMStateEntry::eStateKind::Final)
    {
        return;
    }

    // Same backstop for the Start pseudo-state, in both directions: its transitions are the
    // level's initial ones, taken on entry and naming no stimulus, so an ordinary transition may
    // not be moved onto one and an initial one may not be moved off it.
    if (newSource->isPseudoStart() || oldSource->isPseudoStart())
    {
        return;
    }

    // One undo step: persist the drop geometry under the current (old) id first, then reparent --
    // the reparent captures that edge and re-keys it to the new source, so the begin anchor lands
    // at the release position, the label re-centres, and the edge never flashes back to its old
    // source before the command redraws it.
    const QString text = QCoreApplication::translate("SMScene", "Reconnect transition source");
    SMCompositeCommand* command = new SMCompositeCommand(data, mModel.getNotifier(), text);
    new SMSetEdgeGeometryCommand(data, mModel.getNotifier(), transitionId, SMMoveNodeCommand::takeNextGesture(), geometry, text, command);
    new SMReparentTransitionCommand(data, mModel.getNotifier(), *oldSource, *newSource, transitionId, text, command);
    mModel.getUndoStack().push(command);
}

void SMScene::startRenameOfSelection()
{
    const QList<SMStateItem*> selection{ selectedStateItems() };
    if (selection.size() == 1)
    {
        selection.first()->startInlineRename();
    }
}

bool SMScene::isInlineEditorActive() const
{
    return hasInlineEditorFocus(*this);
}

void SMScene::closeInlineEditors()
{
    // Copied: an item may commit through an undo command, which can rebuild the item map.
    const QList<SMCanvasItem*> items = mItems.values();
    for (SMCanvasItem* item : items)
    {
        if (item != nullptr)
        {
            item->finishInlineEdit();
        }
    }
}

void SMScene::requestEnterSubmachine(uint32_t stateId)
{
    const SMStateEntry* state = mModel.getData().findStateById(stateId);
    if ((state != nullptr) && state->hasNestedStates())
    {
        emit signalEnterSubmachine(stateId);
    }
}

void SMScene::requestSubstate(uint32_t stateId)
{
    // Not gated on hasNestedStates: a plain normal state gets a submachine created on the fly by
    // the Design page. Only guard that the state still exists.
    if (mModel.getData().findStateById(stateId) != nullptr)
    {
        emit signalRequestSubstate(stateId);
    }
}

void SMScene::requestGuardEdit(uint32_t transitionId)
{
    if ((transitionId != 0u) && (mModel.getData().findTransitionById(transitionId) != nullptr))
    {
        emit signalGuardEditRequested(transitionId);
    }
}

void SMScene::requestGotoDefinition(uint32_t elementId, bool isState, int scope)
{
    const bool exists = isState
                      ? (mModel.getData().findStateById(elementId) != nullptr)
                      : (mModel.getData().findTransitionById(elementId) != nullptr);
    if ((elementId != 0u) && exists)
    {
        emit signalGotoDefinitionRequested(elementId, isState, scope);
    }
}

void SMScene::requestGotoRefs(const QList<SMReferences::Ref>& refs)
{
    if (refs.isEmpty() == false)
    {
        emit signalGotoRefsRequested(refs);
    }
}

void SMScene::requestInternalEdit(uint32_t transitionId)
{
    if ((transitionId != 0u) && (mModel.getData().findTransitionById(transitionId) != nullptr))
    {
        emit signalInternalEditRequested(transitionId);
    }
}

void SMScene::showSubmachinePeek(uint32_t stateId, const QPoint& globalPos)
{
    const StateMachineData& data = mModel.getData();
    const SMStateEntry* state = data.findStateById(stateId);
    if ((state == nullptr) || (state->hasNestedStates() == false))
    {
        hideSubmachinePeek();
        return;
    }

    const QList<SMStateEntry*>& children = state->getNestedStates()->getElements();
    QList<SMSubmachinePeek::Shape> shapes;
    for (const SMStateEntry* child : children)
    {
        if (shapes.size() >= SMSubmachinePeek::MaxShapes)
        {
            break;      // a fixed-size view costs a fixed amount to build, too
        }

        const SMLayoutNode* node = (child != nullptr) ? data.getLayout().findNode(child->getId()) : nullptr;
        if (node != nullptr)
        {
            shapes.append({ QRectF(node->x, node->y, node->width, node->height), child->getKind() });
        }
    }

    if (shapes.isEmpty())
    {
        hideSubmachinePeek();   // a level whose nodes have no layout yet has no silhouette to show
        return;
    }

    if (mPeek == nullptr)
    {
        // Parented to the view, not to the scene: the popup is a widget, and it must die with the
        // window rather than outlive the level it belongs to.
        const QList<QGraphicsView*> canvasViews = views();
        mPeek = new SMSubmachinePeek(canvasViews.isEmpty() ? nullptr : canvasViews.first());
    }

    mPeek->showFor(state->getName(), shapes, static_cast<int>(children.size()), globalPos);
}

void SMScene::hideSubmachinePeek()
{
    if (mPeek != nullptr)
    {
        mPeek->hide();
    }
}

void SMScene::populateFromModel()
{
    const SMStateData* level = mModel.getData().findLevel(mLevelId);
    if (level != nullptr)
    {
        for (const SMStateEntry* state : level->getElements())
        {
            createStateItem(state->getId());
        }

        // Edges after all boxes exist, so both endpoints resolve to a box.
        for (const SMStateEntry* state : level->getElements())
        {
            for (const SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if (transition->hasTarget())
                {
                    createEdgeItem(transition->getId());
                }
            }
        }
    }

    for (const SMLayoutNote& note : mModel.getData().getLayout().getNotes())
    {
        if (note.level == mLevelId)
        {
            createNoteItem(note.id);
        }
    }
}

void SMScene::createStateItem(uint32_t stateId)
{
    if (findCanvasItem(stateId) == nullptr)
    {
        SMStateItem* item = new SMStateItem(stateId);
        addItem(item);
        item->updateFromModel();
    }
}

void SMScene::createEdgeItem(uint32_t transitionId)
{
    if (findCanvasItem(transitionId) != nullptr)
    {
        return;
    }

    const SMTransitionEntry* transition = mModel.getData().findTransitionById(transitionId);
    const SMStateEntry* owner = mModel.getData().findTransitionOwner(transitionId);
    if ((transition == nullptr) || (transition->hasTarget() == false) || (owner == nullptr) || (isOnThisLevel(owner->getId()) == false))
    {
        return;     // no target, no edge: it is shown as a state-body row instead
    }

    SMEdgeItem* item = new SMEdgeItem(transitionId);
    addItem(item);
    item->updateFromModel();
}

SMEdgeItem* SMScene::edgeItem(uint32_t transitionId) const
{
    return dynamic_cast<SMEdgeItem*>(findCanvasItem(transitionId));
}

void SMScene::createNoteItem(uint32_t noteId)
{
    if (findCanvasItem(noteId) != nullptr)
    {
        return;
    }

    const SMLayoutNote* note = mModel.getData().getLayout().findNote(noteId);
    // Owned notes are drawn as a badge on their state/transition, not as a free canvas box.
    if ((note == nullptr) || (note->level != mLevelId) || (note->owner != 0))
    {
        return;
    }

    SMNoteItem* item = new SMNoteItem(noteId);
    addItem(item);
    item->updateFromModel();
}

void SMScene::refreshNoteBadges()
{
    // A note bound to a state/transition owner is painted by that owner as a badge, so any
    // note add/remove/change must re-read the owner items (their own ID never names it).
    for (SMCanvasItem* item : std::as_const(mItems))
    {
        if ((dynamic_cast<SMStateItem*>(item) != nullptr) || (dynamic_cast<SMEdgeItem*>(item) != nullptr))
        {
            item->updateFromModel();
        }
    }
}

SMNoteItem* SMScene::noteItem(uint32_t noteId) const
{
    return dynamic_cast<SMNoteItem*>(findCanvasItem(noteId));
}

SMStateItem* SMScene::stateItem(uint32_t stateId) const
{
    return dynamic_cast<SMStateItem*>(findCanvasItem(stateId));
}

SMStateItem* SMScene::stateAt(const QPointF& scenePos) const
{
    const QList<QGraphicsItem*> hit = items(scenePos);
    for (QGraphicsItem* item : hit)
    {
        SMStateItem* state = dynamic_cast<SMStateItem*>(item);
        if (state != nullptr)
        {
            return state;
        }
    }

    return nullptr;
}

SMStateItem* SMScene::stateNear(const QPointF& scenePos, double margin) const
{
    if (SMStateItem* exact = stateAt(scenePos))
    {
        return exact;
    }

    // Just outside a border still counts. Nearest box wins, so overlapping margins are not a
    // coin toss; a box that contains the point outright already returned above.
    SMStateItem* best = nullptr;
    double bestDistance = margin;
    for (SMCanvasItem* item : std::as_const(mItems))
    {
        SMStateItem* state = dynamic_cast<SMStateItem*>(item);
        if (state == nullptr)
        {
            continue;
        }

        const QRectF box = state->getBoxGeometry();
        const double dx = std::max({ box.left() - scenePos.x(), 0.0, scenePos.x() - box.right() });
        const double dy = std::max({ box.top() - scenePos.y(), 0.0, scenePos.y() - box.bottom() });
        const double distance = std::hypot(dx, dy);
        if (distance <= bestDistance)
        {
            bestDistance = distance;
            best = state;
        }
    }

    return best;
}

void SMScene::updateEdgesForState(uint32_t stateId, bool fromModel /*= false*/)
{
    for (SMCanvasItem* item : std::as_const(mItems))
    {
        SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(item);
        if ((edge != nullptr) && ((edge->getSourceId() == stateId) || (edge->getTargetId() == stateId)))
        {
            edge->refreshAnchors(fromModel);
        }
    }
}

void SMScene::refreshStateBodies()
{
    for (SMCanvasItem* item : std::as_const(mItems))
    {
        SMStateItem* state = dynamic_cast<SMStateItem*>(item);
        if (state != nullptr)
        {
            state->updateFromModel();
        }
    }
}

void SMScene::refreshEdges()
{
    for (SMCanvasItem* item : std::as_const(mItems))
    {
        SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(item);
        if (edge != nullptr)
        {
            edge->updateFromModel();
        }
    }
}

void SMScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    // Edge lines paint under the state boxes (z = -1) so borders stay clean, but their labels
    // must stay readable even where they overlap a box; paint them here, above every item.
    const QPalette palette{ (views().isEmpty() == false) ? views().first()->palette() : QPalette() };
    for (SMCanvasItem* item : std::as_const(mItems))
    {
        SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(item);
        if ((edge != nullptr) && rect.intersects(edge->labelBounds()))
        {
            edge->paintLabels(painter, palette);
        }
    }

    QGraphicsScene::drawForeground(painter, rect);
}

void SMScene::refreshCompositeBoxes()
{
    for (SMCanvasItem* item : std::as_const(mItems))
    {
        SMStateItem* box = dynamic_cast<SMStateItem*>(item);
        if (box == nullptr)
        {
            continue;
        }

        const SMStateEntry* state = mModel.getData().findStateById(box->getElementId());
        if ((state != nullptr) && state->hasNestedStates())
        {
            box->updateFromModel();
        }
    }
}

bool SMScene::isOnThisLevel(uint32_t stateId) const
{
    const SMStateData* level = mModel.getData().findLevel(mLevelId);
    return (level != nullptr) && (level->findStateById(stateId) != nullptr);
}

void SMScene::updateConnHighlights()
{
    // Selected states: their IDs key the incoming side, their transitions the outgoing.
    QSet<uint32_t> selectedIds;
    QSet<uint32_t> outgoing;
    for (QGraphicsItem* item : selectedItems())
    {
        const SMStateItem* stateItem = dynamic_cast<const SMStateItem*>(item);
        if (stateItem == nullptr)
        {
            continue;
        }

        const SMStateEntry* state = mModel.getData().findStateById(stateItem->getElementId());
        if (state == nullptr)
        {
            continue;
        }

        selectedIds.insert(state->getId());
        for (const SMTransitionEntry* transition : state->getTransitions().getElements())
        {
            if (transition->hasTarget())
            {
                outgoing.insert(transition->getId());
            }
        }
    }

    QSet<uint32_t> incoming;
    const SMStateData* level = mModel.getData().findLevel(mLevelId);
    if ((level != nullptr) && (selectedIds.isEmpty() == false))
    {
        for (const SMStateEntry* state : level->getElements())
        {
            for (const SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if (transition->hasTarget() && selectedIds.contains(transition->getToId()))
                {
                    incoming.insert(transition->getId());
                }
            }
        }
    }

    for (SMCanvasItem* item : std::as_const(mItems))
    {
        const uint32_t id  = item->getElementId();
        const bool     out = outgoing.contains(id);
        const bool     in  = incoming.contains(id);
        item->setConnHighlight(  out && in ? SMCanvasItem::eConnHighlight::Both
                               : out       ? SMCanvasItem::eConnHighlight::Outgoing
                               : in        ? SMCanvasItem::eConnHighlight::Incoming
                                           : SMCanvasItem::eConnHighlight::None);
    }
}

bool SMScene::nudgeSelectedEdgePoint(int dx, int dy, bool coarse, bool pixel)
{
    const QList<SMEdgeItem*> edges{ selectedEdgeItems() };
    if (edges.size() != 1)
    {
        return false;
    }

    // The arrow keys move whichever of the selected edge's parts is active: an interior waypoint,
    // the repositioned label block, or a grabbed begin/end endpoint (issue #532).
    SMEdgeItem* edge = edges.first();
    if (edge->hasSelectedPoint())
    {
        return edge->nudgeSelectedPoint(dx, dy, coarse, pixel);
    }
    else if (edge->hasActiveLabel())
    {
        return edge->nudgeLabel(dx, dy, coarse, pixel);
    }
    else if (edge->hasActiveEnd())
    {
        return edge->nudgeActiveEnd(dx, dy, coarse, pixel);
    }

    return false;
}

bool SMScene::nudgeSelection(int dx, int dy, bool pixelWise)
{
    const QList<QGraphicsItem*> selection{ selectedItems() };
    if (selection.isEmpty())
    {
        return false;
    }

    const qreal step = pixelWise ? 1.0 : static_cast<qreal>(mGridSize);
    const QPointF delta{ step * dx, step * dy };
    if ((delta.x() == 0.0) && (delta.y() == 0.0))
    {
        return false;
    }

    // Every box travels the identical step, and the transitions between them are carried along by
    // their own boxes; a transition is nudged in its own right only when neither of the states it
    // connects is moving, otherwise it would take the step twice (issue #550 bugs 1 and 2).
    QSet<uint32_t>     movingStates;
    QList<SMEdgeItem*> edges;
    QList<QGraphicsItem*> boxes;
    for (QGraphicsItem* item : selection)
    {
        if (SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(item))
        {
            edges.append(edge);
            continue;
        }

        if (item->flags().testFlag(QGraphicsItem::ItemIsMovable) == false)
        {
            continue;
        }

        // A selected ancestor already carries the child along.
        QGraphicsItem* parent = item->parentItem();
        bool moved{ false };
        while (parent != nullptr)
        {
            if (parent->isSelected())
            {
                moved = true;
                break;
            }

            parent = parent->parentItem();
        }

        if (moved)
        {
            continue;
        }

        boxes.append(item);
        if (const SMStateItem* stateItem = dynamic_cast<const SMStateItem*>(item))
        {
            movingStates.insert(stateItem->getElementId());
        }
    }

    for (QGraphicsItem* item : std::as_const(boxes))
    {
        item->moveBy(delta.x(), delta.y());
    }

    for (SMEdgeItem* edge : std::as_const(edges))
    {
        if ((movingStates.contains(edge->getSourceId()) == false)
            && (movingStates.contains(edge->getTargetId()) == false))
        {
            edge->nudgeGeometry(delta);
        }
    }

    // One undo step per key press: the shared commit writes back the moved boxes together with
    // every transition whose geometry the step changed.
    commitSelectionMove(QCoreApplication::translate("SMScene", "Move selection"));
    return true;
}

void SMScene::commitSelectionMove(const QString& text)
{
    SMLayoutData& layout = mModel.getData().getLayout();

    // Every selected state box / note whose item position differs from its layout entry.
    QList<SMStateItem*> movedStates;
    for (SMStateItem* item : selectedStateItems())
    {
        const SMLayoutNode* node = layout.findNode(item->getElementId());
        if ((node != nullptr) && (QPointF(node->x, node->y) != item->pos()))
        {
            movedStates.append(item);
        }
    }

    QList<SMNoteItem*> movedNotes;
    for (SMNoteItem* item : selectedNoteItems())
    {
        const SMLayoutNote* note = layout.findNote(item->getElementId());
        if ((note != nullptr) && (QPointF(note->x, note->y) != item->pos()))
        {
            movedNotes.append(item);
        }
    }

    // A box that moved took the anchors of its transitions with it, and a group move took their
    // waypoints too; those points are stored in scene coordinates, so they are written back in the
    // very same undo step -- otherwise the file would keep anchors of boxes that are no longer
    // there, and re-reading it would snap the transitions to whichever border came nearest.
    const QList<SMEdgeItem*> movedEdges{ driftedEdgeItems() };

    if (movedStates.isEmpty() && movedNotes.isEmpty() && movedEdges.isEmpty())
    {
        return;
    }

    const uint32_t gesture = SMMoveNodeCommand::takeNextGesture();
    const bool     single  = ((movedStates.size() + movedNotes.size() + movedEdges.size()) == 1);
    QUndoCommand*  parent  = single ? nullptr : new SMCompositeCommand(mModel.getData(), mModel.getNotifier(), text);

    for (SMStateItem* item : movedStates)
    {
        const QRectF geometry = item->getBoxGeometry();
        QUndoCommand* command = new SMMoveNodeCommand(  mModel.getData(), mModel.getNotifier()
                                                      , item->getElementId(), gesture
                                                      , geometry.x(), geometry.y(), geometry.width(), geometry.height()
                                                      , text, parent);
        if (single)
        {
            mModel.getUndoStack().push(command);
        }
    }

    for (SMNoteItem* item : movedNotes)
    {
        const QRectF geometry = item->getBoxGeometry();
        QUndoCommand* command = new SMMoveNoteCommand(  mModel.getData(), mModel.getNotifier()
                                                      , item->getElementId(), gesture
                                                      , geometry.x(), geometry.y(), geometry.width(), geometry.height()
                                                      , text, parent);
        if (single)
        {
            mModel.getUndoStack().push(command);
        }
    }

    for (SMEdgeItem* item : movedEdges)
    {
        QUndoCommand* command = new SMSetEdgeGeometryCommand(  mModel.getData(), mModel.getNotifier()
                                                             , item->getElementId(), gesture
                                                             , item->buildGeometry(), text, parent);
        if (single)
        {
            mModel.getUndoStack().push(command);
        }
    }

    if (single == false)
    {
        mModel.getUndoStack().push(parent);
    }
}

QList<SMEdgeItem*> SMScene::driftedEdgeItems() const
{
    QList<SMEdgeItem*> result;
    for (SMCanvasItem* item : std::as_const(mItems))
    {
        SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(item);
        if ((edge != nullptr) && edge->hasGeometryDrift())
        {
            result.append(edge);
        }
    }

    return result;
}
