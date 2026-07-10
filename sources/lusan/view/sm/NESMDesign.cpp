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
#include <limits>

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

QColor NESMDesign::stateBodyColor(const QPalette& palette)
{
    return palette.color(QPalette::AlternateBase);
}

QColor NESMDesign::startStateColor(const QPalette& palette)
{
    // A green "go" fill; brighter on dark themes so it stays vivid.
    const bool dark = (palette.color(QPalette::Base).lightnessF() < 0.5);
    return (dark ? QColor(0x2E, 0x9E, 0x4F) : QColor(0x2E, 0x7D, 0x32));
}

QColor NESMDesign::finalStateColor(const QPalette& palette)
{
    // A deep red "stop" fill clearly distinct from normal state bodies.
    const bool dark = (palette.color(QPalette::Base).lightnessF() < 0.5);
    return (dark ? QColor(0xB5, 0x4A, 0x45) : QColor(0x9A, 0x31, 0x2F));
}

QColor NESMDesign::stateBorderColor(const QPalette& palette)
{
    QColor result{ palette.color(QPalette::WindowText) };
    result.setAlphaF(0.6);
    return result;
}

QColor NESMDesign::deriveHeaderShade(const QColor& bodyColor)
{
    return bodyColor.darker(HeaderShadeFactor);
}

QColor NESMDesign::contrastTextColor(const QColor& fill)
{
    // Rec. 601 luma decides between near-white and near-black text.
    const double luma = 0.299 * fill.redF() + 0.587 * fill.greenF() + 0.114 * fill.blueF();
    return (luma < 0.5 ? QColor(0xF2, 0xF2, 0xF2) : QColor(0x1A, 0x1A, 0x1A));
}

QColor NESMDesign::edgeColor(const QPalette& palette)
{
    QColor result{ palette.color(QPalette::WindowText) };
    result.setAlphaF(0.8);
    return result;
}

QColor NESMDesign::edgeIncomingColor(const QPalette& palette)
{
    return palette.color(QPalette::Highlight);
}

QColor NESMDesign::edgeOutgoingColor(const QPalette& palette)
{
    // A warmer accent so the incoming/outgoing direction is distinguishable at a glance.
    QColor result{ palette.color(QPalette::Highlight) };
    return QColor::fromHsv((result.hue() + 150) % 360, result.saturation(), result.value());
}

QPointF NESMDesign::borderPoint(const QRectF& rect, const QPointF& towards)
{
    const QPointF center = rect.center();
    QPointF dir = towards - center;
    const double halfW = rect.width() / 2.0;
    const double halfH = rect.height() / 2.0;
    if ((halfW <= 0.0) || (halfH <= 0.0))
    {
        return center;
    }

    // Scale the direction so it just reaches the nearer of the vertical/horizontal borders.
    const double sx = (std::abs(dir.x()) > 1e-6) ? halfW / std::abs(dir.x()) : std::numeric_limits<double>::max();
    const double sy = (std::abs(dir.y()) > 1e-6) ? halfH / std::abs(dir.y()) : std::numeric_limits<double>::max();
    const double scale = std::min(sx, sy);
    if (scale >= std::numeric_limits<double>::max())
    {
        return QPointF(rect.right(), center.y());
    }

    return center + dir * scale;
}

QPointF NESMDesign::borderPoint(const QRectF& rect, double radius, const QPointF& towards)
{
    const QPointF plain = borderPoint(rect, towards);
    const double  rad   = std::clamp(radius, 0.0, std::min(rect.width(), rect.height()) / 2.0);
    if (rad <= 1e-6)
    {
        return plain;
    }

    // Only the four corner squares differ from the plain rectangle border.
    const bool nearLeft   = (plain.x() < rect.left() + rad);
    const bool nearRight  = (plain.x() > rect.right() - rad);
    const bool nearTop    = (plain.y() < rect.top() + rad);
    const bool nearBottom = (plain.y() > rect.bottom() - rad);
    if (((nearLeft || nearRight) && (nearTop || nearBottom)) == false)
    {
        return plain;
    }

    const double kx = (nearLeft ? rect.left() + rad : rect.right() - rad);
    const double ky = (nearTop  ? rect.top() + rad  : rect.bottom() - rad);

    // Intersect the center ray with the corner circle (exit point = larger root).
    const QPointF center = rect.center();
    QPointF dir = towards - center;
    const double len = std::hypot(dir.x(), dir.y());
    if (len < 1e-6)
    {
        return plain;
    }

    dir /= len;
    const QPointF toCorner{ kx - center.x(), ky - center.y() };
    const double b    = toCorner.x() * dir.x() + toCorner.y() * dir.y();
    const double c0   = toCorner.x() * toCorner.x() + toCorner.y() * toCorner.y() - rad * rad;
    const double disc = b * b - c0;
    if (disc < 0.0)
    {
        return plain;
    }

    const double t = b + std::sqrt(disc);
    return center + dir * t;
}

