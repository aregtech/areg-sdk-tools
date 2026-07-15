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
 *  \brief       Lusan application, FSM guard folded island token (v7 B2 / E4): the atomic
 *               `{...}` pill text object of the guard field.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QObject>
#include <QTextObjectInterface>

#include <QString>
#include <QTextCharFormat>
#include <QTextFormat>

/**
 * \class   SMInlineToken
 * \brief   The folded-island text object (E4): one ObjectReplacementCharacter per island,
 *          painted as a rounded `{...}` pill in the lambda hue. Because it is a native text
 *          object, the caret, selection, and clipboard treat the whole island as ONE
 *          character (acceptance 23a); the real body rides in the char format's
 *          \ref PropBody property and the tree node stays the truth after commit.
 **/
class SMInlineToken : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    //!< The registered object type of an island token.
    static constexpr int IslandType { QTextFormat::UserObject + 1 };
    //!< The char-format property carrying the island's verbatim body (inner text, no braces).
    static constexpr int PropBody   { QTextFormat::UserProperty + 1 };

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
};

#endif  // LUSAN_VIEW_SM_SMINLINETOKEN_HPP
