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

#include <QDialog>
#include <QFileDialog>

OptionPageProjectDirs::OptionPageProjectDirs(QDialog *parent)
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

void OptionPageProjectDirs::connectSignalHandlers()
{
    connect(mUi->sourceDirBrowseBtn     , &QPushButton::clicked, this, &OptionPageProjectDirs::onSourceDirBrowseBtnClicked);
    connect(mUi->includeDirBrowseBtn    , &QPushButton::clicked, this, &OptionPageProjectDirs::onIncludeDirBrowseBtnClicked);
    connect(mUi->deliveryDirBrowseBtn   , &QPushButton::clicked, this, &OptionPageProjectDirs::onDeliveryDirBrowseBtnClicked);
    connect(mUi->logDirBrowseBtn        , &QPushButton::clicked, this, &OptionPageProjectDirs::onLogDirBrowseBtnClicked);
    
    connect(ctrlSources()               , &QLineEdit::textChanged   , this, [this](){
        emit signalWorkspaceLocationsChanged( sWorkspaceDir{true, ctrlSources()->text()}
                                            , sWorkspaceDir{true, ctrlIncludes()->text()}
                                            , sWorkspaceDir{true, ctrlDelivery()->text()}
                                            , sWorkspaceDir{true, ctrlLogs()->text()});        
    });
    connect(ctrlIncludes()            , &QLineEdit::textChanged   , this, [this](){
        emit signalWorkspaceLocationsChanged( sWorkspaceDir{true, ctrlSources()->text()}
                                            , sWorkspaceDir{true, ctrlIncludes()->text()}
                                            , sWorkspaceDir{true, ctrlDelivery()->text()}
                                            , sWorkspaceDir{true, ctrlLogs()->text()});        
    });
    connect(ctrlDelivery()            , &QLineEdit::textChanged   , this, [this](){
        emit signalWorkspaceLocationsChanged( sWorkspaceDir{true, ctrlSources()->text()}
                                            , sWorkspaceDir{true, ctrlIncludes()->text()}
                                            , sWorkspaceDir{true, ctrlDelivery()->text()}
                                            , sWorkspaceDir{true, ctrlLogs()->text()});        
    });
    connect(ctrlLogs()                  , &QLineEdit::textChanged   , this, [this](){
        emit signalWorkspaceLocationsChanged( sWorkspaceDir{true, ctrlSources()->text()}
                                            , sWorkspaceDir{true, ctrlIncludes()->text()}
                                            , sWorkspaceDir{true, ctrlDelivery()->text()}
                                            , sWorkspaceDir{true, ctrlLogs()->text()});        
    });
}

void OptionPageProjectDirs::onSourceDirBrowseBtnClicked()
{
    QString oldDir(NELusanCommon::fixPath(ctrlSources()->text()));
    QString newDir(NELusanCommon::fixPath(QFileDialog::getExistingDirectory(this, tr("Open Source Directory"), oldDir, QFileDialog::ShowDirsOnly)));
    if ((newDir.isEmpty() == false) && (oldDir != newDir))
    {
        ctrlSources()->setText(newDir);
        setDataModified(true);
    }
}

void OptionPageProjectDirs::onIncludeDirBrowseBtnClicked()
{
    QString oldDir(NELusanCommon::fixPath(ctrlIncludes()->text()));
    QString newDir(NELusanCommon::fixPath(QFileDialog::getExistingDirectory(this, tr("Open Include Directory"), oldDir, QFileDialog::ShowDirsOnly)));
    if ((newDir.isEmpty() == false) && (oldDir != newDir))
    {
        ctrlIncludes()->setText(newDir);
        setDataModified(true);
    }
}

void OptionPageProjectDirs::onDeliveryDirBrowseBtnClicked()
{
    QString oldDir(NELusanCommon::fixPath(ctrlDelivery()->text()));
    QString newDir(NELusanCommon::fixPath(QFileDialog::getExistingDirectory(this, tr("Open Delivery Directory"), oldDir, QFileDialog::ShowDirsOnly)));
    if ((newDir.isEmpty() == false) && (oldDir != newDir))
    {
        ctrlDelivery()->setText(newDir);
        setDataModified(true);
    }
}

void OptionPageProjectDirs::onLogDirBrowseBtnClicked()
{
    QString oldDir(NELusanCommon::fixPath(ctrlLogs()->text()));
    QString newDir(NELusanCommon::fixPath(QFileDialog::getExistingDirectory(this, tr("Open Log Directory"), oldDir, QFileDialog::ShowDirsOnly)));
    if ((newDir.isEmpty() == false) && (oldDir != newDir))
    {
        ctrlLogs()->setText(newDir);
        setDataModified(true);
    }
}

void OptionPageProjectDirs::initializePathsWithCurrentWorkspaceData() const
{
    WorkspaceEntry const currentWorkspace{ LusanApplication::getActiveWorkspace() };

    ctrlRoot()->setText(currentWorkspace.getWorkspaceRoot());
    ctrlSources()->setText(currentWorkspace.getDirSources());
    ctrlIncludes()->setText(currentWorkspace.getDirIncludes());
    ctrlDelivery()->setText(currentWorkspace.getDirDelivery());
    ctrlLogs()->setText(currentWorkspace.getDirLogs());
    mUi->workspaceEdit->setPlainText(currentWorkspace.getWorkspaceDescription());
}

void OptionPageProjectDirs::applyChanges()
{
    WorkspaceEntry currentWorkspace{ LusanApplication::getActiveWorkspace() };

    currentWorkspace.setWorkspaceRoot(ctrlRoot()->text());
    currentWorkspace.setDirSources(ctrlSources()->text());
    currentWorkspace.setDirIncludes(ctrlIncludes()->text());
    currentWorkspace.setDirDelivery(ctrlDelivery()->text());
    currentWorkspace.setDirLogs(ctrlLogs()->text());
    currentWorkspace.setWorkspaceDescription(mUi->workspaceEdit->toPlainText());

    OptionsManager& optionsManager = LusanApplication::getOptions();
    optionsManager.updateWorkspace(currentWorkspace);
    optionsManager.writeOptions();
    
    OptionPageBase::applyChanges();
}

void OptionPageProjectDirs::updateWorkspaceDirectories( const sWorkspaceDir& /*sources*/
                                                      , const sWorkspaceDir& /*includes*/
                                                      , const sWorkspaceDir& /*delivery*/
                                                      , const sWorkspaceDir& logs)
{
    if (logs.isValid && (ctrlLogs()->text() != logs.location))
    {
        ctrlLogs()->blockSignals(true);
        ctrlLogs()->setText(logs.location);
        ctrlLogs()->blockSignals(false);
    }
}

inline QLineEdit* OptionPageProjectDirs::ctrlRoot(void) const
{
    return mUi->rootDirEdit;
}

inline QLineEdit* OptionPageProjectDirs::ctrlSources(void) const
{
    return mUi->sourceDirEdit;
}

inline QLineEdit* OptionPageProjectDirs::ctrlIncludes(void) const
{
    return mUi->includeDirEdit;
}

inline QLineEdit* OptionPageProjectDirs::ctrlDelivery(void) const
{
    return mUi->deliveryDirEdit;
}

inline QLineEdit* OptionPageProjectDirs::ctrlLogs(void) const
{
    return mUi->logDirEdit;
}
