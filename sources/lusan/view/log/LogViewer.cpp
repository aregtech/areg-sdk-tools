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

#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/model/log/LogViewerModel.hpp"
#include <QMdiSubWindow>
#include <QMenu>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>

const QString   LogViewer::_tooltipPauseLogging(tr("Pause currnet logging"));
const QString   LogViewer::_tooltipResumeLogging(tr("Resume current logging"));
const QString   LogViewer::_tooltipStopLogging(tr("Stop current logging"));
const QString   LogViewer::_tooltipRestartLogging(tr("Restart logging in new database"));

LogViewer::LogViewer(MdiMainWindow *wndMain, QWidget *parent)
    : MdiChild      (MdiChild::eMdiWindow::MdiLogViewer, wndMain, parent)

    , ui            (new Ui::LogViewer)
    , mLogModel     (nullptr)
    , mMdiWindow    (new QWidget())
    , mLastSearchText()
    , mLastFoundRow(-1)
    , mCaseSensitiveSearch(false)
{
    ui->setupUi(mMdiWindow);
    mLogModel = new LogViewerModel(this);
    
    QTableView* view = ctrlTable();
    view->setHorizontalHeader(new LogTableHeader(this, view, mLogModel));
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

    view->setModel(mLogModel);

    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mMdiWindow);
    setLayout(layout);
    
    setAttribute(Qt::WA_DeleteOnClose);
    ctrlTable()->setAutoScroll(true);
    ctrlFile()->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);

    // Setup search field context menu
    ctrlSearchText()->setContextMenuPolicy(Qt::CustomContextMenu);

    updateToolbuttons(false, false);
    ctrlPause()->setEnabled(false);
    ctrlStop()->setEnabled(false);

    connect(ctrlPause()     , &QToolButton::clicked                     , this, &LogViewer::onPauseClicked);
    connect(ctrlStop()      , &QToolButton::clicked                     , this, &LogViewer::onStopClicked);
    connect(ctrlClear()     , &QToolButton::clicked                     , this, &LogViewer::onClearClicked);
    connect(ctrlSearchButton(), &QPushButton::clicked                   , this, &LogViewer::onSearchClicked);
    connect(ctrlSearchText(), &QLineEdit::textChanged                   , this, &LogViewer::onSearchTextChanged);
    connect(ctrlSearchText(), &QLineEdit::returnPressed                 , this, &LogViewer::onSearchClicked);
    connect(ctrlSearchText(), &QLineEdit::customContextMenuRequested    , this, &LogViewer::onSearchTextContextMenu);
    connect(mLogModel       , &LogViewerModel::rowsInserted             , this, &LogViewer::onRowsInserted);
    connect(mLogModel       , &LogViewerModel::columnsInserted          , this, &LogViewer::onColumnsInserted);
    connect(mLogModel       , &LogViewerModel::columnsRemoved           , this, &LogViewer::onColumnsRemoved);
    connect(mLogModel       , &LogViewerModel::columnsMoved             , this, &LogViewer::onColumnsMoved);
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
        ctrlFile()->setToolTip(dbPath);
        mMdiSubWindow->setWindowTitle(mLogModel->getLogFileName());
        updateToolbuttons(false, false);
        ctrlPause()->setEnabled(true);
        ctrlStop()->setEnabled(true);
    }
    else if (mMdiSubWindow != nullptr)
    {
        Q_ASSERT(mLogModel->getDabasePath() == dbPath);
        updateToolbuttons(false, false);
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
        ctrlFile()->setToolTip(dbPath);
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
        updateToolbuttons(false, false);
        ctrlPause()->setEnabled(false);
        ctrlStop()->setEnabled(false);
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

void LogViewer::onColumnsInserted(const QModelIndex& parent, int first, int last)
{
}

void LogViewer::onColumnsRemoved(const QModelIndex& parent, int first, int last)
{
}

void LogViewer::onColumnsMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd, const QModelIndex& destinationParent, int destinationColumn)
{
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

QToolButton* LogViewer::ctrlStop(void)
{
    return ui->toolStop;
}

QToolButton* LogViewer::ctrlClear(void)
{
    return ui->toolClear;
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

void LogViewer::updateToolbuttons(bool isPaused, bool isStopped)
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

void LogViewer::onPauseClicked(bool checked)
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

void LogViewer::onStopClicked(bool checked)
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

void LogViewer::onClearClicked(void)
{
    if (mLogModel != nullptr)
    {
        mLogModel->dataReset();
    }
    
    // Clear search state when logs are cleared
    mLastSearchText.clear();
    mLastFoundRow = -1;
    ctrlSearchText()->clear();
    ctrlSearchText()->setStyleSheet(""); // Reset any background color
}

QLineEdit* LogViewer::ctrlSearchText(void)
{
    return ui->textSearch;
}

QPushButton* LogViewer::ctrlSearchButton(void)
{
    return ui->buttonSearch;
}

void LogViewer::onSearchClicked(void)
{
    QString searchText = ctrlSearchText()->text().trimmed();
    if (searchText.isEmpty())
    {
        return;
    }
    
    // If this is a new search term, start from the beginning
    int startRow = 0;
    if (searchText == mLastSearchText && mLastFoundRow != -1)
    {
        // Continue searching from the next row after the last found match
        startRow = mLastFoundRow + 1;
    }
    
    int foundRow = performSearch(searchText, startRow, mCaseSensitiveSearch);
    
    if (foundRow != -1)
    {
        mLastSearchText = searchText;
        mLastFoundRow = foundRow;
        
        // Select and scroll to the found row
        QTableView* table = ctrlTable();
        QModelIndex foundIndex = mLogModel->index(foundRow, 0);
        table->setCurrentIndex(foundIndex);
        table->selectRow(foundRow);
        table->scrollTo(foundIndex, QAbstractItemView::PositionAtCenter);
        
        // Reset search field background to indicate successful search
        ctrlSearchText()->setStyleSheet("");
    }
    else
    {
        // If not found and we were continuing a search, try from the beginning
        if (startRow > 0)
        {
            foundRow = performSearch(searchText, 0, mCaseSensitiveSearch);
            if (foundRow != -1)
            {
                mLastSearchText = searchText;
                mLastFoundRow = foundRow;
                
                QTableView* table = ctrlTable();
                QModelIndex foundIndex = mLogModel->index(foundRow, 0);
                table->setCurrentIndex(foundIndex);
                table->selectRow(foundRow);
                table->scrollTo(foundIndex, QAbstractItemView::PositionAtCenter);
                
                // Reset search field background to indicate successful search
                ctrlSearchText()->setStyleSheet("");
                return;
            }
        }
        
        // No match found - highlight search field to indicate no results
        mLastSearchText = searchText;
        mLastFoundRow = -1;
        ctrlSearchText()->setStyleSheet("QLineEdit { background-color: #ffcccc; }");
    }
}

void LogViewer::onSearchTextChanged(const QString& text)
{
    // Reset search state when text changes
    if (text != mLastSearchText)
    {
        mLastFoundRow = -1;
        // Reset background color when user starts typing
        ctrlSearchText()->setStyleSheet("");
    }
}

int LogViewer::performSearch(const QString& searchText, int startFromRow, bool caseSensitive)
{
    if (mLogModel == nullptr || searchText.isEmpty())
    {
        return -1;
    }
    
    const int rowCount = mLogModel->rowCount();
    if (startFromRow >= rowCount)
    {
        return -1;
    }
    
    Qt::CaseSensitivity sensitivity = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    
    // Search through all rows starting from startFromRow
    for (int row = startFromRow; row < rowCount; ++row)
    {
        // Check all columns for the search text
        const int columnCount = mLogModel->columnCount();
        for (int col = 0; col < columnCount; ++col)
        {
            QModelIndex index = mLogModel->index(row, col);
            QString cellText = mLogModel->data(index, Qt::DisplayRole).toString();
            
            if (cellText.contains(searchText, sensitivity))
            {
                return row;
            }
        }
    }
    
    return -1; // Not found
}

void LogViewer::keyPressEvent(QKeyEvent* event)
{
    // Handle keyboard shortcuts for search functionality
    if (event->key() == Qt::Key_F && (event->modifiers() & Qt::ControlModifier))
    {
        // Ctrl+F: Focus on search field
        ctrlSearchText()->setFocus();
        ctrlSearchText()->selectAll();
        event->accept();
        return;
    }
    else if (event->key() == Qt::Key_F3)
    {
        // F3: Find next (same as clicking search button)
        if (!ctrlSearchText()->text().isEmpty())
        {
            onSearchClicked();
        }
        event->accept();
        return;
    }
    else if (event->key() == Qt::Key_Escape)
    {
        // Escape: Clear search field and focus table
        ctrlSearchText()->clear();
        ctrlTable()->setFocus();
        event->accept();
        return;
    }
    
    // Pass through to parent class
    MdiChild::keyPressEvent(event);
}

void LogViewer::onSearchTextContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    
    // Add standard context menu actions
    QMenu* standardMenu = ctrlSearchText()->createStandardContextMenu();
    if (standardMenu != nullptr)
    {
        menu.addActions(standardMenu->actions());
        menu.addSeparator();
        delete standardMenu;
    }
    
    // Add search-specific options
    QAction* caseSensitiveAction = menu.addAction(tr("Case Sensitive"));
    caseSensitiveAction->setCheckable(true);
    caseSensitiveAction->setChecked(mCaseSensitiveSearch);
    
    QAction* selectedAction = menu.exec(ctrlSearchText()->mapToGlobal(pos));
    
    if (selectedAction == caseSensitiveAction)
    {
        mCaseSensitiveSearch = caseSensitiveAction->isChecked();
        // Reset search state to re-search with new settings
        mLastFoundRow = -1;
    }
}

