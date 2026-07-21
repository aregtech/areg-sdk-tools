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
    return QSizeF(metrics.horizontalAdvance(TOKEN_LABEL) + (2.0 * PAD_X)
                 , metrics.height() + (2.0 * PAD_Y));
}

void SMInlineToken::drawObject(QPainter* painter, const QRectF& rect, QTextDocument* /*doc*/, int /*posInDocument*/, const QTextFormat& format)
{
    // The B15 pill contract: 1 px border in the owner hue at 60%, fill at 12%, 4 px radius.
    QColor hue = NEGuardStyle::ownerColor(NEGuardStyle::eOwner::Fsm);
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
    painter->drawText(rect, Qt::AlignCenter, TOKEN_LABEL);
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
