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
#include "lusan/view/log/LogEmptyState.hpp"
#include "lusan/view/log/LogFilterChips.hpp"
#include "lusan/view/log/LogHeaderItem.hpp"
#include "lusan/view/log/LogSessionBar.hpp"
#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/view/log/ScopeOutputViewer.hpp"

#include "lusan/model/log/LogViewerFilter.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/view/log/LogTextHighlight.hpp"

#include "areg/base/DateTime.hpp"
#include "areg/logging/LoggingDefs.hpp"

#include <QClipboard>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPoint>
#include <QScrollBar>
#include <QShortcut>
#include <QTableView>
#include <QTimer>
#include <QToolButton>

#include <algorithm>


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
    , mFoundRow (LogSearchModel::InvalidPos)
    , mHits     ( )
    , mHiddenHits(0)
    , mHighlight(nullptr)
    , mHighlightColumn(-1)
    , mSessionBar(nullptr)
    , mIsolationText( )
    , mCountTimer(nullptr)
    , mEmptyState(nullptr)
    , mSkew     ( )
    , mSkewShown(false)
    , mFollowScroll(false)
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
    Q_ASSERT(mLogModel != nullptr);
    Q_ASSERT((mLogTable == nullptr) && (mFilter == nullptr) && (mHeader == nullptr));

    const LogSessionBar::eSessionMode barMode
        { getMdiWindowType() == MdiChild::eMdiWindow::MdiLogViewer
        ? LogSessionBar::eSessionMode::ModeLive
        : LogSessionBar::eSessionMode::ModeOffline };

    mSessionBar = new LogSessionBar(barMode, mMdiWindow);
    mLogTable   = new QTableView(mMdiWindow);
    mLogSearch  = mSessionBar->ctrlSearch();
    mEmptyState = new LogEmptyState(mLogTable->viewport());
    mLogTable->viewport()->installEventFilter(this);
    connect(mEmptyState, &LogEmptyState::signalClearFilters, this, [this]() { clearEveryFilter(); });

    QVBoxLayout* stack = new QVBoxLayout(mMdiWindow);
    stack->setContentsMargins(0, 0, 0, 0);
    stack->setSpacing(0);
    stack->addWidget(mSessionBar);
    stack->addWidget(mLogTable, 1);

    mFilter = new LogViewerFilter(mLogModel);
    mHeader = new LogTableHeader(mLogTable, mLogModel);
    QShortcut* shortcutSearch = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    QShortcut* shortcutCopyMsg = new QShortcut(QKeySequence::Copy, this);
    QShortcut* shortcutCopyRow = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), this);
    mSearch.setLogModel(mFilter);

    mLogTable->setHorizontalHeader(mHeader);
    mHeader->setVisible(true);
    mHeader->show();
    mHeader->setContextMenuPolicy(Qt::CustomContextMenu);
    mHeader->setSectionsMovable(true);

    mLogTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mLogTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mLogTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mLogTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mLogTable->setShowGrid(false);
    mLogTable->setCurrentIndex(QModelIndex());
    mLogTable->horizontalHeader()->setStretchLastSection(true);
    mLogTable->verticalHeader()->hide();
    mLogTable->setAutoScroll(true);
    mLogTable->setVerticalScrollMode(QTableView::ScrollPerItem);
    mLogTable->setContextMenuPolicy(Qt::CustomContextMenu);

    // Log payloads are machine text: hex values, identifiers, aligned key=value pairs.
    // A fixed-width face puts the same character position in the same column on every
    // row, so a value that changed between two lines is visible without reading them.
    QFont fixed{ QFontDatabase::systemFont(QFontDatabase::FixedFont) };
    fixed.setPointSizeF(font().pointSizeF());
    mLogTable->setFont(fixed);

    mLogTable->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    mLogTable->setWordWrap(false);
    mLogTable->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
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
                _refitRowSelection();
            });
    connect(mFilter, &QAbstractItemModel::columnsRemoved, this
            , [this](const QModelIndex&, int, int) {
                _updateHighlightColumn();
                _refitRowSelection();
            });
    connect(mFilter, &QAbstractItemModel::modelReset, this
            , [this]() {
                _updateHighlightColumn();
            });
    connect(mFilter, &QAbstractItemModel::rowsRemoved, this
            , [this](const QModelIndex&, int, int) {
                // Rows above the hit are gone, the remembered position no longer names it.
                _resetSearchResult();
            });
    
    QItemSelectionModel* selection= mLogTable->selectionModel();
    connect(mHeader     , &LogTableHeader::signalComboFilterChanged, this
            , [this](int column, const QList<NELusanCommon::FilterData>& items){
                _resetSearchResult();
                mFilter->setComboFilter(column, items);
                _updateChips();
            });
    connect(mHeader     , &LogTableHeader::signalTextFilterChanged, this
            , [this](int column, const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard) {
                _resetSearchResult();
                mFilter->setTextFilter(column, text, isCaseSensitive, isWholeWord, isWildCard);
                _updateChips();
            });
    connect(mHeader     , &LogTableHeader::customContextMenuRequested   , this, [this](const QPoint& pos)  {onHeaderContextMenu(pos);});
    
    connect(mLogTable   , &QTableView::clicked                          , this, [this](const QModelIndex &index){onMouseButtonClicked(index);});
    connect(mLogTable   , &QTableView::doubleClicked                    , this, [this](const QModelIndex &index){onMouseDoubleClicked(index);});
    
    connect(mLogSearch  , &SearchLineEdit::signalSearchTextChanged      , this, [this](const QString& text) {
                mLogSearch->setStyleSheet("");
                mSearch.resetSearch();
                mSessionBar->ctrlFilterMatches()->setEnabled(text.isEmpty() == false);
            });
    connect(mLogSearch  , &SearchLineEdit::signalSearchText             , this
            , [this](const QString& /*text*/, bool /*isMatchCase*/, bool /*isWholeWord*/, bool /*isWildCard*/, bool /*isBackward*/) {
                onSearchClicked(mSearch.canSearchNext() == false);
            });
    
    connect(selection   , &QItemSelectionModel::currentRowChanged       , this
            , [this](const QModelIndex &current, const QModelIndex &previous){onCurrentRowChanged(current, previous);});
    connect(shortcutSearch, &QShortcut::activated                       , this
            , [this]() {ctrlSearchText()->setFocus(); ctrlSearchText()->selectAll();});
    connect(shortcutCopyMsg, &QShortcut::activated                      , this, &LogViewerBase::onCopyMessage);
    connect(shortcutCopyRow, &QShortcut::activated                      , this, &LogViewerBase::onCopyRow);

    connect(mSessionBar->ctrlFilterMatches(), &QToolButton::clicked, this, [this]() {
                filterToPhrase(NELusanCommon::FilterString{ mLogSearch->text()
                                                          , mLogSearch->isMatchCaseChecked()
                                                          , mLogSearch->isMatchWordChecked()
                                                          , mLogSearch->isWildCardChecked() });
            });

    connect(mSessionBar->ctrlSearchScope(), &QToolButton::toggled, this, [this](bool) {
                // A row number means a different row in each scope, so the walk starts again.
                _resetSearchResult();
            });

    connect(mSessionBar, &LogSessionBar::signalViewPriorityChanged, this, [this](uint16_t mask) {
                mFilter->setViewPriority(mask);
                _resetSearchResult();
                _updateChips();
                _updateCounters();
                _updateEmptyState();
            });

    connect(mSessionBar, &LogSessionBar::signalNoticeAction, this, [this](LogSessionBar::eNotice which) {
                if (which == LogSessionBar::eNotice::NoticeRevealed)
                {
                    clearEveryFilter();
                    mFilter->clearRevealedRows();
                    mSessionBar->hideNotice(LogSessionBar::eNotice::NoticeRevealed);
                }
            });

    LogFilterChips* chips{ mSessionBar->ctrlChips() };
    connect(chips, &LogFilterChips::signalChipDropped , this, &LogViewerBase::_dropChip);
    connect(chips, &LogFilterChips::signalClearAll    , this, &LogViewerBase::clearEveryFilter);
    connect(chips, &LogFilterChips::signalSearchInstead, this, [this](const LogFilterChips::sChip& chip) {
                _dropChip(chip);
                mLogSearch->setText(chip.phrase.text);
                mLogSearch->setFocus();
                onSearchClicked(true);
            });

    connect(mLogModel, &LoggingModelBase::signalRefusedScopesChanged, this, [this]() {
                _updateChips();
                _updateCounters();
            });

    connect(mSessionBar->ctrlMoveTop()   , &QToolButton::clicked, this, [this]() {
                mSessionBar->setFollowing(false);
                moveToTop(false);
            });
    connect(mSessionBar->ctrlMoveBottom(), &QToolButton::clicked, this, [this](bool checked) {
                // Unchecking means "stop holding the end", so the table stays where it is.
                if (checked || (mSessionBar->ctrlMoveBottom()->isCheckable() == false))
                {
                    scrollFollowing();
                }
            });

    connect(mLogTable->verticalScrollBar(), &QAbstractSlider::valueChanged, this, [this](int value) {
                QScrollBar* scroll{ mLogTable->verticalScrollBar() };
                if ((mFollowScroll == false) && (value < scroll->maximum()))
                {
                    mSessionBar->setFollowing(false);
                }
            });

    mCountTimer = new QTimer(this);
    mCountTimer->setSingleShot(true);
    mCountTimer->setInterval(LogViewerBase::COUNTER_DELAY_MS);
    connect(mCountTimer, &QTimer::timeout, this, &LogViewerBase::_updateCounters);

    const auto countLater = [this]() { if (mCountTimer->isActive() == false) mCountTimer->start(); };
    connect(mFilter  , &QAbstractItemModel::rowsInserted , this, countLater);
    connect(mFilter  , &QAbstractItemModel::rowsRemoved  , this, countLater);
    connect(mFilter  , &QAbstractItemModel::modelReset   , this, countLater);
    connect(mFilter  , &QAbstractItemModel::layoutChanged, this, countLater);
    connect(mLogModel, &QAbstractItemModel::rowsInserted , this, countLater);
    connect(mLogModel, &QAbstractItemModel::rowsRemoved  , this, countLater);
    connect(mLogModel, &QAbstractItemModel::modelReset   , this, countLater);

    connect(mLogModel, &QAbstractItemModel::rowsInserted , this, &LogViewerBase::onSourceRowsInserted);
    connect(mLogModel, &QAbstractItemModel::modelReset   , this, [this]() {
                mSkew.reset();
                mSkewShown = false;
                mSessionBar->hideNotice(LogSessionBar::eNotice::NoticeClockSkew);
            });

    _updateChips();
    _updateCounters();
}

