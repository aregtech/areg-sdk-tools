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
 *  \file        lusan/model/log/LiveScopesModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log scopes model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/model/log/LiveScopesModel.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"

#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/data/log/LogObserver.hpp"

LiveScopesModel::LiveScopesModel(QObject* parent)
    : LoggingScopesModelBase( parent )
    , mPausedSources    ( )
    , mConSourceState   ( )
    , mConConfigRestored( )
{
}

LiveScopesModel::~LiveScopesModel()
{
    _setupObserverSignals(false);
}

void LiveScopesModel::setLoggingModel(LoggingModelBase* model)
{
    LoggingScopesModelBase::setLoggingModel(model);
    _setupObserverSignals(model != nullptr);
}

void LiveScopesModel::_setupObserverSignals(bool doSetup)
{
    LogObserver* log{ LogObserver::getComponent() };
    if (doSetup)
    {
        if ((log == nullptr) || mConSourceState)
            return;

        mConSourceState = connect(log, &LogObserver::signalLogSourceState, this, [this](ITEM_ID cookie, uint8_t state, ITEM_ID byObserver) {
            _onSourceState(cookie, state, byObserver);
        });

        mConConfigRestored = connect(log, &LogObserver::signalLogConfigRestored, this, [this](ITEM_ID cookie) {
            setTargetState(targetIndex(cookie), ScopeRoot::eTargetState::TargetApplied);
        });
    }
    else
    {
        disconnect(mConSourceState);
        disconnect(mConConfigRestored);
        mConSourceState = QMetaObject::Connection();
        mConConfigRestored = QMetaObject::Connection();
    }
}

void LiveScopesModel::_onSourceState(ITEM_ID cookie, uint8_t state, ITEM_ID byObserver)
{
    const areg::LogSourceState reported{ static_cast<areg::LogSourceState>(state) };
    if (areg::is_source_state_valid(reported) == false)
        return;

    if (reported == areg::LogSourceState::Active)
    {
        mPausedSources.remove(cookie);
    }
    else
    {
        mPausedSources.insert(cookie, qMakePair(reported, byObserver));
    }

    _applySourceState(cookie, reported, byObserver);
}

bool LiveScopesModel::setSourceState(const QModelIndex& node, areg::LogSourceState state)
{
    if (areg::is_source_state_valid(state) == false)
        return false;

    ScopeNodeBase* entry = node.isValid() ? static_cast<ScopeNodeBase*>(node.internalPointer()) : nullptr;
    ScopeNodeBase* treeRoot = entry != nullptr ? entry->getTreeRoot() : nullptr;

    if (treeRoot == nullptr)
    {
        const bool result{ LogObserver::requestSourceState(areg::TARGET_ALL, state) };
        if (result)
        {
            _markAllSourceRequests(state);
        }

        return result;
    }

    ScopeRoot* root{ static_cast<ScopeRoot *>(treeRoot) };
    const bool result{ LogObserver::requestSourceState(root->getRootId(), state) };
    if (result)
    {
        root->markSourceRequest(state);
        const QModelIndex idxRoot{ targetIndex(root->getRootId()) };
        emit dataChanged(idxRoot, idxRoot, QList<int>{ LoggingScopesModelBase::RoleSourceWait
                                                     , Qt::ItemDataRole::ToolTipRole });
        emit signalSafeguardsChanged();
    }

    return result;
}

bool LiveScopesModel::restoreConfiguration(const QModelIndex& node)
{
    ScopeNodeBase* entry = node.isValid() ? static_cast<ScopeNodeBase*>(node.internalPointer()) : nullptr;
    ScopeNodeBase* treeRoot = entry != nullptr ? entry->getTreeRoot() : nullptr;
    const ITEM_ID target{ treeRoot != nullptr ? static_cast<ScopeRoot *>(treeRoot)->getRootId() : areg::TARGET_ALL };

    return LogObserver::requestRestoreConfig(target);
}

void LiveScopesModel::_markAllSourceRequests(areg::LogSourceState wanted)
{
    if (mLoggingModel == nullptr)
        return;

    LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
    for (int pos = 0; pos < static_cast<int>(roots.size()); ++pos)
    {
        ScopeRoot* root = roots[pos];
        if (root != nullptr)
        {
            root->markSourceRequest(wanted);
        }
    }

    if (roots.empty() == false)
    {
        const QModelIndex idxFirst{ index(0, LoggingScopesModelBase::ColumnName, mRootIndex) };
        const QModelIndex idxLast { index(static_cast<int>(roots.size()) - 1, LoggingScopesModelBase::ColumnName, mRootIndex) };
        emit dataChanged(idxFirst, idxLast, QList<int>{ LoggingScopesModelBase::RoleSourceWait
                                                      , Qt::ItemDataRole::ToolTipRole });
        emit signalSafeguardsChanged();
    }
}

bool LiveScopesModel::isLiveSession() const
{
    return true;
}

