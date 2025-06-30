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
 *  \file        lusan/view/common/OfflineLogExplorer.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The offline log explorer implementation.
 *
 ************************************************************************/

#include "lusan/view/common/OfflineLogExplorer.hpp"
#include "ui/ui_OfflineLogExplorer.h"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/log/ScopeNodeBase.hpp"

#include "lusan/model/log/LogScopeIconFactory.hpp"
#include "lusan/model/log/LogScopesModel.hpp"

#include "lusan/view/common/MdiChild.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/log/LogViewer.hpp"

#include "aregextend/db/LogSqliteDatabase.hpp"
#include "areglogger/client/LogObserverApi.h"
#include "areg/component/NEService.hpp"
#include "areg/logging/NELogging.hpp"

#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTreeView>
#include <filesystem>

OfflineLogExplorer::OfflineLogExplorer(MdiMainWindow* wndMain, QWidget* parent)
    : NavigationWindow(NavigationWindow::eNavigationWindow::NaviOfflineLogs, wndMain, parent)

    , ui            (new Ui::OfflineLogExplorer)
    , mDatabasePath ( )
    , mDatabase     (nullptr)
    , mModel        (nullptr)
    , mSelModel     (nullptr)
    , mSignalsActive(false)
    , mActiveViewer (nullptr)
    , mMenuActions  (static_cast<int>(eLogActions::PrioCount))
{
    ui->setupUi(this);
    this->setBaseSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    this->setMinimumSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    this->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    mDatabase = new LogSqliteDatabase();
    
    updateData();
    setupWidgets();
    setupSignals();
}

OfflineLogExplorer::~OfflineLogExplorer(void)
{
    if (mDatabase != nullptr)
    {
        mDatabase->disconnect();
        delete mDatabase;
        mDatabase = nullptr;
    }
    
    if (mModel != nullptr)
    {
        delete mModel;
        mModel = nullptr;
    }
    
    delete ui;
}

bool OfflineLogExplorer::openLogDatabase(const QString& dbFilePath)
{
    if (dbFilePath.isEmpty())
        return false;

    // Close any existing database
    closeLogDatabase();

    if (mDatabase->connect(dbFilePath.toStdString().c_str()))
    {
        mDatabasePath = dbFilePath;
        loadScopeData();
        enableButtons();
        return true;
    }
    else
    {
        QMessageBox::warning(this, tr("Database Error"), tr("Failed to open log database: %1").arg(dbFilePath));
        return false;
    }
}

void OfflineLogExplorer::closeLogDatabase(void)
{
    if (mDatabase != nullptr && mDatabase->isOperable())
    {
        mDatabase->disconnect();
    }
    
    mDatabasePath.clear();
    
    if (mModel != nullptr)
    {
        mModel->release();
    }
    
    enableButtons();
}

void OfflineLogExplorer::updateForActiveViewer(LogViewer* activeViewer)
{
    mActiveViewer = activeViewer;
    
    if (activeViewer != nullptr)
    {
        // Get the database path from the active viewer and update if different
        QString viewerDbPath = activeViewer->currentFile();
        if (!viewerDbPath.isEmpty() && viewerDbPath != mDatabasePath)
        {
            openLogDatabase(viewerDbPath);
        }
    }
}

void OfflineLogExplorer::optionOpenning(void)
{
    // Implementation for when options dialog is opened
}

void OfflineLogExplorer::optionApplied(void)
{
    // Implementation for when options are applied
}

void OfflineLogExplorer::optionClosed(bool OKpressed)
{
    // Implementation for when options dialog is closed
}

void OfflineLogExplorer::setupWidgets(void)
{
    mModel = new LogScopesModel(this);
    mSelModel = new QItemSelectionModel(mModel, this);
    
    ctrlTreeView()->setModel(mModel);
    ctrlTreeView()->setSelectionModel(mSelModel);
    
    // Set up tree view properties
    ctrlTreeView()->setUniformRowHeights(true);
    ctrlTreeView()->setHeaderHidden(false);
    ctrlTreeView()->setRootIsDecorated(true);
    ctrlTreeView()->setAnimated(true);
    ctrlTreeView()->setSortingEnabled(false);
    
    // Initially disable database-dependent buttons
    enableButtons();
}

