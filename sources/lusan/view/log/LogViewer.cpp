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
 *  \file        lusan/view/log/LogViewer.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view widget.
 *
 ************************************************************************/

#include "lusan/view/log/LogViewer.hpp"
#include "ui/ui_LogViewer.h"

#include "lusan/model/log/LogViewerModel.hpp"
#include <QMdiSubWindow>
#include <QMenu>

LogViewer::LogViewer(MdiMainWindow *wndMain, QWidget *parent)
    : MdiChild      (MdiChild::eMdiWindow::MdiLogViewer, wndMain, parent)

    , ui            (new Ui::LogViewer)
    , mLogModel     (nullptr)
    , mMdiWindow    (new QWidget())
{
    ui->setupUi(mMdiWindow);
    
    mLogModel = new LogViewerModel(this);
    QHeaderView* header = ctrlHeader();
    header->setVisible(true);
    header->show();
    header->setContextMenuPolicy(Qt::CustomContextMenu);
    header->setSectionsMovable(true);

    QTableView* view = ctrlTable();    
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setShowGrid(false);
    view->setCurrentIndex(QModelIndex());
    view->horizontalHeader()->setStretchLastSection(true);
    view->verticalHeader()->hide();
    view->setAutoScroll(true);
    view->setVerticalScrollMode(QTableView::ScrollPerItem);
    view->setContextMenuPolicy(Qt::CustomContextMenu);

    view->setModel(mLogModel);

    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mMdiWindow);
    setLayout(layout);
    
    setAttribute(Qt::WA_DeleteOnClose);
    ctrlTable()->setAutoScroll(true);
    ctrlPause()->setEnabled(false);
    ctrlResume()->setEnabled(false);
    ctrlStop()->setEnabled(false);
    ctrlRestart()->setEnabled(false);
    ctrlFile()->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);

    connect(ctrlPause()     , &QToolButton::clicked                     , this, &LogViewer::onPauseClicked);
    connect(ctrlResume()    , &QToolButton::clicked                     , this, &LogViewer::onResumeClicked);
    connect(ctrlStop()      , &QToolButton::clicked                     , this, &LogViewer::onStopClicked);
    connect(ctrlRestart()   , &QToolButton::clicked                     , this, &LogViewer::onRestartClicked);
    connect(mLogModel       , &LogViewerModel::rowsInserted             , this, &LogViewer::onRowsInserted);
    connect(header          , &QHeaderView::customContextMenuRequested  , this, &LogViewer::onHeaderContextMenu);
    connect(view            , &QTableView::customContextMenuRequested   , this, &LogViewer::onTableContextMenu);
}

void LogViewer::logServiceConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath)
{
    Q_ASSERT(mLogModel != nullptr);
    mLogModel->serviceConnected(isConnected, address, port, dbPath);
    if (isConnected)
    {
        Q_ASSERT(mMdiSubWindow != nullptr);
        ctrlFile()->setText(dbPath);
        mMdiSubWindow->setWindowTitle(mLogModel->getLogFileName());
        ctrlPause()->setEnabled(true);
        ctrlStop()->setEnabled(true);
    }
    else if (mMdiSubWindow != nullptr)
    {
        Q_ASSERT(mLogModel->getDabasePath() == dbPath);
        ctrlPause()->setEnabled(false);
        ctrlStop()->setEnabled(false);
    }
}

void LogViewer::logDatabaseCreated(const QString& dbPath)
{
    Q_ASSERT(mLogModel != nullptr);
    mLogModel->setDatabasePath(dbPath);
    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(mLogModel->getLogFileName());
        ctrlFile()->setText(dbPath);
    }
}

bool LogViewer::isServiceConnected(void) const
{
    Q_ASSERT(mLogModel != nullptr);
    return mLogModel->isConnected();
}

void LogViewer::moveToBottom(bool lastSelect)
{
    QTableView* logs = ctrlTable();
    Q_ASSERT(logs != nullptr);
    logs->scrollToBottom();
    if (lastSelect)
    {
        int count = mLogModel->rowCount(QModelIndex());
        if (count > 0)
        {
            logs->selectRow(count - 1);
        }
    }
}

bool LogViewer::isEmpty(void) const
{
    Q_ASSERT(mLogModel != nullptr);
    return mLogModel->isEmpty();
}

void LogViewer::detachLiveLog(void)
{
    Q_ASSERT(mLogModel != nullptr);
    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(mLogModel->getLogFileName());
    }
}

void LogViewer::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    QModelIndex curIndex = ctrlTable()->currentIndex();
    int row = curIndex.isValid() ? curIndex.row() : -1;
    int count = mLogModel->rowCount(parent);
    if ((row < 0) || (row >= count - 2))
    {
        ctrlTable()->scrollToBottom();
        if (row >= 0)
        {
            ctrlTable()->selectRow(count - 1);
        }
    }
}

