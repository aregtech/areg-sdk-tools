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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/log/LogViewerBase.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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


const QString& LogViewerBase::fileExtension()
{
    return LoggingModelBase::getFileExtension();
}

LogViewerBase::LogViewerBase(MdiChild::eMdiWindow windowType, LoggingModelBase* logModel, MdiMainWindow* wndMain, QWidget* parent)
    : MdiChild(windowType, wndMain, parent)

    , mLogModel (logModel)
    , mFilter   (nullptr)
    , mLogTable (nullptr)
    , mLogSearch(nullptr)
    , mMdiWindow(new QWidget())
    , mHeader   (nullptr)
    , mSearch   (nullptr)
    , mFoundPos ()
    , mHighlight(nullptr)
    , mHighlightColumn(-1)
{
}

LogViewerBase::~LogViewerBase()
{
    _clearResources();
}

bool LogViewerBase::isDatabaseOpen() const
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

void LogViewerBase::setupWidgets()
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

    // Set the layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(mMdiWindow);
    setLayout(layout);
    setAttribute(Qt::WA_DeleteOnClose);

    mLogTable->setModel(mFilter);
    mLogTable->setAutoScroll(true);
    if (mHighlight == nullptr)
    {
        mHighlight = new LogTextHighlight(mFoundPos, mLogTable);
        mLogTable->setItemDelegate(mHighlight);
    }
    _updateHighlightColumn();

    connect(mFilter, &QAbstractItemModel::columnsInserted, this
            , [this](const QModelIndex&, int, int) {
                _updateHighlightColumn();
            });
    connect(mFilter, &QAbstractItemModel::columnsRemoved, this
            , [this](const QModelIndex&, int, int) {
                _updateHighlightColumn();
            });
    connect(mFilter, &QAbstractItemModel::modelReset, this
            , [this]() {
                _updateHighlightColumn();
            });
    
    QItemSelectionModel* selection= mLogTable->selectionModel();
    connect(mHeader     , &LogTableHeader::signalComboFilterChanged, this
            , [this](int column, const QList<NELusanCommon::FilterData>& items){
                _resetSearchResult();
                mFilter->setComboFilter(column, items);
            });
    connect(mHeader     , &LogTableHeader::signalTextFilterChanged, this
            , [this](int column, const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard) {
                _resetSearchResult();
                mFilter->setTextFilter(column, text, isCaseSensitive, isWholeWord, isWildCard);
            });
    connect(mHeader     , &LogTableHeader::customContextMenuRequested   , this, [this](const QPoint& pos)  {onHeaderContextMenu(pos);});
    
    connect(mLogTable   , &QTableView::clicked                          , this, [this](const QModelIndex &index){onMouseButtonClicked(index);});
    connect(mLogTable   , &QTableView::doubleClicked                    , this, [this](const QModelIndex &index){onMouseDoubleClicked(index);});
    
    connect(mLogSearch  , &SearchLineEdit::signalSearchTextChanged      , this, [this]() {mLogSearch->setStyleSheet(""); mSearch.resetSearch();});
    connect(mLogSearch  , &SearchLineEdit::signalSearchText             , this
            , [this](const QString& /*text*/, bool /*isMatchCase*/, bool /*isWholeWord*/, bool /*isWildCard*/, bool /*isBackward*/) {
                onSearchClicked(mSearch.canSearchNext() == false);
            });
    
    connect(selection   , &QItemSelectionModel::currentRowChanged       , this
            , [this](const QModelIndex &current, const QModelIndex &previous){onCurrentRowChanged(current, previous);});
    connect(shortcutSearch, &QShortcut::activated                       , this
            , [this]() {ctrlSearchText()->setFocus(); ctrlSearchText()->selectAll();});
}

void LogViewerBase::onWindowClosing(bool isActive)
{
    Q_UNUSED(isActive);
    ScopeOutputViewer& viewScope = mMainWindow->getOutputScopeLogs();
    viewScope.releaseWindow(*this);
}

bool LogViewerBase::saveFile(const QString& fileName)
{
    Q_ASSERT(mLogModel != nullptr);
    QString oldLocation{ mLogModel->getDatabasePath() };
    if (MdiChild::saveFile(fileName))
    {
        // do not change the file path
        setCurrentFile(oldLocation);
        return true;
    }
    else
    {
        return false;
    }
}

const QString& LogViewerBase::fileFilter() const
{
    static const QString _filterLogs{ "Log Files (*." + LoggingModelBase::getFileExtension() + ")\nAll Files(*.*)" };
    return _filterLogs;
}

