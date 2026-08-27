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
#include "lusan/view/log/LogPriorityBar.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QItemSelectionModel>
#include <QHeaderView>
#include <QMenu>
#include <QPoint>
#include <QSize>
#include <QStyle>
#include <QToolButton>
#include <QTreeView>
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
                             , tr("Find log message")
                             , tr("Find log message"));

    addToolSeparator();

    mPrioBar = new LogPriorityBar(this);
    mPrioBar->setToolTip(tr("Set how much the selected scopes generate. The last cell switches the enter and exit lines."));
    mPrioBar->setStatusTip(mPrioBar->toolTip());
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

    setupTreeView(QSize(NaviLogScopeBase::ScopeIconExtent, NaviLogScopeBase::ScopeIconExtent));
    ctrlTable()->setRootIsDecorated(false);
    ctrlTable()->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    setupShowColumn();

    capToolButtonIconSizes();
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

    mPrioBar->setEnabled(node != nullptr);
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

    if (mScopesModel->setLogPriority(target, prio) && (ctrlTable() != nullptr))
    {
        ctrlTable()->viewport()->update();
    }
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
        mScopesModel->saveLogScopePriority(mScopesModel->getRootIndex());
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
