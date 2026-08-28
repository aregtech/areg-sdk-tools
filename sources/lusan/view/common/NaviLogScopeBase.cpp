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
 *  \file        lusan/view/common/NaviLogScopeBase.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The base class of the log explorer view.
 *
 ************************************************************************/

#include "lusan/view/common/NaviLogScopeBase.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/log/ScopeNodeBase.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/LoggingScopesModelBase.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/ScopeNameDelegate.hpp"
#include "lusan/view/common/SearchLineEdit.hpp"
#include "lusan/view/log/LogPriorityBar.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QMenu>
#include <QPoint>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSize>
#include <QSizePolicy>
#include <QStringList>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidgetAction>

NaviLogScopeBase::NaviLogScopeBase(int naviWindow, MdiMainWindow* wndMain, QWidget* parent)
    : NaviToolbarWindow (naviWindow, wndMain, parent)

    , mScopesModel      (nullptr)
    , mSelModel         (nullptr)
    , mToolCollapse     (nullptr)
    , mToolFind         (nullptr)
    , mPrioBar          (nullptr)
    , mToolShowOnly     (nullptr)
    , mToolHide         (nullptr)
    , mToolShowAll      (nullptr)
    , mScopeReach       (eScopeReach::ReachBranch)

    , mFilterBar        (nullptr)
    , mFilterEdit       (nullptr)
    , mFilterCount      (nullptr)
    , mFindBar          (nullptr)
    , mFindEdit         (nullptr)
    , mFindCount        (nullptr)
    , mHighlight        (nullptr)
    , mFindAt           ( )

    , mGuardBar         (nullptr)
    , mBelowRow         (nullptr)
    , mBelowText        (nullptr)
    , mRaiseRow         (nullptr)
    , mRaiseText        (nullptr)
    , mTempRaise        (false)
    , mPrioTip          ( )
{
}

void NaviLogScopeBase::setupScopeToolbar(void)
{
    mToolCollapse = addToolButton( NELusanCommon::iconNodeExpanded(NELusanCommon::SizeBig)
                                 , tr("Collapse or expand log scopes")
                                 , tr("Collapse or expand log scopes.")
                                 , true);
    mToolCollapse->setStyleSheet(NELusanCommon::getStyleToolbutton());

    addSpecificTools();

    mToolFind = addToolButton( NELusanCommon::iconSearch(NELusanCommon::SizeBig)
                             , tr("Find a scope")
                             , tr("Walk from one scope whose name carries the text to the next.")
                             , true);

    addToolSeparator();

    mPrioBar = new LogPriorityBar(this);
    mPrioBar->setToolTip(tr("Set how much the selected scopes generate. The last cell switches the enter and exit lines."));
    mPrioTip = mPrioBar->toolTip();
    mPrioBar->setStatusTip(mPrioTip);
    mPrioBar->setAccessibleName(tr("Scope priority"));
    mPrioBar->setIdle(true);
    mPrioBar->setEnabled(false);
    addToolWidget(mPrioBar);

    addToolSeparator();

    mToolShowOnly = addToolButton( NELusanCommon::iconScopeSolo(NELusanCommon::SizeBig)
                                 , tr("Show only the selected scopes")
                                 , tr("Show only the selected scopes and hide every other one."));
    mToolHide     = addToolButton( NELusanCommon::iconScopeMute(NELusanCommon::SizeBig)
                                 , tr("Hide the selected scopes")
                                 , tr("Hide the selected scopes."));
    mToolShowAll  = addToolButton( NELusanCommon::iconScopeRestoreAll(NELusanCommon::SizeBig)
                                 , tr("Show all scopes")
                                 , tr("Show every scope again."));

    addToolSeparator();

    addMoveTools();

    // A narrow dock gives the row up from the least used entry. The priority ladder never
    // moves: without it the panel is a tree with a connect button.
    setToolRank(mToolCollapse, 10);
    setToolRank(mToolShowAll , 20);
    setToolRank(mToolHide    , 30);
    setToolRank(mToolShowOnly, 40);
    setToolRank(mToolFind    , 50);

    setupTreeView(QSize(NaviLogScopeBase::ScopeIconExtent, NaviLogScopeBase::ScopeIconExtent));
    ctrlTable()->setRootIsDecorated(false);
    ctrlTable()->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    setupShowColumn();
    setupSafeguards();
    setupScopeSearch();

    capToolButtonIconSizes();
}

void NaviLogScopeBase::setupSafeguards(void)
{
    mGuardBar = new QWidget(this);
    QVBoxLayout* guards = new QVBoxLayout(mGuardBar);
    guards->setContentsMargins(0, 0, 0, 0);
    guards->setSpacing(2);

    mBelowRow = new QWidget(mGuardBar);
    QHBoxLayout* belowRow = new QHBoxLayout(mBelowRow);
    belowRow->setContentsMargins(0, 0, 0, 0);
    belowRow->setSpacing(4);

    QLabel* belowIcon = new QLabel(mBelowRow);
    belowIcon->setPixmap(NELusanCommon::iconWarning(NELusanCommon::SizeSmall).pixmap(NELusanCommon::SizeSmall));

    mBelowText = new QLabel(mBelowRow);
    mBelowText->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Preferred);

    QToolButton* restore = new QToolButton(mBelowRow);
    restore->setText(tr("Restore all"));
    restore->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
    restore->setAutoRaise(true);
    restore->setToolTip(tr("Put every scope back to the priority its target started with."));
    restore->setAccessibleName(restore->toolTip());

    belowRow->addWidget(belowIcon, 0);
    belowRow->addWidget(mBelowText, 1);
    belowRow->addWidget(restore, 0);

    mRaiseRow = new QWidget(mGuardBar);
    QHBoxLayout* raiseRow = new QHBoxLayout(mRaiseRow);
    raiseRow->setContentsMargins(0, 0, 0, 0);
    raiseRow->setSpacing(4);

    QLabel* raiseIcon = new QLabel(mRaiseRow);
    raiseIcon->setPixmap(NELusanCommon::iconTimer(NELusanCommon::SizeSmall).pixmap(NELusanCommon::SizeSmall));

    mRaiseText = new QLabel(mRaiseRow);
    mRaiseText->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Preferred);

    QToolButton* keep = new QToolButton(mRaiseRow);
    keep->setText(tr("Keep"));
    keep->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
    keep->setAutoRaise(true);
    keep->setToolTip(tr("Leave the raised priorities in place instead of letting them go back."));
    keep->setAccessibleName(keep->toolTip());

    raiseRow->addWidget(raiseIcon, 0);
    raiseRow->addWidget(mRaiseText, 1);
    raiseRow->addWidget(keep, 0);

    guards->addWidget(mBelowRow);
    guards->addWidget(mRaiseRow);
    addNaviBar(mGuardBar);
    mGuardBar->setVisible(false);

    connect(restore, &QToolButton::clicked, this, [this]() {
        if (mScopesModel != nullptr)
        {
            mScopesModel->restoreDefaults();
            refreshSafeguards();
        }
    });

    connect(keep, &QToolButton::clicked, this, [this]() {
        if (mScopesModel != nullptr)
        {
            mScopesModel->keepTempRaises();
            refreshSafeguards();
        }
    });
}

