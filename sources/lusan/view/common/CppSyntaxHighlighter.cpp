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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/common/CppSyntaxHighlighter.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, basic C++ syntax highlighter for embedded code editors.
 *
 ************************************************************************/

#include "lusan/view/common/CppSyntaxHighlighter.hpp"

#include <QApplication>
#include <QPalette>

namespace
{
    //!< The C++ keywords worth coloring; type names are colored by a separate rule.
    const char* const KEYWORDS[] =
    {
          "alignas", "alignof", "and", "auto", "break", "case", "catch", "class", "const"
        , "constexpr", "const_cast", "continue", "decltype", "default", "delete", "do"
        , "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "final"
        , "for", "friend", "goto", "if", "inline", "mutable", "namespace", "new", "noexcept"
        , "nullptr", "operator", "override", "private", "protected", "public", "register"
        , "reinterpret_cast", "return", "sizeof", "static", "static_cast", "struct", "switch"
        , "template", "this", "throw", "true", "try", "typedef", "typename", "union", "using"
        , "virtual", "volatile", "while"
    };

    //!< Built-in and common fixed-width type names.
    const char* const TYPES[] =
    {
          "bool", "char", "char8_t", "char16_t", "char32_t", "double", "float", "int", "long"
        , "short", "signed", "unsigned", "void", "wchar_t", "size_t"
        , "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t"
    };
}

CppSyntaxHighlighter::CppSyntaxHighlighter(QTextDocument* parent /*= nullptr*/)
    : QSyntaxHighlighter (parent)
    , mCommentStart      (QStringLiteral("/\\*"))
    , mCommentEnd        (QStringLiteral("\\*/"))
{
    buildRules();
}

void CppSyntaxHighlighter::buildRules()
{
    const bool dark = (qApp->palette().color(QPalette::Base).lightness() < 128);

    const QColor keywordColor = dark ? QColor("#569cd6") : QColor("#0000ff");
    const QColor typeColor    = dark ? QColor("#4ec9b0") : QColor("#267f99");
    const QColor numberColor  = dark ? QColor("#b5cea8") : QColor("#098658");
    const QColor stringColor  = dark ? QColor("#ce9178") : QColor("#a31515");
    const QColor commentColor = dark ? QColor("#6a9955") : QColor("#008000");
    const QColor preprocColor = dark ? QColor("#c586c0") : QColor("#808000");

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);
    for (const char* keyword : KEYWORDS)
    {
        mRules.append({ QRegularExpression(QStringLiteral("\\b%1\\b").arg(QLatin1String(keyword))), keywordFormat });
    }

    QTextCharFormat typeFormat;
    typeFormat.setForeground(typeColor);
    for (const char* type : TYPES)
    {
        mRules.append({ QRegularExpression(QStringLiteral("\\b%1\\b").arg(QLatin1String(type))), typeFormat });
    }

    QTextCharFormat numberFormat;
    numberFormat.setForeground(numberColor);
    mRules.append({ QRegularExpression(QStringLiteral("\\b(0[xX][0-9a-fA-F]+|\\d+(\\.\\d+)?)[uUlLfF]*\\b")), numberFormat });

    QTextCharFormat stringFormat;
    stringFormat.setForeground(stringColor);
    // Double- and single-quoted literals, tolerant of escaped quotes; no cross-line spans.
    mRules.append({ QRegularExpression(QStringLiteral("\"(\\\\.|[^\"\\\\])*\"")), stringFormat });
    mRules.append({ QRegularExpression(QStringLiteral("'(\\\\.|[^'\\\\])*'")), stringFormat });

    QTextCharFormat preprocFormat;
    preprocFormat.setForeground(preprocColor);
    mRules.append({ QRegularExpression(QStringLiteral("^\\s*#[^\\n]*")), preprocFormat });

    QTextCharFormat lineCommentFormat;
    lineCommentFormat.setForeground(commentColor);
    lineCommentFormat.setFontItalic(true);
    mRules.append({ QRegularExpression(QStringLiteral("//[^\\n]*")), lineCommentFormat });

    mCommentFormat.setForeground(commentColor);
    mCommentFormat.setFontItalic(true);
}

void CppSyntaxHighlighter::highlightBlock(const QString& text)
{
    for (const Rule& rule : mRules)
    {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext())
        {
            const QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Multi-line `/* ... */` comments carried across blocks with a single in-comment state.
    setCurrentBlockState(0);
    int startIndex = (previousBlockState() == 1) ? 0 : text.indexOf(mCommentStart);
    while (startIndex >= 0)
    {
        const QRegularExpressionMatch endMatch = mCommentEnd.match(text, startIndex);
        int commentLength = 0;
        if (endMatch.hasMatch() == false)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endMatch.capturedEnd() - startIndex;
        }

        setFormat(startIndex, commentLength, mCommentFormat);
        startIndex = text.indexOf(mCommentStart, startIndex + commentLength);
    }
}
