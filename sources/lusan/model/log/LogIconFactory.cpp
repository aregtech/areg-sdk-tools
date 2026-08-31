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
 *  \file        lusan/model/log/LogIconFactory.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log scopes icons.
 *
 ************************************************************************/
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/common/NELogPalette.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "areg/logging/areg_log.h"

#include <QApplication>
#include <QGuiApplication>
#include <QColor>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPen>
#include <QPixmap>
#include <QScreen>
#include <QMap>

namespace
{
    constexpr uint32_t LogActive{ 0x00FF0000u };

    QMap<uint32_t, QIcon>   _logIcons;
    QMap<uint32_t, QIcon>   _chargers;

    constexpr int32_t  AlphaMixed   { 75 };

    /************************************************************************
     * The charger geometry, on the 24 grid, with its ink box at 20 x 20.
     ************************************************************************/

    constexpr qreal Grid        { 24.0 };   //!< The side of the coordinate box every number below is in.
    constexpr qreal CellX       {  6.4 };   //!< The left edge of a cell.
    constexpr qreal CellWidth   { 11.2 };   //!< The width of a cell.
    constexpr qreal CellHeight  {  4.28};   //!< The height of a cell.
    constexpr qreal CellRadius  {  1.2 };   //!< The corner of a cell.
    constexpr qreal CellBottom  { 17.72};   //!< The top edge of the lowest cell.
    constexpr qreal CellStep    {  5.24};   //!< The distance from one cell to the next.
    constexpr qreal HollowInset {  0.48};   //!< How far the outline of a hollow cell sits inside the cell.
    constexpr qreal LineWidth   {  1.7 };   //!< The stroke of the brackets, the family value.
    constexpr qreal GhostWidth  {  1.0 };   //!< The stroke of a cell nothing below generates.
    constexpr qreal HollowWidth {  1.3 };   //!< The stroke of a cell the scopes below disagree about.
    constexpr qreal DashOn      {  3.3 };   //!< The drawn part of an interrupted bracket.
    constexpr qreal DashOff     {  2.5 };   //!< The gap of an interrupted bracket.
    constexpr qreal BracketLeft {  2.0 };   //!< The outer edge of the left bracket.
    constexpr qreal BracketRight{ 22.0 };   //!< The outer edge of the right bracket.
    constexpr qreal BracketArm  {  4.37};   //!< Where the arm of the left bracket ends.
    constexpr qreal BracketTop  {  3.66};   //!< The top arm of a bracket.
    constexpr qreal BracketBottom{20.34};   //!< The bottom arm of a bracket.
    constexpr qreal CornerTop   {  7.08};   //!< Where the top corner stops when the scope lines are off.
    constexpr qreal CornerBottom{ 16.92};   //!< Where the bottom corner starts when the scope lines are off.

    //! The palette role of the four cells, lowest cell first.
    const NELogPalette::eLogColorRole _cellRoles[LogIconFactory::ChargerCells]
    {
          NELogPalette::eLogColorRole::RoleError
        , NELogPalette::eLogColorRole::RoleWarning
        , NELogPalette::eLogColorRole::RoleInformation
        , NELogPalette::eLogColorRole::RoleDebug
    };

    //! The rectangle of the cell with the given index, lowest cell first.
    inline QRectF cellRect(int cell)
    {
        return QRectF(CellX, CellBottom - (CellStep * cell), CellWidth, CellHeight);
    }

    //! The ink the charger falls back to when it carries no priority of its own.
    inline QColor neutralInk(bool frozen)
    {
        const QPalette palette{ QApplication::palette() };
        return palette.color(frozen ? QPalette::Disabled : QPalette::Active, QPalette::Text);
    }

    QIcon _createNotSetIcon(uint32_t pixels)
    {
        pixels = pixels == 0 ? LogIconFactory::IconPixels : pixels;
        QPixmap pixmap(pixels, pixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::NoBrush);
        
        painter.end();
        return QIcon(pixmap);
    }

