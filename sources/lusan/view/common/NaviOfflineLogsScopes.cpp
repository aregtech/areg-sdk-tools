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
    : NavigationWindow(static_cast<int>(NavigationDock::eNaviWindow::NaviOfflineLogs), wndMain, parent)

    , ui            (new Ui::NaviOfflineLogsScopes)
    , mScopesModel  (new OfflineScopesModel(this))
    , mLogPrio      ( 0u )

{
    ui->setupUi(this);
    ctrlCollapse()->setStyleSheet(NELusanCommon::getStyleToolbutton());
    setBaseSize(NELusanCommon::MIN_NAVI_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    setMinimumSize(NELusanCommon::MIN_NAVI_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);

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
        mLogPrio = static_cast<uint32_t>(NELogging::eLogPriority::PrioScopeLogs);
        return true;
    }
    else
    {
        mLogPrio = 0u;
        mScopesModel->setLoggingModel(nullptr);
        QMessageBox::warning(this, tr("Database Error"), tr("Failed to open database file:\n%1").arg(filePath));
        return false;
    }
}

void NaviOfflineLogsScopes::closeDatabase(void)
{
    // Clear the tree view
    mScopesModel->setLoggingModel(nullptr);
    mLogPrio = 0u;
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
    connect(mMainWindow             , &MdiMainWindow::signalOpenOfflineLog      , this, [this](){onOpenDatabaseClicked();});
    connect(ctrlLogError()          , &QToolButton::clicked, this, [this](bool checked){onLogPrioSelected(checked, NELogging::eLogPriority::PrioError);});
    connect(ctrlLogWarning()        , &QToolButton::clicked, this, [this](bool checked){onLogPrioSelected(checked, NELogging::eLogPriority::PrioWarning);});
    connect(ctrlLogInfo()           , &QToolButton::clicked, this, [this](bool checked){onLogPrioSelected(checked, NELogging::eLogPriority::PrioInfo);});
    connect(ctrlLogDebug()          , &QToolButton::clicked, this, [this](bool checked){onLogPrioSelected(checked, NELogging::eLogPriority::PrioDebug);});
    connect(ctrlLogScopes()         , &QToolButton::clicked, this, [this](bool checked){onLogPrioSelected(checked, NELogging::eLogPriority::PrioScope);});
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

void NaviOfflineLogsScopes::expandChildNodesRecursive(const QModelIndex& idxNode, const ScopeNodeBase& node)
{
    if (node.isLeaf() || (idxNode.isValid() == false))
        return; // No children to expand

    QTreeView* navi = ctrlTable();
    int rowCount{node.getChildNodesCount()};
    for (int row = 0; row < rowCount; ++row)
    {
        const ScopeNodeBase* child = node.getChildAt(row);
        Q_ASSERT(child != nullptr);
        if (child->isNodeExpanded())
        {
            QModelIndex idxChild{ mScopesModel->index(row, 0, idxNode) };
            Q_ASSERT(idxChild.isValid());
            navi->expand(idxChild);
            if (child->isNode())
            {
                expandChildNodesRecursive(idxChild, *child);
            }
        }
    }

    enableButtons(idxNode);
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

uint32_t NaviOfflineLogsScopes::getSelectedPrios(void) const
{
    Q_ASSERT(ctrlLogScopes()  != nullptr);
    Q_ASSERT(ctrlLogDebug()   != nullptr);
    Q_ASSERT(ctrlLogInfo()    != nullptr);
    Q_ASSERT(ctrlLogWarning() != nullptr);
    Q_ASSERT(ctrlLogError()   != nullptr);
    
    uint32_t result {static_cast<uint32_t>(ctrlLogScopes()->isChecked() ? NELogging::eLogPriority::PrioScope : NELogging::eLogPriority::PrioInvalid)};
    if (ctrlLogDebug()->isChecked())
    {
        result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug);
    }
    else if (ctrlLogInfo()->isChecked())
    {
        result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo);
    }
    else if (ctrlLogWarning()->isChecked())
    {
        result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);
    }
    else if (ctrlLogError()->isChecked())
    {
        result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
    }
    else if (result == static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid))
    {
        result = static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset);
    }
    
    return result;
}

