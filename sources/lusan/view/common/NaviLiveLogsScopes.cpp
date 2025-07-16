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
 *  \file        lusan/view/common/NaviLiveLogsScopes.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the log explorer.
 *
 ************************************************************************/

#include "lusan/view/common/NaviLiveLogsScopes.hpp"
#include "ui/ui_NaviLiveLogsScopes.h"

#include "lusan/app/LusanApplication.hpp"

#include "lusan/data/log/LogObserver.hpp"
#include "lusan/data/log/ScopeNodeBase.hpp"

#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"
#include "lusan/model/log/LiveScopesModel.hpp"

#include "lusan/view/common/MdiChild.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/log/LiveLogViewer.hpp"

#include "areg/base/NESocket.hpp"

#include <QAction>
#include <QMenu>
#include <filesystem>

NaviLiveLogsScopes* _explorer{ nullptr };
void NaviLiveLogsScopes::_logObserverStarted(void)
{
    if (_explorer != nullptr)
    {
        _explorer->setupLogSignals(true);
    }
}

NaviLiveLogsScopes::NaviLiveLogsScopes(MdiMainWindow* wndMain, QWidget* parent)
    : NavigationWindow(static_cast<int>(Navigation::eNaviWindow::NaviLiveLogs), wndMain, parent)

    , ui            (new Ui::NaviLiveLogsScopes)
    , mAddress      ()
    , mPort         (NESocket::InvalidPort)
    , mInitLogFile  ( )
    , mActiveLogFile( )
    , mLogLocation  ( )
    , mScopesModel  (new LiveScopesModel(this))
    , mSelModel     (new QItemSelectionModel(mScopesModel, this))
    , mSignalsActive(false)
    , mState        (eLoggingStates::LoggingUndefined)
    , mMenuActions  (static_cast<int>(eLogActions::PrioCount))
{
    _explorer = this;
    
    ui->setupUi(this);
    this->setBaseSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    this->setMinimumSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    this->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    updateData();
    setupWidgets();
    setupSignals();
}

NaviLiveLogsScopes::~NaviLiveLogsScopes(void)
{
    _explorer = nullptr;
    delete ui;
}

const QString& NaviLiveLogsScopes::getLogCollectorAddress(void) const
{
    return mAddress;
}

void NaviLiveLogsScopes::setLogCollectorAddress(const QString& address)
{
    mAddress = address;
}

uint16_t NaviLiveLogsScopes::getLogCollectorPort(void) const
{
    return mPort;
}

void NaviLiveLogsScopes::setLogCollectorPort(uint16_t port)
{
    mPort = port;
}

void NaviLiveLogsScopes::setLogCollectorConnection(const QString& address, uint16_t port)
{
    mAddress = address;
    mPort = port;
}

void NaviLiveLogsScopes::setLoggingModel(LiveLogsModel* logModel)
{
    mScopesModel->setLoggingModel(logModel);
}

LiveLogsModel* NaviLiveLogsScopes::getLoggingModel(void)
{
    return static_cast<LiveLogsModel *>(mScopesModel->getLoggingModel());
}

QToolButton* NaviLiveLogsScopes::ctrlCollapse(void)
{
    return ui->toolCollapse;
}

QToolButton* NaviLiveLogsScopes::ctrlConnect(void)
{
    return ui->toolConnect;
}

QToolButton* NaviLiveLogsScopes::ctrlSettings(void)
{
    return ui->toolSettings;
}

QToolButton* NaviLiveLogsScopes::ctrlSaveSettings(void)
{
    return ui->toolSaveSettings;
}

QToolButton* NaviLiveLogsScopes::ctrlFind(void)
{
    return ui->toolFind;
}

QToolButton* NaviLiveLogsScopes::ctrlLogError(void)
{
    return ui->toolError;
}

QToolButton* NaviLiveLogsScopes::ctrlLogWarning(void)
{
    return ui->toolWarning;
}

QToolButton* NaviLiveLogsScopes::ctrlLogInfo(void)
{
    return ui->toolInformation;
}

QToolButton* NaviLiveLogsScopes::ctrlLogDebug(void)
{
    return ui->toolDebug;
}