bool LogViewerBase::eventFilter(QObject* watched, QEvent* event)
{
    if ((mEmptyState != nullptr) && (mLogTable != nullptr) && (watched == mLogTable->viewport())
        && (event->type() == QEvent::Type::Resize))
    {
        mEmptyState->setGeometry(mLogTable->viewport()->rect());
    }

    return MdiChild::eventFilter(watched, event);
}

bool LogViewerBase::isSourceReady() const
{
    return isDatabaseOpen();
}

void LogViewerBase::scrollFollowing()
{
    Q_ASSERT(mLogTable != nullptr);
    mFollowScroll = true;
    mLogTable->scrollToBottom();
    mFollowScroll = false;
}

void LogViewerBase::_updateCounters()
{
    if ((mSessionBar == nullptr) || (mFilter == nullptr) || (mLogModel == nullptr))
        return;

    mSessionBar->setCounters(mFilter->rowCount(QModelIndex()), mLogModel->rowCount(QModelIndex()));
    _updateEmptyState();
}

void LogViewerBase::_updateChips()
{
    if ((mSessionBar == nullptr) || (mFilter == nullptr) || (mLogModel == nullptr) || (mHeader == nullptr))
        return;

    LogFilterChips::ListChips chips;
    const LogViewerFilter::ListActiveFilters filters{ mFilter->activeFilters() };
    for (const LogViewerFilter::sActiveFilter& entry : filters)
    {
        const int index{ mHeader->getColumnIndex(static_cast<LoggingModelBase::eColumn>(entry.column)) };
        const QString name{ index >= 0 ? mLogModel->getHeaderName(index) : QString() };

        LogFilterChips::sChip chip;
        chip.kind   = LogFilterChips::eChipKind::ChipColumn;
        chip.column = entry.column;
        chip.label  = QString("%1: %2").arg(name, entry.text);
        chip.hint   = entry.isText ? tr("Only the rows whose %1 carries \"%2\" are shown").arg(name, entry.text)
                                   : tr("Only the rows whose %1 is one of: %2").arg(name, entry.text);
        chip.phrase = entry.isText ? entry.phrase : NELusanCommon::FilterString{ };
        chips.append(chip);
    }

    if (mFilter->hasIsolation())
    {
        LogFilterChips::sChip chip;
        chip.kind  = LogFilterChips::eChipKind::ChipIsolate;
        chip.label = tr("only %1").arg(mIsolationText);
        chip.hint  = tr("Every other process, thread and scope is kept out of this window.");
        chips.append(chip);
    }

    if (mFilter->viewPriority() != 0)
    {
        LogFilterChips::sChip chip;
        chip.kind  = LogFilterChips::eChipKind::ChipPriority;
        chip.label = mSessionBar->priorityFilterName();
        chip.hint  = tr("Only the rows of these priorities are drawn. The target keeps producing every one of them.");
        chips.append(chip);
    }

    if (mLogModel->hasRefusedScopes())
    {
        LogFilterChips::sChip chip;
        chip.kind  = LogFilterChips::eChipKind::ChipScopes;
        chip.label = tr("hidden scopes");
        chip.hint  = tr("The navigation tree is hiding scopes. Dropping this shows every scope again.");
        chips.append(chip);
    }

    mSessionBar->ctrlChips()->setChips(chips);
}

