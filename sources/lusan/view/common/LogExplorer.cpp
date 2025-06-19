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
 *  \file        lusan/view/common/LogExplorer.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the log explorer.
 *
 ************************************************************************/

#include "lusan/view/common/LogExplorer.hpp"
#include "ui/ui_LogExplorer.h"

#include "lusan/app/LusanApplication.hpp"

#include "lusan/data/log/LogObserver.hpp"
#include "lusan/data/log/ScopeNodeBase.hpp"

#include "lusan/model/log/LogScopeIconFactory.hpp"
#include "lusan/model/log/LogScopesModel.hpp"

#include "lusan/view/common/MdiChild.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/log/LogViewer.hpp"

#include "areg/base/NESocket.hpp"

#include <QAction>
#include <QMenu>
#include <filesystem>

LogExplorer* _explorer{ nullptr };
void LogExplorer::_logObserverStarted(void)
{
    if (_explorer != nullptr)
    {
        _explorer->setupLogSignals(true);
    }
}

LogExplorer::LogExplorer(MdiMainWindow* wndMain, QWidget* parent)
    : NavigationWindow(NavigationWindow::eNavigationWindow::NaviLiveLogs, wndMain, parent)

    , ui            (new Ui::LogExplorer)
    , mAddress      ()
    , mPort         (NESocket::InvalidPort)
    , mInitLogFile  ( )
    , mActiveLogFile( )
    , mLogLocation  ( )
    , mModel        (nullptr)
    , mSelModel     (nullptr)
    , mSignalsActive(false)
    , mState        (eLoggingStates::LoggingUndefined)
{
    _explorer = this;
    for (int i = 0; i < static_cast<int>(eLogActions::PrioCount); ++ i)
    {
        mMenuActions[i] = nullptr;
    }
    
    ui->setupUi(this);
    this->setBaseSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    this->setMinimumSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    this->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    updateData();
    setupWidgets();
    setupSignals();
}

LogExplorer::~LogExplorer(void)
{
    _explorer = nullptr;
    mModel->release();
    delete ui;
}

const QString& LogExplorer::getLogCollectorAddress(void) const
{
    return mAddress;
}

void LogExplorer::setLogCollectorAddress(const QString& address)
{
    mAddress = address;
}

uint16_t LogExplorer::getLogCollectorPort(void) const
{
    return mPort;
}

void LogExplorer::setLogCollectorPort(uint16_t port)
{
    mPort = port;
}

void LogExplorer::setLogCollectorConnection(const QString& address, uint16_t port)
{
    mAddress = address;
    mPort = port;
}

QToolButton* LogExplorer::ctrlCollapse(void)
{
    return ui->toolCollapse;
}

QToolButton* LogExplorer::ctrlConnect(void)
{
    return ui->toolConnect;
}

QToolButton* LogExplorer::ctrlSettings(void)
{
    return ui->toolSettings;
}

QToolButton* LogExplorer::ctrlSaveSettings(void)
{
    return ui->toolSaveSettings;
}

QToolButton* LogExplorer::ctrlFind(void)
{
    return ui->toolFind;
}

QToolButton* LogExplorer::ctrlLogError(void)
{
    return ui->toolError;
}

QToolButton* LogExplorer::ctrlLogWarning(void)
{
    return ui->toolWarning;
}

QToolButton* LogExplorer::ctrlLogInfo(void)
{
    return ui->toolInformation;
}

QToolButton* LogExplorer::ctrlLogDebug(void)
{
    return ui->toolDebug;
}

QToolButton* LogExplorer::ctrlLogScopes(void)
{
    return ui->toolScopes;
}

QToolButton* LogExplorer::ctrlMoveBottom(void)
{
    return ui->toolMoveBottom;
}

QTreeView* LogExplorer::ctrlTable(void)
{
    return ui->treeView;
}

void LogExplorer::updateData(void)
{
}

void LogExplorer::setupWidgets(void)
{
    ctrlCollapse()->setEnabled(false);
    ctrlConnect()->setEnabled(true);
    ctrlSettings()->setEnabled(true);
    ctrlSaveSettings()->setEnabled(true);
    ctrlFind()->setEnabled(false);
    ctrlLogError()->setEnabled(false);
    ctrlLogWarning()->setEnabled(false);
    ctrlLogInfo()->setEnabled(false);
    ctrlLogDebug()->setEnabled(false);
    ctrlLogScopes()->setEnabled(false);

    ctrlTable()->setContextMenuPolicy(Qt::CustomContextMenu);

    onWindowActivated(mMainWindow->getActiveWindow());
}

