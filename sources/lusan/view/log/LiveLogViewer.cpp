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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/log/LiveLogViewer.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view widget.
 *
 ************************************************************************/

#include "lusan/view/log/LiveLogViewer.hpp"

#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/NaviLiveLogsScopes.hpp"
#include "lusan/view/log/LogSessionBar.hpp"
#include "lusan/data/log/LogObserver.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"

#include <QTableView>
#include <QToolButton>
#include <QMdiSubWindow>

const QString   LiveLogViewer::_tooltipPauseLogging     (tr("Pause logging. The database stays open."));
const QString   LiveLogViewer::_tooltipResumeLogging    (tr("Resume logging into the same database."));
const QString   LiveLogViewer::_tooltipStopLogging      (tr("Stop logging and close the database. The connection stays."));
const QString   LiveLogViewer::_tooltipRestartLogging   (tr("Resume logging in a new database."));

LiveLogViewer::LiveLogViewer(MdiMainWindow *wndMain, QWidget *parent)
    : LogViewerBase (MdiChild::eMdiWindow::MdiLogViewer, nullptr, wndMain, parent)
    , mReleased     (false)
{
    mLogModel = new LiveLogsModel(this);
    setupWidgets();

    updateToolbuttons(false, false);
    ctrlPause()->setEnabled(false);
    ctrlStop()->setEnabled(false);

    setupSignals(true);
}

LiveLogViewer::~LiveLogViewer()
{
    cleanResources();
}

void LiveLogViewer::logServiceConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath)
{
    if (mLogModel == nullptr)
        return;
    
    LiveLogsModel* logModel = static_cast<LiveLogsModel*>(mLogModel);
    logModel->serviceConnected(isConnected, address, port, dbPath);
    if (isConnected)
    {
        Q_ASSERT(mMdiSubWindow != nullptr);
        getSessionBar()->setDatabasePath(dbPath);
        mMdiSubWindow->setWindowTitle(logModel->getLogFileName());
        ctrlPause()->setEnabled(true);
        ctrlStop()->setEnabled(true);
        updateToolbuttons(false, false);
    }
    else if (mMdiSubWindow != nullptr)
    {
        Q_ASSERT(logModel->getDatabasePath() == dbPath);
        ctrlPause()->setEnabled(false);
        ctrlStop()->setEnabled(false);
        updateToolbuttons(false, false);
    }
}

void LiveLogViewer::logDatabaseCreated(const QString& dbPath)
{
    Q_ASSERT(mLogModel != nullptr);
    mLogModel->openDatabase(dbPath, true);
    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(mLogModel->getLogFileName());
        getSessionBar()->setDatabasePath(dbPath);
    }
}

bool LiveLogViewer::isServiceConnected() const
{
    Q_ASSERT(mLogModel != nullptr);
    return static_cast<LiveLogsModel *>(mLogModel)->isConnected();
}

bool LiveLogViewer::isSourceReady() const
{
    return isServiceConnected();
}

bool LiveLogViewer::isEmpty() const
{
    Q_ASSERT(mLogModel != nullptr);
    return mLogModel->isEmpty();
}

void LiveLogViewer::detachLiveLog()
{
    Q_ASSERT(mLogModel != nullptr);
    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(mLogModel->getLogFileName());
        ctrlPause()->setEnabled(false);
        ctrlStop()->setEnabled(false);
        updateToolbuttons(false, false);
    }
}

void LiveLogViewer::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);

    if (getSessionBar()->isFollowing())
    {
        scrollFollowing();
    }
}

void LiveLogViewer::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    
    Q_ASSERT(mLogModel != nullptr);
    mLogModel->setSelectedLog(current);
}

QToolButton* LiveLogViewer::ctrlPause()
{
    return getSessionBar()->ctrlPause();
}

QToolButton* LiveLogViewer::ctrlStop()
{
    return getSessionBar()->ctrlStop();
}

QToolButton* LiveLogViewer::ctrlClear()
{
    return getSessionBar()->ctrlClear();
}

void LiveLogViewer::updateToolbuttons(bool isPaused, bool isStopped)
{
    const QSignalBlocker blockPause(ctrlPause());
    const QSignalBlocker blockStop(ctrlStop());

    if (isPaused)
    {
        ctrlPause()->setChecked(true);
        ctrlPause()->setIcon(NELusanCommon::iconPlay(NELusanCommon::SizeBig));
        ctrlPause()->setToolTip(_tooltipResumeLogging);
    }
    else
    {
        ctrlPause()->setChecked(false);
        ctrlPause()->setIcon(NELusanCommon::iconPause(NELusanCommon::SizeBig));
        ctrlPause()->setToolTip(_tooltipPauseLogging);
    }

    if (isStopped)
    {
        ctrlStop()->setChecked(true);
        ctrlStop()->setIcon(NELusanCommon::iconRecord(NELusanCommon::SizeBig));
        ctrlStop()->setToolTip(_tooltipRestartLogging);
        ctrlPause()->setEnabled(false);
    }
    else
    {
        ctrlStop()->setChecked(false);
        ctrlStop()->setIcon(NELusanCommon::iconStop(NELusanCommon::SizeBig));
        ctrlStop()->setToolTip(_tooltipStopLogging);
    }

    LogSessionBar::eLiveState state{ LogSessionBar::eLiveState::StateDisconnected };
    if (isServiceConnected())
    {
        state = isStopped ? LogSessionBar::eLiveState::StateStopped
              : (isPaused ? LogSessionBar::eLiveState::StatePaused
                          : LogSessionBar::eLiveState::StateConnected);
    }

    LiveLogsModel* logModel{ static_cast<LiveLogsModel *>(mLogModel) };
    getSessionBar()->setLiveState(state, logModel->getLofServiceAddress(), logModel->getLogServicePort());
}

