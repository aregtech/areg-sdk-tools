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
 *  \file        lusan/view/sm/SMKindGlyph.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design surfaces: the kind marks of a stimulus or operation.
 *
 ************************************************************************/

#include "lusan/view/sm/SMKindGlyph.hpp"

#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMTransition.hpp"

#include <QColor>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPolygonF>
#include <QRectF>

#include <algorithm>
#include <cmath>

namespace
{
    /**
     * \brief   Which of the two exit marks a generic exit operation draws. `<-|` mirrors the entry
     *          mark `->|`, so the two read as a pair; change this line to adopt `|<-` instead.
     **/
    constexpr SMKindGlyph::eGlyph ExitBandGlyph { SMKindGlyph::eGlyph::Exit };
}

void SMKindGlyph::paint(QPainter& painter, const QRectF& rect, eGlyph glyph, const QColor& color)
{
    if (isDrawn(glyph) == false)
    {
        return;
    }

    painter.save();
    QPen pen{ color, 1.2 };
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    const double midY = rect.center().y();
    // The kind marks are drawn inside a centered square, so one constant resizes them all. Square,
    // not the caller's rect: a bolt stretched to a body row's height would tower over its name.
    const double  markSide = std::min(rect.width(), rect.height()) * StimulusGlyphScale;
    const QRectF  mark{ rect.center().x() - (markSide / 2.0), midY - (markSide / 2.0), markSide, markSide };
    switch (glyph)
    {
    case eGlyph::Entry:
    {
        // `->|`, an arrow running into the bar that stands for the state: control arrives here. The
        // bar sits on the right, so the arrow reads the way the eye travels along the row text.
        const double barX = rect.right() - 1.0;
        const double tipX = barX - 2.0;
        painter.drawLine(QPointF(rect.left() + 1.0, midY), QPointF(tipX, midY));
        painter.drawLine(QPointF(tipX - 3.5, midY - 3.0), QPointF(tipX, midY));
        painter.drawLine(QPointF(tipX - 3.5, midY + 3.0), QPointF(tipX, midY));
        painter.drawLine(QPointF(barX, midY - 4.5), QPointF(barX, midY + 4.5));
        break;
    }

    case eGlyph::Exit:
    {
        // `<-|`, the mirror of the entry mark: the same bar, the arrow running away from it. Entry
        // and exit differ only in arrow direction, which is what makes the pair readable.
        const double barX = rect.right() - 1.0;
        const double tipX = rect.left() + 1.0;
        painter.drawLine(QPointF(barX - 2.0, midY), QPointF(tipX, midY));
        painter.drawLine(QPointF(tipX + 3.5, midY - 3.0), QPointF(tipX, midY));
        painter.drawLine(QPointF(tipX + 3.5, midY + 3.0), QPointF(tipX, midY));
        painter.drawLine(QPointF(barX, midY - 4.5), QPointF(barX, midY + 4.5));
        break;
    }

    case eGlyph::ExitAlt:
    {
        // `|<-`, the alternative exit mark, with the bar on the left. Offered beside `<-|` so the
        // two can be compared on a real diagram; flip ExitBandGlyph to adopt it.
        const double barX = rect.left() + 1.0;
        const double tipX = barX + 2.0;
        painter.drawLine(QPointF(barX, midY - 4.5), QPointF(barX, midY + 4.5));
        painter.drawLine(QPointF(tipX, midY), QPointF(rect.right() - 1.0, midY));
        painter.drawLine(QPointF(tipX + 3.5, midY - 3.0), QPointF(tipX, midY));
        painter.drawLine(QPointF(tipX + 3.5, midY + 3.0), QPointF(tipX, midY));
        break;
    }

    case eGlyph::Internal:
    {
        // An internal transition: the same bar the entry and exit marks use, with a hook that leaves
        // it and turns back in.
        const double barX = rect.right() - 1.0;
        const double left = barX - 6.5;
        const double up   = midY - 2.5;
        const double down = midY + 2.5;
        painter.drawLine(QPointF(barX, midY - 4.5), QPointF(barX, midY + 4.5));
        painter.drawLine(QPointF(barX - 1.0, up), QPointF(left, up));
        painter.drawLine(QPointF(left, up), QPointF(left, down));
        painter.drawLine(QPointF(left, down), QPointF(barX - 2.0, down));
        painter.drawLine(QPointF(barX - 2.0, down), QPointF(barX - 4.5, down - 2.0));
        painter.drawLine(QPointF(barX - 2.0, down), QPointF(barX - 4.5, down + 2.0));
        break;
    }

    case eGlyph::Action:
    {
        // A gear: the operation a state or transition runs
        const QPointF centre = mark.center();
        // A cog carries far less ink than a bolt or a clock face of the same width, so it is drawn
        // to a larger fraction of the mark square to weigh the same beside them on a row.
        const double  outer  = mark.width() * 0.56;
        const double  inner  = outer * 0.68;
        constexpr double pi { 3.14159265358979323846 };
        constexpr int teeth  { 6 };
        QPolygonF cog;
        for (int i = 0; i < (teeth * 2); ++i)
        {
            const double angle  = (i * pi) / teeth;
            const double radius = ((i % 2) == 0) ? outer : inner;
            cog.append(centre + QPointF(std::cos(angle) * radius, std::sin(angle) * radius));
        }

        QPainterPath gear;
        gear.addPolygon(cog);
        gear.closeSubpath();
        gear.addEllipse(centre, outer * 0.30, outer * 0.30);     // odd-even: the hub is a hole
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawPath(gear);
        painter.setBrush(Qt::NoBrush);
        break;
    }

    case eGlyph::Trigger:
    {
        // A push button in profile, a solid cap on an open housing
        const double x = mark.left();
        const double y = mark.top();
        const double w = mark.width();
        const double h = mark.height();
        const double round = w * 0.09;
        painter.drawRoundedRect(QRectF(x + (0.19 * w), y + (0.50 * h), 0.63 * w, 0.31 * h), round, round);
        painter.setBrush(color);
        painter.drawRoundedRect(QRectF(x + (0.33 * w), y + (0.28 * h), 0.34 * w, 0.22 * h), round, round);
        painter.setBrush(Qt::NoBrush);
        break;
    }

    case eGlyph::TimerStart:
    {
        // Clock face with a play triangle (start). Every measure is a fraction of the face radius,
        // so the mark keeps its proportions at any StimulusGlyphScale.
        const double radius = mark.width() * 0.375;
        const QRectF face{ mark.center().x() - radius, midY - radius, 2.0 * radius, 2.0 * radius };
        painter.drawEllipse(face);
        painter.setBrush(color);
        painter.drawPolygon(QPolygonF({ QPointF(face.center().x() - (radius * 0.33), midY - (radius * 0.49))
                                      , QPointF(face.center().x() + (radius * 0.56), midY)
                                      , QPointF(face.center().x() - (radius * 0.33), midY + (radius * 0.49)) }));
        painter.setBrush(Qt::NoBrush);
        break;
    }

    case eGlyph::TimerStop:
    {
        // Clock face with a stop square.
        const double radius = mark.width() * 0.375;
        const QRectF face{ mark.center().x() - radius, midY - radius, 2.0 * radius, 2.0 * radius };
        painter.drawEllipse(face);
        painter.setBrush(color);
        painter.drawRect(QRectF(face.center().x() - (radius * 0.4), midY - (radius * 0.4)
                              , radius * 0.8, radius * 0.8));
        painter.setBrush(Qt::NoBrush);
        break;
    }

    case eGlyph::Event:
    {
        // A filled lightning bolt
        const double x = mark.left();
        const double y = mark.top();
        const double w = mark.width();
        const double h = mark.height();
        QPainterPath bolt;
        bolt.moveTo(x + 0.62 * w, y + 0.05 * h);
        bolt.lineTo(x + 0.25 * w, y + 0.55 * h);
        bolt.lineTo(x + 0.48 * w, y + 0.55 * h);
        bolt.lineTo(x + 0.40 * w, y + 0.95 * h);
        bolt.lineTo(x + 0.78 * w, y + 0.42 * h);
        bolt.lineTo(x + 0.53 * w, y + 0.42 * h);
        bolt.closeSubpath();
        painter.setBrush(color);
        painter.drawPath(bolt);
        painter.setBrush(Qt::NoBrush);
        break;
    }

    default:
        break;      // eGlyph::None never reaches here -- isDrawn() turned it away above.
    }

    painter.restore();
}

