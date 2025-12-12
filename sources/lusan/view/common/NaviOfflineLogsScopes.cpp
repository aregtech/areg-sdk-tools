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

#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/model/log/OfflineScopesModel.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/app/LusanApplication.hpp"

#include <QFileDialog>
#include <QToolButton>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMenu>
#include <QMessageBox>

NaviOfflineLogsScopes::NaviOfflineLogsScopes(MdiMainWindow* wndMain, QWidget* parent)
    : NaviLogScopeBase  (static_cast<int>(NavigationDock::eNaviWindow::NaviOfflineLogs), wndMain, parent)
    , ui                (new Ui::NaviOfflineLogsScopes)
    , mMenuActions      (static_cast<int>(eLogActions::PrioCount))
{
    ui->setupUi(this);
    ctrlCollapse()->setStyleSheet(NELusanCommon::getStyleToolbutton());
    setBaseSize(NELusanCommon::MIN_NAVI_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    setMinimumSize(NELusanCommon::MIN_NAVI_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);

    setupWidgets();

    setupModel(new OfflineScopesModel(this));
    setupControls(ctrlTable(), ctrlLogError(), ctrlLogWarning(), ctrlLogInfo(), ctrlLogDebug(), ctrlLogScopes());
    setupSignals();

    updateControls();
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
        setLoggingModel(logModel);
        return true;
    }
    else
    {
        setLoggingModel(nullptr);
        QMessageBox::warning(this, tr("Database Error"), tr("Failed to open database file:\n%1").arg(filePath));
        return false;
    }
}

void NaviOfflineLogsScopes::closeDatabase(void)
{
    // Clear the tree view
    setLoggingModel(nullptr);
}

bool NaviOfflineLogsScopes::isDatabaseOpen(void) const
{
    LoggingModelBase* logModel{ getLoggingModel() };
    return (logModel != nullptr) && logModel->isOperable();
}

