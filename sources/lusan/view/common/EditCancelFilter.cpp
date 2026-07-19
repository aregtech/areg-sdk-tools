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
 *  \file        lusan/view/common/EditCancelFilter.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Escape-to-cancel behaviour implementation.
 *
 ************************************************************************/

#include "lusan/view/common/EditCancelFilter.hpp"

#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>

void EditCancelFilter::install(QLineEdit* edit)
{
    if (edit != nullptr)
    {
        // Parented to the line edit, so it lives and dies with the field.
        new EditCancelFilter(edit);
    }
}

EditCancelFilter::EditCancelFilter(QLineEdit* edit)
    : QObject       (edit)
    , mEdit         (edit)
    , mBaseline     ( )
    , mHasBaseline  (false)
{
    Q_ASSERT(mEdit != nullptr);
    mEdit->installEventFilter(this);
    // Every committed edit becomes the new baseline that a later Escape reverts to.
    connect(mEdit, &QLineEdit::editingFinished, this, [this]() {
        mBaseline = mEdit->text();
        mHasBaseline = true;
    });
}

bool EditCancelFilter::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == mEdit)
    {
        if (event->type() == QEvent::FocusIn)
        {
            // The value shown when the user starts editing is the committed value to roll back to.
            mBaseline = mEdit->text();
            mHasBaseline = true;
        }
        else if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if ((keyEvent->key() == Qt::Key_Escape) && mHasBaseline)
            {
                // Restore the baseline: setText() re-emits textChanged, so the page's live-sync
                // rolls the model, the list row and the field back to the pre-edit value.
                if (mEdit->text() != mBaseline)
                {
                    mEdit->setText(mBaseline);
                }

                mEdit->selectAll();
                return true;    // Consume: the field is a cancel target, not a dialog Escape.
            }
        }
    }

    return QObject::eventFilter(watched, event);
}
