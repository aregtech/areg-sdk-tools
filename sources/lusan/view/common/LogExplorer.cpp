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

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/log/LogObserver.hpp"
#include "lusan/model/log/LogScopesModel.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/log/LogViewer.hpp"

#include "areg/base/File.hpp"
#include "areg/base/NESocket.hpp"

#include <filesystem>

namespace
{
    LogExplorer* _explorer{ nullptr };
    void _logObserverStarted(void)
    {
        if (_explorer != nullptr)
        {
            _explorer->setupLogSignals(true);
        }
    }
}

LogExplorer::LogExplorer(MdiMainWindow* mainFrame, QWidget* parent)
    : QWidget       (parent)

    , mMainFrame    (mainFrame)
    , ui            (new Ui::LogExplorer)
    , mAddress      ()
    , mPort         (NESocket::InvalidPort)
    , mInitLogFile  ( )
    , mActiveLogFile( )
    , mLogLocation  ( )
    , mShouldConnect(false)
    , mModel        (nullptr)
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
}

void LogExplorer::setupSignals(void)
{
    connect(ctrlConnect()       , &QToolButton::clicked, this, &LogExplorer::onConnectClicked);
    connect(ctrlMoveBottom()    , &QToolButton::clicked, this, &LogExplorer::onMoveBottomClicked);
}

void LogExplorer::blockBasicSignals(bool block)
{
}

void LogExplorer::setupLogSignals(bool setup)
{
    LogObserver* log = LogObserver::getComponent();
    Q_ASSERT(log != nullptr);
    if (setup)
    {
        connect(log, &LogObserver::signalLogObserverConfigured  , this, &LogExplorer::onLogObserverConfigured   , Qt::QueuedConnection);
        connect(log, &LogObserver::signalLogDbConfigured        , this, &LogExplorer::onLogDbConfigured         , Qt::QueuedConnection);
        connect(log, &LogObserver::signalLogServiceConnected    , this, &LogExplorer::onLogServiceConnected     , Qt::QueuedConnection);
        connect(log, &LogObserver::signalLogObserverStarted     , this, &LogExplorer::onLogObserverStarted      , Qt::QueuedConnection);
        connect(log, &LogObserver::signalLogDbCreated           , this, &LogExplorer::onLogDbCreated            , Qt::QueuedConnection);
    }
    else
    {
        disconnect(log, &LogObserver::signalLogObserverConfigured  , this, &LogExplorer::onLogObserverConfigured);
        disconnect(log, &LogObserver::signalLogDbConfigured        , this, &LogExplorer::onLogDbConfigured);
        disconnect(log, &LogObserver::signalLogServiceConnected    , this, &LogExplorer::onLogServiceConnected);
        disconnect(log, &LogObserver::signalLogObserverStarted     , this, &LogExplorer::onLogObserverStarted);
        disconnect(log, &LogObserver::signalLogDbCreated           , this, &LogExplorer::onLogDbCreated);
    }
}

void LogExplorer::onLogObserverConfigured(bool isEnabled, const QString& address, uint16_t port)
{
    ctrlConnect()->setEnabled(isEnabled);
    ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-offline")));
    ctrlConnect()->setToolTip(isEnabled ? tr("Connect to log collector") : tr("Logging is not enabled"));
    
    mAddress = address;
    mPort = port;
}

void LogExplorer::onLogDbConfigured(bool isEnabled, const QString& dbName, const QString& dbLocation, const QString& dbUser)
{
    mInitLogFile    = dbName;
    mLogLocation    = dbLocation;
    
    if (isEnabled && mShouldConnect)
    {
        mModel = new LogScopesModel(this);
        
        std::error_code err;
        std::filesystem::path dbPath(mLogLocation.toStdString());
        dbPath /= mInitLogFile.toStdString();
        QString logPath(std::filesystem::absolute(dbPath, err).c_str());
        LogObserver::connect(mAddress, mPort, logPath);
    }
    
    mShouldConnect = false;
}

void LogExplorer::onLogServiceConnected(bool isConnected, const QString& address, uint16_t port)
{
    if (mModel != nullptr)
    {
        if (isConnected)
        {
            mModel->initialize();
            ctrlTable()->setModel(mModel);
        }
        else
        {
            ctrlTable()->setModel(nullptr);
            mModel->release();
        }
    }
    
    LogObserver* log = LogObserver::getComponent();
    Q_ASSERT(log != nullptr);
    ctrlConnect()->setChecked(isConnected);
    ctrlConnect()->setIcon(QIcon::fromTheme(isConnected ? QString::fromUtf8("network-wireless") : QString::fromUtf8("network-offline")));
    ctrlConnect()->setToolTip(isConnected ? address + ":" + QString::number(port) : tr("Connect to log collector"));
    Q_ASSERT(mMainFrame != nullptr);
    mMainFrame->logCollecttorConnected(isConnected, address, port, log->getActiveDatabase());
}

void LogExplorer::onLogObserverStarted(bool isStarted)
{
}

void LogExplorer::onLogDbCreated(const QString& dbLocation)
{
    mActiveLogFile = dbLocation;
}

void LogExplorer::onConnectClicked(bool checked)
{
    mShouldConnect = checked;
    if (checked)
    {
        LogObserver::createLogObserver(&_logObserverStarted);
    }
    else
    {
        LogObserver::disconnect();
        LogObserver::releaseLogObserver();
        
        ctrlConnect()->setChecked(false);
        ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-offline")));
        ctrlConnect()->setToolTip(tr("Connect to log collector"));
        
        setupLogSignals(false);
    }
}

void LogExplorer::onMoveBottomClicked()
{
    MdiMainWindow* wndMain = LusanApplication::getMainWindow();
    LogViewer * logViewer = wndMain != nullptr ? wndMain->getLiveLogViewer() : nullptr;
    if (logViewer != nullptr)
    {
        logViewer->moveToBottom(true);
    }
}
