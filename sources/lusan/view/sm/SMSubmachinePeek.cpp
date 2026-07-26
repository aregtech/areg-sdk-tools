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
 *  \file        lusan/view/sm/SMSubmachinePeek.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \brief       Lusan application, FSM design canvas submachine quick view.
 *
 ************************************************************************/

#include "lusan/view/sm/SMSubmachinePeek.hpp"

#include "lusan/view/sm/NESMDesign.hpp"

#include <QFont>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QScreen>

#include <algorithm>

namespace
{
    constexpr int       CursorGap   { 16 };     //!< Distance kept from the pointer.
    constexpr double    ContentPad  { 8.0 };    //!< Inner margin around the drawn shapes.
    constexpr double    TitleHeight { 18.0 };   //!< The band the title and the count occupy.
    constexpr double    MinShapeSide{ 2.0 };    //!< A shape never scales below this: it must stay visible.
}

SMSubmachinePeek::SMSubmachinePeek(QWidget* parent /*= nullptr*/)
    : QWidget   (parent)
    , mTitle    ( )
    , mShapes   ( )
    , mTotal    (0)
{
    setObjectName(QStringLiteral("smSubmachinePeek"));
    // A tooltip window, not a tool window: it must never take focus or activation away from the
    // canvas, because the gesture that opens it is a hover with two modifiers still held down.
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFixedSize(PeekWidth, PeekHeight);
}

void SMSubmachinePeek::showFor(const QString& title, const QList<Shape>& shapes, int total, const QPoint& globalPos)
{
    mTitle  = title;
    mTotal  = total;
    mShapes = (shapes.size() > MaxShapes) ? shapes.mid(0, MaxShapes) : shapes;

    QPoint target(globalPos.x() + CursorGap, globalPos.y() + CursorGap);
    const QScreen* screen = QGuiApplication::screenAt(globalPos);
    if (screen == nullptr)
    {
        screen = QGuiApplication::primaryScreen();
    }

    if (screen != nullptr)
    {
        // Flip to the other side of the pointer rather than sliding along the edge: a popup that
        // slides ends up UNDER the pointer, and the pointer is what is holding it open.
        const QRect bounds = screen->availableGeometry();
        if ((target.x() + width()) > (bounds.x() + bounds.width()))
        {
            target.setX(globalPos.x() - CursorGap - width());
        }
        if ((target.y() + height()) > (bounds.y() + bounds.height()))
        {
            target.setY(globalPos.y() - CursorGap - height());
        }

        target.setX(std::clamp(target.x(), bounds.left(), std::max(bounds.left(), bounds.right() - width())));
        target.setY(std::clamp(target.y(), bounds.top(), std::max(bounds.top(), bounds.bottom() - height())));
    }

    move(target);
    update();
    show();
    raise();
}

void SMSubmachinePeek::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QPalette pal = palette();
    const QRectF   frame{ 0.5, 0.5, static_cast<double>(width()) - 1.0, static_cast<double>(height()) - 1.0 };
    painter.setPen(QPen(NESMDesign::stateBorderColor(pal), 1.0));
    painter.setBrush(pal.color(QPalette::ToolTipBase));
    painter.drawRoundedRect(frame, 4.0, 4.0);

    const QColor textColor = pal.color(QPalette::ToolTipText);
    QFont titleFont = font();
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(textColor);

    const QRectF titleRect{ ContentPad, 3.0, frame.width() - (2.0 * ContentPad), TitleHeight };
    const QFontMetricsF titleMetrics{ titleFont };
    painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter
                    , titleMetrics.elidedText(mTitle, Qt::ElideRight, titleRect.width()));

    QFont countFont = font();
    countFont.setPointSizeF(std::max(countFont.pointSizeF() * 0.85, 6.5));
    painter.setFont(countFont);
    QColor countColor = textColor;
    countColor.setAlphaF(0.7);
    painter.setPen(countColor);
    const QRectF countRect{ ContentPad, frame.height() - TitleHeight - 3.0
                          , frame.width() - (2.0 * ContentPad), TitleHeight };
    // The one thing a fixed-size silhouette cannot show is how much of it there is.
    const QString count = (mShapes.size() < mTotal)
                          ? tr("%1 states, %2 shown").arg(mTotal).arg(mShapes.size())
                          : tr("%1 states").arg(mTotal);
    painter.drawText(countRect, Qt::AlignLeft | Qt::AlignVCenter, count);

    if (mShapes.isEmpty())
    {
        return;
    }

    QRectF bounds = mShapes.first().rect;
    for (const Shape& shape : mShapes)
    {
        bounds = bounds.united(shape.rect);
    }

    if ((bounds.width() <= 0.0) || (bounds.height() <= 0.0))
    {
        return;
    }

    const QRectF canvas{ ContentPad, TitleHeight + 5.0
                       , frame.width() - (2.0 * ContentPad)
                       , frame.height() - (2.0 * TitleHeight) - 10.0 };
    // One scale for both axes: the level keeps its real proportions, which is the whole basis of
    // recognizing it by shape. Fitting each axis separately would stretch a tall level flat.
    const double scale = std::min(canvas.width() / bounds.width(), canvas.height() / bounds.height());
    const QPointF origin{ canvas.center().x() - (bounds.width()  * scale / 2.0)
                        , canvas.center().y() - (bounds.height() * scale / 2.0) };

    painter.setBrush(Qt::NoBrush);
    for (const Shape& shape : mShapes)
    {
        const QRectF box{ origin.x() + ((shape.rect.x() - bounds.x()) * scale)
                        , origin.y() + ((shape.rect.y() - bounds.y()) * scale)
                        , std::max(shape.rect.width()  * scale, MinShapeSide)
                        , std::max(shape.rect.height() * scale, MinShapeSide) };

        const bool marker = (shape.kind != SMStateEntry::eStateKind::Normal);
        painter.setPen(QPen(marker ? NESMDesign::contrastTextColor(pal.color(QPalette::ToolTipBase)) : textColor
                           , marker ? 1.3 : 1.0));
        if (shape.kind == SMStateEntry::eStateKind::Start)
        {
            painter.drawEllipse(box);
        }
        else if (shape.kind == SMStateEntry::eStateKind::Final)
        {
            // The canvas draws a Final as the classic double border; the silhouette keeps that,
            // because a level's end marker is one of the landmarks the eye navigates by.
            painter.drawEllipse(box);
            const QRectF inner = box.adjusted(1.5, 1.5, -1.5, -1.5);
            if ((inner.width() > 0.0) && (inner.height() > 0.0))
            {
                painter.drawEllipse(inner);
            }
        }
        else
        {
            painter.drawRoundedRect(box, 2.0, 2.0);
        }
    }
}
