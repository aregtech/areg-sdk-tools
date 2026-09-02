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
 *  \file        lusan/view/log/ScopeOutputViewer.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Scope output viewer widget.
 *
 ************************************************************************/
#include "lusan/view/log/ScopeOutputViewer.hpp"
#include "ui/ui_ScopeOutputViewer.h"

#include "lusan/model/log/ScopeLogViewerFilter.hpp"
#include "lusan/view/log/ScopeOutputDelegate.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/common/NETimeUnits.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/view/common/OutputDock.hpp"
#include "lusan/view/log/LogViewerBase.hpp"

#include "areg/logging/areg_log.h"

#include "lusan/app/LusanApplication.hpp"

#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QRadioButton>
#include <QScrollBar>
#include <QShortcut>
#include <QTableView>

#include <algorithm>

ScopeOutputViewer::ScopeOutputViewer(MdiMainWindow* wndMain, QWidget* parent)
    : OutputWindow  (static_cast<int>(OutputDock::eOutputDock::OutputLogging), wndMain, parent)
    , ui            (new Ui::ScopeOutputViewer)
    , mFilter       (new ScopeLogViewerFilter())
    , mLogModel     (nullptr)
    , mStructure    (new ScopeOutputDelegate(this))
    , mToolFold     (nullptr)
    , mToolPick     (nullptr)
    , mToolClock    (nullptr)
    , mSlowMenu     (nullptr)
    , mSlowUs       (ScopeOutputViewer::SlowCallUs)
{
    ui->setupUi(this);
    setupCallControls();
    ctrlTable()->setModel(nullptr);
    ctrlTable()->setItemDelegate(mStructure);
    LogViewerBase::applyRowHeight(ctrlTable());

    // The window holds one call, so the whole of a message is worth the height it takes and
    // the reader never has to scroll sideways to finish a line.
    ctrlTable()->setWordWrap(true);
    ctrlTable()->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
    ctrlTable()->viewport()->installEventFilter(this);
    ctrlTable()->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(ctrlTable(), &QTableView::customContextMenuRequested, this, &ScopeOutputViewer::onTableContextMenu);

    // The menu of the table names these keys, so the window answers them. Each one answers
    // only while the focus is inside this window: the log window carries the same keys.
    QShortcut* keyCopyMsg{ new QShortcut(QKeySequence::Copy, this) };
    QShortcut* keyCopyRow{ new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), this) };
    QShortcut* keyHide   { new QShortcut(QKeySequence::Delete, this) };
    for (QShortcut* key : { keyCopyMsg, keyCopyRow, keyHide })
    {
        key->setContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    }

    connect(keyCopyMsg, &QShortcut::activated, this, [this]() { copyRows(selectedRows(), true);  });
    connect(keyCopyRow, &QShortcut::activated, this, [this]() { copyRows(selectedRows(), false); });
    connect(keyHide   , &QShortcut::activated, this, [this]() { hideRows(selectedRows());        });

    QItemSelectionModel *selModel = ctrlTable()->selectionModel();
    Q_ASSERT(selModel != nullptr);
    connect(ctrlLogShow()       , &QToolButton::clicked     , this, [this]()              { onShowLog(getSelectedIndex());              });
    connect(ctrlScopeBegin()    , &QToolButton::clicked     , this, [this]()              { onShowLog(mFilter->getIndexStart(false));   });
    connect(ctrlScopeEnd()      , &QToolButton::clicked     , this, [this]()              { onShowLog(mFilter->getIndexEnd(false));     });
    connect(ctrlScopeNext()     , &QToolButton::clicked     , this, [this]()              { onShowNextLog();                            });
    connect(ctrlScopePrev()     , &QToolButton::clicked     , this, [this]()              { onShowPrevLog();                            });

    connect(ctrlTable()         , &QTableView::doubleClicked, this, [this](const QModelIndex& index)  {onShowLog(index);    });
    connect(ctrlTable()         , &QTableView::clicked      , this, [this](const QModelIndex& index)  {ctrlLogShow()->setEnabled(index.isValid());});
    connect(ctrlRadioSession()  , &QRadioButton::toggled    , this, [this](bool checked)  { onRadioChecked(checked, eRadioType::RadioSession);});
    connect(ctrlRadioSublogs()  , &QRadioButton::toggled    , this, [this](bool checked)  { onRadioChecked(checked, eRadioType::RadioSublogs);});
    connect(ctrlRadioScope()    , &QRadioButton::toggled    , this, [this](bool checked)  { onRadioChecked(checked, eRadioType::RadioScope);  });
    connect(ctrlRadioThread()   , &QRadioButton::toggled    , this, [this](bool checked)  { onRadioChecked(checked, eRadioType::RadioThread); });
    connect(ctrlRadioProcess()  , &QRadioButton::toggled    , this, [this](bool checked)  { onRadioChecked(checked, eRadioType::RadioProcess);});
    connect(mFilter, &ScopeLogViewerFilter::signalFilterSelected, this
            , [this](const QModelIndex& start, const QModelIndex& end) {
                onFilterChanged(start, end);
    });
    connect(mFilter, &QAbstractItemModel::modelReset, this, [this]() {
        ctrlDuration()->setText(QString("N/A"));
        shapeLogTable();
    });
    connect(ctrlTable()->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) { shapeLogTable(); });
    connect(mFilter, &QAbstractItemModel::columnsInserted, this
            , [this](const QModelIndex&, int, int) { NELusanCommon::refitRowSelection(ctrlTable()); });
    connect(mFilter, &QAbstractItemModel::columnsRemoved, this
            , [this](const QModelIndex&, int, int) { NELusanCommon::refitRowSelection(ctrlTable()); });
    connect(selModel            , &QItemSelectionModel::currentRowChanged, this
            , [this](const QModelIndex &current, const QModelIndex &previous){
                updateToolbuttons(mFilter->rowCount(), current);
    });

    updateControls(true);
}

