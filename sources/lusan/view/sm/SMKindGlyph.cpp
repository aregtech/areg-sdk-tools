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

namespace
{
    /**
     * \brief   Which of the two exit marks a generic exit operation draws. Both are implemented
     *          (\c Exit is `<-|`, \c ExitAlt is `|<-`) so the pair can be compared on a real
     *          diagram; change this one line to adopt the other. `<-|` is the current choice: it
     *          mirrors the entry mark `->|` exactly -- the same bar, the arrow reversed -- so the
     *          two read as a pair rather than as two unrelated marks.
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
    // The two stimulus marks are drawn inside a centered square scaled by StimulusGlyphScale, so the
    // one constant resizes both. Square, not the caller's rect: a body row is taller than it is wide,
    // and a bolt stretched to the row height would tower over the name it belongs to.
    const double  markSide = std::min(rect.width(), rect.height()) * StimulusGlyphScale;
    const QRectF  mark{ rect.center().x() - (markSide / 2.0), midY - (markSide / 2.0), markSide, markSide };
    switch (glyph)
    {
    case eGlyph::Entry:
    {
        // `->|` -- an arrow running rightwards INTO the bar that stands for the state: control
        // arrives here. The bar sits on the right so the arrow reads left-to-right, the same
        // direction the eye travels along the row text beside it.
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
        // `<-|` -- the mirror of the entry mark: the same bar on the right, but the arrow runs
        // AWAY from it, leftwards. Entry and exit therefore differ only in arrow direction,
        // which is what makes the pair readable at a glance in a small box.
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
        // `|<-` -- the alternative exit mark: the bar on the LEFT with the arrow pointing back
        // into it. Offered beside `<-|` so the two can be compared on a real diagram; flip
        // ExitBandGlyph (above) to adopt it.
        const double barX = rect.left() + 1.0;
        const double tipX = barX + 2.0;
        painter.drawLine(QPointF(barX, midY - 4.5), QPointF(barX, midY + 4.5));
        painter.drawLine(QPointF(tipX, midY), QPointF(rect.right() - 1.0, midY));
        painter.drawLine(QPointF(tipX + 3.5, midY - 3.0), QPointF(tipX, midY));
        painter.drawLine(QPointF(tipX + 3.5, midY + 3.0), QPointF(tipX, midY));
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
        // A filled lightning bolt -- the same mark as the Events page and the toolbar so "event"
        // reads the same way on every surface. Normalized coordinates (0..1 in the mark rect)
        // trace the bolt, then map onto the rect.
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

    case eGlyph::Internal:
    default:
    {
        // Self-loop: an open circle with an arrowhead at the gap.
        const QRectF loop{ rect.center().x() - 4.0, midY - 4.0, 8.0, 8.0 };
        painter.drawArc(loop, 30 * 16, 300 * 16);
        const QPointF tip{ loop.right(), midY + 2.0 };
        painter.drawLine(tip, tip + QPointF(-3.5, 1.0));
        painter.drawLine(tip, tip + QPointF(-0.5, -3.5));
        break;
    }
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

    // A band mark (enter / exit / do) is always drawn: it is the row's position marker, not a
    // spelling of its kind, so the Word style does not replace it.
    switch (glyph)
    {
    case eGlyph::Trigger:
        // A trigger carries NO mark, in either style. An event and a timer are marked because they
        // name where the stimulus COMES FROM, and the two must be told apart; a trigger is the plain
        // case, with nothing to distinguish it from. The `( )` arcs it used to draw read as an empty
        // argument list in front of a signature that already ends in one -- `( )on()`.
        return false;

    case eGlyph::Event:
    case eGlyph::TimerStart:
    case eGlyph::TimerStop:
        return (Style == eStyle::Glyph);
    default:
        return true;
    }
}

SMKindGlyph::eGlyph SMKindGlyph::stimulusGlyph(const SMTransitionEntry& transition)
{
    if (transition.getStimulus().isEmpty())
    {
        return eGlyph::None;
    }

    switch (transition.getStimulusKind())
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
        return eGlyph::None;
    }
}

SMKindGlyph::eGlyph SMKindGlyph::exitGlyph()
{
    return ExitBandGlyph;
}