void NaviLogScopeBase::refreshSafeguards(void)
{
    if ((mGuardBar == nullptr) || (mScopesModel == nullptr))
        return;

    QMap<QString, int> perProcess;
    const int below{ mScopesModel->countBelowDefault(perProcess) };
    const int raised{ mScopesModel->tempRaiseCount() };

    if (below != 0)
    {
        mBelowText->setText(tr("%1 scopes below default").arg(below));
        QStringList where;
        for (auto entry = perProcess.constBegin(); entry != perProcess.constEnd(); ++entry)
        {
            where.append(tr("%1: %2").arg(entry.key()).arg(entry.value()));
        }

        mBelowText->setToolTip(where.join(QStringLiteral("\n")));
    }

    if (raised != 0)
    {
        mRaiseText->setText(tr("%1 raised, going back").arg(raised));
        mRaiseText->setToolTip(tr("These scopes go back to what they generated before, on their own."));
    }

    mBelowRow->setVisible(below != 0);
    mRaiseRow->setVisible(raised != 0);
    mGuardBar->setVisible((below != 0) || (raised != 0));
}

void NaviLogScopeBase::setupScopeSearch(void)
{
    QTreeView* tree = ctrlTable();
    Q_ASSERT(tree != nullptr);

    mHighlight = new ScopeNameDelegate(this);
    tree->setItemDelegateForColumn(LoggingScopesModelBase::ColumnName, mHighlight);

    mFilterBar = new QWidget(this);
    mFilterBar->setObjectName(QStringLiteral("scopeFilterBar"));
    QHBoxLayout* filterRow = new QHBoxLayout(mFilterBar);
    filterRow->setContentsMargins(0, 0, 0, 0);
    filterRow->setSpacing(4);

    mFilterEdit = new QLineEdit(mFilterBar);
    mFilterEdit->setObjectName(QStringLiteral("scopeFilterEdit"));
    mFilterEdit->addAction(NELusanCommon::iconFilter(NELusanCommon::SizeSmall), QLineEdit::ActionPosition::LeadingPosition);
    mFilterEdit->setClearButtonEnabled(true);
    mFilterEdit->setPlaceholderText(tr("Filter scopes"));
    mFilterEdit->setToolTip(tr("Leave in the tree only the scopes whose name carries this text."));
    mFilterEdit->setStatusTip(mFilterEdit->toolTip());
    mFilterEdit->setAccessibleName(tr("Scope name filter"));
    // The dock is narrow. Without this the box holds its own width and pushes the count out.
    mFilterEdit->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Fixed);

    mFilterCount = new QLabel(mFilterBar);
    mFilterCount->setObjectName(QStringLiteral("scopeFilterCount"));
    mFilterCount->setEnabled(false);
    mFilterCount->setVisible(false);

    filterRow->addWidget(mFilterEdit, 1);
    filterRow->addWidget(mFilterCount, 0);
    addNaviBar(mFilterBar);

    const QList<SearchLineEdit::eToolButton> findTools{ SearchLineEdit::eToolButton::ToolButtonMatchCase
                                                      , SearchLineEdit::eToolButton::ToolButtonMatchWord
                                                      , SearchLineEdit::eToolButton::ToolButtonBackward
                                                      , SearchLineEdit::eToolButton::ToolButtonSearch };

    mFindBar = new QWidget(this);
    mFindBar->setObjectName(QStringLiteral("scopeFindBar"));
    QHBoxLayout* findRow = new QHBoxLayout(mFindBar);
    findRow->setContentsMargins(0, 0, 0, 0);
    findRow->setSpacing(4);

    mFindEdit = new SearchLineEdit(findTools, QSize(18, 18), mFindBar);
    mFindEdit->setObjectName(QStringLiteral("scopeFindEdit"));
    mFindEdit->setPlaceholderText(tr("Find scope"));
    mFindEdit->setToolTip(tr("Walk to the next scope whose name carries this text. The tree is left whole."));
    mFindEdit->setStatusTip(mFindEdit->toolTip());
    mFindEdit->setAccessibleName(tr("Find scope"));
    mFindEdit->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Fixed);
    mFindEdit->installEventFilter(this);

    mFindCount = new QLabel(mFindBar);
    mFindCount->setEnabled(false);

    QToolButton* findClose = new QToolButton(mFindBar);
    findClose->setIcon(NELusanCommon::iconClose(NELusanCommon::SizeSmall));
    findClose->setAutoRaise(true);
    findClose->setToolTip(tr("Close the find row"));
    findClose->setAccessibleName(findClose->toolTip());

    findRow->addWidget(mFindEdit, 1);
    findRow->addWidget(mFindCount, 0);
    findRow->addWidget(findClose, 0);
    addNaviBar(mFindBar);
    mFindBar->setVisible(false);

    connect(mFilterEdit, &QLineEdit::textChanged, this, [this](const QString&) { applyScopeFilter(); });
    connect(mToolFind  , &QToolButton::clicked  , this, [this](bool checked) { showScopeFind(checked); });
    connect(findClose  , &QToolButton::clicked  , this, [this]() { showScopeFind(false); });
    connect(mFindEdit  , &SearchLineEdit::signalSearchTextChanged, this, [this](const QString&) {
        mFindAt = QPersistentModelIndex();
        findScope(1);
    });
    connect(mFindEdit  , &SearchLineEdit::signalSearchText, this, [this](const QString&, bool, bool, bool, bool backward) {
        findScope(backward ? -1 : 1);
    });

    QShortcut* openFind = new QShortcut(QKeySequence::StandardKey::Find, this);
    openFind->setContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    connect(openFind, &QShortcut::activated, this, [this]() { showScopeFind(true); });

    QShortcut* focusFilter = new QShortcut(QKeySequence(tr("Ctrl+Shift+F")), this);
    focusFilter->setContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    connect(focusFilter, &QShortcut::activated, this, [this]() {
        mFilterEdit->setFocus();
        mFilterEdit->selectAll();
    });

    QShortcut* nextHit = new QShortcut(QKeySequence(Qt::Key::Key_F3), this);
    nextHit->setContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    connect(nextHit, &QShortcut::activated, this, [this]() { findScope(1); });

    QShortcut* prevHit = new QShortcut(QKeySequence(Qt::Modifier::SHIFT | Qt::Key::Key_F3), this);
    prevHit->setContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    connect(prevHit, &QShortcut::activated, this, [this]() { findScope(-1); });
}