ScopeOutputViewer::~ScopeOutputViewer()
{
    ctrlTable()->setItemDelegate(nullptr);
    ctrlTable()->setModel(nullptr);
    if (mFilter != nullptr)
    {
        mFilter->setSourceModel(nullptr);
        delete mFilter;
        mFilter = nullptr;
    }
    
    delete ui;
    ui = nullptr;
}

bool ScopeOutputViewer::releaseWindow(MdiChild& mdiChild)
{
    bool result = OutputWindow::releaseWindow(mdiChild);
    if (result)
    {
        // Dropping the log takes the columns of the table with it, and the table answers
        // that with a resize. The window lets the table go first, so nothing shapes it
        // against a log it no longer reads.
        mLogModel = nullptr;
        ctrlTable()->setModel(nullptr);
        if (mFilter != nullptr)
        {
            mFilter->setScopeFilter(nullptr, 0, 0, 0, 0);
        }

        updateLogTable();
    }

    return result;
}

void ScopeOutputViewer::setupFilter(LoggingModelBase* logModel, uint32_t scopeId, uint32_t sessionId, ITEM_ID instance)
{
    if (mFilter == nullptr)
    {
        ctrlTable()->setModel(nullptr);
    }
    else
    {
        mLogModel = logModel;
        mFilter->setScopeFilter(logModel, scopeId, sessionId, 0, instance);
        if (ctrlTable()->model() == nullptr)
            ctrlTable()->setModel(logModel == nullptr ? nullptr : mFilter);
    }

    updateLogTable();
}

void ScopeOutputViewer::setupFilter(LoggingModelBase* logModel, const QModelIndex& index)
{
    if (mFilter == nullptr)
    {
        ctrlTable()->setModel(nullptr);
    }
    else
    {
        mLogModel = logModel;
        mFilter->setScopeFilter(logModel, index);
        if (ctrlTable()->model() == nullptr)
            ctrlTable()->setModel(logModel == nullptr ? nullptr : mFilter);
    }
    
    updateLogTable();
}

