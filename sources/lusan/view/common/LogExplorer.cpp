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
#include "lusan/model/log/LogScopeIconFactory.hpp"
#include "lusan/model/log/LogScopesModel.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/log/LogViewer.hpp"
#include "lusan/data/log/ScopeNodes.hpp"

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
    , mSelModel     (nullptr)
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
    connect(ctrlLogError()      , &QToolButton::clicked, this, &LogExplorer::onPrioErrorClicked);
    connect(ctrlLogWarning()    , &QToolButton::clicked, this, &LogExplorer::onPrioWarningClicked);
    connect(ctrlLogInfo()       , &QToolButton::clicked, this, &LogExplorer::onPrioInfoClicked);
    connect(ctrlLogDebug()      , &QToolButton::clicked, this, &LogExplorer::onPrioDebugClicked);
    connect(ctrlLogScopes()     , &QToolButton::clicked, this, &LogExplorer::onPrioScopesClicked);
}

void LogExplorer::blockBasicSignals(bool block)
{
}

void LogExplorer::updateColors(bool errSelected, bool warnSelected, bool infoSelected, bool dbgSelected, bool scopeSelected)
{
    constexpr int selected{60};
    constexpr int notSelected{0};

    QToolButton * toolButton{nullptr};
    QPalette palette;
    QColor color;

    toolButton = ctrlLogDebug();
    palette = toolButton->palette();
    color = LogScopeIconFactory::getColor(NELogging::eLogPriority::PrioDebug);
    color.setAlpha(dbgSelected ? selected : notSelected);
    palette.setColor(QPalette::Button, color);
    toolButton->setPalette(palette);
    toolButton->setAutoFillBackground(true);

    toolButton = ctrlLogInfo();
    palette = toolButton->palette();
    color = LogScopeIconFactory::getColor(NELogging::eLogPriority::PrioInfo);
    color.setAlpha(infoSelected ? selected : notSelected);
    palette.setColor(QPalette::Button, color);
    toolButton->setPalette(palette);
    toolButton->setAutoFillBackground(true);

    toolButton = ctrlLogWarning();
    palette = toolButton->palette();
    color = LogScopeIconFactory::getColor(NELogging::eLogPriority::PrioWarning);
    color.setAlpha(warnSelected ? selected : notSelected);
    palette.setColor(QPalette::Button, color);
    toolButton->setPalette(palette);
    toolButton->setAutoFillBackground(true);

    toolButton = ctrlLogError();
    palette = toolButton->palette();
    color = LogScopeIconFactory::getColor(NELogging::eLogPriority::PrioError);
    color.setAlpha(errSelected ? selected : notSelected);
    palette.setColor(QPalette::Button, color);
    toolButton->setPalette(palette);
    toolButton->setAutoFillBackground(true);

    toolButton = ctrlLogScopes();
    palette = toolButton->palette();
    color = LogScopeIconFactory::getColor(NELogging::eLogPriority::PrioScope);
    color.setAlpha(scopeSelected ? selected : notSelected);
    palette.setColor(QPalette::Button, color);
    toolButton->setPalette(palette);
    toolButton->setAutoFillBackground(true);

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
        disconnect(log, &LogObserver::signalLogObserverConfigured   , this, &LogExplorer::onLogObserverConfigured);
        disconnect(log, &LogObserver::signalLogDbConfigured         , this, &LogExplorer::onLogDbConfigured);
        disconnect(log, &LogObserver::signalLogServiceConnected     , this, &LogExplorer::onLogServiceConnected);
        disconnect(log, &LogObserver::signalLogObserverStarted      , this, &LogExplorer::onLogObserverStarted);
        disconnect(log, &LogObserver::signalLogDbCreated            , this, &LogExplorer::onLogDbCreated);
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
                ctrlLogWarning()->setChecked(true);
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
    
    mAddress = address;
    mPort = port;
}