    QIcon _mergeIcons(QColor color, const QIcon& icon, uint32_t pixels)
    {
        pixels = static_cast<uint32_t>(pixels * 1.0f);
        QPixmap result(pixels, pixels);
        result.fill(Qt::transparent);

        color.setAlpha(AlphaMixed);
        QPainter painter(&result);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(QBrush(color));
        QPainterPath circle;
        circle.addEllipse(0, 0, pixels, pixels);
        painter.setClipPath(circle);
        painter.drawEllipse(0, 0, pixels, pixels);
        painter.setClipping(false);

        int overlaySize =  static_cast<int>(pixels * 0.7);
        int offset = (pixels - overlaySize) / 2;
        QPixmap pix   = icon.pixmap(overlaySize, overlaySize);
        painter.drawPixmap(offset, offset, pix);

        painter.end();
        return QIcon(result);
    }
    
    //! Drops both icon caches when the theme has changed, so no icon keeps the tint of the previous theme.
    void _dropOnThemeChange(void)
    {
        static bool builtForDark{ false };
        static bool built       { false };

        const bool isDark{ NELogPalette::isDarkTheme() };
        if (built && (builtForDark == isDark))
            return;

        _logIcons.clear();
        _chargers.clear();
        builtForDark = isDark;
        built        = true;
    }

    void _initLogIcons(uint32_t pixels)
    {
        _dropOnThemeChange();
        if (_logIcons.isEmpty() == false)
            return;

        uint32_t prio = static_cast<int>(LogIconFactory::eLogIcons::PrioNotset);
        QIcon notset { _createNotSetIcon(pixels) };
        _logIcons[prio] = notset;
        _logIcons[prio | LogActive] = notset;

        prio = static_cast<int>(LogIconFactory::eLogIcons::PrioScope);
        QIcon scope(NELusanCommon::iconLogScope(NELusanCommon::SizeSmall));
        _logIcons[prio] = scope;
        _logIcons[prio | LogActive] = _mergeIcons(NELogPalette::railColor(NELogPalette::eLogColorRole::RoleScope), scope, pixels);

        prio = static_cast<int>(LogIconFactory::eLogIcons::PrioDebug);
        QIcon debug(NELusanCommon::iconLogDebug(NELusanCommon::SizeSmall));
        _logIcons[prio] = debug;
        _logIcons[prio | LogActive] = _mergeIcons(NELogPalette::railColor(NELogPalette::eLogColorRole::RoleDebug), debug, pixels);

        prio = static_cast<int>(LogIconFactory::eLogIcons::PrioInfo);
        QIcon info(NELusanCommon::iconLogInfo(NELusanCommon::SizeSmall));
        _logIcons[prio] = info;
        _logIcons[prio | LogActive] = _mergeIcons(NELogPalette::railColor(NELogPalette::eLogColorRole::RoleInformation), info, pixels);

        prio = static_cast<int>(LogIconFactory::eLogIcons::PrioWarn);
        QIcon warn(NELusanCommon::iconLogWarning(NELusanCommon::SizeSmall));
        _logIcons[prio] = warn;
        _logIcons[prio | LogActive] = _mergeIcons(NELogPalette::railColor(NELogPalette::eLogColorRole::RoleWarning), warn, pixels);

        prio = static_cast<int>(LogIconFactory::eLogIcons::PrioError);
        QIcon error(NELusanCommon::iconLogError(NELusanCommon::SizeSmall));
        _logIcons[prio] = error;
        _logIcons[prio | LogActive] = _mergeIcons(NELogPalette::railColor(NELogPalette::eLogColorRole::RoleError), error, pixels);

        prio = static_cast<int>(LogIconFactory::eLogIcons::PrioFatal);
        QIcon fatal(NELusanCommon::iconLogFatal(NELusanCommon::SizeSmall));
        _logIcons[prio] = fatal;
        _logIcons[prio | LogActive] = _mergeIcons(NELogPalette::railColor(NELogPalette::eLogColorRole::RoleFatal), fatal, pixels);

        prio = static_cast<int>(LogIconFactory::eLogIcons::PrioScopeEnter);
        QIcon enter(NELusanCommon::iconScopeEnter(NELusanCommon::SizeSmall));
        _logIcons[prio] = enter;
        _logIcons[prio | LogActive] = _mergeIcons(NELogPalette::railColor(NELogPalette::eLogColorRole::RoleScope), enter, pixels);

        prio = static_cast<int>(LogIconFactory::eLogIcons::PrioScopeExit);
        QIcon exit(NELusanCommon::iconScopeExit(NELusanCommon::SizeSmall));
        _logIcons[prio] = exit;
        _logIcons[prio | LogActive] = _mergeIcons(NELogPalette::railColor(NELogPalette::eLogColorRole::RoleScope), exit, pixels);
    }

}


