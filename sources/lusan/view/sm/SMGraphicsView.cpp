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
 *  \file        lusan/view/sm/SMGraphicsView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas viewport.
 *
 ************************************************************************/

#include "lusan/view/sm/SMGraphicsView.hpp"

#include "lusan/view/sm/NESMDesign.hpp"

#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

SMGraphicsView::SMGraphicsView(QWidget* parent /*= nullptr*/)
    : QGraphicsView(parent)
    , mZoom     (NESMDesign::ZoomDefault)
    , mPanning  (false)
    , mPanStart ( )
{
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setRubberBandSelectionMode(Qt::IntersectsItemShape);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setCacheMode(QGraphicsView::CacheBackground);
    setFocusPolicy(Qt::StrongFocus);
}

void SMGraphicsView::setZoom(int percent)
{
    percent = std::clamp(percent, NESMDesign::ZoomMin, NESMDesign::ZoomMax);
    if (percent == mZoom)
    {
        return;
    }

    mZoom = percent;
    const qreal scale = static_cast<qreal>(mZoom) / 100.0;
    setTransform(QTransform::fromScale(scale, scale));
    emit signalZoomChanged(mZoom);
}

void SMGraphicsView::zoomIn()
{
    const int next = static_cast<int>(std::lround(mZoom * NESMDesign::ZoomStepFactor));
    setZoom(std::max(next, mZoom + 1));
}

void SMGraphicsView::zoomOut()
{
    const int next = static_cast<int>(std::lround(mZoom / NESMDesign::ZoomStepFactor));
    setZoom(std::min(next, mZoom - 1));
}

void SMGraphicsView::zoomReset()
{
    setZoom(NESMDesign::ZoomDefault);
}

void SMGraphicsView::zoomToFit()
{
    if (scene() == nullptr)
    {
        return;
    }

    const QRectF content{ scene()->itemsBoundingRect() };
    if (content.isEmpty())
    {
        zoomReset();
        centerOn(sceneRect().center());
        return;
    }

    const QRectF frame{ content.adjusted(-NESMDesign::ZoomFitMargin, -NESMDesign::ZoomFitMargin
                                        ,  NESMDesign::ZoomFitMargin,  NESMDesign::ZoomFitMargin) };
    const QSizeF port{ viewport()->size() };
    const qreal scale   = std::min(port.width() / frame.width(), port.height() / frame.height());
    const int   percent = static_cast<int>(std::floor(scale * 100.0));

    setZoom(percent);
    centerOn(frame.center());
}

void SMGraphicsView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier))
    {
        if (event->angleDelta().y() > 0)
        {
            zoomIn();
        }
        else if (event->angleDelta().y() < 0)
        {
            zoomOut();
        }

        event->accept();
        return;
    }

    QGraphicsView::wheelEvent(event);
}

void SMGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        mPanning  = true;
        mPanStart = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void SMGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (mPanning)
    {
        const QPoint delta{ event->pos() - mPanStart };
        mPanStart = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void SMGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (mPanning && (event->button() == Qt::MiddleButton))
    {
        mPanning = false;
        unsetCursor();
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}