void NaviOfflineLogsScopes::setLoggingModel(LoggingModelBase * model)
{
    ctrlLogDebug()->setChecked(model != nullptr);
    ctrlLogError()->setChecked(model != nullptr);
    ctrlLogInfo()->setChecked(model != nullptr);
    ctrlLogScopes()->setChecked(model != nullptr);
    ctrlLogWarning()->setChecked(model != nullptr);

    NaviLogScopeBase::setLoggingModel(model);
    updateControls();
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

QToolButton* NaviOfflineLogsScopes::ctrlCollapse(void) const
{
    return ui->toolCollapse;
}

QToolButton* NaviOfflineLogsScopes::ctrlOpenDatabase(void) const
{
    return ui->toolDbOpen;
}

QToolButton* NaviOfflineLogsScopes::ctrlCloseDatabase(void) const
{
    return ui->toolDbClose;
}

QToolButton* NaviOfflineLogsScopes::ctrlRefreshDatabase(void) const
{
    return ui->toolRefresh;
}

QTreeView* NaviOfflineLogsScopes::ctrlTable(void) const
{
    return ui->treeView;
}

QToolButton* NaviOfflineLogsScopes::ctrlFind(void) const
{
    return ui->toolFind;
}

QToolButton* NaviOfflineLogsScopes::ctrlLogError(void) const
{
    return ui->toolError;
}

QToolButton* NaviOfflineLogsScopes::ctrlLogWarning(void) const
{
    return ui->toolWarning;
}

QToolButton* NaviOfflineLogsScopes::ctrlLogInfo(void) const
{
    return ui->toolInformation;
}

QToolButton* NaviOfflineLogsScopes::ctrlLogDebug(void) const
{
    return ui->toolDebug;
}

QToolButton* NaviOfflineLogsScopes::ctrlLogScopes(void) const
{
    return ui->toolScopes;
}

QToolButton* NaviOfflineLogsScopes::ctrlMoveTop(void) const
{
    return ui->toolMoveTop;
}

QToolButton* NaviOfflineLogsScopes::ctrlMoveBottom(void) const
{
    return ui->toolMoveBottom;
}

void NaviOfflineLogsScopes::setupWidgets(void)
{
    // Configure the tree view for database information display
    // ctrlTable()->setHeaderHidden(false);
    // ctrlTable()->setRootIsDecorated(true);
    ctrlTable()->setContextMenuPolicy(Qt::CustomContextMenu);
    ctrlTable()->setAlternatingRowColors(false);
}

void NaviOfflineLogsScopes::setupSignals(void)
{
    // Connect tool button signals
    connect(ctrlOpenDatabase()      , &QToolButton::clicked, this, &NaviOfflineLogsScopes::onOpenDatabaseClicked);
    connect(ctrlCloseDatabase()     , &QToolButton::clicked, this, &NaviOfflineLogsScopes::onCloseDatabaseClicked);
    connect(ctrlRefreshDatabase()   , &QToolButton::clicked, this, &NaviOfflineLogsScopes::onRefreshDatabaseClicked);
    connect(mScopesModel            , &OfflineScopesModel::signalRootUpdated    , this, &NaviOfflineLogsScopes::onRootUpdated);
    connect(mScopesModel            , &OfflineScopesModel::signalScopesInserted , this, &NaviOfflineLogsScopes::onScopesInserted);
    connect(ctrlTable()             , &QWidget::customContextMenuRequested      , this, &NaviOfflineLogsScopes::onTreeViewContextMenuRequested);
    connect(ctrlCollapse()          , &QToolButton::clicked                     , this, [this](bool checked){onCollapseClicked(checked, ctrlCollapse());});
    connect(mMainWindow             , &MdiMainWindow::signalOpenOfflineLog      , this, [this](){onOpenDatabaseClicked();});
}

void NaviOfflineLogsScopes::updateControls(void)
{
    bool dbOpen = isDatabaseOpen();
    
    ctrlCloseDatabase()->setEnabled(dbOpen);
    ctrlRefreshDatabase()->setEnabled(dbOpen);
    restoreView();
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

void NaviOfflineLogsScopes::restoreView(void)
{
    Q_ASSERT(mScopesModel != nullptr);
    LoggingModelBase* logModel{ mScopesModel->getLoggingModel() };
    if (logModel != nullptr)
    {
        const LoggingModelBase::RootList& roots{ logModel->getRootList() };
        const QModelIndex& idxRoot{ mScopesModel->getRootIndex() };
        int rootCount{static_cast<int>(roots.size())};
        QTreeView* navi = ctrlTable();
        for (int row = 0; row < rootCount; ++row)
        {
            const ScopeRoot* root{roots[row]};
            if (root->isNodeExpanded())
            {
                QModelIndex idxNode{ mScopesModel->index(row, 0, idxRoot) };
                navi->expand(idxNode);
                expandChildNodesRecursive(idxNode, *root);
            }
        }
        
        const QModelIndex& idxSelected = logModel->getSelectedScope();
        enableButtons(idxSelected);
        if (idxSelected.isValid())
        {
            navi->selectionModel()->setCurrentIndex(idxSelected, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            navi->selectionModel()->select(idxSelected, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            navi->scrollTo(idxSelected);
        }
        else
        {
            navi->setCurrentIndex(logModel->getSelectedScope());
            navi->scrollToTop();
        }
    }
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
        setLoggingModel(logModel);
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
    enableButtons(root);
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

void NaviOfflineLogsScopes::onTreeViewContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = ctrlTable()->indexAt(pos);
    if (index.isValid() == false)
        return;

    Q_ASSERT(mScopesModel != nullptr);

    // Get current priority of the selected node
    ScopeNodeBase* node = mScopesModel->data(index, Qt::UserRole).value<ScopeNodeBase*>();
    if ((node == nullptr) || (node->hasPrioValid() == false))
        return;

    QMenu menu(this);
    bool hasNotset = node->hasPrioNotset();
    bool hasScope{ false }, hasDebug{ false }, hasInfo{ false }, hasWarn{ false }, hasError{ false }, hasFatal{ false };
    if (hasNotset == false)
    {
        hasScope = node->hasLogScopes();
        hasDebug = node->hasPrioDebug();
        hasInfo = node->hasPrioInfo();
        hasWarn = node->hasPrioWarning();
        hasError = node->hasPrioError();
        hasFatal = node->hasPrioFatal();
    }

    mMenuActions[static_cast<int>(eLogActions::PrioNotset)] = menu.addAction(tr("&Reset Priorities"));
    mMenuActions[static_cast<int>(eLogActions::PrioNotset)]->setEnabled(true);
    mMenuActions[static_cast<int>(eLogActions::PrioNotset)]->setCheckable(false);

    mMenuActions[static_cast<int>(eLogActions::PrioAllset)] = menu.addAction(tr("&Select All Priorities"));
    mMenuActions[static_cast<int>(eLogActions::PrioAllset)]->setEnabled(true);
    mMenuActions[static_cast<int>(eLogActions::PrioAllset)]->setCheckable(false);

    mMenuActions[static_cast<int>(eLogActions::PrioDebug)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioDebug, hasDebug), hasDebug ? tr("Hide &Debug messages") : tr("Show &Debug messages"));
    mMenuActions[static_cast<int>(eLogActions::PrioDebug)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioDebug)]->setChecked(hasDebug);

    mMenuActions[static_cast<int>(eLogActions::PrioInfo)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioInfo, hasInfo), hasInfo ? tr("Hide &Info messages") : tr("Show &Info messages"));
    mMenuActions[static_cast<int>(eLogActions::PrioInfo)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioInfo)]->setChecked(hasInfo);

    mMenuActions[static_cast<int>(eLogActions::PrioWarn)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioWarn, hasWarn), hasWarn ? tr("Hide &Warning messages") : tr("Show &Warning messages"));
    mMenuActions[static_cast<int>(eLogActions::PrioWarn)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioWarn)]->setChecked(hasWarn);

    mMenuActions[static_cast<int>(eLogActions::PrioError)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioError, hasError), hasError ? tr("Hide &Error messages") : tr("Show &Error messages"));
    mMenuActions[static_cast<int>(eLogActions::PrioError)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioError)]->setChecked(hasError);

    mMenuActions[static_cast<int>(eLogActions::PrioFatal)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioFatal, hasFatal), hasFatal ? tr("Hide &Fatal messages") : tr("Show &Fatal messages"));
    mMenuActions[static_cast<int>(eLogActions::PrioFatal)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioFatal)]->setChecked(hasFatal);

    mMenuActions[static_cast<int>(eLogActions::PrioScope)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioScope, hasScope), hasScope ? tr("Hide &Scopes") : tr("Show &Scopes"));
    mMenuActions[static_cast<int>(eLogActions::PrioScope)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioScope)]->setChecked(hasScope);

    mMenuActions[static_cast<int>(eLogActions::ExpandSelected)] = menu.addAction(NELusanCommon::iconNodeExpanded(NELusanCommon::SizeBig), tr("Expand Selected"));
    mMenuActions[static_cast<int>(eLogActions::ExpandSelected)]->setEnabled((ctrlTable()->isExpanded(index) == false) && (node->hasChildren()));
    mMenuActions[static_cast<int>(eLogActions::ExpandSelected)]->setCheckable(false);

    mMenuActions[static_cast<int>(eLogActions::CollapseSelected)] = menu.addAction(NELusanCommon::iconNodeCollapsed(NELusanCommon::SizeBig), tr("Collapse Selected"));
    mMenuActions[static_cast<int>(eLogActions::CollapseSelected)]->setEnabled(ctrlTable()->isExpanded(index) && node->hasChildren());
    mMenuActions[static_cast<int>(eLogActions::CollapseSelected)]->setCheckable(false);

    mMenuActions[static_cast<int>(eLogActions::ExpandAll)] = menu.addAction(tr("Expand All"));
    mMenuActions[static_cast<int>(eLogActions::ExpandAll)]->setEnabled(true);
    mMenuActions[static_cast<int>(eLogActions::ExpandAll)]->setCheckable(false);

    mMenuActions[static_cast<int>(eLogActions::CollapseAll)] = menu.addAction(tr("Collapse All"));
    mMenuActions[static_cast<int>(eLogActions::CollapseAll)]->setEnabled(areRootsCollapsed() == false);
    mMenuActions[static_cast<int>(eLogActions::CollapseAll)]->setCheckable(false);

    QAction* selectedAction = menu.exec(ctrlTable()->viewport()->mapToGlobal(pos));
    if (nullptr == selectedAction)
        return;

    bool processed{true};
    if (selectedAction == mMenuActions[static_cast<int>(eLogActions::PrioNotset)])
    {
        mScopesModel->setLogPriority(index, static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset));
    }
    else if (selectedAction == mMenuActions[eLogActions::PrioAllset])
    {
        mScopesModel->setLogPriority(index, static_cast<uint32_t>(NELogging::eLogPriority::PrioScopeLogs));
    }
    else if (selectedAction == mMenuActions[eLogActions::PrioDebug])
    {
        updatePriority(index, selectedAction->isChecked(), NELogging::eLogPriority::PrioDebug);
    }
    else if (selectedAction == mMenuActions[eLogActions::PrioInfo])
    {
        updatePriority(index, selectedAction->isChecked(), NELogging::eLogPriority::PrioInfo);
    }
    else if (selectedAction == mMenuActions[eLogActions::PrioWarn])
    {
        updatePriority(index, selectedAction->isChecked(), NELogging::eLogPriority::PrioWarning);
    }
    else if (selectedAction == mMenuActions[eLogActions::PrioError])
    {
        updatePriority(index, selectedAction->isChecked(), NELogging::eLogPriority::PrioError);
    }
    else if (selectedAction == mMenuActions[eLogActions::PrioFatal])
    {
        updatePriority(index, selectedAction->isChecked(), NELogging::eLogPriority::PrioFatal);
    }
    else if (selectedAction == mMenuActions[eLogActions::PrioScope])
    {
        updatePriority(index, selectedAction->isChecked(), NELogging::eLogPriority::PrioScope);
    }
    else if (selectedAction == mMenuActions[eLogActions::ExpandSelected])
    {
        ctrlTable()->expand(index);
        mScopesModel->nodeExpanded(index);
    }
    else if (selectedAction == mMenuActions[eLogActions::CollapseSelected])
    {
        ctrlTable()->collapse(index);
        mScopesModel->nodeCollapsed(index);
    }
    else if (selectedAction == mMenuActions[eLogActions::ExpandAll])
    {
        onCollapseClicked(true, ctrlCollapse());
    }
    else if (selectedAction == mMenuActions[eLogActions::CollapseAll])
    {
        onCollapseClicked(false, ctrlCollapse());
    }
    else
    {
        processed = false;
    }

    if (processed)
    {
        enableButtons(index);
        mScopesModel->nodeSelected(index);
    }
}
