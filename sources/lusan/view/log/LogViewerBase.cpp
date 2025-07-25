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
 *  \file        lusan/view/log/LogViewerBase.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log viewer base widget.
 *
 ************************************************************************/

#include "lusan/view/log/LogViewerBase.hpp"
#include "lusan/view/common/SearchLineEdit.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/view/log/ScopeOutputViewer.hpp"

#include "lusan/model/log/LogViewerFilter.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/view/log/LogTextHighlight.hpp"

#include <QVBoxLayout>
#include <QKeyEvent>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPoint>
#include <QShortcut>
#include <QTableView>


const QString& LogViewerBase::fileExtension(void)
{
    return LoggingModelBase::getFileExtension();
}

LogViewerBase::LogViewerBase(MdiChild::eMdiWindow windowType, LoggingModelBase* logModel, MdiMainWindow* wndMain, QWidget* parent)
    : MdiChild(windowType, wndMain, parent)

    , mLogModel (logModel)
    , mFilter   ( )
    , mLogTable (nullptr)
    , mLogSearch(nullptr)
    , mMdiWindow(new QWidget())
    , mHeader   (nullptr)
    , mSearch   (nullptr)
    , mFoundPos ()
    , mHighlight(nullptr)
{
}

LogViewerBase::~LogViewerBase(void)
{
    _clearResources();
}

bool LogViewerBase::isDatabaseOpen(void) const
{
    Q_ASSERT(mLogModel != nullptr);
    return mLogModel->isOperable();
}

bool LogViewerBase::openDatabase(const QString& logPath)
{
    bool result{ false };
    mLogModel->closeDatabase();
    if (logPath.isEmpty() == false)
    {
        mLogModel->openDatabase(logPath, true);
        if (mLogModel->isOperable())
        {
            setCurrentFile(mLogModel->getDatabasePath());
            result = true;
        }
        else
        {
            QMessageBox::warning(this, tr("Error"), tr("Failed to open log database file: %1").arg(logPath));
        }
    }

    return result;
}

void LogViewerBase::keyPressEvent(QKeyEvent* event)
{
    // Handle keyboard shortcuts for search functionality
    QKeyCombination combi = event->keyCombination();
    // Check for Ctrl+F combination
    if ((event->key() == Qt::Key_F) && ((event->modifiers() & Qt::Modifier::CTRL) != 0))
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
        if (ctrlSearchText()->text().isEmpty() == false)
        {
            onSearchClicked(true);
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

void LogViewerBase::setupWidgets(void)
{
    Q_ASSERT((mLogTable != nullptr) && (mLogSearch != nullptr) && (mLogModel != nullptr));
    Q_ASSERT((mFilter == nullptr) && (mHeader == nullptr));

    QList<SearchLineEdit::eToolButton> tools;
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonSearch);
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonMatchCase);
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonMatchWord);
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonWildCard);
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonBackward);
    mLogSearch->initialize(tools, QSize(20, 20));

    mFilter = new LogViewerFilter(mLogModel);
    mHeader = new LogTableHeader(mLogTable, mLogModel);
    QShortcut* shortcutSearch = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    mSearch.setLogModel(mFilter);

    mLogTable->setHorizontalHeader(mHeader);
    mHeader->setVisible(true);
    mHeader->show();
    mHeader->setContextMenuPolicy(Qt::CustomContextMenu);
    mHeader->setSectionsMovable(true);

    mLogTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mLogTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mLogTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mLogTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mLogTable->setShowGrid(false);
    mLogTable->setCurrentIndex(QModelIndex());
    mLogTable->horizontalHeader()->setStretchLastSection(true);
    mLogTable->verticalHeader()->hide();
    mLogTable->setAutoScroll(true);
    mLogTable->setVerticalScrollMode(QTableView::ScrollPerItem);
    mLogTable->setContextMenuPolicy(Qt::CustomContextMenu);

    // In LogViewerBase constructor or setupWidgets()
    int index = mHeader->getColumnIndex(LoggingModelBase::eColumn::LogColumnMessage);
    if (index >= 0)
    {
        mHighlight = new LogTextHighlight(mFoundPos, mLogTable);
        mLogTable->setItemDelegateForColumn(index, mHighlight);
    }

    // Set the layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(mMdiWindow);
    setLayout(layout);
    setAttribute(Qt::WA_DeleteOnClose);

    mLogTable->setModel(mFilter);
    mLogTable->setAutoScroll(true);
    
    QItemSelectionModel* selection= mLogTable->selectionModel();
    connect(mHeader     , &LogTableHeader::signalComboFilterChanged
            , [this](int logicalColumn, const QStringList& items){
                _resetSearchResult(); mFilter->setComboFilter(logicalColumn, items);});
    connect(mHeader     , &LogTableHeader::signalTextFilterChanged
            , [this](int logicalColumn, const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard) {
                _resetSearchResult(); mFilter->setTextFilter(logicalColumn, text, isCaseSensitive, isWholeWord, isWildCard);});
    connect(mHeader     , &LogTableHeader::customContextMenuRequested   , [this](const QPoint& pos)  {onHeaderContextMenu(pos);});
    
    connect(mLogTable   , &QTableView::clicked                          , [this](const QModelIndex &index){onMouseButtonClicked(index);});
    connect(mLogTable   , &QTableView::doubleClicked                    , [this](const QModelIndex &index){onMouseDoubleClicked(index);});
    
    connect(mLogSearch  , &SearchLineEdit::signalSearchTextChanged      , [this]() {mLogSearch->setStyleSheet(""); mSearch.resetSearch();});
    connect(mLogSearch  , &SearchLineEdit::signalSearchText
            , [this](const QString& /*text*/, bool /*isMatchCase*/, bool /*isWholeWord*/, bool /*isWildCard*/, bool /*isBackward*/) {onSearchClicked(mSearch.canSearchNext() == false);});
    
    connect(selection   , &QItemSelectionModel::currentRowChanged       
            , [this](const QModelIndex &current, const QModelIndex &previous){onCurrentRowChanged(current, previous);});
    connect(shortcutSearch, &QShortcut::activated, this
            , [this]() {ctrlSearchText()->setFocus(); ctrlSearchText()->selectAll();});
}