QPointF NESMDesign::nearestBorderPoint(const QRectF& rect, double radius, const QPointF& point)
{
    if ((rect.width() <= 0.0) || (rect.height() <= 0.0))
    {
        return rect.center();
    }

    // Clamp into the rectangle, then push to the closest side: the nearest point on
    // the plain rectangle border, valid for points inside and outside alike.
    QPointF q{ std::clamp(point.x(), rect.left(), rect.right())
             , std::clamp(point.y(), rect.top(), rect.bottom()) };
    const double dl = q.x() - rect.left();
    const double dr = rect.right() - q.x();
    const double dt = q.y() - rect.top();
    const double db = rect.bottom() - q.y();
    const double dm = std::min({ dl, dr, dt, db });
    if (dm == dl)
    {
        q.setX(rect.left());
    }
    else if (dm == dr)
    {
        q.setX(rect.right());
    }
    else if (dm == dt)
    {
        q.setY(rect.top());
    }
    else
    {
        q.setY(rect.bottom());
    }

    const double rad = std::clamp(radius, 0.0, std::min(rect.width(), rect.height()) / 2.0);
    if (rad <= 1e-6)
    {
        return q;
    }

    // In a corner square the border is the corner arc: project onto the corner circle.
    const bool nearLeft   = (q.x() < rect.left() + rad);
    const bool nearRight  = (q.x() > rect.right() - rad);
    const bool nearTop    = (q.y() < rect.top() + rad);
    const bool nearBottom = (q.y() > rect.bottom() - rad);
    if (((nearLeft || nearRight) && (nearTop || nearBottom)) == false)
    {
        return q;
    }

    const double kx = (nearLeft ? rect.left() + rad : rect.right() - rad);
    const double ky = (nearTop  ? rect.top() + rad  : rect.bottom() - rad);

    QPointF out = point - QPointF(kx, ky);
    const double len = std::hypot(out.x(), out.y());
    if (len < 1e-6)
    {
        return q;
    }

    return QPointF(kx, ky) + out * (rad / len);
}

QList<QPointF> NESMDesign::arcPolyline(const QPointF& begin, const QPointF& end, double bulge, int samples)
{
    constexpr double Pi = 3.14159265358979323846;
    QList<QPointF> points;
    const QPointF chord = end - begin;
    const double c = std::hypot(chord.x(), chord.y());
    if ((c < 1e-6) || (std::abs(bulge) < 1e-6) || (samples < 1))
    {
        points << begin << end;
        return points;
    }

    const double  h    = bulge * c / 2.0;                      // signed sagitta (arc height)
    const QPointF nrm  { -chord.y() / c, chord.x() / c };      // unit left perpendicular
    const QPointF mid  = (begin + end) / 2.0;
    const QPointF apex = mid + nrm * h;
    const double  r    = (c * c / 4.0 + h * h) / (2.0 * std::abs(h));
    const QPointF signedNrm = nrm * (h >= 0.0 ? 1.0 : -1.0);
    const QPointF center    = apex - signedNrm * r;

    const double a0 = std::atan2(begin.y() - center.y(), begin.x() - center.x());
    const double a1 = std::atan2(end.y() - center.y(), end.x() - center.x());
    const double aApex = std::atan2(apex.y() - center.y(), apex.x() - center.x());

    const auto wrap = [Pi](double a) -> double
    {
        while (a <= -Pi) a += 2.0 * Pi;
        while (a > Pi)   a -= 2.0 * Pi;
        return a;
    };

    double sweep  = wrap(a1 - a0);
    const double toApex = wrap(aApex - a0);
    // The drawn sweep must pass through the apex; flip direction when it does not.
    if (((sweep >= 0.0) != (toApex >= 0.0)) || (std::abs(toApex) > std::abs(sweep)))
    {
        sweep += (sweep >= 0.0 ? -2.0 * Pi : 2.0 * Pi);
    }

    points.reserve(samples + 1);
    for (int i = 0; i <= samples; ++i)
    {
        const double a = a0 + sweep * (static_cast<double>(i) / samples);
        points << QPointF(center.x() + r * std::cos(a), center.y() + r * std::sin(a));
    }

    return points;
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
