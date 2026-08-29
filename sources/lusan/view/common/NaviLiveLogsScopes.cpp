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
 *  \file        lusan/view/common/NaviLiveLogsScopes.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the log explorer.
 *
 ************************************************************************/

#include "lusan/view/common/NaviLiveLogsScopes.hpp"

#include "lusan/app/LusanApplication.hpp"

#include "lusan/data/log/LogObserver.hpp"

#include "lusan/model/log/LiveLogsModel.hpp"
#include "lusan/model/log/LiveScopesModel.hpp"

#include "lusan/view/common/MdiChild.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/log/LiveLogViewer.hpp"
#include "lusan/view/log/LogPriorityBar.hpp"

#include "areg/base/SocketDefs.hpp"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <filesystem>

namespace
{
    //!< How long the panel waits before it asks the log collector service again.
    constexpr int   RetryMs { 3000 };
}

NaviLiveLogsScopes* _explorer{ nullptr };
void NaviLiveLogsScopes::_logObserverStarted()
{
    if (_explorer != nullptr)
    {
        _explorer->setupLogSignals(true);
    }
}

NaviLiveLogsScopes::NaviLiveLogsScopes(MdiMainWindow* wndMain, QWidget* parent)
    : NaviLogScopeBase(static_cast<int>(NavigationDock::eNaviWindow::NaviLiveLogs), wndMain, parent)

    , mToolConnect      (nullptr)
    , mToolSettings     (nullptr)
    , mToolSave         (nullptr)
    , mAddress          ()
    , mPort             (areg::InvalidPort)
    , mInitLogFile      ( )
    , mActiveLogFile    ( )
    , mLogLocation      ( )
    , mSignalsActive    (false)
    , mState            (eLoggingStates::LoggingUndefined)
    , mConnectBar       (nullptr)
    , mConnectText      (nullptr)
    , mRetryTimer       (nullptr)
    , mAttempts         (0)
{
    _explorer = this;

    setupScopeToolbar();
    setupModel(new LiveScopesModel(this));
    setupScopeControls();
    setupWidgets();
    setupSignals();
}

NaviLiveLogsScopes::~NaviLiveLogsScopes()
{
    _explorer = nullptr;
}

void NaviLiveLogsScopes::addSpecificTools(void)
{
    mToolConnect = addToolButton( NELusanCommon::iconLiveLogDisconnected(NELusanCommon::SizeBig)
                                , tr("Connect to log collector")
                                , tr("Connect or disconnect log collector service.")
                                , true);
    mToolConnect->setStyleSheet(NELusanCommon::getStyleToolbutton());

    mToolSettings = addToolButton( NELusanCommon::iconSettings(NELusanCommon::SizeBig)
                                 , tr("Change log collector connection settings")
                                 , tr("Change log collector service connection settings"));

    mToolSave = addToolButton( NELusanCommon::iconSaveAsDocument(NELusanCommon::SizeBig)
                             , tr("Save log priorities on every connected target")
                             , tr("Writes the current log priorities into the configuration file of every connected target."));

    // Connect reports the state of the panel, so it stays whatever the width is.
    setToolFixed(mToolConnect);
}

bool NaviLiveLogsScopes::hasSavePrioMenu(void) const
{
    return true;
}

bool NaviLiveLogsScopes::canSavePrio(void) const
{
    return LogObserver::isConnected();
}

const QString& NaviLiveLogsScopes::getLogCollectorAddress() const
{
    return mAddress;
}

void NaviLiveLogsScopes::setLogCollectorAddress(const QString& address)
{
    mAddress = address;
}

uint16_t NaviLiveLogsScopes::getLogCollectorPort() const
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

void NaviLiveLogsScopes::disconnectLogging(void)
{
    if (mSignalsActive || isConnected())
    {
        onConnectClicked(false);
    }
}

void NaviLiveLogsScopes::setupWidgets()
{
    setupConnectStatus();
    ctrlCollapse()->setEnabled(true);
    ctrlConnect()->setEnabled(true);
    ctrlSettings()->setEnabled(true);
    ctrlSaveSettings()->setEnabled(true);
    ctrlFind()->setEnabled(true);
    ctrlPriorityBar()->setEnabled(false);
    ctrlPriorityBar()->setIdle(true);
    ctrlShowOnly()->setEnabled(false);
    ctrlHide()->setEnabled(false);
    ctrlShowAll()->setEnabled(false);
}