QIcon LogIconFactory::getLogIcon(LogIconFactory::eLogIcons prio, bool active, uint32_t pixels)
{
    _initLogIcons(pixels);
    uint32_t key  = active ? static_cast<uint32_t>(prio) | LogActive : static_cast<uint32_t>(prio);
    Q_ASSERT(_logIcons.contains(key));
    return _logIcons[key];
}

void LogIconFactory::paintCharger(QPainter & painter, const QRectF & box, const LogIconFactory::sCharger & charger)
{
    const qreal extent{ qMin(box.width(), box.height()) };
    if (extent <= 0.0)
        return;

    const QColor neutral{ neutralInk(charger.frozen) };
    QColor ghost{ neutral };
    ghost.setAlphaF(NELogPalette::opacity(NELogPalette::eLogOpacity::OpacityGhost));

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(box.center().x() - (extent / 2.0), box.center().y() - (extent / 2.0));
    painter.scale(extent / Grid, extent / Grid);

    for (int cell = 0; cell < LogIconFactory::ChargerCells; ++cell)
    {
        const QRectF rect{ cellRect(cell) };
        const QColor hue { charger.frozen ? neutral : NELogPalette::railColor(_cellRoles[cell]) };

        if ((charger.mixed & (1u << cell)) != 0)
        {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(hue, HollowWidth));
            painter.drawRoundedRect( rect.adjusted(HollowInset, HollowInset, -HollowInset, -HollowInset)
                                   , CellRadius - HollowInset, CellRadius - HollowInset);
        }
        else if (charger.level > static_cast<uint8_t>(cell))
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(hue);
            painter.drawRoundedRect(rect, CellRadius, CellRadius);
        }
        else
        {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(ghost, GhostWidth));
            painter.drawRoundedRect(rect, CellRadius, CellRadius);
        }
    }

    QPainterPath path;
    if (charger.lines == LogIconFactory::eScopeLines::LinesOff)
    {
        path.moveTo(BracketArm  , BracketTop   ); path.lineTo(BracketLeft , BracketTop   ); path.lineTo(BracketLeft , CornerTop   );
        path.moveTo(BracketLeft , CornerBottom ); path.lineTo(BracketLeft , BracketBottom); path.lineTo(BracketArm  , BracketBottom);
        path.moveTo(Grid - BracketArm , BracketTop   ); path.lineTo(BracketRight, BracketTop   ); path.lineTo(BracketRight, CornerTop);
        path.moveTo(BracketRight, CornerBottom ); path.lineTo(BracketRight, BracketBottom); path.lineTo(Grid - BracketArm, BracketBottom);
    }
    else
    {
        path.moveTo(BracketArm , BracketTop); path.lineTo(BracketLeft , BracketTop); path.lineTo(BracketLeft , BracketBottom); path.lineTo(BracketArm , BracketBottom);
        path.moveTo(Grid - BracketArm, BracketTop); path.lineTo(BracketRight, BracketTop); path.lineTo(BracketRight, BracketBottom); path.lineTo(Grid - BracketArm, BracketBottom);
    }

    QPen pen;
    if (charger.lines == LogIconFactory::eScopeLines::LinesOff)
    {
        pen = QPen(ghost, LineWidth);
    }
    else
    {
        pen = QPen(charger.frozen ? neutral : NELogPalette::railColor(NELogPalette::eLogColorRole::RoleScope), LineWidth);
    }

    pen.setJoinStyle(Qt::RoundJoin);
    if (charger.lines == LogIconFactory::eScopeLines::LinesPartial)
    {
        // A dash pattern is measured in pen widths, and a round cap would blob the short dashes shut.
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern(QList<qreal>{ DashOn / LineWidth, DashOff / LineWidth });
    }
    else
    {
        pen.setCapStyle(Qt::RoundCap);
    }

    painter.setBrush(Qt::NoBrush);
    painter.setPen(pen);
    painter.drawPath(path);
    painter.restore();
}

