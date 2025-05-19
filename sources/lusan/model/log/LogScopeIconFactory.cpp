/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/model/log/LogScopeIconFactory.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log scopes icons.
 *
 ************************************************************************/
#include "lusan/model/log/LogScopeIconFactory.hpp"
#include "areg/logging/NELogging.hpp"
#include <atomic>

#include <QColor>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QMap>

namespace
{
    enum ePrioColor
    {
          ColotNotSet   = 0
        , ColorScope
        , ColorFatal
        , ColorError
        , ColorWarn
        , ColorInfo
        , ColorDebug
        , ColoCount
    };

    const QColor _colors[static_cast<int>(ColoCount)] =
    {
          QColor(Qt::transparent)   // ColorNotSet
        , QColor(70, 70, 70)        // ColorScope
        , QColor(Qt::darkRed)       // ColorFatal
        , QColor(Qt::magenta)       // ColorError
        , QColor(Qt::blue)          // ColorWarn
        , QColor(Qt::darkCyan)      // ColorInfo
        , QColor(Qt::darkGreen)     // ColorDebug
    };
    
    std::atomic_bool _initialized   { false };
    QMap<uint32_t, QIcon> _mapIcons;

    constexpr int IconBegin         { 0 };
    constexpr int IconEnd           { LogScopeIconFactory::IconPixels - IconBegin };
    constexpr int IconMiddle        { (IconEnd - IconBegin) / 2 };
    constexpr uint32_t RoundBits    { 0x80000000u };