void ScopeOutputViewer::analyzeAt(LoggingModelBase* logModel, const QModelIndex& index, ScopeLogViewerFilter::eDataFilter mode)
{
    setupFilter(logModel, index);
    if ((mFilter == nullptr) || (logModel == nullptr))
        return;

    QRadioButton* button{ nullptr };
    switch (mode)
    {
    case ScopeLogViewerFilter::eDataFilter::FilterSublogs:
        button = ctrlRadioSublogs();
        break;

    case ScopeLogViewerFilter::eDataFilter::FilterScope:
        button = ctrlRadioScope();
        break;

    case ScopeLogViewerFilter::eDataFilter::FilterThread:
        button = ctrlRadioThread();
        break;

    case ScopeLogViewerFilter::eDataFilter::FilterProcess:
        button = ctrlRadioProcess();
        break;

    case ScopeLogViewerFilter::eDataFilter::FilterSession:
    default:
        button = ctrlRadioSession();
        break;
    }

    // A button that already stands checked sends nothing, so the filter is told directly.
    if ((button != nullptr) && (button->isChecked() == false))
    {
        button->setChecked(true);
    }
    else
    {
        mFilter->filterData(mode);
        updateLogTable();
    }
}

void ScopeOutputViewer::analyzeRows(LoggingModelBase* logModel, const QList<int>& sourceRows)
{
    if (mFilter == nullptr)
    {
        ctrlTable()->setModel(nullptr);
        return;
    }

    mLogModel = logModel;
    mFilter->setRowFilter(logModel, sourceRows);
    if (ctrlTable()->model() == nullptr)
    {
        ctrlTable()->setModel(logModel == nullptr ? nullptr : mFilter);
    }

    // The picked rows belong to no one call, so none of the radios names what is shown.
    updateLogTable();
    ctrlRadioSession()->setEnabled(false);
    ctrlRadioSublogs()->setEnabled(false);
    ctrlRadioScope()->setEnabled(false);
    ctrlRadioThread()->setEnabled(false);
    ctrlRadioProcess()->setEnabled(false);
}

QList<int> ScopeOutputViewer::selectedRows(void) const
{
    QList<int> result;
    const QTableView* table{ ctrlTable() };
    const QItemSelectionModel* selection{ table != nullptr ? table->selectionModel() : nullptr };
    if (selection == nullptr)
        return result;

    for (const QModelIndex& index : selection->selectedRows())
    {
        result.append(index.row());
    }

    if (result.isEmpty() && selection->currentIndex().isValid())
    {
        result.append(selection->currentIndex().row());
    }

    std::sort(result.begin(), result.end());
    return result;
}

void ScopeOutputViewer::copyRows(const QList<int>& rows, bool messageOnly) const
{
    if ((mFilter == nullptr) || (mLogModel == nullptr) || rows.isEmpty())
        return;

    const int message{ mLogModel->fromColumnToIndex(LoggingModelBase::eColumn::LogColumnMessage) };
    const int columns{ mFilter->columnCount() };
    QString text;
    for (int row : rows)
    {
        if (messageOnly)
        {
            if (message >= 0)
            {
                text += mFilter->index(row, message).data(Qt::ItemDataRole::DisplayRole).toString();
                text += QLatin1Char('\n');
            }

            continue;
        }

        QStringList cells;
        for (int column = 0; column < columns; ++column)
        {
            if (column != mLogModel->fromColumnToIndex(LoggingModelBase::eColumn::LogColumnRail))
            {
                cells.append(mFilter->index(row, column).data(Qt::ItemDataRole::DisplayRole).toString());
            }
        }

        text += cells.join(QLatin1Char('\t'));
        text += QLatin1Char('\n');
    }

    if (text.isEmpty() == false)
    {
        QApplication::clipboard()->setText(text);
    }
}

void ScopeOutputViewer::hideRows(const QList<int>& rows)
{
    if ((mFilter == nullptr) || rows.isEmpty())
        return;

    QList<int> sourceRows;
    sourceRows.reserve(rows.size());
    for (int row : rows)
    {
        const QModelIndex source{ mFilter->mapToSource(mFilter->index(row, 0)) };
        if (source.isValid())
        {
            sourceRows.append(source.row());
        }
    }

    mFilter->hideRows(sourceRows);
    updateLogTable();
}

