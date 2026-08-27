/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/common/NaviOfflineLogsScopes.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the offline log explorer.
 *
 ************************************************************************/

#include "lusan/view/common/NaviOfflineLogsScopes.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/model/log/OfflineScopesModel.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/log/LogPriorityBar.hpp"

#include <QIcon>
#include <QMessageBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QToolButton>
#include <QTreeView>

NaviOfflineLogsScopes::NaviOfflineLogsScopes(MdiMainWindow* wndMain, QWidget* parent)
    : NaviLogScopeBase  (static_cast<int>(NavigationDock::eNaviWindow::NaviOfflineLogs), wndMain, parent)

    , mToolDbOpen       (nullptr)
    , mToolDbClose      (nullptr)
    , mToolRefresh      (nullptr)
    , mToolMoveBottom   (nullptr)
    , mToolMoveTop      (nullptr)
{
    setupScopeToolbar();
    setBaseSize(NELusanCommon::MIN_NAVI_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    setMinimumSize(NELusanCommon::MIN_NAVI_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);

    setupModel(new OfflineScopesModel(this));
    setupScopeControls();
    setupSignals();

    updateControls();
}

NaviOfflineLogsScopes::~NaviOfflineLogsScopes()
{
    ctrlTable()->setModel(nullptr);
    delete mScopesModel;
}

void NaviOfflineLogsScopes::addSpecificTools(void)
{
    mToolDbOpen = addToolButton( NELusanCommon::iconOpenFile(NELusanCommon::SizeBig)
                               , tr("Open log file")
                               , tr("Opens log file"));

    mToolDbClose = addToolButton( NELusanCommon::iconClose(NELusanCommon::SizeBig)
                                , tr("Close log file")
                                , tr("Close log file"));

    mToolRefresh = addToolButton( NELusanCommon::iconRefresh(NELusanCommon::SizeBig)
                                , tr("Refresh and reset filters")
                                , tr("Reloads the scopes and clears every filter."));
}

void NaviOfflineLogsScopes::addMoveTools(void)
{
    mToolMoveBottom = addToolButton( NELusanCommon::iconScrollBottom(NELusanCommon::SizeBig)
                                   , tr("Move at bottom of logs")
                                   , tr("Move at bottom of logs"));
    mToolMoveBottom->setArrowType(Qt::ArrowType::DownArrow);
    mToolMoveBottom->setWhatsThis(tr("Click to move to bottom of the logs"));

    mToolMoveTop = addToolButton( NELusanCommon::iconScrollTop(NELusanCommon::SizeBig)
                                , tr("Move at top of logs")
                                , tr("Move at top of logs"));
    mToolMoveTop->setArrowType(Qt::ArrowType::UpArrow);
    mToolMoveTop->setWhatsThis(tr("Click to move to top of the logs"));
}

bool NaviOfflineLogsScopes::hasSelectAllPrioMenu(void) const
{
    return true;
}

QString NaviOfflineLogsScopes::getOpenedDatabasePath() const
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

void NaviOfflineLogsScopes::closeDatabase()
{
    setLoggingModel(nullptr);
}

bool NaviOfflineLogsScopes::isDatabaseOpen() const
{
    LoggingModelBase* logModel{ getLoggingModel() };
    return (logModel != nullptr) && logModel->isOperable();
}

void NaviOfflineLogsScopes::setLoggingModel(LoggingModelBase * model)
{
    ctrlPriorityBar()->setEnabled(model != nullptr);
    ctrlPriorityBar()->setIdle(model == nullptr);

    NaviLogScopeBase::setLoggingModel(model);
    updateControls();
}

void NaviOfflineLogsScopes::optionOpenning()
{
}

void NaviOfflineLogsScopes::optionApplied()
{
}

void NaviOfflineLogsScopes::optionClosed(bool OKpressed)
{
    Q_UNUSED(OKpressed);
}

void NaviOfflineLogsScopes::setupSignals()
{
    connect(ctrlOpenDatabase()      , &QToolButton::clicked, this, &NaviOfflineLogsScopes::onOpenDatabaseClicked);
    connect(ctrlCloseDatabase()     , &QToolButton::clicked, this, &NaviOfflineLogsScopes::onCloseDatabaseClicked);
    connect(ctrlRefreshDatabase()   , &QToolButton::clicked, this, &NaviOfflineLogsScopes::onRefreshDatabaseClicked);
    connect(mScopesModel            , &OfflineScopesModel::signalRootUpdated    , this, &NaviOfflineLogsScopes::onRootUpdated);
    connect(mScopesModel            , &OfflineScopesModel::signalScopesInserted , this, &NaviOfflineLogsScopes::onScopesInserted);
    connect(mMainWindow             , &MdiMainWindow::signalOpenOfflineLog      , this, [this](){onOpenDatabaseClicked();});
}

void NaviOfflineLogsScopes::updateControls()
{
    bool dbOpen = isDatabaseOpen();

    ctrlCloseDatabase()->setEnabled(dbOpen);
    ctrlRefreshDatabase()->setEnabled(dbOpen);
    restoreView();
}

void NaviOfflineLogsScopes::showDatabaseInfo()
{
    Q_ASSERT(false);
    LoggingModelBase* logModel{ getLoggingModel() };

    if ((logModel == nullptr) || (isDatabaseOpen() == false))
    {
        ctrlTable()->setModel(nullptr);
        return;
    }

    QStandardItemModel* infoModel = new QStandardItemModel(this);
    infoModel->setHorizontalHeaderLabels(QStringList() << tr("Database Information"));

    QStandardItem* dbPathItem = new QStandardItem(tr("Database File"));
    dbPathItem->appendRow(new QStandardItem(logModel->getLogFileName()));
    infoModel->appendRow(dbPathItem);

    QStandardItem* statusItem = new QStandardItem(tr("Status"));
    statusItem->appendRow(new QStandardItem(tr("Connected")));
    infoModel->appendRow(statusItem);

    try
    {
        std::vector<areg::String> instanceNames;
        logModel->getLogInstanceNames(instanceNames);

        QStandardItem* instancesItem = new QStandardItem(tr("Instances (%1)").arg(instanceNames.size()));
        for (const auto& name : instanceNames)
        {
            instancesItem->appendRow(new QStandardItem(QString::fromStdString(name.data())));
        }

        infoModel->appendRow(instancesItem);

        std::vector<areg::String> threadNames;
        logModel->getLogThreadNames(threadNames);

        QStandardItem* threadsItem = new QStandardItem(tr("Threads (%1)").arg(threadNames.size()));
        for (const auto& name : threadNames)
        {
            threadsItem->appendRow(new QStandardItem(QString::fromStdString(name.data())));
        }

        infoModel->appendRow(threadsItem);
    }
    catch (...)
    {
        QStandardItem* errorItem = new QStandardItem(tr("Error"));
        errorItem->appendRow(new QStandardItem(tr("Could not retrieve database information")));
        infoModel->appendRow(errorItem);
    }

    ctrlTable()->setModel(infoModel);
    ctrlTable()->expandAll();
}

void NaviOfflineLogsScopes::restoreView()
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

void NaviOfflineLogsScopes::onOpenDatabaseClicked()
{
    QString filePath = mMainWindow->openLogFile();
    if (filePath.isEmpty() == false)
    {
        openDatabase(filePath);
    }
}

void NaviOfflineLogsScopes::onCloseDatabaseClicked()
{
    closeDatabase();
}

void NaviOfflineLogsScopes::onRefreshDatabaseClicked()
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
    enableButtons(root);
    expandNodeAndChildren(root, false);
}

void NaviOfflineLogsScopes::onScopesInserted(const QModelIndex& parent)
{
    Q_ASSERT(mScopesModel != nullptr);
    expandNode(parent, false);
}
