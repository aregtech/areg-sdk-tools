/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/NaviLogScopeBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The base class of the log explorer view.
 *
 ************************************************************************/

#include "lusan/view/common/NaviLogScopeBase.hpp"

#include "lusan/data/log/ScopeNodeBase.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/LoggingScopesModelBase.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"

#include <QItemSelectionModel>
#include <QToolButton>
#include <QTreeView>

NaviLogScopeBase::NaviLogScopeBase(int naviWindow, MdiMainWindow* wndMain, QWidget* parent)
    : NavigationWindow(naviWindow, wndMain, parent)
    , mScopesModel(nullptr)
    , mNaviTree(nullptr)
    , mSelModel(nullptr)
{
}

void NaviLogScopeBase::setupModel(LoggingScopesModelBase* model)
{
    mScopesModel = model;
    mSelModel = model != nullptr ? new QItemSelectionModel(mScopesModel, this) : nullptr;

    if (mNaviTree != nullptr)
    {
        mNaviTree->setSelectionModel(mSelModel);
        mNaviTree->setModel(mScopesModel);
        connect(mSelModel   , &QItemSelectionModel::currentRowChanged,this, [this](const QModelIndex &current, const QModelIndex &previous) {onRowChanged(current, previous);});
    }
}

void NaviLogScopeBase::setupControls(QTreeView* treeView, QToolButton* prioError, QToolButton* prioWarning, QToolButton* prioInfo, QToolButton* prioDebug, QToolButton* prioScopes)
{
    mNaviTree       = treeView;
    mPrioError      = prioError;
    mPrioWarning    = prioWarning;
    mPrioInfo       = prioInfo;
    mPrioDebug      = prioDebug;
    mPrioScopes     = prioScopes;
    Q_ASSERT(mScopesModel != nullptr);

    validateControls();

    connect(mPrioError  , &QToolButton::clicked, this, [this](bool checked) {onLogPrioChecked(checked, *mPrioError  , NELogging::eLogPriority::PrioError);  });
    connect(mPrioWarning, &QToolButton::clicked, this, [this](bool checked) {onLogPrioChecked(checked, *mPrioWarning, NELogging::eLogPriority::PrioWarning);});
    connect(mPrioInfo   , &QToolButton::clicked, this, [this](bool checked) {onLogPrioChecked(checked, *mPrioInfo   , NELogging::eLogPriority::PrioInfo);   });
    connect(mPrioDebug  , &QToolButton::clicked, this, [this](bool checked) {onLogPrioChecked(checked, *mPrioDebug  , NELogging::eLogPriority::PrioDebug);  });
    connect(mPrioScopes , &QToolButton::clicked, this, [this](bool checked) {onLogPrioChecked(checked, *mPrioScopes , NELogging::eLogPriority::PrioScope);  });
    if (mSelModel != nullptr)
    {
        mNaviTree->setModel(mScopesModel);
        mNaviTree->setSelectionModel(mSelModel);
        connect(mSelModel   , &QItemSelectionModel::currentRowChanged,this, [this](const QModelIndex &current, const QModelIndex &previous) {onRowChanged(current, previous);});
    }
}

inline void NaviLogScopeBase::validateControls(void)
{
    Q_ASSERT(mPrioDebug     != nullptr);
    Q_ASSERT(mPrioInfo      != nullptr);
    Q_ASSERT(mPrioWarning   != nullptr);
    Q_ASSERT(mPrioError     != nullptr);
    Q_ASSERT(mPrioScopes    != nullptr);
    Q_ASSERT(mNaviTree      != nullptr);
    Q_ASSERT(mScopesModel   != nullptr);
}