bool LogViewerBase::writeToFile(const QString& filePath)
{
    bool result{ false };
    if (mLogModel != nullptr)
    {
        const QString oldLocation{ mLogModel->getDatabasePath() };
        if (oldLocation.isEmpty() == false)
        {
            result = areg::File::copy_file(oldLocation.toStdString().c_str(), filePath.toStdString().c_str(), true);
        }
        else
        {
            QMessageBox::warning(this, tr("Error"), tr("Cannot export logs to file: %1.").arg(filePath));
        }
    }

    return result;
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
        mFoundPos.colFound = mHighlightColumn >= 0 ? mHighlightColumn : 0;
        mLogSearch->setStyleSheet(QString());
        moveToRow(mFoundPos.rowFound, true);
        mLogSearch->update();
    }
    else
    {
        mFoundPos.colFound = static_cast<int32_t>(LogSearchModel::InvalidPos);
        mLogSearch->setStyleSheet(QString::fromUtf8("QLineEdit { background-color: #ffcccc; }"));
        mLogSearch->update();
    }

    if (mHighlight)
    {
        mLogTable->viewport()->update();
    }
}

QTableView* LogViewerBase::ctrlTable()
{
    return mLogTable;
}

LogTableHeader* LogViewerBase::ctrlHeader()
{
    return mHeader;
}

SearchLineEdit* LogViewerBase::ctrlSearchText()
{
    return mLogSearch;
}

QToolButton* LogViewerBase::ctrlButtonSearch()
{
    return mLogSearch->buttonSearch();
}

QToolButton* LogViewerBase::ctrlButtonCaseSensitive()
{
    return mLogSearch->buttonMatchCase();
}

QToolButton* LogViewerBase::ctrlButtonWholeWords()
{
    return mLogSearch->buttonMatchWord();
}

QToolButton* LogViewerBase::ctrlSearchWildcard()
{
    return mLogSearch->buttonWildCard();
}

QToolButton* LogViewerBase::ctrlSearchBackward()
{
    return mLogSearch->buttonSearchBackward();
}

