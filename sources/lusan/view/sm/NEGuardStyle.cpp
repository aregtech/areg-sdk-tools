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
 *  \brief       Lusan application, FSM guard editor visual tokens (v7 B15, one place).
 *
 ************************************************************************/

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QApplication>
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
    case eOwner::Stimulus:  return dark ? QColor(0x6F, 0xA8, 0xF5) : QColor(0x2F, 0x6F, 0xD0);
    case eOwner::Fsm:       return dark ? QColor(0x4F, 0xC0, 0x9E) : QColor(0x1F, 0x8A, 0x70);
    case eOwner::Handler:   return dark ? QColor(0xE0, 0x8A, 0x50) : QColor(0xC0, 0x5A, 0x1F);
    case eOwner::Literal:   return dark ? QColor(0x9A, 0x9A, 0x9A) : QColor(0x6A, 0x6A, 0x6A);
    case eOwner::Raw:       return dark ? QColor(0x9A, 0x9A, 0x9A) : QColor(0x6A, 0x6A, 0x6A);
    case eOwner::Operator:
    default:                return QApplication::palette().color(QPalette::WindowText);
    }
}

QColor NEGuardStyle::severityColor(eSeverity severity)
{
    const bool dark = isDark();
    switch (severity)
    {
    case eSeverity::Ok:     return dark ? QColor(0x5F, 0xB8, 0x63) : QColor(0x2E, 0x7D, 0x32);
    case eSeverity::Warn:   return dark ? QColor(0xD8, 0xB0, 0x40) : QColor(0xB8, 0x86, 0x0B);
    case eSeverity::Err:
    default:                return dark ? QColor(0xE0, 0x6A, 0x5C) : QColor(0xC0, 0x39, 0x2B);
    }
}

QColor NEGuardStyle::autoMapTint()
{
    return isDark() ? QColor(0x27, 0x3A, 0x52) : QColor(0xDC, 0xE9, 0xF9);
}

QString NEGuardStyle::ownerGlyph(eOwner owner)
{
    switch (owner)
    {
    case eOwner::Stimulus:  return QStringLiteral("a");
    case eOwner::Fsm:       return QStringLiteral("#");
    case eOwner::Handler:   return QStringLiteral("h");
    case eOwner::Raw:       return QStringLiteral("<>");
    case eOwner::Literal:   return QStringLiteral("=");
    case eOwner::Operator:
    default:                return QString();
    }
}
