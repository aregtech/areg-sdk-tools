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
 *  \file        lusan/view/common/LogOfflineExplorer.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the offline log explorer.
 *
 ************************************************************************/

#include "lusan/view/common/LogOfflineExplorer.hpp"
#include "ui/ui_LogOfflineExplorer.h"

#include "lusan/model/log/LogOfflineModel.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QFileDialog>
#include <QToolButton>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMessageBox>

LogOfflineExplorer::LogOfflineExplorer(MdiMainWindow* wndMain, QWidget* parent)
    : NavigationWindow(NavigationWindow::eNavigationWindow::NaviOfflineLogs, wndMain, parent)

    , ui            (new Ui::LogOfflineExplorer)
    , mModel        (nullptr)
    , mDatabasePath ()
{
    ui->setupUi(this);
    this->setBaseSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    this->setMinimumSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);

    setupWidgets();
    setupSignals();
    updateControls();
}

LogOfflineExplorer::~LogOfflineExplorer(void)
{
    if (mModel != nullptr)
    {
        delete mModel;
        mModel = nullptr;
    }
    
    delete ui;
}

QString LogOfflineExplorer::getOpenedDatabasePath(void) const
{
    return mDatabasePath;
}

bool LogOfflineExplorer::openDatabase(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        return false;
    }

    // Close existing database if open
    closeDatabase();

    // Create new model if needed
    if (mModel == nullptr)
    {
        mModel = new LogOfflineModel(this);
    }

    // Try to open the database
    if (mModel->openDatabase(filePath))
    {
        mDatabasePath = filePath;
        updateControls();
        showDatabaseInfo();
        return true;
    }
    else
    {
        QMessageBox::warning(this, tr("Database Error"), 
                           tr("Failed to open database file:\n%1").arg(filePath));
        return false;
    }
}

void LogOfflineExplorer::closeDatabase(void)
{
    if (mModel != nullptr && mModel->isDatabaseOpen())
    {
        mModel->closeDatabase();
    }
    
    mDatabasePath.clear();
    updateControls();
    
    // Clear the tree view
    ctrlDatabaseInfo()->setModel(nullptr);
}

bool LogOfflineExplorer::isDatabaseOpen(void) const
{
    return (mModel != nullptr) && mModel->isDatabaseOpen();
}

void LogOfflineExplorer::optionOpenning(void)
{
    // Called when options dialog is opened
    // No specific actions needed for offline explorer
}

void LogOfflineExplorer::optionApplied(void)
{
    // Called when apply button is pressed in options dialog
    // No specific actions needed for offline explorer
}

void LogOfflineExplorer::optionClosed(bool OKpressed)
{
    // Called when options dialog is closed
    // No specific actions needed for offline explorer
    Q_UNUSED(OKpressed);
}

QToolButton* LogOfflineExplorer::ctrlOpenDatabase(void)
{
    return ui->toolOpenDatabase;
}

QToolButton* LogOfflineExplorer::ctrlCloseDatabase(void)
{
    return ui->toolCloseDatabase;
}

QToolButton* LogOfflineExplorer::ctrlRefreshDatabase(void)
{
    return ui->toolRefreshDatabase;
}

QTreeView* LogOfflineExplorer::ctrlDatabaseInfo(void)
{
    return ui->treeDatabaseInfo;
}

void LogOfflineExplorer::setupWidgets(void)
{
    // Configure the tree view for database information display
    ctrlDatabaseInfo()->setHeaderHidden(false);
    ctrlDatabaseInfo()->setRootIsDecorated(true);
    ctrlDatabaseInfo()->setAlternatingRowColors(true);
}

void LogOfflineExplorer::setupSignals(void)
{
    // Connect tool button signals
    connect(ctrlOpenDatabase(), &QToolButton::clicked, this, &LogOfflineExplorer::onOpenDatabaseClicked);
    connect(ctrlCloseDatabase(), &QToolButton::clicked, this, &LogOfflineExplorer::onCloseDatabaseClicked);
    connect(ctrlRefreshDatabase(), &QToolButton::clicked, this, &LogOfflineExplorer::onRefreshDatabaseClicked);
}

void LogOfflineExplorer::updateControls(void)
{
    bool dbOpen = isDatabaseOpen();
    
    ctrlCloseDatabase()->setEnabled(dbOpen);
    ctrlRefreshDatabase()->setEnabled(dbOpen);
}

void LogOfflineExplorer::showDatabaseInfo(void)
{
    if (!isDatabaseOpen())
    {
        ctrlDatabaseInfo()->setModel(nullptr);
        return;
    }

    // Create a simple model to show database information
    QStandardItemModel* infoModel = new QStandardItemModel(this);
    infoModel->setHorizontalHeaderLabels(QStringList() << tr("Database Information"));

    // Add database file path
    QStandardItem* dbPathItem = new QStandardItem(tr("Database File"));
    dbPathItem->appendRow(new QStandardItem(mDatabasePath));
    infoModel->appendRow(dbPathItem);

    // Add database status
    QStandardItem* statusItem = new QStandardItem(tr("Status"));
    statusItem->appendRow(new QStandardItem(tr("Connected")));
    infoModel->appendRow(statusItem);

    // Get some basic information from the model
    if (mModel != nullptr)
    {
        try 
        {
            std::vector<String> instanceNames;
            mModel->getLogInstanceNames(instanceNames);
            
            QStandardItem* instancesItem = new QStandardItem(tr("Instances (%1)").arg(instanceNames.size()));
            for (const auto& name : instanceNames)
            {
                instancesItem->appendRow(new QStandardItem(QString::fromStdString(name.getData())));
            }
            infoModel->appendRow(instancesItem);

            std::vector<String> threadNames;
            mModel->getLogThreadNames(threadNames);
            
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

    ctrlDatabaseInfo()->setModel(infoModel);
    ctrlDatabaseInfo()->expandAll();
}

void LogOfflineExplorer::onOpenDatabaseClicked(void)
{
    QString filter = QString("Database Files (*%1);;All Files (*.*)").arg(LogOfflineModel::getFileExtension());
    QString filePath = QFileDialog::getOpenFileName(this, 
                                                  tr("Open Log Database"), 
                                                  QString(), 
                                                  filter);

    if (!filePath.isEmpty())
    {
        openDatabase(filePath);
    }
}

void LogOfflineExplorer::onCloseDatabaseClicked(void)
{
    closeDatabase();
}

void LogOfflineExplorer::onRefreshDatabaseClicked(void)
{
    if (isDatabaseOpen())
    {
        showDatabaseInfo();
    }
}