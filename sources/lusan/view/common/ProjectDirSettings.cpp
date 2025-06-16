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
 *  \file        lusan/view/common/ProjectDirSettings.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, project settings widget.
 *
 ************************************************************************/

#include "lusan/view/common/ProjectDirSettings.hpp"
#include "ui/ui_ProjectDirSettings.h"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/view/common/ProjectSettings.hpp"

#include <QFileDialog>

ProjectDirSettings::ProjectDirSettings(ProjectSettings *parent)
    : QWidget(parent)
    , mUi(std::make_unique<Ui::projectDirSettingsDlg>())
{
    mUi->setupUi(this);
    connectSignalHandlers();
    initializePathsWithCurrentWorkspaceData();
}

ProjectDirSettings::~ProjectDirSettings()
{
}

void ProjectDirSettings::connectSignalHandlers() const
{
    connect(mUi->rootDirBrowseBtn,       &QPushButton::clicked, this, &ProjectDirSettings::onRootDirBrowseBtnClicked);
    connect(mUi->sourceDirBrowseBtn,     &QPushButton::clicked, this, &ProjectDirSettings::onSourceDirBrowseBtnClicked);
    connect(mUi->includeDirBrowseBtn,    &QPushButton::clicked, this, &ProjectDirSettings::onIncludeDirBrowseBtnClicked);
    connect(mUi->deliveryDirBrowseBtn,   &QPushButton::clicked, this, &ProjectDirSettings::onDeliveryDirBrowseBtnClicked);
    connect(mUi->logDirBrowseBtn,        &QPushButton::clicked, this, &ProjectDirSettings::onLogDirBrowseBtnClicked);
}

void ProjectDirSettings::onRootDirBrowseBtnClicked()
{
    mUi->rootDirEdit->setText(
        QFileDialog::getExistingDirectory(this, tr("Open Root Directory"), "", QFileDialog::ShowDirsOnly));
}

void ProjectDirSettings::onSourceDirBrowseBtnClicked()
{
    mUi->sourceDirEdit->setText(
        QFileDialog::getExistingDirectory(this, tr("Open Source Directory"), "", QFileDialog::ShowDirsOnly));
}

void ProjectDirSettings::onIncludeDirBrowseBtnClicked()
{
    mUi->includeDirEdit->setText(
        QFileDialog::getExistingDirectory(this, tr("Open Include Directory"), "", QFileDialog::ShowDirsOnly));
}

void ProjectDirSettings::onDeliveryDirBrowseBtnClicked()
{
    mUi->deliveryDirEdit->setText(
        QFileDialog::getExistingDirectory(this, tr("Open Delivery Directory"), "", QFileDialog::ShowDirsOnly));
}

void ProjectDirSettings::onLogDirBrowseBtnClicked()
{
    mUi->logDirEdit->setText(
        QFileDialog::getExistingDirectory(this, tr("Open Log Directory"), "", QFileDialog::ShowDirsOnly));
}

void ProjectDirSettings::initializePathsWithCurrentWorkspaceData() const
{
    WorkspaceEntry const currentWorkspace{ LusanApplication::getActiveWorkspace() };

    mUi->rootDirEdit->setText(currentWorkspace.getWorkspaceRoot());
    mUi->sourceDirEdit->setText(currentWorkspace.getDirSources());
    mUi->includeDirEdit->setText(currentWorkspace.getDirIncludes());
    mUi->deliveryDirEdit->setText(currentWorkspace.getDirDelivery());
    mUi->logDirEdit->setText(currentWorkspace.getDirLogs());
    mUi->workspaceEdit->setPlainText(currentWorkspace.getWorkspaceDescription());
}

void ProjectDirSettings::applyChanges() const
{
    WorkspaceEntry currentWorkspace{ LusanApplication::getActiveWorkspace() };

    currentWorkspace.setWorkspaceRoot(mUi->rootDirEdit->text());
    currentWorkspace.setDirSources(mUi->sourceDirEdit->text());
    currentWorkspace.setDirIncludes(mUi->includeDirEdit->text());
    currentWorkspace.setDirDelivery(mUi->deliveryDirEdit->text());
    currentWorkspace.setDirLogs(mUi->logDirEdit->text());
    currentWorkspace.setWorkspaceDescription(mUi->workspaceEdit->toPlainText());


    OptionsManager& optionsManager = LusanApplication::getOptions();

    optionsManager.updateWorkspace(currentWorkspace);
    optionsManager.writeOptions();
}