void LogViewerBase::_dropChip(const LogFilterChips::sChip& chip)
{
    if (chip.kind == LogFilterChips::eChipKind::ChipScopes)
    {
        mLogModel->requestShowAllScopes();
    }
    else if (chip.kind == LogFilterChips::eChipKind::ChipIsolate)
    {
        mFilter->clearIsolation();
        mIsolationText.clear();
    }
    else if (chip.kind == LogFilterChips::eChipKind::ChipPriority)
    {
        mSessionBar->resetPriorityFilter();
    }
    else if (chip.kind == LogFilterChips::eChipKind::ChipColumn)
    {
        const int index{ mHeader->getColumnIndex(static_cast<LoggingModelBase::eColumn>(chip.column)) };
        LogHeaderItem* item{ mHeader->getHeaderItem(index) };
        if (item != nullptr)
        {
            item->resetFilter();
        }
    }

    _updateChips();
    _updateCounters();
}

void LogViewerBase::filterToPhrase(const NELusanCommon::FilterString& phrase)
{
    if (mHeader == nullptr)
        return;

    const int index{ mHeader->getColumnIndex(LoggingModelBase::eColumn::LogColumnMessage) };
    LogHeaderItem* item{ mHeader->getHeaderItem(index) };
    if (item == nullptr)
        return;

    _resetSearchResult();
    item->setFilterData(phrase);
    _updateChips();
    _updateCounters();
}

