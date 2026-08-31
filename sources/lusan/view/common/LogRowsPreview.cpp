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
 *  \file        lusan/view/common/LogRowsPreview.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the sample log rows shown in the settings.
 *
 ************************************************************************/

#include "lusan/view/common/LogRowsPreview.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/OptionsManager.hpp"

#include <QApplication>
#include <QFontDatabase>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>

namespace
{
    //!< One sample row: the role it is drawn in, the word in the priority cell, the mark it
    //!< carries in the gutter, and the message text.
    struct sSampleRow
    {
        NELogPalette::eLogColorRole role;
        const char*                 priority;
        const char*                 mark;
        const char*                 message;
    };

    const sSampleRow _rows[]
    {
          { NELogPalette::eLogColorRole::RoleFatal      , "FATAL", "", "Service < TrafficLightService > terminated, connection lost" }
        , { NELogPalette::eLogColorRole::RoleError      , "ERROR", "", "Failed to send event to < TrafficLight >: target not connected" }
        , { NELogPalette::eLogColorRole::RoleWarning    , "WARN" , "", "Timer < TrafficLightTimer > restarted late by 12 ms" }
        , { NELogPalette::eLogColorRole::RoleInformation, "INFO" , "", "Log observer registered 136 scopes of instance 256" }
        , { NELogPalette::eLogColorRole::RoleScopeEnter , "Enter", "enter", "TrafficLightFSM.on_timer_YellowGreen" }
        , { NELogPalette::eLogColorRole::RoleDebug      , "DEBUG", "", "FSM < TrafficLight >: dispatching Timer signal" }
        , { NELogPalette::eLogColorRole::RoleScopeExit  , "Exit" , "exit" , "TrafficLightFSM.on_timer_YellowGreen" }
    };

    constexpr int _rowCount { static_cast<int>(sizeof(_rows) / sizeof(_rows[0])) };
    constexpr int _gutter   { 24 };
    constexpr int _markLeft {  6 };
    constexpr int _markSide { 11 };
    constexpr int _prioWidth{ 58 };
    constexpr int _inset    {  2 };
}

LogRowsPreview::LogRowsPreview(QWidget* parent /*= nullptr*/)
    : QWidget   (parent)
    , mPalette  (NELogPalette::DefaultPalette)
    , mRowHeight(OptionsManager::LogRowHeightDefault)
{
    QFont fixed{ QFontDatabase::systemFont(QFontDatabase::FixedFont) };
    fixed.setPointSizeF(font().pointSizeF());
    setFont(fixed);
}

void LogRowsPreview::setSample(NELogPalette::eLogPalette palette, int height)
{
    if ((palette == mPalette) && (height == mRowHeight))
        return;

    mPalette   = palette;
    mRowHeight = height;
    updateGeometry();
    update();
}

QSize LogRowsPreview::sizeHint(void) const
{
    return QSize(420, (mRowHeight * _rowCount) + (_inset * 2));
}

void LogRowsPreview::paintEvent(QPaintEvent* event)
{
    const QPalette theme{ QApplication::palette() };
    const bool dark{ theme.color(QPalette::ColorRole::Base).lightness() < 128 };

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing, false);
    painter.fillRect(event->rect(), theme.color(QPalette::ColorRole::Base));

    const QIcon markEnter{ NELusanCommon::iconScopeEnter(QSize(_markSide, _markSide)) };
    const QIcon markExit { NELusanCommon::iconScopeExit (QSize(_markSide, _markSide)) };

    int top{ _inset };
    for (int i = 0; i < _rowCount; ++i)
    {
        const sSampleRow& row{ _rows[i] };
        const QRect box(0, top, width(), mRowHeight);

        const QColor band{ NELogPalette::rowBackgroundOf(row.role, dark) };
        if (band.alpha() != 0)
        {
            painter.fillRect(box, band);
        }
        else if ((i % 2) != 0)
        {
            painter.fillRect(box, theme.color(QPalette::ColorRole::AlternateBase));
        }

        // Error and Fatal carry a heavier rail, so the two loud rows are told apart with
        // the colour taken away.
        const bool heavy{ (row.role == NELogPalette::eLogColorRole::RoleFatal)
                          || (row.role == NELogPalette::eLogColorRole::RoleError) };
        const QColor rail{ NELogPalette::railColorOf(row.role, dark) };
        if (rail.alpha() != 0)
        {
            painter.fillRect(QRect(_inset, top + 1, heavy ? 4 : 3, mRowHeight - 2), rail);
        }

        const QString mark{ QString::fromLatin1(row.mark) };
        if (mark.isEmpty() == false)
        {
            const int side{ qMin(_markSide, mRowHeight - 2) };
            if (side > 0)
            {
                const QRect glyph(_markLeft, top + ((mRowHeight - side) / 2), side, side);
                (mark == QLatin1String("enter") ? markEnter : markExit).paint(&painter, glyph);
            }
        }

        painter.setPen(NELogPalette::textColorOf(row.role, mPalette, dark));
        painter.drawText( QRect(_gutter, top, _prioWidth, mRowHeight)
                        , Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter
                        , QString::fromLatin1(row.priority));
        painter.drawText( QRect(_gutter + _prioWidth, top, width() - _gutter - _prioWidth - _inset, mRowHeight)
                        , Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter
                        , QString::fromLatin1(row.message));

        top += mRowHeight;
    }
}