bool LiveScopesModel::setLogPriority(const QModelIndex& index, uint32_t prio)
{
    bool result{ false };
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if (root == nullptr)
        return result;
    
    if (node->getPriority() != prio)
    {
        node->setPriority(prio);
        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        
        ScopeInfo scope;
        scope.lsId = 0;
        scope.lsPrio = node->getPriority();
        QString path = node->makePath();
        if (node->isLeaf() == false)
        {
            path += NELusanCommon::SCOPE_ALL;
        }
        
        areg::copy_string(scope.lsName, LENGTH_SCOPE, path.toStdString().c_str());
        result = LogObserver::requestChangeScopePrio(static_cast<ScopeRoot *>(root)->getRootId(), &scope, 1);
        setTargetState(index, result ? ScopeRoot::eTargetState::TargetSent : ScopeRoot::eTargetState::TargetPending);
    }
    else
    {
        result = true;
    }

    return result;
}

bool LiveScopesModel::addLogPriority(const QModelIndex& index, uint32_t prio)
{
    bool result{ false };
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if (root == nullptr)
        return result;
    
    if (node->canAddPriority(prio))
    {
        node->addPriority(prio);        
        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        
        result = _requestNodePriority(static_cast<const ScopeRoot &>(*root), *node);
        setTargetState(index, result ? ScopeRoot::eTargetState::TargetSent : ScopeRoot::eTargetState::TargetPending);
    }
    else
    {
        result = true;
    }

    return result;
}

bool LiveScopesModel::removLogPriority(const QModelIndex& index, uint32_t prio)
{
    bool result{ false };
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if (root == nullptr)
        return result;
    
    // if ((node->getPriority() & static_cast<uint32_t>(prio)) != 0)
    if (node->canRemovePriority(prio))
    {
        node->removePriority(prio);
        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        result = _requestNodePriority(static_cast<const ScopeRoot &>(*root), *node);
        setTargetState(index, result ? ScopeRoot::eTargetState::TargetSent : ScopeRoot::eTargetState::TargetPending);
    }
    else
    {
        result = true;
    }

    return result;
}

bool LiveScopesModel::saveLogScopePriority(const QModelIndex& target /*= QModelIndex()*/)
{
    ScopeNodeBase* node = target.isValid() ? static_cast<ScopeNodeBase *>(target.internalPointer()) : nullptr;
    if (node != nullptr)
    {
        ScopeRoot* root = static_cast<ScopeRoot *>(node->getTreeRoot());
        const bool result{ LogObserver::requestSaveConfig(root->getRootId()) };
        setTargetState(target, result ? ScopeRoot::eTargetState::TargetSaved : ScopeRoot::eTargetState::TargetPending);
        return result;
    }

    const bool result{ LogObserver::requestSaveConfig(areg::TARGET_ALL) };
    if (mLoggingModel != nullptr)
    {
        const LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        for (int pos = 0; pos < static_cast<int>(roots.size()); ++pos)
        {
            setTargetState(index(pos, LoggingScopesModelBase::ColumnName, mRootIndex)
                          , result ? ScopeRoot::eTargetState::TargetSaved : ScopeRoot::eTargetState::TargetPending);
        }
    }

    return result;
}

bool LiveScopesModel::slotInstancesAvailable(const std::vector<areg::ConnectedInstance> & instances)
{
    if (LoggingScopesModelBase::slotInstancesAvailable(instances))
    {
        for (const auto & entry : instances)
        {
            LogObserver::requestScopes(entry.ciCookie);

            // The collector may have named a stopped target before its tree row existed.
            const auto found{ mPausedSources.constFind(entry.ciCookie) };
            if (found != mPausedSources.constEnd())
            {
                _applySourceState(entry.ciCookie, found.value().first, found.value().second);
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

int LiveScopesModel::applyRememberedPriorities(ScopeRoot & root)
{
    const int count{ LoggingScopesModelBase::applyRememberedPriorities(root) };
    if (count != 0)
    {
        _requestNodePriority(root, root);
    }

    return count;
}

bool LiveScopesModel::_requestNodePriority(const ScopeRoot& root, const ScopeNodeBase& node)
{
    bool result{false};
    QList<ScopeNodeBase*> nodes;
    int count = node.extractChildNodesWithPriority(nodes);
    count += count == 0 ? 1 : 0;
    ScopeInfo* scopes = new ScopeInfo[count];
    if (scopes != nullptr)
    {
        int pos = 0;
        if (nodes.isEmpty())
        {
            // Q_ASSERT(node.hasPrioNotset());
            ScopeInfo& scope = scopes[0];
            scope.lsId = 0;
            scope.lsPrio = node.getPriority();
            QString path = node.makePath();
            if (node.isLeaf() == false)
            {
                path += NELusanCommon::SCOPE_ALL;
            }

            areg::mem_copy(scope.lsName, LENGTH_SCOPE, path.toStdString().c_str(), path.length() + 1);
            ++pos;
        }

        for (; pos < count; ++pos)
        {
            ScopeNodeBase* nodeBase = nodes[pos];
            ScopeInfo& scope = scopes[pos];
            scope.lsId = 0;
            scope.lsPrio = nodeBase->getPriority();
            QString path = nodeBase->makePath();
            if (nodeBase->isLeaf() == false)
            {
                path += NELusanCommon::SCOPE_ALL;
            }

            areg::mem_copy(scope.lsName, LENGTH_SCOPE, path.toStdString().c_str(), path.length() + 1);
        }

        result = LogObserver::requestChangeScopePrio(root.getRootId(), scopes, count);
        delete[] scopes;
    }
    
    return result;
}
