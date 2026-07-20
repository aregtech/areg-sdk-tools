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
 *  \file        lusan/view/sm/SMOperationsDialog.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operations editor dialog (context-menu access path).
 *
 ************************************************************************/

#include "lusan/view/sm/SMOperationsDialog.hpp"

#include "lusan/view/sm/SMOperationsEditor.hpp"

#include <QDialogButtonBox>
#include <QVBoxLayout>

SMOperationsDialog::SMOperationsDialog( StateMachineModel& model
                                      , const QString& title
                                      , uint32_t ownerId
                                      , eDocElementKind ownerKind
                                      , uint32_t scopeTransition
                                      , ElementBase* owner
                                      , SMOperationList* list
                                      , QWidget* parent /*= nullptr*/)
    : QDialog   (parent)
    , mEditor   (nullptr)
{
    setWindowTitle(title);
    setMinimumSize(420, 380);

    QVBoxLayout* layout = new QVBoxLayout(this);
    mEditor = new SMOperationsEditor(model, this);
    mEditor->bind(ownerId, ownerKind, scopeTransition, owner, list);
    layout->addWidget(mEditor, 1);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttons);
}
