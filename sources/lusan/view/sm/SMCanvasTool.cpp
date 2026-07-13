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
 *  \file        lusan/view/sm/SMCanvasTool.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas tool strategies.
 *
 ************************************************************************/

#include "lusan/view/sm/SMCanvasTool.hpp"

#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMNoteItem.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QCoreApplication>
#include <QCursor>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPainterPath>
#include <QPen>
#include <QToolTip>

#include <algorithm>
#include <cmath>

namespace
{
    //!< The pointer travel that promotes a press into a drag gesture (scene units).
    constexpr double DragThreshold { 4.0 };

    inline double toolDistance(const QPointF& a, const QPointF& b)
    {
        return std::hypot(a.x() - b.x(), a.y() - b.y());
    }
}

SMCanvasTool::SMCanvasTool(SMScene& scene)
    : mScene(scene)
{
}

void SMCanvasTool::activate()
{
}

void SMCanvasTool::cancelGesture()
{
}

bool SMCanvasTool::mousePress(QGraphicsSceneMouseEvent* /*event*/)
{
    return false;
}

bool SMCanvasTool::mouseMove(QGraphicsSceneMouseEvent* /*event*/)
{
    return false;
}

bool SMCanvasTool::mouseRelease(QGraphicsSceneMouseEvent* /*event*/)
{
    return false;
}

bool SMCanvasTool::mouseDoubleClick(QGraphicsSceneMouseEvent* /*event*/)
{
    return false;
}

bool SMCanvasTool::keyPress(QKeyEvent* /*event*/)
{
    return false;
}

SMSelectTool::SMSelectTool(SMScene& scene)
    : SMCanvasTool(scene)
{
}

NESMDesign::eCanvasTool SMSelectTool::getKind() const
{
    return NESMDesign::eCanvasTool::Select;
}

SMPlaceStateTool::SMPlaceStateTool(SMScene& scene, bool finalState)
    : SMCanvasTool  (scene)
    , mFinal        (finalState)
    , mPressed      (false)
    , mDragging     (false)
    , mPressPos     ( )
    , mPreview      (nullptr)
{
}

SMPlaceStateTool::~SMPlaceStateTool()
{
    clearPreview();
}

NESMDesign::eCanvasTool SMPlaceStateTool::getKind() const
{
    return (mFinal ? NESMDesign::eCanvasTool::AddFinalState : NESMDesign::eCanvasTool::AddState);
}

void SMPlaceStateTool::cancelGesture()
{
    clearPreview();
    mPressed  = false;
    mDragging = false;
}

void SMPlaceStateTool::createPreview()
{
    if (mPreview == nullptr)
    {
        mPreview = new QGraphicsRectItem();
        QPen pen{ NESMDesign::selectionColor(QPalette()), 1.0 };
        pen.setStyle(Qt::DashLine);
        pen.setCosmetic(true);
        mPreview->setPen(pen);
        mPreview->setBrush(Qt::NoBrush);
        mPreview->setZValue(1000.0);
        getScene().addItem(mPreview);
    }
}

void SMPlaceStateTool::clearPreview()
{
    if (mPreview != nullptr)
    {
        getScene().removeItem(mPreview);
        delete mPreview;
        mPreview = nullptr;
    }
}

QRectF SMPlaceStateTool::dragRect(const QPointF& cursor) const
{
    SMScene& canvas = getScene();
    QRectF rect{ QRectF(canvas.snappedPosition(mPressPos), canvas.snappedPosition(cursor)).normalized() };
    rect.setWidth(std::max(rect.width(), NESMDesign::StateMinWidth));
    rect.setHeight(std::max(rect.height(), NESMDesign::StateMinHeight));
    return rect;
}

bool SMPlaceStateTool::mousePress(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        return false;
    }

    mPressed  = true;
    mDragging = false;
    mPressPos = event->scenePos();
    event->accept();
    return true;
}

bool SMPlaceStateTool::mouseMove(QGraphicsSceneMouseEvent* event)
{
    if (mPressed == false)
    {
        return false;
    }

    if ((mDragging == false) && (toolDistance(event->scenePos(), mPressPos) > DragThreshold))
    {
        mDragging = true;
        createPreview();
    }

    if (mDragging)
    {
        mPreview->setRect(dragRect(event->scenePos()));
    }

    event->accept();
    return true;
}