QToolButton* NaviLiveLogsScopes::ctrlLogScopes(void)
{
    return ui->toolScopes;
}

QToolButton* NaviLiveLogsScopes::ctrlMoveBottom(void)
{
    return ui->toolMoveBottom;
}

QTreeView* NaviLiveLogsScopes::ctrlTable(void)
{
    return ui->treeView;
}

void NaviLiveLogsScopes::updateData(void)
{
}

void NaviLiveLogsScopes::setupWidgets(void)
{
    ctrlCollapse()->setEnabled(true);
    ctrlConnect()->setEnabled(true);
    ctrlSettings()->setEnabled(true);
    ctrlSaveSettings()->setEnabled(true);
    ctrlFind()->setEnabled(false);
    ctrlLogError()->setEnabled(false);
    ctrlLogWarning()->setEnabled(false);
    ctrlLogInfo()->setEnabled(false);
    ctrlLogDebug()->setEnabled(false);
    ctrlLogScopes()->setEnabled(false);
    ctrlTable()->setModel(mScopesModel);
    ctrlTable()->setSelectionModel(mSelModel);
    ctrlTable()->setContextMenuPolicy(Qt::CustomContextMenu);
}

void NaviLiveLogsScopes::setupSignals(void)
{
    connect(ctrlConnect()       , &QToolButton::clicked, this, &NaviLiveLogsScopes::onConnectClicked);
    connect(ctrlMoveBottom()    , &QToolButton::clicked, this, &NaviLiveLogsScopes::onMoveBottomClicked);
    connect(ctrlLogError()      , &QToolButton::clicked, this, &NaviLiveLogsScopes::onPrioErrorClicked);
    connect(ctrlLogWarning()    , &QToolButton::clicked, this, &NaviLiveLogsScopes::onPrioWarningClicked);
    connect(ctrlLogInfo()       , &QToolButton::clicked, this, &NaviLiveLogsScopes::onPrioInfoClicked);
    connect(ctrlLogDebug()      , &QToolButton::clicked, this, &NaviLiveLogsScopes::onPrioDebugClicked);
    connect(ctrlLogScopes()     , &QToolButton::clicked, this, &NaviLiveLogsScopes::onPrioScopesClicked);
    connect(ctrlSaveSettings()  , &QToolButton::clicked, this, &NaviLiveLogsScopes::onSaveSettingsClicked);
    connect(ctrlSettings()      , &QToolButton::clicked, this, &NaviLiveLogsScopes::onOptionsClicked);
    connect(ctrlCollapse()      , &QToolButton::clicked, this, &NaviLiveLogsScopes::onCollapseClicked);
    connect(ctrlTable()         , &QTreeView::expanded , this, &NaviLiveLogsScopes::onNodeExpanded);
    connect(ctrlTable()         , &QTreeView::collapsed, this, &NaviLiveLogsScopes::onNodeCollapsed);
    connect(ctrlTable()         , &QTreeView::activated, this, &NaviLiveLogsScopes::onNodeActivated);
    connect(ctrlTable()         , &QWidget::customContextMenuRequested  , this  , &NaviLiveLogsScopes::onTreeViewContextMenuRequested);
    connect(mMainWindow         , &MdiMainWindow::signalMdiWindowCreated, this  , &NaviLiveLogsScopes::onWindowCreated);

    setupLogSignals(true);    
}

void NaviLiveLogsScopes::blockBasicSignals(bool block)
{
}

void NaviLiveLogsScopes::updateColors(bool errSelected, bool warnSelected, bool infoSelected, bool dbgSelected, bool scopeSelected)
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

void NaviLiveLogsScopes::updateExpanded(const QModelIndex& current)
{
    QTreeView* tree = current.isValid() && (mScopesModel != nullptr) ? ctrlTable() : nullptr;
    if (tree != nullptr)
    {
        tree->update(current);
        int count = tree->isExpanded(current) ? mScopesModel->rowCount(current) : 0;
        for (int i = 0; i < count; ++ i)
        {
            QModelIndex index = mScopesModel->index(i, 0, current);
            updateExpanded(index);
        }
    }
}