void NaviLogScopeBase::setupShowColumn(void)
{
    QTreeView* tree = ctrlTable();
    Q_ASSERT(tree != nullptr);

    // The box column stays flat: the depth of the tree belongs to the name column, so every
    // box sits at the same place whatever the depth of its scope.
    tree->setTreePosition(LoggingScopesModelBase::ColumnName);

    // The box and the name are one scope, so a click on either highlights the whole row.
    tree->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

    QHeaderView* header = tree->header();
    header->setMinimumSectionSize(showColumnWidth());
    header->setStretchLastSection(true);

    // Attaching a model rebuilds the header sections and drops the per section settings.
    connect(header, &QHeaderView::sectionCountChanged, this, [this](int, int) { applyShowColumnLayout(); });
    applyShowColumnLayout();
}

void NaviLogScopeBase::applyShowColumnLayout(void)
{
    QTreeView* tree = ctrlTable();
    QHeaderView* header = tree != nullptr ? tree->header() : nullptr;
    if ((header == nullptr) || (header->count() <= LoggingScopesModelBase::ColumnName))
        return;

    const int width{ showColumnWidth() };
    header->setSectionResizeMode(LoggingScopesModelBase::ColumnShow, QHeaderView::ResizeMode::Fixed);
    header->setSectionResizeMode(LoggingScopesModelBase::ColumnName, QHeaderView::ResizeMode::Stretch);
    header->resizeSection(LoggingScopesModelBase::ColumnShow, width);
}

int NaviLogScopeBase::showColumnWidth(void) const
{
    const QStyle* uiStyle = style();
    Q_ASSERT(uiStyle != nullptr);

    const int indicator{ uiStyle->pixelMetric(QStyle::PixelMetric::PM_IndicatorWidth) };
    const int margin   { uiStyle->pixelMetric(QStyle::PixelMetric::PM_FocusFrameHMargin) };
    return (indicator + (margin * 2) + 2);
}

void NaviLogScopeBase::applyScopeFilter(void)
{
    QTreeView* tree = ctrlTable();
    if ((tree == nullptr) || (mScopesModel == nullptr) || (mFilterEdit == nullptr) || (tree->model() != mScopesModel))
        return;

    const QString needle{ mFilterEdit->text().trimmed() };
    const QSignalBlocker blocker(tree);

    if (needle.isEmpty())
    {
        restoreExpanded(tree->rootIndex());
        mFilterCount->setVisible(false);
    }
    else
    {
        int matches{ 0 };
        filterBranch(tree->rootIndex(), needle, false, matches);
        mFilterCount->setText(matches != 0 ? tr("%1 found").arg(matches) : tr("none"));
        mFilterCount->setVisible(true);
    }

    updateMatchMark();
}

bool NaviLogScopeBase::filterBranch(const QModelIndex& parent, const QString& needle, bool inKept, int& matches)
{
    QTreeView* tree = ctrlTable();
    const int rows{ mScopesModel->rowCount(parent) };
    bool anyKept{ false };

    for (int row = 0; row < rows; ++row)
    {
        const QModelIndex child{ mScopesModel->index(row, 0, parent) };
        const QString name{ mScopesModel->index(row, LoggingScopesModelBase::ColumnName, parent).data(Qt::ItemDataRole::DisplayRole).toString() };
        const bool self{ name.contains(needle, Qt::CaseSensitivity::CaseInsensitive) };
        matches += self ? 1 : 0;

        const bool below{ filterBranch(child, needle, inKept || self, matches) };
        const bool kept{ inKept || self || below };
        tree->setRowHidden(row, parent, kept == false);
        if (kept && (mScopesModel->rowCount(child) != 0))
        {
            tree->setExpanded(child, true);
        }

        anyKept = anyKept || kept;
    }

    return anyKept;
}

void NaviLogScopeBase::restoreExpanded(const QModelIndex& parent)
{
    QTreeView* tree = ctrlTable();
    const int rows{ mScopesModel->rowCount(parent) };

    for (int row = 0; row < rows; ++row)
    {
        const QModelIndex child{ mScopesModel->index(row, 0, parent) };
        const ScopeNodeBase* node{ static_cast<const ScopeNodeBase*>(child.internalPointer()) };
        tree->setRowHidden(row, parent, false);
        tree->setExpanded(child, (node != nullptr) && node->isNodeExpanded());
        restoreExpanded(child);
    }
}

void NaviLogScopeBase::collectMatches(const QModelIndex& parent, const QString& needle, Qt::CaseSensitivity sensitivity, QList<QModelIndex>& matches) const
{
    const int rows{ mScopesModel->rowCount(parent) };
    for (int row = 0; row < rows; ++row)
    {
        const QModelIndex child{ mScopesModel->index(row, 0, parent) };
        const QString name{ mScopesModel->index(row, LoggingScopesModelBase::ColumnName, parent).data(Qt::ItemDataRole::DisplayRole).toString() };
        if (name.contains(needle, sensitivity))
        {
            matches.append(child);
        }

        collectMatches(child, needle, sensitivity, matches);
    }
}

void NaviLogScopeBase::showScopeFind(bool show)
{
    if (mFindBar == nullptr)
        return;

    mFindBar->setVisible(show);
    if (mToolFind != nullptr)
    {
        const QSignalBlocker blocker(mToolFind);
        mToolFind->setChecked(show);
    }

    if (show)
    {
        mFindEdit->setFocus();
        mFindEdit->selectAll();
    }
    else
    {
        mFindAt = QPersistentModelIndex();
        mFindCount->clear();
        if (ctrlTable() != nullptr)
        {
            ctrlTable()->setFocus();
        }
    }

    updateMatchMark();
}

