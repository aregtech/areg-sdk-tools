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

std::unique_ptr<SMCanvasTool> createCanvasTool(NESMDesign::eCanvasTool tool, SMScene& scene)
{
    switch (tool)
    {
    case NESMDesign::eCanvasTool::Select:
        return std::make_unique<SMSelectTool>(scene);
    default:
        return nullptr;
    }
}
