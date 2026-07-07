#ifndef LUSAN_VIEW_COMMON_CPPSYNTAXHIGHLIGHTER_HPP
#define LUSAN_VIEW_COMMON_CPPSYNTAXHIGHLIGHTER_HPP
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
 *  \file        lusan/view/common/CppSyntaxHighlighter.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, basic C++ syntax highlighter for embedded code editors.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>

/**
 * \class   CppSyntaxHighlighter
 * \brief   Lightweight, presentation-only C++ highlighter: keywords, types, numbers,
 *          strings, characters, preprocessor lines and both comment styles. It never parses
 *          or validates the text — it only colors it. Palette-derived colors keep it readable
 *          in both light and dark themes. Reused by every embedded-code editor (condition
 *          bodies, inline code).
 **/
class CppSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
private:
    //!< One single-line rule: a pattern and the format applied to every match.
    struct Rule
    {
        QRegularExpression  pattern;
        QTextCharFormat     format;
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit CppSyntaxHighlighter(QTextDocument* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    //!< Applies the single-line rules, then the multi-line block-comment state machine.
    void highlightBlock(const QString& text) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildRules();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QVector<Rule>       mRules;             //!< Ordered single-line rules.
    QTextCharFormat     mCommentFormat;     //!< Format for `/* ... */` spans.
    QRegularExpression  mCommentStart;      //!< `/*`
    QRegularExpression  mCommentEnd;        //!< `*/`
};

#endif  // LUSAN_VIEW_COMMON_CPPSYNTAXHIGHLIGHTER_HPP