void LogViewerBase::clearEveryFilter()
{
    resetFilters();
    mFilter->clearIsolation();
    mIsolationText.clear();
    mSessionBar->resetPriorityFilter();
    if (mLogModel->hasRefusedScopes())
    {
        mLogModel->requestShowAllScopes();
    }

    _updateChips();
    _updateCounters();
}

void LogViewerBase::_updateEmptyState()
{
    if ((mEmptyState == nullptr) || (mFilter == nullptr) || (mLogModel == nullptr))
        return;

    const int total{ mLogModel->rowCount(QModelIndex()) };
    const int shown{ mFilter->rowCount(QModelIndex()) };
    const bool live { getMdiWindowType() == MdiChild::eMdiWindow::MdiLogViewer };

    LogEmptyState::eEmptyReason reason{ LogEmptyState::eEmptyReason::ReasonNone };
    if (shown == 0)
    {
        if (total > 0)
            reason = LogEmptyState::eEmptyReason::ReasonFiltered;
        else if (isSourceReady() == false)
            reason = live ? LogEmptyState::eEmptyReason::ReasonNotConnected
                          : LogEmptyState::eEmptyReason::ReasonNoArchive;
        else
            reason = live ? LogEmptyState::eEmptyReason::ReasonNoLiveLogs
                          : LogEmptyState::eEmptyReason::ReasonEmptyArchive;
    }

    mEmptyState->setGeometry(mLogTable->viewport()->rect());
    mEmptyState->setReason(reason, total - shown, mLogModel->hasRefusedScopes(), mFilter->hasWindowFilters());
}

