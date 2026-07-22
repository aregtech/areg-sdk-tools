#ifndef LUSAN_VIEW_SM_SMINLINETOKEN_HPP
#define LUSAN_VIEW_SM_SMINLINETOKEN_HPP
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
 *  \file        lusan/view/sm/SMInlineToken.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard folded island token: the atomic
 *               `{...}` pill text object of the guard field.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QObject>
#include <QTextObjectInterface>

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QString>
#include <QTextCharFormat>
#include <QTextFormat>

/**
 * \class   SMInlineToken
 * \brief   The folded text objects of the guard field: one ObjectReplacementCharacter per
 *          folded item, painted as a rounded pill. Two kinds share the exact same machinery:
 *          an ISLAND is a `{...}` lambda body in the lambda hue; a CHIP is a
 *          committed `#kind:name` reference painted as owner glyph + colored name (the
 *          `#kind:` prefix hidden unless a same-name/different-kind collision reveals it).
 *          Because both are native text objects, the caret, selection, and
 *          clipboard treat the whole pill as ONE character (acceptance 23a); the committable
 *          text rides in the char format's \ref PropBody property and the tree node stays the
 *          truth after commit. A chip is addressed by its INDEX among chips (nth in text
 *          order = nth reference node in pre-order), never by a cached offset.
 **/
class SMInlineToken : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    //!< The registered object type of an island token.
    static constexpr int IslandType { QTextFormat::UserObject + 1 };
    //!< The registered object type of a reference chip token.
    static constexpr int ChipType   { QTextFormat::UserObject + 2 };
    //!< The char-format property carrying the folded committable body: an island's inner text
    //!< (no braces) or a chip's canonical `#kind:name`.
    static constexpr int PropBody   { QTextFormat::UserProperty + 1 };
    //!< A chip's display name (the bare symbol name shown in the pill).
    static constexpr int PropName   { QTextFormat::UserProperty + 2 };
    //!< A chip's owner hue as an int (NEGuardStyle::eOwner).
    static constexpr int PropOwner  { QTextFormat::UserProperty + 3 };
    //!< A chip's `#kind:` prefix (shown only when \ref PropReveal is set).
    static constexpr int PropPrefix { QTextFormat::UserProperty + 4 };
    //!< True when the chip must show its `#kind:` prefix permanently (a same-name collision).
    static constexpr int PropReveal { QTextFormat::UserProperty + 5 };

public:
    explicit SMInlineToken(QObject* parent = nullptr);

    QSizeF intrinsicSize(QTextDocument* doc, int posInDocument, const QTextFormat& format) override;
    void drawObject(QPainter* painter, const QRectF& rect, QTextDocument* doc, int posInDocument, const QTextFormat& format) override;

    //!< The char format of a new island token carrying \p body.
    static QTextCharFormat makeFormat(const QString& body);

    //!< True when \p format is an island token's format.
    static bool isIsland(const QTextFormat& format);

    //!< The island body carried by \p format (empty when not an island).
    static QString bodyOf(const QTextFormat& format);

    //!< The char format of a new reference chip: committable \p body (`#kind:name`), display
    //!< \p name, \p owner hue, `#kind:` \p prefix, and whether that prefix is \p reveal-ed.
    static QTextCharFormat makeChipFormat(const QString& body, const QString& name
                                        , NEGuardStyle::eOwner owner, const QString& prefix, bool reveal);

    //!< True when \p format is a reference chip's format.
    static bool isChip(const QTextFormat& format);

    //!< The chip's display label (owner glyph + name, plus the `#kind:` prefix when revealed).
    static QString chipLabel(const QTextFormat& format);

    //!< The pixel width of the leading glyph hot-zone of a chip (the reveal click target).
    static qreal chipGlyphWidth(const QTextFormat& format, const QFont& font);
};

#endif  // LUSAN_VIEW_SM_SMINLINETOKEN_HPP