void NaviLiveLogsScopes::setupConnectStatus()
{
    mConnectBar = new QWidget(this);
    QHBoxLayout* row = new QHBoxLayout(mConnectBar);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(4);

    QLabel* icon = new QLabel(mConnectBar);
    icon->setPixmap(NELusanCommon::iconLiveLogDisconnected(NELusanCommon::SizeSmall).pixmap(NELusanCommon::SizeSmall));

    mConnectText = new QLabel(mConnectBar);
    mConnectText->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Preferred);

    QToolButton* stop = new QToolButton(mConnectBar);
    stop->setText(tr("Stop"));
    stop->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
    stop->setAutoRaise(true);
    stop->setToolTip(tr("Stop waiting for the log collector service."));
    stop->setAccessibleName(stop->toolTip());

    row->addWidget(icon, 0);
    row->addWidget(mConnectText, 1);
    row->addWidget(stop, 0);

    addNaviBar(mConnectBar);
    mConnectBar->setVisible(false);

    mRetryTimer = new QTimer(this);
    mRetryTimer->setInterval(RetryMs);

    connect(stop        , &QToolButton::clicked, this, [this]() { onConnectClicked(false); });
    connect(mRetryTimer , &QTimer::timeout     , this, [this]() { retryConnect(); });
}

void NaviLiveLogsScopes::beginConnecting()
{
    mState = eLoggingStates::LoggingConnecting;
    mAttempts = 0;
    updateConnectStatus();
    mRetryTimer->start();
}

void NaviLiveLogsScopes::stopConnecting()
{
    mRetryTimer->stop();
    mAttempts = 0;
    if (mConnectBar != nullptr)
    {
        mConnectBar->setVisible(false);
    }
}

void NaviLiveLogsScopes::countAttempt()
{
    ++mAttempts;
    updateConnectStatus();
}

void NaviLiveLogsScopes::updateConnectStatus()
{
    if (mConnectBar == nullptr)
        return;

    const QString target{ QString("%1:%2").arg(mAddress).arg(mPort) };
    mConnectText->setText(mAttempts == 0
                            ? tr("Connecting to %1...").arg(target)
                            : tr("Retrying to connect to %1, attempt %2").arg(target).arg(mAttempts));
    mConnectText->setToolTip(tr("The scopes appear here as soon as the log collector service answers."));
    mConnectBar->setVisible(true);
}

void NaviLiveLogsScopes::retryConnect()
{
    if (isConnecting() == false)
    {
        mRetryTimer->stop();
        return;
    }

    if (LogObserver::isConnected())
        return;

    LogObserver::connect(mAddress, mPort, databasePath());
}

QString NaviLiveLogsScopes::databasePath() const
{
    std::error_code err;
    std::filesystem::path dbPath(mLogLocation.toStdString());
    dbPath /= mInitLogFile.toStdString();
    return QString(std::filesystem::absolute(dbPath, err).c_str());
}

void NaviLiveLogsScopes::setupSignals()
{
    connect(ctrlConnect()       , &QToolButton::clicked, this, &NaviLiveLogsScopes::onConnectClicked);
    connect(ctrlSaveSettings()  , &QToolButton::clicked, this, &NaviLiveLogsScopes::onSaveSettingsClicked);
    connect(ctrlSettings()      , &QToolButton::clicked, this, &NaviLiveLogsScopes::onOptionsClicked);
    connect(mMainWindow         , &MdiMainWindow::signalNewLiveLog      , this  , [this](){onConnectClicked(true);});

    setupLogSignals(true);
}

void NaviLiveLogsScopes::blockBasicSignals(bool block)
{
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
            connect(mScopesModel, &LiveScopesModel::signalRootUpdated   , this, &NaviLiveLogsScopes::onRootUpdated);
            connect(mScopesModel, &LiveScopesModel::signalScopesInserted, this, &NaviLiveLogsScopes::onScopesInserted);
            connect(mScopesModel, &LiveScopesModel::dataChanged         , this, &NaviLiveLogsScopes::onScopesDataChanged);
        }
    }
    else if (mSignalsActive)
    {
        Q_ASSERT(mScopesModel != nullptr);
        LoggingModelBase* logModel{ mScopesModel->getLoggingModel() };

        disconnect(mScopesModel  , &LiveScopesModel::signalRootUpdated   , this, &NaviLiveLogsScopes::onRootUpdated);
        disconnect(mScopesModel  , &LiveScopesModel::signalScopesInserted, this, &NaviLiveLogsScopes::onScopesInserted);
        disconnect(mScopesModel  , &LiveScopesModel::dataChanged         , this, &NaviLiveLogsScopes::onScopesDataChanged);

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

void NaviLiveLogsScopes::onLogObserverConfigured(bool isEnabled, const QString& address, uint16_t port)
{
    ctrlConnect()->setEnabled(isEnabled);
    ctrlConnect()->setIcon(NELusanCommon::iconLiveLogDisconnected(NELusanCommon::SizeBig));
    ctrlConnect()->setToolTip(isEnabled ? tr("Connect to log collector") : tr("Logging is not enabled"));

    mAddress= address;
    mPort   = port;
    if (isConnecting() == false)
    {
        mState = eLoggingStates::LoggingConfigured;
    }
    else
    {
        updateConnectStatus();
    }
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
        stopConnecting();
    }
    else if (isConnecting())
    {
        // The log collector service is not answering yet. The panel keeps the session and
        // the button as they are, so the user stays where the live scopes will appear.
        countAttempt();
        return;
    }

    enableButtons(QModelIndex());

    LogObserver* log = LogObserver::getComponent();
    ctrlConnect()->setChecked(isConnected);
    ctrlConnect()->setIcon(isConnected ? NELusanCommon::iconLiveLogConnected(NELusanCommon::SizeBig) : NELusanCommon::iconLiveLogDisconnected(NELusanCommon::SizeBig));
    // Connected, the button disconnects. The tooltip names the action as well as the
    // address, so the hover text never describes a different button.
    ctrlConnect()->setToolTip(isConnected
                                ? tr("Disconnect from %1:%2").arg(address).arg(port)
                                : tr("Connect to log collector"));
    Q_ASSERT(mMainWindow != nullptr);
    mMainWindow->logCollecttorConnected(isConnected, address, port, log != nullptr ? log->getActiveDatabase() : mActiveLogFile);
}

