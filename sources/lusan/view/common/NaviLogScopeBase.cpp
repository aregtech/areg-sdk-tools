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

#include <QAction>
#include <QItemSelectionModel>
#include <QMenu>
#include <QPoint>
#include <QSize>
#include <QToolButton>
#include <QTreeView>

NaviLogScopeBase::NaviLogScopeBase(int naviWindow, MdiMainWindow* wndMain, QWidget* parent)
    : NaviToolbarWindow (naviWindow, wndMain, parent)

    , mScopesModel      (nullptr)
    , mSelModel         (nullptr)
    , mToolCollapse     (nullptr)
    , mToolFind         (nullptr)
    , mPrioError        (nullptr)
    , mPrioWarning      (nullptr)
    , mPrioInfo         (nullptr)
    , mPrioDebug        (nullptr)
    , mPrioScopes       (nullptr)
    , mMenuActions      (static_cast<int>(eScopeMenu::MenuCount))
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

    mPrioError   = addToolButton(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioError, false), tr("Show / hide error messages")      , tr("Show / hide error messages")      , true);
    mPrioWarning = addToolButton(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioWarn , false), tr("Show / hide warning messages")    , tr("Show / hide warning messages")    , true);
    mPrioInfo    = addToolButton(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioInfo , false), tr("Show / hide information messages"), tr("Show / hide information messages"), true);
    mPrioDebug   = addToolButton(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioDebug, false), tr("Show / hide debug messages")      , tr("Show / hide debug messages")      , true);
    mPrioScopes  = addToolButton(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioScope, false), tr("Show / hide scope messages")      , tr("Show / hide scope messages")      , true);

    addToolSeparator();

    addMoveTools();

    setupTreeView(QSize(10, 10));
    ctrlTable()->setRootIsDecorated(false);
    ctrlTable()->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    capToolButtonIconSizes();
}

void NaviLogScopeBase::setupScopeControls(void)
{
    validateControls();

    QTreeView* tree = ctrlTable();
    connect(mPrioError  , &QToolButton::clicked, this, [this](bool checked) {onLogPrioChecked(checked, *mPrioError  , areg::LogPriority::PrioError);  });
    connect(mPrioWarning, &QToolButton::clicked, this, [this](bool checked) {onLogPrioChecked(checked, *mPrioWarning, areg::LogPriority::PrioWarning);});
    connect(mPrioInfo   , &QToolButton::clicked, this, [this](bool checked) {onLogPrioChecked(checked, *mPrioInfo   , areg::LogPriority::PrioInfo);   });
    connect(mPrioDebug  , &QToolButton::clicked, this, [this](bool checked) {onLogPrioChecked(checked, *mPrioDebug  , areg::LogPriority::PrioDebug);  });
    connect(mPrioScopes , &QToolButton::clicked, this, [this](bool checked) {onLogPrioChecked(checked, *mPrioScopes , areg::LogPriority::PrioScope);  });

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
    if (tree != nullptr)
    {
        tree->setSelectionModel(mSelModel);
        tree->setModel(mScopesModel);
        connect(mSelModel, &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {onRowChanged(current, previous);});
    }
}

inline void NaviLogScopeBase::validateControls(void) const
{
    Q_ASSERT(mPrioDebug     != nullptr);
    Q_ASSERT(mPrioInfo      != nullptr);
    Q_ASSERT(mPrioWarning   != nullptr);
    Q_ASSERT(mPrioError     != nullptr);
    Q_ASSERT(mPrioScopes    != nullptr);
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
    if (node != nullptr)
    {
        bool errSelected{ false }, warnSelected{ false }, infoSelected{ false }, dbgSelected{ false }, scopeSelected{ false };

        mPrioError->setEnabled(true);
        mPrioWarning->setEnabled(true);
        mPrioInfo->setEnabled(true);
        mPrioDebug->setEnabled(true);
        mPrioScopes->setEnabled(true);

        mPrioError->setChecked(false);
        mPrioWarning->setChecked(false);
        mPrioInfo->setChecked(false);
        mPrioDebug->setChecked(false);
        mPrioScopes->setChecked(false);

        if (node->isValid() && (node->hasPrioNotset() == false))
        {
            if (node->hasPrioDebug())
            {
                mPrioDebug->setChecked(true);
                dbgSelected = true;
            }

            if (node->hasPrioInfo())
            {
                mPrioInfo->setChecked(true);
                infoSelected = true;
            }

            if (node->hasPrioWarning())
            {
                mPrioWarning->setChecked(true);
                warnSelected = true;
            }

            if (node->hasPrioError() || node->hasPrioFatal())
            {
                mPrioError->setChecked(true);
                errSelected = true;
            }

            if (node->hasScopeEntries())
            {
                mPrioScopes->setChecked(true);
                scopeSelected = true;
            }
        }

        updateColors(errSelected, warnSelected, infoSelected, dbgSelected, scopeSelected);
    }
    else
    {
        mPrioError->setEnabled(false);
        mPrioWarning->setEnabled(false);
        mPrioInfo->setEnabled(false);
        mPrioDebug->setEnabled(false);
        mPrioScopes->setEnabled(false);
    }
}