QString LogViewerBase::_formatOffset(qint64 offsetUs)
{
    if (offsetUs >= 1000000)
        return tr("%1 s").arg(QString::number(offsetUs / 1000000.0, 'f', 1));
    else
        return tr("%1 ms").arg(offsetUs / 1000);
}

void LogViewerBase::onSourceRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_ASSERT(mLogModel != nullptr);

    for (int row = first; row <= last; ++row)
    {
        const areg::LogEntry* entry{ mLogModel->getLogData(row) };
        if (entry != nullptr)
        {
            mSkew.feed(*entry);
        }
    }

    if (mSkew.hasSkew() && (mSkewShown == false))
    {
        mSkewShown = true;
        const LogClockSkew::sSkewReport& report{ mSkew.report() };
        mSessionBar->showNotice(LogSessionBar::eNotice::NoticeClockSkew
                               , tr("The clocks disagree: %1 stamps its logs %2 ahead of the collector, so its rows sort into an order that never happened.")
                                .arg(report.source, LogViewerBase::_formatOffset(report.offsetUs)));
    }
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

    // The scope decides which rows the search walks: the ones the table draws, or every row
    // the window holds. Changing it changes what a row number means, so the search restarts.
    const bool allLogs{ mSessionBar->isSearchingAllLogs() };
    QAbstractItemModel* searchIn{ allLogs ? static_cast<QAbstractItemModel*>(mLogModel)
                                          : static_cast<QAbstractItemModel*>(mFilter) };
    if (mSearch.getLogModel() != searchIn)
    {
        mSearch.setLogModel(searchIn);
        newSearch = true;
    }

    if (newSearch || (mSearch.isValidPosition(mFoundPos) == false))
    {
        const QModelIndex idx{ ctrlTable()->currentIndex() };
        uint32_t row{ 0 };
        if (idx.isValid())
        {
            row = allLogs ? static_cast<uint32_t>(mFilter->mapToSource(idx).row())
                          : static_cast<uint32_t>(idx.row());
        }

        mFoundPos = mSearch.startSearch(  searchPhrase
                                        , row
                                        , mLogSearch->isMatchCaseChecked()
                                        , mLogSearch->isMatchWordChecked()
                                        , mLogSearch->isWildCardChecked()
                                        , mLogSearch->isBackwardChecked());

        mHits       = mSearch.collectMatches();
        mHiddenHits = 0;
        if (allLogs)
        {
            for (uint32_t hit : mHits)
            {
                if (mFilter->mapFromSource(mLogModel->index(static_cast<int>(hit), 0)).isValid() == false)
                {
                    ++mHiddenHits;
                }
            }
        }
    }
    else
    {
        mFoundPos = mSearch.nextSearch(mFoundRow);
    }

    if (mSearch.isValidPosition(mFoundPos))
    {
        mFoundRow = mFoundPos.rowFound;
        mLogSearch->setStyleSheet(QString());
        _showSearchHit(allLogs);
    }
    else
    {
        mFoundRow = LogSearchModel::InvalidPos;
        mFoundPos.colFound = static_cast<int32_t>(LogSearchModel::InvalidPos);
        mLogSearch->setStyleSheet(QString::fromUtf8("QLineEdit { background-color: #ffcccc; }"));
    }

    _drawSearchState(allLogs);
    mLogSearch->update();

    if (mHighlight)
    {
        mLogTable->viewport()->update();
    }
}

