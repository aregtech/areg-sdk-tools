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

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QCoreApplication>
#include <QGraphicsSceneMouseEvent>

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
    default:
        return nullptr;
    }
}