bool NaviLiveLogsScopes::updatePriority(const QModelIndex& node, bool addPrio, NELogging::eLogPriority prio)
{
    bool result{ false };
    if (node.isValid())
    {
        Q_ASSERT(mScopesModel != nullptr);
        if (addPrio)
        {
            result = mScopesModel->addLogPriority(node, prio);
        }
        else
        {
            result = mScopesModel->removeLogPriority(node, prio);
        }
    }
    
    return result;
}

void NaviLiveLogsScopes::setupLogSignals(bool setup)
{
    LogObserver* log = LogObserver::getComponent();
    if (log == nullptr)
    {
        mSignalsActive = false;
        return;
    }
    
    if (setup)
    {
        if (mSignalsActive == false)
        {
            mSignalsActive = true;
            connect(log, &LogObserver::signalLogObserverConfigured  , this, &NaviLiveLogsScopes::onLogObserverConfigured   , Qt::QueuedConnection);
            connect(log, &LogObserver::signalLogServiceConnected    , this, &NaviLiveLogsScopes::onLogServiceConnected     , Qt::QueuedConnection);
            connect(log, &LogObserver::signalLogDbConfigured        , this, &NaviLiveLogsScopes::onLogDbConfigured         , Qt::QueuedConnection);
            connect(log, &LogObserver::signalLogObserverStarted     , this, &NaviLiveLogsScopes::onLogObserverStarted      , Qt::QueuedConnection);
            connect(log, &LogObserver::signalLogDbCreated           , this, &NaviLiveLogsScopes::onLogDbCreated            , Qt::QueuedConnection);
            connect(log, &LogObserver::signalLogObserverInstance    , this, &NaviLiveLogsScopes::onLogObserverInstance     , Qt::QueuedConnection);

            Q_ASSERT(mScopesModel != nullptr);
            Q_ASSERT(mSelModel != nullptr);

            connect(mScopesModel, &LiveScopesModel::signalRootUpdated   , this, &NaviLiveLogsScopes::onRootUpdated);
            connect(mScopesModel, &LiveScopesModel::signalScopesInserted, this, &NaviLiveLogsScopes::onScopesInserted);
            connect(mScopesModel, &LiveScopesModel::dataChanged         , this, &NaviLiveLogsScopes::onScopesDataChanged);
            connect(mSelModel   , &QItemSelectionModel::selectionChanged, this, &NaviLiveLogsScopes::onSelectionChanged);
        }
    }
    else if (mSignalsActive)
    {
        Q_ASSERT(mScopesModel != nullptr);
        Q_ASSERT(mSelModel != nullptr);

        LoggingModelBase* logModel{ mScopesModel->getLoggingModel() };

        disconnect(mScopesModel  , &LiveScopesModel::signalRootUpdated   , this, &NaviLiveLogsScopes::onRootUpdated);
        disconnect(mScopesModel  , &LiveScopesModel::signalScopesInserted, this, &NaviLiveLogsScopes::onScopesInserted);
        disconnect(mScopesModel  , &LiveScopesModel::dataChanged         , this, &NaviLiveLogsScopes::onScopesDataChanged);
        disconnect(mSelModel    , &QItemSelectionModel::selectionChanged, this, &NaviLiveLogsScopes::onSelectionChanged);

        disconnect(log, &LogObserver::signalLogObserverConfigured   , this, &NaviLiveLogsScopes::onLogObserverConfigured);
        disconnect(log, &LogObserver::signalLogDbConfigured         , this, &NaviLiveLogsScopes::onLogDbConfigured);
        disconnect(log, &LogObserver::signalLogServiceConnected     , this, &NaviLiveLogsScopes::onLogServiceConnected);
        disconnect(log, &LogObserver::signalLogObserverStarted      , this, &NaviLiveLogsScopes::onLogObserverStarted);
        disconnect(log, &LogObserver::signalLogDbCreated            , this, &NaviLiveLogsScopes::onLogDbCreated);
        disconnect(log, &LogObserver::signalLogObserverInstance     , this, &NaviLiveLogsScopes::onLogObserverInstance);

        if (logModel != nullptr)
        {
            logModel->releaseModel();
            mScopesModel->setLoggingModel(nullptr);
            enableButtons(QModelIndex());
        }

        mSignalsActive =  false;
    }
}

