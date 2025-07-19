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
#include "lusan/view/common/SearchLineEdit.hpp"
#include "lusan/view/log/LogTableHeader.hpp"

#include "lusan/data/log/LogObserver.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"
#include "lusan/model/log/LogViewerFilterProxy.hpp"

#include <QMdiSubWindow>
#include <QMenu>

const QString   LiveLogViewer::_tooltipPauseLogging(tr("Pause currnet logging"));
const QString   LiveLogViewer::_tooltipResumeLogging(tr("Resume current logging"));
const QString   LiveLogViewer::_tooltipStopLogging(tr("Stop current logging"));
const QString   LiveLogViewer::_tooltipRestartLogging(tr("Restart logging in new database"));

LiveLogViewer::LiveLogViewer(MdiMainWindow *wndMain, QWidget *parent)
    : MdiChild      (MdiChild::eMdiWindow::MdiLogViewer, wndMain, parent)

    , ui            (new Ui::LiveLogViewer)
    , mLogModel     (nullptr)
    , mFilter       (nullptr)
    , mMdiWindow    (new QWidget())
    , mHeader       (nullptr)
{
    QList<SearchLineEdit::eToolButton> tools;
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonSearch);
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonMatchCase);
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonMatchWord);
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonBackward);
    ui->setupUi(mMdiWindow);
    ctrlSearchText()->initialize(tools, QSize(20, 20));
    
    QTableView* view = ctrlTable();
    mLogModel   = new LiveLogsModel(this);
    mFilter     = new LogViewerFilterProxy(mLogModel);
    mHeader     = new LogTableHeader(view, mLogModel);
    
    view->setHorizontalHeader(mHeader);
    QHeaderView* header = ctrlHeader();    
    header->setVisible(true);
    header->show();
    header->setContextMenuPolicy(Qt::CustomContextMenu);
    header->setSectionsMovable(true);
    
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

    view->setModel(mFilter);

    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mMdiWindow);
    setLayout(layout);
    
    setAttribute(Qt::WA_DeleteOnClose);
    ctrlTable()->setAutoScroll(true);
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
    Q_ASSERT(mLogModel != nullptr);
    mLogModel->serviceConnected(isConnected, address, port, dbPath);
    if (isConnected)
    {
        Q_ASSERT(mMdiSubWindow != nullptr);
        ctrlFile()->setText(dbPath);
        ctrlFile()->setToolTip(dbPath);
        mMdiSubWindow->setWindowTitle(mLogModel->getLogFileName());
        updateToolbuttons(false, false);
        ctrlPause()->setEnabled(true);
        ctrlStop()->setEnabled(true);
    }
    else if (mMdiSubWindow != nullptr)
    {
        Q_ASSERT(mLogModel->getDatabasePath() == dbPath);
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
    return mLogModel->isConnected();
}

void LiveLogViewer::moveToBottom(bool lastSelect)
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

void LiveLogViewer::onColumnsInserted(const QModelIndex& parent, int first, int last)
{
}

void LiveLogViewer::onColumnsRemoved(const QModelIndex& parent, int first, int last)
{
}

void LiveLogViewer::onColumnsMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd, const QModelIndex& destinationParent, int destinationColumn)
{
}

void LiveLogViewer::onHeaderContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QModelIndex idx{ctrlTable()->currentIndex()};
    populateColumnsMenu(&menu, idx.isValid() ? idx.row() : -1);
    menu.exec(ctrlHeader()->mapToGlobal(pos));
}

void LiveLogViewer::onTableContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QMenu* columnsMenu = menu.addMenu(tr("Columns"));
    QModelIndex idx{ctrlTable()->currentIndex()};    
    populateColumnsMenu(columnsMenu, idx.isValid() ? idx.row() : -1);
    menu.exec(ctrlTable()->viewport()->mapToGlobal(pos));
}

void LiveLogViewer::onRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    
    Q_ASSERT(mLogModel != nullptr);
    mLogModel->setSelectedLog(current);
}

QTableView* LiveLogViewer::ctrlTable(void)
{
    return ui->logView;
}

QHeaderView* LiveLogViewer::ctrlHeader(void)
{
    return ui->logView->horizontalHeader();
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
    return ui->lableFile;
}

SearchLineEdit* LiveLogViewer::ctrlSearchText(void)
{
    return ui->textSearch;
}

QToolButton* LiveLogViewer::ctrlButtonSearch(void)
{
    return ui->textSearch->buttonSearch();
}

QToolButton* LiveLogViewer::ctrlButtonCaseSensitive(void)
{
    return ui->textSearch->buttonMatchCase();
}

