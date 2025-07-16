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
 *  \file        lusan/view/common/NaviOfflineLogsScopes.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the offline log explorer.
 *
 ************************************************************************/

#include "lusan/view/common/NaviOfflineLogsScopes.hpp"
#include "ui/ui_NaviOfflineLogsScopes.h"

#include "lusan/model/log/OfflineLogsModel.hpp"
#include "lusan/model/log/OfflineScopesModel.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/app/LusanApplication.hpp"

#include <QFileDialog>
#include <QToolButton>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMessageBox>

NaviOfflineLogsScopes::NaviOfflineLogsScopes(MdiMainWindow* wndMain, QWidget* parent)
    : NavigationWindow(static_cast<int>(Navigation::eNaviWindow::NaviOfflineLogs), wndMain, parent)

    , ui            (new Ui::NaviOfflineLogsScopes)
    , mScopesModel  (new OfflineScopesModel(this))

{
    ui->setupUi(this);
    setBaseSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    setMinimumSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);

    setupWidgets();
    setupSignals();
    updateControls();
    ctrlTable()->setModel(mScopesModel);
    
}

NaviOfflineLogsScopes::~NaviOfflineLogsScopes(void)
{
    ctrlTable()->setModel(nullptr);
    delete mScopesModel;
    delete ui;
}

QString NaviOfflineLogsScopes::getOpenedDatabasePath(void) const
{
    LoggingModelBase* logModel{ getLoggingModel() };
    return (logModel != nullptr ? logModel->getLogFileName() : QString());
}

bool NaviOfflineLogsScopes::openDatabase(const QString& filePath)
{
    LoggingModelBase* logModel{ getLoggingModel() };
    if (filePath.isEmpty() || (logModel == nullptr))
    {
        return false;
    }

    // Try to open the database
    logModel->openDatabase(filePath, true);
    if (logModel->isOperable())
    {
        mScopesModel->setLoggingModel(logModel);
        return true;
    }
    else
    {
        mScopesModel->setLoggingModel(nullptr);
        QMessageBox::warning(this, tr("Database Error"), tr("Failed to open database file:\n%1").arg(filePath));
        return false;
    }
}

void NaviOfflineLogsScopes::closeDatabase(void)
{
    // Clear the tree view
    mScopesModel->setLoggingModel(nullptr);
    updateControls();    
}

bool NaviOfflineLogsScopes::isDatabaseOpen(void) const
{
    LoggingModelBase* logModel{ getLoggingModel() };
    return (logModel != nullptr) && logModel->isOperable();
}

void NaviOfflineLogsScopes::setLoggingModel(OfflineLogsModel * model)
{
    mScopesModel->setLoggingModel(model);
    updateControls();
}

OfflineLogsModel* NaviOfflineLogsScopes::getLoggingModel(void)
{
    return static_cast<OfflineLogsModel *>(mScopesModel->getLoggingModel());
}

void NaviOfflineLogsScopes::optionOpenning(void)
{
    // Called when options dialog is opened
    // No specific actions needed for offline explorer
}

void NaviOfflineLogsScopes::optionApplied(void)
{
    // Called when apply button is pressed in options dialog
    // No specific actions needed for offline explorer
}

void NaviOfflineLogsScopes::optionClosed(bool OKpressed)
{
    // Called when options dialog is closed
    // No specific actions needed for offline explorer
    Q_UNUSED(OKpressed);
}

QToolButton* NaviOfflineLogsScopes::ctrlOpenDatabase(void)
{
    return ui->toolDbOpen;
}

QToolButton* NaviOfflineLogsScopes::ctrlCloseDatabase(void)
{
    return ui->toolDbClose;
}

QToolButton* NaviOfflineLogsScopes::ctrlRefreshDatabase(void)
{
    return ui->toolRefresh;
}

QTreeView* NaviOfflineLogsScopes::ctrlTable(void)
{
    return ui->treeView;
}

QToolButton* NaviOfflineLogsScopes::ctrlFind(void)
{
    return ui->toolFind;
}

QToolButton* NaviOfflineLogsScopes::ctrlLogError(void)
{
    return ui->toolError;
}

QToolButton* NaviOfflineLogsScopes::ctrlLogWarning(void)
{
    return ui->toolWarning;
}

QToolButton* NaviOfflineLogsScopes::ctrlLogInfo(void)
{
    return ui->toolInformation;
}

QToolButton* NaviOfflineLogsScopes::ctrlLogDebug(void)
{
    return ui->toolDebug;
}

QToolButton* NaviOfflineLogsScopes::ctrlLogScopes(void)
{
    return ui->toolScopes;
}

QToolButton* NaviOfflineLogsScopes::ctrlMoveTop(void)
{
    return ui->toolMoveTop;
}

QToolButton* NaviOfflineLogsScopes::ctrlMoveBottom(void)
{
    return ui->toolMoveBottom;
}

