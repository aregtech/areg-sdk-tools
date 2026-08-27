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
 *  \file        lusan/common/NELogPalette.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the colours of the log windows.
 *
 ************************************************************************/

#include "lusan/common/NELogPalette.hpp"

#include <QApplication>
#include <QPalette>

namespace
{
    constexpr int _roleCount{ static_cast<int>(NELogPalette::eLogColorRole::RoleCount) };

    //! Message text, light theme. Every value clears 4.5:1 against white.
    const QColor _textLight[_roleCount]
    {
          QColor(0x7F, 0x00, 0x12)      // Fatal
        , QColor(0xC6, 0x28, 0x28)      // Error
        , QColor(0x8A, 0x5B, 0x00)      // Warning
        , QColor(0x15, 0x65, 0xC0)      // Information
        , QColor(0x00, 0x69, 0x5C)      // Debug
        , QColor(0x5B, 0x3F, 0xD6)      // Scope
        , QColor(0x5B, 0x64, 0x72)      // Not set
    };

    //! Message text, dark theme. Every value clears 4.5:1 against the dark base.
    const QColor _textDark[_roleCount]
    {
          QColor(0xFF, 0xD7, 0xD2)      // Fatal
        , QColor(0xFF, 0x7A, 0x70)      // Error
        , QColor(0xFF, 0xC2, 0x4D)      // Warning
        , QColor(0x7A, 0xB0, 0xFF)      // Information
        , QColor(0x4D, 0xD0, 0xB1)      // Debug
        , QColor(0xC4, 0xB2, 0xFF)      // Scope
        , QColor(0x9A, 0xA5, 0xB5)      // Not set
    };

    //! The priority rail, light theme. Saturated, because a 4 px bar carries less ink than a letter.
    const QColor _railLight[_roleCount]
    {
          QColor(0xB3, 0x00, 0x1B)      // Fatal
        , QColor(0xE5, 0x39, 0x35)      // Error
        , QColor(0xF2, 0xA1, 0x00)      // Warning
        , QColor(0x2F, 0x80, 0xED)      // Information
        , QColor(0x14, 0x90, 0x7F)      // Debug
        , QColor(0x7A, 0x5A, 0xF8)      // Scope
        , QColor(Qt::transparent)       // Not set
    };

    //! The priority rail, dark theme.
    const QColor _railDark[_roleCount]
    {
          QColor(0xFF, 0x52, 0x52)      // Fatal
        , QColor(0xF4, 0x56, 0x4B)      // Error
        , QColor(0xFF, 0xB0, 0x20)      // Warning
        , QColor(0x4F, 0x8C, 0xFF)      // Information
        , QColor(0x2E, 0xC4, 0xA6)      // Debug
        , QColor(0xA7, 0x8B, 0xFA)      // Scope
        , QColor(Qt::transparent)       // Not set
    };

    //! Row background. Only Fatal has one: 12 % of the Fatal rail colour over the
    //! theme base, so the band carries the same strength in both themes.
    const QColor _bandLight{ 0xF6, 0xE0, 0xE4 };
    const QColor _bandDark { 0x40, 0x30, 0x38 };

    constexpr int _opacityCount{ static_cast<int>(NELogPalette::eLogOpacity::OpacityCount) };

    //! Tint, hover, ghost track. The ghost differs most between the themes: at one
    //! value it recedes much further on white than it does on a dark base.
    const qreal _opacityLight[_opacityCount]{ 0.15, 0.12, 0.28 };
    const qreal _opacityDark [_opacityCount]{ 0.20, 0.12, 0.20 };

    inline int roleIndex(NELogPalette::eLogColorRole role)
    {
        const int index{ static_cast<int>(role) };
        return ((index >= 0) && (index < _roleCount)) ? index : static_cast<int>(NELogPalette::eLogColorRole::RoleNotset);
    }

    inline int opacityIndex(NELogPalette::eLogOpacity use)
    {
        const int index{ static_cast<int>(use) };
        return ((index >= 0) && (index < _opacityCount)) ? index : static_cast<int>(NELogPalette::eLogOpacity::OpacityTint);
    }
}

bool NELogPalette::isDarkTheme(void)
{
    const QPalette palette{ QApplication::palette() };
    return palette.color(QPalette::Base).lightness() < 128;
}

qreal NELogPalette::opacity(NELogPalette::eLogOpacity use)
{
    return isDarkTheme() ? _opacityDark[opacityIndex(use)] : _opacityLight[opacityIndex(use)];
}

QColor NELogPalette::withOpacity(const QColor & color, NELogPalette::eLogOpacity use)
{
    QColor result{ color };
    result.setAlphaF(NELogPalette::opacity(use));
    return result;
}

QColor NELogPalette::textColor(NELogPalette::eLogColorRole role)
{
    return isDarkTheme() ? _textDark[roleIndex(role)] : _textLight[roleIndex(role)];
}

QColor NELogPalette::railColor(NELogPalette::eLogColorRole role)
{
    return isDarkTheme() ? _railDark[roleIndex(role)] : _railLight[roleIndex(role)];
}

QColor NELogPalette::rowBackground(NELogPalette::eLogColorRole role)
{
    if (role != NELogPalette::eLogColorRole::RoleFatal)
        return QColor(Qt::transparent);

    return isDarkTheme() ? _bandDark : _bandLight;
}

NELogPalette::eLogColorRole NELogPalette::roleOf(areg::LogPriority prio)
{
    switch (prio)
    {
    case areg::LogPriority::PrioFatal:
        return NELogPalette::eLogColorRole::RoleFatal;
    case areg::LogPriority::PrioError:
        return NELogPalette::eLogColorRole::RoleError;
    case areg::LogPriority::PrioWarning:
        return NELogPalette::eLogColorRole::RoleWarning;
    case areg::LogPriority::PrioInfo:
        return NELogPalette::eLogColorRole::RoleInformation;
    case areg::LogPriority::PrioDebug:
        return NELogPalette::eLogColorRole::RoleDebug;
    case areg::LogPriority::PrioScope:
        return NELogPalette::eLogColorRole::RoleScope;
    default:
        return NELogPalette::eLogColorRole::RoleNotset;
    }
}

NELogPalette::eLogColorRole NELogPalette::roleOf(const areg::LogEntry & entry)
{
    return NELogPalette::roleOf(entry.logMessagePrio);
}