void LogExplorer::setupSignals(void)
{
    connect(ctrlConnect()       , &QToolButton::clicked, this, &LogExplorer::onConnectClicked);
    connect(ctrlMoveBottom()    , &QToolButton::clicked, this, &LogExplorer::onMoveBottomClicked);
    connect(ctrlLogError()      , &QToolButton::clicked, this, &LogExplorer::onPrioErrorClicked);
    connect(ctrlLogWarning()    , &QToolButton::clicked, this, &LogExplorer::onPrioWarningClicked);
    connect(ctrlLogInfo()       , &QToolButton::clicked, this, &LogExplorer::onPrioInfoClicked);
    connect(ctrlLogDebug()      , &QToolButton::clicked, this, &LogExplorer::onPrioDebugClicked);
    connect(ctrlLogScopes()     , &QToolButton::clicked, this, &LogExplorer::onPrioScopesClicked);
    connect(ctrlSaveSettings()  , &QToolButton::clicked, this, &LogExplorer::onSaveSettingsClicked);
    connect(ctrlSettings()      , &QToolButton::clicked, this, &LogExplorer::onOptionsClicked);
    connect(ctrlTable()         , &QWidget::customContextMenuRequested, this, &LogExplorer::onTreeViewContextMenuRequested);

    connect(mMainWindow         , &MdiMainWindow::signalWindowActivated , this  , &LogExplorer::onWindowActivated);
    connect(mMainWindow         , &MdiMainWindow::signalWindowCreated   , this  , &LogExplorer::onWindowCreated);

    setupLogSignals(true);    
}

void LogExplorer::blockBasicSignals(bool block)
{
}

void LogExplorer::updateColors(bool errSelected, bool warnSelected, bool infoSelected, bool dbgSelected, bool scopeSelected)
{
    ctrlLogDebug()->setIcon(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioDebug, dbgSelected));
    ctrlLogInfo()->setIcon(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioInfo, infoSelected));
    ctrlLogWarning()->setIcon(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioWarn, warnSelected));
    ctrlLogError()->setIcon(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioError, errSelected));
    ctrlLogScopes()->setIcon(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioScope, scopeSelected));

    ctrlLogError()->update();
    ctrlLogWarning()->update();
    ctrlLogInfo()->update();
    ctrlLogDebug()->update();
    ctrlLogScopes()->update();
}

void LogExplorer::updateExpanded(const QModelIndex& current)
{
    QTreeView* tree = current.isValid() && (mModel != nullptr) ? ctrlTable() : nullptr;
    if (tree != nullptr)
    {
        tree->update(current);
        int count = tree->isExpanded(current) ? mModel->rowCount(current) : 0;
        for (int i = 0; i < count; ++ i)
        {
            QModelIndex index = mModel->index(i, 0, current);
            updateExpanded(index);
        }
    }
}

bool LogExplorer::updatePriority(const QModelIndex& node, bool addPrio, NELogging::eLogPriority prio)
{
    bool result{ false };
    if (node.isValid())
    {
        Q_ASSERT(mModel != nullptr);
        if (addPrio)
        {
            result = mModel->addLogPriority(node, prio);
        }
        else
        {
            result = mModel->removeLogPriority(node, prio);
        }
    }
    
    return result;
}

void LogExplorer::setupLogSignals(bool setup)
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
            connect(log, &LogObserver::signalLogObserverConfigured  , this, &LogExplorer::onLogObserverConfigured   , Qt::QueuedConnection);
            connect(log, &LogObserver::signalLogDbConfigured        , this, &LogExplorer::onLogDbConfigured         , Qt::QueuedConnection);
            connect(log, &LogObserver::signalLogServiceConnected    , this, &LogExplorer::onLogServiceConnected     , Qt::QueuedConnection);
            connect(log, &LogObserver::signalLogObserverStarted     , this, &LogExplorer::onLogObserverStarted      , Qt::QueuedConnection);
            connect(log, &LogObserver::signalLogDbCreated           , this, &LogExplorer::onLogDbCreated            , Qt::QueuedConnection);
            connect(log, &LogObserver::signalLogObserverInstance    , this, &LogExplorer::onLogObserverInstance     , Qt::QueuedConnection);
        }
    }
    else if (mSignalsActive)
    {
        disconnect(log, &LogObserver::signalLogObserverConfigured   , this, &LogExplorer::onLogObserverConfigured);
        disconnect(log, &LogObserver::signalLogDbConfigured         , this, &LogExplorer::onLogDbConfigured);
        disconnect(log, &LogObserver::signalLogServiceConnected     , this, &LogExplorer::onLogServiceConnected);
        disconnect(log, &LogObserver::signalLogObserverStarted      , this, &LogExplorer::onLogObserverStarted);
        disconnect(log, &LogObserver::signalLogDbCreated            , this, &LogExplorer::onLogDbCreated);
        disconnect(log, &LogObserver::signalLogObserverInstance     , this, &LogExplorer::onLogObserverInstance);
        mSignalsActive =  false;
    }
}

