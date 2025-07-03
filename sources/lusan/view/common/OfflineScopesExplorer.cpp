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
 *  \file        lusan/view/common/OfflineScopesExplorer.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the offline log explorer.
 *
 ************************************************************************/

#include "lusan/view/common/OfflineScopesExplorer.hpp"
#include "ui/ui_OfflineScopesExplorer.h"

#include "lusan/model/log/LogOfflineModel.hpp"
#include "lusan/model/log/LogOfflineScopesModel.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/app/LusanApplication.hpp"

#include <QFileDialog>
#include <QToolButton>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMessageBox>

OfflineScopesExplorer::OfflineScopesExplorer(MdiMainWindow* wndMain, QWidget* parent)
    : NavigationWindow(static_cast<int>(Navigation::eNaviWindow::NaviOfflineLogs), wndMain, parent)

    , ui            (new Ui::OfflineScopesExplorer)
    , mLogModel     (nullptr)
    , mScopesModel  (new LogOfflineScopesModel(this))

{
    ui->setupUi(this);
    this->setBaseSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    this->setMinimumSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);

    setupWidgets();
    setupSignals();
    updateControls();
}

OfflineScopesExplorer::~OfflineScopesExplorer(void)
{
    ctrlTable()->setModel(nullptr);
    delete mScopesModel;
    delete ui;
}

QString OfflineScopesExplorer::getOpenedDatabasePath(void) const
{
    return (mLogModel != nullptr ? mLogModel->getLogFileName() : QString());
}

bool OfflineScopesExplorer::openDatabase(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        return false;
    }

    // Try to open the database
    mLogModel->openDatabase(filePath, true);
    if (mLogModel->isOperable() && mScopesModel->setScopeModel(mLogModel))
    {
        ctrlTable()->setModel(nullptr);
        ctrlTable()->setModel(mScopesModel);
        // updateControls();
        // showDatabaseInfo();
        return true;
    }
    else
    {
        QMessageBox::warning(this, tr("Database Error"), tr("Failed to open database file:\n%1").arg(filePath));
        return false;
    }
}

void OfflineScopesExplorer::closeDatabase(void)
{
    mScopesModel->release();
    updateControls();    
    mLogModel = nullptr;
    // Clear the tree view
    ctrlTable()->setModel(nullptr);
}

bool OfflineScopesExplorer::isDatabaseOpen(void) const
{
    return (mLogModel != nullptr) && mLogModel->isOperable();
}

void OfflineScopesExplorer::setLoggingModel(LogOfflineModel * model)
{
    mLogModel = model;
    mScopesModel->setScopeModel(model);
    updateControls();
}

void OfflineScopesExplorer::optionOpenning(void)
{
    // Called when options dialog is opened
    // No specific actions needed for offline explorer
}

void OfflineScopesExplorer::optionApplied(void)
{
    // Called when apply button is pressed in options dialog
    // No specific actions needed for offline explorer
}

void OfflineScopesExplorer::optionClosed(bool OKpressed)
{
    // Called when options dialog is closed
    // No specific actions needed for offline explorer
    Q_UNUSED(OKpressed);
}

QToolButton* OfflineScopesExplorer::ctrlOpenDatabase(void)
{
    return ui->toolDbOpen;
}

QToolButton* OfflineScopesExplorer::ctrlCloseDatabase(void)
{
    return ui->toolDbClose;
}

QToolButton* OfflineScopesExplorer::ctrlRefreshDatabase(void)
{
    return ui->toolRefresh;
}

QTreeView* OfflineScopesExplorer::ctrlTable(void)
{
    return ui->treeView;
}

QToolButton* OfflineScopesExplorer::ctrlFind(void)
{
    return ui->toolFind;
}

QToolButton* OfflineScopesExplorer::ctrlLogError(void)
{
    return ui->toolError;
}

QToolButton* OfflineScopesExplorer::ctrlLogWarning(void)
{
    return ui->toolWarning;
}

QToolButton* OfflineScopesExplorer::ctrlLogInfo(void)
{
    return ui->toolInformation;
}

QToolButton* OfflineScopesExplorer::ctrlLogDebug(void)
{
    return ui->toolDebug;
}