void LiveLogViewer::onPauseClicked(bool checked)
{
    Q_ASSERT(mLogModel != nullptr);
    LiveLogsModel  * logModel = static_cast<LiveLogsModel *>(mLogModel);
    if (checked)
    {
        logModel->pauseLogging();
        updateToolbuttons(true, false);
    }
    else
    {
        logModel->resumeLogging();
        updateToolbuttons(false, false);
    }
}

void LiveLogViewer::onStopClicked(bool checked)
{
    Q_ASSERT(mLogModel != nullptr);
    LiveLogsModel  * logModel = static_cast<LiveLogsModel *>(mLogModel);
    if (checked)
    {
        logModel->stopLogging();
        updateToolbuttons(false, true);
    }
    else
    {
        logModel->restartLogging();
        ctrlPause()->setEnabled(true);
        updateToolbuttons(false, false);
    }
}

void LiveLogViewer::onClearClicked()
{
    Q_ASSERT(mLogModel != nullptr);
    mLogModel->dataReset();
}

QString LiveLogViewer::getDatabasePath() const
{
    Q_ASSERT(mLogModel != nullptr);
    return mLogModel->getDatabasePath();
}

void LiveLogViewer::onWindowClosing(bool isActive)
{
    Q_ASSERT(mMainWindow != nullptr);
    
    LogViewerBase::onWindowClosing(isActive);
    mMainWindow->getNaviLiveScopes().setLoggingModel(nullptr);
    if (mLogModel != nullptr)
    {
        setupSignals(false);
        cleanResources();
    }
}

void LiveLogViewer::setupSignals(bool doSetup)
{
    Q_ASSERT(mLogModel != nullptr);
    if (mSessionBar == nullptr)
        return;

    if (doSetup)
    {
        connect(mLogModel       , &LoggingModelBase::rowsInserted, this,&LiveLogViewer::onRowsInserted);
        connect(ctrlPause()     , &QToolButton::clicked         , this, &LiveLogViewer::onPauseClicked);
        connect(ctrlStop()      , &QToolButton::clicked         , this, &LiveLogViewer::onStopClicked);
        connect(ctrlClear()     , &QToolButton::clicked         , this, &LiveLogViewer::onClearClicked);
        connect(getSessionBar() , &LogSessionBar::signalDisconnectRequested, this, [this]() {
                    mMainWindow->getNaviLiveScopes().disconnectLogging();
                });
    }
    else
    {
        disconnect(mLogModel    , &LoggingModelBase::rowsInserted, this,&LiveLogViewer::onRowsInserted);
        disconnect(ctrlPause()  , &QToolButton::clicked         , this, &LiveLogViewer::onPauseClicked);
        disconnect(ctrlStop()   , &QToolButton::clicked         , this, &LiveLogViewer::onStopClicked);
        disconnect(ctrlClear()  , &QToolButton::clicked         , this, &LiveLogViewer::onClearClicked);
        disconnect(getSessionBar(), &LogSessionBar::signalDisconnectRequested, this, nullptr);
    }
}

void LiveLogViewer::cleanResources()
{
    if (mReleased)
    {
        Q_ASSERT(mLogModel == nullptr);
        Q_ASSERT(mFilter == nullptr);
        return;
    }

    Q_ASSERT(mLogModel != nullptr);
    Q_ASSERT(mFilter != nullptr);

    setupSignals(false);
    LogObserver::releaseLogObserver();
    disconnect(mFilter, nullptr, this, nullptr);
    disconnect(mLogModel, nullptr, this, nullptr);

    QTableView* view = ctrlTable();
    view->setModel(nullptr);
    view->setHorizontalHeader(nullptr);
    mFilter->setSourceModel(nullptr);
    mSearch.setLogModel(nullptr);
    mLogModel->closeDatabase();

    mReleased = true;

    delete mMdiWindow;
    mMdiWindow = nullptr;
    mHeader = nullptr;
    mLogTable = nullptr;
    mLogSearch = nullptr;
    mHighlight = nullptr;
    mSessionBar = nullptr;
    mEmptyState = nullptr;
    mHighlightColumn = -1;
    
    delete mFilter;
    mFilter = nullptr;
    
    delete mLogModel;
    mLogModel = nullptr;
}
