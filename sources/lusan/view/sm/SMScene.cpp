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
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMCanvasItem.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QCoreApplication>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPainter>
#include <QPalette>
#include <QSet>
#include <QVarLengthArray>

#include <algorithm>
#include <cmath>

SMScene::SMScene(StateMachineModel& model, uint32_t levelId, QObject* parent /*= nullptr*/)
    : QGraphicsScene(parent)
    , mModel        (model)
    , mLevelId      (levelId)
    , mItems        ( )
    , mTool         (createCanvasTool(NESMDesign::eCanvasTool::Select, *this))
    , mToolSticky   (false)
    , mGridSize     (NESMDesign::GridSizeDefault)
    , mGridVisible  (true)
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
    connect(&notifier, &DocModelNotifier::nameChanged, this, &SMScene::onNameChanged);
    connect(&notifier, &DocModelNotifier::layoutChanged, this, &SMScene::onLayoutChanged);

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
    if ((mTool == nullptr) || (mTool->mouseDoubleClick(event) == false))
    {
        QGraphicsScene::mouseDoubleClickEvent(event);
    }
}

void SMScene::keyPressEvent(QKeyEvent* event)
{
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

    case Qt::Key_Left:
        if (nudgeSelection(-1, 0, event->modifiers().testFlag(Qt::ShiftModifier)))
        {
            event->accept();
            return;
        }
        break;

    case Qt::Key_Right:
        if (nudgeSelection(1, 0, event->modifiers().testFlag(Qt::ShiftModifier)))
        {
            event->accept();
            return;
        }
        break;

    case Qt::Key_Up:
        if (nudgeSelection(0, -1, event->modifiers().testFlag(Qt::ShiftModifier)))
        {
            event->accept();
            return;
        }
        break;

    case Qt::Key_Down:
        if (nudgeSelection(0, 1, event->modifiers().testFlag(Qt::ShiftModifier)))
        {
            event->accept();
            return;
        }
        break;

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
    if ((kind == eDocElementKind::State) && isOnThisLevel(id))
    {
        createStateItem(id);
    }

    if ((kind == eDocElementKind::State) || (kind == eDocElementKind::Transition))
    {
        updateConnHighlights();
    }
}

void SMScene::onElementRemoved(uint32_t id, eDocElementKind kind)
{
    if ((kind == eDocElementKind::State) || (kind == eDocElementKind::Transition))
    {
        // The destructor removes the item from the scene and the ID hash.
        delete findCanvasItem(id);
        updateConnHighlights();
    }
}

void SMScene::onElementChanged(uint32_t id, eDocElementKind /*kind*/)
{
    SMCanvasItem* item = findCanvasItem(id);
    if (item != nullptr)
    {
        item->updateFromModel();
    }
}

void SMScene::onNameChanged(uint32_t id, const QString& /*oldName*/, const QString& /*newName*/)
{
    SMCanvasItem* item = findCanvasItem(id);
    if (item != nullptr)
    {
        item->updateFromModel();
    }

    // Transition targets reference states by name; the connection map may shift.
    updateConnHighlights();
}

void SMScene::onLayoutChanged(const QList<uint32_t>& ownerIds)
{
    for (uint32_t id : ownerIds)
    {
        SMCanvasItem* item = findCanvasItem(id);
        if (item != nullptr)
        {
            item->updateFromModel();
        }
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

void SMScene::startRenameOfSelection()
{
    const QList<SMStateItem*> selection{ selectedStateItems() };
    if (selection.size() == 1)
    {
        selection.first()->startInlineRename();
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
        if ((stateItem != nullptr) && (mModel.getData().getLayout().findNode(stateItem->getElementId()) != nullptr))
        {
            states.append(stateItem);
        }
        else
        {
            item->moveBy(delta.x(), delta.y());
        }
    }

    if (states.isEmpty() == false)
    {
        // One undo step per key press; a fresh gesture ID keeps presses separate.
        const uint32_t gesture = SMMoveNodeCommand::takeNextGesture();
        const QString  text    = QCoreApplication::translate("SMScene", "Move state", nullptr, states.size());
        if (states.size() == 1)
        {
            const QRectF geometry = states.first()->getBoxGeometry().translated(delta);
            mModel.getUndoStack().push(new SMMoveNodeCommand(  mModel.getData(), mModel.getNotifier()
                                                             , states.first()->getElementId(), gesture
                                                             , geometry.x(), geometry.y(), geometry.width(), geometry.height()
                                                             , text));
        }
        else
        {
            SMCompositeCommand* composite = new SMCompositeCommand(mModel.getData(), mModel.getNotifier(), text);
            for (SMStateItem* stateItem : states)
            {
                const QRectF geometry = stateItem->getBoxGeometry().translated(delta);
                new SMMoveNodeCommand(  mModel.getData(), mModel.getNotifier()
                                      , stateItem->getElementId(), gesture
                                      , geometry.x(), geometry.y(), geometry.width(), geometry.height()
                                      , text, composite);
            }

            mModel.getUndoStack().push(composite);
        }
    }

    return true;
}