void OfflineLogExplorer::setupSignals(void)
{
    // Connect toolbar button signals
    connect(ctrlOpenDatabase(), &QToolButton::clicked, this, &OfflineLogExplorer::onOpenDatabaseClicked);
    connect(ctrlCloseDatabase(), &QToolButton::clicked, this, &OfflineLogExplorer::onCloseDatabaseClicked);
    connect(ctrlCollapse(), &QToolButton::toggled, this, &OfflineLogExplorer::onCollapseClicked);
    connect(ctrlResetFilters(), &QToolButton::clicked, this, &OfflineLogExplorer::onResetFiltersClicked);
    connect(ctrlMoveBottom(), &QToolButton::clicked, this, &OfflineLogExplorer::onMoveBottomClicked);
    
    // Connect priority filter buttons
    connect(ctrlPrioError(), &QToolButton::toggled, this, &OfflineLogExplorer::onPrioErrorClicked);
    connect(ctrlPrioWarning(), &QToolButton::toggled, this, &OfflineLogExplorer::onPrioWarningClicked);
    connect(ctrlPrioInfo(), &QToolButton::toggled, this, &OfflineLogExplorer::onPrioInfoClicked);
    connect(ctrlPrioDebug(), &QToolButton::toggled, this, &OfflineLogExplorer::onPrioDebugClicked);
    connect(ctrlPrioScopes(), &QToolButton::clicked, this, &OfflineLogExplorer::onPrioScopesClicked);
    
    // Connect selection model
    connect(mSelModel, &QItemSelectionModel::selectionChanged, this, &OfflineLogExplorer::onSelectionChanged);
    
    // Connect model signals
    connect(mModel, &LogScopesModel::signalScopesInserted, this, &OfflineLogExplorer::onRootUpdated);
    
    mSignalsActive = true;
}

void OfflineLogExplorer::updateData(void)
{
    // Update data from database if loaded
    if (isDatabaseLoaded())
    {
        loadScopeData();
    }
}

void OfflineLogExplorer::loadScopeData(void)
{
    if (!isDatabaseLoaded() || mModel == nullptr)
        return;
        
    try
    {
        mModel->release();
        mModel->initialize();
        
        // For now, just initialize the model
        // In a complete implementation, we would populate the model 
        // with data from the offline database using LogSqliteDatabase
        // However, this requires either:
        // 1. A custom offline scope model, or
        // 2. Modifications to LogScopesModel to work with offline data
        // For this initial implementation, we'll just show the structure
    }
    catch (const std::exception& ex)
    {
        QMessageBox::warning(this, tr("Database Error"), tr("Error loading scope data: %1").arg(ex.what()));
    }
}

void OfflineLogExplorer::enableButtons(const QModelIndex& index)
{
    bool dbLoaded = isDatabaseLoaded();
    
    // Enable/disable database operation buttons
    ctrlOpenDatabase()->setEnabled(true); // Always enabled
    ctrlCloseDatabase()->setEnabled(dbLoaded);
    
    // Enable/disable scope operation buttons
    ctrlCollapse()->setEnabled(dbLoaded);
    ctrlResetFilters()->setEnabled(dbLoaded);
    ctrlPrioError()->setEnabled(dbLoaded);
    ctrlPrioWarning()->setEnabled(dbLoaded);
    ctrlPrioInfo()->setEnabled(dbLoaded);
    ctrlPrioDebug()->setEnabled(dbLoaded);
    ctrlPrioScopes()->setEnabled(dbLoaded && index.isValid());
    
    // Enable/disable navigation buttons based on active viewer
    bool hasActiveViewer = (mActiveViewer != nullptr);
    ctrlMoveBottom()->setEnabled(hasActiveViewer);
}

