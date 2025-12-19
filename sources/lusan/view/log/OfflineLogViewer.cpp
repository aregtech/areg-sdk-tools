/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/log/OfflineLogViewer.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, offline log viewer widget.
 *
 ************************************************************************/

#include "lusan/view/log/OfflineLogViewer.hpp"
#include "ui/ui_OfflineLogViewer.h"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/NaviOfflineLogsScopes.hpp"
#include "lusan/view/log/LiveLogViewer.hpp"
#include "lusan/view/log/LogTableHeader.hpp"

#include "lusan/model/log/OfflineLogsModel.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"

#include <QFileInfo>
#include <QTableView>
#include <QLabel>
#include <QMdiSubWindow>

OfflineLogViewer::OfflineLogViewer(MdiMainWindow *wndMain, QWidget *parent)
    : LogViewerBase (MdiChild::eMdiWindow::MdiOfflineLogViewer, nullptr, wndMain, parent)
    , ui            (new Ui::OfflineLogViewer)
{
    ui->setupUi(mMdiWindow);
    mLogModel   = new OfflineLogsModel(this);
    mLogTable   = ui->logView;
    mLogSearch  = ui->textSearch;
    
    setupWidgets();
    setupSignals(true);
}

OfflineLogViewer::OfflineLogViewer(MdiMainWindow* wndMain, LiveLogViewer& liveLogs, QWidget* parent)
    : LogViewerBase (MdiChild::eMdiWindow::MdiOfflineLogViewer, nullptr, wndMain, parent)
    , ui            (new Ui::OfflineLogViewer)
{
    ui->setupUi(mMdiWindow);
    mLogModel   = new OfflineLogsModel(this);
    mLogTable   = ui->logView;
    mLogSearch  = ui->textSearch;

    LoggingModelBase* liveModel = liveLogs.getLoggingModel();
    if (liveModel != nullptr)
    {
        mLogModel->dataTransfer(*liveModel);
        setCurrentFile(mLogModel->getDatabasePath());
    }
    
    setupWidgets();
    ctrlFile()->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);

    setupSignals(true);
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

OfflineLogViewer::~OfflineLogViewer(void)
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

void OfflineLogViewer::onWindowActivated(void)
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
    QFileInfo info(dbPath);
    QString fileName = info.fileName();
    
    ctrlFile()->setToolTip(dbPath);
    
    if (LusanApplication::isWorkpacePath(info.absoluteFilePath()) == false)
    {
        ctrlFile()->setText(QString("⚠️ %1").arg(fileName)); // Add space to avoid icon overwrite
        if (mMdiSubWindow != nullptr)
        {
            mMdiSubWindow->setWindowTitle(tr("Offline Logs - ⚠️ %1").arg(fileName));
        }
    }
    else
    {
        ctrlFile()->setText(fileName);    // Ensure text is set after clearing pixmap
        if (mMdiSubWindow != nullptr)
        {
            mMdiSubWindow->setWindowTitle(tr("Offline Logs - %1").arg(fileName));
        }
    }
}

void OfflineLogViewer::onDatabaseClosed(const QString& dbPath)
{
    Q_UNUSED(dbPath);
    ctrlFile()->setText("");
    ctrlFile()->setToolTip("");
    
    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(tr("Offline Logs"));
    }
}

QLabel* OfflineLogViewer::ctrlFile(void)
{
    return ui->labelFile;
}

void OfflineLogViewer::setupSignals(bool doSetup)
{
    Q_ASSERT(mLogModel != nullptr);
    
    OfflineLogsModel* logModel = static_cast<OfflineLogsModel *>(mLogModel);
    if (doSetup)
    {
        // Connect signals
        connect(logModel, &OfflineLogsModel::signalDatabaseIsOpened, this      , &OfflineLogViewer::onDatabaseOpened);
        connect(logModel, &OfflineLogsModel::signalDatabaseIsClosed, this      , &OfflineLogViewer::onDatabaseClosed);
    }
    else
    {
        // Disconnect signals
        disconnect(logModel, &OfflineLogsModel::signalDatabaseIsOpened, this      , &OfflineLogViewer::onDatabaseOpened);
        disconnect(logModel, &OfflineLogsModel::signalDatabaseIsClosed, this      , &OfflineLogViewer::onDatabaseClosed);
    }
}

void OfflineLogViewer::cleanResources(void)
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