QIcon SMKindGlyph::icon(eGlyph glyph, const QColor& color, int size /*= GlyphSize*/)
{
    if ((isDrawn(glyph) == false) || (size <= 0))
    {
        return QIcon();
    }

    const qreal ratio = (QGuiApplication::instance() != nullptr) ? qGuiApp->devicePixelRatio() : 1.0;
    QPixmap pixmap(QSize(size, size) * ratio);
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    paint(painter, QRectF(0.0, 0.0, size, size), glyph, color);
    painter.end();

    return QIcon(pixmap);
}

QString SMKindGlyph::word(eGlyph glyph)
{
    // The kind only: the Word style cannot tell a timer start from a timer stop, which is exactly
    // why Glyph is the default. Words are the readability fallback, not a second vocabulary.
    switch (glyph)
    {
    case eGlyph::Event:
        return QCoreApplication::translate("SMKindGlyph", "event");
    case eGlyph::TimerStart:
    case eGlyph::TimerStop:
        return QCoreApplication::translate("SMKindGlyph", "timer");
    case eGlyph::Trigger:
        return QCoreApplication::translate("SMKindGlyph", "trigger");
    default:
        return QString();
    }
}

QString SMKindGlyph::prefix(eGlyph glyph)
{
    if (Style != eStyle::Word)
    {
        return QString();
    }

    const QString text = word(glyph);
    return text.isEmpty() ? text : (text + QLatin1Char(' '));
}