void NaviLogScopeBase::findScope(int step)
{
    QTreeView* tree = ctrlTable();
    if ((tree == nullptr) || (mScopesModel == nullptr) || (mFindEdit == nullptr) || (mFindBar->isVisible() == false) || (tree->model() != mScopesModel))
        return;

    const QString needle{ mFindEdit->text() };
    if (needle.isEmpty())
    {
        mFindCount->clear();
        updateMatchMark();
        return;
    }

    const Qt::CaseSensitivity sensitivity{ mFindEdit->isMatchCaseChecked() ? Qt::CaseSensitivity::CaseSensitive : Qt::CaseSensitivity::CaseInsensitive };
    QList<QModelIndex> matches;
    collectMatches(tree->rootIndex(), needle, sensitivity, matches);
    if (matches.isEmpty())
    {
        mFindAt = QPersistentModelIndex();
        mFindCount->setText(tr("none"));
        updateMatchMark();
        return;
    }

    const int count{ static_cast<int>(matches.size()) };
    const int was{ mFindAt.isValid() ? static_cast<int>(matches.indexOf(QModelIndex(mFindAt))) : -1 };
    const int at{ was < 0 ? (step > 0 ? 0 : count - 1) : ((was + step + count) % count) };
    const QModelIndex hit{ matches.at(at) };

    // A hit the filter box narrowed away is brought back, so the walk never stops on a row
    // that cannot be seen.
    const QSignalBlocker blocker(tree);
    const QModelIndex modelRoot{ mScopesModel->getRootIndex() };
    for (QModelIndex up = hit; up.isValid() && (up != modelRoot); up = up.parent())
    {
        const QModelIndex above{ up.parent() };
        tree->setRowHidden(up.row(), (above == modelRoot) ? tree->rootIndex() : above, false);
    }

    // A node counts as open only once the node above it is, so the chain opens from the top.
    QList<QModelIndex> above;
    for (QModelIndex up = hit.parent(); up.isValid() && (up != modelRoot); up = up.parent())
    {
        above.prepend(up);
    }

    for (const QModelIndex& up : above)
    {
        expandNode(up, true);
    }

    mFindAt = hit;
    tree->setCurrentIndex(hit);
    tree->scrollTo(hit, QAbstractItemView::ScrollHint::EnsureVisible);
    mFindCount->setText(tr("%1 of %2").arg(at + 1).arg(count));
    updateMatchMark();
}

void NaviLogScopeBase::updateMatchMark(void)
{
    QTreeView* tree = ctrlTable();
    if ((mHighlight == nullptr) || (tree == nullptr))
        return;

    const bool finding{ (mFindBar != nullptr) && mFindBar->isVisible() && (mFindEdit->text().isEmpty() == false) };
    const QString needle{ finding ? mFindEdit->text() : (mFilterEdit != nullptr ? mFilterEdit->text().trimmed() : QString()) };
    const Qt::CaseSensitivity sensitivity{ (finding && mFindEdit->isMatchCaseChecked()) ? Qt::CaseSensitivity::CaseSensitive
                                                                                       : Qt::CaseSensitivity::CaseInsensitive };
    if (mHighlight->setNeedle(needle, sensitivity))
    {
        tree->viewport()->update();
    }
}

bool NaviLogScopeBase::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == mFindEdit) && (event->type() == QEvent::Type::KeyPress)
        && (static_cast<QKeyEvent*>(event)->key() == Qt::Key::Key_Escape))
    {
        showScopeFind(false);
        return true;
    }

    return NaviToolbarWindow::eventFilter(watched, event);
}

void NaviLogScopeBase::setupScopeControls(void)
{
    validateControls();

    QTreeView* tree = ctrlTable();
    connect(mPrioBar, &LogPriorityBar::signalLevelChanged, this, [this](LogPriorityBar::eLogLevel level) {onPriorityLevelChosen(static_cast<int>(level));});
    connect(mPrioBar, &LogPriorityBar::signalScopeToggled, this, [this](bool enabled) {onScopeLinesToggled(enabled);});

    connect(mToolShowOnly, &QToolButton::clicked, this, [this]() {onScopeVisibilityClicked(eScopeMenu::MenuShowOnlyThis);});
    connect(mToolHide    , &QToolButton::clicked, this, [this]() {onScopeVisibilityClicked(eScopeMenu::MenuHideThis);   });
    connect(mToolShowAll , &QToolButton::clicked, this, [this]() {onScopeVisibilityClicked(eScopeMenu::MenuShowAll);    });

    connect(mToolCollapse, &QToolButton::clicked, this, [this](bool checked) {onCollapseClicked(checked);});
    connect(tree, &QTreeView::expanded , this, [this](const QModelIndex& index) {onNodeExpanded(index, true );});
    connect(tree, &QTreeView::collapsed, this, [this](const QModelIndex& index) {onNodeExpanded(index, false);});
    connect(tree, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {showScopeContextMenu(pos);});
}

void NaviLogScopeBase::addSpecificTools(void)
{
}

void NaviLogScopeBase::addMoveTools(void)
{
}

bool NaviLogScopeBase::hasSavePrioMenu(void) const
{
    return false;
}

bool NaviLogScopeBase::canSavePrio(void) const
{
    return false;
}

bool NaviLogScopeBase::hasSelectAllPrioMenu(void) const
{
    return false;
}

void NaviLogScopeBase::setupModel(LoggingScopesModelBase* model)
{
    mScopesModel = model;
    mSelModel = model != nullptr ? new QItemSelectionModel(mScopesModel, this) : nullptr;

    QTreeView* tree = ctrlTable();
    if (tree == nullptr)
        return;

    // The view rejects a selection model built on a model it does not hold yet, so the model
    // is set first. Setting it afterwards would replace the selection model with a new one.
    tree->setModel(mScopesModel);
    if (mSelModel != nullptr)
    {
        tree->setSelectionModel(mSelModel);
        connect(mSelModel, &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {onRowChanged(current, previous);});
    }

    if (mScopesModel != nullptr)
    {
        // Scopes that arrive later have no hidden state yet, so the filter runs again. The
        // delayed call lets the explorer place and expand the new nodes first.
        const auto refilter = [this](const QModelIndex&) {
            if ((mFilterEdit != nullptr) && (mFilterEdit->text().trimmed().isEmpty() == false))
            {
                QTimer::singleShot(0, this, [this]() { applyScopeFilter(); });
            }
        };

        connect(mScopesModel, &LoggingScopesModelBase::signalScopesInserted, this, refilter);
        connect(mScopesModel, &LoggingScopesModelBase::signalRootUpdated   , this, refilter);

        connect(mScopesModel, &LoggingScopesModelBase::signalSafeguardsChanged, this, [this]() { refreshSafeguards(); });
        connect(mScopesModel, &LoggingScopesModelBase::signalScopesInserted   , this, [this](const QModelIndex&) { refreshSafeguards(); });
        connect(mScopesModel, &LoggingScopesModelBase::signalScopesUpdated    , this, [this](const QModelIndex&) { refreshSafeguards(); });
    }
}

