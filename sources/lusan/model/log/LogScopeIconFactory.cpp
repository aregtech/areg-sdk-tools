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
#include <QPainterPath>
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

    constexpr uint32_t LogActive{ 0x00FF0000u };
    
    std::atomic_bool        _initialized{ false };
    std::atomic_bool        _logInit    { false };
    QMap<uint32_t, QIcon>   _mapIcons;
    QMap<uint32_t, QIcon>   _logIcons;
    
    const QColor NoColor            { Qt::transparent };
    constexpr int IconBegin         { 0 };
    constexpr int IconEnd           { LogScopeIconFactory::IconPixels - IconBegin };
    constexpr int IconMiddle        { (IconEnd - IconBegin) / 2 };
    constexpr uint32_t NoPrio       { static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset) };
    constexpr int32_t  AlphaSolid   { 255 };
    constexpr int32_t  AlphaMixed   { 75 };

    QIcon _createRoundFourIcon(QColor color1, QColor color2, QColor color3, QColor color4, bool setAlpha, uint32_t pixels);

    QIcon _createNotSetIcon(uint32_t pixels)
    {
        pixels = pixels == 0 ? LogScopeIconFactory::IconPixels : pixels;
        QPixmap pixmap(pixels, pixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::NoBrush);
        
        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createScopeIcon(const QColor& color, uint32_t pixels)
    {
        pixels = pixels == 0 ? LogScopeIconFactory::IconPixels : pixels;
        QPixmap pixIcon(pixels, pixels);
        pixIcon.fill(Qt::transparent);
        QPainter painter(&pixIcon);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(Qt::NoBrush);
        
        // Line from top to bottom
        painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, 2), QPointF(pixels - 2, pixels - 2));
        // Line from left to right
        painter.setPen(QPen(color, 3, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, pixels - 2), QPointF(pixels - 2, 2));
        
        painter.end();
        return QIcon(pixIcon);
    }

    QIcon _createOneIcon(QColor color, bool setAlpha, uint32_t pixels)
    {
#if 0
        pixels = pixels == 0 ? LogScopeIconFactory::IconPixels : pixels;
        QPixmap pixmap(pixels, pixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);
        color.setAlpha(setAlpha && (color != NoColor) ? AlphaMixed : AlphaSolid);
        painter.setBrush(color);
        painter.drawRect(IconBegin, IconBegin, IconEnd, IconEnd);
        
        painter.end();
        return QIcon(pixmap);
#else
        return _createRoundFourIcon(color, color, color, color, setAlpha, pixels);
#endif
    }

    QIcon _createTwoIcon(QColor color1, QColor color2, bool setAlpha, uint32_t pixels)
    {
        pixels = pixels == 0 ? LogScopeIconFactory::IconPixels : pixels;
        QPixmap pixmap(pixels, pixels);
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

    QIcon _createFourIcon(QColor color1, QColor color2, QColor color3, QColor color4, bool setAlpha, uint32_t pixels)
    {
        pixels = pixels == 0 ? LogScopeIconFactory::IconPixels : pixels;
        QPixmap pixmap(pixels, pixels);
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
    
    QIcon _setScope(const QIcon& icon, QColor color, uint32_t pixels)
    {
        pixels = pixels == 0 ? LogScopeIconFactory::IconPixels : pixels;
        QPixmap pixIcon  = icon.pixmap(pixels, pixels);
        QPainter painter(&pixIcon);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(Qt::NoBrush);
        
        // Line from top to bottom
        painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, 2), QPointF(pixels - 2, pixels - 2));
        // Line from left to right
        painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, pixels - 2), QPointF(pixels - 2, 2));
        
        painter.end();
        return QIcon(pixIcon);
    }

    QIcon _setScopeRound(const QIcon& icon, QColor color, uint32_t pixels)
    {
        // constexpr int margin {LogScopeIconFactory::IconPixels / 2 };
        // constexpr int shift  { 0 };

        pixels = pixels == 0 ? LogScopeIconFactory::IconPixels : pixels;
        QPixmap pixmap = icon.pixmap(pixels, pixels);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(Qt::NoBrush);

        // line from top to bottom
        painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, 2), QPointF(pixels - 2, pixels - 2));
        // painter.drawLine(QPointF(0, margin + shift), QPointF(pixels, margin + shift));
        // line from left to right
        painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, pixels - 2), QPointF(pixels - 2, 2));
        // painter.drawLine(QPointF(margin, 0), QPointF(margin, pixels));

        painter.end();
        return QIcon(pixmap);
    }

    QIcon _createRoundFourIcon(QColor color1, QColor color2, QColor color3, QColor color4, bool setAlpha, uint32_t pixels)
    {
        pixels = pixels == 0 ? LogScopeIconFactory::IconPixels : pixels;
        QPixmap pixmap(pixels, pixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QRectF rect(0, 0, pixels, pixels);

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

    QIcon _createRoundFourIconWithDiagonals(QColor color1, QColor color2, QColor color3, QColor color4, QColor colorDiag, bool setAlpha, uint32_t pixels)
    {
        pixels = pixels == 0 ? LogScopeIconFactory::IconPixels : pixels;
        QPixmap pixmap(pixels, pixels);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QRectF rect(0, 0, pixels, pixels);

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
        painter.drawLine(QPointF(2, 2), QPointF(pixels - 2, pixels - 2));
        // Diagonal from left to right
        painter.setPen(QPen(colorDiag, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(2, pixels - 2), QPointF(pixels - 2, 2));

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
    
    void _initLogIcons(uint32_t pixels)
    {
        if (_logInit.exchange(true))
            return;

        uint32_t prio = static_cast<int>(LogScopeIconFactory::eLogIcons::PrioNotset);
        QIcon notset { _createNotSetIcon(pixels) };
        _logIcons[prio] = notset;
        _logIcons[prio | LogActive] = notset;

        prio = static_cast<int>(LogScopeIconFactory::eLogIcons::PrioScope);
        QIcon scope(QIcon::fromTheme(QString::fromUtf8("media-playlist-shuffle")));
        _logIcons[prio] = scope;
        _logIcons[prio | LogActive] = _mergeIcons(_colors[static_cast<int>(ColorScope)], scope, pixels);

        prio = static_cast<int>(LogScopeIconFactory::eLogIcons::PrioDebug);
        QIcon debug(QIcon::fromTheme(QString::fromUtf8("format-justify-left")));
        _logIcons[prio] = debug;
        _logIcons[prio | LogActive] = _mergeIcons(_colors[static_cast<int>(ColorDebug)], debug, pixels);

        prio = static_cast<int>(LogScopeIconFactory::eLogIcons::PrioInfo);
        QIcon info(QIcon::fromTheme(QString::fromUtf8("dialog-information")));
        _logIcons[prio] = info;
        _logIcons[prio | LogActive] = _mergeIcons(_colors[static_cast<int>(ColorInfo)], info, pixels);

        prio = static_cast<int>(LogScopeIconFactory::eLogIcons::PrioWarn);
        QIcon warn(QIcon::fromTheme(QString::fromUtf8("dialog-warning")));
        _logIcons[prio] = warn;
        _logIcons[prio | LogActive] = _mergeIcons(_colors[static_cast<int>(ColorWarn)], warn, pixels);

        prio = static_cast<int>(LogScopeIconFactory::eLogIcons::PrioError);
        QIcon error(QIcon::fromTheme(QString::fromUtf8("dialog-error")));
        _logIcons[prio] = error;
        _logIcons[prio | LogActive] = _mergeIcons(_colors[static_cast<int>(ColorError)], error, pixels);

        prio = static_cast<int>(LogScopeIconFactory::eLogIcons::PrioFatal);
        QIcon fatal(QIcon::fromTheme(QString::fromUtf8("media-optical")));
        _logIcons[prio] = fatal;
        _logIcons[prio | LogActive] = _mergeIcons(_colors[static_cast<int>(ColorFatal)], fatal, pixels);

        prio = static_cast<int>(LogScopeIconFactory::eLogIcons::PrioScopeEnter);
        QIcon enter(QIcon::fromTheme(QString::fromUtf8("go-up")));
        do
        {
            QPixmap pixmap = enter.pixmap(pixels, pixels);
            QTransform trans;
            trans.rotate(90);
            pixmap = pixmap.transformed(trans, Qt::SmoothTransformation);
            enter = QIcon(pixmap);
        } while (false);
        _logIcons[prio] = enter;
        _logIcons[prio | LogActive] = _mergeIcons(_colors[static_cast<int>(ColorScope)], enter, pixels);

        prio = static_cast<int>(LogScopeIconFactory::eLogIcons::PrioScopeExit);
        QIcon exit(QIcon::fromTheme(QString::fromUtf8("go-up")));
        do
        {
            QPixmap pixmap = exit.pixmap(pixels, pixels);
            QTransform trans;
            trans.rotate(-90);
            pixmap = pixmap.transformed(trans, Qt::SmoothTransformation);
            exit = QIcon(pixmap);
        } while (false);
        _logIcons[prio] = exit;
        _logIcons[prio | LogActive] = _mergeIcons(_colors[static_cast<int>(ColorScope)], exit, pixels);
    }

    void _initialize(uint32_t pixels)
    {
        if (_initialized.exchange(true))
            return;
        
        Q_ASSERT(_mapIcons.isEmpty());

        // Icon no prio
        uint32_t prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid);
        QIcon notSet = _createNotSetIcon(pixels);
        _mapIcons[static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid)]  = notSet;
        _mapIcons[static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset)]   = notSet;

        // icon prio scope
        QIcon scope = _createScopeIcon(_colors[static_cast<int>(ColorScope)], pixels);
        uint32_t prioScope = static_cast<uint32_t>(NELogging::eLogPriority::PrioScope);
        _mapIcons[prioScope] = scope;
        _mapIcons[prioScope | NoPrio] = scope;

        // icon prio fatal
        QIcon fatal;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);        
        fatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , false, pixels);
        _mapIcons[prio] = fatal;
        _mapIcons[prio | prioScope] = _setScopeRound(fatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        fatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColorFatal)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , true, pixels);
        _mapIcons[prio | NoPrio] = fatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(fatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio error
        QIcon error;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
        error = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , false, pixels);
        _mapIcons[prio] = error;
        _mapIcons[prio | prioScope] = _setScopeRound(error, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        error = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColorError)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , true, pixels);
        _mapIcons[prio | NoPrio] = error;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(error, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio warn
        QIcon warn;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);

        warn  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColotNotSet)]
                                                 , false, pixels);
        _mapIcons[prio] = warn;
        _mapIcons[prio | prioScope] = _setScopeRound(warn, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        warn  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColorWarn)]
                                                 , _colors[static_cast<int>(ColotNotSet)]
                                                 , true, pixels);
        _mapIcons[prio | NoPrio] = warn;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(warn, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio info
        QIcon info;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo);

        info  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColotNotSet)]
                                                 , false, pixels);
        _mapIcons[prio] = info;
        _mapIcons[prio | prioScope] = _setScopeRound(info, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        info  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColorInfo)]
                                                 , _colors[static_cast<int>(ColotNotSet)]
                                                 , true, pixels);
        _mapIcons[prio | NoPrio] = info;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(info, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug
        QIcon debug;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug);

        debug  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , false, pixels);
        _mapIcons[prio] = debug;
        _mapIcons[prio | prioScope] = _setScopeRound(debug, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        debug  = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColorDebug)]
                                                  , _colors[static_cast<int>(ColotNotSet)]
                                                  , true, pixels);
        _mapIcons[prio | NoPrio] = debug;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(debug, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug + info
        QIcon dbgInfo;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo);

        dbgInfo = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , false, pixels);
        _mapIcons[prio] = dbgInfo;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgInfo, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        dbgInfo = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , true, pixels);
        _mapIcons[prio | NoPrio] = dbgInfo;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgInfo, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug + warn
        QIcon dbgWarn;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);

        dbgWarn = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , false, pixels);
        _mapIcons[prio] = dbgWarn;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgWarn, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        dbgWarn = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , true, pixels);
        _mapIcons[prio | NoPrio] = dbgWarn;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgWarn, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug + error
        QIcon dbgError;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);

        dbgError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColorError)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , false, pixels);
        _mapIcons[prio] = dbgError;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        dbgError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                     , _colors[static_cast<int>(ColorDebug)]
                                                     , _colors[static_cast<int>(ColorError)]
                                                     , _colors[static_cast<int>(ColorError)]
                                                     , _colors[static_cast<int>(ColotNotSet)]
                                                     , true, pixels);
        _mapIcons[prio | NoPrio] = dbgError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug + fatal
        QIcon dbgFatal;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);

        dbgFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorDebug)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColorFatal)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , false, pixels);
        _mapIcons[prio] = dbgFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        dbgFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                     , _colors[static_cast<int>(ColorDebug)]
                                                     , _colors[static_cast<int>(ColorFatal)]
                                                     , _colors[static_cast<int>(ColorFatal)]
                                                     , _colors[static_cast<int>(ColotNotSet)]
                                                     , true, pixels);
        _mapIcons[prio | NoPrio] = dbgFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio info + warn
        QIcon infoWarn;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);

        infoWarn = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorInfo)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColorWarn)]
                                                    , _colors[static_cast<int>(ColotNotSet)]
                                                    , false, pixels);
        _mapIcons[prio] = infoWarn;
        _mapIcons[prio | prioScope] = _setScopeRound(infoWarn, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        infoWarn = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorInfo)]
                                                     , _colors[static_cast<int>(ColorInfo)]
                                                     , _colors[static_cast<int>(ColorWarn)]
                                                     , _colors[static_cast<int>(ColorWarn)]
                                                     , _colors[static_cast<int>(ColotNotSet)]
                                                     , true, pixels);
        _mapIcons[prio | NoPrio] = infoWarn;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoWarn, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio info + error
        QIcon infoError;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);

        infoError = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , false, pixels);
        _mapIcons[prio] = infoError;
        _mapIcons[prio | prioScope] = _setScopeRound(infoError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        infoError = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , true, pixels);
        _mapIcons[prio | NoPrio] = infoError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio info + fatal
        QIcon infoFatal;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);

        infoFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , false, pixels);
        _mapIcons[prio] = infoFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(infoFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        infoFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorInfo)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , true, pixels);
        _mapIcons[prio | NoPrio] = infoFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio warn + error
        QIcon warnError;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);

        warnError = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , false, pixels);
        _mapIcons[prio] = warnError;
        _mapIcons[prio | prioScope] = _setScopeRound(warnError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        warnError = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColorError)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , true, pixels);
        _mapIcons[prio | NoPrio] = warnError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(warnError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio warn + fatal
        QIcon warnFatal;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);

        warnFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , false, pixels);
        _mapIcons[prio] = warnFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(warnFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        warnFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorWarn)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColorFatal)]
                                                      , _colors[static_cast<int>(ColotNotSet)]
                                                      , true, pixels);
        _mapIcons[prio | NoPrio] = warnFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(warnFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio error + fatal
        QIcon errorFatal;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);

        errorFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorError)]
                                                       , _colors[static_cast<int>(ColorError)]
                                                       , _colors[static_cast<int>(ColorFatal)]
                                                       , _colors[static_cast<int>(ColorFatal)]
                                                       , _colors[static_cast<int>(ColotNotSet)]
                                                       , false, pixels);
        _mapIcons[prio] = errorFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(errorFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        errorFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorError)]
                                                       , _colors[static_cast<int>(ColorError)]
                                                       , _colors[static_cast<int>(ColorFatal)]
                                                       , _colors[static_cast<int>(ColorFatal)]
                                                       , _colors[static_cast<int>(ColotNotSet)]
                                                       , true, pixels);
        _mapIcons[prio | NoPrio] = errorFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(errorFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug + info + warn
        QIcon dbgInfoWarn;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);

        dbgInfoWarn = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false, pixels);
        _mapIcons[prio] = dbgInfoWarn;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgInfoWarn, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        dbgInfoWarn = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , true, pixels);
        _mapIcons[prio | NoPrio] = dbgInfoWarn;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgInfoWarn, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug + info + error
        QIcon dbgInfoError;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);

        dbgInfoError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false, pixels);
        _mapIcons[prio] = dbgInfoError;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgInfoError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        dbgInfoError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                         , _colors[static_cast<int>(ColorInfo)]
                                                         , _colors[static_cast<int>(ColorError)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , true, pixels);
        _mapIcons[prio | NoPrio] = dbgInfoError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgInfoError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug + info + fatal
        QIcon dbgInfoFatal;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);

        dbgInfoFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false, pixels);
        _mapIcons[prio] = dbgInfoFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgInfoFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        dbgInfoFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                         , _colors[static_cast<int>(ColorInfo)]
                                                         , _colors[static_cast<int>(ColorFatal)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , true, pixels);
        _mapIcons[prio | NoPrio] = dbgInfoFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgInfoFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug + warn + error
        QIcon dbgWarnError;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);

        dbgWarnError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false, pixels);
        _mapIcons[prio] = dbgWarnError;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgWarnError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        dbgWarnError = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                         , _colors[static_cast<int>(ColorWarn)]
                                                         , _colors[static_cast<int>(ColorError)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , true, pixels);
        _mapIcons[prio | NoPrio] = dbgWarnError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgWarnError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug + warn + fatal
        QIcon dbgWarnFatal;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);

        dbgWarnFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false, pixels);
        _mapIcons[prio] = dbgWarnFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgWarnFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        dbgWarnFatal = _createRoundFourIconWithDiagonals( _colors[static_cast<int>(ColorDebug)]
                                                         , _colors[static_cast<int>(ColorWarn)]
                                                         , _colors[static_cast<int>(ColorFatal)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , _colors[static_cast<int>(ColotNotSet)]
                                                         , true, pixels);
        _mapIcons[prio | NoPrio] = dbgWarnFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgWarnFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug + error + fatal
        QIcon dbgErrorFatal;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);

        dbgErrorFatal = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorDebug)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false, pixels);
        _mapIcons[prio] = dbgErrorFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgErrorFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        dbgErrorFatal = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorDebug)]
                                                          , _colors[static_cast<int>(ColorError)]
                                                          , _colors[static_cast<int>(ColorFatal)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , true, pixels);
        _mapIcons[prio | NoPrio] = dbgErrorFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgErrorFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio info + warn + error
        QIcon infoWarnError;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);

        infoWarnError = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorError)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false, pixels);
        _mapIcons[prio] = infoWarnError;
        _mapIcons[prio | prioScope] = _setScopeRound(infoWarnError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        infoWarnError = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorInfo)]
                                                          , _colors[static_cast<int>(ColorWarn)]
                                                          , _colors[static_cast<int>(ColorError)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , true, pixels);
        _mapIcons[prio | NoPrio] = infoWarnError;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoWarnError, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio info + warn + fatal
        QIcon infoWarnFatal;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);

        infoWarnFatal = _createRoundFourIconWithDiagonals(_colors[static_cast<int>(ColorInfo)]
                                                        , _colors[static_cast<int>(ColorWarn)]
                                                        , _colors[static_cast<int>(ColorFatal)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , _colors[static_cast<int>(ColotNotSet)]
                                                        , false, pixels);
        _mapIcons[prio] = infoWarnFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(infoWarnFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        infoWarnFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorInfo)]
                                                          , _colors[static_cast<int>(ColorWarn)]
                                                          , _colors[static_cast<int>(ColorFatal)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , _colors[static_cast<int>(ColotNotSet)]
                                                          , true, pixels);
        _mapIcons[prio | NoPrio] = infoWarnFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoWarnFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio info + error + fatal
        QIcon infoErrorFatal;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);

        infoErrorFatal = _createRoundFourIconWithDiagonals(   _colors[static_cast<int>(ColorInfo)]
                                                            , _colors[static_cast<int>(ColorError)]
                                                            , _colors[static_cast<int>(ColorFatal)]
                                                            , _colors[static_cast<int>(ColotNotSet)]
                                                            , _colors[static_cast<int>(ColotNotSet)]
                                                            , false, pixels);
        _mapIcons[prio] = infoErrorFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(infoErrorFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        infoErrorFatal = _createRoundFourIconWithDiagonals(   _colors[static_cast<int>(ColorInfo)]
                                                           , _colors[static_cast<int>(ColorError)]
                                                           , _colors[static_cast<int>(ColorFatal)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , true, pixels);
        _mapIcons[prio | NoPrio] = infoErrorFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(infoErrorFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio warn + error + fatal
        QIcon warnErrorFatal;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError) | static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal);

        warnErrorFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                           , _colors[static_cast<int>(ColorError)]
                                                           , _colors[static_cast<int>(ColorFatal)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , false, pixels);
        _mapIcons[prio] = warnErrorFatal;
        _mapIcons[prio | prioScope] = _setScopeRound(warnErrorFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        warnErrorFatal = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorWarn)]
                                                           , _colors[static_cast<int>(ColorError)]
                                                           , _colors[static_cast<int>(ColorFatal)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , true, pixels);
        _mapIcons[prio | NoPrio] = warnErrorFatal;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(warnErrorFatal, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio debug + info + warn + error
        QIcon dbgInfoWarnErr;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);

        dbgInfoWarnErr = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                           , _colors[static_cast<int>(ColorInfo)]
                                                           , _colors[static_cast<int>(ColorWarn)]
                                                           , _colors[static_cast<int>(ColorError)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , false, pixels);
        _mapIcons[prio] = dbgInfoWarnErr;
        _mapIcons[prio | prioScope] = _setScopeRound(dbgInfoWarnErr, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        dbgInfoWarnErr = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                           , _colors[static_cast<int>(ColorInfo)]
                                                           , _colors[static_cast<int>(ColorWarn)]
                                                           , _colors[static_cast<int>(ColorError)]
                                                           , _colors[static_cast<int>(ColotNotSet)]
                                                           , true, pixels);
        _mapIcons[prio | NoPrio] = dbgInfoWarnErr;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(dbgInfoWarnErr, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        // icon prio all
        QIcon all;
        prio = static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug) | static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo) | static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning) | static_cast<uint32_t>(NELogging::eLogPriority::PrioError);

        all = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                , _colors[static_cast<int>(ColorInfo)]
                                                , _colors[static_cast<int>(ColorWarn)]
                                                , _colors[static_cast<int>(ColorError)]
                                                , _colors[static_cast<int>(ColotNotSet)]
                                                , false, pixels);
        _mapIcons[prio] = all;
        _mapIcons[prio | prioScope] = _setScopeRound(all, _colors[static_cast<int>(ColorWithScope)], pixels);
        
        all = _createRoundFourIconWithDiagonals(  _colors[static_cast<int>(ColorDebug)]
                                                , _colors[static_cast<int>(ColorInfo)]
                                                , _colors[static_cast<int>(ColorWarn)]
                                                , _colors[static_cast<int>(ColorError)]
                                                , _colors[static_cast<int>(ColotNotSet)]
                                                , true, pixels);
        _mapIcons[prio | NoPrio] = all;
        _mapIcons[prio | NoPrio | prioScope] = _setScopeRound(all, _colors[static_cast<int>(ColorWithScope)], pixels);

    }
}

QIcon LogScopeIconFactory::getIcon(uint32_t scopePrio, uint32_t pixels)
{
    _initialize(pixels);
    uint32_t prio = (scopePrio & static_cast<uint32_t>(NELogging::eLogPriority::PrioValidLogs));
    Q_ASSERT(_mapIcons.contains(prio));
    return _mapIcons[prio];
}

QIcon LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons prio, bool active, uint32_t pixels)
{
    _initLogIcons(pixels);
    uint32_t key  = active ? static_cast<uint32_t>(prio) | LogActive : static_cast<uint32_t>(prio);
    Q_ASSERT(_logIcons.contains(key));
    return _logIcons[key];
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
