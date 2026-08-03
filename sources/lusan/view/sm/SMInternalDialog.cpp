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
 *  \file        lusan/view/sm/SMInternalDialog.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM internal transitions dialog (context-menu access path).
 *
 ************************************************************************/

#include "lusan/view/sm/SMInternalDialog.hpp"

#include "lusan/view/sm/SMInternalEditor.hpp"

#include <QDialogButtonBox>
#include <QVBoxLayout>

SMInternalDialog::SMInternalDialog( StateMachineModel& model
                                  , const QString& title
                                  , uint32_t stateId
                                  , uint32_t transitionId /*= 0u*/
                                  , QWidget* parent /*= nullptr*/)
    : QDialog   (parent)
    , mEditor   (nullptr)
{
    setWindowTitle(title);
    // Taller than the operations dialog: this one hosts a list ABOVE the same accordion.
    setMinimumSize(460, 460);

    QVBoxLayout* layout = new QVBoxLayout(this);
    mEditor = new SMInternalEditor(model, this);
    mEditor->bind(stateId);
    if (transitionId != 0u)
    {
        mEditor->setCurrentTransition(transitionId);
    }

    layout->addWidget(mEditor, 1);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttons);
}