void LogViewerBase::moveToBottom(bool select)
{
    Q_ASSERT(mLogTable != nullptr);
    mLogTable->scrollToBottom();
    if (select)
    {
        int count = mFilter->rowCount(QModelIndex());
        if (count > 0)
        {
            QModelIndex idxSelected = mFilter->index(count - 1, 0, QModelIndex());

            mLogTable->selectionModel()->setCurrentIndex(idxSelected, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            mLogTable->selectionModel()->select(idxSelected, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            mLogTable->selectRow(count - 1);
            mLogModel->selectBottom();
        }
    }
}

void LogViewerBase::moveToTop(bool select)
{
    Q_ASSERT(mLogTable != nullptr);
    mLogTable->scrollToTop();
    if (select)
    {
        int count = mFilter->rowCount(QModelIndex());
        if (count > 0)
        {
            QModelIndex idxSelected = mFilter->index(0, 0, QModelIndex());

            mLogTable->selectionModel()->setCurrentIndex(idxSelected, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            mLogTable->selectionModel()->select(idxSelected, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            mLogTable->selectRow(0);
            mLogModel->selectTop();
        }
    }
}

void LogViewerBase::moveToRow(int row, bool select)
{
    int count = mFilter->rowCount(QModelIndex());
    Q_ASSERT(mLogTable != nullptr);
    if ((row >= 0) && (count > 0) && (row < count))
    {
        int colMessage = mHeader != nullptr ? mHeader->getColumnIndex(LoggingModelBase::eColumn::LogColumnMessage) : 0;
        if (colMessage < 0)
            colMessage = 0;

        QModelIndex idxSelected = mFilter->index(row, colMessage, QModelIndex());
        mLogTable->scrollTo(idxSelected);
        if (select)
        {
            mLogTable->selectionModel()->setCurrentIndex(idxSelected, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            mLogTable->selectionModel()->select(idxSelected, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            mLogTable->selectRow(idxSelected.row());
            mLogModel->setSelectedLog(idxSelected);
        }
    }
}

void LogViewerBase::selectSourceElement(const QModelIndex & index)
{
    if (index.isValid() && (mFilter != nullptr) && (mLogModel != nullptr))
    {
        Q_ASSERT(mFilter != nullptr);
        if (_selectSourceLog(index) == false)
        {
            QMessageBox box(  QMessageBox::Question
                            , tr("Clear Filters?")
                            , tr("The log entry is not visible and cannot be select due to active filters.\nDo you want to clean filters and select the log?")
                            , QMessageBox::Yes | QMessageBox::No
                            , this);
            if (box.exec() == static_cast<int>(QMessageBox::Yes))
            {
                resetFilters();
                _selectSourceLog(index);
            }
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
    mLogTable->setModel(mFilter);
    _updateHighlightColumn();
}

void LogViewerBase::resetFilters()
{
    Q_ASSERT(mLogTable != nullptr);
    Q_ASSERT(mHeader != nullptr);
    mLogTable->setModel(nullptr);
    mHeader->resetFilters();
    mLogTable->setModel(mFilter);
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
    if (index.row() != static_cast<int>(mFoundPos.rowFound))
    {
        mSearch.resetSearch();
    }
}

void LogViewerBase::onMouseDoubleClicked(const QModelIndex& index)
{
    if (index.row() != static_cast<int>(mFoundPos.rowFound))
    {
        mSearch.resetSearch();
    }
    
    if (mMainWindow != nullptr)
    {
        ScopeOutputViewer & viewScope = mMainWindow->getOutputScopeLogs();
        viewScope.bindWindow(*this);
        viewScope.setupFilter(mLogModel, index);
    }
}

void LogViewerBase::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
}

inline void LogViewerBase::_clearResources()
{
    if (mFilter != nullptr)
    {
        disconnect(mFilter, nullptr, this, nullptr);
    }

    mSearch.setLogModel(nullptr);

    delete mMdiWindow;
    mMdiWindow = nullptr;
    mHeader = nullptr;
    mLogTable = nullptr;
    mLogSearch = nullptr;
    mHighlight = nullptr;
    mHighlightColumn = -1;

    delete mFilter;
    mFilter = nullptr;

    delete mLogModel;
    mLogModel = nullptr;
}

void LogViewerBase::_updateHighlightColumn()
{
    if ((mLogTable == nullptr) || (mHeader == nullptr))
        return;

    mHighlightColumn = mHeader->getColumnIndex(LoggingModelBase::eColumn::LogColumnMessage);
}

void LogViewerBase::_populateColumnsMenu(QMenu* menu, int curRow)
{
    // Get current active columns from the model
    const QList<LoggingModelBase::eColumn>& activeCols = mLogModel->getActiveColumns();
    const QStringList& headers{ LoggingModelBase::getHeaderList() };

    QAction* actResetFilters = menu->addAction(tr("Reset Filters"));
    actResetFilters->setCheckable(false);
    connect(actResetFilters, &QAction::triggered, this, [this, curRow]() {
            resetFilters();
            ctrlTable()->viewport()->update();
            moveToBottom(true);
        });

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

        connect(action, &QAction::triggered, this, [this, curRow, action](bool /*checked*/) {
                if (curRow < 0)
                    moveToBottom(false);
                if (action->isChecked())
                    mLogModel->addColumn(static_cast<LoggingModelBase::eColumn>(action->data().toInt()));
                else
                    mLogModel->removeColumn(static_cast<LoggingModelBase::eColumn>(action->data().toInt()));
            });
    }

    QAction* actResetColumns = menu->addAction(tr("Reset Columns"));
    actResetColumns->setCheckable(false);
    connect(actResetColumns, &QAction::triggered, this, [this, curRow]() {
            mLogModel->setActiveColumns(QList<LoggingModelBase::eColumn>());
            resetColumnOrder();
            ctrlTable()->viewport()->update();
            moveToBottom(true);
        });
}

inline void LogViewerBase::_resetSearchResult()
{
    mFoundPos = LogSearchModel::sFoundPos{};
    mSearch.resetSearch();
    mLogTable->viewport()->update();
}

inline bool LogViewerBase::_selectSourceLog(const QModelIndex& source)
{
    bool result {false};
    Q_ASSERT(source.isValid());
    if (mFilter != nullptr)
    {
        QModelIndex target = mFilter->mapFromSource(source);
        if (target.isValid())
        {
            _selectTargetLog(target);
            mLogModel->setSelectedLog(source);
            result = true;
        }
    }
    
    return result;
}

inline void LogViewerBase::_selectTargetLog(const QModelIndex& target)
{
    Q_ASSERT(target.isValid());
    mLogTable->selectionModel()->setCurrentIndex(target, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    mLogTable->selectionModel()->select(target, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    mLogTable->selectRow(target.row());
    mLogTable->scrollTo(target);
}