void LogViewer::onHeaderContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QModelIndex idx{ctrlTable()->currentIndex()};
    populateColumnsMenu(&menu, idx.isValid() ? idx.row() : -1);
    menu.exec(ctrlHeader()->mapToGlobal(pos));
}

void LogViewer::onTableContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QMenu* columnsMenu = menu.addMenu(tr("Columns"));
    QModelIndex idx{ctrlTable()->currentIndex()};    
    populateColumnsMenu(&menu, idx.isValid() ? idx.row() : -1);
    menu.exec(ctrlTable()->viewport()->mapToGlobal(pos));
}

QTableView* LogViewer::ctrlTable(void)
{
    return ui->logView;
}

QHeaderView* LogViewer::ctrlHeader(void)
{
    return ui->logView->horizontalHeader();
}

QToolButton* LogViewer::ctrlPause(void)
{
    return ui->toolPause;
}

QToolButton* LogViewer::ctrlResume(void)
{
    return ui->toolContinue;
}

QToolButton* LogViewer::ctrlStop(void)
{
    return ui->toolStop;
}

QToolButton* LogViewer::ctrlRestart(void)
{
    return ui->toolRestart;
}

QLabel* LogViewer::ctrlFile(void)
{
    return ui->lableFile;
}

void LogViewer::populateColumnsMenu(QMenu* menu, int curRow)
{
    // Get current active columns from the model
    const QList<LogViewerModel::eColumn>& activeCols = mLogModel->getActiveColumns();
    const QStringList& headers{ LogViewerModel::getHeaderList() };

    for (int i = 0; i < static_cast<int>(headers.size()); ++i)
    {
        LogViewerModel::eColumn col = static_cast<LogViewerModel::eColumn>(i);
        if (col == LogViewerModel::eColumn::LogColumnMessage)
            continue; // exclude "log message" menu entry.
        
        bool isVisible = activeCols.contains(col);

        QAction* action = menu->addAction(headers[i]);
        action->setCheckable(true);
        action->setChecked(isVisible);
        action->setData(i); // Store index for later

        connect(action, &QAction::triggered, this, [this, action, col, isVisible, curRow]() {
                    if (curRow < 0)
                        moveToBottom(false);
                    if (isVisible)
                        mLogModel->removeColumn(col);
                    else
                        mLogModel->addColumn(col);
                });
    }

    QAction* actReset = menu->addAction(tr("Reset Columns"));
    actReset->setCheckable(false);
    connect(actReset, &QAction::triggered, this, [this, curRow]() {
                ctrlTable()->scrollToBottom();
                mLogModel->setActiveColumns(QList<LogViewerModel::eColumn>());
                resetColumnOrder();
            });
}

void LogViewer::resetColumnOrder()
{
    // Force the view to update its columns to match the model
    ctrlTable()->setModel(nullptr);
    ctrlTable()->setModel(mLogModel);

    QHeaderView* header = ctrlHeader();
    int columnCount = header->count();
    // Restore to default order: 0, 1, 2, ..., N-1
    for (int logical = 0; logical < columnCount; ++logical)
    {
        int visual = header->visualIndex(logical);
        if (visual != logical)
        {
            header->moveSection(visual, logical);
        }
    }
}

void LogViewer::onPauseClicked()
{
    if (mLogModel != nullptr)
    {
        mLogModel->pauseLogging();
        ctrlPause()->setEnabled(false);
        ctrlResume()->setEnabled(true);
        ctrlRestart()->setEnabled(false);
        ctrlStop()->setEnabled(true);
    }
}

void LogViewer::onResumeClicked()
{
    if (mLogModel != nullptr)
    {
        mLogModel->resumeLogging();
        ctrlPause()->setEnabled(true);
        ctrlResume()->setEnabled(false);
        ctrlRestart()->setEnabled(false);
        ctrlStop()->setEnabled(true);
    }
}

void LogViewer::onStopClicked()
{
    if (mLogModel != nullptr)
    {
        mLogModel->stopLogging();
        ctrlPause()->setEnabled(false);
        ctrlResume()->setEnabled(false);
        ctrlRestart()->setEnabled(true);
        ctrlStop()->setEnabled(false);
    }
}

void LogViewer::onRestartClicked()
{
    if (mLogModel != nullptr)
    {
        mLogModel->restartLogging();
        ctrlPause()->setEnabled(true);
        ctrlResume()->setEnabled(false);
        ctrlRestart()->setEnabled(false);
        ctrlStop()->setEnabled(true);
    }
}