bool NaviLiveLogsScopes::areRootsCollapsed(void) const
{
    bool result{ true };
    QTreeView* treeView = ui->treeView;

    int rowCount = mScopesModel != nullptr ? mScopesModel->rowCount(mScopesModel->getRootIndex()) : 0; // root items
    for (int row = 0; row < rowCount; ++row)
    {
        QModelIndex index = mScopesModel->index(row, 0, mScopesModel->getRootIndex());
        if (treeView->isExpanded(index))
        {
            result = false;
            break;
        }
    }

    return result;
}

void NaviLiveLogsScopes::collapseRoots(void)
{
    QTreeView* treeView = ctrlTable();
    int rowCount = mScopesModel != nullptr ? mScopesModel->rowCount(mScopesModel->getRootIndex()) : 0; // root items
    for (int row = 0; row < rowCount; ++row)
    {
        QModelIndex index = mScopesModel->index(row, 0, mScopesModel->getRootIndex());
        treeView->collapse(index);
    }
}

void NaviLiveLogsScopes::enableButtons(const QModelIndex & selection)
{
    ScopeNodeBase * node = selection.isValid() ? mScopesModel->data(selection, Qt::ItemDataRole::UserRole).value<ScopeNodeBase *>() : nullptr;
    if (node != nullptr)
    {
        bool errSelected{false}, warnSelected{false}, infoSelected{false}, dbgSelected{false}, scopeSelected{false};

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

void NaviLiveLogsScopes::onLogObserverConfigured(bool isEnabled, const QString& address, uint16_t port)
{
    ctrlConnect()->setEnabled(isEnabled);
    ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-offline")));
    ctrlConnect()->setToolTip(isEnabled ? tr("Connect to log collector") : tr("Logging is not enabled"));
    
    mAddress= address;
    mPort   = port;
    mState  = eLoggingStates::LoggingConfigured;
}

void NaviLiveLogsScopes::onLogDbConfigured(bool isEnabled, const QString& dbName, const QString& dbLocation, const QString& dbUser)
{
    mInitLogFile    = dbName;
    mLogLocation    = dbLocation;
}

void NaviLiveLogsScopes::onLogServiceConnected(bool isConnected, const QString& address, uint16_t port)
{
    if (isConnected)
    {
        mState = eLoggingStates::LoggingConnected;
    }

    enableButtons(QModelIndex());

    LogObserver* log = LogObserver::getComponent();
    ctrlConnect()->setChecked(isConnected);
    ctrlConnect()->setIcon(QIcon::fromTheme(isConnected ? QString::fromUtf8("network-wireless") : QString::fromUtf8("network-offline")));
    ctrlConnect()->setToolTip(isConnected ? address + ":" + QString::number(port) : tr("Connect to log collector"));
    Q_ASSERT(mMainWindow != nullptr);
    mMainWindow->logCollecttorConnected(isConnected, address, port, log != nullptr ? log->getActiveDatabase() : mActiveLogFile);
}

void NaviLiveLogsScopes::onLogObserverStarted(bool isStarted)
{
}

void NaviLiveLogsScopes::onLogDbCreated(const QString& dbLocation)
{
    mActiveLogFile = dbLocation;
    LogObserver* log = LogObserver::getComponent();
    if (log != nullptr)
    {
        mMainWindow->logDatabaseCreated(dbLocation);
    }
}

void NaviLiveLogsScopes::onLogObserverInstance(bool isStarted, const QString& address, uint16_t port, const QString& filePath)
{
    if (isStarted)
    {
        mScopesModel->setupModel();
        std::error_code err;
        std::filesystem::path dbPath(mLogLocation.toStdString());
        dbPath /= mInitLogFile.toStdString();
        QString logPath(std::filesystem::absolute(dbPath, err).c_str());
        LogObserver::connect(mAddress, mPort, logPath);
    }
    else
    {
        mScopesModel->releaseModel();
    }
    
    setupLogSignals(isStarted);
    enableButtons(QModelIndex());
}

void NaviLiveLogsScopes::onConnectClicked(bool checked)
{
    if (checked)
    {
        Q_ASSERT(mMainWindow != nullptr);
        LiveLogsModel* logModel{ mMainWindow->setupLiveLogging() };
        mScopesModel->setLoggingModel(logModel);
        LogObserver::createLogObserver(&NaviLiveLogsScopes::_logObserverStarted);
    }
    else
    {
        QString address{ LogObserver::getConnectedAddress() };
        uint16_t port{ LogObserver::getConnectedPort() };
        QString logFile{ mActiveLogFile.isEmpty() == false ? mActiveLogFile : LogObserver::getActiveDatabase() };

        setupLogSignals(false);
        
        ctrlConnect()->setChecked(false);
        ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-offline")));
        ctrlConnect()->setToolTip(tr("Connect to log collector"));
        
        mState = eLoggingStates::LoggingDisconnected;
        LogObserver::disconnect();
        LogObserver::releaseLogObserver();

        mMainWindow->logCollecttorConnected(false, address, port, logFile);
        mScopesModel->setLoggingModel(nullptr);
        mScopesModel->releaseModel();
    }

    enableButtons(QModelIndex());
}

void NaviLiveLogsScopes::onMoveBottomClicked()
{
    Q_ASSERT(mMainWindow != nullptr);
    MdiChild* wndActive = mMainWindow->getActiveWindow();
    if ((wndActive != nullptr) && wndActive->isLogViewerWindow())
    {
        qobject_cast<LiveLogViewer*>(wndActive)->moveToBottom(true);
    }
}

void NaviLiveLogsScopes::onPrioErrorClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    bool result = updatePriority(current, checked, NELogging::eLogPriority::PrioError);
    if (result == false)
    {
        ctrlLogError()->setChecked(!checked);
    }
}

void NaviLiveLogsScopes::onPrioWarningClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    bool result = updatePriority(current, checked, NELogging::eLogPriority::PrioWarning);
    if (result == false)
    {
        ctrlLogWarning()->setChecked(!checked);
    }
}

