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
#include <QCoreApplication>
#include <QPalette>

namespace
{
    constexpr int _roleCount{ static_cast<int>(NELogPalette::eLogColorRole::RoleCount) };

    constexpr int _paletteCount{ static_cast<int>(NELogPalette::eLogPalette::PaletteCount) };

    //! Message text, light theme, one row per colour set. Ladder gives every priority its own
    //! ink and lets chroma and contrast fall down the list. Quiet colours only the three
    //! alarms and steps the rest down in the neutral ink. Classic is the set that shipped
    //! before, where Debug carries the weight of ordinary text.
    const QColor _textLight[_paletteCount][_roleCount]
    {
        {   QColor(0x8C, 0x0F, 0x28)    // Fatal
        ,   QColor(0xC6, 0x28, 0x28)    // Error
        ,   QColor(0x8A, 0x5A, 0x00)    // Warning
        ,   QColor(0x15, 0x65, 0xC0)    // Information
        ,   QColor(0x35, 0x78, 0x6F)    // Debug
        ,   QColor(0x5A, 0x66, 0x75)    // Scope
        ,   QColor(0x5B, 0x64, 0x72)    // Not set
        ,   QColor(0x5A, 0x66, 0x75)    // Scope enter
        ,   QColor(0x81, 0x8C, 0x9A)    // Scope exit
        },
        {   QColor(0x8C, 0x0F, 0x28)    // Fatal
        ,   QColor(0xC6, 0x28, 0x28)    // Error
        ,   QColor(0x8A, 0x5A, 0x00)    // Warning
        ,   QColor(0x33, 0x40, 0x4F)    // Information
        ,   QColor(0x5A, 0x66, 0x75)    // Debug
        ,   QColor(0x6B, 0x76, 0x86)    // Scope
        ,   QColor(0x5B, 0x64, 0x72)    // Not set
        ,   QColor(0x6B, 0x76, 0x86)    // Scope enter
        ,   QColor(0x8A, 0x94, 0xA2)    // Scope exit
        },
        {   QColor(0x7A, 0x00, 0x12)    // Fatal
        ,   QColor(0xD3, 0x2F, 0x2F)    // Error
        ,   QColor(0x8A, 0x6A, 0x00)    // Warning
        ,   QColor(0x15, 0x65, 0xC0)    // Information
        ,   QColor(0x23, 0x30, 0x3F)    // Debug
        ,   QColor(0x8A, 0x93, 0xA1)    // Scope
        ,   QColor(0x5B, 0x64, 0x72)    // Not set
        ,   QColor(0x8A, 0x93, 0xA1)    // Scope enter
        ,   QColor(0x8A, 0x93, 0xA1)    // Scope exit
        }
    };

    //! Message text, dark theme. The same three sets, read against a dark base.
    const QColor _textDark[_paletteCount][_roleCount]
    {
        {   QColor(0xFF, 0xB4, 0xAB)    // Fatal
        ,   QColor(0xFF, 0x7A, 0x6D)    // Error
        ,   QColor(0xF0, 0xB4, 0x29)    // Warning
        ,   QColor(0x7A, 0xB0, 0xFF)    // Information
        ,   QColor(0x64, 0xC7, 0xB6)    // Debug
        ,   QColor(0xA6, 0xB0, 0xBE)    // Scope
        ,   QColor(0x9A, 0xA5, 0xB5)    // Not set
        ,   QColor(0xA6, 0xB0, 0xBE)    // Scope enter
        ,   QColor(0x76, 0x7F, 0x8C)    // Scope exit
        },
        {   QColor(0xFF, 0xB4, 0xAB)    // Fatal
        ,   QColor(0xFF, 0x7A, 0x6D)    // Error
        ,   QColor(0xF0, 0xB4, 0x29)    // Warning
        ,   QColor(0xD8, 0xDE, 0xE9)    // Information
        ,   QColor(0xAE, 0xB7, 0xC4)    // Debug
        ,   QColor(0x96, 0xA0, 0xAE)    // Scope
        ,   QColor(0x9A, 0xA5, 0xB5)    // Not set
        ,   QColor(0x96, 0xA0, 0xAE)    // Scope enter
        ,   QColor(0x78, 0x82, 0x8F)    // Scope exit
        },
        {   QColor(0xFF, 0xD7, 0xD2)    // Fatal
        ,   QColor(0xFF, 0x6B, 0x5E)    // Error
        ,   QColor(0xF0, 0xB4, 0x29)    // Warning
        ,   QColor(0x7A, 0xB0, 0xFF)    // Information
        ,   QColor(0xD8, 0xDE, 0xE9)    // Debug
        ,   QColor(0x79, 0x82, 0x8F)    // Scope
        ,   QColor(0x9A, 0xA5, 0xB5)    // Not set
        ,   QColor(0x79, 0x82, 0x8F)    // Scope enter
        ,   QColor(0x79, 0x82, 0x8F)    // Scope exit
        }
    };

