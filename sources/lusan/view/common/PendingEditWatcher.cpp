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
 *  \file        lusan/view/common/PendingEditWatcher.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, reports text typed into a field but not yet given to the document.
 *
 ************************************************************************/

#include "lusan/view/common/PendingEditWatcher.hpp"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QTextDocument>
#include <QTextEdit>
#include <QVariant>
#include <QWidget>

namespace
{
    //!< The mark a field or a form carries, holding the document its text belongs to.
    constexpr const char* const DocumentProperty{ "lusanEditDocument" };
}

void PendingEditWatcher::watchField(QWidget* field, const QObject& document)
{
    if (field != nullptr)
    {
        field->setProperty(DocumentProperty, QVariant::fromValue(const_cast<QObject*>(&document)));
    }
}

PendingEditWatcher::PendingEditWatcher(const QObject& document, QObject* parent /*= nullptr*/)
    : QObject   (parent)
    , mDocument (&document)
    , mField    ( )
    , mBaseline ( )
    , mPending  (false)
{
    connect(qApp, &QApplication::focusChanged, this, &PendingEditWatcher::onFocusChanged);
    bindField(QApplication::focusWidget());
}

void PendingEditWatcher::acceptPendingEdit(void)
{
    if (mField.isNull() == false)
    {
        mBaseline = fieldText(mField.data());
    }

    setPending(false);
}

void PendingEditWatcher::onFocusChanged(QWidget* previous, QWidget* current)
{
    Q_UNUSED(previous);
    bindField(current);
}

void PendingEditWatcher::onFieldTextChanged(void)
{
    if (mField.isNull())
    {
        return;
    }

    // Text a page put into the field carries no editing history, text the author put there does.
    // So a refill under the caret is not an edit -- it is what the next typing is measured against.
    if (fieldEdited(mField.data()) == false)
    {
        mBaseline = fieldText(mField.data());
        setPending(false);
        return;
    }

    setPending(fieldText(mField.data()) != mBaseline);
}

void PendingEditWatcher::bindField(QWidget* field)
{
    if (mField.isNull() == false)
    {
        disconnect(mField.data(), nullptr, this, nullptr);
    }

    mField.clear();
    mBaseline.clear();

    if ((field == nullptr) || (documentOf(field) != mDocument))
    {
        setPending(false);
        return;
    }

    bool watched{ false };
    if (QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(field))
    {
        watched = connect(edit, &QPlainTextEdit::textChanged, this, &PendingEditWatcher::onFieldTextChanged);
    }
    else if (QTextEdit* edit = qobject_cast<QTextEdit*>(field))
    {
        watched = connect(edit, &QTextEdit::textChanged, this, &PendingEditWatcher::onFieldTextChanged);
    }
    else if (QLineEdit* edit = qobject_cast<QLineEdit*>(field))
    {
        watched = connect(edit, &QLineEdit::textChanged, this, &PendingEditWatcher::onFieldTextChanged);
    }
    else if (QSpinBox* edit = qobject_cast<QSpinBox*>(field))
    {
        watched = connect(edit, &QSpinBox::textChanged, this, &PendingEditWatcher::onFieldTextChanged);
    }
    else if (QDoubleSpinBox* edit = qobject_cast<QDoubleSpinBox*>(field))
    {
        watched = connect(edit, &QDoubleSpinBox::textChanged, this, &PendingEditWatcher::onFieldTextChanged);
    }

    if (watched)
    {
        mField = field;
        mBaseline = fieldText(field);
    }

    setPending(false);
}

void PendingEditWatcher::setPending(bool pending)
{
    if (mPending != pending)
    {
        mPending = pending;
        emit signalPendingEditChanged(pending);
    }
}

QString PendingEditWatcher::fieldText(const QWidget* field)
{
    if (const QPlainTextEdit* edit = qobject_cast<const QPlainTextEdit*>(field))
        return edit->toPlainText();
    else if (const QTextEdit* edit = qobject_cast<const QTextEdit*>(field))
        return edit->toPlainText();
    else if (const QLineEdit* edit = qobject_cast<const QLineEdit*>(field))
        return edit->text();
    else if (const QAbstractSpinBox* edit = qobject_cast<const QAbstractSpinBox*>(field))
        return edit->text();
    else
        return QString();
}

bool PendingEditWatcher::fieldEdited(const QWidget* field)
{
    if (const QPlainTextEdit* edit = qobject_cast<const QPlainTextEdit*>(field))
        return edit->document()->isUndoAvailable();
    else if (const QTextEdit* edit = qobject_cast<const QTextEdit*>(field))
        return edit->document()->isUndoAvailable();
    else if (const QLineEdit* edit = qobject_cast<const QLineEdit*>(field))
        return edit->isUndoAvailable();
    else if (const QAbstractSpinBox* edit = qobject_cast<const QAbstractSpinBox*>(field))
    {
        // A spin box types into a line edit of its own; stepping with the arrows or the wheel
        // writes it the way a page would, and is left to the commit the box already does.
        const QLineEdit* inner = edit->findChild<QLineEdit*>();
        return ((inner != nullptr) && inner->isUndoAvailable());
    }
    else
        return false;
}

const QObject* PendingEditWatcher::documentOf(const QWidget* field)
{
    for (const QObject* object = field; object != nullptr; object = object->parent())
    {
        const QVariant mark = object->property(DocumentProperty);
        if (mark.isValid())
        {
            return mark.value<QObject*>();
        }
    }

    return nullptr;
}
