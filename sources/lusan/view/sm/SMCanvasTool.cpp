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
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QCoreApplication>
#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainterPath>
#include <QPen>

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
{
}

NESMDesign::eCanvasTool SMPlaceStateTool::getKind() const
{
    return (mFinal ? NESMDesign::eCanvasTool::AddFinalState : NESMDesign::eCanvasTool::AddState);
}

bool SMPlaceStateTool::mousePress(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        return false;
    }

    placeState(event->scenePos());
    event->accept();
    return true;
}

bool SMPlaceStateTool::mouseRelease(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        return false;
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

void SMPlaceStateTool::placeState(const QPointF& scenePos)
{
    SMScene& canvas = getScene();
    StateMachineModel& model = canvas.getModel();
    StateMachineData&  data  = model.getData();
    SMStateData* level = data.findLevel(canvas.getLevelId());
    if (level == nullptr)
    {
        return;
    }

    const QString base = (mFinal ? QStringLiteral("NewFinal") : QStringLiteral("NewState"));
    QString name{ base };
    for (int i = 2; data.findState(name) != nullptr; ++i)
    {
        name = base + QString::number(i);
    }

    const QSizeF  size{ NESMDesign::StateDefaultWidth, NESMDesign::StateDefaultHeight };
    const QPointF topLeft = canvas.snappedPosition(scenePos - QPointF(size.width() / 2.0, size.height() / 2.0));

    const SMStateEntry::eStateKind kind = (mFinal ? SMStateEntry::eStateKind::Final : SMStateEntry::eStateKind::Normal);
    SMCreateStateCommand* command = new SMCreateStateCommand(  data, model.getNotifier(), *level, name, kind
                                                             , QRectF(topLeft, size)
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

    const QPointF ref = (mWaypoints.isEmpty() ? cursor : mWaypoints.first());
    QPainterPath path;
    path.moveTo(NESMDesign::borderPoint(src, ref));
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
    if (mArmed == false)
    {
        if (state == nullptr)
        {
            return false;
        }

        mArmed    = true;
        mDragging = false;
        mSourceId = state->getElementId();
        mPressPos = event->scenePos();
        mWaypoints.clear();
        createPreview();
        updatePreview(event->scenePos());
        event->accept();
        return true;
    }

    if (state != nullptr)
    {
        completeExternal(state->getElementId());
    }
    else
    {
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

    if ((mDragging == false) && (toolDistance(event->scenePos(), mPressPos) > DragThreshold))
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

    if (mDragging)
    {
        SMStateItem* state = getScene().stateAt(event->scenePos());
        if (state != nullptr)
        {
            completeExternal(state->getElementId());
        }
        else
        {
            offerInternalOnEmpty();
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
    // Swallow the repeat of a click pair while a gesture is in progress.
    if (mArmed)
    {
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

    return false;
}

void SMTransitionTool::beginDragFrom(uint32_t sourceId, const QPointF& scenePos)
{
    mArmed      = true;
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
        const QPointF beginRef = waypoints.isEmpty() ? tgtRect.center() : waypoints.first();
        const QPointF endRef   = waypoints.isEmpty() ? srcRect.center() : waypoints.last();
        points.append(NESMDesign::borderPoint(srcRect, beginRef));
        points.append(waypoints);
        points.append(NESMDesign::borderPoint(tgtRect, endRef));
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

void SMTransitionTool::offerInternalOnEmpty()
{
    SMScene& canvas = getScene();
    QWidget* parent = (canvas.views().isEmpty() == false) ? canvas.views().first() : nullptr;
    const QMessageBox::StandardButton answer =
            QMessageBox::question(parent, QCoreApplication::translate("SMTransitionTool", "Internal Transition")
                                  , QCoreApplication::translate("SMTransitionTool", "Create an internal transition (no target) on the source state?")
                                  , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (answer == QMessageBox::Yes)
    {
        completeInternal();
    }
    else
    {
        cancelGesture();
        canvas.finishToolGesture();
    }
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
    default:
        return nullptr;
    }
}