    QIcon _createNotSetIcon(void)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::NoBrush);
        
        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createScopeIcon(const QColor& color)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(color);
        painter.setBrush(Qt::NoBrush);
        QPolygon polyScope1, polyScope2;
        polyScope1 << QPoint( IconBegin,  IconBegin) << QPoint(IconEnd, IconEnd);
        polyScope2 << QPoint(IconEnd,  IconBegin) << QPoint( IconBegin, IconEnd);
        painter.drawLines(polyScope1);
        painter.drawLines(polyScope2);
        
        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createOneIcon(const QColor & color)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawRect(IconBegin, IconBegin, IconEnd, IconEnd);
        
        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createTwoIcon(const QColor& color1, const QColor& color2)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);

        // Top-left triangle
        QPolygon poly2;
        poly2 << QPoint(IconBegin, IconBegin) << QPoint(IconEnd, IconBegin) << QPoint(IconBegin, IconEnd);
        painter.setBrush(color2);
        painter.drawPolygon(poly2);

        // Bottom-right triangle
        QPolygon poly1;
        poly1 << QPoint(IconEnd, IconEnd) << QPoint(IconEnd, IconBegin) << QPoint(IconBegin, IconEnd);
        painter.setBrush(color1);
        painter.drawPolygon(poly1);
        
        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createFourIcon(const QColor& color1, const QColor& color2, const QColor& color3, const QColor& color4)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);

        // Top-left triangle
        QPolygon poly4;
        poly4 << QPoint(IconBegin, IconBegin) << QPoint(IconMiddle, IconBegin) << QPoint(IconBegin, IconMiddle);
        painter.setBrush(color4);
        painter.drawPolygon(poly4);
        
        // Top-right triangle
        QPolygon poly3;
        poly3 << QPoint(IconBegin, IconBegin) << QPoint(IconEnd, IconBegin) << QPoint(IconMiddle, IconMiddle);
        painter.setBrush(color3);
        painter.drawPolygon(poly3);
        
        // Bottom-left triangle
        QPolygon poly2;
        poly2 << QPoint(IconEnd, IconBegin) << QPoint(IconEnd, IconEnd) << QPoint(IconMiddle, IconMiddle);
        painter.setBrush(color2);
        painter.drawPolygon(poly2);
        
        // Bottom-right triangle
        QPolygon poly1;
        poly1 << QPoint(IconEnd, IconEnd) << QPoint(IconBegin, IconEnd) << QPoint(IconMiddle, IconMiddle);
        painter.setBrush(color1);
        painter.drawPolygon(poly1);
        
        painter.end();
        return QIcon(pixmap);
    }
    
    QIcon _setScope(const QIcon& icon, const QColor& color)
    {
        QPixmap pixIcon  = icon.pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        QPainter painter(&pixIcon);
        QPen pen(color);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        QPolygon polyScope1, polyScope2;
        polyScope1 << QPoint( IconBegin,  IconBegin) << QPoint(IconEnd, IconEnd);
        polyScope2 << QPoint(IconEnd,  IconBegin) << QPoint( IconBegin, IconEnd);
        painter.drawLines(polyScope1);
        painter.drawLines(polyScope2);
        
        painter.end();
        return QIcon(pixIcon);
    }

    QIcon _setScopeRound(const QIcon& icon, const QColor& color)
    {
        QPixmap pixmap = icon.pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // Draw two black diagonals (width 2)
        painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
        int margin = LogScopeIconFactory::IconPixels / 2;
        
        // Diagonal from top-left to bottom-right
        // painter.drawLine(QPointF(margin + 2, margin + 2), QPointF(LogScopeIconFactory::IconPixels - margin - 2, LogScopeIconFactory::IconPixels - margin - 2));
        painter.drawLine(QPointF(0, margin + 1), QPointF(LogScopeIconFactory::IconPixels, margin + 1));
        // Diagonal from bottom-left to top-right
        // painter.drawLine(QPointF(margin + 2, LogScopeIconFactory::IconPixels - margin - 2), QPointF(LogScopeIconFactory::IconPixels - margin - 2, margin + 2));
        painter.drawLine(QPointF(margin, 0), QPointF(margin, LogScopeIconFactory::IconPixels));

        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createRoundFourIcon(const QColor& color1, const QColor& color2, const QColor& color3, const QColor& color4)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QRectF rect(0, 0, LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);

        // Draw four colored quadrants as pie slices
        painter.setPen(Qt::NoPen);

        // Top-right (0° to 90°)
        painter.setBrush(color1);
        painter.drawPie(rect, 0 * 16, 90 * 16);

        // Bottom-right (90° to 180°)
        painter.setBrush(color2);
        painter.drawPie(rect, 90 * 16, 90 * 16);

        // Bottom-left (180° to 270°)
        painter.setBrush(color3);
        painter.drawPie(rect, 180 * 16, 90 * 16);

        // Top-left (270° to 360°)
        painter.setBrush(color4);
        painter.drawPie(rect, 270 * 16, 90 * 16);

        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createRoundFourIconWithDiagonals(const QColor& color1, const QColor& color2, const QColor& color3, const QColor& color4, const QColor& colorDiag)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QRectF rect(0, 0, LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);

        // Draw four colored quadrants as pie slices
        painter.setPen(Qt::NoPen);

        // Top-right (0° to 90°)
        painter.setBrush(color1);
        painter.drawPie(rect, 0 * 16, 90 * 16);

        // Bottom-right (90° to 180°)
        painter.setBrush(color2);
        painter.drawPie(rect, 90 * 16, 90 * 16);

        // Bottom-left (180° to 270°)
        painter.setBrush(color3);
        painter.drawPie(rect, 180 * 16, 90 * 16);

        // Top-left (270° to 360°)
        painter.setBrush(color4);
        painter.drawPie(rect, 270 * 16, 90 * 16);

        // Draw two black diagonals (width 2)
        painter.setPen(QPen(colorDiag, 2, Qt::SolidLine, Qt::RoundCap));
        // Diagonal from top-left to bottom-right
        painter.drawLine(QPointF(2, 2), QPointF(LogScopeIconFactory::IconPixels - 2, LogScopeIconFactory::IconPixels - 2));
        // Diagonal from bottom-left to top-right
        painter.drawLine(QPointF(2, LogScopeIconFactory::IconPixels - 2), QPointF(LogScopeIconFactory::IconPixels - 2, 2));

        painter.end();
        return QIcon(pixmap);
    }
    
    void _initialize(void)
    {
        if (_initialized.exchange(true))
            return;
        
        Q_ASSERT(_mapIcons.isEmpty());

        // Icon no prio
        uint32_t prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid);
        QIcon notSet = _createNotSetIcon();
        _mapIcons[static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid)]  = notSet;
        _mapIcons[static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset)]   = notSet;

        // icon prio scope
        QIcon scope = _createScopeIcon(_colors[static_cast<int>(ColorScope)]);
        uint32_t prioScope = static_cast<uint32_t>(NELogging::eLogPriority::PrioScope);
        _mapIcons[prioScope]    = scope;

        scope = _createRoundFourIconWithDiagonals(    _colors[static_cast<int>(ColotNotSet)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , _colors[static_cast<int>(ColorScope)]);
        _mapIcons[prioScope | RoundBits] = scope;

        // icon prio fatal
        QIcon fatal = _createOneIcon(_colors[static_cast<int>(ColorFatal)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio] = fatal;
        _mapIcons[prio | prioScope] = _setScope(fatal, _colors[static_cast<int>(ColorScope)]);

        fatal = _createRoundFourIconWithDiagonals(    _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = fatal;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(fatal, _colors[static_cast<int>(ColorScope)]);

        // icon prio error
        QIcon error = _createOneIcon(_colors[static_cast<int>(ColorError)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio] = error;
        _mapIcons[prio | prioScope] = _setScope(error, _colors[static_cast<int>(ColorScope)]);

        error = _createRoundFourIconWithDiagonals(    _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = error;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(error, _colors[static_cast<int>(ColorScope)]);

        // icon prio warn
        QIcon warn = _createOneIcon(_colors[static_cast<int>(ColorWarn)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);
        _mapIcons[prio] = warn;
        _mapIcons[prio | prioScope] = _setScope(warn, _colors[static_cast<int>(ColorScope)]);

        warn  = _createRoundFourIconWithDiagonals(    _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = warn;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(warn, _colors[static_cast<int>(ColorScope)]);

        // icon prio info
        QIcon info = _createOneIcon(_colors[static_cast<int>(ColorInfo)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo);
        _mapIcons[prio] = info;
        _mapIcons[prio | prioScope] = _setScope(info, _colors[static_cast<int>(ColorScope)]);
        info  = _createRoundFourIconWithDiagonals(    _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = info;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(info, _colors[static_cast<int>(ColorScope)]);

        // icon prio debug
        QIcon debug = _createOneIcon(_colors[static_cast<int>(ColorDebug)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug);
        _mapIcons[prio] = debug;
        _mapIcons[prio | prioScope] = _setScope(debug, _colors[static_cast<int>(ColorScope)]);

        debug  = _createRoundFourIconWithDiagonals(   _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = debug;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(debug, _colors[static_cast<int>(ColorScope)]);

        // icon prio debug + info
        QIcon dbgInfo = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorInfo)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo);
        _mapIcons[prio] = dbgInfo;
        _mapIcons[prio | prioScope] = _setScope(dbgInfo, _colors[static_cast<int>(ColorScope)]);

        dbgInfo = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = dbgInfo;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(dbgInfo, _colors[static_cast<int>(ColorScope)]);

        // icon prio debug + warn
        QIcon dbgWarn = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorWarn)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);
        _mapIcons[prio] = dbgWarn;
        _mapIcons[prio | prioScope] = _setScope(dbgWarn, _colors[static_cast<int>(ColorScope)]);

        dbgWarn = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = dbgWarn;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(dbgWarn, _colors[static_cast<int>(ColorScope)]);

        // icon prio debug + error
        QIcon dbgError = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorError)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio] = dbgError;
        _mapIcons[prio | prioScope] = _setScope(dbgError, _colors[static_cast<int>(ColorScope)]);

        dbgError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = dbgError;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(dbgError, _colors[static_cast<int>(ColorScope)]);

        // icon prio debug + fatal
        QIcon dbgFatal = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorFatal)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio] = dbgFatal;
        _mapIcons[prio | prioScope] = _setScope(dbgFatal, _colors[static_cast<int>(ColorScope)]);

        dbgFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = dbgFatal;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(dbgFatal, _colors[static_cast<int>(ColorScope)]);

        // icon prio info + warn
        QIcon infoWarn = _createTwoIcon(_colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorWarn)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);
        _mapIcons[prio] = infoWarn;
        _mapIcons[prio | prioScope] = _setScope(infoWarn, _colors[static_cast<int>(ColorScope)]);

        infoWarn = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = infoWarn;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(infoWarn, _colors[static_cast<int>(ColorScope)]);

        // icon prio info + error
        QIcon infoError = _createTwoIcon(_colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorError)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio] = infoError;
        _mapIcons[prio | prioScope] = _setScope(infoError, _colors[static_cast<int>(ColorScope)]);

        infoError = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = infoError;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(infoError, _colors[static_cast<int>(ColorScope)]);

        // icon prio info + fatal
        QIcon infoFatal = _createTwoIcon(_colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorFatal)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio] = infoFatal;
        _mapIcons[prio | prioScope] = _setScope(infoFatal, _colors[static_cast<int>(ColorScope)]);

        infoFatal = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = infoFatal;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(infoFatal, _colors[static_cast<int>(ColorScope)]);

        // icon prio warn + error
        QIcon warnError = _createTwoIcon(_colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorError)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio] = warnError;
        _mapIcons[prio | prioScope] = _setScope(warnError, _colors[static_cast<int>(ColorScope)]);

        warnError = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = warnError;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(warnError, _colors[static_cast<int>(ColorScope)]);

        // icon prio warn + fatal
        QIcon warnFatal = _createTwoIcon(_colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorFatal)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio] = warnFatal;
        _mapIcons[prio | prioScope] = _setScope(warnFatal, _colors[static_cast<int>(ColorScope)]);

        warnFatal = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = warnFatal;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(warnFatal, _colors[static_cast<int>(ColorScope)]);

        // icon prio error + fatal
        QIcon errorFatal = _createTwoIcon(_colors[static_cast<int>(ColorError)], _colors[static_cast<int>(ColorFatal)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio] = errorFatal;
        _mapIcons[prio | prioScope] = _setScope(errorFatal, _colors[static_cast<int>(ColorScope)]);

        errorFatal = _createRoundFourIconWithDiagonals(   _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = errorFatal;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(errorFatal, _colors[static_cast<int>(ColorScope)]);

        // icon prio debug + info + warn
        QIcon dbgInfoWarn = _createFourIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColotNotSet)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);
        _mapIcons[prio] = dbgInfoWarn;
        _mapIcons[prio | prioScope] = _setScope(dbgInfoWarn, _colors[static_cast<int>(ColorScope)]);

        dbgInfoWarn = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = dbgInfoWarn;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(dbgInfoWarn, _colors[static_cast<int>(ColorScope)]);

        // icon prio debug + info + error
        QIcon dbgInfoError = _createFourIcon(_colors[static_cast<int>(ColorDebug)], _colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorError)], _colors[static_cast<int>(ColotNotSet)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio] = dbgInfoError;
        _mapIcons[prio | prioScope] = _setScope(dbgInfoError, _colors[static_cast<int>(ColorScope)]);

        dbgInfoError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = dbgInfoError;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(dbgInfoError, _colors[static_cast<int>(ColorScope)]);

        // icon prio debug + info + fatal
        QIcon dbgInfoFatal = _createFourIcon(_colors[static_cast<int>(ColorDebug)], _colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorFatal)], _colors[static_cast<int>(ColotNotSet)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio] = dbgInfoFatal;
        _mapIcons[prio | prioScope] = _setScope(dbgInfoFatal, _colors[static_cast<int>(ColorScope)]);

        dbgInfoFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = dbgInfoFatal;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(dbgInfoFatal, _colors[static_cast<int>(ColorScope)]);

        // icon prio debug + warn + error
        QIcon dbgWarnError = _createFourIcon(_colors[static_cast<int>(ColorDebug)], _colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorError)], _colors[static_cast<int>(ColotNotSet)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio] = dbgWarnError;
        _mapIcons[prio | prioScope] = _setScope(dbgWarnError, _colors[static_cast<int>(ColorScope)]);

        dbgWarnError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = dbgWarnError;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(dbgWarnError, _colors[static_cast<int>(ColorScope)]);

        // icon prio debug + warn + fatal
        QIcon dbgWarnFatal = _createFourIcon(_colors[static_cast<int>(ColorDebug)], _colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorFatal)], _colors[static_cast<int>(ColotNotSet)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio] = dbgWarnFatal;
        _mapIcons[prio | prioScope] = _setScope(dbgWarnFatal, _colors[static_cast<int>(ColorScope)]);

        dbgWarnFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = dbgWarnFatal;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(dbgWarnFatal, _colors[static_cast<int>(ColorScope)]);

        // icon prio debug + error + fatal
        QIcon dbgErrorFatal = _createFourIcon(_colors[static_cast<int>(ColorDebug)], _colors[static_cast<int>(ColorError)], _colors[static_cast<int>(ColorFatal)], _colors[static_cast<int>(ColotNotSet)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio] = dbgErrorFatal;
        _mapIcons[prio | prioScope] = _setScope(dbgErrorFatal, _colors[static_cast<int>(ColorScope)]);

        dbgErrorFatal = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = dbgErrorFatal;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(dbgErrorFatal, _colors[static_cast<int>(ColorScope)]);

        // icon prio info + warn + error
        QIcon infoWarnError = _createFourIcon(_colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorError)], _colors[static_cast<int>(ColotNotSet)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio] = infoWarnError;
        _mapIcons[prio | prioScope] = _setScope(infoWarnError, _colors[static_cast<int>(ColorScope)]);

        infoWarnError = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = infoWarnError;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(infoWarnError, _colors[static_cast<int>(ColorScope)]);

        // icon prio info + warn + fatal
        QIcon infoWarnFatal = _createFourIcon(_colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorFatal)], _colors[static_cast<int>(ColotNotSet)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio] = infoWarnFatal;
        _mapIcons[prio | prioScope] = _setScope(infoWarnFatal, _colors[static_cast<int>(ColorScope)]);

        infoWarnFatal = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = infoWarnFatal;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(infoWarnFatal, _colors[static_cast<int>(ColorScope)]);

        // icon prio info + error + fatal
        QIcon infoErrorFatal = _createFourIcon(_colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorError)], _colors[static_cast<int>(ColorFatal)], _colors[static_cast<int>(ColotNotSet)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio] = infoErrorFatal;
        _mapIcons[prio | prioScope] = _setScope(infoErrorFatal, _colors[static_cast<int>(ColorScope)]);

        infoErrorFatal = _createRoundFourIconWithDiagonals(   _colors[static_cast<int>(ColorInfo)]
                                                            , _colors[static_cast<int>(ColorError)]
                                                            , _colors[static_cast<int>(ColorFatal)]
                                                            , _colors[static_cast<int>(ColotNotSet)]
                                                            , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = infoErrorFatal;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(infoErrorFatal, _colors[static_cast<int>(ColorScope)]);

        // icon prio warn + error + fatal
        QIcon warnErrorFatal = _createFourIcon(_colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorError)], _colors[static_cast<int>(ColorFatal)], _colors[static_cast<int>(ColotNotSet)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio] = warnErrorFatal;
        _mapIcons[prio | prioScope] = _setScope(warnErrorFatal, _colors[static_cast<int>(ColorScope)]);

        warnErrorFatal = _createRoundFourIconWithDiagonals(   _colors[static_cast<int>(ColorWarn)]
                                                            , _colors[static_cast<int>(ColorError)]
                                                            , _colors[static_cast<int>(ColorFatal)]
                                                            , _colors[static_cast<int>(ColotNotSet)]
                                                            , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = warnErrorFatal;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(warnErrorFatal, _colors[static_cast<int>(ColorScope)]);


        // icon prio debug + info + warn + error
        QIcon dbgInfoWarnErr = _createFourIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColorError)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio] = dbgInfoWarnErr;
        _mapIcons[prio | prioScope] = _setScope(dbgInfoWarnErr, _colors[static_cast<int>(ColorScope)]);

        dbgInfoWarnErr = _createRoundFourIconWithDiagonals(   _colors[static_cast<int>(ColorDebug)]
                                                            , _colors[static_cast<int>(ColorInfo)]
                                                            , _colors[static_cast<int>(ColorWarn)]
                                                            , _colors[static_cast<int>(ColorError)]
                                                            , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = dbgInfoWarnErr;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(dbgInfoWarnErr, _colors[static_cast<int>(ColorScope)]);

        // icon prio all
        QIcon all = _createFourIcon(_colors[static_cast<int>(ColorDebug)], _colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorError)]);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio] = all;
        _mapIcons[prio | prioScope] = _setScope(all, _colors[static_cast<int>(ColorScope)]);

        all = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                , _colors[static_cast<int>(ColorInfo)]
                                                , _colors[static_cast<int>(ColorWarn)]
                                                , _colors[static_cast<int>(ColorError)]
                                                , _colors[static_cast<int>(ColotNotSet)]);
        _mapIcons[prio | RoundBits] = all;
        _mapIcons[prio | prioScope | RoundBits] = _setScopeRound(all, _colors[static_cast<int>(ColorScope)]);
    }
    
}

QIcon LogScopeIconFactory::getIcon(uint32_t scopePrio)
{
    _initialize();
    uint32_t prio = (scopePrio & static_cast<uint32_t>(NELogging::eLogPriority::PrioValidLogs));
    if ((prio != static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset)) && (prio & static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset)) != 0)
        prio &= ~static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset);
    
    Q_ASSERT(_mapIcons.contains(prio));
    if (prio == static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset))
    {
        return _mapIcons[prio];
    }
    else
    {
        Q_ASSERT(_mapIcons.contains(prio | RoundBits));
        return _mapIcons[prio | RoundBits];
    }
}