inline void NaviLogScopeBase::validateControls(void) const
{
    Q_ASSERT(mPrioBar       != nullptr);
    Q_ASSERT(mToolShowOnly  != nullptr);
    Q_ASSERT(mToolHide      != nullptr);
    Q_ASSERT(mToolShowAll   != nullptr);
    Q_ASSERT(ctrlTable()    != nullptr);
    Q_ASSERT(mScopesModel   != nullptr);
}

bool NaviLogScopeBase::areRootsCollapsed(void) const
{
    bool result{ false };
    const QTreeView* tree = ctrlTable();
    if ((tree != nullptr) && (mScopesModel != nullptr))
    {
        result = true;
        int rowCount = mScopesModel->rowCount(mScopesModel->getRootIndex());
        for (int row = 0; row < rowCount; ++row)
        {
            QModelIndex index = mScopesModel->index(row, 0, mScopesModel->getRootIndex());
            if (tree->isExpanded(index))
            {
                result = false;
                break;
            }
        }
    }

    return result;
}

void NaviLogScopeBase::enableButtons(const QModelIndex& selection)
{
    validateControls();
    ScopeNodeBase* node = selection.isValid() ? mScopesModel->data(selection, Qt::ItemDataRole::UserRole).value<ScopeNodeBase*>() : nullptr;

    // A stopped process receives nothing, so the controls that send to it are turned off.
    const bool gone{ mScopesModel->isGoneTarget(node) };
    mPrioBar->setEnabled((node != nullptr) && (gone == false));
    mPrioBar->setToolTip(gone ? tr("The process has stopped, so its scopes cannot be changed.") : mPrioTip);
    if (node != nullptr)
    {
        const ScopeNodeBase::sPrioRollup rollup{ node->priorityRollup() };
        mPrioBar->setLevel(static_cast<LogPriorityBar::eLogLevel>(NaviLogScopeBase::levelOfPriority(node->getPriority())));
        mPrioBar->setScopeEnabled(node->hasScopeEntries());
        mPrioBar->setMixed(rollup.levelLow != rollup.levelHigh);

        // A lit ladder says "this is what the selection generates". A node that carries no
        // priority yet says nothing, so the ladder shows nothing.
        mPrioBar->setIdle((node->isValid() == false) || node->hasPrioNotset());
    }
    else
    {
        mPrioBar->setMixed(false);
        mPrioBar->setIdle(true);
    }

    mToolShowOnly->setEnabled(node != nullptr);
    mToolHide->setEnabled((node != nullptr) && (node->shownState() != Qt::CheckState::Unchecked));
    mToolShowAll->setEnabled(mScopesModel->hasHiddenScopes());
}

void NaviLogScopeBase::updateExpanded(const QModelIndex& current)
{
    QTreeView* tree = current.isValid() && (mScopesModel != nullptr) ? ctrlTable() : nullptr;
    if (tree != nullptr)
    {
        tree->update(current);
        int count = tree->isExpanded(current) ? mScopesModel->rowCount(current) : 0;
        for (int i = 0; i < count; ++i)
        {
            QModelIndex index = mScopesModel->index(i, 0, current);
            updateExpanded(index);
        }
    }
}

bool NaviLogScopeBase::updatePriority(const QModelIndex& node, bool addPrio, areg::LogPriority prio)
{
    bool result{ false };
    if (node.isValid())
    {
        Q_ASSERT(mScopesModel != nullptr);
        if (addPrio)
        {
            result = mScopesModel->addLogPriority(node, static_cast<uint32_t>(prio));
        }
        else
        {
            result = mScopesModel->removLogPriority(node, static_cast<uint32_t>(prio));
        }
    }

    if (result && (ctrlTable() != nullptr))
    {
        // Force immediate repaint of visible rows so updated node icons are not delayed until hover.
        ctrlTable()->viewport()->update();
    }

    return result;
}

void NaviLogScopeBase::expandChildNodesRecursive(const QModelIndex& idxNode, const ScopeNodeBase& node)
{
    if (node.isLeaf() || (idxNode.isValid() == false))
        return;

    QTreeView* navi = ctrlTable();
    int rowCount{ node.getChildNodesCount() };
    for (int row = 0; row < rowCount; ++row)
    {
        const ScopeNodeBase* child = node.getChildAt(row);
        Q_ASSERT(child != nullptr);
        if (child->isNodeExpanded())
        {
            QModelIndex idxChild{ mScopesModel->index(row, 0, idxNode) };
            Q_ASSERT(idxChild.isValid());
            navi->expand(idxChild);
            if (child->isNode())
            {
                expandChildNodesRecursive(idxChild, *child);
            }
        }
    }

    enableButtons(idxNode);
}

void NaviLogScopeBase::collapseRoots(void)
{
    QTreeView* treeView = ctrlTable();
    int rowCount = mScopesModel != nullptr ? mScopesModel->rowCount(mScopesModel->getRootIndex()) : 0;
    for (int row = 0; row < rowCount; ++row)
    {
        QModelIndex index = mScopesModel->index(row, 0, mScopesModel->getRootIndex());
        treeView->collapse(index);
        mScopesModel->nodeCollapsed(index);
    }
}

void NaviLogScopeBase::expandNode(const QModelIndex& node, bool markExpanded)
{
    QTreeView* navi = ctrlTable();
    if (node.isValid() && (navi->isExpanded(node) == false))
    {
        navi->expand(node);
        if (markExpanded)
        {
            mScopesModel->nodeExpanded(node);
        }
    }
}