void ScopeOutputViewer::onTableContextMenu(const QPoint& pos)
{
    QTableView* table{ ctrlTable() };
    if ((table == nullptr) || (mFilter == nullptr))
        return;

    QModelIndex clicked{ table->indexAt(pos) };
    if (clicked.isValid())
    {
        // The menu speaks about the row it opened on, so a right click also moves the cursor.
        table->selectionModel()->setCurrentIndex(clicked, QItemSelectionModel::NoUpdate);
    }
    else
    {
        clicked = getSelectedIndex();
    }

    const QList<int> rows{ selectedRows() };
    const bool hasRows{ rows.isEmpty() == false };

    // The row is carried by number, not by index: the view can be filtered again while the
    // menu stands open, and an index does not survive that.
    const int clickedRow{ clicked.isValid() ? clicked.row() : -1 };

    QMenu menu(this);

    QAction* reveal = menu.addAction(tr("Show in the log window"));
    reveal->setEnabled((clickedRow >= 0) && (mMdiChild != nullptr));
    connect(reveal, &QAction::triggered, this, [this, clickedRow]() { onShowLog(mFilter->index(clickedRow, 0)); });

    menu.addSeparator();

    QAction* copyMsg = menu.addAction(tr("Copy message"));
    copyMsg->setShortcut(QKeySequence::Copy);
    copyMsg->setEnabled(hasRows);
    connect(copyMsg, &QAction::triggered, this, [this, rows]() { copyRows(rows, true); });

    QAction* copyRow = menu.addAction(tr("Copy row"));
    copyRow->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    copyRow->setEnabled(hasRows);
    connect(copyRow, &QAction::triggered, this, [this, rows]() { copyRows(rows, false); });

    menu.addSeparator();

    QAction* hide = menu.addAction(rows.size() > 1 ? tr("Hide these rows") : tr("Hide this row"));
    hide->setShortcut(QKeySequence::Delete);
    hide->setEnabled(hasRows);
    connect(hide, &QAction::triggered, this, [this, rows]() { hideRows(rows); });

    QAction* unhide = menu.addAction(tr("Show the hidden rows"));
    unhide->setEnabled(mFilter->hasHiddenRows());
    connect(unhide, &QAction::triggered, this, [this]() {
            mFilter->showHiddenRows();
            updateLogTable();
        });

    menu.addSeparator();

    QAction* fold = menu.addAction(tr("Fold or open this call"));
    fold->setEnabled((clickedRow >= 0)
                     && (clicked.data(ScopeLogViewerFilter::RoleCallFold).toInt()
                         != ScopeLogViewerFilter::eCallFold::FoldNone));
    connect(fold, &QAction::triggered, this, [this, clickedRow]() {
            if (mFilter->toggleFold(mFilter->index(clickedRow, 0)))
            {
                updateLogTable();
            }
        });

    QAction* quiet = menu.addAction(tr("Fold the quiet calls"));
    quiet->setCheckable(true);
    quiet->setChecked(mFilter->isAutoFold());
    connect(quiet, &QAction::triggered, this, [this](bool checked) { onAutoFoldToggled(checked); });

    QAction* worth = menu.addAction(tr("Keep only what is worth reading"));
    worth->setCheckable(true);
    worth->setChecked(mFilter->isInterestingOnly());
    connect(worth, &QAction::triggered, this, [this](bool checked) { onInterestingToggled(checked); });

    QAction* since = menu.addAction(tr("Count the time from the entry of the call"));
    since->setCheckable(true);
    since->setChecked(mFilter->isRelativeTime());
    connect(since, &QAction::triggered, this, [this](bool checked) { onRelativeTimeToggled(checked); });

    for (QAction* action : menu.actions())
    {
        action->setShortcutVisibleInContextMenu(true);
    }

    menu.exec(table->viewport()->mapToGlobal(pos));
}

