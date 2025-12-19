/************************************************************************
 * This file is part of the Lusan project, an official component of the AREG SDK.
 * Lusan is a graphical user interface (GUI) tool designed to support the development,
 * debugging, and testing of applications built with the AREG Framework.
 *
 * Lusan is available as free and open-source software under the MIT License,
 * providing essential features for developers.
 *
 * For detailed licensing terms, please refer to the LICENSE.txt file included
 * with this distribution or contact us at info[at]areg.tech.
 *
 * \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 * \file        lusan/view/common/WorkspaceSetupDialog.cpp
 * \ingroup     Lusan - GUI Tool for AREG SDK
 * \author      Artak Avetyan
 * \brief       Lusan application, the new workspace setup dialog.
 *
 ************************************************************************/

#include "lusan/view/common/WorkspaceSetupDialog.hpp"
#include "lusan/view/common/OptionPageProjectDirs.hpp"
#include "ui/ui_WorkspaceSetupDialog.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>

WorkspaceSetupDialog::WorkspaceSetupDialog(void)
    : QDialog           (nullptr)
    , mUi               (new Ui::WorkspaceSetupDialog())
    , mOptionProjectDirs(nullptr)
{
    mUi->setupUi(this);
    mOptionProjectDirs = new OptionPageProjectDirs(this);
    QSize size{mOptionProjectDirs->size()};
    
    mOptionProjectDirs->setFixedSize(size);
    mOptionProjectDirs->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    mUi->verticalLayout->addWidget(mOptionProjectDirs, 1);
    mUi->verticalLayout->setStretch(0, 1);
    setWindowTitle(tr("Setup New Workspace"));
    
    QDialogButtonBox *buttonBox = mUi->buttonBox;
    // buttonBox->setFixedWidth(size.width());
    // Connect buttons
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

WorkspaceSetupDialog::~WorkspaceSetupDialog(void)
{
    delete mUi;
    mUi = nullptr;
    
    if (mOptionProjectDirs != nullptr)
    {
        delete mOptionProjectDirs;
        mOptionProjectDirs = nullptr;
    }
}

void WorkspaceSetupDialog::applyDirectories(void)
{
    if (mOptionProjectDirs != nullptr)
    {
        mOptionProjectDirs->applyChanges();
    }
}
