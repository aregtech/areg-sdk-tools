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
#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/NELogPalette.hpp"
#include "lusan/data/common/OptionsManager.hpp"
#include "lusan/view/log/LogEmptyState.hpp"
#include "lusan/view/log/LogFilterChips.hpp"
#include "lusan/view/log/LogHeaderItem.hpp"
#include "lusan/view/log/LogHitMap.hpp"
#include "lusan/view/common/NaviLogScopeBase.hpp"
#include "lusan/view/log/LogSessionBar.hpp"
#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/view/log/LogTableView.hpp"
#include "lusan/view/log/LogViewPanels.hpp"
#include "lusan/view/log/ScopeOutputViewer.hpp"

#include "lusan/model/log/LogViewerFilter.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/view/log/LogTextHighlight.hpp"

#include "areg/base/DateTime.hpp"
#include "areg/logging/LoggingDefs.hpp"

#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QHeaderView>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
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
    , mHitMap   (nullptr)
    , mPickColumns(nullptr)
    , mPickFilters(nullptr)
    , mSkew     ( )
    , mSkewShown(false)
    , mFollowScroll(false)
    , mFollowSelect(false)
    , mWordWrap   (LusanApplication::getOptions().isLogWordWrap())
    , mMeasuring  (false)
    , mFittedWidth(-1)
    , mFittedTaken(-1)
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
    mLogTable   = new LogTableView(mMdiWindow);
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
    mHitMap = new LogHitMap(mLogTable);
    mHitMap->setSource(mLogModel, mFilter);
    QShortcut* shortcutSearch = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    QShortcut* shortcutCopyMsg = new QShortcut(QKeySequence::Copy, this);
    QShortcut* shortcutCopyRow = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), this);
    QShortcut* shortcutNextBad = new QShortcut(QKeySequence(Qt::Key_F8), this);
    QShortcut* shortcutPrevBad = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F8), this);
    QShortcut* shortcutFilter  = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F), this);
    QShortcut* shortcutNoFilter= new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R), this);
    QShortcut* shortcutWrap    = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_W), this);
    QShortcut* shortcutColumns = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K), this);
    QShortcut* shortcutAnalyze = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A), this);

    // Every MDI child shares one window, so a window wide shortcut in two of them is ambiguous
    // and Qt then fires neither. Each one answers only while the focus is inside this window.
    for (QShortcut* shortcut : { shortcutSearch, shortcutCopyMsg, shortcutCopyRow, shortcutNextBad, shortcutPrevBad
                               , shortcutFilter, shortcutNoFilter, shortcutWrap, shortcutColumns
                               , shortcutAnalyze })
    {
        shortcut->setContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    }
    mSearch.setLogModel(mFilter);

    mLogTable->setHorizontalHeader(mHeader);
    mHeader->setVisible(true);
    mHeader->show();
    mHeader->setContextMenuPolicy(Qt::CustomContextMenu);
    mHeader->setSectionsMovable(true);

    mLogTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mLogTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mLogTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    // The columns can add up to more than the window holds, so the bar appears and says so.
    // The last section is not stretched: a stretched one cannot be widened past the viewport,
    // and widening the message column is exactly what a long line calls for.
    mLogTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mLogTable->setShowGrid(false);
    mLogTable->setCurrentIndex(QModelIndex());
    mLogTable->horizontalHeader()->setStretchLastSection(false);
    mLogTable->verticalHeader()->hide();
    LogViewerBase::applyRowHeight(mLogTable);
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
    mLogTable->setWordWrap(mWordWrap);
    mLogTable->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(mMdiWindow);
    setLayout(layout);
    setAttribute(Qt::WA_DeleteOnClose);

    mLogTable->setModel(mFilter);
    _bindSelection();
    mLogTable->setAutoScroll(true);
    if (mHighlight == nullptr)
    {
        mHighlight = new LogTextHighlight(mFoundPos, mLogTable);
        mHighlight->setWordWrap(mWordWrap, LusanApplication::getOptions().getLogWrapLines());
        mLogTable->setItemDelegate(mHighlight);
    }
    _updateHighlightColumn();
    _restoreLayout();

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
    connect(mHeader     , &LogTableHeader::signalColumnsRequested       , this, [this](const QRect& anchor){_showColumnPicker(anchor);});
    connect(mLogTable   , &QTableView::customContextMenuRequested       , this, [this](const QPoint& pos)  {onTableContextMenu(pos);});

    // A wrapped row is as tall as its message, and only the rows on screen are measured, so
    // every move of the table asks for the measurement again.
    connect(mLogTable->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) { _measureShownRows(); });
    connect(mHeader     , &QHeaderView::sectionResized, this, [this](int, int, int) {
                _measureShownRows();
            });
    connect(mFilter     , &QAbstractItemModel::modelReset, this, [this]() { _measureShownRows(); });
    connect(mFilter     , &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex&, int, int) {
                _measureShownRows();
            });


    connect(mLogTable   , &QTableView::clicked                          , this, [this](const QModelIndex &index){onMouseButtonClicked(index);});
    connect(mLogTable   , &QTableView::doubleClicked                    , this, [this](const QModelIndex &index){onMouseDoubleClicked(index);});
    
    connect(mLogSearch  , &SearchLineEdit::signalSearchTextChanged      , this, [this](const QString& text) {
                mSearch.resetSearch();
                mSessionBar->ctrlFilterMatches()->setEnabled(text.isEmpty() == false);
                if (text.isEmpty() && (mHitMap != nullptr))
                {
                    mHitMap->clearHits();
                }
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
    connect(shortcutNextBad, &QShortcut::activated                      , this, [this]() { _stepToProblem(true);  });
    connect(shortcutPrevBad, &QShortcut::activated                      , this, [this]() { _stepToProblem(false); });
    connect(shortcutFilter , &QShortcut::activated                      , this, [this]() { filterCurrentColumn(); });
    connect(shortcutNoFilter,&QShortcut::activated                      , this, [this]() { clearEveryFilter();    });
    connect(shortcutWrap   , &QShortcut::activated                      , this, [this]() { setWordWrap(!mWordWrap); });
    connect(shortcutColumns, &QShortcut::activated                      , this, [this]() { _showColumnPicker();  });
    connect(shortcutAnalyze, &QShortcut::activated                      , this, [this]() { analyzeSelection();   });

    connect(mSessionBar->ctrlFilters(), &QToolButton::clicked, this, [this]() { _showFilterPanel();  });
    connect(mSessionBar->ctrlColumns(), &QToolButton::clicked, this, [this]() { _showColumnPicker(); });
    connect(mSessionBar, &LogSessionBar::signalWordWrapToggled, this, [this](bool wrap) { setWordWrap(wrap); });
    mSessionBar->setWordWrap(mWordWrap);

    connect(mSessionBar->ctrlFilterMatches(), &QToolButton::clicked, this, [this]() {
                filterToPhrase(NELusanCommon::FilterString{ mLogSearch->text()
                                                          , mLogSearch->isMatchCaseChecked()
                                                          , mLogSearch->isMatchWordChecked()
                                                          , mLogSearch->isWildCardChecked() });
            });

    connect(mSessionBar->ctrlHitList(), &QToolButton::clicked, this, [this]() {
                _showHitList();
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
    connect(chips, &LogFilterChips::signalChipClicked, this, [this](const LogFilterChips::sChip& chip, const QRect& anchor) {
                // The chip names the filter, so it is also where the filter is changed.
                mHeader->showFilterPanelAt(static_cast<LoggingModelBase::eColumn>(chip.column), anchor);
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
    if ((mLogTable != nullptr) && (watched == mLogTable->viewport()) && (event->type() == QEvent::Type::Resize))
    {
        if (mEmptyState != nullptr)
        {
            mEmptyState->setGeometry(mLogTable->viewport()->rect());
        }

        _fitMessageColumn();
        _measureShownRows();
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

QString LogViewerBase::_columnName(int column)
{
    const QStringList& names{ LoggingModelBase::getHeaderList() };
    return ((column >= 0) && (column < names.size()) ? names.at(column) : QString());
}

void LogViewerBase::_updateChips()
{
    if ((mSessionBar == nullptr) || (mFilter == nullptr) || (mLogModel == nullptr) || (mHeader == nullptr))
        return;

    LogFilterChips::ListChips chips;
    const LogViewerFilter::ListActiveFilters filters{ mFilter->activeFilters() };
    for (const LogViewerFilter::sActiveFilter& entry : filters)
    {
        const QString name{ LogViewerBase::_columnName(entry.column) };

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
        LogHeaderItem* item{ mHeader->getHeaderItem(static_cast<LoggingModelBase::eColumn>(chip.column)) };
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

    LogHeaderItem* item{ mHeader->getHeaderItem(LoggingModelBase::eColumn::LogColumnMessage) };
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
        _showSearchHit(allLogs);
    }
    else
    {
        mFoundRow = LogSearchModel::InvalidPos;
        mFoundPos.colFound = static_cast<int32_t>(LogSearchModel::InvalidPos);
    }

    if (mHitMap != nullptr)
    {
        mHitMap->setHits(mHits, mFoundRow == LogSearchModel::InvalidPos ? -1 : static_cast<int>(mFoundRow));
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
    mSessionBar->ctrlHitList()->setEnabled(mHits.isEmpty() == false);

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

void LogViewerBase::_saveLayout(void) const
{
    if ((mLogTable == nullptr) || (mLogModel == nullptr))
        return;

    OptionsManager& options{ LusanApplication::getOptions() };
    WorkspaceEntry workspace{ options.getActiveWorkspace() };
    if (workspace.isValid() == false)
        return;

    WorkspaceEntry::ListLogColumns columns;
    const QList<LoggingModelBase::eColumn>& active{ mLogModel->getActiveColumns() };
    for (int i = 0; i < active.size(); ++i)
    {
        WorkspaceEntry::sLogColumn column;
        column.key   = LoggingModelBase::getColumnKey(active[i]);
        column.width = mLogTable->columnWidth(i);
        if (column.key.isEmpty() == false)
        {
            columns.append(column);
        }
    }

    workspace.setLogColumns(_columnMode(), columns);
    workspace.setLogDatabase(mLogModel->getDatabasePath());
    options.updateWorkspace(workspace);

    // The record is only cached here. It reaches the file when the application closes, so a
    // window that opens and closes many times in one run costs no write.
}

WorkspaceEntry::eLogMode LogViewerBase::_columnMode(void) const
{
    return ((mLogModel != nullptr) && mLogModel->isLiveLogging())
                ? WorkspaceEntry::eLogMode::LogModeLive
                : WorkspaceEntry::eLogMode::LogModeOffline;
}

void LogViewerBase::_restoreLayout(void)
{
    const WorkspaceEntry workspace{ LusanApplication::getActiveWorkspace() };
    const WorkspaceEntry::ListLogColumns& saved{ workspace.getLogColumns(_columnMode()) };
    if (saved.isEmpty())
        return;

    QList<LoggingModelBase::eColumn> columns;
    QList<int> widths;
    for (const WorkspaceEntry::sLogColumn& column : saved)
    {
        const LoggingModelBase::eColumn col{ LoggingModelBase::getColumnByKey(column.key) };
        if ((col != LoggingModelBase::eColumn::LogColumnInvalid) && (columns.contains(col) == false))
        {
            columns.append(col);
            widths.append(column.width);
        }
    }

    if (columns.isEmpty())
        return;

    // A width belongs to a column, and the shaping may add the rail or the message back, so
    // the widths are carried by column and given to the shaped list afterwards.
    QMap<int, int> byColumn;
    for (int i = 0; i < columns.size(); ++i)
    {
        byColumn.insert(static_cast<int>(columns.at(i)), widths.at(i));
    }

    const QList<LoggingModelBase::eColumn> shaped{ LoggingModelBase::shapeColumns(columns) };
    mLogTable->setModel(nullptr);
    mLogModel->setActiveColumns(shaped);
    mLogTable->setModel(mFilter);
    _bindSelection();

    // A view drops every section size when a model is set, so the widths are applied here and
    // never before the model.
    for (int i = 0; i < shaped.size(); ++i)
    {
        const int width{ byColumn.value(static_cast<int>(shaped.at(i)), 0) };
        if (width > 0)
        {
            mLogTable->setColumnWidth(i, width);
        }
    }

    _updateHighlightColumn();
    _fitMessageColumn();
}

void LogViewerBase::_forgetLayout(void) const
{
    OptionsManager& options{ LusanApplication::getOptions() };
    WorkspaceEntry workspace{ options.getActiveWorkspace() };
    if (workspace.isValid() == false)
        return;

    workspace.setLogColumns(WorkspaceEntry::eLogMode::LogModeLive, WorkspaceEntry::ListLogColumns());
    workspace.setLogColumns(WorkspaceEntry::eLogMode::LogModeOffline, WorkspaceEntry::ListLogColumns());
    options.updateWorkspace(workspace);
    options.writeOptions();
}

NaviLogScopeBase* LogViewerBase::_scopePanel(void) const
{
    if (mMainWindow == nullptr)
        return nullptr;

    return getMdiWindowType() == MdiChild::eMdiWindow::MdiLogViewer
         ? static_cast<NaviLogScopeBase*>(&mMainWindow->getNaviLiveScopes())
         : static_cast<NaviLogScopeBase*>(&mMainWindow->getNaviOfflineScopes());
}

void LogViewerBase::_stepToProblem(bool forward)
{
    if ((mFilter == nullptr) || (mLogModel == nullptr))
        return;

    const int shown{ mFilter->rowCount(QModelIndex()) };
    if (shown <= 0)
        return;

    const QModelIndex current{ ctrlTable()->currentIndex() };
    const int from{ current.isValid() ? current.row() : (forward ? -1 : shown) };
    const int step{ forward ? 1 : -1 };

    for (int row = from + step; (row >= 0) && (row < shown); row += step)
    {
        const QModelIndex source{ mFilter->mapToSource(mFilter->index(row, 0)) };
        if (LoggingModelBase::isProblemEntry(mLogModel->getLogData(source.row())) == false)
            continue;

        const QModelIndex target{ mFilter->index(row, current.isValid() ? current.column() : 0) };
        ctrlTable()->setCurrentIndex(target);
        ctrlTable()->scrollTo(target, QAbstractItemView::ScrollHint::PositionAtCenter);
        return;
    }
}

void LogViewerBase::_showHitList(void)
{
    if (mHits.isEmpty() || (mLogModel == nullptr))
        return;

    QMenu menu(this);
    int listed{ 0 };
    for (uint32_t hit : mHits)
    {
        if (listed >= LogViewerBase::HitListMax)
        {
            QAction* more = menu.addAction(tr("... and %1 more").arg(mHits.size() - listed));
            more->setEnabled(false);
            break;
        }

        const areg::LogEntry* entry{ mLogModel->getLogData(static_cast<int>(hit)) };
        if (entry == nullptr)
            continue;

        QString text{ QString::fromUtf8(entry->logMessage).simplified() };
        if (text.size() > LogViewerBase::HitListChars)
        {
            text = text.left(LogViewerBase::HitListChars) + QStringLiteral("...");
        }

        QAction* action = menu.addAction(tr("%1:  %2").arg(hit + 1).arg(text));
        action->setData(hit);
        ++listed;
    }

    QAction* chosen{ menu.exec(mSessionBar->ctrlHitList()->mapToGlobal(QPoint(0, mSessionBar->ctrlHitList()->height()))) };
    if ((chosen == nullptr) || chosen->data().isValid() == false)
        return;

    mFoundRow = chosen->data().toUInt();
    mFoundPos.rowFound = mFoundRow;
    _showSearchHit(mSessionBar->isSearchingAllLogs());
    _drawSearchState(mSessionBar->isSearchingAllLogs());
    if (mHitMap != nullptr)
    {
        mHitMap->setHits(mHits, static_cast<int>(mFoundRow));
    }
}

QString LogViewerBase::_filterSummary() const
{
    QStringList names;
    const LogViewerFilter::ListActiveFilters filters{ mFilter->activeFilters() };
    for (const LogViewerFilter::sActiveFilter& entry : filters)
    {
        const QString name{ LogViewerBase::_columnName(entry.column) };
        if (name.isEmpty() == false)
        {
            names.append(name);
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
    // Going to the end is the one selection the window makes for itself; it must not read back
    // as the user picking a row, which is what stops the table from following the newest log.
    mFollowSelect = true;
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

    mFollowSelect = false;
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
        // The row is revealed by the rail, the column it opens with. Asking for the message
        // cell instead reveals the right end of the widest column of the table.
        QModelIndex idxSelected = mFilter->index(row, 0, QModelIndex());
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
    _bindSelection();
    _updateHighlightColumn();
}

void LogViewerBase::resetFilters()
{
    Q_ASSERT(mLogTable != nullptr);
    Q_ASSERT(mHeader != nullptr);
    mLogTable->setModel(nullptr);
    mHeader->resetFilters();
    mLogTable->setModel(mFilter);
    _bindSelection();
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

    QAction* selectAll = menu.addAction(tr("Select all rows"));
    selectAll->setShortcut(QKeySequence::SelectAll);
    connect(selectAll, &QAction::triggered, this, [this]() { ctrlTable()->selectAll(); });

    QAction* dropSelection = menu.addAction(tr("Clear the selection"));
    dropSelection->setEnabled((mLogTable != nullptr)
                              && (mLogTable->selectionModel() != nullptr)
                              && mLogTable->selectionModel()->hasSelection());
    connect(dropSelection, &QAction::triggered, this, [this]() { mLogTable->clearSelection(); });

    menu.addSeparator();

    QAction* find = menu.addAction(tr("Find..."));
    find->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
    connect(find, &QAction::triggered, this, [this]() { mLogSearch->setFocus(Qt::FocusReason::ShortcutFocusReason); });

    QAction* nextBad = menu.addAction(tr("Next error or warning"));
    nextBad->setShortcut(QKeySequence(Qt::Key_F8));
    connect(nextBad, &QAction::triggered, this, [this]() { _stepToProblem(true); });

    QAction* prevBad = menu.addAction(tr("Previous error or warning"));
    prevBad->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F8));
    connect(prevBad, &QAction::triggered, this, [this]() { _stepToProblem(false); });

    menu.addSeparator();
    _populateFilterMenu(&menu, idx);

    // One entry per subject of the clicked row, each carrying the verbs that act on it. The
    // scope entries come from the navigation panel, so the two surfaces cannot drift apart.
    NaviLogScopeBase* panel{ _scopePanel() };
    QModelIndex scopeNode;
    _populateSubjectMenus(&menu, idx.isValid() ? idx.row() : -1, panel, scopeNode);

    menu.addSeparator();

    QAction* wrap = menu.addAction(tr("Word wrap"));
    wrap->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_W));
    wrap->setCheckable(true);
    wrap->setChecked(mWordWrap);
    connect(wrap, &QAction::triggered, this, [this](bool checked) { setWordWrap(checked); });

    QAction* pick = menu.addAction(tr("Choose columns..."));
    pick->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K));
    connect(pick, &QAction::triggered, this, [this]() { _showColumnPicker(); });

    QMenu* columnsMenu = menu.addMenu(tr("Columns"));
    _populateColumnsMenu(columnsMenu, idx.isValid() ? idx.row() : -1);

    // A context menu hides the shortcut of its entries by default on this platform, and the
    // menu is where a reader learns them.
    for (QAction* action : menu.actions())
    {
        action->setShortcutVisibleInContextMenu(true);
    }

    const QAction* chosen{ menu.exec(ctrlTable()->viewport()->mapToGlobal(pos)) };
    if ((chosen != nullptr) && scopeNode.isValid() && (panel != nullptr))
    {
        panel->applyScopeMenu(*chosen, scopeNode);
    }
}

void LogViewerBase::_populateFilterMenu(QMenu* menu, const QModelIndex& index)
{
    if ((mHeader == nullptr) || (mFilter == nullptr) || (mLogModel == nullptr))
        return;

    const int row{ index.isValid() ? index.row() : -1 };
    const QModelIndex source{ row >= 0 ? mFilter->mapToSource(mFilter->index(row, 0, QModelIndex())) : QModelIndex() };
    const areg::LogEntry* entry{ source.isValid() ? mLogModel->getLogData(source.row()) : nullptr };

    // The columns the reader filters a log by, in the order a log is read: what happened,
    // who wrote it, and the text itself.
    static const LoggingModelBase::eColumn _byValue[]
    {
          LoggingModelBase::eColumn::LogColumnPriority
        , LoggingModelBase::eColumn::LogColumnSource
        , LoggingModelBase::eColumn::LogColumnThread
    };

    const LoggingModelBase::eColumn clicked{ index.isValid()
                                           ? mHeader->getColumn(index.column())
                                           : LoggingModelBase::eColumn::LogColumnInvalid };

    if (entry == nullptr)
    {
        QAction* none = menu->addAction(tr("Filter by this value"));
        none->setEnabled(false);
        return;
    }

    // A message can be several hundred characters over several lines. The entry names the
    // value, it does not carry it, so it is cut to one short line.
    const auto shortText = [](const QString& text) {
            QString one{ text.simplified() };
            constexpr int limit{ 52 };
            return one.size() > limit ? (one.left(limit - 1) + QChar(0x2026)) : one;
        };

    const auto addEntry = [this, entry, row, shortText](QMenu* target, LoggingModelBase::eColumn column, bool exclude) {
            const QString value{ shortText(mLogModel->getCellData(entry, column)) };
            if (value.isEmpty())
                return false;

            const QString name{ LogViewerBase::_columnName(static_cast<int>(column)) };
            QAction* action = target->addAction(exclude ? tr("Exclude: %1 = %2").arg(name).arg(value)
                                                        : tr("Filter: %1 = %2").arg(name).arg(value));

            // An entry that names a priority carries the colour the rail paints that
            // priority with, so the menu and the row the reader clicked agree.
            if (column == LoggingModelBase::eColumn::LogColumnPriority)
            {
                action->setIcon(LogViewerBase::priorityDot(entry->logMessagePrio, mLogTable));
            }
            connect(action, &QAction::triggered, this, [this, column, row, exclude]() {
                    _filterByCell(column, row, exclude);
                });

            return true;
        };

    // Only a column narrowed by picking values can keep everything but one of them. A column
    // narrowed by a phrase has nothing to write that would mean "not this".
    const auto canExclude = [](LoggingModelBase::eColumn column) {
            return (column != LoggingModelBase::eColumn::LogColumnMessage)
                && (column != LoggingModelBase::eColumn::LogColumnTimeDuration);
        };

    // The column the reader clicked is the one they are looking at, so it costs one click.
    // The others are one level down, in the order a log row is read.
    if (mHeader->canFilter(clicked) && addEntry(menu, clicked, false) && canExclude(clicked))
    {
        addEntry(menu, clicked, true);
    }

    QMenu* more = menu->addMenu(tr("Filter by"));
    for (LoggingModelBase::eColumn column : _byValue)
    {
        if (column != clicked)
        {
            addEntry(more, column, false);
        }
    }

    if (clicked != LoggingModelBase::eColumn::LogColumnMessage)
    {
        addEntry(more, LoggingModelBase::eColumn::LogColumnMessage, false);
    }

    more->addSeparator();
    for (LoggingModelBase::eColumn column : _byValue)
    {
        if (column != clicked)
        {
            addEntry(more, column, true);
        }
    }

    if (more->isEmpty())
    {
        menu->removeAction(more->menuAction());
    }
}

bool LogViewerBase::_rowSubject(int row, LogViewerFilter::sIsolation& base, QString& process, QString& thread, QString& scope) const
{
    const QModelIndex source{ (row >= 0) && (mFilter != nullptr)
                            ? mFilter->mapToSource(mFilter->index(row, 0, QModelIndex()))
                            : QModelIndex() };
    const areg::LogEntry* entry{ (source.isValid() && (mLogModel != nullptr)) ? mLogModel->getLogData(source.row()) : nullptr };
    if (entry == nullptr)
        return false;

    base.cookie    = entry->logCookie;
    base.thread    = entry->logThreadId;
    base.scopeId   = entry->logScopeId;
    base.sessionId = entry->logSessionId;

    // A name a menu entry is built from is never left empty: a source that sent none is
    // named by the number the collector knows it under.
    process = QString(entry->logModule);
    if (process.isEmpty())
    {
        process = tr("process %1").arg(entry->logCookie);
    }

    thread = QString(entry->logThread);
    if (thread.isEmpty())
    {
        thread = tr("thread %1").arg(entry->logThreadId);
    }

    scope = mLogModel->getScopeName(entry->logCookie, entry->logScopeId);
    if (scope.isEmpty())
    {
        scope = tr("scope %1").arg(entry->logScopeId);
    }

    return true;
}

QMenu* LogViewerBase::_addSubjectMenu( QMenu* menu, const LogViewerFilter::sIsolation& base, int row
                                     , LogViewerFilter::eIsolation kind, const QString& title
                                     , const QString& chip, const QIcon& icon, bool strong)
{
    QMenu* subject = menu->addMenu(icon, title);

    // A title that is a value the clicked row carries is written in bold, so the reader tells
    // the words of the menu from the words of their own log at a glance.
    if (strong)
    {
        QAction* opener{ subject->menuAction() };
        QFont face{ opener->font() };
        face.setBold(true);
        opener->setFont(face);
    }

    QAction* isolate = subject->addAction(tr("Isolate these rows"));
    connect(isolate, &QAction::triggered, this, [this, base, kind, chip]() {
            LogViewerFilter::sIsolation pick{ base };
            pick.kind = kind;
            mIsolationText = chip;
            mFilter->setIsolation(pick);
            _resetSearchResult();
            _updateChips();
            _updateCounters();
        });

    QAction* mark = subject->addAction(tr("Select these rows"));
    connect(mark, &QAction::triggered, this, [this, base, kind]() {
            LogViewerFilter::sIsolation pick{ base };
            pick.kind = kind;
            selectMatching(pick);
        });

    if (mMainWindow != nullptr)
    {
        // The row is carried by number, not by index: rows keep arriving while the menu
        // stands open, and an index of a live model does not survive that.
        const int sourceRow{ mFilter->mapToSource(mFilter->index(row, 0, QModelIndex())).row() };
        const auto addAnalyze = [this, subject, sourceRow](const QString& text, ScopeLogViewerFilter::eDataFilter mode) {
                QAction* action = subject->addAction(text);
                connect(action, &QAction::triggered, this, [this, sourceRow, mode]() {
                        ScopeOutputViewer& viewScope = mMainWindow->getOutputScopeLogs();
                        viewScope.bindWindow(*this);
                        viewScope.analyzeAt(mLogModel, mLogModel->index(sourceRow, 0, QModelIndex()), mode);
                    });
            };

        switch (kind)
        {
        case LogViewerFilter::eIsolation::IsolationCall:
            addAnalyze(tr("Analyze this call")          , ScopeLogViewerFilter::eDataFilter::FilterSession);
            addAnalyze(tr("Analyze with what it called"), ScopeLogViewerFilter::eDataFilter::FilterSublogs);
            break;

        case LogViewerFilter::eIsolation::IsolationScope:
            addAnalyze(tr("Analyze the scope")  , ScopeLogViewerFilter::eDataFilter::FilterScope);
            break;

        case LogViewerFilter::eIsolation::IsolationThread:
            addAnalyze(tr("Analyze the thread") , ScopeLogViewerFilter::eDataFilter::FilterThread);
            break;

        case LogViewerFilter::eIsolation::IsolationProcess:
            addAnalyze(tr("Analyze the process"), ScopeLogViewerFilter::eDataFilter::FilterProcess);
            break;

        default:
            break;
        }
    }

    return subject;
}

void LogViewerBase::_populateSubjectMenus(QMenu* menu, int row, NaviLogScopeBase* panel, QModelIndex& node)
{
    node = QModelIndex();

    LogViewerFilter::sIsolation base;
    QString process;
    QString thread;
    QString scope;
    if (_rowSubject(row, base, process, thread, scope))
    {
        // A group heading is drawn as a bare line while a theme style sheet is on, so the
        // group is marked by a separator and the icons that open it.
        menu->addSeparator();

        // The call is the row itself, so its entry names no value and stays in plain text.
        _addSubjectMenu(menu, base, row, LogViewerFilter::eIsolation::IsolationCall
                       , tr("This call"), tr("one call of %1").arg(scope)
                       , NELusanCommon::iconSubjectCall(), false);

        QMenu* scopeMenu{ _addSubjectMenu(menu, base, row, LogViewerFilter::eIsolation::IsolationScope
                                         , scope, scope, NELusanCommon::iconScopeLines(), true) };

        _addSubjectMenu(menu, base, row, LogViewerFilter::eIsolation::IsolationThread
                       , thread, tr("thread %1").arg(thread)
                       , NELusanCommon::iconSubjectThread(), true);

        _addSubjectMenu(menu, base, row, LogViewerFilter::eIsolation::IsolationProcess
                       , process, tr("process %1").arg(process)
                       , NELusanCommon::iconSubjectProcess(), true);

        // What the navigation panel offers for the same scope, under the scope entry.
        if (panel != nullptr)
        {
            const QModelIndex found{ panel->findScopeIndex(base.cookie, base.scopeId) };
            if (found.isValid())
            {
                scopeMenu->addSeparator();
                if (panel->populateScopeMenu(*scopeMenu, found))
                {
                    node = found;
                }
            }
        }
    }

    QAction* analyzeMarked = menu->addAction(tr("Analyze the selected rows"));
    analyzeMarked->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
    analyzeMarked->setEnabled((mMainWindow != nullptr) && (_selectedRows().isEmpty() == false));
    connect(analyzeMarked, &QAction::triggered, this, [this]() { analyzeSelection(); });

    QAction* drop = menu->addAction(tr("Show every process and scope again"));
    drop->setEnabled(mFilter->hasIsolation());
    connect(drop, &QAction::triggered, this, [this]() {
            mFilter->clearIsolation();
            mIsolationText.clear();
            _updateChips();
            _updateCounters();
        });
}

void LogViewerBase::selectMatching(const LogViewerFilter::sIsolation& pick)
{
    QItemSelectionModel* selection{ mLogTable != nullptr ? mLogTable->selectionModel() : nullptr };
    if ((selection == nullptr) || (mFilter == nullptr) || (mLogModel == nullptr))
        return;

    const int rows{ mFilter->rowCount() };
    const int last{ mFilter->columnCount() - 1 };
    if ((rows <= 0) || (last < 0))
        return;

    // The rows of one call stand apart from each other, so the runs are gathered first and
    // handed over in one call. Selecting them row by row redraws the table on every row.
    QItemSelection marked;
    int first{ -1 };
    for (int row = 0; row <= rows; ++row)
    {
        const areg::LogEntry* entry{ nullptr };
        if (row < rows)
        {
            const QModelIndex source{ mFilter->mapToSource(mFilter->index(row, 0, QModelIndex())) };
            entry = source.isValid() ? mLogModel->getLogData(source.row()) : nullptr;
        }

        const bool keep{ (entry != nullptr) && LogViewerFilter::matchesIsolation(pick, entry) };
        if (keep && (first < 0))
        {
            first = row;
        }
        else if ((keep == false) && (first >= 0))
        {
            marked.select(mFilter->index(first, 0, QModelIndex()), mFilter->index(row - 1, last, QModelIndex()));
            first = -1;
        }
    }

    selection->select(marked, QItemSelectionModel::SelectionFlag::ClearAndSelect);
    if (marked.isEmpty() == false)
    {
        const QModelIndex head{ marked.first().topLeft() };
        selection->setCurrentIndex(head, QItemSelectionModel::SelectionFlag::NoUpdate);
        mLogTable->scrollTo(head, QAbstractItemView::ScrollHint::PositionAtCenter);
    }

    _refitRowSelection();
}

void LogViewerBase::analyzeSelection(void)
{
    if ((mMainWindow == nullptr) || (mFilter == nullptr) || (mLogModel == nullptr))
        return;

    const QList<int> rows{ _selectedRows() };
    QList<int> sourceRows;
    sourceRows.reserve(rows.size());
    for (int row : rows)
    {
        const QModelIndex source{ mFilter->mapToSource(mFilter->index(row, 0, QModelIndex())) };
        if (source.isValid())
        {
            sourceRows.append(source.row());
        }
    }

    if (sourceRows.isEmpty())
        return;

    ScopeOutputViewer& viewScope = mMainWindow->getOutputScopeLogs();
    viewScope.bindWindow(*this);
    viewScope.analyzeRows(mLogModel, sourceRows);
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

        // The layout the target writes into its own log file, see areg.init, with the two
        // fields the framework's own lines drop: the scope name on a message row, and the
        // elapsed time on an exit row.
        //   message  %d: [ %a.%x.%t.%z.%p >>> ] %m
        //   enter    %d: [ %a.%x.%t.%z: Enter -->]
        //   exit     %d: [ %a.%x.%t.%z: Exit <-- ] (elapsed)
        const bool enters{ entry->logMsgType == areg::LogMessageType::ScopeEnter };
        const bool leaves{ entry->logMsgType == areg::LogMessageType::ScopeExit  };

        QString scope{ mLogModel->getScopeName(entry->logCookie, entry->logScopeId) };
        if (scope.isEmpty())
        {
            // An enter or exit row carries the scope name as its message, so it names itself
            // even when the target never announced its scopes.
            scope = (enters || leaves) ? message : QString("scope %1").arg(entry->logScopeId);
        }

        const QString stamp{ QString::fromStdString(areg::DateTime(entry->logTimestamp).format_time().data()) };
        const QString head { QString("%1: [ %2.%3.%4.%5")
                                .arg(stamp)
                                .arg(static_cast<quint64>(entry->logCookie))
                                .arg(QString::fromUtf8(entry->logModule))
                                .arg(static_cast<quint64>(entry->logThreadId))
                                .arg(scope) };

        if (enters)
        {
            lines.append(head + ": Enter -->]");
        }
        else if (leaves)
        {
            lines.append(head + QString(": Exit <-- ] (%1 ms)")
                                    .arg(static_cast<double>(entry->logDuration) / 1000.0, 0, 'f', 3));
        }
        else
        {
            const QString prio{ QString::fromStdString(areg::priority_to_string(entry->logMessagePrio).data()) };
            QString tail{ message };
            if (areg::is_log_message_cut(*entry))
            {
                tail += QString(" [message cut, %1 characters original]").arg(entry->logMessageLen);
            }

            lines.append(head + "." + prio + " >>> ] " + tail);
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

    // The Analyzer decides which rows belong to the call; the table only has to redraw,
    // because the mark is read back from the model row by row.
    if (mLogTable != nullptr)
    {
        mLogTable->viewport()->update();
    }
}

void LogViewerBase::applyRowHeight(QTableView* table)
{
    if (table == nullptr)
        return;

    const int height{ LusanApplication::getOptions().getLogRowHeight() };
    table->setProperty(LogViewerBase::PropertyLogTable, true);
    QHeaderView* rows{ table->verticalHeader() };
    rows->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    rows->setDefaultSectionSize(height);
    rows->setMinimumSectionSize(OptionsManager::LogRowHeightMin);
}

QIcon LogViewerBase::priorityDot(areg::LogPriority prio, const QWidget* on)
{
    constexpr int extent{ 12 };
    constexpr qreal inset{ 2.0 };

    const qreal ratio{ on != nullptr ? on->devicePixelRatioF() : qApp->devicePixelRatio() };
    QPixmap mark(QSize(qRound(extent * ratio), qRound(extent * ratio)));
    mark.setDevicePixelRatio(ratio);
    mark.fill(Qt::GlobalColor::transparent);

    QPainter painter(&mark);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);
    painter.setPen(Qt::PenStyle::NoPen);
    painter.setBrush(NELogPalette::railColor(NELogPalette::roleOf(prio)));
    painter.drawEllipse(QRectF(inset, inset, extent - (2 * inset), extent - (2 * inset)));
    painter.end();

    return QIcon(mark);
}

void LogViewerBase::refreshRowHeights(void)
{
    const bool wrap{ LusanApplication::getOptions().isLogWordWrap() };
    const QWidgetList widgets{ QApplication::allWidgets() };
    for (QWidget* widget : widgets)
    {
        QTableView* table{ qobject_cast<QTableView *>(widget) };
        if ((table != nullptr) && table->property(LogViewerBase::PropertyLogTable).toBool())
        {
            LogViewerBase::applyRowHeight(table);
        }
    }

    // The rows are given their one line height above, so the wrapped ones are measured after
    // it and not before.
    for (QWidget* widget : widgets)
    {
        LogViewerBase* viewer{ qobject_cast<LogViewerBase *>(widget) };
        if (viewer == nullptr)
            continue;

        viewer->setWordWrap(wrap);
        if (viewer->mHighlight != nullptr)
        {
            viewer->mHighlight->setWordWrap(wrap, LusanApplication::getOptions().getLogWrapLines());
        }

        viewer->_resetRowHeights();
        viewer->_measureShownRows();
    }
}

void LogViewerBase::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
}

inline void LogViewerBase::_clearResources()
{
    // The columns are read off the table, so this runs while the table is still alive.
    _saveLayout();

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
    mHitMap = nullptr;

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

    if (mHighlight != nullptr)
    {
        // A clock reading loses its meaning from its left end, so a narrow time column drops
        // the day and the hour rather than the seconds the reader came for.
        quint32 mask{ 0 };
        for (LoggingModelBase::eColumn column : { LoggingModelBase::eColumn::LogColumnTimestamp
                                                , LoggingModelBase::eColumn::LogColumnTimeReceived })
        {
            const int logical{ mHeader->getColumnIndex(column) };
            if ((logical >= 0) && (logical < 32))
            {
                mask |= (1u << logical);
            }
        }

        mHighlight->setElideLeftColumns(mask);
    }
}

void LogViewerBase::applyColumns(const QList<LoggingModelBase::eColumn>& columns)
{
    Q_ASSERT((mLogTable != nullptr) && (mLogModel != nullptr));

    const QList<LoggingModelBase::eColumn> wanted{ LoggingModelBase::shapeColumns(columns) };
    if (wanted == mLogModel->getActiveColumns())
        return;

    // A width belongs to the column, not to the place it stands in, and setting a model on a
    // view drops every section size. The widths are taken by column and given back after.
    QMap<int, int> widths;
    const QList<LoggingModelBase::eColumn> active{ mLogModel->getActiveColumns() };
    for (int i = 0; i < active.size(); ++i)
    {
        widths.insert(static_cast<int>(active.at(i)), mLogTable->columnWidth(i));
    }

    mLogTable->setModel(nullptr);
    mLogModel->setActiveColumns(wanted);
    mLogTable->setModel(mFilter);
    _bindSelection();

    for (int i = 0; i < wanted.size(); ++i)
    {
        const int width{ widths.value(static_cast<int>(wanted.at(i)), 0) };
        if (width > 0)
        {
            mLogTable->setColumnWidth(i, width);
        }
    }

    _updateHighlightColumn();
    _fitMessageColumn();
    _refitRowSelection();
    _measureShownRows();
}

void LogViewerBase::_fitMessageColumn()
{
    if ((mLogTable == nullptr) || (mLogModel == nullptr) || (mHeader == nullptr))
        return;

    const int message{ mHeader->getColumnIndex(LoggingModelBase::eColumn::LogColumnMessage) };
    if (message < 0)
        return;

    int taken{ 0 };
    const int columns{ mLogModel->columnCount(QModelIndex()) };
    for (int i = 0; i < columns; ++i)
    {
        if (i != message)
        {
            taken += mLogTable->columnWidth(i);
        }
    }

    const int room{ mLogTable->viewport()->width() - taken };
    if (room <= 0)
        return;

    // The window owns the width of the message column only while nothing else has changed it.
    // While it owns it, the column takes the room that is left and gives it back when the
    // viewport loses it -- which is what happens the moment the first rows bring the vertical
    // bar up and it takes its own width out of the viewport. A width the reader set, and the
    // room another column took, are both kept, and the table scrolls sideways instead.
    const int width{ mLogTable->columnWidth(message) };
    if ((width < room) || ((width == mFittedWidth) && (taken == mFittedTaken)))
    {
        mLogTable->setColumnWidth(message, room);
        mFittedWidth = room;
        mFittedTaken = taken;
    }
}

void LogViewerBase::_resetRowHeights()
{
    if (mLogTable == nullptr)
        return;

    const int height{ LusanApplication::getOptions().getLogRowHeight() };
    const int rows{ mLogTable->model() != nullptr ? mLogTable->model()->rowCount(QModelIndex()) : 0 };
    for (int row = 0; row < rows; ++row)
    {
        mLogTable->setRowHeight(row, height);
    }
}

void LogViewerBase::measureShownRows(QTableView* table, int column, int least, int maxLines)
{
    const QAbstractItemModel* model{ table != nullptr ? table->model() : nullptr };
    const int rows{ model != nullptr ? model->rowCount(QModelIndex()) : 0 };
    if ((rows <= 0) || (column < 0))
        return;

    const int width{ table->columnWidth(column) - 6 };
    const QFontMetrics metrics{ table->font() };
    const int space{ table->viewport()->height() };

    int row{ table->rowAt(0) };
    row = (row < 0 ? 0 : row);

    // A row that grows pushes the next one down, so the walk stops at the first row that no
    // longer reaches the viewport, and the rows below keep the height they have.
    for (int used = 0; (row < rows) && (used < space); ++row)
    {
        const QString text{ model->index(row, column).data(Qt::ItemDataRole::DisplayRole).toString() };
        const int height{ qMax(least, LogTextHighlight::wrappedHeight(text, metrics, width, maxLines)) };
        if (table->rowHeight(row) != height)
        {
            table->setRowHeight(row, height);
        }

        used += height;
    }
}

void LogViewerBase::_measureShownRows()
{
    if ((mWordWrap == false) || mMeasuring || (mLogTable == nullptr) || (mHeader == nullptr))
        return;

    mMeasuring = true;
    LogViewerBase::measureShownRows( mLogTable
                                   , mHeader->getColumnIndex(LoggingModelBase::eColumn::LogColumnMessage)
                                   , LusanApplication::getOptions().getLogRowHeight()
                                   , LusanApplication::getOptions().getLogWrapLines());
    mMeasuring = false;
}

void LogViewerBase::setWordWrap(bool wrap)
{
    if ((mLogTable == nullptr) || (mWordWrap == wrap))
        return;

    mWordWrap = wrap;
    mLogTable->setWordWrap(wrap);
    if (mHighlight != nullptr)
    {
        mHighlight->setWordWrap(wrap, LusanApplication::getOptions().getLogWrapLines());
    }

    if (wrap)
    {
        _measureShownRows();
    }
    else
    {
        _resetRowHeights();
    }

    if (mSessionBar != nullptr)
    {
        mSessionBar->setWordWrap(wrap);
    }

    mLogTable->viewport()->update();
}

bool LogViewerBase::filterCurrentColumn()
{
    if ((mHeader == nullptr) || (mLogTable == nullptr))
        return false;

    const QModelIndex current{ mLogTable->currentIndex() };
    LoggingModelBase::eColumn column{ current.isValid()
                                    ? mHeader->getColumn(current.column())
                                    : LoggingModelBase::eColumn::LogColumnMessage };

    // The rail carries no value, so it is not a column a filter can be written for. It is
    // also where a row that was revealed by the application leaves the current cell.
    if (LoggingModelBase::isPinnedColumn(column))
    {
        column = LoggingModelBase::eColumn::LogColumnMessage;
    }

    return mHeader->showFilterPanel(column);
}

void LogViewerBase::_filterByCell(LoggingModelBase::eColumn column, int row, bool exclude)
{
    if ((mHeader == nullptr) || (mFilter == nullptr) || (row < 0))
        return;

    const QModelIndex source{ mFilter->mapToSource(mFilter->index(row, 0, QModelIndex())) };
    const areg::LogEntry* entry{ source.isValid() ? mLogModel->getLogData(source.row()) : nullptr };
    if (entry != nullptr)
    {
        mHeader->pickValue(column, *entry, exclude);
    }
}

void LogViewerBase::_showFilterPanel()
{
    if ((mSessionBar == nullptr) || (mHeader == nullptr) || (mFilter == nullptr))
        return;

    if (mPickFilters == nullptr)
    {
        mPickFilters = new LogFilterPanel(this);
        connect(mPickFilters, &LogFilterPanel::signalClearFilters, this, [this]() { clearEveryFilter(); });
        connect(mPickFilters, &LogFilterPanel::signalOpenFilter, this, [this](int column, const QRect& anchor) {
                // One panel at a time. The column panel opens where its row stood, so the
                // reader keeps the place the list gave it.
                mPickFilters->hide();
                mHeader->showFilterPanelAt(static_cast<LoggingModelBase::eColumn>(column), anchor);
            });
    }

    // Every column that carries a panel is listed, whether or not the table shows it, so the
    // reader sees what can be narrowed instead of what happens to be on screen.
    LogFilterPanel::ListEntries entries;
    const QList<LoggingModelBase::eColumn> active{ mLogModel->getActiveColumns() };
    const LogViewerFilter::ListActiveFilters filters{ mFilter->activeFilters() };
    for (int i = 0; i < static_cast<int>(LoggingModelBase::eColumn::LogColumnCount); ++i)
    {
        const LoggingModelBase::eColumn column{ static_cast<LoggingModelBase::eColumn>(i) };
        if (mHeader->canFilter(column) == false)
            continue;

        LogFilterPanel::sEntry entry;
        entry.column = column;
        entry.shown  = active.contains(column);
        for (const LogViewerFilter::sActiveFilter& filter : filters)
        {
            if (filter.column == i)
            {
                entry.state = filter.text;
                break;
            }
        }

        entries.append(entry);
    }

    mPickFilters->setEntries(entries);

    const QWidget* button{ mSessionBar->ctrlFilters() };
    mPickFilters->showAt(QRect(button->mapToGlobal(QPoint(0, 0)), button->size()));
}

void LogViewerBase::_showColumnPicker(const QRect& anchor /*= QRect()*/)
{
    if ((mSessionBar == nullptr) || (mLogModel == nullptr))
        return;

    if (mPickColumns == nullptr)
    {
        mPickColumns = new LogColumnPicker(this);
        connect(mPickColumns, &LogColumnPicker::signalColumnsChanged, this
                , [this](const LogColumnPicker::ListColumns& columns) {
                    applyColumns(columns);
                });
        connect(mPickColumns, &LogColumnPicker::signalColumnsReset, this, [this]() {
                _forgetLayout();
                resetColumnOrder();
                moveToBottom(true);
            });
    }

    mPickColumns->setColumns(mLogModel->getActiveColumns());

    if (anchor.isEmpty() == false)
    {
        mPickColumns->showAt(anchor);
        return;
    }

    const QWidget* button{ mSessionBar->ctrlColumns() };
    mPickColumns->showAt(QRect(button->mapToGlobal(QPoint(0, 0)), button->size()));
}

void LogViewerBase::_refitRowSelection()
{
    // Re-selecting the same rows over their new columns is the window's own work.
    mFollowSelect = true;
    NELusanCommon::refitRowSelection(mLogTable);
    mFollowSelect = false;
}

void LogViewerBase::_bindSelection()
{
    if ((mLogTable == nullptr) || (mLogTable->selectionModel() == nullptr))
        return;

    connect(mLogTable->selectionModel(), &QItemSelectionModel::selectionChanged, this
            , [this](const QItemSelection& selected, const QItemSelection&) {
                // A row the user picks is a row the user wants to keep looking at, so the
                // table stops jumping to the newest log until the follow toggle is pressed again.
                if ((mFollowSelect == false) && (selected.isEmpty() == false) && (mSessionBar != nullptr))
                {
                    mSessionBar->setFollowing(false);
                }
            });
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
        if (LoggingModelBase::isPinnedColumn(col))
            continue; // the table places these itself, so they are not offered.

        bool isVisible = activeCols.contains(col);
        QAction* action = menu->addAction(headers[i]);
        action->setCheckable(true);
        action->setChecked(isVisible);
        action->setData(i); // Store index for later

        // The menu and the column panel take the same road, so a column shown from either
        // one lands in the same place and keeps the widths of the columns around it.
        connect(action, &QAction::triggered, this, [this, curRow, action](bool /*checked*/) {
                if (curRow < 0)
                    moveToBottom(false);

                const LoggingModelBase::eColumn column{ static_cast<LoggingModelBase::eColumn>(action->data().toInt()) };
                QList<LoggingModelBase::eColumn> columns{ mLogModel->getActiveColumns() };
                if (action->isChecked())
                    columns.insert(LoggingModelBase::placeOfColumn(columns, column), column);
                else
                    columns.removeAll(column);

                applyColumns(columns);
            });
    }

    QAction* actResetColumns = menu->addAction(tr("Reset Columns"));
    actResetColumns->setCheckable(false);
    connect(actResetColumns, &QAction::triggered, this, [this]() {
            _forgetLayout();
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