void NaviLiveLogsScopes::onPrioInfoClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    bool result = updatePriority(current, checked, NELogging::eLogPriority::PrioInfo);
    if (result == false)
    {
        ctrlLogInfo()->setChecked(!checked);
    }
}

void NaviLiveLogsScopes::onPrioDebugClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    bool result = updatePriority(current, checked, NELogging::eLogPriority::PrioDebug);
    if (result == false)
    {
        ctrlLogDebug()->setChecked(!checked);
    }
}

void NaviLiveLogsScopes::onPrioScopesClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    bool result = updatePriority(current, checked, NELogging::eLogPriority::PrioScope);
    if (result == false)
    {
        ctrlLogScopes()->setChecked(!checked);
    }
}

void NaviLiveLogsScopes::onSaveSettingsClicked(bool checked)
{
    if (mScopesModel != nullptr)
    {
        mScopesModel->saveLogScopePriority(QModelIndex());
    }
}

void NaviLiveLogsScopes::onOptionsClicked(bool checked)
{
    LogObserver* log = LogObserver::getComponent();
    
    QString address     {mAddress};
    QString hostName    {mAddress};
    uint16_t port       {mPort};
    QString logFile     {mInitLogFile};
    QString logLocation {mLogLocation};
    
    if (log != nullptr)
    {
        address     = log->getConnectedAddress();
        hostName    = log->getConnectedHostName();
        port        = log->getConnectedPort();
        logFile     = log->getConfigDatabaseName();
        logLocation = log->getConfigDatabaseLocation();
    }        
    
    mMainWindow->showOptionPageLogging(address, hostName, port, logFile, logLocation);
}

