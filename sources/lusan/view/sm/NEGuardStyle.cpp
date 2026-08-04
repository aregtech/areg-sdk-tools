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
 *  \file        lusan/view/sm/NEGuardStyle.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard editor visual tokens.
 *
 ************************************************************************/

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QPalette>

bool NEGuardStyle::isDark()
{
    // The theme installs a dark palette; the system default is judged by window lightness.
    return (QApplication::palette().color(QPalette::Window).lightness() < 128);
}

QColor NEGuardStyle::ownerColor(eOwner owner)
{
    const bool dark = isDark();
    switch (owner)
    {
    case eOwner::Stimulus:  return dark ? QColor(0x80, 0xB2, 0xF6) : QColor(0x2F, 0x6F, 0xD0);
    case eOwner::Fsm:       return dark ? QColor(0x53, 0xC1, 0xA0) : QColor(0x1F, 0x8A, 0x70);
    case eOwner::Handler:   return dark ? QColor(0xE6, 0xA0, 0x70) : QColor(0xC0, 0x5A, 0x1F);
    case eOwner::Literal:   return dark ? QColor(0xB0, 0xB0, 0xB0) : QColor(0x6A, 0x6A, 0x6A);
    case eOwner::Raw:       return dark ? QColor(0xB0, 0xB0, 0xB0) : QColor(0x6A, 0x6A, 0x6A);
    case eOwner::Operator:
    default:                return QApplication::palette().color(QPalette::WindowText);
    }
}

QColor NEGuardStyle::severityColor(eSeverity severity)
{
    // The dark severity hues clear WCAG AA against the lightest dark surface. The underline has no
    // glyph channel, so the status-line icon and text carry the distinction where hues are close.
    const bool dark = isDark();
    switch (severity)
    {
    case eSeverity::Ok:     return dark ? QColor(0x72, 0xC1, 0x76) : QColor(0x2E, 0x7D, 0x32);
    case eSeverity::Warn:   return dark ? QColor(0xD8, 0xB0, 0x40) : QColor(0xB8, 0x86, 0x0B);
    case eSeverity::Err:
    default:                return dark ? QColor(0xEA, 0x9B, 0x92) : QColor(0xC0, 0x39, 0x2B);
    }
}

QColor NEGuardStyle::autoMapTint()
{
    return isDark() ? QColor(0x27, 0x3A, 0x52) : QColor(0xDC, 0xE9, 0xF9);
}

QColor NEGuardStyle::unmappedTint()
{
    // A warm amber wash, deliberately distinct from the blue auto-map tint (a slot that IS filled,
    // just unvisited). Both values sit behind dark text/light text without dropping legibility.
    return isDark() ? QColor(0x5A, 0x48, 0x1C) : QColor(0xFB, 0xEE, 0xC8);
}

QString NEGuardStyle::ownerGlyph(eOwner owner)
{
    switch (owner)
    {
    case eOwner::Stimulus:  return QStringLiteral("a");
    case eOwner::Fsm:       return QStringLiteral("f");
    case eOwner::Handler:   return QStringLiteral("h");
    case eOwner::Raw:       return QStringLiteral("r");
    case eOwner::Literal:   return QStringLiteral("=");
    case eOwner::Operator:
    default:                return QString();
    }
}

QString NEGuardStyle::ownerMeaning(eOwner owner)
{
    switch (owner)
    {
    case eOwner::Stimulus:
        return QCoreApplication::translate("NEGuardStyle", "an argument of the stimulus that fires this transition");
    case eOwner::Fsm:
        return QCoreApplication::translate("NEGuardStyle", "declared by the machine itself: an attribute, a constant, or a named lambda");
    case eOwner::Handler:
        return QCoreApplication::translate("NEGuardStyle", "a condition method your handler implements");
    case eOwner::Raw:
        return QCoreApplication::translate("NEGuardStyle", "verbatim C++: Lusan never parses it, so nothing inside is checked");
    case eOwner::Literal:
        return QCoreApplication::translate("NEGuardStyle", "a literal value written in place");
    case eOwner::Operator:
    default:
        return QString();
    }
}
