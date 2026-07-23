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
 *  \file        lusan/view/sm/SMToolIcons.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM drawing-toolbar vector icons.
 *
 ************************************************************************/

#include "lusan/view/sm/SMToolIcons.hpp"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPen>
#include <QPixmap>
#include <QPolygonF>

#include <cmath>

namespace
{
    //!< The logical drawing canvas size; the icon scales from here.
    constexpr int IconSize{ 32 };

    //!< Draws a small filled arrowhead at \p tip pointing along (\p tip - \p from).
    void arrowHead(QPainter& p, const QPointF& from, const QPointF& tip, double size, const QColor& color)
    {
        QPointF dir = tip - from;
        const double len = std::hypot(dir.x(), dir.y());
        if (len < 1e-6)
        {
            return;
        }

        dir /= len;
        const QPointF normal{ -dir.y(), dir.x() };
        const QPointF base = tip - dir * size;
        QPainterPath head;
        head.moveTo(tip);
        head.lineTo(base + normal * (size * 0.55));
        head.lineTo(base - normal * (size * 0.55));
        head.closeSubpath();
        p.save();
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        p.drawPath(head);
        p.restore();
    }

    void drawGlyph(QPainter& p, SMToolIcons::eIcon kind, const QColor& color)
    {
        QPen pen{ color, 2.0 };
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);