LoggingModelBase* NaviOfflineLogsScopes::getLoggingModel(void) const
{
    Q_ASSERT(mScopesModel != nullptr);
    return mScopesModel->getLoggingModel();
}

void NaviOfflineLogsScopes::setupWidgets(void)
{
    // Configure the tree view for database information display
    ctrlTable()->setHeaderHidden(false);
    ctrlTable()->setRootIsDecorated(true);
    ctrlTable()->setAlternatingRowColors(true);
}

void NaviOfflineLogsScopes::setupSignals(void)
{
    // Connect tool button signals
    connect(ctrlOpenDatabase()      , &QToolButton::clicked, this, &NaviOfflineLogsScopes::onOpenDatabaseClicked);
    connect(ctrlCloseDatabase()     , &QToolButton::clicked, this, &NaviOfflineLogsScopes::onCloseDatabaseClicked);
    connect(ctrlRefreshDatabase()   , &QToolButton::clicked, this, &NaviOfflineLogsScopes::onRefreshDatabaseClicked);
    connect(mScopesModel            , &OfflineScopesModel::signalRootUpdated    , this, &NaviOfflineLogsScopes::onRootUpdated);
    connect(mScopesModel            , &OfflineScopesModel::signalScopesInserted , this, &NaviOfflineLogsScopes::onScopesInserted);
}

void NaviOfflineLogsScopes::updateControls(void)
{
    bool dbOpen = isDatabaseOpen();
    
    ctrlCloseDatabase()->setEnabled(dbOpen);
    ctrlRefreshDatabase()->setEnabled(dbOpen);
}

void NaviOfflineLogsScopes::showDatabaseInfo(void)
{
    Q_ASSERT(false);
    LoggingModelBase* logModel{ getLoggingModel() };

    if ((logModel == nullptr) || (isDatabaseOpen() == false))
    {
        ctrlTable()->setModel(nullptr);
        return;
    }

    // Create a simple model to show database information
    QStandardItemModel* infoModel = new QStandardItemModel(this);
    infoModel->setHorizontalHeaderLabels(QStringList() << tr("Database Information"));

    // Add database file path
    QStandardItem* dbPathItem = new QStandardItem(tr("Database File"));
    dbPathItem->appendRow(new QStandardItem(logModel->getLogFileName()));
    infoModel->appendRow(dbPathItem);

    // Add database status
    QStandardItem* statusItem = new QStandardItem(tr("Status"));
    statusItem->appendRow(new QStandardItem(tr("Connected")));
    infoModel->appendRow(statusItem);

    // Get some basic information from the model
    try
    {
        std::vector<String> instanceNames;
        logModel->getLogInstanceNames(instanceNames);

        QStandardItem* instancesItem = new QStandardItem(tr("Instances (%1)").arg(instanceNames.size()));
        for (const auto& name : instanceNames)
        {
            instancesItem->appendRow(new QStandardItem(QString::fromStdString(name.getData())));
        }

        infoModel->appendRow(instancesItem);

        std::vector<String> threadNames;
        logModel->getLogThreadNames(threadNames);

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

    ctrlTable()->setModel(infoModel);
    ctrlTable()->expandAll();
}

void NaviOfflineLogsScopes::onOpenDatabaseClicked(void)
{
    QString filePath = mMainWindow->openLogFile();
    if (filePath.isEmpty() == false)
    {
        openDatabase(filePath);
    }
}

void NaviOfflineLogsScopes::onCloseDatabaseClicked(void)
{
    closeDatabase();
}

void NaviOfflineLogsScopes::onRefreshDatabaseClicked(void)
{
    LoggingModelBase* logModel{ getLoggingModel() };
    if ((logModel != nullptr) && isDatabaseOpen())
    {
        mScopesModel->setLoggingModel(nullptr);
        mScopesModel->setLoggingModel(logModel);
    }
}

void NaviOfflineLogsScopes::onRootUpdated(const QModelIndex& root)
{
    Q_ASSERT(mScopesModel != nullptr);
    QTreeView* navi = ctrlTable();
    Q_ASSERT(navi != nullptr);
    if (navi->isExpanded(root) == false)
    {
        navi->expand(root);
    }

    // Ensure all children of root are expanded and visible
    int rowCount = mScopesModel->rowCount(root);
    for (int row = 0; row < rowCount; ++row)
    {
        QModelIndex child = mScopesModel->index(row, 0, root);
        if (child.isValid() && !navi->isExpanded(child))
        {
            navi->expand(child);
        }
    }
}

void NaviOfflineLogsScopes::onScopesInserted(const QModelIndex& parent)
{
    Q_ASSERT(mScopesModel != nullptr);
    if (parent.isValid())
    {
        QTreeView* navi = ctrlTable();
        Q_ASSERT(navi != nullptr);
        if (navi->isExpanded(parent) == false)
        {
            navi->expand(parent);
        }
    }
}
