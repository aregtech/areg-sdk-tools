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
    };

    const QColor _colors[] =
    {
          QColog(Qt::transparent)   // ColorNotSet
        , QColor(Qt::darkGray)      // ColorScope
        , QColor(Qt::darkRed)       // ColorFatal
        , QColor(Qt::red)           // ColorError
        , QColor(Qt::darkYellow)    // ColorWarn
        , QColor(Qt::darkBlue)      // ColorInfo
        , QColor(Qt::darkGreen)     // ColorDebug
    };
    
    QIcon _scopeIcons[static_cast<int>(eScopeIcon::IconAll) + 1]{};
    std::atomic_bool _initialized{ false };

    void _createNotSetIcon(void)
    {
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::NoBrush);

        _scopeIcons[static_cast<int>(eScopeIcon::IconNotset)] = pixmap;
    }

    void _createScopeIcon(eScopeIcon icon, const QColor& color)
    {
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(color);
        painter.setBrush(Qt::NoBrush);
        QPolygon polyScope1, polyScope2;
        polyScope1 << QPoint( 0,  0) << QPoint(16, 16);
        polyScope2 << QPoint(16,  0) << QPoint( 0, 16);
        painter.drawLines(polyScope1);
        painter.drawLines(polyScope2);

        _scopeIcons[static_cast<int>(icon)] = pixmap;
    }

    void _createOneIcon(eScopeIcon icon, const QColor & color)
    {
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawRect(0, 0, 16, 16);

        _scopeIcons[static_cast<int>(icon)] = pixmap;
    }

    void _createTwoIcon(eScopeIcon icon, const QColor& color1, const QColor& color2)
    {
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);

        // Top-left triangle
        QPolygon poly2;
        poly2 << QPoint(0, 0) << QPoint(16, 0) << QPoint(0, 16);
        painter.setBrush(color2);
        painter.drawPolygon(poly2);

        // Bottom-right triangle
        QPolygon poly1;
        poly1 << QPoint(16, 16) << QPoint(16, 0) << QPoint(0, 16);
        painter.setBrush(color1);
        painter.drawPolygon(poly1);

        _scopeIcons[static_cast<int>(icon)] = pixmap;
    }

    void _createFourIcon(eScopeIcon icon, const QColor& color1, const QColor& color2, const QColor& color3, const QColor& color4)
    {
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);

        // Top-left triangle
        QPolygon poly4;
        poly4 << QPoint(0, 0) << QPoint(8, 0) << QPoint(0, 8);
        painter.setBrush(color4);
        painter.drawPolygon(poly4);
        
        // Top-right triangle
        QPolygon poly3;
        poly3 << QPoint(0, 0) << QPoint(16, 0) << QPoint(8, 8);
        painter.setBrush(color3);
        painter.drawPolygon(poly3);
        
        // Bottom-left triangle
        QPolygon poly2;
        poly2 << QPoint(16, 0) << QPoint(16, 16) << QPoint(8, 8);
        painter.setBrush(color2);
        painter.drawPolygon(poly2);
        
        // Bottom-right triangle
        QPolygon poly1;
        poly1 << QPoint(16, 16) << QPoint(0, 16) << QPoint(8, 8);
        painter.setBrush(color1);
        painter.drawPolygon(poly1);
        
        _scopeIcons[static_cast<int>(icon)] = pixmap;
    }
    
    QIcon _setScope(const QIcon& icon)
    {
        const QIcon& scope = _scopeIcons[static_cast<int>(eScopeIcon::IconScope)];
        QPixmap pixScope = scope.pixmap(16, 16);
        QPixmap pixIcon  = icon.pixmap(16, 16);
        
        for (int i = 0; i < 16; ++i)
        {
            for (int j = 0; j < 16; ++j)
            {
                pixScope.
            }
        }

    }
    
    void _initialize(void)
    {
        if (_initialized.exchange(true))
            return;

        _createNotSetIcon();
        _createScopeIcon(eScopeIcon::IconScope, _colors[static_cast<int>(ColorScope]);

        _createOneIcon(eScopeIcon::IconFatal, _colors[static_cast<int>(ColorFatal)]);
        _createOneIcon(eScopeIcon::IconError, _colors[static_cast<int>(ColorError)]);
        _createOneIcon(eScopeIcon::IconWarn , _colors[static_cast<int>(ColorWarn)]);
        _createOneIcon(eScopeIcon::IconInfo , _colors[static_cast<int>(ColorInfo)]);
        _createOneIcon(eScopeIcon::IconDebug, _colors[static_cast<int>(ColorDebug)]);

        _createTwoIcon(eScopeIcon::IconDebugInfo    , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorInfo)]);
        _createTwoIcon(eScopeIcon::IconDebugWarn    , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorWarn)]);
        _createTwoIcon(eScopeIcon::IconDebugError   , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorError)]);
        _createTwoIcon(eScopeIcon::IconDebugFatal   , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorFatal)]);
        _createTwoIcon(eScopeIcon::IconInfoWarn     , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorWarn)]);
        _createTwoIcon(eScopeIcon::IconInfoError    , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorError)]);
        _createTwoIcon(eScopeIcon::IconInfoFatal    , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorFatal)]);
        _createTwoIcon(eScopeIcon::IconWarnError    , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColorError)]);
        _createTwoIcon(eScopeIcon::IconWarnFatal    , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColorFatal)]);
        _createTwoIcon(eScopeIcon::IconErrorFatal   , _colors[static_cast<int>(ColorError)] , _colors[static_cast<int>(ColorFatal)]);


        _createFourIcon(eScopeIcon::IconDebugInfoWarn   , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColotNotSet)]);
        _createFourIcon(eScopeIcon::IconDebugInfoError  , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorError)] , _colors[static_cast<int>(ColotNotSet)]);
        _createFourIcon(eScopeIcon::IconDebugInfoFatal  , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorFatal)] , _colors[static_cast<int>(ColotNotSet)]);
        _createFourIcon(eScopeIcon::IconDebugWarnError  , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColorError)] , _colors[static_cast<int>(ColotNotSet)]);
        _createFourIcon(eScopeIcon::IconDebugWarnFatal  , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColorFatal)] , _colors[static_cast<int>(ColotNotSet)]);
        _createFourIcon(eScopeIcon::IconDebugErrorFatal , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorError)] , _colors[static_cast<int>(ColorFatal)] , _colors[static_cast<int>(ColotNotSet)]);
        _createFourIcon(eScopeIcon::IconInfoWarnError   , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColorError)] , _colors[static_cast<int>(ColotNotSet)]);
        _createFourIcon(eScopeIcon::IconInfoWarnFatal   , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColorFatal)] , _colors[static_cast<int>(ColotNotSet)]);
        _createFourIcon(eScopeIcon::IconInfoErrorFatal  , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorError)] , _colors[static_cast<int>(ColorFatal)] , _colors[static_cast<int>(ColotNotSet)]);
        _createFourIcon(eScopeIcon::IconWarnErrorFatal  , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColorError)] , _colors[static_cast<int>(ColorFatal)] , _colors[static_cast<int>(ColotNotSet)]);

        _createFourIcon(eScopeIcon::IconDebugInfoWarnError  , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColorError)]);
        _createFourIcon(eScopeIcon::IconAll                 , _colors[static_cast<int>(ColorDebug)] , _colors[static_cast<int>(ColorInfo)]  , _colors[static_cast<int>(ColorWarn)]  , _colors[static_cast<int>(ColorError)]);
    }
    
}