void NaviLogScopeBase::updateColors(bool errSelected, bool warnSelected, bool infoSelected, bool dbgSelected, bool scopeSelected)
{
    validateControls();

    mPrioDebug->setIcon(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioDebug, dbgSelected));
    mPrioInfo->setIcon(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioInfo, infoSelected));
    mPrioWarning->setIcon(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioWarn, warnSelected));
    mPrioError->setIcon(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioError, errSelected));
    mPrioScopes->setIcon(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioScope, scopeSelected));

    mPrioError->update();
    mPrioWarning->update();
    mPrioInfo->update();
    mPrioDebug->update();
    mPrioScopes->update();
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

void NaviLogScopeBase::onLogPrioChecked(bool checked, QToolButton& toolButton, areg::LogPriority prio)
{
    validateControls();
    QModelIndex current = ctrlTable()->currentIndex();
    if (updatePriority(current, checked, prio) == false)
    {
        toolButton.setChecked(!checked);
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

    bool hasScope{ false }, hasDebug{ false }, hasInfo{ false }, hasWarn{ false }, hasError{ false }, hasFatal{ false };
    if (node->hasPrioNotset() == false)
    {
        hasScope = node->hasScopeEntries();
        hasDebug = node->hasPrioDebug();
        hasInfo  = node->hasPrioInfo();
        hasWarn  = node->hasPrioWarning();
        hasError = node->hasPrioError();
        hasFatal = node->hasPrioFatal();
    }

    const bool isExpanded{ tree->isExpanded(index) };
    const bool hasChildren{ node->hasChildren() };

    mMenuActions.fill(nullptr);
    QMenu menu(this);

    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioNotset)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioNotset, false), tr("&Reset Priorities"));
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioNotset)]->setCheckable(false);

    if (hasSelectAllPrioMenu())
    {
        mMenuActions[static_cast<int>(eScopeMenu::MenuPrioAllset)] = menu.addAction(tr("&Select All Priorities"));
        mMenuActions[static_cast<int>(eScopeMenu::MenuPrioAllset)]->setCheckable(false);
    }

    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioDebug)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioDebug, hasDebug), hasDebug ? tr("Hide &Debug messages") : tr("Show &Debug messages"));
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioDebug)]->setCheckable(true);
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioDebug)]->setChecked(hasDebug);

    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioInfo)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioInfo, hasInfo), hasInfo ? tr("Hide &Info messages") : tr("Show &Info messages"));
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioInfo)]->setCheckable(true);
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioInfo)]->setChecked(hasInfo);

    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioWarn)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioWarn, hasWarn), hasWarn ? tr("Hide &Warning messages") : tr("Show &Warning messages"));
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioWarn)]->setCheckable(true);
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioWarn)]->setChecked(hasWarn);

    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioError)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioError, hasError), hasError ? tr("Hide &Error messages") : tr("Show &Error messages"));
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioError)]->setCheckable(true);
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioError)]->setChecked(hasError);

    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioFatal)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioFatal, hasFatal), hasFatal ? tr("Hide &Fatal messages") : tr("Show &Fatal messages"));
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioFatal)]->setCheckable(true);
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioFatal)]->setChecked(hasFatal);

    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioScope)] = menu.addAction(LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioScope, hasScope), hasScope ? tr("Hide &Scopes") : tr("Show &Scopes"));
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioScope)]->setCheckable(true);
    mMenuActions[static_cast<int>(eScopeMenu::MenuPrioScope)]->setChecked(hasScope);

    mMenuActions[static_cast<int>(eScopeMenu::MenuExpandSelected)] = menu.addAction(NELusanCommon::iconNodeExpanded(NELusanCommon::SizeBig), tr("Expand Selected"));
    mMenuActions[static_cast<int>(eScopeMenu::MenuExpandSelected)]->setCheckable(false);
    mMenuActions[static_cast<int>(eScopeMenu::MenuExpandSelected)]->setEnabled((isExpanded == false) && hasChildren);

    mMenuActions[static_cast<int>(eScopeMenu::MenuCollapseSelected)] = menu.addAction(NELusanCommon::iconNodeCollapsed(NELusanCommon::SizeBig), tr("Collapse Selected"));
    mMenuActions[static_cast<int>(eScopeMenu::MenuCollapseSelected)]->setCheckable(false);
    mMenuActions[static_cast<int>(eScopeMenu::MenuCollapseSelected)]->setEnabled(isExpanded && hasChildren);

    mMenuActions[static_cast<int>(eScopeMenu::MenuExpandAll)] = menu.addAction(tr("Expand All"));
    mMenuActions[static_cast<int>(eScopeMenu::MenuExpandAll)]->setCheckable(false);

    mMenuActions[static_cast<int>(eScopeMenu::MenuCollapseAll)] = menu.addAction(tr("Collapse All"));
    mMenuActions[static_cast<int>(eScopeMenu::MenuCollapseAll)]->setCheckable(false);
    mMenuActions[static_cast<int>(eScopeMenu::MenuCollapseAll)]->setEnabled(areRootsCollapsed() == false);

    if (hasSavePrioMenu())
    {
        const bool canSave{ canSavePrio() };
        mMenuActions[static_cast<int>(eScopeMenu::MenuSavePrioTarget)] = menu.addAction(NELusanCommon::iconSaveDocument(NELusanCommon::SizeBig), tr("&Save Selection on Target"));
        mMenuActions[static_cast<int>(eScopeMenu::MenuSavePrioTarget)]->setCheckable(false);
        mMenuActions[static_cast<int>(eScopeMenu::MenuSavePrioTarget)]->setEnabled(canSave);

        mMenuActions[static_cast<int>(eScopeMenu::MenuSavePrioAll)] = menu.addAction(tr("Save &All Targets"));
        mMenuActions[static_cast<int>(eScopeMenu::MenuSavePrioAll)]->setCheckable(false);
        mMenuActions[static_cast<int>(eScopeMenu::MenuSavePrioAll)]->setEnabled(canSave);
    }

    QAction* selected = menu.exec(tree->viewport()->mapToGlobal(pos));
    if (selected == nullptr)
        return;

    bool processed{ true };
    if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuPrioNotset)])
    {
        mScopesModel->setLogPriority(index, static_cast<uint32_t>(areg::LogPriority::PrioNotset));
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuPrioAllset)])
    {
        mScopesModel->setLogPriority(index, static_cast<uint32_t>(areg::LogPriority::PrioScopeLogs));
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuPrioDebug)])
    {
        updatePriority(index, selected->isChecked(), areg::LogPriority::PrioDebug);
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuPrioInfo)])
    {
        updatePriority(index, selected->isChecked(), areg::LogPriority::PrioInfo);
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuPrioWarn)])
    {
        updatePriority(index, selected->isChecked(), areg::LogPriority::PrioWarning);
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuPrioError)])
    {
        updatePriority(index, selected->isChecked(), areg::LogPriority::PrioError);
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuPrioFatal)])
    {
        updatePriority(index, selected->isChecked(), areg::LogPriority::PrioFatal);
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuPrioScope)])
    {
        updatePriority(index, selected->isChecked(), areg::LogPriority::PrioScope);
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuExpandSelected)])
    {
        tree->expand(index);
        mScopesModel->nodeExpanded(index);
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuCollapseSelected)])
    {
        tree->collapse(index);
        mScopesModel->nodeCollapsed(index);
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuExpandAll)])
    {
        onCollapseClicked(true);
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuCollapseAll)])
    {
        onCollapseClicked(false);
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuSavePrioTarget)])
    {
        mScopesModel->saveLogScopePriority(index);
    }
    else if (selected == mMenuActions[static_cast<int>(eScopeMenu::MenuSavePrioAll)])
    {
        mScopesModel->saveLogScopePriority(mScopesModel->getRootIndex());
    }
    else
    {
        processed = false;
    }

    if (processed)
    {
        enableButtons(index);
        mScopesModel->nodeSelected(index);
    }
}