void LogExplorer::onLogDbConfigured(bool isEnabled, const QString& dbName, const QString& dbLocation, const QString& dbUser)
{
    mInitLogFile    = dbName;
    mLogLocation    = dbLocation;
    
    if (isEnabled && mShouldConnect)
    {
        mModel = new LogScopesModel(ctrlTable());
        mSelModel = new QItemSelectionModel(mModel, ctrlTable());
        
        std::error_code err;
        std::filesystem::path dbPath(mLogLocation.toStdString());
        dbPath /= mInitLogFile.toStdString();
        QString logPath(std::filesystem::absolute(dbPath, err).c_str());
        LogObserver::connect(mAddress, mPort, logPath);
        
        connect(mModel      , &LogScopesModel::signalRootUpdated    , this, &LogExplorer::onRootUpdated);
        connect(mModel      , &LogScopesModel::signalScopesInserted , this, &LogExplorer::onScopesInserted);
        connect(mModel      , &LogScopesModel::signalScopesUpdated  , this, &LogExplorer::onScopesUpdated);
        connect(mModel      , &LogScopesModel::dataChanged          , this, &LogExplorer::onScopesDataChanged);
        connect(mSelModel   , &QItemSelectionModel::selectionChanged, this, &LogExplorer::onSelectionChanged);
    }
    
    mShouldConnect = false;
}

void LogExplorer::onLogServiceConnected(bool isConnected, const QString& address, uint16_t port)
{
    if (mModel != nullptr)
    {
        Q_ASSERT(mSelModel != nullptr);

        if (isConnected)
        {
            mModel->initialize();
            mSelModel->reset();
            ctrlTable()->setModel(mModel);
            ctrlTable()->setSelectionModel(mSelModel);
        }
        else
        {
            ctrlTable()->setSelectionModel(nullptr);
            ctrlTable()->setModel(nullptr);
            mSelModel->reset();
            mModel->release();
        }

        enableButtons(QModelIndex());
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

void LogExplorer::onPrioErrorClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    if (current.isValid())
    {
        Q_ASSERT(mModel != nullptr);
        if (checked)
        {
            mModel->setLogPriority(current, NELogging::eLogPriority::PrioError);
        }
        else
        {
            mModel->removeLogPriority(current, NELogging::eLogPriority::PrioError);
        }
    }
}

void LogExplorer::onPrioWarningClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    if (current.isValid())
    {
        Q_ASSERT(mModel != nullptr);
        if (checked)
        {
            mModel->setLogPriority(current, NELogging::eLogPriority::PrioWarning);
        }
        else
        {
            mModel->removeLogPriority(current, NELogging::eLogPriority::PrioWarning);
        }
    }
}

void LogExplorer::onPrioInfoClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    if (current.isValid())
    {
        Q_ASSERT(mModel != nullptr);
        if (checked)
        {
            mModel->setLogPriority(current, NELogging::eLogPriority::PrioInfo);
        }
        else
        {
            mModel->removeLogPriority(current, NELogging::eLogPriority::PrioInfo);
        }
    }
}

void LogExplorer::onPrioDebugClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    if (current.isValid())
    {
        Q_ASSERT(mModel != nullptr);
        if (checked)
        {
            mModel->setLogPriority(current, NELogging::eLogPriority::PrioDebug);
        }
        else
        {
            mModel->removeLogPriority(current, NELogging::eLogPriority::PrioDebug);
        }
    }
}

void LogExplorer::onPrioScopesClicked(bool checked)
{
    QModelIndex current = ctrlTable()->currentIndex();
    if (current.isValid())
    {
        Q_ASSERT(mModel != nullptr);
        if (checked)
        {
            ScopeNodeBase* node = static_cast<ScopeNodeBase *>(current.internalPointer());
            if (node->hasPrioNotset())
            {
                checked = mModel->setLogPriority(current, NELogging::eLogPriority::PrioScope);
            }
            else
            {
                checked = mModel->addLogPriority(current, NELogging::eLogPriority::PrioScope);
            }
        }
        else
        {
            checked = mModel->removeLogPriority(current, NELogging::eLogPriority::PrioScope) == false;
        }
        
        ctrlLogScopes()->setChecked(checked);
    }
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
        QModelIndex current = ctrlTable()->currentIndex();        
        enableButtons(current);
        updateExpanded(current);
    }
}