bool SMKindGlyph::isDrawn(eGlyph glyph)
{
    if (glyph == eGlyph::None)
    {
        return false;
    }

    // A band mark (enter / exit / internal) is always drawn: it is the row's position marker,
    // not a spelling of its kind, so the Word style does not replace it.
    switch (glyph)
    {
    case eGlyph::Trigger:
    case eGlyph::Event:
    case eGlyph::TimerStart:
    case eGlyph::TimerStop:
    case eGlyph::Action:
        // A trigger is marked too, otherwise the row `on someTrigger` drew an empty gutter next to
        // `someAction()`. It must not draw the `( )` arc pair, which read as an empty argument list.
        return (Style == eStyle::Glyph);
    default:
        return true;
    }
}

SMKindGlyph::eGlyph SMKindGlyph::stimulusGlyph(const SMTransitionEntry& transition)
{
    return transition.getStimulus().isEmpty() ? eGlyph::None : stimulusGlyph(transition.getStimulusKind());
}

SMKindGlyph::eGlyph SMKindGlyph::stimulusGlyph(SMTransitionEntry::eStimulusKind kind)
{
    switch (kind)
    {
    case SMTransitionEntry::eStimulusKind::Event:
        return eGlyph::Event;
    case SMTransitionEntry::eStimulusKind::Timer:
        // A timer stimulus is the expiry of a running timer, so it takes the running clock.
        return eGlyph::TimerStart;
    default:
        return eGlyph::Trigger;
    }
}

SMKindGlyph::eGlyph SMKindGlyph::operationGlyph(const SMOperationBase& op)
{
    switch (op.getOperationType())
    {
    case SMOperationBase::eOperation::EventSend:
        return eGlyph::Event;
    case SMOperationBase::eOperation::TimerStart:
        return eGlyph::TimerStart;
    case SMOperationBase::eOperation::TimerStop:
        return eGlyph::TimerStop;
    default:
        // An action call and an attribute assignment are both actions, so both wear the gear. The
        // band-mark fallback made them draw the self-loop of the row that fires them.
        return eGlyph::Action;
    }
}

SMKindGlyph::eGlyph SMKindGlyph::exitGlyph()
{
    return ExitBandGlyph;
}