void LogExplorer::enableButtons(const QModelIndex & selection)
{
    ScopeNodeBase * node = selection.isValid() ? mModel->data(selection, Qt::ItemDataRole::UserRole).value<ScopeNodeBase *>() : nullptr;
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

void LogExplorer::onLogObserverConfigured(bool isEnabled, const QString& address, uint16_t port)
{
    ctrlConnect()->setEnabled(isEnabled);
    ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-offline")));
    ctrlConnect()->setToolTip(isEnabled ? tr("Connect to log collector") : tr("Logging is not enabled"));
    
    mAddress= address;
    mPort   = port;
    mState  = eLoggingStates::LoggingConfigured;
}

void LogExplorer::onLogDbConfigured(bool isEnabled, const QString& dbName, const QString& dbLocation, const QString& dbUser)
{
    mInitLogFile    = dbName;
    mLogLocation    = dbLocation;
}

void LogExplorer::onLogServiceConnected(bool isConnected, const QString& address, uint16_t port)
{
    if (isConnected)
    {
        mState = eLoggingStates::LoggingConnected;
        mModel->release();
        mModel->initialize();
    }
    else
    {
        mSelModel->reset();
        mModel->release();
        if (this->isConnected())
        {
            mState = eLoggingStates::LoggingDisconnected;
        }
    }

    enableButtons(QModelIndex());

    LogObserver* log = LogObserver::getComponent();
    ctrlConnect()->setChecked(isConnected);
    ctrlConnect()->setIcon(QIcon::fromTheme(isConnected ? QString::fromUtf8("network-wireless") : QString::fromUtf8("network-offline")));
    ctrlConnect()->setToolTip(isConnected ? address + ":" + QString::number(port) : tr("Connect to log collector"));
    Q_ASSERT(mMainWindow != nullptr);
    mMainWindow->logCollecttorConnected(isConnected, address, port, log != nullptr ? log->getActiveDatabase() : mActiveLogFile);
}

void LogExplorer::onLogObserverStarted(bool isStarted)
{
}

void LogExplorer::onLogDbCreated(const QString& dbLocation)
{
    mActiveLogFile = dbLocation;
    LogObserver* log = LogObserver::getComponent();
    Q_ASSERT(log != nullptr);
    mMainWindow->logCollecttorConnected(true, log->getConnectedAddress(), log->getConnectedPort(), mActiveLogFile);
}

void LogExplorer::onLogObserverInstance(bool isStarted, const QString& address, uint16_t port, const QString& filePath)
{
    if (isStarted)
    {
        if (mSelModel == nullptr)
        {
            Q_ASSERT(mModel == nullptr);
            mModel = new LogScopesModel(ctrlTable());
            mSelModel = new QItemSelectionModel(mModel, ctrlTable());
            ctrlTable()->setModel(mModel);
            ctrlTable()->setSelectionModel(mSelModel);
            
            connect(mModel      , &LogScopesModel::signalRootUpdated    , this, &LogExplorer::onRootUpdated);
            connect(mModel      , &LogScopesModel::signalScopesInserted , this, &LogExplorer::onScopesInserted);
            connect(mModel      , &LogScopesModel::dataChanged          , this, &LogExplorer::onScopesDataChanged);
            connect(mSelModel   , &QItemSelectionModel::selectionChanged, this, &LogExplorer::onSelectionChanged);        
        }

        std::error_code err;
        std::filesystem::path dbPath(mLogLocation.toStdString());
        dbPath /= mInitLogFile.toStdString();
        QString logPath(std::filesystem::absolute(dbPath, err).c_str());
        LogObserver::connect(mAddress, mPort, logPath);
    }
    
    setupLogSignals(isStarted);
    enableButtons(QModelIndex());
}

void LogExplorer::onConnectClicked(bool checked)
{
    if (checked)
    {
        LogObserver::createLogObserver(&LogExplorer::_logObserverStarted);
    }
    else
    {
        LogObserver::disconnect();
        
        ctrlConnect()->setChecked(false);
        ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-offline")));
        ctrlConnect()->setToolTip(tr("Connect to log collector"));
        
        setupLogSignals(false);
        mState = eLoggingStates::LoggingDisconnected;
        LogObserver::releaseLogObserver();
    }
}

void LogExplorer::onMoveBottomClicked()
{
    Q_ASSERT(mMainWindow != nullptr);
    MdiChild* wndActive = mMainWindow->getActiveWindow();
    if ((wndActive != nullptr) && wndActive->isLogViewerWindow())
    {
        qobject_cast<LogViewer*>(wndActive)->moveToBottom(true);
    }
}

void LogExplorer::onPrioErrorClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    bool result = updatePriority(current, checked, NELogging::eLogPriority::PrioError);
    if (result == false)
    {
        ctrlLogError()->setChecked(!checked);
    }
}

