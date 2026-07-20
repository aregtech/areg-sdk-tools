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
 *  \file        lusan/view/sm/SMInlineToken.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard folded island token (v7 B2 / E4).
 *
 ************************************************************************/

#include "lusan/view/sm/SMInlineToken.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QFontMetricsF>
#include <QPainter>
#include <QTextDocument>

namespace
{
    const QString TOKEN_LABEL { QStringLiteral("{} {...}") };
    constexpr qreal PAD_X { 6.0 };
    constexpr qreal PAD_Y { 2.0 };
    constexpr qreal GLYPH_GAP { 4.0 };

    //!< The owner glyph + a gap prefixed to a chip's name, e.g. "a " (empty owner -> "").
    QString chipGlyphText(const QTextFormat& format)
    {
        const auto owner = static_cast<NEGuardStyle::eOwner>(format.property(SMInlineToken::PropOwner).toInt());
        const QString glyph = NEGuardStyle::ownerGlyph(owner);
        return glyph.isEmpty() ? QString() : (glyph + QLatin1Char(' '));
    }
}

SMInlineToken::SMInlineToken(QObject* parent /*= nullptr*/)
    : QObject               (parent)
    , QTextObjectInterface  ( )
{
}

QSizeF SMInlineToken::intrinsicSize(QTextDocument* doc, int /*posInDocument*/, const QTextFormat& format)
{
    const QTextCharFormat charFormat = format.toCharFormat();
    QFont font = charFormat.font();
    if (font.family().isEmpty() && (doc != nullptr))
    {
        font = doc->defaultFont();
    }

    const QFontMetricsF metrics(font);
    const QString label = isChip(format) ? chipLabel(format) : TOKEN_LABEL;
    return QSizeF(metrics.horizontalAdvance(label) + (2.0 * PAD_X)
                 , metrics.height() + (2.0 * PAD_Y));
}

void SMInlineToken::drawObject(QPainter* painter, const QRectF& rect, QTextDocument* /*doc*/, int /*posInDocument*/, const QTextFormat& format)
{
    const bool chip = isChip(format);
    // A chip is painted in its owner hue (glyph is the accessibility channel); an island stays
    // in the lambda hue. The B15 pill contract: 1 px border at 60%, fill at 12%, 4 px radius.
    const NEGuardStyle::eOwner owner = chip
        ? static_cast<NEGuardStyle::eOwner>(format.property(PropOwner).toInt())
        : NEGuardStyle::eOwner::Fsm;
    QColor hue = NEGuardStyle::ownerColor(owner);
    QColor border(hue);
    border.setAlphaF(0.6);
    QColor fill(hue);
    fill.setAlphaF(0.12);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(border, 1.0));
    painter->setBrush(fill);
    painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 4.0, 4.0);

    painter->setPen(hue);
    painter->setFont(format.toCharFormat().font());
    painter->drawText(rect, Qt::AlignCenter, chip ? chipLabel(format) : TOKEN_LABEL);
    painter->restore();
}

QTextCharFormat SMInlineToken::makeFormat(const QString& body)
{
    QTextCharFormat format;
    format.setObjectType(IslandType);
    format.setProperty(PropBody, body);
    return format;
}

bool SMInlineToken::isIsland(const QTextFormat& format)
{
    return (format.objectType() == IslandType);
}

QString SMInlineToken::bodyOf(const QTextFormat& format)
{
    return format.property(PropBody).toString();
}

QTextCharFormat SMInlineToken::makeChipFormat(const QString& body, const QString& name
                                            , NEGuardStyle::eOwner owner, const QString& prefix, bool reveal)
{
    QTextCharFormat format;
    format.setObjectType(ChipType);
    format.setProperty(PropBody, body);
    format.setProperty(PropName, name);
    format.setProperty(PropOwner, static_cast<int>(owner));
    format.setProperty(PropPrefix, prefix);
    format.setProperty(PropReveal, reveal);
    return format;
}

bool SMInlineToken::isChip(const QTextFormat& format)
{
    return (format.objectType() == ChipType);
}

QString SMInlineToken::chipLabel(const QTextFormat& format)
{
    const QString name    = format.property(PropName).toString();
    const bool    reveal  = format.property(PropReveal).toBool();
    const QString shown   = reveal ? (format.property(PropPrefix).toString() + name) : name;
    return chipGlyphText(format) + shown;
}

qreal SMInlineToken::chipGlyphWidth(const QTextFormat& format, const QFont& font)
{
    const QFontMetricsF metrics(font);
    // The hot-zone spans the leading padding plus the owner glyph and its gap.
    return PAD_X + metrics.horizontalAdvance(chipGlyphText(format)) + GLYPH_GAP;
}
