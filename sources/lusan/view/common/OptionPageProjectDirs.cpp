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
 *  \file        lusan/view/common/OptionPageProjectDirs.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, project settings widget.
 *
 ************************************************************************/

#include "lusan/view/common/OptionPageProjectDirs.hpp"
#include "ui/ui_OptionPageProjectDirs.h"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/view/common/ProjectSettings.hpp"

#include <QFileDialog>

OptionPageProjectDirs::OptionPageProjectDirs(ProjectSettings *parent)
    : OptionPageBase(parent)
    , mUi           (std::make_unique<Ui::OptionPageProjectDirsDlg>())
{
    mUi->setupUi(this);
    connectSignalHandlers();
    initializePathsWithCurrentWorkspaceData();
}

OptionPageProjectDirs::~OptionPageProjectDirs()
{
}

void OptionPageProjectDirs::connectSignalHandlers() const
{
    connect(mUi->rootDirBrowseBtn,       &QPushButton::clicked, this, &OptionPageProjectDirs::onRootDirBrowseBtnClicked);
    connect(mUi->sourceDirBrowseBtn,     &QPushButton::clicked, this, &OptionPageProjectDirs::onSourceDirBrowseBtnClicked);
    connect(mUi->includeDirBrowseBtn,    &QPushButton::clicked, this, &OptionPageProjectDirs::onIncludeDirBrowseBtnClicked);
    connect(mUi->deliveryDirBrowseBtn,   &QPushButton::clicked, this, &OptionPageProjectDirs::onDeliveryDirBrowseBtnClicked);
    connect(mUi->logDirBrowseBtn,        &QPushButton::clicked, this, &OptionPageProjectDirs::onLogDirBrowseBtnClicked);
}

void OptionPageProjectDirs::onRootDirBrowseBtnClicked()
{
    QString oldDir(NELusanCommon::fixPath(mUi->rootDirEdit->text()));
    QString newDir(NELusanCommon::fixPath(QFileDialog::getExistingDirectory(this, tr("Open Root Directory"), oldDir, QFileDialog::ShowDirsOnly)));
    if (oldDir != newDir)
    {
        mUi->rootDirEdit->setText(newDir);
        setDataModified(true);
    }
}

void OptionPageProjectDirs::onSourceDirBrowseBtnClicked()
{
    QString oldDir(NELusanCommon::fixPath(mUi->sourceDirEdit->text()));
    QString newDir(NELusanCommon::fixPath(QFileDialog::getExistingDirectory(this, tr("Open Source Directory"), oldDir, QFileDialog::ShowDirsOnly)));
    if (oldDir != newDir)
    {
        mUi->sourceDirEdit->setText(newDir);
        setDataModified(true);
    }
}

void OptionPageProjectDirs::onIncludeDirBrowseBtnClicked()
{
    QString oldDir(NELusanCommon::fixPath(mUi->includeDirEdit->text()));
    QString newDir(NELusanCommon::fixPath(QFileDialog::getExistingDirectory(this, tr("Open Include Directory"), oldDir, QFileDialog::ShowDirsOnly)));
    if (oldDir != newDir)
    {
        mUi->includeDirEdit->setText(newDir);
        setDataModified(true);
    }
}

void OptionPageProjectDirs::onDeliveryDirBrowseBtnClicked()
{
    QString oldDir(NELusanCommon::fixPath(mUi->deliveryDirEdit->text()));
    QString newDir(NELusanCommon::fixPath(QFileDialog::getExistingDirectory(this, tr("Open Delivery Directory"), oldDir, QFileDialog::ShowDirsOnly)));
    if (oldDir != newDir)
    {
        mUi->deliveryDirEdit->setText(newDir);
        setDataModified(true);
    }
}

void OptionPageProjectDirs::onLogDirBrowseBtnClicked()
{
    QString oldDir(NELusanCommon::fixPath(mUi->logDirEdit->text()));
    QString newDir(NELusanCommon::fixPath(QFileDialog::getExistingDirectory(this, tr("Open Log Directory"), oldDir, QFileDialog::ShowDirsOnly)));
    if (oldDir != newDir)
    {
        mUi->logDirEdit->setText(newDir);
        setDataModified(true);
    }
}

void OptionPageProjectDirs::initializePathsWithCurrentWorkspaceData() const
{
    WorkspaceEntry const currentWorkspace{ LusanApplication::getActiveWorkspace() };

    mUi->rootDirEdit->setText(currentWorkspace.getWorkspaceRoot());
    mUi->sourceDirEdit->setText(currentWorkspace.getDirSources());
    mUi->includeDirEdit->setText(currentWorkspace.getDirIncludes());
    mUi->deliveryDirEdit->setText(currentWorkspace.getDirDelivery());
    mUi->logDirEdit->setText(currentWorkspace.getDirLogs());
    mUi->workspaceEdit->setPlainText(currentWorkspace.getWorkspaceDescription());
}

void OptionPageProjectDirs::applyChanges()
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
    
    OptionPageBase::applyChanges();
}