    //! The priority rail, light theme. Saturated, because a 4 px bar carries less ink than a
    //! letter. The rail is the same in every colour set: it always says what the row is.
    const QColor _railLight[_roleCount]
    {
          QColor(0xB3, 0x00, 0x1B)      // Fatal
        , QColor(0xE5, 0x39, 0x35)      // Error
        , QColor(0xE9, 0xA4, 0x00)      // Warning
        , QColor(0x2F, 0x80, 0xED)      // Information
        , QColor(0x14, 0x90, 0x7F)      // Debug
        , QColor(0x6C, 0x7A, 0x93)      // Scope
        , QColor(Qt::transparent)       // Not set
        , QColor(0x6C, 0x7A, 0x93)      // Scope enter
        , QColor(0xA3, 0xAD, 0xBD)      // Scope exit
    };

    //! The priority rail, dark theme.
    const QColor _railDark[_roleCount]
    {
          QColor(0xFF, 0x52, 0x52)      // Fatal
        , QColor(0xF4, 0x56, 0x4B)      // Error
        , QColor(0xFF, 0xB0, 0x20)      // Warning
        , QColor(0x4F, 0x8C, 0xFF)      // Information
        , QColor(0x2E, 0xC4, 0xA6)      // Debug
        , QColor(0x7E, 0x8C, 0xA3)      // Scope
        , QColor(Qt::transparent)       // Not set
        , QColor(0x88, 0x95, 0xA8)      // Scope enter
        , QColor(0x59, 0x63, 0x6F)      // Scope exit
    };

    //! The colour set in use. Replaced whole when the user picks another one.
    NELogPalette::eLogPalette _activePalette{ NELogPalette::DefaultPalette };

    inline int paletteIndex(NELogPalette::eLogPalette forPalette)
    {
        const int index{ static_cast<int>(forPalette) };
        return ((index >= 0) && (index < _paletteCount)) ? index : static_cast<int>(NELogPalette::DefaultPalette);
    }

    //! Row background. Only Fatal has one: 12 % of the Fatal rail colour over the
    //! theme base, so the band carries the same strength in both themes.
    const QColor _bandLight{ 0xF6, 0xE0, 0xE4 };
    const QColor _bandDark { 0x40, 0x30, 0x38 };

    constexpr int _markCount{ static_cast<int>(NELogPalette::eLogMarkRole::MarkCount) };

    //! Search hit, revealed row and its gutter mark, light theme. One amber hue at three
    //! strengths, so a revealed row reads as a consequence of the search that found it.
    const QColor _markLight[_markCount]
    {
          QColor(0xFF, 0xD5, 0x4F)      // Hit background
        , QColor(0x2A, 0x21, 0x00)      // Hit text
        , QColor(0xFF, 0xF6, 0xDC)      // Revealed row
        , QColor(0xE0, 0x9B, 0x00)      // Revealed gutter mark
    };

    //! The same three, dark theme.
    const QColor _markDark[_markCount]
    {
          QColor(0x8A, 0x63, 0x00)      // Hit background
        , QColor(0xFF, 0xF3, 0xC4)      // Hit text
        , QColor(0x3A, 0x31, 0x16)      // Revealed row
        , QColor(0xFF, 0xC2, 0x4D)      // Revealed gutter mark
    };

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