void NaviLogScopeBase::expandNodeAndChildren(const QModelIndex& node, bool markExpanded)
{
    Q_ASSERT(mScopesModel != nullptr);
    expandNode(node, markExpanded);

    int rowCount = mScopesModel->rowCount(node);
    for (int row = 0; row < rowCount; ++row)
    {
        expandNode(mScopesModel->index(row, 0, node), markExpanded);
    }
}

void NaviLogScopeBase::onPriorityLevelChosen(int level)
{
    validateControls();
    const QModelIndex current{ ctrlTable()->currentIndex() };
    applyPriorityLevel(current, level);
    enableButtons(current);
}

void NaviLogScopeBase::onScopeLinesToggled(bool enabled)
{
    validateControls();
    const QModelIndex current{ ctrlTable()->currentIndex() };
    updatePriority(reachTarget(current), enabled, areg::LogPriority::PrioScope);
    enableButtons(current);
}

void NaviLogScopeBase::onScopeVisibilityClicked(eScopeMenu entry)
{
    validateControls();
    const QModelIndex current{ ctrlTable()->currentIndex() };
    applyScopeVisibility(current, entry);
    enableButtons(current);
}

void NaviLogScopeBase::applyScopeVisibility(const QModelIndex& node, eScopeMenu entry)
{
    Q_ASSERT(mScopesModel != nullptr);
    switch (entry)
    {
    case eScopeMenu::MenuShowOnlyThis:
        mScopesModel->showScopeAlone(node);
        break;

    case eScopeMenu::MenuHideThis:
        mScopesModel->setScopeShown(node, false);
        break;

    case eScopeMenu::MenuShowAll:
        mScopesModel->showAllScopes();
        break;

    default:
        break;
    }
}

void NaviLogScopeBase::onNodeExpanded(const QModelIndex& index, bool expanded)
{
    if (mToolCollapse != nullptr)
    {
        if (expanded && (areRootsCollapsed() == false))
        {
            mToolCollapse->setIcon(NELusanCommon::iconNodeCollapsed(NELusanCommon::SizeSmall));
            mToolCollapse->setChecked(false);
        }
        else if ((expanded == false) && areRootsCollapsed())
        {
            mToolCollapse->setIcon(NELusanCommon::iconNodeExpanded(NELusanCommon::SizeSmall));
            mToolCollapse->setChecked(true);
        }
    }

    Q_ASSERT(mScopesModel != nullptr);
    if (expanded)
    {
        mScopesModel->nodeExpanded(index);
    }
    else
    {
        mScopesModel->nodeCollapsed(index);
    }
}

void NaviLogScopeBase::onRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);

    enableButtons(current);
    mScopesModel->nodeSelected(current);
}

void NaviLogScopeBase::setLoggingModel(LoggingModelBase* logModel)
{
    Q_ASSERT(mScopesModel != nullptr);
    mScopesModel->setLoggingModel(nullptr);
    if (logModel != nullptr)
    {
        mScopesModel->setLoggingModel(logModel);
    }
}

LoggingModelBase* NaviLogScopeBase::getLoggingModel(void) const
{
    Q_ASSERT(mScopesModel != nullptr);
    return mScopesModel->getLoggingModel();
}

void NaviLogScopeBase::onCollapseClicked(bool checked)
{
    Q_ASSERT(mScopesModel != nullptr);
    Q_ASSERT(mToolCollapse != nullptr);

    QTreeView* tree = ctrlTable();
    Q_ASSERT(tree != nullptr);

    if (mScopesModel->rowCount(mScopesModel->getRootIndex()) == 0)
    {
        mToolCollapse->blockSignals(true);
        mToolCollapse->setChecked(false);
        mToolCollapse->blockSignals(false);
        return;
    }

    mToolCollapse->blockSignals(true);
    tree->blockSignals(true);

    if (checked)
    {
        mToolCollapse->setIcon(NELusanCommon::iconNodeExpanded(NELusanCommon::SizeBig));
        mToolCollapse->setChecked(true);

        collapseRoots();
        tree->expand(mScopesModel->getRootIndex());
        mScopesModel->nodeExpanded(mScopesModel->getRootIndex());
    }
    else
    {
        mToolCollapse->setIcon(NELusanCommon::iconNodeCollapsed(NELusanCommon::SizeBig));
        mToolCollapse->setChecked(false);

        tree->expandAll();
        mScopesModel->nodeTreeExpanded(mScopesModel->getRootIndex());
    }

    tree->setCurrentIndex(mScopesModel->getRootIndex());

    tree->blockSignals(false);
    mToolCollapse->blockSignals(false);
}

uint32_t NaviLogScopeBase::priorityOfLevel(int level)
{
    uint32_t result{ static_cast<uint32_t>(areg::LogPriority::PrioNotset) };
    if (level >= static_cast<int>(LogPriorityBar::eLogLevel::LevelError))
        result = static_cast<uint32_t>(areg::LogPriority::PrioFatal) | static_cast<uint32_t>(areg::LogPriority::PrioError);
    if (level >= static_cast<int>(LogPriorityBar::eLogLevel::LevelWarning))
        result |= static_cast<uint32_t>(areg::LogPriority::PrioWarning);
    if (level >= static_cast<int>(LogPriorityBar::eLogLevel::LevelInformation))
        result |= static_cast<uint32_t>(areg::LogPriority::PrioInfo);
    if (level >= static_cast<int>(LogPriorityBar::eLogLevel::LevelDebug))
        result |= static_cast<uint32_t>(areg::LogPriority::PrioDebug);

    return result;
}

int NaviLogScopeBase::levelOfPriority(uint32_t prio)
{
    return static_cast<int>(ScopeNodeBase::priorityLevel(prio));
}

QModelIndex NaviLogScopeBase::reachTarget(const QModelIndex& node) const
{
    if ((node.isValid() == false) || (mScopesModel == nullptr))
        return QModelIndex();

    if (mScopeReach != eScopeReach::ReachProcess)
        return node;

    QModelIndex result{ node };
    QModelIndex parent{ result.parent() };
    while (parent.isValid() && (parent != mScopesModel->getRootIndex()))
    {
        result = parent;
        parent = result.parent();
    }

    return result;
}