bool SMPlaceStateTool::mouseRelease(QGraphicsSceneMouseEvent* event)
{
    if ((mPressed == false) || (event->button() != Qt::LeftButton))
    {
        return false;
    }

    const bool    dragged = mDragging;
    const QPointF cursor  = event->scenePos();
    mPressed  = false;
    mDragging = false;
    clearPreview();

    if (dragged)
    {
        // The user drew the box: press-to-release rectangle, minimum size enforced.
        placeState(dragRect(cursor));
    }
    else
    {
        // A plain click: a default-sized box centered on the click position.
        const QSizeF size = mFinal ? QSizeF(NESMDesign::MarkerStateWidth, NESMDesign::MarkerStateHeight)
                                   : QSizeF(NESMDesign::StateDefaultWidth, NESMDesign::StateDefaultHeight);
        const QPointF topLeft = getScene().snappedPosition(cursor - QPointF(size.width() / 2.0, size.height() / 2.0));
        placeState(QRectF(topLeft, size));
    }

    event->accept();
    return true;
}

bool SMPlaceStateTool::mouseDoubleClick(QGraphicsSceneMouseEvent* event)
{
    // The first click of the pair already placed a state; swallow the repeat.
    event->accept();
    return true;
}

void SMPlaceStateTool::placeState(const QRectF& box)
{
    SMScene& canvas = getScene();
    StateMachineModel& model = canvas.getModel();
    StateMachineData&  data  = model.getData();
    SMStateData* level = data.findLevel(canvas.getLevelId());
    if (level == nullptr)
    {
        return;
    }

    // Final states read as terminals, so they are named plainly "Final" (then Final2,
    // Final3, ...), not "NewFinalN" (issue #514).
    const QString base = (mFinal ? QStringLiteral("Final") : QStringLiteral("NewState"));
    QString name{ base };
    for (int i = 2; data.findState(name) != nullptr; ++i)
    {
        name = base + QString::number(i);
    }

    const SMStateEntry::eStateKind kind = (mFinal ? SMStateEntry::eStateKind::Final : SMStateEntry::eStateKind::Normal);
    SMCreateStateCommand* command = new SMCreateStateCommand(  data, model.getNotifier(), *level, name, kind
                                                             , box
                                                             , QCoreApplication::translate("SMCanvasTool", "Add state %1").arg(name));
    model.getUndoStack().push(command);

    const uint32_t stateId = command->getStateId();
    model.getSelectionModel().setSelection(QList<uint32_t>{ stateId });

    SMStateItem* item = dynamic_cast<SMStateItem*>(canvas.findCanvasItem(stateId));

    // May retire this tool (single-shot); only stack locals are used afterwards.
    canvas.finishToolGesture();
    if (item != nullptr)
    {
        item->startInlineRename();
    }
}

//////////////////////////////////////////////////////////////////////////
// SMTransitionTool
//////////////////////////////////////////////////////////////////////////

SMTransitionTool::SMTransitionTool(SMScene& scene)
    : SMCanvasTool  (scene)
    , mArmed        (false)
    , mButtonDown   (false)
    , mDragging     (false)
    , mFromBorder   (false)
    , mSourceId     (0)
    , mPressPos     ( )
    , mWaypoints    ( )
    , mPreview      (nullptr)
{
}

SMTransitionTool::~SMTransitionTool()
{
    clearPreview();
}

NESMDesign::eCanvasTool SMTransitionTool::getKind() const
{
    return NESMDesign::eCanvasTool::AddTransition;
}

void SMTransitionTool::activate()
{
    resetGesture();
}

void SMTransitionTool::cancelGesture()
{
    clearPreview();
    resetGesture();
}

void SMTransitionTool::resetGesture()
{
    mArmed      = false;
    mButtonDown = false;
    mDragging   = false;
    mFromBorder = false;
    mSourceId   = 0;
    mWaypoints.clear();
}

void SMTransitionTool::createPreview()
{
    if (mPreview == nullptr)
    {
        mPreview = new QGraphicsPathItem();
        QPen pen{ NESMDesign::selectionColor(QPalette()), NESMDesign::EdgeLineWidth };
        pen.setStyle(Qt::DashLine);
        pen.setCosmetic(true);
        mPreview->setPen(pen);
        mPreview->setZValue(1000.0);
        getScene().addItem(mPreview);
    }
}

void SMTransitionTool::clearPreview()
{
    if (mPreview != nullptr)
    {
        getScene().removeItem(mPreview);
        delete mPreview;
        mPreview = nullptr;
    }
}