void LogViewerBase::onSearchClicked(bool newSearch)
{
    Q_ASSERT(mLogSearch != nullptr);
    QString searchPhrase = mLogSearch->text();
    if (searchPhrase.isEmpty())
    {
        _resetSearchResult();
        return;
    }
    
    if (newSearch || (mSearch.isValidPosition(mFoundPos) == false))
    {
        QModelIndex idx{ctrlTable()->currentIndex()};
        int row = idx.isValid() ? idx.row() : 0;
        
        mFoundPos = mSearch.startSearch(  searchPhrase
                                        , static_cast<uint32_t>(row)
                                        , mLogSearch->isMatchCaseChecked()
                                        , mLogSearch->isMatchWordChecked()
                                        , mLogSearch->isWildCardChecked()
                                        , mLogSearch->isBackwardChecked());
    }
    else
    {
        mFoundPos = mSearch.nextSearch(mFoundPos.rowFound);
    }
    
    if (mSearch.isValidPosition(mFoundPos))
    {
        mLogSearch->setStyleSheet(QString());
        moveToRow(mFoundPos.rowFound, true);
        mLogSearch->update();
    }
    else
    {
        mLogSearch->setStyleSheet(QString::fromUtf8("QLineEdit { background-color: #ffcccc; }"));
        mLogSearch->update();
    }

    if (mHighlight)
    {
        mLogTable->viewport()->update();
    }
}

QTableView* LogViewerBase::ctrlTable(void)
{
    return mLogTable;
}

LogTableHeader* LogViewerBase::ctrlHeader(void)
{
    return mHeader;
}

SearchLineEdit* LogViewerBase::ctrlSearchText(void)
{
    return mLogSearch;
}

QToolButton* LogViewerBase::ctrlButtonSearch(void)
{
    return mLogSearch->buttonSearch();
}

QToolButton* LogViewerBase::ctrlButtonCaseSensitive(void)
{
    return mLogSearch->buttonMatchCase();
}

QToolButton* LogViewerBase::ctrlButtonWholeWords(void)
{
    return mLogSearch->buttonMatchWord();
}

QToolButton* LogViewerBase::ctrlSearchWildcard(void)
{
    return mLogSearch->buttonWildCard();
}

QToolButton* LogViewerBase::ctrlSearchBackward(void)
{
    return mLogSearch->buttonSearchBackward();
}

