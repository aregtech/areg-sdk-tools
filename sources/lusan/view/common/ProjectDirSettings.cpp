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
#include <QFileDialog>

ProjectDirSettings::ProjectDirSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::projectDirSettingsDlg)
{
    ui->setupUi(this);
    connectSignalHandlers();
    initialisePathsWithCurrentWorkspaceData();
}

ProjectDirSettings::~ProjectDirSettings()
{
    delete ui;
}

void ProjectDirSettings::connectSignalHandlers() const
{
    connect(ui->rootDirBrowseBtn,       &QPushButton::clicked, this, &ProjectDirSettings::onRootDirBrowseBtnClicked);
    connect(ui->sourceDirBrowseBtn,     &QPushButton::clicked, this, &ProjectDirSettings::onSourceDirBrowseBtnClicked);
    connect(ui->includeDirBrowseBtn,    &QPushButton::clicked, this, &ProjectDirSettings::onIncludeDirBrowseBtnClicked);
    connect(ui->deliveryDirBrowseBtn,   &QPushButton::clicked, this, &ProjectDirSettings::onDeliveryDirBrowseBtnClicked);
    connect(ui->logDirBrowseBtn,        &QPushButton::clicked, this, &ProjectDirSettings::onLogDirBrowseBtnClicked);
}

void ProjectDirSettings::onRootDirBrowseBtnClicked()
{
    ui->rootDirEdit->setText(
        QFileDialog::getExistingDirectory(this, tr("Open Root Directory"), "", QFileDialog::ShowDirsOnly));
}

void ProjectDirSettings::onSourceDirBrowseBtnClicked()
{
    ui->sourceDirEdit->setText(
        QFileDialog::getExistingDirectory(this, tr("Open Source Directory"), "", QFileDialog::ShowDirsOnly));
}

void ProjectDirSettings::onIncludeDirBrowseBtnClicked()
{
    ui->includeDirEdit->setText(
        QFileDialog::getExistingDirectory(this, tr("Open Include Directory"), "", QFileDialog::ShowDirsOnly));
}

void ProjectDirSettings::onDeliveryDirBrowseBtnClicked()
{
    ui->deliveryDirEdit->setText(
        QFileDialog::getExistingDirectory(this, tr("Open Delivery Directory"), "", QFileDialog::ShowDirsOnly));
}

void ProjectDirSettings::onLogDirBrowseBtnClicked()
{
    ui->logDirEdit->setText(
        QFileDialog::getExistingDirectory(this, tr("Open Log Directory"), "", QFileDialog::ShowDirsOnly));
}

void ProjectDirSettings::initialisePathsWithCurrentWorkspaceData() const
{
    WorkspaceEntry const currentWorkspace{ LusanApplication::getActiveWorkspace() };

    ui->rootDirEdit->setText(currentWorkspace.getWorkspaceRoot());
    ui->sourceDirEdit->setText(currentWorkspace.getDirSources());
    ui->includeDirEdit->setText(currentWorkspace.getDirIncludes());
    ui->deliveryDirEdit->setText(currentWorkspace.getDirDelivery());
    ui->logDirEdit->setText(currentWorkspace.getDirLogs());
    ui->workspaceEdit->setPlainText(currentWorkspace.getWorkspaceDescription());
}

void ProjectDirSettings::applyChanges() const
{
    WorkspaceEntry currentWorkspace{ LusanApplication::getActiveWorkspace() };

    currentWorkspace.setWorkspaceRoot(ui->rootDirEdit->text());
    currentWorkspace.setDirSources(ui->sourceDirEdit->text());
    currentWorkspace.setDirIncludes(ui->includeDirEdit->text());
    currentWorkspace.setDirDelivery(ui->deliveryDirEdit->text());
    currentWorkspace.setDirLogs(ui->logDirEdit->text());
    currentWorkspace.setWorkspaceDescription(ui->workspaceEdit->toPlainText());


    OptionsManager& optionsManager = LusanApplication::getOptions();

    optionsManager.updateWorkspace(currentWorkspace);
    optionsManager.writeOptions();
}
