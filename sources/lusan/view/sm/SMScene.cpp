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
    , mGridSize     (NESMDesign::GridSizeDefault)
    , mGridVisible  (true)
    , mGridStyle    (NESMDesign::eGridStyle::Lines)
    , mGridDotSize  (NESMDesign::GridDotSizeDefault)
    , mSnapToGrid   (true)
    , mMouseDrag    (false)
    , mSyncSelection(false)
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
    if ((mToolSticky == false) && (getActiveTool() != NESMDesign::eCanvasTool::Select))
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
    if (event->button() == Qt::LeftButton)
    {
        mMouseDrag = true;

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
    if ((mTool == nullptr) || (mTool->mouseMove(event) == false))
    {
        QGraphicsScene::mouseMoveEvent(event);
    }
}

void SMScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    const bool handled = (mTool != nullptr) && mTool->mouseRelease(event);
    if (handled == false)
    {
        QGraphicsScene::mouseReleaseEvent(event);
    }

    if (event->button() == Qt::LeftButton)
    {
        mMouseDrag = false;
    }
}

void SMScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
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
}

void SMScene::onElementChanged(uint32_t id, eDocElementKind kind)
{
    if (kind == eDocElementKind::Transition)
    {
        const SMTransitionEntry* transition = mModel.getData().findTransitionById(id);
        SMEdgeItem* edge = edgeItem(id);
        if ((transition != nullptr) && transition->isExternal())
        {
            if (edge != nullptr)
            {
                edge->updateFromModel();
            }
            else
            {
                createEdgeItem(id);     // became external
            }
        }
        else if (edge != nullptr)
        {
            delete edge;                // became internal (or gone)
        }

        refreshStateBodies();
        updateConnHighlights();
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
}

void SMScene::onNameChanged(uint32_t id, const QString& /*oldName*/, const QString& /*newName*/)
{
    SMCanvasItem* item = findCanvasItem(id);
    if (item != nullptr)
    {
        item->updateFromModel();
    }

    // Transition targets reference states by name; refresh edges and the connection map.
    for (SMCanvasItem* edgeItem : std::as_const(mItems))
    {
        SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(edgeItem);
        if (edge != nullptr)
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

        // A state box move/resize re-anchors its connected edges.
        if (stateItem(id) != nullptr)
        {
            updateEdgesForState(id);
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

void SMScene::reconnectTransitionTarget(uint32_t transitionId, uint32_t targetStateId)
{
    StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if (transition == nullptr)
    {
        return;
    }

    const SMStateEntry* target = (targetStateId != 0 ? data.findStateById(targetStateId) : nullptr);
    if ((target == nullptr) || (target->getName() == transition->getTo()))
    {
        return;
    }

    mModel.getUndoStack().push(new SMSetTransitionTargetCommand(  data, mModel.getNotifier()
                                                               , transitionId, target->getName()
                                                               , QCoreApplication::translate("SMScene", "Reconnect transition")));
}

void SMScene::reparentTransition(uint32_t transitionId, uint32_t newSourceStateId)
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

    mModel.getUndoStack().push(new SMReparentTransitionCommand(  data, mModel.getNotifier()
                                                              , *oldSource, *newSource, transitionId
                                                              , QCoreApplication::translate("SMScene", "Reconnect transition source")));
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

void SMScene::requestEnterSubmachine(uint32_t stateId)
{
    const SMStateEntry* state = mModel.getData().findStateById(stateId);
    if ((state != nullptr) && state->hasNestedStates())
    {
        emit signalEnterSubmachine(stateId);
    }
}

void SMScene::requestGuardEdit(uint32_t transitionId)
{
    if ((transitionId != 0u) && (mModel.getData().findTransitionById(transitionId) != nullptr))
    {
        emit signalGuardEditRequested(transitionId);
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
                if (transition->isExternal())
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
    if ((transition == nullptr) || (transition->isExternal() == false) || (owner == nullptr) || (isOnThisLevel(owner->getId()) == false))
    {
        return;     // internal transitions are shown as a state-body row, not an edge
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

void SMScene::updateEdgesForState(uint32_t stateId)
{
    for (SMCanvasItem* item : std::as_const(mItems))
    {
        SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(item);
        if ((edge != nullptr) && ((edge->getSourceId() == stateId) || (edge->getTargetId() == stateId)))
        {
            edge->refreshAnchors();
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
    // Selected states: their names key the incoming side, their transitions the outgoing.
    QSet<QString>  selectedNames;
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

        selectedNames.insert(state->getName());
        for (const SMTransitionEntry* transition : state->getTransitions().getElements())
        {
            if (transition->isExternal())
            {
                outgoing.insert(transition->getId());
            }
        }
    }

    QSet<uint32_t> incoming;
    const SMStateData* level = mModel.getData().findLevel(mLevelId);
    if ((level != nullptr) && (selectedNames.isEmpty() == false))
    {
        for (const SMStateEntry* state : level->getElements())
        {
            for (const SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if (transition->isExternal() && selectedNames.contains(transition->getTo()))
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
    if ((edges.size() != 1) || (edges.first()->hasSelectedPoint() == false))
    {
        return false;
    }

    return edges.first()->nudgeSelectedPoint(dx, dy, coarse, pixel);
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

    QList<SMStateItem*> states;
    QList<SMNoteItem*>  notes;
    for (QGraphicsItem* item : selection)
    {
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

        SMStateItem* stateItem = dynamic_cast<SMStateItem*>(item);
        SMNoteItem*  notePick  = dynamic_cast<SMNoteItem*>(item);
        if ((stateItem != nullptr) && (mModel.getData().getLayout().findNode(stateItem->getElementId()) != nullptr))
        {
            states.append(stateItem);
        }
        else if ((notePick != nullptr) && (mModel.getData().getLayout().findNote(notePick->getElementId()) != nullptr))
        {
            notes.append(notePick);
        }
        else
        {
            item->moveBy(delta.x(), delta.y());
        }
    }

    if ((states.isEmpty() == false) || (notes.isEmpty() == false))
    {
        // One undo step per key press; a fresh gesture ID keeps presses separate.
        const uint32_t gesture = SMMoveNodeCommand::takeNextGesture();
        const QString  text    = QCoreApplication::translate("SMScene", "Move selection");
        const bool     single  = ((states.size() + notes.size()) == 1);
        QUndoCommand*  parent  = single ? nullptr : new SMCompositeCommand(mModel.getData(), mModel.getNotifier(), text);

        for (SMStateItem* stateItem : states)
        {
            const QRectF geometry = stateItem->getBoxGeometry().translated(delta);
            QUndoCommand* command = new SMMoveNodeCommand(  mModel.getData(), mModel.getNotifier()
                                                          , stateItem->getElementId(), gesture
                                                          , geometry.x(), geometry.y(), geometry.width(), geometry.height()
                                                          , text, parent);
            if (single)
            {
                mModel.getUndoStack().push(command);
            }
        }

        for (SMNoteItem* noteItem : notes)
        {
            const QRectF geometry = noteItem->getBoxGeometry().translated(delta);
            QUndoCommand* command = new SMMoveNoteCommand(  mModel.getData(), mModel.getNotifier()
                                                          , noteItem->getElementId(), gesture
                                                          , geometry.x(), geometry.y(), geometry.width(), geometry.height()
                                                          , text, parent);
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

    if (movedStates.isEmpty() && movedNotes.isEmpty())
    {
        return;
    }

    const uint32_t gesture = SMMoveNodeCommand::takeNextGesture();
    const bool     single  = ((movedStates.size() + movedNotes.size()) == 1);
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

    if (single == false)
    {
        mModel.getUndoStack().push(parent);
    }
}