void NaviLiveLogsScopes::onLogObserverStarted(bool isStarted)
{
    if ((isStarted == false) && (isConnecting() == false))
    {
        onConnectClicked(false);
    }
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
        LogObserver::connect(mAddress, mPort, databasePath());
        setupLogSignals(true);
        enableButtons(QModelIndex());
    }
    else if (isConnecting() == false)
    {
        onConnectClicked(false);
    }
}

void NaviLiveLogsScopes::onConnectClicked(bool checked)
{
    if (checked)
    {
        ctrlConnect()->setChecked(true);
        if (LogObserver::isConnected() == false)
        {
            Q_ASSERT(mMainWindow != nullptr);
            beginConnecting();
            LiveLogsModel* logModel{ mMainWindow->setupLiveLogging() };
            mScopesModel->setLoggingModel(logModel);
            LogObserver::createLogObserver(&NaviLiveLogsScopes::_logObserverStarted);
        }
    }
    else
    {
        QString address{ LogObserver::getConnectedAddress() };
        uint16_t port{ LogObserver::getConnectedPort() };
        QString logFile{ mActiveLogFile.isEmpty() == false ? mActiveLogFile : LogObserver::getActiveDatabase() };

        stopConnecting();
        setupLogSignals(false);

        ctrlConnect()->setChecked(false);
        ctrlConnect()->setIcon(NELusanCommon::iconLiveLogDisconnected(NELusanCommon::SizeBig));
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

void NaviLiveLogsScopes::onRootUpdated(const QModelIndex & root)
{
    Q_ASSERT(mScopesModel != nullptr);
    if (isConnected())
    {
        mState = eLoggingStates::LoggingRunning;
    }

    expandNodeAndChildren(root, true);
}

void NaviLiveLogsScopes::onScopesInserted(const QModelIndex & parent)
{
    Q_ASSERT(mScopesModel != nullptr);
    if (parent.isValid())
    {
        refreshButtons();
        expandNode(parent, true);
    }
}

void NaviLiveLogsScopes::onScopesUpdated(const QModelIndex & parent)
{
    if (parent.isValid())
    {
        refreshButtons();
        ctrlTable()->update(parent);
    }
}

void NaviLiveLogsScopes::onScopesDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles /*= QList<int>()*/)
{
    refreshButtons();
    updateExpanded(ctrlTable()->rootIndex());
}

void NaviLiveLogsScopes::optionOpenning()
{
    stopConnecting();
    if (isConnected())
    {
        QString address{ LogObserver::getConnectedAddress() };
        uint16_t port{ LogObserver::getConnectedPort() };
        QString logFile{ mActiveLogFile.isEmpty() == false ? mActiveLogFile : LogObserver::getActiveDatabase() };

        setupLogSignals(false);
        mState = eLoggingStates::LoggingPaused;
        LogObserver::disconnect();
        LogObserver::releaseLogObserver();
        mMainWindow->logCollecttorConnected(false, address, port, logFile);
        mScopesModel->setLoggingModel(nullptr);
        mScopesModel->releaseModel();
    }

    enableButtons(QModelIndex());
}

void NaviLiveLogsScopes::optionApplied()
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
        ctrlConnect()->setChecked(true);
        Q_ASSERT(mMainWindow != nullptr);
        beginConnecting();
        LiveLogsModel* logModel{ mMainWindow->setupLiveLogging() };
        mScopesModel->setLoggingModel(logModel);
        LogObserver::createLogObserver(&NaviLiveLogsScopes::_logObserverStarted);
    }
    else if (mState != eLoggingStates::LoggingUndefined)
    {
        mState = eLoggingStates::LoggingConfigured;
    }
}

