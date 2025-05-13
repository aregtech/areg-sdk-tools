/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/ProjectSettings.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, options dialog.
 *
 ************************************************************************/

#include "lusan/view/common/ProjectSettings.hpp"
#include "ui/ui_ProjectSettings.h"
#include "lusan/view/common/ProjectDirSettings.hpp"
#include "lusan/view/common/WorkspaceManager.hpp"
#include "lusan/view/common/LogSettings.hpp"

#include <QAbstractItemView>
#include <QtAssert>
#include <QAbstractButton>


ProjectSettings::ProjectSettings(QWidget *parent)
    : QDialog(parent)
    , mUi(std::make_unique<Ui::ProjectSettingsDlg>())
    , mSettingsStackedWidget(std::make_unique<QStackedWidget>(this))
    , mModel                (this)
    , mDirSettings          (new ProjectDirSettings(this))
    , mWorkspaceManager     (new WorkspaceManager(this))
    , mLogSettings          (new LogSettings(this))
{
    mUi->setupUi(this);
    setupDialog();
    connectSignals();
}

ProjectSettings::~ProjectSettings()
{
}

void ProjectSettings::setupDialog()
{
    addSettings();

    mUi->horizontalLayout->setStretch(0, 1);
    mUi->horizontalLayout->addWidget(mSettingsStackedWidget.get(), 4);
    mUi->settingsList->setModel(&mModel);

    selectPage(0);
    setFixedSize(size());
}

void ProjectSettings::connectSignals() const
{
    connect(mUi->settingsList,   &QAbstractItemView::clicked,    this, &ProjectSettings::settingsListSelectionChanged);
    connect(mUi->buttonBox,      &QDialogButtonBox::clicked,     this, &ProjectSettings::buttonClicked);
}

void ProjectSettings::settingsListSelectionChanged(QModelIndex const& index)
{
    selectSetting(index.row());
}

void ProjectSettings::selectSetting(int const index) const
{
    Q_ASSERT(index < mSettingsStackedWidget->count());

    mSettingsStackedWidget->setCurrentIndex(index);
}

void ProjectSettings::addSettings()
{
    mSettingsStackedWidget->addWidget(mWorkspaceManager);
    mSettingsStackedWidget->addWidget(mDirSettings);
    mSettingsStackedWidget->addWidget(mLogSettings);

    QStringList settingsList;
    settingsList.append(tr("Workspaces"));
    settingsList.append(tr("Directories"));
    settingsList.append(tr("Log settings"));
    mModel.setStringList(settingsList);
}

void ProjectSettings::buttonClicked(QAbstractButton* button) const
{
    QDialogButtonBox::ButtonRole const role = mUi->buttonBox->buttonRole(button);

    if ((QDialogButtonBox::ButtonRole::AcceptRole != role) &&
        (QDialogButtonBox::ButtonRole::ApplyRole != role))
    {
        return;
    }

    if (mSettingsStackedWidget->currentWidget() == mDirSettings)
    {
        mDirSettings->applyChanges();
    }
    else if (mSettingsStackedWidget->currentWidget() == mWorkspaceManager)
    {
        mWorkspaceManager->applyChanges();
    }
    else if (mSettingsStackedWidget->currentWidget() == mLogSettings)
    {
        mLogSettings->applyChanges();
    }
}

void ProjectSettings::selectPage(int const index) const
{
    selectSetting(index);

    mUi->settingsList->setCurrentIndex(mModel.index(index));
}
