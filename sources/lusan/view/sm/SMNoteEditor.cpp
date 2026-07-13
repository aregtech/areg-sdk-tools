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
 *  \file        lusan/view/sm/SMNoteEditor.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas in-place note text editor.
 *
 ************************************************************************/

#include "lusan/view/sm/SMNoteEditor.hpp"

#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QKeyEvent>
#include <QPlainTextEdit>

namespace
{
    /**
     * \brief   The in-place multi-line editor: Esc cancels, focus-out commits, Enter/Return
     *          inserts a newline (a note is free-form multi-line text).
     **/
    class NoteTextEdit : public QPlainTextEdit
    {
    public:
        std::function<void()>  mCommit;    //!< Commits the current text.
        std::function<void()>  mCancel;    //!< Abandons the edit.

        explicit NoteTextEdit(const QString& text)
            : QPlainTextEdit(text)
        {
        }

    protected:
        virtual void keyPressEvent(QKeyEvent* event) override
        {
            if (event->key() == Qt::Key_Escape)
            {
                event->accept();
                if (mCancel)
                {
                    mCancel();
                }

                return;
            }

            QPlainTextEdit::keyPressEvent(event);
        }

        virtual void focusOutEvent(QFocusEvent* event) override
        {
            QPlainTextEdit::focusOutEvent(event);
            if (mCommit)
            {
                mCommit();
            }
        }
    };
}

SMNoteEditor::~SMNoteEditor()
{
    close();
}

bool SMNoteEditor::isActive() const
{
    return (mProxy != nullptr);
}

void SMNoteEditor::open(QGraphicsItem* host, const QRectF& rect, const QString& text
                       , std::function<void(const QString&)> commit)
{
    if ((mProxy != nullptr) || (host == nullptr))
    {
        return;
    }

    NoteTextEdit* edit = new NoteTextEdit(text);
    edit->mCommit = [this, edit, commit]() {
        if (mProxy == nullptr)
        {
            return;
        }

        const QString committed = edit->toPlainText();
        close();
        if (commit)
        {
            commit(committed);
        }
    };
    edit->mCancel = [this]() { close(); };

    mProxy = new QGraphicsProxyWidget(host);
    mProxy->setWidget(edit);
    mProxy->setGeometry(rect);
    mProxy->setZValue(2.0);

    edit->selectAll();
    edit->setFocus();
}

void SMNoteEditor::close()
{
    if ((mProxy == nullptr) || mClosing)
    {
        return;
    }

    mClosing = true;
    QGraphicsProxyWidget* proxy = mProxy;
    mProxy = nullptr;
    proxy->deleteLater();
    mClosing = false;
}
