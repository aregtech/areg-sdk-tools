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
 *  \file        lusan/view/sm/SMGuardHighlighter.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard field highlighter (owner colors + diagnostics).
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardHighlighter.hpp"

#include <QTextCharFormat>

SMGuardHighlighter::SMGuardHighlighter(QTextDocument* document)
    : QSyntaxHighlighter(document)
{
}

void SMGuardHighlighter::setDecorations(const QList<OwnerSpan>& owners, const QList<DiagSpan>& diags)
{
    mOwners = owners;
    mDiags  = diags;
    rehighlight();
}

void SMGuardHighlighter::highlightBlock(const QString& text)
{
    const int length = text.length();

    for (const OwnerSpan& span : mOwners)
    {
        if ((span.start < 0) || (span.length <= 0) || (span.start >= length))
        {
            continue;
        }

        const int len = qMin(span.length, length - span.start);
        QTextCharFormat fmt;
        fmt.setForeground(NEGuardStyle::ownerColor(span.owner));
        setFormat(span.start, len, fmt);
    }

    for (const DiagSpan& span : mDiags)
    {
        if ((span.start < 0) || (span.length <= 0) || (span.start >= length))
        {
            continue;
        }

        const int len = qMin(span.length, length - span.start);
        QTextCharFormat fmt = format(span.start);
        switch (span.kind)
        {
        case eUnderline::Error:
            fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
            fmt.setUnderlineColor(NEGuardStyle::severityColor(NEGuardStyle::eSeverity::Err));
            break;

        case eUnderline::Warning:
            fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
            fmt.setUnderlineColor(NEGuardStyle::severityColor(NEGuardStyle::eSeverity::Warn));
            break;

        case eUnderline::Raw:
        default:
            fmt.setUnderlineStyle(QTextCharFormat::DotLine);
            fmt.setUnderlineColor(NEGuardStyle::ownerColor(NEGuardStyle::eOwner::Raw));
            break;
        }

        setFormat(span.start, len, fmt);
    }
}