        switch (kind)
        {
        case SMToolIcons::eIcon::AddState:
            p.drawRoundedRect(QRectF(4.0, 10.0, 15.0, 12.0), 3.0, 3.0);
            p.drawLine(QPointF(24.0, 7.0), QPointF(24.0, 15.0));
            p.drawLine(QPointF(20.0, 11.0), QPointF(28.0, 11.0));
            break;

        case SMToolIcons::eIcon::AddFinalState:
            p.drawEllipse(QPointF(16.0, 16.0), 9.0, 9.0);
            p.setBrush(color);
            p.drawEllipse(QPointF(16.0, 16.0), 4.0, 4.0);
            break;

        case SMToolIcons::eIcon::AddTransition:
            p.setBrush(color);
            p.drawEllipse(QPointF(6.0, 23.0), 2.2, 2.2);
            p.setBrush(Qt::NoBrush);
            p.drawLine(QPointF(6.0, 23.0), QPointF(24.0, 10.0));
            arrowHead(p, QPointF(6.0, 23.0), QPointF(25.0, 9.0), 6.0, color);
            break;

        case SMToolIcons::eIcon::AddNote:
        case SMToolIcons::eIcon::NoteColor:
        {
            const double fold = 6.0;
            QPainterPath page;
            page.moveTo(8.0, 6.0);
            page.lineTo(24.0 - fold, 6.0);
            page.lineTo(24.0, 6.0 + fold);
            page.lineTo(24.0, 26.0);
            page.lineTo(8.0, 26.0);
            page.closeSubpath();
            p.drawPath(page);
            p.drawLine(QPointF(24.0 - fold, 6.0), QPointF(24.0 - fold, 6.0 + fold));
            p.drawLine(QPointF(24.0 - fold, 6.0 + fold), QPointF(24.0, 6.0 + fold));
            p.drawLine(QPointF(11.0, 15.0), QPointF(21.0, 15.0));
            p.drawLine(QPointF(11.0, 19.0), QPointF(19.0, 19.0));
            if (kind == SMToolIcons::eIcon::NoteColor)
            {
                p.fillRect(QRectF(18.0, 20.0, 8.0, 8.0), color);
            }
            break;
        }

        case SMToolIcons::eIcon::StateColor:
        {
            const QRectF sw(6.0, 9.0, 20.0, 14.0);
            p.drawRoundedRect(sw, 3.0, 3.0);
            QPainterPath half;
            half.moveTo(sw.topLeft());
            half.lineTo(sw.bottomLeft());
            half.lineTo(sw.bottomRight());
            half.closeSubpath();
            p.fillPath(half, color);
            break;
        }

        case SMToolIcons::eIcon::EdgeColor:
            p.drawLine(QPointF(5.0, 22.0), QPointF(20.0, 11.0));
            arrowHead(p, QPointF(5.0, 22.0), QPointF(21.0, 10.0), 5.0, color);
            p.fillRect(QRectF(20.0, 18.0, 8.0, 8.0), color);
            break;

        case SMToolIcons::eIcon::AlignLeft:
            p.drawLine(QPointF(6.0, 6.0), QPointF(6.0, 26.0));
            p.fillRect(QRectF(6.0, 9.0, 11.0, 5.0), color);
            p.fillRect(QRectF(6.0, 18.0, 18.0, 5.0), color);
            break;

        case SMToolIcons::eIcon::AlignRight:
            p.drawLine(QPointF(26.0, 6.0), QPointF(26.0, 26.0));
            p.fillRect(QRectF(15.0, 9.0, 11.0, 5.0), color);
            p.fillRect(QRectF(8.0, 18.0, 18.0, 5.0), color);
            break;

        case SMToolIcons::eIcon::AlignTop:
            p.drawLine(QPointF(6.0, 6.0), QPointF(26.0, 6.0));
            p.fillRect(QRectF(9.0, 6.0, 5.0, 11.0), color);
            p.fillRect(QRectF(18.0, 6.0, 5.0, 18.0), color);
            break;

        case SMToolIcons::eIcon::AlignBottom:
            p.drawLine(QPointF(6.0, 26.0), QPointF(26.0, 26.0));
            p.fillRect(QRectF(9.0, 15.0, 5.0, 11.0), color);
            p.fillRect(QRectF(18.0, 8.0, 5.0, 18.0), color);
            break;

        case SMToolIcons::eIcon::DistributeHorizontal:
            p.fillRect(QRectF(6.0, 9.0, 4.0, 14.0), color);
            p.fillRect(QRectF(14.0, 9.0, 4.0, 14.0), color);
            p.fillRect(QRectF(22.0, 9.0, 4.0, 14.0), color);
            break;

        case SMToolIcons::eIcon::DistributeVertical:
            p.fillRect(QRectF(9.0, 6.0, 14.0, 4.0), color);
            p.fillRect(QRectF(9.0, 14.0, 14.0, 4.0), color);
            p.fillRect(QRectF(9.0, 22.0, 14.0, 4.0), color);
            break;

        case SMToolIcons::eIcon::ToggleSnap:
            p.drawLine(QPointF(16.0, 6.0), QPointF(16.0, 26.0));
            p.drawLine(QPointF(6.0, 16.0), QPointF(26.0, 16.0));
            p.setBrush(color);
            p.drawEllipse(QPointF(16.0, 16.0), 3.0, 3.0);
            break;

        case SMToolIcons::eIcon::ToggleGrid:
            p.drawRect(QRectF(6.0, 6.0, 20.0, 20.0));
            p.drawLine(QPointF(12.6, 6.0), QPointF(12.6, 26.0));
            p.drawLine(QPointF(19.3, 6.0), QPointF(19.3, 26.0));
            p.drawLine(QPointF(6.0, 12.6), QPointF(26.0, 12.6));
            p.drawLine(QPointF(6.0, 19.3), QPointF(26.0, 19.3));
            break;

        case SMToolIcons::eIcon::GridDots:
            p.setBrush(color);
            for (int r = 0; r < 3; ++r)
            {
                for (int c = 0; c < 3; ++c)
                {
                    p.drawEllipse(QPointF(9.0 + c * 7.0, 9.0 + r * 7.0), 1.8, 1.8);
                }
            }
            break;

        case SMToolIcons::eIcon::GridDotSize:
            p.setBrush(color);
            p.drawEllipse(QPointF(8.0, 21.0), 1.6, 1.6);
            p.drawEllipse(QPointF(16.0, 18.0), 2.6, 2.6);
            p.drawEllipse(QPointF(25.0, 14.0), 3.8, 3.8);
            break;

        case SMToolIcons::eIcon::GridSize:
            p.drawRect(QRectF(6.0, 6.0, 20.0, 20.0));
            p.drawLine(QPointF(16.0, 6.0), QPointF(16.0, 26.0));
            p.drawLine(QPointF(6.0, 16.0), QPointF(26.0, 16.0));
            arrowHead(p, QPointF(26.0, 26.0), QPointF(29.0, 29.0), 5.0, color);
            break;

        case SMToolIcons::eIcon::EnterSubmachine:
            p.drawRect(QRectF(7.0, 7.0, 18.0, 18.0));
            p.drawLine(QPointF(9.0, 9.0), QPointF(18.0, 18.0));
            arrowHead(p, QPointF(9.0, 9.0), QPointF(19.0, 19.0), 6.0, color);
            break;

        case SMToolIcons::eIcon::GoToParent:
            p.drawLine(QPointF(16.0, 25.0), QPointF(16.0, 9.0));
            arrowHead(p, QPointF(16.0, 25.0), QPointF(16.0, 7.0), 6.5, color);
            break;

        case SMToolIcons::eIcon::CenterMachine:
            // A tiny state box under a crosshair: bring the machine back into view.
            p.drawRoundedRect(QRectF(11.0, 12.5, 10.0, 7.0), 2.0, 2.0);
            p.drawLine(QPointF(16.0, 4.0), QPointF(16.0, 9.5));
            p.drawLine(QPointF(16.0, 22.5), QPointF(16.0, 28.0));
            p.drawLine(QPointF(4.0, 16.0), QPointF(8.0, 16.0));
            p.drawLine(QPointF(24.0, 16.0), QPointF(28.0, 16.0));
            break;

        case SMToolIcons::eIcon::ZoomIn:
        case SMToolIcons::eIcon::ZoomOut:
        case SMToolIcons::eIcon::ZoomReset:
        {
            const QPointF c(14.0, 14.0);
            const double rad = 8.0;
            p.drawEllipse(c, rad, rad);
            p.drawLine(QPointF(20.0, 20.0), QPointF(27.0, 27.0));
            if (kind == SMToolIcons::eIcon::ZoomIn)
            {
                p.drawLine(c + QPointF(-3.5, 0.0), c + QPointF(3.5, 0.0));
                p.drawLine(c + QPointF(0.0, -3.5), c + QPointF(0.0, 3.5));
            }
            else if (kind == SMToolIcons::eIcon::ZoomOut)
            {
                p.drawLine(c + QPointF(-3.5, 0.0), c + QPointF(3.5, 0.0));
            }
            else
            {
                p.setBrush(color);
                p.drawEllipse(c, 1.8, 1.8);
            }
            break;
        }

        case SMToolIcons::eIcon::ZoomFit:
        {
            const double x0 = 6.0, y0 = 8.0, x1 = 26.0, y1 = 24.0, e = 5.0;
            p.drawLine(QPointF(x0, y0), QPointF(x0 + e, y0)); p.drawLine(QPointF(x0, y0), QPointF(x0, y0 + e));
            p.drawLine(QPointF(x1, y0), QPointF(x1 - e, y0)); p.drawLine(QPointF(x1, y0), QPointF(x1, y0 + e));
            p.drawLine(QPointF(x0, y1), QPointF(x0 + e, y1)); p.drawLine(QPointF(x0, y1), QPointF(x0, y1 - e));
            p.drawLine(QPointF(x1, y1), QPointF(x1 - e, y1)); p.drawLine(QPointF(x1, y1), QPointF(x1, y1 - e));
            break;
        }

        case SMToolIcons::eIcon::Undo:
            p.drawArc(QRectF(8.0, 8.0, 18.0, 16.0), 40 * 16, 180 * 16);
            p.drawLine(QPointF(8.5, 12.0), QPointF(8.5, 18.0));
            arrowHead(p, QPointF(12.0, 14.0), QPointF(7.0, 18.5), 6.0, color);
            break;

        case SMToolIcons::eIcon::Redo:
            p.drawArc(QRectF(6.0, 8.0, 18.0, 16.0), 140 * 16, -180 * 16);
            arrowHead(p, QPointF(20.0, 14.0), QPointF(25.0, 18.5), 6.0, color);
            break;

        case SMToolIcons::eIcon::SelectAll:
        {
            QPen dashed{ color, 1.8 };
            dashed.setStyle(Qt::DashLine);
            p.setPen(dashed);
            p.drawRect(QRectF(6.0, 8.0, 20.0, 16.0));
            break;
        }

        case SMToolIcons::eIcon::Cut:
            // Scissors: crossed blades with the handle rings at the bottom.
            p.drawLine(QPointF(10.5, 7.0), QPointF(21.0, 22.0));
            p.drawLine(QPointF(21.5, 7.0), QPointF(11.0, 22.0));
            p.drawEllipse(QPointF(9.5, 24.5), 2.6, 2.6);
            p.drawEllipse(QPointF(22.5, 24.5), 2.6, 2.6);
            break;

        case SMToolIcons::eIcon::Copy:
            // Two offset pages.
            p.drawRect(QRectF(7.0, 6.0, 13.0, 15.0));
            p.drawRect(QRectF(12.0, 11.0, 13.0, 15.0));
            break;

        case SMToolIcons::eIcon::Paste:
            // A clipboard with its clip and a pasted page.
            p.drawRoundedRect(QRectF(7.0, 7.0, 15.0, 19.0), 2.0, 2.0);
            p.drawRect(QRectF(11.0, 4.5, 7.0, 4.0));
            p.drawRect(QRectF(14.0, 14.0, 11.0, 12.0));
            break;

        case SMToolIcons::eIcon::Duplicate:
            // Two offset pages plus the add marker.
            p.drawRect(QRectF(6.0, 6.0, 12.0, 14.0));
            p.drawRect(QRectF(11.0, 11.0, 12.0, 14.0));
            p.drawLine(QPointF(26.0, 20.0), QPointF(26.0, 28.0));
            p.drawLine(QPointF(22.0, 24.0), QPointF(30.0, 24.0));
            break;

        case SMToolIcons::eIcon::Declare:
            p.drawLine(QPointF(7.0, 9.0), QPointF(19.0, 9.0));
            p.drawLine(QPointF(7.0, 15.0), QPointF(19.0, 15.0));
            p.drawLine(QPointF(7.0, 21.0), QPointF(15.0, 21.0));
            p.drawLine(QPointF(23.0, 18.0), QPointF(23.0, 28.0));
            p.drawLine(QPointF(18.0, 23.0), QPointF(28.0, 23.0));
            break;

        case SMToolIcons::eIcon::NewTrigger:
        {
            // A filled lightning bolt: the stimulus that fires a transition.
            QPainterPath bolt;
            bolt.moveTo(18.0, 5.0);
            bolt.lineTo(9.0, 18.0);
            bolt.lineTo(14.5, 18.0);
            bolt.lineTo(13.0, 27.0);
            bolt.lineTo(23.0, 13.5);
            bolt.lineTo(17.0, 13.5);
            bolt.closeSubpath();
            p.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            p.setBrush(color);
            p.drawPath(bolt);
            break;
        }

        case SMToolIcons::eIcon::NewAction:
        {
            // A simple gear: the operation a state or transition runs.
            const QPointF c(16.0, 16.0);
            p.drawEllipse(c, 5.5, 5.5);
            constexpr double pi{ 3.14159265358979323846 };
            for (int i = 0; i < 8; ++i)
            {
                const double angle = i * pi / 4.0;
                const QPointF dir(std::cos(angle), std::sin(angle));
                p.drawLine(c + dir * 7.5, c + dir * 10.5);
            }
            break;
        }

        case SMToolIcons::eIcon::NewCondition:
            // The flowchart decision diamond.
            {
                QPainterPath diamond;
                diamond.moveTo(16.0, 6.0);
                diamond.lineTo(26.0, 16.0);
                diamond.lineTo(16.0, 26.0);
                diamond.lineTo(6.0, 16.0);
                diamond.closeSubpath();
                p.drawPath(diamond);
                p.setBrush(color);
                p.drawEllipse(QPointF(16.0, 16.0), 1.8, 1.8);
            }
            break;

        case SMToolIcons::eIcon::NewEvent:
        {
            // The event glyph is a lightning bolt, the same family as the trigger stimulus, shared by
            // every event surface (the Events page and the state-body event row). The square signal
            // pulse is kept below behind a compile-time switch so the look can be swapped while
            // experimenting: flip EVENT_USE_PULSE to true to render the pulse instead of the bolt.
            constexpr bool EVENT_USE_PULSE = false;
            if constexpr (EVENT_USE_PULSE)
            {
                // A square signal pulse.
                QPainterPath pulse;
                pulse.moveTo(5.0, 21.0);
                pulse.lineTo(11.0, 21.0);
                pulse.lineTo(11.0, 10.0);
                pulse.lineTo(20.0, 10.0);
                pulse.lineTo(20.0, 21.0);
                pulse.lineTo(27.0, 21.0);
                p.drawPath(pulse);
            }
            else
            {
                // A filled lightning bolt.
                QPainterPath bolt;
                bolt.moveTo(18.0, 5.0);
                bolt.lineTo(9.0, 18.0);
                bolt.lineTo(14.5, 18.0);
                bolt.lineTo(13.0, 27.0);
                bolt.lineTo(23.0, 13.5);
                bolt.lineTo(17.0, 13.5);
                bolt.closeSubpath();
                p.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                p.setBrush(color);
                p.drawPath(bolt);
            }
            break;
        }

        case SMToolIcons::eIcon::NewTimer:
        {
            // A clock face with hands.
            const QPointF c(16.0, 16.0);
            p.drawEllipse(c, 9.5, 9.5);
            p.drawLine(c, c + QPointF(0.0, -5.5));
            p.drawLine(c, c + QPointF(4.2, 2.2));
            break;
        }

        case SMToolIcons::eIcon::NewAttribute:
        {
            // A name tag with its hole: a named data field.
            QPainterPath tag;
            tag.moveTo(6.0, 9.5);
            tag.lineTo(17.5, 9.5);
            tag.lineTo(26.0, 16.0);
            tag.lineTo(17.5, 22.5);
            tag.lineTo(6.0, 22.5);
            tag.closeSubpath();
            p.drawPath(tag);
            p.setBrush(color);
            p.drawEllipse(QPointF(11.0, 16.0), 1.8, 1.8);
            break;
        }

        case SMToolIcons::eIcon::NewConstant:
            // A padlock: a fixed, immutable value.
            p.drawRoundedRect(QRectF(9.0, 15.0, 14.0, 11.0), 2.5, 2.5);
            p.drawArc(QRectF(11.5, 6.5, 9.0, 13.0), 0 * 16, 180 * 16);
            p.setBrush(color);
            p.drawEllipse(QPointF(16.0, 20.5), 1.8, 1.8);
            break;

        case SMToolIcons::eIcon::NewDataType:
        {
            // An isometric cube: a structured type.
            QPainterPath cube;
            cube.moveTo(16.0, 5.0);
            cube.lineTo(25.0, 10.0);
            cube.lineTo(25.0, 21.0);
            cube.lineTo(16.0, 26.0);
            cube.lineTo(7.0, 21.0);
            cube.lineTo(7.0, 10.0);
            cube.closeSubpath();
            p.drawPath(cube);
            p.drawLine(QPointF(7.0, 10.0), QPointF(16.0, 15.0));
            p.drawLine(QPointF(25.0, 10.0), QPointF(16.0, 15.0));
            p.drawLine(QPointF(16.0, 15.0), QPointF(16.0, 26.0));
            break;
        }

        case SMToolIcons::eIcon::GuardInsert:
            // A caret bar with a plus: drop a reference where the cursor is.
            p.drawLine(QPointF(11.0, 7.0), QPointF(11.0, 25.0));
            p.drawLine(QPointF(7.0, 7.0), QPointF(15.0, 7.0));
            p.drawLine(QPointF(7.0, 25.0), QPointF(15.0, 25.0));
            p.drawLine(QPointF(23.0, 11.0), QPointF(23.0, 21.0));
            p.drawLine(QPointF(18.0, 16.0), QPointF(28.0, 16.0));
            break;

        case SMToolIcons::eIcon::GuardPreview:
            // Angle brackets: generated source.
            p.drawPolyline(QPolygonF({ QPointF(12.0, 9.0), QPointF(5.0, 16.0), QPointF(12.0, 23.0) }));
            p.drawPolyline(QPolygonF({ QPointF(20.0, 9.0), QPointF(27.0, 16.0), QPointF(20.0, 23.0) }));
            break;

        case SMToolIcons::eIcon::GuardPopout:
            // A frame with an out-pointing arrow: the same guard, more room.
            p.drawRoundedRect(QRectF(5.0, 8.0, 15.0, 16.0), 3.0, 3.0);
            p.drawLine(QPointF(17.0, 15.0), QPointF(27.0, 5.0));
            arrowHead(p, QPointF(17.0, 15.0), QPointF(28.0, 4.0), 6.0, color);
            break;

        case SMToolIcons::eIcon::GuardClear:
            // A circled cross: back to "always fires".
            p.drawEllipse(QPointF(16.0, 16.0), 10.0, 10.0);
            p.drawLine(QPointF(12.0, 12.0), QPointF(20.0, 20.0));
            p.drawLine(QPointF(20.0, 12.0), QPointF(12.0, 20.0));
            break;

        case SMToolIcons::eIcon::GuardHelp:
        {
            p.drawEllipse(QPointF(16.0, 16.0), 10.0, 10.0);
            QPainterPath hook;
            hook.moveTo(12.5, 13.0);
            hook.cubicTo(12.5, 8.5, 20.0, 9.0, 19.0, 13.5);
            hook.cubicTo(18.3, 16.5, 16.0, 16.5, 16.0, 19.0);
            p.drawPath(hook);
            p.setBrush(color);
            p.drawEllipse(QPointF(16.0, 22.5), 1.4, 1.4);
            p.setBrush(Qt::NoBrush);
            break;
        }

        case SMToolIcons::eIcon::GuardCompact:
            // Two arrows folding towards one line: keep a single section open.
            p.drawLine(QPointF(6.0, 16.0), QPointF(26.0, 16.0));
            p.drawPolyline(QPolygonF({ QPointF(11.0, 7.0), QPointF(16.0, 12.0), QPointF(21.0, 7.0) }));
            p.drawPolyline(QPolygonF({ QPointF(11.0, 25.0), QPointF(16.0, 20.0), QPointF(21.0, 25.0) }));
            break;

        case SMToolIcons::eIcon::GuardConditions:
            // A checklist: the palette of defined condition methods.
            p.drawPolyline(QPolygonF({ QPointF(5.0, 10.0), QPointF(7.5, 12.5), QPointF(12.0, 8.0) }));
            p.drawPolyline(QPolygonF({ QPointF(5.0, 22.0), QPointF(7.5, 24.5), QPointF(12.0, 20.0) }));
            p.drawLine(QPointF(16.0, 11.0), QPointF(27.0, 11.0));
            p.drawLine(QPointF(16.0, 23.0), QPointF(27.0, 23.0));
            break;

        case SMToolIcons::eIcon::GuardArguments:
            // A two-column table: formal -> source.
            p.drawRoundedRect(QRectF(5.0, 8.0, 22.0, 16.0), 2.5, 2.5);
            p.drawLine(QPointF(15.0, 8.0), QPointF(15.0, 24.0));
            p.drawLine(QPointF(5.0, 16.0), QPointF(27.0, 16.0));
            break;

        case SMToolIcons::eIcon::GuardData:
            // A stack of browsable entries with a lens over it: the symbol catalog.
            p.drawLine(QPointF(5.0, 9.0), QPointF(21.0, 9.0));
            p.drawLine(QPointF(5.0, 15.0), QPointF(17.0, 15.0));
            p.drawLine(QPointF(5.0, 21.0), QPointF(21.0, 21.0));
            p.drawEllipse(QPointF(21.0, 19.0), 5.5, 5.5);
            p.drawLine(QPointF(25.0, 23.0), QPointF(28.0, 26.0));
            break;

        case SMToolIcons::eIcon::SectionDetails:
            // A labelled form: a short label and a longer field on each of three rows.
            p.drawLine(QPointF(5.0, 10.0), QPointF(11.0, 10.0));
            p.drawLine(QPointF(14.0, 10.0), QPointF(27.0, 10.0));
            p.drawLine(QPointF(5.0, 16.0), QPointF(11.0, 16.0));
            p.drawLine(QPointF(14.0, 16.0), QPointF(27.0, 16.0));
            p.drawLine(QPointF(5.0, 22.0), QPointF(11.0, 22.0));
            p.drawLine(QPointF(14.0, 22.0), QPointF(27.0, 22.0));
            break;

        case SMToolIcons::eIcon::SectionList:
            // Two states joined by an arrow: the transitions list.
            p.drawEllipse(QPointF(8.0, 16.0), 3.5, 3.5);
            p.drawEllipse(QPointF(24.0, 16.0), 3.5, 3.5);
            p.drawLine(QPointF(12.0, 16.0), QPointF(19.5, 16.0));
            arrowHead(p, QPointF(12.0, 16.0), QPointF(21.0, 16.0), 5.0, color);
            break;

        case SMToolIcons::eIcon::SectionText:
            // Paragraph lines: a free-text description.
            p.drawLine(QPointF(5.0, 10.0), QPointF(27.0, 10.0));
            p.drawLine(QPointF(5.0, 16.0), QPointF(27.0, 16.0));
            p.drawLine(QPointF(5.0, 22.0), QPointF(19.0, 22.0));
            break;

        case SMToolIcons::eIcon::SectionEnter:
            // An arrow entering a box from the left: work done on entry.
            p.drawRoundedRect(QRectF(15.0, 7.0, 11.0, 18.0), 2.5, 2.5);
            p.drawLine(QPointF(5.0, 16.0), QPointF(15.0, 16.0));
            arrowHead(p, QPointF(6.0, 16.0), QPointF(16.0, 16.0), 5.5, color);
            break;

        case SMToolIcons::eIcon::SectionExit:
            // An arrow leaving a box to the right: work done on exit.
            p.drawRoundedRect(QRectF(6.0, 7.0, 11.0, 18.0), 2.5, 2.5);
            p.drawLine(QPointF(17.0, 16.0), QPointF(27.0, 16.0));
            arrowHead(p, QPointF(17.0, 16.0), QPointF(28.0, 16.0), 5.5, color);
            break;

        case SMToolIcons::eIcon::SectionDo:
            // A circular arrow: actions repeated while the state is active.
            p.drawArc(QRectF(8.0, 8.0, 16.0, 16.0), 300 * 16, 300 * 16);
            arrowHead(p, QPointF(15.0, 8.5), QPointF(22.0, 10.0), 5.0, color);
            break;
        }
    }
}

QIcon SMToolIcons::icon(eIcon kind)
{
    const QColor color = QApplication::palette().color(QPalette::WindowText);

    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    drawGlyph(painter, kind, color);
    painter.end();

    return QIcon(pixmap);
}