void ScopeOutputViewer::onRadioChecked(bool checked, eRadioType radio)
{
    if ((mFilter == nullptr) || (checked == false))
        return;

    switch (radio)
    {
    case eRadioType::RadioSession:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::FilterSession);
        break;
    
    case eRadioType::RadioSublogs:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::FilterSublogs);
        break;

    case eRadioType::RadioScope:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::FilterScope);
        break;

    case eRadioType::RadioThread:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::FilterThread);
        break;

    case eRadioType::RadioProcess:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::FilterProcess);
        break;

    case eRadioType::RadioNone:
    default:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::NoFilter);
        break;
    }
}

void ScopeOutputViewer::setupCallControls(void)
{
    QLayout* row{ ui->horizontalLayout };
    Q_ASSERT(row != nullptr);

    auto build = [this, row](const QIcon& icon, const QString& text, const QString& tip) -> QToolButton*
    {
        QToolButton* button = new QToolButton(this);
        button->setIcon(icon);
        button->setText(text);
        button->setToolTip(tip);
        button->setStatusTip(tip);
        button->setAccessibleName(text);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
        row->addWidget(button);
        return button;
    };

    mToolFold = build( NELusanCommon::iconNodeCollapsed(NELusanCommon::SizeBig)
                     , tr("Fold the quiet calls")
                     , tr("Folds every call that carries nothing above Information. A folded call shows only the line that opened it, with the time it took."));

    mToolPick = build( NELusanCommon::iconFilter(NELusanCommon::SizeBig)
                     , tr("Only what is worth reading")
                     , QString());

    mSlowMenu = new QMenu(mToolPick);
    QActionGroup* steps = new QActionGroup(mSlowMenu);
    steps->setExclusive(true);
    for (uint32_t step : ScopeOutputViewer::SlowCallSteps)
    {
        QAction* entry = mSlowMenu->addAction(step == 0u
                                                ? tr("Warnings and worse only")
                                                : tr("Also calls slower than %1").arg(NETimeUnits::duration(step)));
        entry->setCheckable(true);
        entry->setChecked(step == mSlowUs);
        entry->setData(step);
        steps->addAction(entry);
    }

    connect(steps, &QActionGroup::triggered, this, [this](QAction* entry) {
        onSlowStepChosen(entry != nullptr ? entry->data().toUInt() : ScopeOutputViewer::SlowCallUs);
    });

    NELusanCommon::decorateToolButton(mToolPick, mSlowMenu);
    refreshSlowTip();

    mToolClock = build( NELusanCommon::iconTimer(NELusanCommon::SizeBig)
                      , tr("Time since the call started")
                      , tr("The time column counts from the moment the call the row belongs to was entered, instead of showing the time of day."));

    connect(mToolFold , &QToolButton::toggled, this, [this](bool checked) { onAutoFoldToggled(checked);      });
    connect(mToolPick , &QToolButton::toggled, this, [this](bool checked) { onInterestingToggled(checked);   });
    connect(mToolClock, &QToolButton::toggled, this, [this](bool checked) { onRelativeTimeToggled(checked);  });
}

void ScopeOutputViewer::onSlowStepChosen(uint32_t slowUs)
{
    if (mSlowUs == slowUs)
        return;

    mSlowUs = slowUs;
    refreshSlowTip();
    if ((mFilter != nullptr) && mToolPick->isChecked())
    {
        mFilter->setInterestingOnly(true, mSlowUs);
        refreshCallControls();
    }
}

void ScopeOutputViewer::refreshSlowTip(void)
{
    const QString tip{ mSlowUs == 0u
                        ? tr("Keeps the entries of Warning priority or worse, and the calls that carry one.")
                        : tr("Keeps the entries of Warning priority or worse, and the calls that carry one or ran longer than %1.")
                            .arg(NETimeUnits::duration(mSlowUs)) };
    mToolPick->setToolTip(tip);
    mToolPick->setStatusTip(tip);
}

void ScopeOutputViewer::refreshCallControls(void)
{
    const bool hasRows{ (mFilter != nullptr) && (mFilter->rowCount() != 0) };
    mToolFold->setEnabled(hasRows);
    mToolPick->setEnabled(hasRows);
    mToolClock->setEnabled(hasRows);
}