void LogViewerBase::_showSearchHit(bool allLogs)
{
    int drawnRow{ static_cast<int>(mFoundRow) };
    if (allLogs)
    {
        const QModelIndex source{ mLogModel->index(static_cast<int>(mFoundRow), 0) };
        QModelIndex drawn{ mFilter->mapFromSource(source) };
        if (drawn.isValid() == false)
        {
            // The hit is on a row a filter keeps out. It is let through and marked, so it is
            // never mistaken for a row that passed the filters.
            mFilter->revealRow(source.row());
            drawn = mFilter->mapFromSource(source);
        }

        drawnRow = drawn.isValid() ? drawn.row() : -1;
    }

    mFoundPos.rowFound = (drawnRow >= 0) ? static_cast<uint32_t>(drawnRow) : LogSearchModel::InvalidPos;
    mFoundPos.colFound = mHighlightColumn >= 0 ? mHighlightColumn : 0;
    if (drawnRow >= 0)
    {
        moveToRow(drawnRow, true);
    }
}

void LogViewerBase::_drawSearchState(bool allLogs)
{
    if (mLogSearch->text().isEmpty())
    {
        mLogSearch->setCounter(QString());
        mSessionBar->hideNotice(LogSessionBar::eNotice::NoticeRevealed);
        return;
    }

    if (mHits.isEmpty() || (mFoundRow == LogSearchModel::InvalidPos))
    {
        mLogSearch->setCounter(tr("no match"));
    }
    else
    {
        const int at{ static_cast<int>(std::lower_bound(mHits.cbegin(), mHits.cend(), mFoundRow) - mHits.cbegin()) + 1 };
        mLogSearch->setCounter(allLogs && (mHiddenHits > 0)
            ? tr("%1 of %2 - %3 hidden").arg(at).arg(mHits.size()).arg(mHiddenHits)
            : tr("%1 of %2").arg(at).arg(mHits.size()));
    }

    if (mFilter->hasRevealedRows())
    {
        mSessionBar->showNotice(LogSessionBar::eNotice::NoticeRevealed
                               , tr("A row below is drawn only because the search found it. %1").arg(_filterSummary())
                               , tr("Drop the filters"));
    }
    else
    {
        mSessionBar->hideNotice(LogSessionBar::eNotice::NoticeRevealed);
    }
}

QString LogViewerBase::_filterSummary() const
{
    QStringList names;
    const LogViewerFilter::ListActiveFilters filters{ mFilter->activeFilters() };
    for (const LogViewerFilter::sActiveFilter& entry : filters)
    {
        const int index{ mHeader->getColumnIndex(static_cast<LoggingModelBase::eColumn>(entry.column)) };
        if (index >= 0)
        {
            names.append(mLogModel->getHeaderName(index));
        }
    }

    if (mLogModel->hasRefusedScopes())
    {
        names.append(tr("hidden scopes"));
    }

    return names.isEmpty() ? tr("No filter is on.")
                           : tr("It is kept out by: %1.").arg(names.join(QStringLiteral(", ")));
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
            // The row is kept out by a filter. It is let through and marked apart, so the
            // reader sees the row and what hid it instead of answering a dialog first.
            mFilter->revealRow(index.row());
            _selectSourceLog(index);
            mSessionBar->showNotice(LogSessionBar::eNotice::NoticeRevealed
                                   , tr("The selected row is drawn only because it was asked for. %1").arg(_filterSummary())
                                   , tr("Drop the filters"));
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
    QModelIndex idx{ ctrlTable()->indexAt(pos) };
    if (idx.isValid() == false)
    {
        idx = ctrlTable()->currentIndex();
    }

    QAction* copyMsg = menu.addAction(tr("Copy message"));
    copyMsg->setShortcut(QKeySequence::Copy);
    copyMsg->setEnabled(idx.isValid());
    connect(copyMsg, &QAction::triggered, this, &LogViewerBase::onCopyMessage);

    QAction* copyRow = menu.addAction(tr("Copy row"));
    copyRow->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    copyRow->setEnabled(idx.isValid());
    connect(copyRow, &QAction::triggered, this, &LogViewerBase::onCopyRow);

    menu.addSeparator();
    QMenu* isolateMenu = menu.addMenu(tr("Isolate"));
    _populateIsolateMenu(isolateMenu, idx.isValid() ? idx.row() : -1);

    menu.addSeparator();
    QMenu* columnsMenu = menu.addMenu(tr("Columns"));
    _populateColumnsMenu(columnsMenu, idx.isValid() ? idx.row() : -1);
    menu.exec(ctrlTable()->viewport()->mapToGlobal(pos));
}

