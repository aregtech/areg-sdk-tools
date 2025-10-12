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
 *  \file        lusan/view/log/LiveLogViewer.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view widget.
 *
 ************************************************************************/

#include "lusan/view/log/LiveLogViewer.hpp"
#include "ui/ui_LiveLogViewer.h"

#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/NaviLiveLogsScopes.hpp"
#include "lusan/data/log/LogObserver.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"

#include <QTableView>
#include <QLabel>
#include <QMdiSubWindow>

const QString   LiveLogViewer::_tooltipPauseLogging     (tr("Pause current logging"));
const QString   LiveLogViewer::_tooltipResumeLogging    (tr("Resume current logging"));
const QString   LiveLogViewer::_tooltipStopLogging      (tr("Stop current logging"));
const QString   LiveLogViewer::_tooltipRestartLogging   (tr("Restart logging in new database"));

LiveLogViewer::LiveLogViewer(MdiMainWindow *wndMain, QWidget *parent)
    : LogViewerBase (MdiChild::eMdiWindow::MdiLogViewer, nullptr, wndMain, parent)
    , ui            (new Ui::LiveLogViewer)
{
    ui->setupUi(mMdiWindow);
    mLogModel   = new LiveLogsModel(this);
    mLogTable   = ui->logView;
    mLogSearch  = ui->textSearch;
    
    setupWidgets();
    
    ctrlFile()->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
    updateToolbuttons(false, false);
    ctrlPause()->setEnabled(false);
    ctrlStop()->setEnabled(false);

    setupSignals(true);
}

LiveLogViewer::~LiveLogViewer(void)
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
        ctrlFile()->setText(dbPath);
        ctrlFile()->setToolTip(dbPath);
        mMdiSubWindow->setWindowTitle(logModel->getLogFileName());
        updateToolbuttons(false, false);
        ctrlPause()->setEnabled(true);
        ctrlStop()->setEnabled(true);
    }
    else if (mMdiSubWindow != nullptr)
    {
        Q_ASSERT(logModel->getDatabasePath() == dbPath);
        updateToolbuttons(false, false);
        ctrlPause()->setEnabled(false);
        ctrlStop()->setEnabled(false);
    }
}

void LiveLogViewer::logDatabaseCreated(const QString& dbPath)
{
    Q_ASSERT(mLogModel != nullptr);
    mLogModel->openDatabase(dbPath, true);
    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(mLogModel->getLogFileName());
        ctrlFile()->setText(dbPath);
        ctrlFile()->setToolTip(dbPath);
    }
}

bool LiveLogViewer::isServiceConnected(void) const
{
    Q_ASSERT(mLogModel != nullptr);
    return static_cast<LiveLogsModel *>(mLogModel)->isConnected();
}

bool LiveLogViewer::isEmpty(void) const
{
    Q_ASSERT(mLogModel != nullptr);
    return mLogModel->isEmpty();
}

void LiveLogViewer::detachLiveLog(void)
{
    Q_ASSERT(mLogModel != nullptr);
    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(mLogModel->getLogFileName());
        updateToolbuttons(false, false);
        ctrlPause()->setEnabled(false);
        ctrlStop()->setEnabled(false);
    }
}

void LiveLogViewer::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    const QModelIndex & idxSelected = mLogModel->getSelectedLog();
    int row = idxSelected.isValid() ? idxSelected.row() : -1;
    int count = mFilter->rowCount(parent);
    if ((row < 0) || (row >= count - 2))
    {
        ctrlTable()->scrollToBottom();
        if (row >= 0)
        {
            ctrlTable()->selectRow(count - 1);
        }
    }
}

void LiveLogViewer::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    
    Q_ASSERT(mLogModel != nullptr);
    mLogModel->setSelectedLog(current);
}

QToolButton* LiveLogViewer::ctrlPause(void)
{
    return ui->toolPause;
}

QToolButton* LiveLogViewer::ctrlStop(void)
{
    return ui->toolStop;
}

QToolButton* LiveLogViewer::ctrlClear(void)
{
    return ui->toolClear;
}

QLabel* LiveLogViewer::ctrlFile(void)
{
    return ui->labelFile;
}

