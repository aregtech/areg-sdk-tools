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

#include "areglogger/client/LogObserverApi.h"

#include <QLocale>
#include <QTableView>
#include <QTimer>
#include <QToolButton>
#include <QMdiSubWindow>

const QString   LiveLogViewer::_tooltipPauseLogging     (tr("Pause logging. The database stays open."));
const QString   LiveLogViewer::_tooltipResumeLogging    (tr("Resume logging into the same database."));
const QString   LiveLogViewer::_tooltipStopLogging      (tr("Stop logging and close the database. The connection stays."));
const QString   LiveLogViewer::_tooltipRestartLogging   (tr("Resume logging in a new database."));

LiveLogViewer::LiveLogViewer(MdiMainWindow *wndMain, QWidget *parent)
    : LogViewerBase (MdiChild::eMdiWindow::MdiLogViewer, nullptr, wndMain, parent)
    , mReleased     (false)
    , mFollowTimer  (new QTimer(this))
    , mSaveTimer    (new QTimer(this))
    , mSkippedLogs  (0u)
    , mDroppedLogs  (0u)
{
    mSaveTimer->setInterval(LiveLogViewer::SAVE_CHECK_MS);
    connect(mSaveTimer, &QTimer::timeout, this, &LiveLogViewer::onCheckSaveQueue);

    mFollowTimer->setSingleShot(true);
    mFollowTimer->setInterval(LiveLogViewer::FOLLOW_SCROLL_MS);
    connect(mFollowTimer, &QTimer::timeout, this, [this]() {
                if ((mReleased == false) && getSessionBar()->isFollowing())
                {
                    scrollFollowing();
                }
            });

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
        mSaveTimer->start();
    }
    else if (mMdiSubWindow != nullptr)
    {
        mSaveTimer->stop();
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

    if (getSessionBar()->isFollowing() && (mFollowTimer->isActive() == false))
    {
        mFollowTimer->start();
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
    mSkippedLogs = 0u;
    getSessionBar()->hideNotice(LogSessionBar::eNotice::NoticeSkipped);
}

void LiveLogViewer::onLogsSkipped(uint32_t count)
{
    mSkippedLogs += count;
    getSessionBar()->showNotice(LogSessionBar::eNotice::NoticeSkipped
                              , tr("%1 logs are not in this window, it could not draw them as fast as they arrived. The log file has all of them.")
                                    .arg(QLocale().toString(static_cast<qulonglong>(mSkippedLogs))));
}

void LiveLogViewer::onCheckSaveQueue()
{
    LogQueueState state{};
    if ((mReleased == false) && LogObserver::queryQueueState(state))
    {
        if (state.lqDropped < mDroppedLogs)
        {
            mDroppedLogs = 0u;
        }

        if (state.lqDropped > mDroppedLogs)
        {
            mDroppedLogs = state.lqDropped;
            getSessionBar()->showNotice(LogSessionBar::eNotice::NoticeDropped
                                      , tr("%1 logs were not saved, the log file could not keep up with the rate they arrived at.")
                                            .arg(QLocale().toString(static_cast<qulonglong>(mDroppedLogs))));
        }
    }
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
        connect(static_cast<LiveLogsModel*>(mLogModel), &LiveLogsModel::signalLogsSkipped, this, &LiveLogViewer::onLogsSkipped);
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
        disconnect(static_cast<LiveLogsModel*>(mLogModel), &LiveLogsModel::signalLogsSkipped, this, &LiveLogViewer::onLogsSkipped);
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
    mFollowTimer->stop();
    mSaveTimer->stop();
    disconnect(mFilter, nullptr, this, nullptr);
    disconnect(mLogModel, nullptr, this, nullptr);

    QTableView* view = ctrlTable();
    view->setModel(nullptr);
    view->setHorizontalHeader(nullptr);
    mFilter->setSourceModel(nullptr);
    mSearch.setLogModel(nullptr);

    // The read connection closes before the log observer closes the log file.
    mLogModel->closeDatabase();
    LogObserver::releaseLogObserver();

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