void LogViewerBase::_populateIsolateMenu(QMenu* menu, int row)
{
    const QModelIndex source{ (row >= 0) && (mFilter != nullptr)
                            ? mFilter->mapToSource(mFilter->index(row, 0, QModelIndex()))
                            : QModelIndex() };
    const areg::LogEntry* entry{ source.isValid() ? mLogModel->getLogData(source.row()) : nullptr };

    if (entry != nullptr)
    {
        LogViewerFilter::sIsolation base;
        base.cookie    = entry->logCookie;
        base.thread    = entry->logThreadId;
        base.scopeId   = entry->logScopeId;
        base.sessionId = entry->logSessionId;

        const QString process{ QString(entry->logModule) };
        const QString thread { QString(entry->logThread) };
        QString scope { mLogModel->getScopeName(entry->logCookie, entry->logScopeId) };
        if (scope.isEmpty())
        {
            scope = tr("scope %1").arg(entry->logScopeId);
        }

        const auto addEntry = [this, menu, base](const QString& text, const QString& chip, LogViewerFilter::eIsolation kind) {
                QAction* action = menu->addAction(text);
                connect(action, &QAction::triggered, this, [this, base, chip, kind]() {
                        LogViewerFilter::sIsolation pick{ base };
                        pick.kind = kind;
                        mIsolationText = chip;
                        mFilter->setIsolation(pick);
                        _resetSearchResult();
                        _updateChips();
                        _updateCounters();
                    });
            };

        addEntry(tr("Only this call of %1").arg(scope) , tr("one call of %1").arg(scope) , LogViewerFilter::eIsolation::IsolationCall);
        addEntry(tr("Only the thread %1").arg(thread)  , tr("thread %1").arg(thread)     , LogViewerFilter::eIsolation::IsolationThread);
        addEntry(tr("Only the process %1").arg(process), tr("process %1").arg(process)   , LogViewerFilter::eIsolation::IsolationProcess);
        addEntry(tr("Only the scope %1").arg(scope)    , scope                           , LogViewerFilter::eIsolation::IsolationScope);
        menu->addSeparator();
    }

    QAction* drop = menu->addAction(tr("Show every process and scope again"));
    drop->setEnabled(mFilter->hasIsolation());
    connect(drop, &QAction::triggered, this, [this]() {
            mFilter->clearIsolation();
            mIsolationText.clear();
            _updateChips();
            _updateCounters();
        });
}