QRectF SMTransitionTool::sourceRect() const
{
    SMStateItem* item = getScene().stateItem(mSourceId);
    if (item != nullptr)
    {
        return item->getBoxGeometry();
    }

    const SMLayoutNode* node = getScene().getModel().getData().getLayout().findNode(mSourceId);
    if (node != nullptr)
    {
        return QRectF(node->x, node->y, node->width, node->height);
    }

    return QRectF();
}

void SMTransitionTool::updatePreview(const QPointF& cursor)
{
    if (mPreview == nullptr)
    {
        return;
    }

    const QRectF src = sourceRect();
    if ((src.width() <= 0.0) || (src.height() <= 0.0))
    {
        return;
    }

    SMStateItem* sourceItem = getScene().stateItem(mSourceId);
    const double srcRadius = (sourceItem != nullptr ? sourceItem->boxCornerRadius() : NESMDesign::StateCornerRadius);
    const QPointF ref = (mWaypoints.isEmpty() ? cursor : mWaypoints.first());
    QPainterPath path;
    path.moveTo(NESMDesign::borderPoint(src, srcRadius, ref));
    for (const QPointF& wp : mWaypoints)
    {
        path.lineTo(wp);
    }

    path.lineTo(cursor);
    mPreview->setPath(path);
}

bool SMTransitionTool::mousePress(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        return false;
    }

    SMStateItem* state = getScene().stateAt(event->scenePos());
    mButtonDown = true;
    mPressPos   = event->scenePos();
    if (mArmed == false)
    {
        if (state == nullptr)
        {
            mButtonDown = false;
            return false;
        }

        mArmed    = true;
        mDragging = false;
        mSourceId = state->getElementId();
        mWaypoints.clear();
        createPreview();
        updatePreview(event->scenePos());
        event->accept();
        return true;
    }

    // Any follow-up press continues the gesture deliberately; the border-started
    // "plain click selects the state" fallback no longer applies.
    mFromBorder = false;
    if (state != nullptr)
    {
        completeExternal(state->getElementId());
    }
    else
    {
        // A click on empty canvas drops a polyline waypoint; the gesture continues.
        mWaypoints.append(getScene().snappedPosition(event->scenePos()));
        updatePreview(event->scenePos());
    }

    event->accept();
    return true;
}

bool SMTransitionTool::mouseMove(QGraphicsSceneMouseEvent* event)
{
    if (mArmed == false)
    {
        return false;
    }

    // Only a button held past the threshold is a drag; the scene delivers tracking moves
    // with the button up too, and those must never promote the gesture into a drag.
    if (mButtonDown && (mDragging == false) && (toolDistance(event->scenePos(), mPressPos) > DragThreshold))
    {
        mDragging = true;
    }

    updatePreview(event->scenePos());
    event->accept();
    return true;
}

bool SMTransitionTool::mouseRelease(QGraphicsSceneMouseEvent* event)
{
    if ((mArmed == false) || (event->button() != Qt::LeftButton))
    {
        return false;
    }

    const bool dragged = mDragging;
    mButtonDown = false;
    mDragging   = false;

    if (dragged)
    {
        SMStateItem* state = getScene().stateAt(event->scenePos());
        if (state != nullptr)
        {
            completeExternal(state->getElementId());
        }
        else
        {
            // Released on empty canvas: drop a waypoint there and continue the gesture
            // click by click until a state is picked (Enter = internal, Esc cancels).
            mFromBorder = false;
            mWaypoints.append(getScene().snappedPosition(event->scenePos()));
            updatePreview(event->scenePos());
        }

        event->accept();
        return true;
    }

    // A press with no drag: a border-started gesture selects the state, a tool-started
    // one keeps the source armed for the second (target) click.
    if (mFromBorder)
    {
        const uint32_t source = mSourceId;
        cancelGesture();
        getScene().getModel().getSelectionModel().setSelection(QList<uint32_t>{ source });
        getScene().finishToolGesture();
    }

    event->accept();
    return true;
}

bool SMTransitionTool::mouseDoubleClick(QGraphicsSceneMouseEvent* event)
{
    if (mArmed)
    {
        // A double-click away from any state abandons the unfinished transition and cleans up
        // the preview (issue #516 bug 3): without a valid target the user would otherwise be
        // stuck with a pending gesture. A double-click on a state was already completed as a
        // transition by the first click, so there only the repeat is swallowed.
        if (getScene().stateAt(event->scenePos()) == nullptr)
        {
            cancelGesture();
            getScene().finishToolGesture();
        }

        event->accept();
        return true;
    }

    return false;
}

