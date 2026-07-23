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
 *  \brief       Lusan application, FSM guard folded island token.
 *
 ************************************************************************/

#include "lusan/view/sm/SMInlineToken.hpp"

#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QFontMetricsF>
#include <QPainter>
#include <QTextDocument>

namespace
{
    const QString TOKEN_LABEL { QStringLiteral("{} {...}") };

    // A chip reads as an inline mention -- `[h] #Name` -- not as a boxed widget: only the tiny kind
    // badge keeps an edge, the name follows it in the owner hue over a soft highlight, and the whole
    // thing sits on the surrounding text's baseline. The vertical metric
    // is the font ASCENT, because the layout aligns an inline object's BOTTOM with the line baseline:
    // asking for anything taller lifts the chip above the text, which is what looked broken before.
    constexpr qreal BADGE_PAD_X { 3.0 };    //!< Horizontal padding inside the kind badge.
    constexpr qreal BADGE_GAP   { 3.0 };    //!< Gap between the badge and the `#name` text.
    constexpr qreal EDGE_PAD    { 2.0 };    //!< Leading/trailing breathing room of the highlight.
    constexpr qreal RADIUS      { 3.0 };    //!< Corner radius of the badge and the highlight.

    //!< The island token keeps its pill; a chip has no outer border at all.
    constexpr qreal ISLAND_PAD_X{ 6.0 };

    //!< The chip's kind badge letter ("h", "a", "f", ...); empty when the owner has no glyph.
    QString chipGlyph(const QTextFormat& format)
    {
        const auto owner = static_cast<NEGuardStyle::eOwner>(format.property(SMInlineToken::PropOwner).toInt());
        return NEGuardStyle::ownerGlyph(owner);
    }

    //!< The chip's mention text: the reference sigil + the shown name (`#Name`, or `#kind:Name`).
    QString chipMention(const QTextFormat& format)
    {
        const QString name   = format.property(SMInlineToken::PropName).toString();
        const bool    reveal = format.property(SMInlineToken::PropReveal).toBool();
        const QString shown  = reveal ? (format.property(SMInlineToken::PropPrefix).toString() + name) : name;
        return QString(NEGuardText::RefSigil) + shown;
    }

    //!< The width of the badge box (0 when the owner has no glyph).
    qreal badgeWidth(const QTextFormat& format, const QFontMetricsF& metrics)
    {
        const QString glyph = chipGlyph(format);
        return glyph.isEmpty() ? 0.0 : (metrics.horizontalAdvance(glyph) + (2.0 * BADGE_PAD_X));
    }

    //!< The font a chip paints with: the char format's own font, falling back to the document's.
    QFont chipFont(QTextDocument* doc, const QTextFormat& format)
    {
        QFont font = format.toCharFormat().font();
        if (font.family().isEmpty() && (doc != nullptr))
        {
            font = doc->defaultFont();
        }

        return font;
    }
}

SMInlineToken::SMInlineToken(QObject* parent /*= nullptr*/)
    : QObject               (parent)
    , QTextObjectInterface  ( )
{
}

QSizeF SMInlineToken::intrinsicSize(QTextDocument* doc, int /*posInDocument*/, const QTextFormat& format)
{
    const QFont font = chipFont(doc, format);
    const QFontMetricsF metrics(font);

    if (isChip(format) == false)
    {
        return QSizeF(metrics.horizontalAdvance(TOKEN_LABEL) + (2.0 * ISLAND_PAD_X), metrics.ascent());
    }

    const qreal badge = badgeWidth(format, metrics);
    const qreal gap   = (badge > 0.0) ? BADGE_GAP : 0.0;
    // The height is the ascent ONLY: the layout puts an inline object's bottom edge on the line
    // baseline, so an ascent-tall box makes the chip's own baseline the line's baseline. Descenders
    // of the name paint into the line's descent band, exactly as plain text does.
    return QSizeF((2.0 * EDGE_PAD) + badge + gap + metrics.horizontalAdvance(chipMention(format))
                 , metrics.ascent());
}

void SMInlineToken::drawObject(QPainter* painter, const QRectF& rect, QTextDocument* doc, int /*posInDocument*/, const QTextFormat& format)
{
    const NEGuardStyle::eOwner owner = isChip(format)
        ? static_cast<NEGuardStyle::eOwner>(format.property(PropOwner).toInt())
        : NEGuardStyle::eOwner::Fsm;
    const QColor hue = NEGuardStyle::ownerColor(owner);
    const QFont  font = chipFont(doc, format);
    const QFontMetricsF metrics(font);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setFont(font);

    // rect.bottom() is the text baseline of the line this object sits on (see intrinsicSize).
    const qreal baseline = rect.bottom();

    if (isChip(format) == false)
    {
        // The folded island keeps its pill: it stands for a block of code, not for one name.
        QColor border(hue);
        border.setAlphaF(0.6);
        QColor fill(hue);
        fill.setAlphaF(0.12);
        const QRectF pill(rect.left() + 0.5, baseline - metrics.ascent() + 0.5
                         , rect.width() - 1.0, metrics.ascent() + metrics.descent() - 1.0);
        painter->setPen(QPen(border, 1.0));
        painter->setBrush(fill);
        painter->drawRoundedRect(pill, RADIUS, RADIUS);
        painter->setPen(hue);
        painter->drawText(QPointF(rect.left() + ISLAND_PAD_X, baseline), TOKEN_LABEL);
        painter->restore();
        return;
    }

    // A mention: soft highlight, a bordered kind badge, then `#Name` -- no outer border, and every
    // glyph drawn from the same baseline so the chip reads as one line with the text around it.
    QColor highlight(hue);
    highlight.setAlphaF(0.13);
    const QRectF band(rect.left(), baseline - metrics.ascent()
                     , rect.width(), metrics.ascent() + metrics.descent());
    painter->setPen(Qt::NoPen);
    painter->setBrush(highlight);
    painter->drawRoundedRect(band, RADIUS, RADIUS);

    qreal x = rect.left() + EDGE_PAD;
    const QString glyph = chipGlyph(format);
    if (glyph.isEmpty() == false)
    {
        QColor badgeBorder(hue);
        badgeBorder.setAlphaF(0.65);
        QColor badgeFill(hue);
        badgeFill.setAlphaF(0.20);
        const qreal width = badgeWidth(format, metrics);
        const QRectF badge(x + 0.5, baseline - metrics.ascent() + 0.5
                          , width - 1.0, metrics.ascent() + (metrics.descent() * 0.5) - 1.0);
        painter->setPen(QPen(badgeBorder, 1.0));
        painter->setBrush(badgeFill);
        painter->drawRoundedRect(badge, RADIUS, RADIUS);
        painter->setPen(hue);
        painter->drawText(QPointF(x + BADGE_PAD_X, baseline), glyph);
        x += width + BADGE_GAP;
    }

    painter->setPen(hue);
    painter->drawText(QPointF(x, baseline), chipMention(format));
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
    const QString glyph = chipGlyph(format);
    const QString mention = chipMention(format);
    return glyph.isEmpty() ? mention : (glyph + QLatin1Char(' ') + mention);
}

qreal SMInlineToken::chipGlyphWidth(const QTextFormat& format, const QFont& font)
{
    const QFontMetricsF metrics(font);
    // The hot-zone spans the leading padding plus the kind badge and the gap after it.
    return EDGE_PAD + badgeWidth(format, metrics) + BADGE_GAP;
}
