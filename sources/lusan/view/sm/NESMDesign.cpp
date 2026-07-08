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
 *  \file        lusan/view/sm/NESMDesign.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas rendering constants and helpers.
 *
 ************************************************************************/

#include "lusan/view/sm/NESMDesign.hpp"

#include <QPainter>
#include <QPalette>
#include <QPen>

#include <algorithm>
#include <cmath>

QColor NESMDesign::canvasBackground(const QPalette& palette)
{
    return palette.color(QPalette::Base);
}

QColor NESMDesign::gridColor(const QPalette& palette, double opacity /*= 1.0*/)
{
    QColor result{ palette.color(QPalette::WindowText) };
    result.setAlphaF(0.16 * std::clamp(opacity, 0.0, 1.0));
    return result;
}

QColor NESMDesign::selectionColor(const QPalette& palette)
{
    return palette.color(QPalette::Highlight);
}

qreal NESMDesign::snapValue(qreal value, int gridSize)
{
    const qreal grid = static_cast<qreal>(std::max(gridSize, GridSizeMin));
    return std::round(value / grid) * grid;
}

QPointF NESMDesign::snapPoint(const QPointF& point, int gridSize)
{
    return QPointF(snapValue(point.x(), gridSize), snapValue(point.y(), gridSize));
}

void NESMDesign::paintSelectionFrame(QPainter* painter, const QRectF& bounds, const QPalette& palette, bool hasFocus)
{
    QPen pen{ selectionColor(palette) };
    pen.setCosmetic(true);
    pen.setWidthF(hasFocus ? 2.0 : 1.5);
    pen.setStyle(hasFocus ? Qt::SolidLine : Qt::DashLine);

    painter->save();
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(bounds);
    painter->restore();
}
