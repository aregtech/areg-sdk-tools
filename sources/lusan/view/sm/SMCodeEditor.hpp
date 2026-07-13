#ifndef LUSAN_VIEW_SM_SMCODEEDITOR_HPP
#define LUSAN_VIEW_SM_SMCODEEDITOR_HPP
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
 *  \file        lusan/view/sm/SMCodeEditor.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, embedded C++ code editor (condition body / inline code).
 *
 ************************************************************************/

#include <QWidget>

class CppSyntaxHighlighter;
class QLabel;
class QPlainTextEdit;

/**
 * \brief   The embedded-code editor: a read-only signature line above a
 *          monospaced body editor with basic C++ syntax highlighting, and a fixed note below
 *          reminding that the machine instance is always captured (attributes, constants and
 *          accessors are in scope) and — for conditions — that the body must end in
 *          `return <value>;`. The editor never parses or validates the text; the body is
 *          stored verbatim. Reused by the Methods page and, later, by inline code blocks.
 **/
class SMCodeEditor : public QWidget
{
    Q_OBJECT

public:
    explicit SMCodeEditor(QWidget* parent = nullptr);

    //!< The verbatim body text editor.
    QPlainTextEdit* ctrlBody() const;

    //!< Sets the read-only declared signature shown above the body.
    void setSignature(const QString& signature);
    //!< Sets the fixed reminder shown below the body (capture note and return requirement).
    void setNote(const QString& note);

private:
    void buildUi();

private:
    QLabel*                 mSignature;     //!< Read-only declared signature.
    QPlainTextEdit*         mBody;          //!< Monospaced verbatim body editor.
    QLabel*                 mNote;          //!< Fixed capture / return reminder.
    CppSyntaxHighlighter*   mHighlighter;   //!< Basic C++ highlighter over the body document.
};

#endif  // LUSAN_VIEW_SM_SMCODEEDITOR_HPP
