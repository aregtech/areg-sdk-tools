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
 *  \file        lusan/view/log/OfflineLogViewer.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, offline log viewer widget.
 *
 ************************************************************************/

#include "lusan/view/log/OfflineLogViewer.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/NaviOfflineLogsScopes.hpp"
#include "lusan/view/log/LiveLogViewer.hpp"
#include "lusan/view/log/LogSessionBar.hpp"
#include "lusan/view/log/LogTableHeader.hpp"

#include "lusan/model/log/OfflineLogsModel.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"

#include <QFileInfo>
#include <QTableView>
#include <QToolButton>
#include <QMdiSubWindow>

OfflineLogViewer::OfflineLogViewer(MdiMainWindow *wndMain, QWidget *parent)
    : LogViewerBase (MdiChild::eMdiWindow::MdiOfflineLogViewer, nullptr, wndMain, parent)
    , mReleased     (false)
{
    mLogModel = new OfflineLogsModel(this);
    setupWidgets();
    setupSignals(true);
    onDatabaseClosed(QString());
}

OfflineLogViewer::OfflineLogViewer(MdiMainWindow* wndMain, LiveLogViewer& liveLogs, QWidget* parent)
    : LogViewerBase (MdiChild::eMdiWindow::MdiOfflineLogViewer, nullptr, wndMain, parent)
    , mReleased     (false)
{
    mLogModel = new OfflineLogsModel(this);

    LoggingModelBase* liveModel = liveLogs.getLoggingModel();
    if (liveModel != nullptr)
    {
        mLogModel->dataTransfer(*liveModel);
        setCurrentFile(mLogModel->getDatabasePath());
    }

    setupWidgets();
    setupSignals(true);

    const QString path{ mLogModel->getDatabasePath() };
    if (path.isEmpty())
    {
        onDatabaseClosed(QString());
    }
    else
    {
        onDatabaseOpened(path);
        updateSpan();
    }

    const QModelIndex idxSelected = mLogModel->getSelectedLog();
    if (idxSelected.isValid())
    {
        moveToRow(idxSelected.row(), true);
    }
    else
    {
        ctrlTable()->setCurrentIndex(idxSelected);
        ctrlTable()->scrollToTop();
    }
}

OfflineLogViewer::~OfflineLogViewer()
{
    cleanResources();
}

void OfflineLogViewer::onWindowClosing(bool isActive)
{
    Q_ASSERT(mMainWindow != nullptr);

    LogViewerBase::onWindowClosing(isActive);
    setupSignals(false);
    if (isActive)
    {
        mMainWindow->getNaviOfflineScopes().setLoggingModel(nullptr);
    }

    cleanResources();
}

void OfflineLogViewer::onWindowActivated()
{
    Q_ASSERT(mMainWindow != nullptr);
    if (mMainWindow->getNaviOfflineScopes().getLoggingModel() != mLogModel)
    {
        mMainWindow->getNaviOfflineScopes().setLoggingModel(nullptr);
        mMainWindow->getNaviOfflineScopes().setLoggingModel(static_cast<OfflineLogsModel *>(mLogModel));
    }

    mMainWindow->getNaviOfflineScopes().activateWindow();
}

void OfflineLogViewer::onDatabaseOpened(const QString& dbPath)
{
    const QFileInfo info(dbPath);
    const QString   fileName{ info.fileName() };
    const bool      inWorkspace{ LusanApplication::isWorkpacePath(info.absoluteFilePath()) };

    getSessionBar()->setArchive(fileName, dbPath, inWorkspace);
    getSessionBar()->ctrlReload()->setEnabled(true);
    getSessionBar()->ctrlClose()->setEnabled(true);

    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(inWorkspace ? tr("Offline Logs - %1").arg(fileName)
                                                  : tr("Offline Logs - [!] %1").arg(fileName));
    }
}

void OfflineLogViewer::onDatabaseClosed(const QString& dbPath)
{
    Q_UNUSED(dbPath);

    getSessionBar()->setArchive(QString(), QString(), true);
    getSessionBar()->setSpan(0, 0);
    getSessionBar()->ctrlReload()->setEnabled(false);
    getSessionBar()->ctrlClose()->setEnabled(false);

    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(tr("Offline Logs"));
    }
}

void OfflineLogViewer::onReloadClicked()
{
    Q_ASSERT(mLogModel != nullptr);
    const QString path{ mLogModel->getDatabasePath() };
    if (path.isEmpty())
        return;

    if (openDatabase(path))
    {
        resetFilters();
        onWindowActivated();
    }
}

void OfflineLogViewer::onCloseClicked()
{
    Q_ASSERT(mLogModel != nullptr);
    mMainWindow->getNaviOfflineScopes().setLoggingModel(nullptr);
    mLogModel->closeDatabase();
    setCurrentFile(QString());
}

void OfflineLogViewer::updateSpan()
{
    Q_ASSERT(mLogModel != nullptr);
    const int rows{ mLogModel->rowCount(QModelIndex()) };
    if (rows <= 0)
    {
        getSessionBar()->setSpan(0, 0);
        return;
    }

    const areg::LogEntry* first{ mLogModel->getLogData(0) };
    const areg::LogEntry* last { mLogModel->getLogData(rows - 1) };
    if ((first != nullptr) && (last != nullptr))
    {
        getSessionBar()->setSpan(first->logTimestamp, last->logTimestamp);
    }
}

void OfflineLogViewer::setupSignals(bool doSetup)
{
    Q_ASSERT(mLogModel != nullptr);

    OfflineLogsModel* logModel = static_cast<OfflineLogsModel *>(mLogModel);
    if (doSetup)
    {
        connect(logModel, &OfflineLogsModel::signalDatabaseIsOpened, this, &OfflineLogViewer::onDatabaseOpened);
        connect(logModel, &OfflineLogsModel::signalDatabaseIsClosed, this, &OfflineLogViewer::onDatabaseClosed);
        connect(logModel, &QAbstractItemModel::rowsInserted        , this, &OfflineLogViewer::updateSpan);
        connect(logModel, &QAbstractItemModel::modelReset          , this, &OfflineLogViewer::updateSpan);
        connect(getSessionBar()->ctrlReload(), &QToolButton::clicked, this, &OfflineLogViewer::onReloadClicked);
        connect(getSessionBar()->ctrlClose() , &QToolButton::clicked, this, &OfflineLogViewer::onCloseClicked);
    }
    else
    {
        disconnect(logModel, nullptr, this, nullptr);
        if (mSessionBar != nullptr)
        {
            disconnect(getSessionBar()->ctrlReload(), &QToolButton::clicked, this, &OfflineLogViewer::onReloadClicked);
            disconnect(getSessionBar()->ctrlClose() , &QToolButton::clicked, this, &OfflineLogViewer::onCloseClicked);
        }
    }
}

void OfflineLogViewer::cleanResources()
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
