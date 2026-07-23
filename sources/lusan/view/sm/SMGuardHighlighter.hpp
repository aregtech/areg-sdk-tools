#ifndef LUSAN_VIEW_SM_SMGUARDHIGHLIGHTER_HPP
#define LUSAN_VIEW_SM_SMGUARDHIGHLIGHTER_HPP
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
 *  \file        lusan/view/sm/SMGuardHighlighter.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard field highlighter (owner colors + diagnostics).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QSyntaxHighlighter>

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QList>

class QTextDocument;

/**
 * \class   SMGuardHighlighter
 * \brief   Paints the guard field's document from a decoration set the field hands it: owner
 *          foreground colors and diagnostic underlines -- an error squiggle, a
 *          warning underline and the dotted underline of a raw fragment. The
 *          highlighter owns no analysis; the field computes the spans (from render spans on
 *          a resolved tree, or from a live token classification while typing) and calls
 *          \ref setDecorations. One document block: the field forbids newlines.
 **/
class SMGuardHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    /**
     * \enum    eUnderline
     * \brief   The diagnostic underline of a span.
     **/
    enum class eUnderline
    {
          Error     //!< Red wavy squiggle (unresolved name / malformed).
        , Warning   //!< Amber wavy underline (shadowing).
        , Raw       //!< Gray dotted underline (accepted raw-C++ fragment).
    };

    /**
     * \struct  OwnerSpan
     * \brief   One owner-colored span of the field text.
     **/
    struct OwnerSpan
    {
        int                     start;
        int                     length;
        NEGuardStyle::eOwner    owner;
    };

    /**
     * \struct  DiagSpan
     * \brief   One diagnostic-underlined span of the field text.
     **/
    struct DiagSpan
    {
        int         start;
        int         length;
        eUnderline  kind;
    };

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMGuardHighlighter(QTextDocument* document);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Replaces the decoration set and repaints.
    void setDecorations(const QList<OwnerSpan>& owners, const QList<DiagSpan>& diags);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    void highlightBlock(const QString& text) override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QList<OwnerSpan>    mOwners;    //!< The owner color spans.
    QList<DiagSpan>     mDiags;     //!< The diagnostic underline spans.
};

#endif  // LUSAN_VIEW_SM_SMGUARDHIGHLIGHTER_HPP