void ScopeOutputViewer::onAutoFoldToggled(bool checked)
{
    if (mFilter != nullptr)
    {
        mFilter->setAutoFold(checked);
        refreshCallControls();
    }
}

void ScopeOutputViewer::onInterestingToggled(bool checked)
{
    if (mFilter != nullptr)
    {
        mFilter->setInterestingOnly(checked, mSlowUs);
        refreshCallControls();
    }
}

void ScopeOutputViewer::onRelativeTimeToggled(bool checked)
{
    if (mFilter != nullptr)
    {
        mFilter->setRelativeTime(checked);
    }
}

void ScopeOutputViewer::onFilterChanged(const QModelIndex & indexStart, const QModelIndex& indexEnd)
{
    ctrlDuration()->setText(QString("N/A"));
    if (indexEnd.isValid())
    {
        const areg::LogEntry * log = mLogModel != nullptr ? mLogModel->data(indexEnd, static_cast<int>(Qt::UserRole)).value<const areg::LogEntry *>() : nullptr;
        if (log != nullptr)
        {
            ctrlDuration()->setText(NETimeUnits::duration(log->logDuration));
        }
    }
}

inline QTableView* ScopeOutputViewer::ctrlTable() const
{
    return ui->logTable;
}

inline QRadioButton* ScopeOutputViewer::ctrlRadioSession() const
{
    return ui->radioSession;
}

inline QRadioButton* ScopeOutputViewer::ctrlRadioSublogs() const
{
    return ui->radioSublogs;
}

inline QRadioButton* ScopeOutputViewer::ctrlRadioScope() const
{
    return ui->radioScope;
}

inline QRadioButton* ScopeOutputViewer::ctrlRadioThread() const
{
    return ui->radioThread;
}

inline QRadioButton* ScopeOutputViewer::ctrlRadioProcess() const
{
    return ui->radioProcess;
}

inline QLineEdit* ScopeOutputViewer::ctrlDuration() const
{
    return ui->editDuration;
}

inline QToolButton* ScopeOutputViewer::ctrlLogShow() const
{
    return ui->toolLogShow;
}

inline QToolButton* ScopeOutputViewer::ctrlScopeBegin() const
{
    return ui->toolScopeBegin;
}

inline QToolButton* ScopeOutputViewer::ctrlScopeEnd() const
{
    return ui->toolScopeEnd;
}

inline QToolButton* ScopeOutputViewer::ctrlScopeNext() const
{
    return ui->toolScopeNext;
}

inline QToolButton* ScopeOutputViewer::ctrlScopePrev() const
{
    return ui->toolScopePrev;
}

void ScopeOutputViewer::shapeLogTable()
{
    QTableView* table{ ctrlTable() };
    if ((table == nullptr) || (mLogModel == nullptr) || (table->model() == nullptr))
        return;

    // The table loses its sections the moment the log is dropped, and the resize it answers
    // with reaches this call while the log is still named here. A section is only shaped
    // when the header holds it.
    QHeaderView* header{ table->horizontalHeader() };
    const int sections{ header != nullptr ? header->count() : 0 };
    if (sections <= 0)
        return;

    // The structure of the call is drawn in the leading column, which the table keeps at its
    // own width and never moves.
    const int rail{ mLogModel->fromColumnToIndex(LoggingModelBase::eColumn::LogColumnRail) };
    if ((rail >= 0) && (rail < sections))
    {
        header->setSectionResizeMode(rail, QHeaderView::ResizeMode::Fixed);
        table->setColumnWidth(rail, LoggingModelBase::RailWidth);
    }

    LogViewerBase::measureShownRows( table
                                   , mLogModel->fromColumnToIndex(LoggingModelBase::eColumn::LogColumnMessage)
                                   , LusanApplication::getOptions().getLogRowHeight()
                                   , LusanApplication::getOptions().getLogWrapLines());
}