void LogExplorer::onPrioWarningClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    bool result = updatePriority(current, checked, NELogging::eLogPriority::PrioWarning);
    if (result == false)
    {
        ctrlLogWarning()->setChecked(!checked);
    }
}

void LogExplorer::onPrioInfoClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    bool result = updatePriority(current, checked, NELogging::eLogPriority::PrioInfo);
    if (result == false)
    {
        ctrlLogInfo()->setChecked(!checked);
    }
}

void LogExplorer::onPrioDebugClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    bool result = updatePriority(current, checked, NELogging::eLogPriority::PrioDebug);
    if (result == false)
    {
        ctrlLogDebug()->setChecked(!checked);
    }
}

void LogExplorer::onPrioScopesClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    bool result = updatePriority(current, checked, NELogging::eLogPriority::PrioScope);
    if (result == false)
    {
        ctrlLogScopes()->setChecked(!checked);
    }
}

void LogExplorer::onSaveSettingsClicked(bool checked)
{
    if (mModel != nullptr)
    {
        mModel->saveLogScopePriority(QModelIndex());
    }
}

void LogExplorer::onOptionsClicked(bool checked)
{
    LogObserver* log = LogObserver::getComponent();
    QString address = log != nullptr ? log->getConnectedAddress() : mAddress;
    uint16_t port = log != nullptr ? log->getConnectedPort() : mPort;
    QString logFile = log != nullptr ? log->getConfigDatabaseName() : mInitLogFile;
    QString logLocation = log != nullptr ? log->getConfigDatabaseLocation() : mLogLocation;
    
    mMainWindow->showOptionPageLogging(address, port, logFile, logLocation);
}

void LogExplorer::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList list = selected.indexes();
    enableButtons(list.isEmpty() ? QModelIndex() : list.front());
}

void LogExplorer::onRootUpdated(const QModelIndex & root)
{
    if (mModel != nullptr)
    {
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
        int rowCount = mModel->rowCount(root);
        for (int row = 0; row < rowCount; ++row)
        {
            QModelIndex child = mModel->index(row, 0, root);
            if (child.isValid() && !navi->isExpanded(child))
            {
                navi->expand(child);
            }
        }
    }
}

void LogExplorer::onScopesInserted(const QModelIndex & parent)
{
    if ((mModel != nullptr) && (parent.isValid()))
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

void LogExplorer::onScopesUpdated(const QModelIndex & parent)
{
    if (parent.isValid())
    {
        enableButtons(parent);
        ctrlTable()->update(parent);
    }
}

void LogExplorer::onScopesDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles /*= QList<int>()*/)
{
    if (mSelModel != nullptr)
    {
        enableButtons(ctrlTable()->currentIndex());
        updateExpanded(ctrlTable()->rootIndex());
    }
}

void LogExplorer::optionOpenning(void)
{
    if (isConnected())
    {
        setupLogSignals(false);
        mState = eLoggingStates::LoggingPaused;
        LogObserver::disconnect();
        LogObserver::releaseLogObserver();
    }
}

void LogExplorer::optionApplied(void)
{
    if (isPaused())
    {
        mState = eLoggingStates::LoggingStopped;
    }
}

void LogExplorer::optionClosed(bool OKpressed)
{
    if (isStopped() || isPaused())
    {
        LogObserver::createLogObserver(&LogExplorer::_logObserverStarted);
    }
    else if (mState != eLoggingStates::LoggingUndefined)
    {
        mState = eLoggingStates::LoggingConfigured;
    }
}