bool SMTransitionTool::keyPress(QKeyEvent* event)
{
    if (mArmed && ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)))
    {
        completeInternal();
        event->accept();
        return true;
    }

    // Esc abandons the unfinished transition and cleans up the preview (issue #516 bug 3).
    // The scene also maps Esc to cancelActiveGesture(), but handling it here guarantees the
    // pending gesture is dropped even if the key is routed to the active tool first.
    if (mArmed && (event->key() == Qt::Key_Escape))
    {
        getScene().cancelActiveGesture();
        event->accept();
        return true;
    }

    return false;
}

void SMTransitionTool::beginDragFrom(uint32_t sourceId, const QPointF& scenePos)
{
    mArmed      = true;
    mButtonDown = true;
    mDragging   = false;
    mFromBorder = true;
    mSourceId   = sourceId;
    mPressPos   = scenePos;
    mWaypoints.clear();
    createPreview();
    updatePreview(scenePos);
}

void SMTransitionTool::completeExternal(uint32_t targetId)
{
    SMScene& canvas = getScene();
    StateMachineModel& model = canvas.getModel();
    StateMachineData&  data  = model.getData();

    SMStateEntry* source = data.findStateById(mSourceId);
    SMStateEntry* target = data.findStateById(targetId);
    if ((source == nullptr) || (target == nullptr))
    {
        cancelGesture();
        canvas.finishToolGesture();
        return;
    }

    // A Start state is the entry point of its machine level: no transition may enter it
    // (same for a submachine's own Start). Reject the target and keep the gesture armed so
    // the user can pick a valid target; give a brief hint at the cursor.
    if (target->getKind() == SMStateEntry::eStateKind::Start)
    {
        const QList<QGraphicsView*> views = canvas.views();
        QToolTip::showText(QCursor::pos()
                         , QCoreApplication::translate("SMTransitionTool", "A transition cannot enter a Start state.")
                         , (views.isEmpty() ? nullptr : views.first()));
        return;
    }

    const bool selfLoop = (targetId == mSourceId);
    QRectF srcRect = sourceRect();
    QRectF tgtRect = selfLoop ? srcRect : QRectF();
    if (selfLoop == false)
    {
        SMStateItem* targetItem = canvas.stateItem(targetId);
        const SMLayoutNode* node = data.getLayout().findNode(targetId);
        tgtRect = (targetItem != nullptr) ? targetItem->getBoxGeometry()
                : (node != nullptr) ? QRectF(node->x, node->y, node->width, node->height) : QRectF();
    }

    QList<QPointF> waypoints = mWaypoints;
    if (selfLoop && waypoints.isEmpty())
    {
        const double off = 44.0;
        waypoints.append(QPointF(srcRect.center().x() - 22.0, srcRect.top() - off));
        waypoints.append(QPointF(srcRect.center().x() + 22.0, srcRect.top() - off));
    }

    QList<QPointF> points;
    if ((srcRect.width() > 0.0) && (tgtRect.width() > 0.0))
    {
        SMStateItem* sourceItem = canvas.stateItem(mSourceId);
        SMStateItem* targetItem = canvas.stateItem(targetId);
        const double srcRadius = (sourceItem != nullptr ? sourceItem->boxCornerRadius() : NESMDesign::StateCornerRadius);
        const double tgtRadius = (targetItem != nullptr ? targetItem->boxCornerRadius() : NESMDesign::StateCornerRadius);
        const QPointF beginRef = waypoints.isEmpty() ? tgtRect.center() : waypoints.first();
        const QPointF endRef   = waypoints.isEmpty() ? srcRect.center() : waypoints.last();
        points.append(NESMDesign::borderPoint(srcRect, srcRadius, beginRef));
        points.append(waypoints);
        points.append(NESMDesign::borderPoint(tgtRect, tgtRadius, endRef));
    }

    const QString text = QCoreApplication::translate("SMTransitionTool", "Add transition");
    SMCreateTransitionCommand* command = new SMCreateTransitionCommand(  data, model.getNotifier(), *source
                                                                       , SMTransitionEntry::eStimulusKind::Trigger, QString()
                                                                       , target->getName(), points, text);
    model.getUndoStack().push(command);
    const uint32_t transitionId = command->getTransitionId();

    clearPreview();
    resetGesture();
    canvas.finishToolGesture();
    model.getSelectionModel().setSelection(QList<uint32_t>{ transitionId });
}