void NaviLiveLogsScopes::onCollapseClicked(bool checked)
{
    Q_ASSERT(mScopesModel != nullptr);
    if (mScopesModel->rowCount(mScopesModel->getRootIndex()) == 0)
    {
        ctrlCollapse()->blockSignals(true);
        ctrlCollapse()->setChecked(false);        
        ctrlCollapse()->blockSignals(false);
        return;
    }
    
    QTreeView * navi = ctrlTable();
    if (checked)
    {
        ctrlCollapse()->blockSignals(true);
        ctrlCollapse()->setIcon(QIcon::fromTheme(QString::fromUtf8("list-remove")));
        ctrlCollapse()->setChecked(true);
        
        navi->blockSignals(true);
        collapseRoots();
        navi->expand(mScopesModel->getRootIndex());
        navi->setCurrentIndex(mScopesModel->getRootIndex());
        navi->blockSignals(false);
        
        ctrlCollapse()->blockSignals(false);
    }
    else
    {
        ctrlCollapse()->blockSignals(true);
        ctrlCollapse()->setIcon(QIcon::fromTheme(QString::fromUtf8("list-add")));
        ctrlCollapse()->setChecked(false);
        
        navi->blockSignals(true);
        navi->expandAll();
        navi->setCurrentIndex(mScopesModel->getRootIndex());
        navi->blockSignals(false);
        
        ctrlCollapse()->blockSignals(false);
    }
}

void NaviLiveLogsScopes::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList list = selected.indexes();
    enableButtons(list.isEmpty() ? QModelIndex() : list.front());
}

void NaviLiveLogsScopes::onRootUpdated(const QModelIndex & root)
{
    Q_ASSERT(mScopesModel != nullptr);
    if (isConnected())
    {
        mState = eLoggingStates::LoggingRunning;
    }

    QTreeView * navi = ctrlTable();
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

void NaviLiveLogsScopes::onScopesInserted(const QModelIndex & parent)
{
    Q_ASSERT(mScopesModel != nullptr);
    if (parent.isValid())
    {
        enableButtons(parent);
        QTreeView * navi = ctrlTable();
        Q_ASSERT(navi != nullptr);
        if (navi->isExpanded(parent) == false)
        {
            navi->expand(parent);
        }
    }
}

void NaviLiveLogsScopes::onScopesUpdated(const QModelIndex & parent)
{
    if (parent.isValid())
    {
        enableButtons(parent);
        ctrlTable()->update(parent);
    }
}

void NaviLiveLogsScopes::onScopesDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles /*= QList<int>()*/)
{
    Q_ASSERT(mSelModel != nullptr);
    enableButtons(ctrlTable()->currentIndex());
    updateExpanded(ctrlTable()->rootIndex());
}

void NaviLiveLogsScopes::optionOpenning(void)
{
    if (isConnected())
    {
        setupLogSignals(false);
        mState = eLoggingStates::LoggingPaused;
        LogObserver::disconnect();
        LogObserver::releaseLogObserver();
    }
}

void NaviLiveLogsScopes::optionApplied(void)
{
    if (isPaused())
    {
        mState = eLoggingStates::LoggingStopped;
    }
}

void NaviLiveLogsScopes::optionClosed(bool OKpressed)
{
    if (isStopped() || isPaused())
    {
        LogObserver::createLogObserver(&NaviLiveLogsScopes::_logObserverStarted);
    }
    else if (mState != eLoggingStates::LoggingUndefined)
    {
        mState = eLoggingStates::LoggingConfigured;
    }
}