    inline int markIndex(NELogPalette::eLogMarkRole role)
    {
        const int index{ static_cast<int>(role) };
        return ((index >= 0) && (index < _markCount)) ? index : 0;
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

NELogPalette::eLogPalette NELogPalette::palette(void)
{
    return _activePalette;
}

void NELogPalette::setPalette(NELogPalette::eLogPalette newPalette)
{
    _activePalette = (static_cast<int>(newPalette) < _paletteCount) ? newPalette : NELogPalette::DefaultPalette;
}

QString NELogPalette::paletteName(NELogPalette::eLogPalette forPalette)
{
    switch (forPalette)
    {
    case NELogPalette::eLogPalette::PaletteQuiet:
        return QCoreApplication::translate("NELogPalette", "Quiet");
    case NELogPalette::eLogPalette::PaletteClassic:
        return QCoreApplication::translate("NELogPalette", "Classic");
    case NELogPalette::eLogPalette::PaletteLadder:
    default:
        return QCoreApplication::translate("NELogPalette", "Ladder");
    }
}

QString NELogPalette::paletteHint(NELogPalette::eLogPalette forPalette)
{
    switch (forPalette)
    {
    case NELogPalette::eLogPalette::PaletteQuiet:
        return QCoreApplication::translate("NELogPalette", "Only Fatal, Error and Warning are coloured. The quietest table to hunt a failure in.");
    case NELogPalette::eLogPalette::PaletteClassic:
        return QCoreApplication::translate("NELogPalette", "Debug takes the ordinary text colour, and a scope keeps one grey for both of its ends.");
    case NELogPalette::eLogPalette::PaletteLadder:
    default:
        return QCoreApplication::translate("NELogPalette", "Every priority owns a colour, graded so the loud ones still stand out.");
    }
}

QString NELogPalette::paletteKey(NELogPalette::eLogPalette forPalette)
{
    switch (forPalette)
    {
    case NELogPalette::eLogPalette::PaletteQuiet:
        return QStringLiteral("Quiet");
    case NELogPalette::eLogPalette::PaletteClassic:
        return QStringLiteral("Classic");
    case NELogPalette::eLogPalette::PaletteLadder:
    default:
        return QStringLiteral("Ladder");
    }
}

NELogPalette::eLogPalette NELogPalette::paletteFromKey(const QString& key)
{
    if (key.compare(QStringLiteral("Quiet"), Qt::CaseInsensitive) == 0)
        return NELogPalette::eLogPalette::PaletteQuiet;
    else if (key.compare(QStringLiteral("Classic"), Qt::CaseInsensitive) == 0)
        return NELogPalette::eLogPalette::PaletteClassic;
    else
        return NELogPalette::eLogPalette::PaletteLadder;
}

QColor NELogPalette::textColor(NELogPalette::eLogColorRole role)
{
    return NELogPalette::textColorOf(role, _activePalette, isDarkTheme());
}

QColor NELogPalette::textColorOf(NELogPalette::eLogColorRole role, NELogPalette::eLogPalette forPalette, bool dark)
{
    const int set{ paletteIndex(forPalette) };
    return dark ? _textDark[set][roleIndex(role)] : _textLight[set][roleIndex(role)];
}

QColor NELogPalette::railColor(NELogPalette::eLogColorRole role)
{
    return NELogPalette::railColorOf(role, isDarkTheme());
}

QColor NELogPalette::railColorOf(NELogPalette::eLogColorRole role, bool dark)
{
    return dark ? _railDark[roleIndex(role)] : _railLight[roleIndex(role)];
}

QColor NELogPalette::markColor(NELogPalette::eLogMarkRole role)
{
    return isDarkTheme() ? _markDark[markIndex(role)] : _markLight[markIndex(role)];
}

QColor NELogPalette::stateColor(NELogPalette::eLogStateRole role)
{
    switch (role)
    {
    case NELogPalette::eLogStateRole::StatePending:
        return NELogPalette::railColor(NELogPalette::eLogColorRole::RoleWarning);

    case NELogPalette::eLogStateRole::StateSent:
        return NELogPalette::railColor(NELogPalette::eLogColorRole::RoleInformation);

    case NELogPalette::eLogStateRole::StateSaved:
        return NELogPalette::railColor(NELogPalette::eLogColorRole::RoleDebug);

    default:
        return QColor(Qt::transparent);
    }
}

QColor NELogPalette::rowBackground(NELogPalette::eLogColorRole role)
{
    return NELogPalette::rowBackgroundOf(role, isDarkTheme());
}

QColor NELogPalette::rowBackgroundOf(NELogPalette::eLogColorRole role, bool dark)
{
    if (role != NELogPalette::eLogColorRole::RoleFatal)
        return QColor(Qt::transparent);

    return dark ? _bandDark : _bandLight;
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
    if (entry.logMessagePrio == areg::LogPriority::PrioScope)
    {
        if (entry.logMsgType == areg::LogMessageType::ScopeEnter)
            return NELogPalette::eLogColorRole::RoleScopeEnter;
        else if (entry.logMsgType == areg::LogMessageType::ScopeExit)
            return NELogPalette::eLogColorRole::RoleScopeExit;
    }

    return NELogPalette::roleOf(entry.logMessagePrio);
}