bool NaviLogScopeBase::areRootsCollapsed(void) const
{
    bool result{ false };
    if ((mNaviTree != nullptr) && (mScopesModel != nullptr))
    {
        result = true;
        int rowCount = mScopesModel != nullptr ? mScopesModel->rowCount(mScopesModel->getRootIndex()) : 0; // root items
        for (int row = 0; row < rowCount; ++row)
        {
            QModelIndex index = mScopesModel->index(row, 0, mScopesModel->getRootIndex());
            if (mNaviTree->isExpanded(index))
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

            if (node->hasLogScopes())
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
    QTreeView* tree = current.isValid() && (mScopesModel != nullptr) ? mNaviTree : nullptr;
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

bool NaviLogScopeBase::updatePriority(const QModelIndex& node, bool addPrio, NELogging::eLogPriority prio)
{
    bool result{ false };
    if (node.isValid())
    {
        Q_ASSERT(mScopesModel != nullptr);
        if (addPrio)
        {
            result = mScopesModel->addLogPriority(node, prio);
        }
        else
        {
            result = mScopesModel->removeLogPriority(node, prio);
        }
    }

    return result;
}

void NaviLogScopeBase::expandChildNodesRecursive(const QModelIndex& idxNode, const ScopeNodeBase& node)
{
    if (node.isLeaf() || (idxNode.isValid() == false))
        return; // No children to expand

    QTreeView* navi = mNaviTree;
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
    QTreeView* treeView = mNaviTree;
    int rowCount = mScopesModel != nullptr ? mScopesModel->rowCount(mScopesModel->getRootIndex()) : 0; // root items
    for (int row = 0; row < rowCount; ++row)
    {
        QModelIndex index = mScopesModel->index(row, 0, mScopesModel->getRootIndex());
        treeView->collapse(index);
        mScopesModel->nodeCollapsed(index);
    }
}

void NaviLogScopeBase::onLogPrioChecked(bool checked, QToolButton& toolButton, NELogging::eLogPriority prio)
{
    validateControls();
    QModelIndex current = mNaviTree->currentIndex();
    if (updatePriority(current, checked, prio) == false)
    {
        toolButton.setChecked(!checked);
    }
}

void NaviLogScopeBase::onNodeExpanded(const QModelIndex& index, bool expanded, QToolButton* toolButton)
{
    if ((toolButton != nullptr))
    {
        if (expanded && (areRootsCollapsed() == false))
        {
            toolButton->setIcon(NELusanCommon::iconNodeCollapsed(NELusanCommon::SizeSmall));
            toolButton->setChecked(false);
        }
        else if ((expanded == false) && areRootsCollapsed())
        {
            toolButton->setIcon(NELusanCommon::iconNodeExpanded(NELusanCommon::SizeSmall));
            toolButton->setChecked(true);
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

void NaviLogScopeBase::onCollapseClicked(bool checked, QToolButton* button)
{
    Q_ASSERT(mScopesModel != nullptr);
    Q_ASSERT(button != nullptr);
    Q_ASSERT(mNaviTree != nullptr);

    if (mScopesModel->rowCount(mScopesModel->getRootIndex()) == 0)
    {
        button->blockSignals(true);
        button->setChecked(false);
        button->blockSignals(false);
        return;
    }

    if (checked)
    {
        button->blockSignals(true);
        button->setIcon(NELusanCommon::iconNodeExpanded(NELusanCommon::SizeBig));
        button->setChecked(true);

        mNaviTree->blockSignals(true);
        collapseRoots();
        mNaviTree->expand(mScopesModel->getRootIndex());
        mScopesModel->nodeExpanded(mScopesModel->getRootIndex());
        mNaviTree->setCurrentIndex(mScopesModel->getRootIndex());
        mNaviTree->blockSignals(false);

        button->blockSignals(false);
    }
    else
    {
        button->blockSignals(true);
        button->setIcon(NELusanCommon::iconNodeCollapsed(NELusanCommon::SizeBig));
        button->setChecked(false);

        mNaviTree->blockSignals(true);
        mNaviTree->expandAll();
        mScopesModel->nodeTreeExpanded(mScopesModel->getRootIndex());
        mNaviTree->setCurrentIndex(mScopesModel->getRootIndex());
        mNaviTree->blockSignals(false);

        button->blockSignals(false);
    }
}