void NaviLiveLogsScopes::onTreeViewContextMenuRequested(const QPoint& pos)
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
    bool hasScope{false}, hasDebug{false}, hasInfo{false}, hasWarn{false}, hasError{false}, hasFatal{false};
    if (hasNotset == false)
    {
        hasScope = node->hasLogScopes();
        hasDebug = node->hasPrioDebug();
        hasInfo  = node->hasPrioInfo();
        hasWarn  = node->hasPrioWarning();
        hasError = node->hasPrioError();
        hasFatal = node->hasPrioFatal();
    }

    mMenuActions[static_cast<int>(eLogActions::PrioNotset)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioNotset, false), tr("&Reset Priority"));
    mMenuActions[static_cast<int>(eLogActions::PrioNotset)]->setCheckable(false);

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
    
    mMenuActions[static_cast<int>(eLogActions::ExpandSelected)] = menu.addAction(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove), tr("Expand Selected"));
    mMenuActions[static_cast<int>(eLogActions::ExpandSelected)]->setEnabled((ctrlTable()->isExpanded(index) == false) && (node->hasChildren()));
    
    mMenuActions[static_cast<int>(eLogActions::CollapseSelected)] = menu.addAction(QIcon::fromTheme(QIcon::ThemeIcon::ListAdd), tr("Collapse Selected"));
    mMenuActions[static_cast<int>(eLogActions::CollapseSelected)]->setEnabled(ctrlTable()->isExpanded(index) && node->hasChildren());
    
    mMenuActions[static_cast<int>(eLogActions::ExpandAll)] = menu.addAction(tr("Expand All"));
    mMenuActions[static_cast<int>(eLogActions::ExpandAll)]->setEnabled(true);

    mMenuActions[static_cast<int>(eLogActions::CollapseAll)] = menu.addAction(tr("Collapse All"));
    mMenuActions[static_cast<int>(eLogActions::CollapseAll)]->setEnabled(areRootsCollapsed() == false);

    mMenuActions[static_cast<int>(eLogActions::SavePrioTarget)] = menu.addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave), tr("&Save Selection on Target"));
    mMenuActions[static_cast<int>(eLogActions::SavePrioTarget)]->setEnabled(LogObserver::isConnected());

    mMenuActions[static_cast<int>(eLogActions::SavePrioAll)] = menu.addAction(tr("Save &All Targets"));
    mMenuActions[static_cast<int>(eLogActions::SavePrioAll)]->setEnabled(LogObserver::isConnected());


    QAction* selectedAction = menu.exec(ctrlTable()->viewport()->mapToGlobal(pos));
    if (!selectedAction)
        return;

    if (selectedAction == mMenuActions[static_cast<int>(eLogActions::PrioNotset)])
    {
        mScopesModel->setLogPriority(index, NELogging::eLogPriority::PrioNotset);
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
    }
    else if (selectedAction == mMenuActions[eLogActions::CollapseSelected])
    {
        ctrlTable()->collapse(index);
    }
    else if (selectedAction == mMenuActions[eLogActions::ExpandAll])
    {
        onCollapseClicked(true);
    }
    else if (selectedAction == mMenuActions[eLogActions::CollapseAll])
    {
        onCollapseClicked(false);
    }
    else if (selectedAction == mMenuActions[eLogActions::SavePrioTarget])
    {
        Q_ASSERT(mScopesModel != nullptr);
        mScopesModel->saveLogScopePriority(index);
    }
    else if (selectedAction == mMenuActions[eLogActions::SavePrioAll])
    {
        mScopesModel->saveLogScopePriority(mScopesModel != nullptr ? mScopesModel->getRootIndex() : QModelIndex());
    }
}

void NaviLiveLogsScopes::onWindowCreated(MdiChild* mdiChild)
{
    ctrlMoveBottom()->setEnabled((mdiChild != nullptr) && (mdiChild->isLogViewerWindow()));
}

void NaviLiveLogsScopes::onNodeExpanded(const QModelIndex& index)
{
    if (areRootsCollapsed() == false)
    {
        ctrlCollapse()->setIcon(QIcon::fromTheme(QString::fromUtf8("list-add")));
        ctrlCollapse()->setChecked(false);
    }
    
    Q_ASSERT(mScopesModel != nullptr);    
    mScopesModel->nodeExpanded(index);
}

void NaviLiveLogsScopes::onNodeCollapsed(const QModelIndex& index)
{
    if (areRootsCollapsed())
    {
        ctrlCollapse()->setIcon(QIcon::fromTheme(QString::fromUtf8("list-remove")));
        ctrlCollapse()->setChecked(true);
    }
    
    Q_ASSERT(mScopesModel != nullptr);    
    mScopesModel->nodeCollapsed(index);    
}

void NaviLiveLogsScopes::onNodeActivated(const QModelIndex &idxNode)
{
    Q_ASSERT(mScopesModel != nullptr);    
    mScopesModel->nodeSelected(index);
}
