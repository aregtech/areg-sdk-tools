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
 *  \file        lusan/view/sm/SMCanvasItem.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas item base.
 *
 ************************************************************************/

#include "lusan/view/sm/SMCanvasItem.hpp"

#include "lusan/view/sm/SMScene.hpp"

SMCanvasItem::SMCanvasItem(uint32_t elementId, QGraphicsItem* parent /*= nullptr*/)
    : QGraphicsItem (parent)
    , mElementId    (elementId)
    , mConnHighlight(eConnHighlight::None)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

void SMCanvasItem::setConnHighlight(eConnHighlight highlight)
{
    if (highlight != mConnHighlight)
    {
        mConnHighlight = highlight;
        update();
    }
}

void SMCanvasItem::updateFromModel()
{
}

SMCanvasItem::~SMCanvasItem()
{
    SMScene* canvas = qobject_cast<SMScene*>(scene());
    if (canvas != nullptr)
    {
        canvas->unregisterCanvasItem(*this);
    }
}

QVariant SMCanvasItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    switch (change)
    {
    case QGraphicsItem::ItemPositionChange:
    {
        const SMScene* canvas = qobject_cast<const SMScene*>(scene());
        if ((canvas != nullptr) && canvas->isInteractiveSnap())
        {
            return NESMDesign::snapPoint(value.toPointF(), canvas->getGridSize());
        }

        break;
    }

    case QGraphicsItem::ItemSceneChange:
    {
        SMScene* canvas = qobject_cast<SMScene*>(scene());
        if (canvas != nullptr)
        {
            canvas->unregisterCanvasItem(*this);
        }

        break;
    }

    case QGraphicsItem::ItemSceneHasChanged:
    {
        SMScene* canvas = qobject_cast<SMScene*>(scene());
        if (canvas != nullptr)
        {
            canvas->registerCanvasItem(*this);
        }

        break;
    }

    default:
        break;
    }

    return QGraphicsItem::itemChange(change, value);
}