void LogViewerBase::moveToBottom(bool select)
{
    Q_ASSERT(mLogTable != nullptr);
    mLogTable->scrollToBottom();
    if (select)
    {
        int count = mLogModel->rowCount(QModelIndex());
        if (count > 0)
        {
            QModelIndex idxSelected = mLogModel->index(count - 1, 0, QModelIndex());

            mLogTable->selectionModel()->setCurrentIndex(idxSelected, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            mLogTable->selectRow(idxSelected.row());
            mLogTable->setCurrentIndex(idxSelected);
        }
    }
}

void LogViewerBase::moveToTop(bool select)
{
    Q_ASSERT(mLogTable != nullptr);
    mLogTable->scrollToTop();
    if (select)
    {
        int count = mLogModel->rowCount(QModelIndex());
        if (count > 0)
        {
            QModelIndex idxSelected = mLogModel->index(0, 0, QModelIndex());

            mLogTable->selectionModel()->setCurrentIndex(idxSelected, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            mLogTable->selectRow(idxSelected.row());
            mLogTable->setCurrentIndex(idxSelected);
        }
    }
}

void LogViewerBase::moveToRow(int row, bool select)
{
    int count = mLogModel->rowCount(QModelIndex());
    Q_ASSERT(mLogTable != nullptr);
    if ((row >= 0) && (count > 0) && (row < count))
    {
        QModelIndex idxSelected = mLogModel->index(row, 0, QModelIndex());
        mLogTable->scrollTo(idxSelected);
        if (select)
        {
            mLogTable->selectionModel()->setCurrentIndex(idxSelected, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            mLogTable->selectRow(idxSelected.row());
            // mLogTable->setCurrentIndex(idxSelected);
        }
    }
}

void LogViewerBase::resetColumnOrder()
{
    Q_ASSERT(mLogTable != nullptr);
    Q_ASSERT(mHeader != nullptr);

    // Force the view to update its columns to match the model
    mLogTable->setModel(nullptr);
    mLogModel->setActiveColumns(LoggingModelBase::getDefaultColumns());
    mHeader->resetFilters();
    mLogTable->setModel(mLogModel);

#if 0
    int columnCount = mHeader->count();
    // Restore to default order: 0, 1, 2, ..., N-1
    for (int logical = 0; logical < columnCount; ++logical)
    {
        int visual = mHeader->visualIndex(logical);
        if (visual != logical)
        {
            mHeader->moveSection(visual, logical);
        }
    }
#endif
}

void LogViewerBase::onHeaderContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QModelIndex idx{ mLogTable->currentIndex() };
    _populateColumnsMenu(&menu, idx.isValid() ? idx.row() : -1);
    menu.exec(mHeader->mapToGlobal(pos));
}

void LogViewerBase::onTableContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QMenu* columnsMenu = menu.addMenu(tr("Columns"));
    QModelIndex idx{ ctrlTable()->currentIndex() };
    _populateColumnsMenu(columnsMenu, idx.isValid() ? idx.row() : -1);
    menu.exec(ctrlTable()->viewport()->mapToGlobal(pos));
}

void LogViewerBase::onMouseButtonClicked(const QModelIndex& index)
{
    if (index.row() != mFoundPos.rowFound)
    {
        mSearch.resetSearch();
    }
}

void LogViewerBase::onMouseDoubleClicked(const QModelIndex& index)
{
    if (index.row() != mFoundPos.rowFound)
    {
        mSearch.resetSearch();
    }
    
    if (mMainWindow != nullptr)
    {
        ScopeOutputViewer & viewScope = mMainWindow->getOutputScopeLogs();
        viewScope.setupFilter(mLogModel, index);
    }
}

void LogViewerBase::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
}

void LogViewerBase::_clearResources(void)
{
    mSearch.setLogModel(nullptr);

    delete mMdiWindow;
    mMdiWindow = nullptr;

    delete mFilter;
    mFilter = nullptr;

    delete mLogModel;
    mLogModel = nullptr;
}

void LogViewerBase::_populateColumnsMenu(QMenu* menu, int curRow)
{
    // Get current active columns from the model
    const QList<LoggingModelBase::eColumn>& activeCols = mLogModel->getActiveColumns();
    const QStringList& headers{ LoggingModelBase::getHeaderList() };

    // Add actions for each available column
    for (int i = 0; i < static_cast<int>(LoggingModelBase::eColumn::LogColumnCount); ++i)
    {
        LoggingModelBase::eColumn col = static_cast<LoggingModelBase::eColumn>(i);
        if (col == LoggingModelBase::eColumn::LogColumnMessage)
            continue; // exclude "log message" menu entry.

        bool isVisible = activeCols.contains(col);
        QAction* action = menu->addAction(headers[i]);
        action->setCheckable(true);
        action->setChecked(isVisible);
        action->setData(i); // Store index for later

        connect(action, &QAction::triggered, [this, curRow, action](bool /*checked*/) {
                if (curRow < 0)
                    moveToBottom(false);
                if (action->isChecked())
                    mLogModel->addColumn(static_cast<LoggingModelBase::eColumn>(action->data().toInt()));
                else
                    mLogModel->removeColumn(static_cast<LoggingModelBase::eColumn>(action->data().toInt()));
            });
    }

    QAction* actReset = menu->addAction(tr("Reset Columns"));
    actReset->setCheckable(false);
    connect(actReset, &QAction::triggered, this, [this, curRow]() {
            mLogTable->scrollToBottom();
            mLogModel->setActiveColumns(QList<LiveLogsModel::eColumn>());
            resetColumnOrder();
        });
}

void LogViewerBase::_resetSearchResult(void)
{
    mFoundPos = LogSearchModel::sFoundPos{};
    mSearch.resetSearch();
    mLogTable->viewport()->update();
}