void OfflineLogExplorer::onOpenDatabaseClicked(void)
{
    QString fileName = QFileDialog::getOpenFileName(this, 
                                                  tr("Open Log Database"), 
                                                  QString(), 
                                                  tr("Database Files (*.db *.sqlite *.sqlite3);;All Files (*.*)"));
    if (!fileName.isEmpty())
    {
        openLogDatabase(fileName);
        
        // Notify main window about database opening
        if (mMainWindow != nullptr)
        {
            mMainWindow->openFile(fileName);
        }
    }
}

void OfflineLogExplorer::onCloseDatabaseClicked(void)
{
    closeLogDatabase();
}

void OfflineLogExplorer::onCollapseClicked(bool checked)
{
    if (ctrlTreeView() != nullptr)
    {
        if (checked)
        {
            ctrlTreeView()->collapseAll();
        }
        else
        {
            ctrlTreeView()->expandAll();
        }
    }
}

void OfflineLogExplorer::onResetFiltersClicked(void)
{
    // Reset all priority filter buttons to default state
    ctrlPrioError()->setChecked(true);
    ctrlPrioWarning()->setChecked(true);
    ctrlPrioInfo()->setChecked(true);
    ctrlPrioDebug()->setChecked(false);
}

void OfflineLogExplorer::onMoveBottomClicked(void)
{
    if (mActiveViewer != nullptr)
    {
        mActiveViewer->moveToBottom(true);
    }
}

void OfflineLogExplorer::onPrioErrorClicked(bool checked)
{
    // Implementation for error priority filter
    // This is a filter setting, not a runtime change
}

void OfflineLogExplorer::onPrioWarningClicked(bool checked)
{
    // Implementation for warning priority filter
    // This is a filter setting, not a runtime change
}

void OfflineLogExplorer::onPrioInfoClicked(bool checked)
{
    // Implementation for info priority filter
    // This is a filter setting, not a runtime change
}

void OfflineLogExplorer::onPrioDebugClicked(bool checked)
{
    // Implementation for debug priority filter
    // This is a filter setting, not a runtime change
}

void OfflineLogExplorer::onPrioScopesClicked(bool checked)
{
    // Implementation for changing scope priorities
    // This is a filter setting, not a runtime change
}

void OfflineLogExplorer::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndex current = mSelModel->currentIndex();
    enableButtons(current);
}

void OfflineLogExplorer::onRootUpdated(const QModelIndex & root)
{
    enableButtons();
    if (ctrlTreeView() != nullptr)
    {
        ctrlTreeView()->expandToDepth(1); // Expand to show instances
    }
}

QTreeView* OfflineLogExplorer::ctrlTreeView(void) const
{
    return ui->treeView;
}

QToolButton* OfflineLogExplorer::ctrlOpenDatabase(void) const
{
    return ui->toolOpenDatabase;
}

QToolButton* OfflineLogExplorer::ctrlCloseDatabase(void) const
{
    return ui->toolCloseDatabase;
}

QToolButton* OfflineLogExplorer::ctrlCollapse(void) const
{
    return ui->toolCollapse;
}

QToolButton* OfflineLogExplorer::ctrlResetFilters(void) const
{
    return ui->toolResetFilters;
}

QToolButton* OfflineLogExplorer::ctrlMoveBottom(void) const
{
    return ui->toolMoveBottom;
}

QToolButton* OfflineLogExplorer::ctrlPrioError(void) const
{
    return ui->toolError;
}

QToolButton* OfflineLogExplorer::ctrlPrioWarning(void) const
{
    return ui->toolWarning;
}

QToolButton* OfflineLogExplorer::ctrlPrioInfo(void) const
{
    return ui->toolInformation;
}

QToolButton* OfflineLogExplorer::ctrlPrioDebug(void) const
{
    return ui->toolDebug;
}

QToolButton* OfflineLogExplorer::ctrlPrioScopes(void) const
{
    return ui->toolScopes;
}

bool OfflineLogExplorer::isDatabaseLoaded(void) const
{
    return (mDatabase != nullptr) && mDatabase->isOperable();
}