void SMTransitionTool::completeInternal()
{
    SMScene& canvas = getScene();
    StateMachineModel& model = canvas.getModel();
    StateMachineData&  data  = model.getData();

    SMStateEntry* source = data.findStateById(mSourceId);
    if (source != nullptr)
    {
        const QString text = QCoreApplication::translate("SMTransitionTool", "Add internal transition");
        model.getUndoStack().push(new SMCreateTransitionCommand(  data, model.getNotifier(), *source
                                                                , SMTransitionEntry::eStimulusKind::Trigger, QString()
                                                                , QString(), QList<QPointF>(), text));
    }

    clearPreview();
    resetGesture();
    canvas.finishToolGesture();
}

//////////////////////////////////////////////////////////////////////////
// SMPlaceNoteTool
//////////////////////////////////////////////////////////////////////////

SMPlaceNoteTool::SMPlaceNoteTool(SMScene& scene)
    : SMCanvasTool(scene)
{
}

NESMDesign::eCanvasTool SMPlaceNoteTool::getKind() const
{
    return NESMDesign::eCanvasTool::AddNote;
}

bool SMPlaceNoteTool::mousePress(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        return false;
    }

    SMScene& canvas = getScene();
    StateMachineModel& model = canvas.getModel();

    // Clicking on a state binds the note to that state (badge + popup editor over the box);
    // clicking empty canvas drops a free note box. Only one note per state, so an existing
    // state note is edited instead of creating a second.
    SMStateItem* state = canvas.stateAt(event->scenePos());
    if (state != nullptr)
    {
        const uint32_t stateId = state->getElementId();
        if (model.getData().getLayout().findNoteByOwner(stateId) == nullptr)
        {
            const QRectF box = state->getBoxGeometry();
            SMAddNoteCommand* command = new SMAddNoteCommand(  model.getData(), model.getNotifier()
                                                            , canvas.getLevelId(), stateId, box, QString()
                                                            , QCoreApplication::translate("SMCanvasTool", "Add note"));
            model.getUndoStack().push(command);
        }

        // May retire this tool (single-shot); only stack locals are used afterwards.
        canvas.finishToolGesture();
        SMStateItem* item = canvas.stateItem(stateId);
        if (item != nullptr)
        {
            item->startNoteEdit();
        }

        event->accept();
        return true;
    }

    const QSizeF size{ NESMDesign::NoteDefaultWidth, NESMDesign::NoteDefaultHeight };
    const QPointF topLeft = canvas.snappedPosition(event->scenePos() - QPointF(size.width() / 2.0, size.height() / 2.0));
    const QRectF  box{ topLeft, size };

    SMAddNoteCommand* command = new SMAddNoteCommand(  model.getData(), model.getNotifier()
                                                     , canvas.getLevelId(), box, QString()
                                                     , QCoreApplication::translate("SMCanvasTool", "Add note"));
    model.getUndoStack().push(command);

    const uint32_t noteId = command->getNoteId();
    model.getSelectionModel().setSelection(QList<uint32_t>{ noteId });

    SMNoteItem* item = dynamic_cast<SMNoteItem*>(canvas.findCanvasItem(noteId));

    // May retire this tool (single-shot); only stack locals are used afterwards.
    canvas.finishToolGesture();
    if (item != nullptr)
    {
        item->startInlineEdit();
    }

    event->accept();
    return true;
}

std::unique_ptr<SMCanvasTool> createCanvasTool(NESMDesign::eCanvasTool tool, SMScene& scene)
{
    switch (tool)
    {
    case NESMDesign::eCanvasTool::Select:
        return std::make_unique<SMSelectTool>(scene);
    case NESMDesign::eCanvasTool::AddState:
        return std::make_unique<SMPlaceStateTool>(scene, false);
    case NESMDesign::eCanvasTool::AddFinalState:
        return std::make_unique<SMPlaceStateTool>(scene, true);
    case NESMDesign::eCanvasTool::AddTransition:
        return std::make_unique<SMTransitionTool>(scene);
    case NESMDesign::eCanvasTool::AddNote:
        return std::make_unique<SMPlaceNoteTool>(scene);
    default:
        return nullptr;
    }
}