QToolButton* OfflineScopesExplorer::ctrlLogScopes(void)
{
    return ui->toolScopes;
}

QToolButton* OfflineScopesExplorer::ctrlMoveTop(void)
{
    return ui->toolMoveTop;
}

QToolButton* OfflineScopesExplorer::ctrlMoveBottom(void)
{
    return ui->toolMoveBottom;
}

void OfflineScopesExplorer::setupWidgets(void)
{
    // Configure the tree view for database information display
    ctrlTable()->setHeaderHidden(false);
    ctrlTable()->setRootIsDecorated(true);
    ctrlTable()->setAlternatingRowColors(true);
}

void OfflineScopesExplorer::setupSignals(void)
{
    // Connect tool button signals
    connect(ctrlOpenDatabase()      , &QToolButton::clicked, this, &OfflineScopesExplorer::onOpenDatabaseClicked);
    connect(ctrlCloseDatabase()     , &QToolButton::clicked, this, &OfflineScopesExplorer::onCloseDatabaseClicked);
    connect(ctrlRefreshDatabase()   , &QToolButton::clicked, this, &OfflineScopesExplorer::onRefreshDatabaseClicked);
}

void OfflineScopesExplorer::updateControls(void)
{
    bool dbOpen = isDatabaseOpen();
    
    ctrlCloseDatabase()->setEnabled(dbOpen);
    ctrlRefreshDatabase()->setEnabled(dbOpen);
}

void OfflineScopesExplorer::showDatabaseInfo(void)
{
    if (!isDatabaseOpen())
    {
        ctrlTable()->setModel(nullptr);
        return;
    }

    // Create a simple model to show database information
    QStandardItemModel* infoModel = new QStandardItemModel(this);
    infoModel->setHorizontalHeaderLabels(QStringList() << tr("Database Information"));

    // Add database file path
    QStandardItem* dbPathItem = new QStandardItem(tr("Database File"));
    dbPathItem->appendRow(new QStandardItem(mLogModel->getLogFileName()));
    infoModel->appendRow(dbPathItem);

    // Add database status
    QStandardItem* statusItem = new QStandardItem(tr("Status"));
    statusItem->appendRow(new QStandardItem(tr("Connected")));
    infoModel->appendRow(statusItem);

    // Get some basic information from the model
    if (mLogModel != nullptr)
    {
        try 
        {
            std::vector<String> instanceNames;
            mLogModel->getLogInstanceNames(instanceNames);
            
            QStandardItem* instancesItem = new QStandardItem(tr("Instances (%1)").arg(instanceNames.size()));
            for (const auto& name : instanceNames)
            {
                instancesItem->appendRow(new QStandardItem(QString::fromStdString(name.getData())));
            }

            infoModel->appendRow(instancesItem);

            std::vector<String> threadNames;
            mLogModel->getLogThreadNames(threadNames);
            
            QStandardItem* threadsItem = new QStandardItem(tr("Threads (%1)").arg(threadNames.size()));
            for (const auto& name : threadNames)
            {
                threadsItem->appendRow(new QStandardItem(QString::fromStdString(name.getData())));
            }

            infoModel->appendRow(threadsItem);
        }
        catch (...)
        {
            // If there's an error getting information, just show basic info
            QStandardItem* errorItem = new QStandardItem(tr("Error"));
            errorItem->appendRow(new QStandardItem(tr("Could not retrieve database information")));
            infoModel->appendRow(errorItem);
        }
    }

    ctrlTable()->setModel(infoModel);
    ctrlTable()->expandAll();
}

void OfflineScopesExplorer::onOpenDatabaseClicked(void)
{
    QString filePath = mMainWindow->openLogFile();
    if (filePath.isEmpty() == false)
    {
        openDatabase(filePath);
    }
}

void OfflineScopesExplorer::onCloseDatabaseClicked(void)
{
    closeDatabase();
}

void OfflineScopesExplorer::onRefreshDatabaseClicked(void)
{
    if (isDatabaseOpen())
    {
        mScopesModel->setScopeModel(nullptr);
        mScopesModel->setScopeModel(mLogModel);
    }
}