void LiveLogViewer::updateToolbuttons(bool isPaused, bool isStopped)
{
    ctrlPause()->blockSignals(true);
    ctrlStop()->blockSignals(true);
    if (isPaused)
    {
        ctrlPause()->setEnabled(true);
        ctrlPause()->setChecked(true);
        ctrlPause()->setIcon(NELusanCommon::iconPlay(NELusanCommon::SizeBig));
        ctrlPause()->setToolTip(_tooltipResumeLogging);

        ctrlStop()->setEnabled(true);
        ctrlStop()->setChecked(false);
        ctrlStop()->setIcon(NELusanCommon::iconStop(NELusanCommon::SizeBig));
        ctrlStop()->setToolTip(_tooltipStopLogging);
    }
    else
    {
        ctrlPause()->setEnabled(true);
        ctrlPause()->setChecked(false);
        ctrlPause()->setIcon(NELusanCommon::iconPause(NELusanCommon::SizeBig));
        ctrlPause()->setToolTip(_tooltipPauseLogging);
    }

    if (isStopped)
    {
        ctrlStop()->setEnabled(true);
        ctrlStop()->setChecked(true);
        ctrlStop()->setIcon(NELusanCommon::iconRecord(NELusanCommon::SizeBig));
        ctrlStop()->setToolTip(_tooltipRestartLogging);

        ctrlPause()->setEnabled(false);
        ctrlPause()->setChecked(false);
        ctrlPause()->setIcon(NELusanCommon::iconPause(NELusanCommon::SizeBig));
        ctrlPause()->setToolTip(_tooltipPauseLogging);
    }
    else
    {
        ctrlStop()->setEnabled(true);
        ctrlStop()->setChecked(false);
        ctrlStop()->setIcon(NELusanCommon::iconStop(NELusanCommon::SizeBig));
        ctrlStop()->setToolTip(_tooltipStopLogging);
    }

    ctrlPause()->blockSignals(false);
    ctrlStop()->blockSignals(false);
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
        updateToolbuttons(false, false);
    }
}

void LiveLogViewer::onClearClicked(void)
{
    Q_ASSERT(mLogModel != nullptr);
    mLogModel->dataReset();
}

QString LiveLogViewer::getDatabasePath(void) const
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
    if (doSetup)
    {
        connect(mLogModel       , &LoggingModelBase::rowsInserted, this,&LiveLogViewer::onRowsInserted);
        connect(ctrlPause()     , &QToolButton::clicked         , this, &LiveLogViewer::onPauseClicked);
        connect(ctrlStop()      , &QToolButton::clicked         , this, &LiveLogViewer::onStopClicked);
        connect(ctrlClear()     , &QToolButton::clicked         , this, &LiveLogViewer::onClearClicked);
    }
    else
    {
        disconnect(mLogModel    , &LoggingModelBase::rowsInserted, this,&LiveLogViewer::onRowsInserted);
        disconnect(ctrlPause()  , &QToolButton::clicked         , this, &LiveLogViewer::onPauseClicked);
        disconnect(ctrlStop()   , &QToolButton::clicked         , this, &LiveLogViewer::onStopClicked);
        disconnect(ctrlClear()  , &QToolButton::clicked         , this, &LiveLogViewer::onClearClicked);
    }
}

void LiveLogViewer::cleanResources(void)
{
    if (ui == nullptr)
    {
        Q_ASSERT(mLogModel == nullptr);
        Q_ASSERT(mFilter == nullptr);
        return;
    }

    Q_ASSERT(mLogModel != nullptr);
    Q_ASSERT(mFilter != nullptr);

    setupSignals(false);
    LogObserver::releaseLogObserver();

    QTableView* view = ctrlTable();
    view->setModel(nullptr);
    view->setHorizontalHeader(nullptr);
    mFilter->setSourceModel(nullptr);
    mSearch.setLogModel(nullptr);
    mLogModel->closeDatabase();

    delete ui;
    ui = nullptr;
    
    delete mMdiWindow;
    mMdiWindow = nullptr;
    
    delete mFilter;
    mFilter = nullptr;
    
    delete mLogModel;
    mLogModel = nullptr;
}