void NaviLogScopeBase::applyPriorityLevel(const QModelIndex& node, int level)
{
    const QModelIndex target{ reachTarget(node) };
    if ((target.isValid() == false) || (mScopesModel == nullptr))
        return;

    const ScopeNodeBase* entry{ static_cast<const ScopeNodeBase*>(target.internalPointer()) };
    uint32_t prio{ NaviLogScopeBase::priorityOfLevel(level) };

    // The enter and exit lines are a separate switch, so they survive a change of level.
    if ((entry != nullptr) && entry->hasScopeEntries() && (level != static_cast<int>(LogPriorityBar::eLogLevel::LevelOff)))
    {
        prio |= static_cast<uint32_t>(areg::LogPriority::PrioScope);
    }

    // The walk back is scheduled before the change, so it carries what the scope had.
    const uint32_t was{ entry != nullptr ? entry->getPriority() : prio };
    if (mTempRaise && (NaviLogScopeBase::levelOfPriority(prio) > NaviLogScopeBase::levelOfPriority(was)))
    {
        mScopesModel->scheduleRevert(target, was, NaviLogScopeBase::TempRaiseMs);
    }

    if (mScopesModel->setLogPriority(target, prio) && (ctrlTable() != nullptr))
    {
        ctrlTable()->viewport()->update();
    }

    refreshSafeguards();
}

void NaviLogScopeBase::copyScopePath(const QModelIndex& node) const
{
    const ScopeNodeBase* entry{ node.isValid() ? static_cast<const ScopeNodeBase*>(node.internalPointer()) : nullptr };
    if (entry == nullptr)
        return;

    const QString path{ entry->makePath() };
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard != nullptr)
    {
        clipboard->setText(path.isEmpty() ? entry->getDisplayName() : path);
    }
}

void NaviLogScopeBase::buildScopeMenu(QMenu& menu, const QModelIndex& node, const ScopeNodeBase& entry)
{
    const int level{ NaviLogScopeBase::levelOfPriority(entry.getPriority()) };
    const bool isMixed{ entry.priorityRollup().levelLow != entry.priorityRollup().levelHigh };
    const bool hasLines{ entry.hasScopeEntries() };

    // The ladder is one click at the cursor. The text items below it drive the same slot and
    // are what the keyboard and a screen reader reach.
    LogPriorityBar* ladder = new LogPriorityBar(&menu);
    ladder->setLevel(static_cast<LogPriorityBar::eLogLevel>(level));
    ladder->setScopeEnabled(hasLines);
    ladder->setMixed(isMixed);
    connect(ladder, &LogPriorityBar::signalLevelChanged, &menu, [this, &menu, node](LogPriorityBar::eLogLevel newLevel) {
        menu.close();
        applyPriorityLevel(node, static_cast<int>(newLevel));
    });
    connect(ladder, &LogPriorityBar::signalScopeToggled, &menu, [this, &menu, node](bool enabled) {
        menu.close();
        updatePriority(reachTarget(node), enabled, areg::LogPriority::PrioScope);
    });

    QWidgetAction* ladderAction = new QWidgetAction(&menu);
    ladderAction->setDefaultWidget(ladder);
    menu.addAction(ladderAction);

    menu.addSeparator();

    QActionGroup* levels = new QActionGroup(&menu);
    levels->setExclusive(true);
    const struct { eScopeMenu id; LogPriorityBar::eLogLevel level; const char* text; } _levels[]
    {
          { eScopeMenu::MenuLevelOff    , LogPriorityBar::eLogLevel::LevelOff        , QT_TR_NOOP("O&ff")           }
        , { eScopeMenu::MenuLevelError  , LogPriorityBar::eLogLevel::LevelError      , QT_TR_NOOP("&Error")         }
        , { eScopeMenu::MenuLevelWarning, LogPriorityBar::eLogLevel::LevelWarning    , QT_TR_NOOP("&Warning")       }
        , { eScopeMenu::MenuLevelInfo   , LogPriorityBar::eLogLevel::LevelInformation, QT_TR_NOOP("&Information")   }
        , { eScopeMenu::MenuLevelDebug  , LogPriorityBar::eLogLevel::LevelDebug      , QT_TR_NOOP("&Debug")         }
    };

    for (const auto& item : _levels)
    {
        QAction* action = menu.addAction(tr(item.text));
        action->setData(static_cast<int>(item.id));
        action->setCheckable(true);
        action->setChecked((isMixed == false) && (level == static_cast<int>(item.level)));
        levels->addAction(action);
    }

    QAction* lines = menu.addAction(NELusanCommon::iconScopeLines(NELusanCommon::SizeBig), tr("&Scope lines"));
    lines->setData(static_cast<int>(eScopeMenu::MenuScopeLines));
    lines->setCheckable(true);
    lines->setChecked(hasLines);

    QAction* temporary = menu.addAction(NELusanCommon::iconTimer(NELusanCommon::SizeBig), tr("&Go back after five minutes"));
    temporary->setData(static_cast<int>(eScopeMenu::MenuTempRaise));
    temporary->setCheckable(true);
    temporary->setChecked(mTempRaise);
    temporary->setEnabled(mScopesModel->isLiveSession());
    temporary->setToolTip(tr("Raising a scope from here puts it back on its own after five minutes."));

    menu.addSeparator();
    menu.addSection(tr("Apply to"));

    QActionGroup* reach = new QActionGroup(&menu);
    reach->setExclusive(true);
    const struct { eScopeMenu id; eScopeReach value; const char* text; } _reaches[]
    {
          { eScopeMenu::MenuReachScope  , eScopeReach::ReachScope  , QT_TR_NOOP("This scope")                     }
        , { eScopeMenu::MenuReachBranch , eScopeReach::ReachBranch , QT_TR_NOOP("This node and everything under it") }
        , { eScopeMenu::MenuReachProcess, eScopeReach::ReachProcess, QT_TR_NOOP("All scopes of this process")      }
    };

    for (const auto& item : _reaches)
    {
        QAction* action = menu.addAction(tr(item.text));
        action->setData(static_cast<int>(item.id));
        action->setCheckable(true);
        action->setChecked(mScopeReach == item.value);
        reach->addAction(action);
        if ((item.value == eScopeReach::ReachScope) && (entry.isLeaf() == false))
        {
            // A node is a path segment, not a scope, and the target is addressed by a wildcard
            // path, so a node cannot be changed without its children.
            action->setEnabled(false);
            action->setToolTip(tr("Only a scope can be changed on its own."));
        }
    }

    menu.addSeparator();
    menu.addSection(tr("Show and hide"));

    QAction* solo = menu.addAction(NELusanCommon::iconScopeSolo(NELusanCommon::SizeBig), tr("Show &only this"));
    solo->setData(static_cast<int>(eScopeMenu::MenuShowOnlyThis));

    QAction* mute = menu.addAction(NELusanCommon::iconScopeMute(NELusanCommon::SizeBig), tr("&Hide this"));
    mute->setData(static_cast<int>(eScopeMenu::MenuHideThis));
    mute->setEnabled(entry.shownState() != Qt::CheckState::Unchecked);

    QAction* showAll = menu.addAction(NELusanCommon::iconScopeRestoreAll(NELusanCommon::SizeBig), tr("Show &all"));
    showAll->setData(static_cast<int>(eScopeMenu::MenuShowAll));
    showAll->setEnabled(mScopesModel->hasHiddenScopes());

    menu.addSeparator();

    const bool isExpanded{ ctrlTable()->isExpanded(node) };
    const bool hasChildren{ entry.hasChildren() };

    QAction* expand = menu.addAction(NELusanCommon::iconNodeExpanded(NELusanCommon::SizeBig), tr("Expand selected"));
    expand->setData(static_cast<int>(eScopeMenu::MenuExpandSelected));
    expand->setEnabled((isExpanded == false) && hasChildren);

    QAction* collapse = menu.addAction(NELusanCommon::iconNodeCollapsed(NELusanCommon::SizeBig), tr("Collapse selected"));
    collapse->setData(static_cast<int>(eScopeMenu::MenuCollapseSelected));
    collapse->setEnabled(isExpanded && hasChildren);

    menu.addAction(tr("Expand all"))->setData(static_cast<int>(eScopeMenu::MenuExpandAll));

    QAction* collapseAll = menu.addAction(tr("Collapse all"));
    collapseAll->setData(static_cast<int>(eScopeMenu::MenuCollapseAll));
    collapseAll->setEnabled(areRootsCollapsed() == false);

    if (hasSavePrioMenu())
    {
        menu.addSeparator();
        menu.addSection(tr("Target"));

        const bool canSave{ canSavePrio() };
        QAction* saveOne = menu.addAction(NELusanCommon::iconSaveDocument(NELusanCommon::SizeBig), tr("Save priorities on &target"));
        saveOne->setData(static_cast<int>(eScopeMenu::MenuSavePrioTarget));
        saveOne->setEnabled(canSave);

        QAction* saveAll = menu.addAction(tr("Save priorities on all targets"));
        saveAll->setData(static_cast<int>(eScopeMenu::MenuSavePrioAll));
        saveAll->setEnabled(canSave);
    }

    menu.addSeparator();
    menu.addAction(tr("&Copy scope path"))->setData(static_cast<int>(eScopeMenu::MenuCopyScopePath));
}