QToolButton* LiveLogViewer::ctrlButtonWholeWords(void)
{
    return ui->textSearch->buttonMatchWord();
}

QToolButton* LiveLogViewer::ctrlSearchWildcard(void)
{
    return ui->textSearch->buttonWildCard();
}

QToolButton* LiveLogViewer::ctrlSearchBackward(void)
{
    return ui->textSearch->buttonSearchBackward();
}

void LiveLogViewer::populateColumnsMenu(QMenu* menu, int curRow)
{
    // Get current active columns from the model
    const QList<LiveLogsModel::eColumn>& activeCols = mLogModel->getActiveColumns();
    const QStringList& headers{ LiveLogsModel::getHeaderList() };

    for (int i = 0; i < static_cast<int>(headers.size()); ++i)
    {
        LiveLogsModel::eColumn col = static_cast<LiveLogsModel::eColumn>(i);
        if (col == LiveLogsModel::eColumn::LogColumnMessage)
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
                mLogModel->setActiveColumns(QList<LiveLogsModel::eColumn>());
                resetColumnOrder();
            });
}

void LiveLogViewer::resetColumnOrder()
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

void LiveLogViewer::updateToolbuttons(bool isPaused, bool isStopped)
{
    ctrlPause()->blockSignals(true);
    ctrlStop()->blockSignals(true);
    if (isPaused)
    {
        ctrlPause()->setEnabled(true);
        ctrlPause()->setChecked(true);
        ctrlPause()->setIcon(QIcon::fromTheme(QString::fromUtf8("media-playback-start")));
        ctrlPause()->setToolTip(_tooltipResumeLogging);

        ctrlStop()->setEnabled(true);
        ctrlStop()->setChecked(false);
        ctrlStop()->setIcon(QIcon::fromTheme(QString::fromUtf8("media-playback-stop")));
        ctrlStop()->setToolTip(_tooltipStopLogging);
    }
    else
    {
        ctrlPause()->setEnabled(true);
        ctrlPause()->setChecked(false);
        ctrlPause()->setIcon(QIcon::fromTheme(QString::fromUtf8("media-playback-pause")));
        ctrlPause()->setToolTip(_tooltipPauseLogging);
    }

    if (isStopped)
    {
        ctrlStop()->setEnabled(true);
        ctrlStop()->setChecked(true);
        ctrlStop()->setIcon(QIcon::fromTheme(QString::fromUtf8("media-record")));
        ctrlStop()->setToolTip(_tooltipRestartLogging);

        ctrlPause()->setEnabled(false);
        ctrlPause()->setChecked(false);
        ctrlPause()->setIcon(QIcon::fromTheme(QString::fromUtf8("media-playback-pause")));
        ctrlPause()->setToolTip(_tooltipPauseLogging);
    }
    else
    {
        ctrlStop()->setEnabled(true);
        ctrlStop()->setChecked(false);
        ctrlStop()->setIcon(QIcon::fromTheme(QString::fromUtf8("media-playback-stop")));
        ctrlStop()->setToolTip(_tooltipStopLogging);
    }

    ctrlPause()->blockSignals(false);
    ctrlStop()->blockSignals(false);
}

void LiveLogViewer::onPauseClicked(bool checked)
{
    if (mLogModel != nullptr)
    {
        if (checked)
        {
            mLogModel->pauseLogging();
            updateToolbuttons(true, false);
        }
        else
        {
            mLogModel->resumeLogging();
            updateToolbuttons(false, false);
        }
    }
    else
    {
        updateToolbuttons(false, false);
        ctrlPause()->setEnabled(false);
        ctrlStop()->setEnabled(false);
    }
}

void LiveLogViewer::onStopClicked(bool checked)
{
    if (mLogModel != nullptr)
    {
        if (checked)
        {
            mLogModel->stopLogging();
            updateToolbuttons(false, true);
        }
        else
        {
            mLogModel->restartLogging();
            updateToolbuttons(false, false);
        }
    }
    else
    {
        updateToolbuttons(false, false);
        ctrlPause()->setEnabled(false);
        ctrlStop()->setEnabled(false);
    }
}

void LiveLogViewer::onClearClicked(void)
{
    if (mLogModel != nullptr)
    {
        mLogModel->dataReset();
    }
}

void LiveLogViewer::onMainWindowClosing(void)
{
    setupSignals(false);
    if (mLogModel != nullptr)
    {
        mMainWindow->getNaviLiveScopes().setLoggingModel(nullptr);
    }

    cleanResources();
}

LiveLogsModel* LiveLogViewer::getLoggingModel(void) const
{
    return mLogModel;
}