QList<int> LogViewerBase::_selectedRows(void) const
{
    QList<int> result;
    QTableView* table{ const_cast<LogViewerBase*>(this)->ctrlTable() };
    if (table == nullptr)
        return result;

    const QItemSelectionModel* selection{ table->selectionModel() };
    if (selection != nullptr)
    {
        const QModelIndexList indexes{ selection->selectedRows() };
        for (const QModelIndex& index : indexes)
        {
            result.append(index.row());
        }
    }

    if (result.isEmpty())
    {
        const QModelIndex current{ table->currentIndex() };
        if (current.isValid())
        {
            result.append(current.row());
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

QString LogViewerBase::_rowsToText(const QList<int>& rows, bool fullLayout) const
{
    if ((mLogModel == nullptr) || rows.isEmpty())
        return QString();

    QStringList lines;
    lines.reserve(rows.size());

    for (int row : rows)
    {
        const areg::LogEntry* entry{ mLogModel->getLogData(row) };
        if (entry == nullptr)
            continue;

        const QString message{ QString::fromUtf8(entry->logMessage) };
        if (fullLayout == false)
        {
            lines.append(message);
            continue;
        }

        // The layout the target writes into its own log file, see areg.init:
        //   message  %d: [ %a.%x.%t.%p >>> ] %m
        //   enter    %d: [ %a.%x.%t.%z: Enter -->]
        //   exit     %d: [ %a.%x.%t.%z: Exit <-- ]
        const QString stamp{ QString::fromStdString(areg::DateTime(entry->logTimestamp).format_time().data()) };
        const QString head { QString("%1: [ %2.%3.%4.")
                                .arg(stamp)
                                .arg(static_cast<quint64>(entry->logCookie))
                                .arg(QString::fromUtf8(entry->logModule))
                                .arg(static_cast<quint64>(entry->logThreadId)) };

        if (entry->logMsgType == areg::LogMessageType::ScopeEnter)
        {
            lines.append(head + message + ": Enter -->]");
        }
        else if (entry->logMsgType == areg::LogMessageType::ScopeExit)
        {
            lines.append(head + message + ": Exit <-- ]");
        }
        else
        {
            const QString prio{ QString::fromStdString(areg::priority_to_string(entry->logMessagePrio).data()) };
            lines.append(head + prio + " >>> ] " + message);
        }
    }

    return lines.join(QChar('\n'));
}

void LogViewerBase::onCopyMessage(void)
{
    const QString text{ _rowsToText(_selectedRows(), false) };
    if (text.isEmpty() == false)
    {
        QGuiApplication::clipboard()->setText(text);
    }
}

void LogViewerBase::onCopyRow(void)
{
    const QString text{ _rowsToText(_selectedRows(), true) };
    if (text.isEmpty() == false)
    {
        QGuiApplication::clipboard()->setText(text);
    }
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
    mSessionBar = nullptr;
    mEmptyState = nullptr;

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

void LogViewerBase::_refitRowSelection()
{
    QItemSelectionModel* selection{ mLogTable != nullptr ? mLogTable->selectionModel() : nullptr };
    if ((selection == nullptr) || (selection->hasSelection() == false))
        return;

    const QAbstractItemModel* model{ selection->model() };
    const int lastColumn{ model != nullptr ? model->columnCount() - 1 : -1 };
    if (lastColumn < 0)
        return;

    QItemSelection rows;
    const QItemSelection current{ selection->selection() };
    for (const QItemSelectionRange& range : current)
    {
        const QModelIndex left { model->index(range.top()   , 0         , range.parent()) };
        const QModelIndex right{ model->index(range.bottom(), lastColumn, range.parent()) };
        rows.merge(QItemSelection(left, right), QItemSelectionModel::Select);
    }

    selection->select(rows, QItemSelectionModel::ClearAndSelect);
}

void LogViewerBase::_populateColumnsMenu(QMenu* menu, int curRow)
{
    // Get current active columns from the model
    const QList<LoggingModelBase::eColumn>& activeCols = mLogModel->getActiveColumns();
    const QStringList& headers{ LoggingModelBase::getHeaderList() };

    QAction* actResetFilters = menu->addAction(tr("Reset Filters"));
    actResetFilters->setCheckable(false);
    connect(actResetFilters, &QAction::triggered, this, [this]() {
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
    connect(actResetColumns, &QAction::triggered, this, [this]() {
            mLogModel->setActiveColumns(QList<LoggingModelBase::eColumn>());
            resetColumnOrder();
            ctrlTable()->viewport()->update();
            moveToBottom(true);
        });
}

inline void LogViewerBase::_resetSearchResult()
{
    mFoundPos = LogSearchModel::sFoundPos{};
    mFoundRow = LogSearchModel::InvalidPos;
    mHits.clear();
    mHiddenHits = 0;
    mSearch.resetSearch();
    if (mFilter != nullptr)
    {
        mFilter->clearRevealedRows();
    }

    if (mLogSearch != nullptr)
    {
        mLogSearch->setCounter(QString());
    }

    if (mSessionBar != nullptr)
    {
        mSessionBar->hideNotice(LogSessionBar::eNotice::NoticeRevealed);
    }

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