void LogExplorer::onTreeViewContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = ctrlTable()->indexAt(pos);
    if (!index.isValid() || !mModel)
        return;

    // Get current priority of the selected node
    ScopeNodeBase* node = mModel->data(index, Qt::UserRole).value<ScopeNodeBase*>();
    if ((node == nullptr) || (node->hasPrioValid() == false))
        return;

    uint32_t prio = node->getPriority();
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

    mMenuActions[static_cast<int>(eLogActions::PrioNotset)] = menu.addAction(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioNotset, false), tr("&Reset Priority"));
    mMenuActions[static_cast<int>(eLogActions::PrioNotset)]->setCheckable(false);

    mMenuActions[static_cast<int>(eLogActions::PrioDebug)] = menu.addAction(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioDebug, hasDebug), hasDebug ? tr("Hide &Debug messages") : tr("Show &Debug messages"));
    mMenuActions[static_cast<int>(eLogActions::PrioDebug)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioDebug)]->setChecked(hasDebug);

    mMenuActions[static_cast<int>(eLogActions::PrioInfo)] = menu.addAction(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioInfo, hasInfo), hasInfo ? tr("Hide &Info messages") : tr("Show &Info messages"));
    mMenuActions[static_cast<int>(eLogActions::PrioInfo)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioInfo)]->setChecked(hasInfo);

    mMenuActions[static_cast<int>(eLogActions::PrioWarn)] = menu.addAction(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioWarn, hasWarn), hasWarn ? tr("Hide &Warning messages") : tr("Show &Warning messages"));
    mMenuActions[static_cast<int>(eLogActions::PrioWarn)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioWarn)]->setChecked(hasWarn);

    mMenuActions[static_cast<int>(eLogActions::PrioError)] = menu.addAction(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioError, hasError), hasError ? tr("Hide &Error messages") : tr("Show &Error messages"));
    mMenuActions[static_cast<int>(eLogActions::PrioError)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioError)]->setChecked(hasError);

    mMenuActions[static_cast<int>(eLogActions::PrioFatal)] = menu.addAction(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioFatal, hasFatal), hasFatal ? tr("Hide &Fatal messages") : tr("Show &Fatal messages"));
    mMenuActions[static_cast<int>(eLogActions::PrioFatal)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioFatal)]->setChecked(hasFatal);

    mMenuActions[static_cast<int>(eLogActions::PrioScope)] = menu.addAction(LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioScope, hasScope), hasScope ? tr("Hide &Scopes") : tr("Show &Scopes"));
    mMenuActions[static_cast<int>(eLogActions::PrioScope)]->setCheckable(true);
    mMenuActions[static_cast<int>(eLogActions::PrioScope)]->setChecked(hasScope);

    mMenuActions[static_cast<int>(eLogActions::SavePrioTarget)] = menu.addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave), tr("&Save Selection on Target"));
    mMenuActions[static_cast<int>(eLogActions::SavePrioTarget)]->setEnabled(LogObserver::isConnected());

    mMenuActions[static_cast<int>(eLogActions::SavePrioAll)] = menu.addAction(tr("Save &All Targets"));
    mMenuActions[static_cast<int>(eLogActions::SavePrioAll)]->setEnabled(LogObserver::isConnected());


    QAction* selectedAction = menu.exec(ctrlTable()->viewport()->mapToGlobal(pos));
    if (!selectedAction)
        return;

    if (selectedAction == mMenuActions[static_cast<int>(eLogActions::PrioNotset)])
    {
        mModel->setLogPriority(index, NELogging::eLogPriority::PrioNotset);
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
    else if (selectedAction == mMenuActions[eLogActions::SavePrioTarget])
    {
        Q_ASSERT(mModel != nullptr);
        mModel->saveLogScopePriority(index);
    }
    else if (selectedAction == mMenuActions[eLogActions::SavePrioAll])
    {
        mModel->saveLogScopePriority(QModelIndex());
    }
}

void LogExplorer::onWindowCreated(MdiChild* mdiChild)
{
    ctrlMoveBottom()->setEnabled((mdiChild != nullptr) && (mdiChild->isLogViewerWindow()));
}

void LogExplorer::onWindowActivated(MdiChild* mdiChild)
{
    if ((mdiChild != nullptr) && (mdiChild->isLogViewerWindow()))
    {
        enableButtons(ctrlTable()->currentIndex());
        ctrlMoveBottom()->setEnabled(true);
        if (isActiveWindow() == false)
        {
            activateWindow();
        }
    }
    else
    {
        enableButtons(QModelIndex());
        ctrlMoveBottom()->setEnabled(false);
    }
}
