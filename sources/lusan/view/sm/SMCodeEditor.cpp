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
 *  \file        lusan/view/sm/SMCodeEditor.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, embedded C++ code editor (condition body / inline code).
 *
 ************************************************************************/

#include "lusan/view/sm/SMCodeEditor.hpp"

#include "lusan/view/common/CppSyntaxHighlighter.hpp"

#include <QAbstractItemView>
#include <QCompleter>
#include <QFontDatabase>
#include <QLabel>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QStringListModel>
#include <QTextCursor>
#include <QVBoxLayout>

SMCodeEditor::SMCodeEditor(QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , mSignature    (nullptr)
    , mBody         (nullptr)
    , mNote         (nullptr)
    , mHighlighter  (nullptr)
    , mCompleter    (nullptr)
    , mWordModel    (nullptr)
{
    buildUi();
}

void SMCodeEditor::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(2);

    const QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    mSignature = new QLabel(this);
    mSignature->setFont(mono);
    mSignature->setWordWrap(true);
    mSignature->setTextInteractionFlags(Qt::TextSelectableByMouse);
    mSignature->setStyleSheet(QStringLiteral("color: palette(mid);"));
    root->addWidget(mSignature);

    mBody = new QPlainTextEdit(this);
    mBody->setFont(mono);
    mBody->setLineWrapMode(QPlainTextEdit::NoWrap);
    mBody->setTabStopDistance(4 * QFontMetricsF(mono).horizontalAdvance(QLatin1Char(' ')));
    mBody->setPlaceholderText(tr("Write the C++ body here"));
    mBody->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    root->addWidget(mBody, 1);

    mHighlighter = new CppSyntaxHighlighter(mBody->document());

    mNote = new QLabel(this);
    mNote->setWordWrap(true);
    mNote->setStyleSheet(QStringLiteral("color: palette(mid);"));
    root->addWidget(mNote);
}

QPlainTextEdit* SMCodeEditor::ctrlBody() const
{
    return mBody;
}

void SMCodeEditor::setSignature(const QString& signature)
{
    mSignature->setText(signature);
}

void SMCodeEditor::setNote(const QString& note)
{
    mNote->setText(note);
}

void SMCodeEditor::setCompletions(const QStringList& words)
{
    if (words.isEmpty())
    {
        if (mWordModel != nullptr)
        {
            mWordModel->setStringList(QStringList());
        }
        if ((mCompleter != nullptr) && (mCompleter->popup() != nullptr))
        {
            mCompleter->popup()->hide();
        }
        return;
    }

    if (mCompleter == nullptr)
    {
        // Known-identifier completion only: the words are the machine's own names, never a
        // parse of the edited text (verbatim stays opaque).
        mWordModel = new QStringListModel(words, this);
        mCompleter = new QCompleter(mWordModel, this);
        mCompleter->setWidget(mBody);
        mCompleter->setCompletionMode(QCompleter::PopupCompletion);
        mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        mCompleter->setWrapAround(false);

        connect(mCompleter, QOverload<const QString&>::of(&QCompleter::activated), this, &SMCodeEditor::insertCompletion);
        connect(mBody, &QPlainTextEdit::textChanged, this, &SMCodeEditor::updateCompletionPopup);
    }
    else
    {
        mWordModel->setStringList(words);
    }
}

QString SMCodeEditor::wordUnderCursor() const
{
    QTextCursor cursor = mBody->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    return cursor.selectedText();
}

void SMCodeEditor::insertCompletion(const QString& completion)
{
    if ((mCompleter == nullptr) || (mCompleter->widget() != mBody))
    {
        return;
    }

    QTextCursor cursor = mBody->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    cursor.insertText(completion);
    mBody->setTextCursor(cursor);
}

void SMCodeEditor::updateCompletionPopup()
{
    if (mCompleter == nullptr)
    {
        return;
    }

    // Never pop up while the body is being filled programmatically (it is not focused then):
    // this keeps a document load or an undo from flashing the completer.
    if (mBody->hasFocus() == false)
    {
        return;
    }

    const QString prefix = wordUnderCursor();
    if (prefix.isEmpty())
    {
        mCompleter->popup()->hide();
        return;
    }

    if (prefix != mCompleter->completionPrefix())
    {
        mCompleter->setCompletionPrefix(prefix);
        mCompleter->popup()->setCurrentIndex(mCompleter->completionModel()->index(0, 0));
    }

    if (mCompleter->completionCount() == 0)
    {
        mCompleter->popup()->hide();
        return;
    }

    QRect rect = mBody->cursorRect();
    rect.setWidth(mCompleter->popup()->sizeHintForColumn(0) + mCompleter->popup()->verticalScrollBar()->sizeHint().width());
    mCompleter->complete(rect);
}