void NaviOfflineLogsScopes::updatePriority(const QModelIndex& node)
{
    Q_ASSERT(mScopesModel != nullptr);
    mScopesModel->setLogPriority(node, mLogPrio);
}

void NaviOfflineLogsScopes::updateColors(bool errSelected, bool warnSelected, bool infoSelected, bool dbgSelected, bool scopeSelected)
{
    ctrlLogDebug()->setIcon(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioDebug, dbgSelected));
    ctrlLogInfo()->setIcon(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioInfo, infoSelected));
    ctrlLogWarning()->setIcon(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioWarn, warnSelected));
    ctrlLogError()->setIcon(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioError, errSelected));
    ctrlLogScopes()->setIcon(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioScope, scopeSelected));
    
    ctrlLogError()->update();
    ctrlLogWarning()->update();
    ctrlLogInfo()->update();
    ctrlLogDebug()->update();
    ctrlLogScopes()->update();
}

void NaviOfflineLogsScopes::enableButtons(const QModelIndex& selection)
{
    ScopeNodeBase* node = selection.isValid() ? mScopesModel->data(selection, Qt::ItemDataRole::UserRole).value<ScopeNodeBase*>() : nullptr;
    if (node != nullptr)
    {
        bool errSelected{ false }, warnSelected{ false }, infoSelected{ false }, dbgSelected{ false }, scopeSelected{ false };

        ctrlLogError()->setEnabled(true);
        ctrlLogWarning()->setEnabled(true);
        ctrlLogInfo()->setEnabled(true);
        ctrlLogDebug()->setEnabled(true);
        ctrlLogScopes()->setEnabled(true);

        ctrlLogError()->setChecked(false);
        ctrlLogWarning()->setChecked(false);
        ctrlLogInfo()->setChecked(false);
        ctrlLogDebug()->setChecked(false);
        ctrlLogScopes()->setChecked(false);

        if (node->isValid() && (node->hasPrioNotset() == false))
        {
            if (node->hasPrioDebug())
            {
                ctrlLogDebug()->setChecked(true);
                dbgSelected = true;
            }

            if (node->hasPrioInfo())
            {
                ctrlLogInfo()->setChecked(true);
                infoSelected = true;
            }

            if (node->hasPrioWarning())
            {
                ctrlLogWarning()->setChecked(true);
                warnSelected = true;
            }

            if (node->hasPrioError() || node->hasPrioFatal())
            {
                ctrlLogError()->setChecked(true);
                errSelected = true;
            }

            if (node->hasLogScopes())
            {
                ctrlLogScopes()->setChecked(true);
                scopeSelected = true;
            }
        }

        updateColors(errSelected, warnSelected, infoSelected, dbgSelected, scopeSelected);
    }
    else
    {
        ctrlLogError()->setEnabled(false);
        ctrlLogWarning()->setEnabled(false);
        ctrlLogInfo()->setEnabled(false);
        ctrlLogDebug()->setEnabled(false);
        ctrlLogScopes()->setEnabled(false);
    }
}

void NaviOfflineLogsScopes::onLogPrioSelected(bool isChecked, NELogging::eLogPriority logPrio)
{
    if (isChecked)
    {
        if (logPrio == NELogging::eLogPriority::PrioScope)
        {
            mLogPrio |= static_cast<uint32_t>(NELogging::eLogPriority::PrioScope);
        }
        else
        {
            mLogPrio &= static_cast<uint32_t>(NELogging::eLogPriority::PrioScope);
            switch (logPrio)
            {
            case NELogging::eLogPriority::PrioDebug:
                mLogPrio |= static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug);
                break;

            case NELogging::eLogPriority::PrioInfo:
                mLogPrio |= static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo);
                break;

            case NELogging::eLogPriority::PrioWarning:
                mLogPrio |= static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning);
                break;

            case NELogging::eLogPriority::PrioError:
                mLogPrio |= static_cast<uint32_t>(NELogging::eLogPriority::PrioError);
                break;

            default:
                break; // ignore
            }
        }
    }
    else
    {
        mLogPrio &= ~static_cast<uint32_t>(logPrio);
    }

    mLogPrio = mLogPrio == static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid) ? static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset) : mLogPrio;
    updatePriority(ctrlTable()->currentIndex());
}