QIcon LogIconFactory::chargerIcon(const LogIconFactory::sCharger & charger, uint32_t pixels)
{
    pixels = (pixels == 0) ? LogIconFactory::IconPixels : pixels;
    _dropOnThemeChange();

    const uint32_t key{ static_cast<uint32_t>(charger.level & 0x07u)
                      | (static_cast<uint32_t>(charger.mixed & 0x0Fu) << 3)
                      | (static_cast<uint32_t>(charger.lines) << 7)
                      | (charger.frozen ? (1u << 9) : 0u)
                      | (pixels << 10) };

    const auto found = _chargers.constFind(key);
    if (found != _chargers.constEnd())
        return found.value();

    const QScreen* screen{ QGuiApplication::primaryScreen() };
    const qreal    ratio { screen != nullptr ? screen->devicePixelRatio() : 1.0 };

    QPixmap pixmap(QSize(static_cast<int>(pixels * ratio), static_cast<int>(pixels * ratio)));
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(ratio);

    QPainter painter(&pixmap);
    LogIconFactory::paintCharger(painter, QRectF(0.0, 0.0, pixels, pixels), charger);
    painter.end();

    const QIcon icon{ pixmap };
    _chargers.insert(key, icon);
    return icon;
}

LogIconFactory::sCharger LogIconFactory::chargerOf(uint32_t scopePrio)
{
    const uint32_t prio{ scopePrio & static_cast<uint32_t>(areg::LogPriority::PrioValidLogs) };

    LogIconFactory::sCharger charger;
    charger.mixed  = 0u;
    charger.frozen = false;
    charger.lines  = (prio & static_cast<uint32_t>(areg::LogPriority::PrioScope)) != 0
                        ? LogIconFactory::eScopeLines::LinesOn
                        : LogIconFactory::eScopeLines::LinesOff;

    if ((prio & static_cast<uint32_t>(areg::LogPriority::PrioDebug)) != 0)
        charger.level = 4u;
    else if ((prio & static_cast<uint32_t>(areg::LogPriority::PrioInfo)) != 0)
        charger.level = 3u;
    else if ((prio & static_cast<uint32_t>(areg::LogPriority::PrioWarning)) != 0)
        charger.level = 2u;
    else if ((prio & (static_cast<uint32_t>(areg::LogPriority::PrioError) | static_cast<uint32_t>(areg::LogPriority::PrioFatal))) != 0)
        charger.level = 1u;
    else
        charger.level = 0u;

    return charger;
}

LogIconFactory::sCharger LogIconFactory::chargerOfRange(uint8_t levelLow, uint8_t levelHigh, LogIconFactory::eScopeLines lines)
{
    const uint8_t high{ qMin(levelHigh, static_cast<uint8_t>(LogIconFactory::ChargerCells)) };
    const uint8_t low { qMin(levelLow , high) };

    LogIconFactory::sCharger charger;
    charger.level  = high;
    charger.lines  = lines;
    charger.frozen = false;
    charger.mixed  = 0u;

    // A cell the lowest level does not reach but the highest does is one the scopes disagree about.
    for (uint8_t cell = low; cell < high; ++cell)
    {
        charger.mixed |= static_cast<uint8_t>(1u << cell);
    }

    return charger;
}

QIcon LogIconFactory::getIcon(uint32_t scopePrio, uint32_t pixels)
{
    return LogIconFactory::chargerIcon(LogIconFactory::chargerOf(scopePrio), pixels);
}

