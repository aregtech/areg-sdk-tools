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

#include <QContextMenuEvent>
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QResizeEvent>
#include <QScrollBar>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace
{
    /**
     * \brief   Draws the crosshair shown while a placement tool is armed. Qt::CrossCursor has a
     *          fixed, heavier size, so it is drawn instead at NESMDesign::ToolCursorSize, as a
     *          dark cross on a light halo that stays visible on a light and a dark canvas.
     * \param   ratio   The device pixel ratio of the screen showing the cursor.
     **/
    QCursor makeToolCursor(qreal ratio)
    {
        if (NESMDesign::ToolCursorSize <= 0)
        {
            return QCursor(Qt::CrossCursor);
        }

        // An odd span puts the hot spot on a single center pixel, so the click lands exactly
        // where the arms meet instead of half a pixel off.
        const int span  = std::max(3, NESMDesign::ToolCursorSize | 1);
        const int width = std::max(1, NESMDesign::ToolCursorWidth);
        QPixmap pixmap(QSize(span, span) * ratio);
        pixmap.setDevicePixelRatio(ratio);
        pixmap.fill(Qt::transparent);

        // Half-pixel offset: a pen centered on a pixel boundary would smear over two rows.
        const qreal mid = (span / 2) + 0.5;
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, false);
        for (int pass = 0; pass < 2; ++pass)
        {
            const bool halo = (pass == 0);
            painter.setPen(QPen(halo ? QColor(255, 255, 255, 190) : QColor(16, 16, 16)
                                , halo ? (width + 2) : width));
            painter.drawLine(QPointF(0.0, mid), QPointF(span, mid));
            painter.drawLine(QPointF(mid, 0.0), QPointF(mid, span));
        }

        painter.end();

        return QCursor(pixmap, span / 2, span / 2);
    }
}

SMGraphicsView::SMGraphicsView(QWidget* parent /*= nullptr*/)
    : QGraphicsView(parent)
    , mZoom         (NESMDesign::ZoomDefault)
    , mPanning      (false)
    , mPanStart     ( )
    , mToolArmed    (false)
{
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setRubberBandSelectionMode(Qt::IntersectsItemShape);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    // No CacheBackground: scrolling blits the cached pixmap and repaints only the exposed
    // strip, which misaligns the grid lines by a subpixel at non-integer zoom levels.
    setFocusPolicy(Qt::StrongFocus);
}

void SMGraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    // The view keeps its centre while it is resized, so every pixel of the scene moves. The
    // default minimal repaint only covers the strip that was added and leaves the rest stale.
    viewport()->update();
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

void SMGraphicsView::setToolCursor(NESMDesign::eCanvasTool tool)
{
    // Only the tools that drop a new element at the clicked spot need to aim; see
    // NESMDesign::toolAims for the list and the reasoning.
    mToolArmed = NESMDesign::toolAims(tool);
    if (mPanning == false)
    {
        applyToolCursor();
    }
}

void SMGraphicsView::applyToolCursor()
{
    if (mToolArmed)
    {
        // The viewport takes the cursor too. One the scene has written there sits in front of the
        // view's own and would mask the tool shape.
        const QCursor tool = makeToolCursor(devicePixelRatioF());
        setCursor(tool);
        viewport()->setCursor(tool);
        return;
    }

    // Disarming clears the viewport as well. QGraphicsView remembers the viewport cursor when an
    // item sets a hover cursor and writes that copy back later, leaving the crosshair on the canvas.
    viewport()->unsetCursor();
    unsetCursor();
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

    // Re-assert the tool shape here, because the scene may write its remembered cursor onto the
    // viewport after applyToolCursor() has run. An item that carries its own cursor keeps it.
    if (mToolArmed && (mPanning == false))
    {
        const QGraphicsItem* under = itemAt(event->pos());
        if (((under == nullptr) || (under->hasCursor() == false))
            && (viewport()->cursor().shape() != Qt::BitmapCursor))
        {
            applyToolCursor();
        }
    }
}

void SMGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (mPanning && (event->button() == Qt::MiddleButton))
    {
        mPanning = false;
        applyToolCursor();
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void SMGraphicsView::contextMenuEvent(QContextMenuEvent* event)
{
    // event->pos() is already in viewport coordinates: the Design page maps it to the
    // scene (mapToScene) and to the global position (viewport()->mapToGlobal).
    emit signalContextMenuRequested(event->pos());
    event->accept();
}
