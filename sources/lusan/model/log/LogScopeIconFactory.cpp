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
        , ColorWithScope
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
        , QColor(Qt::white)         // ColorWithScope
    };
    
    std::atomic_bool _initialized   { false };
    QMap<uint32_t, QIcon> _mapIcons;
    
    const QColor NoColor            { Qt::transparent };
    constexpr int IconBegin         { 0 };
    constexpr int IconEnd           { LogScopeIconFactory::IconPixels - IconBegin };
    constexpr int IconMiddle        { (IconEnd - IconBegin) / 2 };
    constexpr uint32_t SquareBits   { 0x80000000u };
    constexpr uint32_t NoPrio       { static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset) };
    constexpr int32_t  AlphaSolid   { 255 };
    constexpr int32_t  AlphaMixed   { 75 };

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
        QPixmap pixIcon(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixIcon.fill(Qt::transparent);
        QPainter painter(&pixIcon);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(Qt::NoBrush);
        
        // Line from top to bottom
        painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, 2), QPointF(LogScopeIconFactory::IconPixels - 2, LogScopeIconFactory::IconPixels - 2));
        // Line from left to right
        painter.setPen(QPen(color, 3, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, LogScopeIconFactory::IconPixels - 2), QPointF(LogScopeIconFactory::IconPixels - 2, 2));
        
        painter.end();
        return QIcon(pixIcon);
    }

    QIcon _createOneIcon(QColor color, bool setAlpha)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);
        color.setAlpha(setAlpha && (color != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color);
        painter.drawRect(IconBegin, IconBegin, IconEnd, IconEnd);
        
        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createTwoIcon(QColor color1, QColor color2, bool setAlpha)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);

        // Top-left triangle
        QPolygon poly2;
        poly2 << QPoint(IconBegin, IconBegin) << QPoint(IconEnd, IconBegin) << QPoint(IconBegin, IconEnd);
        color2.setAlpha(setAlpha && (color2 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color2);
        painter.drawPolygon(poly2);

        // Bottom-right triangle
        QPolygon poly1;
        poly1 << QPoint(IconEnd, IconEnd) << QPoint(IconEnd, IconBegin) << QPoint(IconBegin, IconEnd);
        color1.setAlpha(setAlpha && (color1 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color1);
        painter.drawPolygon(poly1);
        
        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createFourIcon(QColor color1, QColor color2, QColor color3, QColor color4, bool setAlpha)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);

        // Top-left triangle
        QPolygon poly4;
        poly4 << QPoint(IconBegin, IconBegin) << QPoint(IconMiddle, IconBegin) << QPoint(IconBegin, IconMiddle);
        color4.setAlpha(setAlpha && (color4 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color4);
        painter.drawPolygon(poly4);
        
        // Top-right triangle
        QPolygon poly3;
        poly3 << QPoint(IconBegin, IconBegin) << QPoint(IconEnd, IconBegin) << QPoint(IconMiddle, IconMiddle);
        color3.setAlpha(setAlpha && (color3 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color3);
        painter.drawPolygon(poly3);
        
        // Bottom-left triangle
        QPolygon poly2;
        poly2 << QPoint(IconEnd, IconBegin) << QPoint(IconEnd, IconEnd) << QPoint(IconMiddle, IconMiddle);
        color2.setAlpha(setAlpha && (color2 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color2);
        painter.drawPolygon(poly2);
        
        // Bottom-right triangle
        QPolygon poly1;
        poly1 << QPoint(IconEnd, IconEnd) << QPoint(IconBegin, IconEnd) << QPoint(IconMiddle, IconMiddle);
        color1.setAlpha(setAlpha && (color1 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color1);
        painter.drawPolygon(poly1);
        
        painter.end();
        return QIcon(pixmap);
    }
    
    QIcon _setScope(const QIcon& icon, QColor color)
    {
        QPixmap pixIcon  = icon.pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        QPainter painter(&pixIcon);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(Qt::NoBrush);
        
        // Line from top to bottom
        painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, 2), QPointF(LogScopeIconFactory::IconPixels - 2, LogScopeIconFactory::IconPixels - 2));
        // Line from left to right
        painter.setPen(QPen(color, 3, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, LogScopeIconFactory::IconPixels - 2), QPointF(LogScopeIconFactory::IconPixels - 2, 2));
        
        painter.end();
        return QIcon(pixIcon);
    }

    QIcon _setScopeRound(const QIcon& icon, QColor color)
    {
        // constexpr int margin {LogScopeIconFactory::IconPixels / 2 };
        // constexpr int shift  { 0 };

        QPixmap pixmap = icon.pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(Qt::NoBrush);

        // line from top to bottom
        painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, 2), QPointF(LogScopeIconFactory::IconPixels - 2, LogScopeIconFactory::IconPixels - 2));
        // painter.drawLine(QPointF(0, margin + shift), QPointF(LogScopeIconFactory::IconPixels, margin + shift));
        // line from left to right
        painter.setPen(QPen(color, 3, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, LogScopeIconFactory::IconPixels - 2), QPointF(LogScopeIconFactory::IconPixels - 2, 2));
        // painter.drawLine(QPointF(margin, 0), QPointF(margin, LogScopeIconFactory::IconPixels));

        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createRoundFourIcon(QColor color1, QColor color2, QColor color3, QColor color4, bool setAlpha)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QRectF rect(0, 0, LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);

        // Draw four colored quadrants as pie slices
        painter.setPen(Qt::NoPen);

        // Top-right (0° to 90°)
        color1.setAlpha(setAlpha && (color1 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color1);
        painter.drawPie(rect, 0 * 16, 90 * 16);

        // Bottom-right (90° to 180°)
        color2.setAlpha(setAlpha && (color2 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color2);
        painter.drawPie(rect, 90 * 16, 90 * 16);

        // Bottom-left (180° to 270°)
        color3.setAlpha(setAlpha && (color3 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color3);
        painter.drawPie(rect, 180 * 16, 90 * 16);

        // Top-left (270° to 360°)
        color4.setAlpha(setAlpha && (color4 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color4);
        painter.drawPie(rect, 270 * 16, 90 * 16);

        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createRoundFourIconWithDiagonals(QColor color1, QColor color2, QColor color3, QColor color4, QColor colorDiag, bool setAlpha)
    {
        QPixmap pixmap(LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QRectF rect(0, 0, LogScopeIconFactory::IconPixels, LogScopeIconFactory::IconPixels);

        // Draw four colored quadrants as pie slices
        painter.setPen(Qt::NoPen);

        // Top-right (0° to 90°)
        color1.setAlpha(setAlpha && (color1 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color1);
        painter.drawPie(rect, 0 * 16, 90 * 16);

        // Bottom-right (90° to 180°)
        color2.setAlpha(setAlpha && (color2 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color2);
        painter.drawPie(rect, 90 * 16, 90 * 16);

        // Bottom-left (180° to 270°)
        color3.setAlpha(setAlpha && (color3 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color3);
        painter.drawPie(rect, 180 * 16, 90 * 16);

        // Top-left (270° to 360°)
        color4.setAlpha(setAlpha && (color4 != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color4);
        painter.drawPie(rect, 270 * 16, 90 * 16);

        // Draw two black diagonals (width 2)
        painter.setPen(QPen(colorDiag, 2, Qt::SolidLine, Qt::RoundCap));
        // Diagonal from top to bottom
        painter.drawLine(QPointF(2, 2), QPointF(LogScopeIconFactory::IconPixels - 2, LogScopeIconFactory::IconPixels - 2));
        // Diagonal from left to right
        painter.setPen(QPen(colorDiag, 3, Qt::SolidLine, Qt::RoundCap));
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
        _mapIcons[prioScope | SquareBits]    = scope;
        
        /*
        scope = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColotNotSet)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , _colors[static_cast<int>(ColorScope)]
                                                  , false);
        */
        
        _mapIcons[prioScope] = scope;
        _mapIcons[prioScope | NoPrio] = scope;

        // icon prio fatal
        QIcon fatal = _createOneIcon(_colors[static_cast<int>(ColorFatal)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio | SquareBits] = fatal;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(fatal, _colors[static_cast<int>(ColorWithScope)]);
        
        fatal = _createOneIcon(_colors[static_cast<int>(ColorFatal)], true);
        _mapIcons[prio | NoPrio | SquareBits] = fatal;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(fatal, _colors[static_cast<int>(ColorWithScope)]);

        fatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , false);
        _mapIcons[prio] = fatal;
        _mapIcons[prio | prioScope] = _setScopeRound(fatal, _colors[static_cast<int>(ColorWithScope)]);
        
        fatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , true);
        _mapIcons[prio | NoPrio] = fatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(fatal, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio error
        QIcon error = _createOneIcon(_colors[static_cast<int>(ColorError)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio | SquareBits] = error;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(error, _colors[static_cast<int>(ColorWithScope)]);
        
        error = _createOneIcon(_colors[static_cast<int>(ColorError)], true);
        _mapIcons[prio | NoPrio | SquareBits] = error;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(error, _colors[static_cast<int>(ColorWithScope)]);
        
        error = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , false);
        _mapIcons[prio] = error;
        _mapIcons[prio | prioScope] = _setScopeRound(error, _colors[static_cast<int>(ColorWithScope)]);
        
        error = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , true);
        _mapIcons[prio | NoPrio] = error;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(error, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio warn
        QIcon warn = _createOneIcon(_colors[static_cast<int>(ColorWarn)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);
        _mapIcons[prio | SquareBits] = warn;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(warn, _colors[static_cast<int>(ColorWithScope)]);
        
        warn = _createOneIcon(_colors[static_cast<int>(ColorWarn)], true);
        _mapIcons[prio | NoPrio | SquareBits] = warn;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(warn, _colors[static_cast<int>(ColorWithScope)]);
        
        warn  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColotNotSet)]
                                                 , false);
        _mapIcons[prio] = warn;
        _mapIcons[prio | prioScope] = _setScopeRound(warn, _colors[static_cast<int>(ColorWithScope)]);
        
        warn  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColotNotSet)]
                                                 , true);
        _mapIcons[prio | NoPrio] = warn;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(warn, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio info
        QIcon info = _createOneIcon(_colors[static_cast<int>(ColorInfo)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo);
        _mapIcons[prio | SquareBits] = info;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(info, _colors[static_cast<int>(ColorWithScope)]);
        
        info = _createOneIcon(_colors[static_cast<int>(ColorInfo)], true);
        _mapIcons[prio | NoPrio | SquareBits] = info;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(info, _colors[static_cast<int>(ColorWithScope)]);
        
        info  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColotNotSet)]
                                                 , false);
        _mapIcons[prio] = info;
        _mapIcons[prio | prioScope] = _setScopeRound(info, _colors[static_cast<int>(ColorWithScope)]);
        
        info  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColotNotSet)]
                                                 , true);
        _mapIcons[prio | NoPrio] = info;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(info, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug
        QIcon debug = _createOneIcon(_colors[static_cast<int>(ColorDebug)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug);
        _mapIcons[prio | SquareBits] = debug;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(debug, _colors[static_cast<int>(ColorWithScope)]);
        
        debug = _createOneIcon(_colors[static_cast<int>(ColorDebug)], true);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug);
        _mapIcons[prio | NoPrio | SquareBits] = debug;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(debug, _colors[static_cast<int>(ColorWithScope)]);
        
        debug  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , false);
        _mapIcons[prio] = debug;
        _mapIcons[prio | prioScope] = _setScopeRound(debug, _colors[static_cast<int>(ColorWithScope)]);
        
        debug  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , true);
        _mapIcons[prio | NoPrio] = debug;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(debug, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug + info
        QIcon dbgInfo = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorInfo)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo);
        _mapIcons[prio | SquareBits] = dbgInfo;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(dbgInfo, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfo = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorInfo)], true);
        _mapIcons[prio | NoPrio | SquareBits] = dbgInfo;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(dbgInfo, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfo = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , false);
        _mapIcons[prio] = dbgInfo;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgInfo, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfo = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , true);
        _mapIcons[prio | NoPrio] = dbgInfo;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgInfo, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug + warn
        QIcon dbgWarn = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorWarn)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);
        _mapIcons[prio | SquareBits] = dbgWarn;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(dbgWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgWarn = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorWarn)], true);
        _mapIcons[prio | NoPrio | SquareBits] = dbgWarn;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(dbgWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgWarn = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , false);
        _mapIcons[prio] = dbgWarn;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgWarn = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , true);
        _mapIcons[prio | NoPrio] = dbgWarn;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug + error
        QIcon dbgError = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorError)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio | SquareBits] = dbgError;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(dbgError, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgError = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorError)], true);
        _mapIcons[prio | NoPrio | SquareBits] = dbgError;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(dbgError, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , false);
        _mapIcons[prio] = dbgError;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgError, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                     , _colors[static_cast<int>(ColorDebug)]
                                                     , _colors[static_cast<int>(ColorError)]
                                                     , _colors[static_cast<int>(ColorError)]
                                                     , _colors[static_cast<int>(ColotNotSet)]
                                                     , true);
        _mapIcons[prio | NoPrio] = dbgError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgError, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug + fatal
        QIcon dbgFatal = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorFatal)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio | SquareBits] = dbgFatal;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(dbgFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgFatal = _createTwoIcon(_colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorFatal)], true);
        _mapIcons[prio | NoPrio | SquareBits] = dbgFatal;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(dbgFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , false);
        _mapIcons[prio] = dbgFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                     , _colors[static_cast<int>(ColorDebug)]
                                                     , _colors[static_cast<int>(ColorFatal)]
                                                     , _colors[static_cast<int>(ColorFatal)]
                                                     , _colors[static_cast<int>(ColotNotSet)]
                                                     , true);
        _mapIcons[prio | NoPrio] = dbgFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio info + warn
        QIcon infoWarn = _createTwoIcon(_colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorWarn)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);
        _mapIcons[prio | SquareBits] = infoWarn;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(infoWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        infoWarn = _createTwoIcon(_colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorWarn)], true);
        _mapIcons[prio | NoPrio | SquareBits] = infoWarn;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(infoWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        infoWarn = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , false);
        _mapIcons[prio] = infoWarn;
        _mapIcons[prio | prioScope] = _setScopeRound(infoWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        infoWarn = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorInfo)]
                                                     , _colors[static_cast<int>(ColorInfo)]
                                                     , _colors[static_cast<int>(ColorWarn)]
                                                     , _colors[static_cast<int>(ColorWarn)]
                                                     , _colors[static_cast<int>(ColotNotSet)]
                                                     , true);
        _mapIcons[prio | NoPrio] = infoWarn;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio info + error
        QIcon infoError = _createTwoIcon(_colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorError)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio | SquareBits] = infoError;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(infoError, _colors[static_cast<int>(ColorWithScope)]);
        
        infoError = _createTwoIcon(_colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorError)], true);
        _mapIcons[prio | NoPrio | SquareBits] = infoError;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(infoError, _colors[static_cast<int>(ColorWithScope)]);
        
        infoError = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , false);
        _mapIcons[prio] = infoError;
        _mapIcons[prio | prioScope] = _setScopeRound(infoError, _colors[static_cast<int>(ColorWithScope)]);
        
        infoError = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , true);
        _mapIcons[prio | NoPrio] = infoError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoError, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio info + fatal
        QIcon infoFatal = _createTwoIcon(_colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorFatal)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio | SquareBits] = infoFatal;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(infoFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        infoFatal = _createTwoIcon(_colors[static_cast<int>(ColorInfo)], _colors[static_cast<int>(ColorFatal)], true);
        _mapIcons[prio | NoPrio | SquareBits] = infoFatal;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(infoFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        infoFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , false);
        _mapIcons[prio] = infoFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(infoFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        infoFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , true);
        _mapIcons[prio | NoPrio] = infoFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio warn + error
        QIcon warnError = _createTwoIcon(_colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorError)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio | SquareBits] = warnError;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(warnError, _colors[static_cast<int>(ColorWithScope)]);
        
        warnError = _createTwoIcon(_colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorError)], true);
        _mapIcons[prio | NoPrio | SquareBits] = warnError;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(warnError, _colors[static_cast<int>(ColorWithScope)]);
        
        warnError = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , false);
        _mapIcons[prio] = warnError;
        _mapIcons[prio | prioScope] = _setScopeRound(warnError, _colors[static_cast<int>(ColorWithScope)]);
        
        warnError = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , true);
        _mapIcons[prio | NoPrio] = warnError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(warnError, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio warn + fatal
        QIcon warnFatal = _createTwoIcon(_colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorFatal)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio | SquareBits] = warnFatal;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(warnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        warnFatal = _createTwoIcon(_colors[static_cast<int>(ColorWarn)], _colors[static_cast<int>(ColorFatal)], true);
        _mapIcons[prio | NoPrio | SquareBits] = warnFatal;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(warnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        warnFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , false);
        _mapIcons[prio] = warnFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(warnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        warnFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , true);
        _mapIcons[prio | NoPrio] = warnFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(warnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio error + fatal
        QIcon errorFatal = _createTwoIcon(_colors[static_cast<int>(ColorError)], _colors[static_cast<int>(ColorFatal)], false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio | SquareBits] = errorFatal;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(errorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        errorFatal = _createTwoIcon(_colors[static_cast<int>(ColorError)], _colors[static_cast<int>(ColorFatal)], true);
        _mapIcons[prio | NoPrio | SquareBits] = errorFatal;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(errorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        errorFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorError)]
                                                       , _colors[static_cast<int>(ColorError)]
                                                       , _colors[static_cast<int>(ColorFatal)]
                                                       , _colors[static_cast<int>(ColorFatal)]
                                                       , _colors[static_cast<int>(ColotNotSet)]
                                                       , false);
        _mapIcons[prio] = errorFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(errorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        errorFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorError)]
                                                       , _colors[static_cast<int>(ColorError)]
                                                       , _colors[static_cast<int>(ColorFatal)]
                                                       , _colors[static_cast<int>(ColorFatal)]
                                                       , _colors[static_cast<int>(ColotNotSet)]
                                                       , true);
        _mapIcons[prio | NoPrio] = errorFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(errorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug + info + warn
        QIcon dbgInfoWarn = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                            , _colors[static_cast<int>(ColorInfo)]
                                            , _colors[static_cast<int>(ColorWarn)]
                                            , _colors[static_cast<int>(ColotNotSet)]
                                            , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);
        _mapIcons[prio | SquareBits] = dbgInfoWarn;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(dbgInfoWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoWarn = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                      , _colors[static_cast<int>(ColorInfo)]
                                      , _colors[static_cast<int>(ColorWarn)]
                                      , _colors[static_cast<int>(ColotNotSet)]
                                      , true);
        _mapIcons[prio | NoPrio | SquareBits] = dbgInfoWarn;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(dbgInfoWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoWarn = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false);
        _mapIcons[prio] = dbgInfoWarn;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgInfoWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoWarn = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , true);
        _mapIcons[prio | NoPrio] = dbgInfoWarn;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgInfoWarn, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug + info + error
        QIcon dbgInfoError = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                             , _colors[static_cast<int>(ColorInfo)]
                                             , _colors[static_cast<int>(ColorError)]
                                             , _colors[static_cast<int>(ColotNotSet)]
                                             , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio | SquareBits] = dbgInfoError;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(dbgInfoError, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoError = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                       , _colors[static_cast<int>(ColorInfo)]
                                       , _colors[static_cast<int>(ColorError)]
                                       , _colors[static_cast<int>(ColotNotSet)]
                                       , true);
        _mapIcons[prio | NoPrio | SquareBits] = dbgInfoError;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(dbgInfoError, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false);
        _mapIcons[prio] = dbgInfoError;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgInfoError, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                         , _colors[static_cast<int>(ColorInfo)]
                                                         , _colors[static_cast<int>(ColorError)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , true);
        _mapIcons[prio | NoPrio] = dbgInfoError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgInfoError, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug + info + fatal
        QIcon dbgInfoFatal = _createFourIcon( _colors[static_cast<int>(ColorDebug)]
                                             , _colors[static_cast<int>(ColorInfo)]
                                             , _colors[static_cast<int>(ColorFatal)]
                                             , _colors[static_cast<int>(ColotNotSet)]
                                             , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio | SquareBits] = dbgInfoFatal;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(dbgInfoFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoFatal = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                       , _colors[static_cast<int>(ColorInfo)]
                                       , _colors[static_cast<int>(ColorFatal)]
                                       , _colors[static_cast<int>(ColotNotSet)]
                                       , true);
        _mapIcons[prio | NoPrio | SquareBits] = dbgInfoFatal;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(dbgInfoFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false);
        _mapIcons[prio] = dbgInfoFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgInfoFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                         , _colors[static_cast<int>(ColorInfo)]
                                                         , _colors[static_cast<int>(ColorFatal)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , true);
        _mapIcons[prio | NoPrio] = dbgInfoFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgInfoFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug + warn + error
        QIcon dbgWarnError = _createFourIcon( _colors[static_cast<int>(ColorDebug)]
                                             , _colors[static_cast<int>(ColorWarn)]
                                             , _colors[static_cast<int>(ColorError)]
                                             , _colors[static_cast<int>(ColotNotSet)]
                                             , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio | SquareBits] = dbgWarnError;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(dbgWarnError, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgWarnError = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                       , _colors[static_cast<int>(ColorWarn)]
                                       , _colors[static_cast<int>(ColorError)]
                                       , _colors[static_cast<int>(ColotNotSet)]
                                       , true);
        _mapIcons[prio | NoPrio | SquareBits] = dbgWarnError;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(dbgWarnError, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgWarnError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false);
        _mapIcons[prio] = dbgWarnError;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgWarnError, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgWarnError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                         , _colors[static_cast<int>(ColorWarn)]
                                                         , _colors[static_cast<int>(ColorError)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , true);
        _mapIcons[prio | NoPrio] = dbgWarnError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgWarnError, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug + warn + fatal
        QIcon dbgWarnFatal = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                             , _colors[static_cast<int>(ColorWarn)]
                                             , _colors[static_cast<int>(ColorFatal)]
                                             , _colors[static_cast<int>(ColotNotSet)]
                                             , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio | SquareBits] = dbgWarnFatal;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(dbgWarnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgWarnFatal = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                       , _colors[static_cast<int>(ColorWarn)]
                                       , _colors[static_cast<int>(ColorFatal)]
                                       , _colors[static_cast<int>(ColotNotSet)]
                                       , true);
        _mapIcons[prio | NoPrio | SquareBits] = dbgWarnFatal;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(dbgWarnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgWarnFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false);
        _mapIcons[prio] = dbgWarnFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgWarnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgWarnFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                         , _colors[static_cast<int>(ColorWarn)]
                                                         , _colors[static_cast<int>(ColorFatal)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , true);
        _mapIcons[prio | NoPrio] = dbgWarnFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgWarnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug + error + fatal
        QIcon dbgErrorFatal = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                              , _colors[static_cast<int>(ColorError)]
                                              , _colors[static_cast<int>(ColorFatal)]
                                              , _colors[static_cast<int>(ColotNotSet)]
                                              , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio | SquareBits] = dbgErrorFatal;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(dbgErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgErrorFatal = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                        , _colors[static_cast<int>(ColorError)]
                                        , _colors[static_cast<int>(ColorFatal)]
                                        , _colors[static_cast<int>(ColotNotSet)]
                                        , true);
        _mapIcons[prio | NoPrio | SquareBits] = dbgErrorFatal;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(dbgErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgErrorFatal = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false);
        _mapIcons[prio] = dbgErrorFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgErrorFatal = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorDebug)]
                                                          , _colors[static_cast<int>(ColorError)]
                                                          , _colors[static_cast<int>(ColorFatal)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , true);
        _mapIcons[prio | NoPrio] = dbgErrorFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio info + warn + error
        QIcon infoWarnError = _createFourIcon(  _colors[static_cast<int>(ColorInfo)]
                                              , _colors[static_cast<int>(ColorWarn)]
                                              , _colors[static_cast<int>(ColorError)]
                                              , _colors[static_cast<int>(ColotNotSet)]
                                              , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio | SquareBits] = infoWarnError;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(infoWarnError, _colors[static_cast<int>(ColorWithScope)]);
        
        infoWarnError = _createFourIcon(  _colors[static_cast<int>(ColorInfo)]
                                        , _colors[static_cast<int>(ColorWarn)]
                                        , _colors[static_cast<int>(ColorError)]
                                        , _colors[static_cast<int>(ColotNotSet)]
                                        , true);
        _mapIcons[prio | NoPrio | SquareBits] = infoWarnError;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(infoWarnError, _colors[static_cast<int>(ColorWithScope)]);
        
        infoWarnError = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false);
        _mapIcons[prio] = infoWarnError;
        _mapIcons[prio | prioScope] = _setScopeRound(infoWarnError, _colors[static_cast<int>(ColorWithScope)]);
        
        infoWarnError = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorInfo)]
                                                          , _colors[static_cast<int>(ColorWarn)]
                                                          , _colors[static_cast<int>(ColorError)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , true);
        _mapIcons[prio | NoPrio] = infoWarnError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoWarnError, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio info + warn + fatal
        QIcon infoWarnFatal = _createFourIcon(  _colors[static_cast<int>(ColorInfo)]
                                              , _colors[static_cast<int>(ColorWarn)]
                                              , _colors[static_cast<int>(ColorFatal)]
                                              , _colors[static_cast<int>(ColotNotSet)]
                                              , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio | SquareBits] = infoWarnFatal;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(infoWarnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        infoWarnFatal = _createFourIcon(  _colors[static_cast<int>(ColorInfo)]
                                        , _colors[static_cast<int>(ColorWarn)]
                                        , _colors[static_cast<int>(ColorFatal)]
                                        , _colors[static_cast<int>(ColotNotSet)]
                                        , true);
        _mapIcons[prio | NoPrio | SquareBits] = infoWarnFatal;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(infoWarnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        infoWarnFatal = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false);
        _mapIcons[prio] = infoWarnFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(infoWarnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        infoWarnFatal = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorInfo)]
                                                          , _colors[static_cast<int>(ColorWarn)]
                                                          , _colors[static_cast<int>(ColorFatal)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , true);
        _mapIcons[prio | NoPrio] = infoWarnFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoWarnFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio info + error + fatal
        QIcon infoErrorFatal = _createFourIcon(  _colors[static_cast<int>(ColorInfo)]
                                               , _colors[static_cast<int>(ColorError)]
                                               , _colors[static_cast<int>(ColorFatal)]
                                               , _colors[static_cast<int>(ColotNotSet)]
                                               , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio | SquareBits] = infoErrorFatal;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(infoErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        infoErrorFatal = _createFourIcon(  _colors[static_cast<int>(ColorInfo)]
                                         , _colors[static_cast<int>(ColorError)]
                                         , _colors[static_cast<int>(ColorFatal)]
                                         , _colors[static_cast<int>(ColotNotSet)]
                                         , true);
        _mapIcons[prio | NoPrio | SquareBits] = infoErrorFatal;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(infoErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        infoErrorFatal = _createRoundFourIconWithDiagonals(   _colors[static_cast<int>(ColorInfo)]
                                                            , _colors[static_cast<int>(ColorError)]
                                                            , _colors[static_cast<int>(ColorFatal)]
                                                            , _colors[static_cast<int>(ColotNotSet)]
                                                            , _colors[static_cast<int>(ColotNotSet)]
                                                            , false);
        _mapIcons[prio] = infoErrorFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(infoErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        infoErrorFatal = _createRoundFourIconWithDiagonals(   _colors[static_cast<int>(ColorInfo)]
                                                           , _colors[static_cast<int>(ColorError)]
                                                           , _colors[static_cast<int>(ColorFatal)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , true);
        _mapIcons[prio | NoPrio] = infoErrorFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio warn + error + fatal
        QIcon warnErrorFatal = _createFourIcon(  _colors[static_cast<int>(ColorWarn)]
                                               , _colors[static_cast<int>(ColorError)]
                                               , _colors[static_cast<int>(ColorFatal)]
                                               , _colors[static_cast<int>(ColotNotSet)]
                                               , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);
        _mapIcons[prio | SquareBits] = warnErrorFatal;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(warnErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        warnErrorFatal = _createFourIcon(  _colors[static_cast<int>(ColorWarn)]
                                         , _colors[static_cast<int>(ColorError)]
                                         , _colors[static_cast<int>(ColorFatal)]
                                         , _colors[static_cast<int>(ColotNotSet)]
                                         , true);
        _mapIcons[prio | NoPrio | SquareBits] = warnErrorFatal;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(warnErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        warnErrorFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                           , _colors[static_cast<int>(ColorError)]
                                                           , _colors[static_cast<int>(ColorFatal)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , false);
        _mapIcons[prio] = warnErrorFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(warnErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        warnErrorFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                           , _colors[static_cast<int>(ColorError)]
                                                           , _colors[static_cast<int>(ColorFatal)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , true);
        _mapIcons[prio | NoPrio] = warnErrorFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(warnErrorFatal, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio debug + info + warn + error
        QIcon dbgInfoWarnErr = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                               , _colors[static_cast<int>(ColorInfo)]
                                               , _colors[static_cast<int>(ColorWarn)]
                                               , _colors[static_cast<int>(ColorError)]
                                               , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio | SquareBits] = dbgInfoWarnErr;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(dbgInfoWarnErr, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoWarnErr = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                         , _colors[static_cast<int>(ColorInfo)]
                                         , _colors[static_cast<int>(ColorWarn)]
                                         , _colors[static_cast<int>(ColorError)]
                                         , true);
        _mapIcons[prio | NoPrio | SquareBits] = dbgInfoWarnErr;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(dbgInfoWarnErr, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoWarnErr = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                           , _colors[static_cast<int>(ColorInfo)]
                                                           , _colors[static_cast<int>(ColorWarn)]
                                                           , _colors[static_cast<int>(ColorError)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , false);
        _mapIcons[prio] = dbgInfoWarnErr;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgInfoWarnErr, _colors[static_cast<int>(ColorWithScope)]);
        
        dbgInfoWarnErr = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                           , _colors[static_cast<int>(ColorInfo)]
                                                           , _colors[static_cast<int>(ColorWarn)]
                                                           , _colors[static_cast<int>(ColorError)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , true);
        _mapIcons[prio | NoPrio] = dbgInfoWarnErr;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgInfoWarnErr, _colors[static_cast<int>(ColorWithScope)]);
        
        // icon prio all
        QIcon all = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                                    , _colors[static_cast<int>(ColorInfo)]
                                    , _colors[static_cast<int>(ColorWarn)]
                                    , _colors[static_cast<int>(ColorError)]
                                    , false);
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        _mapIcons[prio | SquareBits] = all;
        _mapIcons[prio | prioScope | SquareBits] = _setScope(all, _colors[static_cast<int>(ColorWithScope)]);
        
        all = _createFourIcon(  _colors[static_cast<int>(ColorDebug)]
                              , _colors[static_cast<int>(ColorInfo)]
                              , _colors[static_cast<int>(ColorWarn)]
                              , _colors[static_cast<int>(ColorError)]
                              , true);
        _mapIcons[prio | NoPrio | SquareBits] = all;
        _mapIcons[prio | NoPrio | prioScope | SquareBits] = _setScope(all, _colors[static_cast<int>(ColorWithScope)]);
        
        all = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                , _colors[static_cast<int>(ColorInfo)]
                                                , _colors[static_cast<int>(ColorWarn)]
                                                , _colors[static_cast<int>(ColorError)]
                                                , _colors[static_cast<int>(ColotNotSet)]
                                                , false);
        _mapIcons[prio] = all;
        _mapIcons[prio | prioScope] = _setScopeRound(all, _colors[static_cast<int>(ColorWithScope)]);
        
        all = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                , _colors[static_cast<int>(ColorInfo)]
                                                , _colors[static_cast<int>(ColorWarn)]
                                                , _colors[static_cast<int>(ColorError)]
                                                , _colors[static_cast<int>(ColotNotSet)]
                                                , true);
        _mapIcons[prio | NoPrio] = all;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(all, _colors[static_cast<int>(ColorWithScope)]);
    }
}

QIcon LogScopeIconFactory::getIcon(uint32_t scopePrio)
{
    _initialize();
    uint32_t prio = (scopePrio & static_cast<uint32_t>(NELogging::eLogPriority::PrioValidLogs));
    // if ((prio != static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset)) && (prio & static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset)) != 0)
    //     prio &= ~static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset);
    
    Q_ASSERT(_mapIcons.contains(prio));
    if (prio == static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset))
    {
        return _mapIcons[prio];
    }
    else
    {
        Q_ASSERT(_mapIcons.contains(prio));
        return _mapIcons[prio];
    }
}

QColor LogScopeIconFactory::getColor(NELogging::eLogPriority logPrio)
{
    switch (logPrio)
    {
    case NELogging::eLogPriority::PrioFatal:
        return _colors[static_cast<int>(ColorFatal)];
    case NELogging::eLogPriority::PrioError:
        return _colors[static_cast<int>(ColorError)];
    case NELogging::eLogPriority::PrioWarning:
        return _colors[static_cast<int>(ColorWarn)];
    case NELogging::eLogPriority::PrioInfo:
        return _colors[static_cast<int>(ColorInfo)];
    case NELogging::eLogPriority::PrioDebug:
        return _colors[static_cast<int>(ColorDebug)];
    case NELogging::eLogPriority::PrioScope:
        return _colors[static_cast<int>(ColorScope)];
    case NELogging::eLogPriority::PrioNotset:
    default:
        return _colors[static_cast<int>(ColotNotSet)];
    }
}