bool ScopeOutputViewer::eventFilter(QObject* watched, QEvent* event)
{
    if ((event != nullptr) && (event->type() == QEvent::Type::Resize)
        && (ctrlTable() != nullptr) && (watched == ctrlTable()->viewport()))
    {
        shapeLogTable();
    }

    return OutputWindow::eventFilter(watched, event);
}

inline void ScopeOutputViewer::updateLogTable()
{
    QTableView *logTable = mMdiChild != nullptr ? static_cast<LogViewerBase *>(mMdiChild)->getLoggingTable() : nullptr;
    if (logTable != nullptr)
    {
        logTable->viewport()->update();
    }

    shapeLogTable();
    updateControls(true);
}

inline void ScopeOutputViewer::updateControls(bool selectSession)
{
    int count{ mFilter != nullptr ? mFilter->rowCount() : 0 };
    bool hasEntries{count != 0};
    blockSignals(true);
    
    if (hasEntries == false)
    {
        ctrlRadioSession()->setChecked(false);
        ctrlRadioSublogs()->setChecked(false);
        ctrlRadioScope()->setChecked(false);
        ctrlRadioThread()->setChecked(false);
        ctrlRadioProcess()->setChecked(false);
    }
    else if (selectSession)
    {
        ctrlRadioSession()->setChecked(true);
    }

    ctrlRadioSession()->setEnabled(hasEntries);
    ctrlRadioSublogs()->setEnabled(hasEntries);
    ctrlRadioScope()->setEnabled(hasEntries);
    ctrlRadioThread()->setEnabled(hasEntries);
    ctrlRadioProcess()->setEnabled(hasEntries);

    updateToolbuttons(count, getSelectedIndex());
    refreshCallControls();

    blockSignals(false);
}

inline void ScopeOutputViewer::updateToolbuttons(int rowCount, const QModelIndex& selIndex)
{
    if (selIndex.isValid() == false)
    {
        ctrlDuration()->setText(QString("N/A"));
    }
    
    bool hasEntries(rowCount != 0);
    QModelIndex start = mFilter->getIndexStart(true);
    QModelIndex end = mFilter->getIndexEnd(true);
    
    ctrlLogShow()->setEnabled(selIndex.isValid());
    ctrlScopeBegin()->setEnabled(start.isValid() && (selIndex != start));
    ctrlScopeEnd()->setEnabled(end.isValid() && (selIndex != end));
    ctrlScopeNext()->setEnabled(hasEntries && ((selIndex.isValid() == false) || (selIndex.row() < (rowCount - 1))));
    ctrlScopePrev()->setEnabled(hasEntries && ((selIndex.isValid() == false) || (selIndex.row() > 0)));
}

void ScopeOutputViewer::onShowLog(const QModelIndex& idxTarget)
{
    if ((mMdiChild != nullptr) && (mFilter != nullptr))
    {
        if (idxTarget.isValid())
        {
            blockSignals(true);
            QTableView* table = ctrlTable();
            table->selectionModel()->setCurrentIndex(idxTarget, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            table->selectionModel()->select(idxTarget, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            table->selectRow(idxTarget.row());
            table->scrollTo(idxTarget);
            
            QModelIndex srcIndex = mFilter->mapToSource(idxTarget);
            static_cast<LogViewerBase*>(mMdiChild)->selectSourceElement(srcIndex);
            updateToolbuttons(mFilter->rowCount(), idxTarget);
            blockSignals(false);
        }
    }
}

void ScopeOutputViewer::onShowNextLog()
{
    QModelIndex idxTarget = getSelectedIndex();
    idxTarget = mFilter->getIndexNextScope(idxTarget, false);
    onShowLog(idxTarget.isValid() ? idxTarget : mFilter->index(mFilter->rowCount() - 1, 0));
}

void ScopeOutputViewer::onShowPrevLog()
{
    QModelIndex idxTarget = getSelectedIndex();
    idxTarget = mFilter->getIndexPrevScope(idxTarget, false);
    onShowLog(idxTarget.isValid() ? idxTarget : mFilter->index(0, 0));
}

inline QModelIndex ScopeOutputViewer::getSelectedIndex() const
{
    return ctrlTable()->selectionModel()->currentIndex();
}