bool NaviLogScopeBase::runScopeMenu(const QAction& action, const QModelIndex& node)
{
    QTreeView* tree = ctrlTable();
    Q_ASSERT(tree != nullptr);
    Q_ASSERT(mScopesModel != nullptr);

    switch (static_cast<eScopeMenu>(action.data().toInt()))
    {
    case eScopeMenu::MenuLevelOff:
        applyPriorityLevel(node, static_cast<int>(LogPriorityBar::eLogLevel::LevelOff));
        break;

    case eScopeMenu::MenuLevelError:
        applyPriorityLevel(node, static_cast<int>(LogPriorityBar::eLogLevel::LevelError));
        break;

    case eScopeMenu::MenuLevelWarning:
        applyPriorityLevel(node, static_cast<int>(LogPriorityBar::eLogLevel::LevelWarning));
        break;

    case eScopeMenu::MenuLevelInfo:
        applyPriorityLevel(node, static_cast<int>(LogPriorityBar::eLogLevel::LevelInformation));
        break;

    case eScopeMenu::MenuLevelDebug:
        applyPriorityLevel(node, static_cast<int>(LogPriorityBar::eLogLevel::LevelDebug));
        break;

    case eScopeMenu::MenuScopeLines:
        updatePriority(reachTarget(node), action.isChecked(), areg::LogPriority::PrioScope);
        break;

    case eScopeMenu::MenuTempRaise:
        mTempRaise = action.isChecked();
        break;

    case eScopeMenu::MenuReachScope:
        mScopeReach = eScopeReach::ReachScope;
        break;

    case eScopeMenu::MenuReachBranch:
        mScopeReach = eScopeReach::ReachBranch;
        break;

    case eScopeMenu::MenuReachProcess:
        mScopeReach = eScopeReach::ReachProcess;
        break;

    case eScopeMenu::MenuShowOnlyThis:
    case eScopeMenu::MenuHideThis:
    case eScopeMenu::MenuShowAll:
        applyScopeVisibility(node, static_cast<eScopeMenu>(action.data().toInt()));
        break;

    case eScopeMenu::MenuExpandSelected:
        tree->expand(node);
        mScopesModel->nodeExpanded(node);
        break;

    case eScopeMenu::MenuCollapseSelected:
        tree->collapse(node);
        mScopesModel->nodeCollapsed(node);
        break;

    case eScopeMenu::MenuExpandAll:
        onCollapseClicked(true);
        break;

    case eScopeMenu::MenuCollapseAll:
        onCollapseClicked(false);
        break;

    case eScopeMenu::MenuSavePrioTarget:
        mScopesModel->saveLogScopePriority(node);
        break;

    case eScopeMenu::MenuSavePrioAll:
        mScopesModel->saveLogScopePriority(QModelIndex());
        break;

    case eScopeMenu::MenuCopyScopePath:
        copyScopePath(node);
        break;

    default:
        return false;
    }

    return true;
}

void NaviLogScopeBase::showScopeContextMenu(const QPoint& pos)
{
    QTreeView* tree = ctrlTable();
    QModelIndex index = tree->indexAt(pos);
    if (index.isValid() == false)
        return;

    Q_ASSERT(mScopesModel != nullptr);

    ScopeNodeBase* node = mScopesModel->data(index, Qt::UserRole).value<ScopeNodeBase*>();
    if ((node == nullptr) || (node->hasPrioValid() == false))
        return;

    QMenu menu(this);
    menu.setToolTipsVisible(true);
    buildScopeMenu(menu, index, *node);

    const QAction* selected = menu.exec(tree->viewport()->mapToGlobal(pos));
    if ((selected != nullptr) && runScopeMenu(*selected, index))
    {
        enableButtons(index);
        mScopesModel->nodeSelected(index);
    }
}