QString LiveLogViewer::getDatabasePath(void) const
{
    return (mLogModel != nullptr ? mLogModel->getDatabasePath() : QString());
}

void LiveLogViewer::onWindowClosing(bool isActive)
{
    Q_UNUSED(isActive);
    Q_ASSERT(mMainWindow != nullptr);
    
    setupSignals(false);
    if (mLogModel != nullptr)
    {
        mMainWindow->getNaviLiveScopes().setLoggingModel(nullptr);
    }

    cleanResources();
}

void LiveLogViewer::onWindowActivated(void)
{
    Q_ASSERT(mMainWindow != nullptr);
    mMainWindow->getNaviLiveScopes().activateWindow();
}

void LiveLogViewer::setupSignals(bool doSetup)
{
    if (ui == nullptr)
    {
        Q_ASSERT(mLogModel == nullptr);
        Q_ASSERT(mFilter == nullptr);
        return;
    }

    QTableView* view        = ctrlTable();
    QItemSelectionModel* sel= view->selectionModel();
    MdiMainWindow* wndMain  = mMainWindow;
    Q_ASSERT(sel != nullptr);

    if (doSetup)
    {
        if (mLogModel != nullptr)
        {
            connect(mLogModel, &LiveLogsModel::rowsInserted     , this, &LiveLogViewer::onRowsInserted);
            connect(mLogModel, &LiveLogsModel::columnsInserted  , this, &LiveLogViewer::onColumnsInserted);
            connect(mLogModel, &LiveLogsModel::columnsRemoved   , this, &LiveLogViewer::onColumnsRemoved);
            connect(mLogModel, &LiveLogsModel::columnsMoved     , this, &LiveLogViewer::onColumnsMoved);
        }

        connect(ctrlPause() , &QToolButton::clicked                         , this      , &LiveLogViewer::onPauseClicked);
        connect(ctrlStop()  , &QToolButton::clicked                         , this      , &LiveLogViewer::onStopClicked);
        connect(ctrlClear() , &QToolButton::clicked                         , this      , &LiveLogViewer::onClearClicked);
        connect(view        , &QTableView::customContextMenuRequested       , this      , &LiveLogViewer::onTableContextMenu);
        connect(sel         , &QItemSelectionModel::currentRowChanged        , this     , &LiveLogViewer::onRowChanged);
        connect(mHeader     , &LogTableHeader::customContextMenuRequested   , this      , &LiveLogViewer::onHeaderContextMenu);
        connect(mHeader     , &LogTableHeader::signalComboFilterChanged     , mFilter   , &LogViewerFilterProxy::setComboFilter);
        connect(mHeader     , &LogTableHeader::signalTextFilterChanged      , mFilter   , &LogViewerFilterProxy::setTextFilter);
        connect(wndMain     , &MdiMainWindow::signalMainwindowClosing       , this      , &LiveLogViewer::onMainWindowClosing);
    }
    else
    {
        if (mLogModel != nullptr)
        {
            disconnect(mLogModel, &LiveLogsModel::rowsInserted, this, &LiveLogViewer::onRowsInserted);
            disconnect(mLogModel, &LiveLogsModel::columnsInserted, this, &LiveLogViewer::onColumnsInserted);
            disconnect(mLogModel, &LiveLogsModel::columnsRemoved, this, &LiveLogViewer::onColumnsRemoved);
            disconnect(mLogModel, &LiveLogsModel::columnsMoved, this, &LiveLogViewer::onColumnsMoved);
        }

        disconnect(ctrlPause()  , &QToolButton::clicked                         , this      , &LiveLogViewer::onPauseClicked);
        disconnect(ctrlStop()   , &QToolButton::clicked                         , this      , &LiveLogViewer::onStopClicked);
        disconnect(ctrlClear()  , &QToolButton::clicked                         , this      , &LiveLogViewer::onClearClicked);
        disconnect(view         , &QTableView::customContextMenuRequested       , this      , &LiveLogViewer::onTableContextMenu);
        disconnect(mHeader      , &LogTableHeader::customContextMenuRequested   , this      , &LiveLogViewer::onHeaderContextMenu);
        disconnect(mHeader      , &LogTableHeader::signalComboFilterChanged     , mFilter   , &LogViewerFilterProxy::setComboFilter);
        disconnect(mHeader      , &LogTableHeader::signalTextFilterChanged      , mFilter   , &LogViewerFilterProxy::setTextFilter);
        disconnect(wndMain      , &MdiMainWindow::signalMainwindowClosing       , this      , &LiveLogViewer::onMainWindowClosing);
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
