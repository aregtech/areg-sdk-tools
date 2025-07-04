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
 *  \file        lusan/model/log/LogScopesModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log scopes model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/model/log/LogScopesModel.hpp"
#include "lusan/model/log/LogScopeIconFactory.hpp"
#include "lusan/model/log/LogViewerModel.hpp"

#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/data/log/LogObserver.hpp"

LogScopesModel::LogScopesModel(QObject* parent)
    : LoggingScopesModelBase( parent )
    , mLogViewerModel   (nullptr)
{
}

bool LogScopesModel::initialize(LogViewerModel* logViewerModel)
{
    _clear();
    
    if (logViewerModel == nullptr)
        return false;
    
    mLogViewerModel = logViewerModel;

    connect(mLogViewerModel, &LogViewerModel::signalLogInstancesConnect    , this, &LogScopesModel::slotLogInstancesConnect);
    connect(mLogViewerModel, &LogViewerModel::signalLogInstancesDisconnect , this, &LogScopesModel::slotLogInstancesDisconnect);
    connect(mLogViewerModel, &LogViewerModel::signalLogServiceDisconnected , this, &LogScopesModel::slotLogServiceDisconnected);
    connect(mLogViewerModel, &LogViewerModel::signalLogRegisterScopes      , this, &LogScopesModel::slotLogRegisterScopes);
    connect(mLogViewerModel, &LogViewerModel::signalLogUpdateScopes        , this, &LogScopesModel::slotLogUpdateScopes);
    
    return LogObserver::requestInstances();
}

void LogScopesModel::release(void)
{
    if (mLogViewerModel != nullptr)
    {
        disconnect(mLogViewerModel, &LogViewerModel::signalLogInstancesConnect     , this, &LogScopesModel::slotLogInstancesConnect);
        disconnect(mLogViewerModel, &LogViewerModel::signalLogInstancesDisconnect  , this, &LogScopesModel::slotLogInstancesDisconnect);
        disconnect(mLogViewerModel, &LogViewerModel::signalLogServiceDisconnected  , this, &LogScopesModel::slotLogServiceDisconnected);
        disconnect(mLogViewerModel, &LogViewerModel::signalLogRegisterScopes       , this, &LogScopesModel::slotLogRegisterScopes);
        disconnect(mLogViewerModel, &LogViewerModel::signalLogUpdateScopes         , this, &LogScopesModel::slotLogUpdateScopes);
        mLogViewerModel = nullptr;
    }
}

bool LogScopesModel::setLogPriority(const QModelIndex& index, NELogging::eLogPriority prio)
{
    bool result{ false };
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if (root == nullptr)
        return result;
    
    if (node->getPriority() != static_cast<uint32_t>(prio))
    {
        node->setPriority(static_cast<uint32_t>(prio));
        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        
        sLogScope scope;
        scope.lsId = 0;
        scope.lsPrio = node->getPriority();
        QString path = node->makePath();
        if (node->isLeaf() == false)
        {
            path += NELusanCommon::SCOPE_ALL;
        }
        
        NEString::copyString(scope.lsName, LENGTH_SCOPE, path.toStdString().c_str());
        result = LogObserver::requestChangeScopePrio(static_cast<ScopeRoot *>(root)->getRootId(), &scope, 1);
    }
    else
    {
        result = true;
    }

    return result;
}

bool LogScopesModel::addLogPriority(const QModelIndex& index, NELogging::eLogPriority prio)
{
    bool result{ false };
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if (root == nullptr)
        return result;
    
    if (((node->getPriority() & static_cast<uint32_t>(prio)) == 0) || node->hasMultiPrio(static_cast<uint32_t>(prio)))
    {
        node->addPriority(static_cast<uint32_t>(prio));        
        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        
        result = _requestNodePriority(static_cast<const ScopeRoot &>(*root), *node);
    }
    else
    {
        result = true;
    }

    return result;
}

bool LogScopesModel::removeLogPriority(const QModelIndex& index, NELogging::eLogPriority prio)
{
    bool result{ false };
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if (root == nullptr)
        return result;
    
    if ((node->getPriority() & static_cast<uint32_t>(prio)) != 0)
    {
        node->removePriority(static_cast<uint32_t>(prio));
        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        result = _requestNodePriority(static_cast<const ScopeRoot &>(*root), *node);
    }
    else
    {
        result = true;
    }

    return result;
}

bool LogScopesModel::saveLogScopePriority(const QModelIndex& target /*= QModelIndex()*/) const
{
    if (target.isValid())
    {
        ScopeNodeBase* node = static_cast<ScopeNodeBase *>(target.internalPointer());
        Q_ASSERT(node != nullptr);
        ScopeRoot* root = static_cast<ScopeRoot *>(node->getTreeRoot());
        return LogObserver::requestSaveConfig(root->getRootId());
    }
    else
    {
        return LogObserver::requestSaveConfig(NEService::TARGET_ALL);
    }
}

bool LogScopesModel::_requestNodePriority(const ScopeRoot& root, const ScopeNodeBase& node)
{
    bool result{false};
    QList<ScopeNodeBase*> nodes;
    int count = node.extractChildNodesWithPriority(nodes);
    count += count == 0 ? 1 : 0;
    sLogScope* scopes = new sLogScope[count];
    if (scopes != nullptr)
    {
        int pos = 0;
        if (nodes.isEmpty())
        {
            Q_ASSERT(node.hasPrioNotset());
            sLogScope& scope = scopes[0];
            scope.lsId = 0;
            scope.lsPrio = node.getPriority();
            QString path = node.makePath();
            if (node.isLeaf() == false)
            {
                path += NELusanCommon::SCOPE_ALL;
            }

            NEString::copyString(scope.lsName, LENGTH_SCOPE, path.toStdString().c_str());
            ++pos;
        }

        for (; pos < count; ++pos)
        {
            ScopeNodeBase* nodeBase = nodes[pos];
            sLogScope& scope = scopes[pos];
            scope.lsId = 0;
            scope.lsPrio = nodeBase->getPriority();
            QString path = nodeBase->makePath();
            if (nodeBase->isLeaf() == false)
            {
                path += NELusanCommon::SCOPE_ALL;
            }

            NEString::copyString(scope.lsName, LENGTH_SCOPE, path.toStdString().c_str());
        }

        result = LogObserver::requestChangeScopePrio(root.getRootId(), scopes, count);
        delete[] scopes;
    }
    
    return result;
}

void LogScopesModel::slotLogInstancesConnect(const QList<NEService::sServiceConnectedInstance>& instances)
{
    beginResetModel();
    mRootIndex = createIndex(0, 0, nullptr);
    for (const NEService::sServiceConnectedInstance& instance : instances)
    {
        if (instance.ciSource == NEService::eMessageSource::MessageSourceObserver)
            continue;

        ScopeRoot* root = new ScopeRoot(instance);
        if (_appendRoot(root, true))
        {
            LogObserver::requestScopes(root->getRootId());
        }
        else
        {
            delete root;
        }
    }

    endResetModel();
    emit signalRootUpdated(mRootIndex);
}

void LogScopesModel::slotLogInstancesDisconnect(const QList<NEService::sServiceConnectedInstance>& instances)
{
    for (const NEService::sServiceConnectedInstance& instance : instances)
    {
        int pos = _findRoot(instance.ciCookie);
        if (pos != static_cast<int>(NECommon::INVALID_INDEX))
        {
            beginRemoveRows(mRootIndex, pos, pos);
            mRootList.erase(mRootList.begin() + pos);
            endRemoveRows();
        }
    }
    
    emit signalRootUpdated(mRootIndex);
}

void LogScopesModel::slotLogServiceDisconnected(const QMap<ITEM_ID, NEService::sServiceConnectedInstance>& instances)
{
    beginResetModel();
    _clear();
    endResetModel();
}

void LogScopesModel::slotLogRegisterScopes(ITEM_ID cookie, const QList<sLogScope*>& scopes)
{
    int pos = scopes.isEmpty() == false ? _findRoot(cookie) : static_cast<int>(NECommon::INVALID_INDEX);
    if (pos != static_cast<int>(NECommon::INVALID_INDEX))
    {
        int count = static_cast<int>(scopes.size());
        QModelIndex idxInstance = index(pos, 0, mRootIndex);
        beginInsertRows(idxInstance, 0, count);

        ScopeRoot* root = mRootList[pos];
        Q_ASSERT(root != nullptr);
        root->resetPrioritiesRecursive(false);
        for (int i = 0; i < count; ++i)
        {
            const sLogScope* scope = scopes[i];
            Q_ASSERT(scope != nullptr);
            QString scopeName{scope->lsName};
            root->addChildRecursive(scopeName, scope->lsPrio);
        }
        
        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        
        endInsertRows();
        emit signalScopesInserted(idxInstance);
    }
}

void LogScopesModel::slotLogUpdateScopes(ITEM_ID cookie, const QList<sLogScope*>& scopes)
{
    int pos = scopes.isEmpty() == false ? _findRoot(cookie) : static_cast<int>(NECommon::INVALID_INDEX);
    if (pos != static_cast<int>(NECommon::INVALID_INDEX))
    {
        QModelIndex idxInstance = index(pos, 0, mRootIndex);
        int count = static_cast<int>(scopes.size());
        ScopeRoot* root = mRootList[pos];
        Q_ASSERT(root != nullptr);
        for (int i = 0; i < static_cast<int>(scopes.size()); ++i)
        {
            const sLogScope* scope = scopes[i];
            Q_ASSERT(scope != nullptr);
            QString scopeName{scope->lsName};
            root->addChildPriorityRecursive(scopeName, scope->lsPrio);
        }
        
        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        
        QModelIndex entry = index(pos, 0, mRootIndex);
        emit signalScopesUpdated(idxInstance);
        emit dataChanged(entry, entry, { Qt::ItemDataRole::DecorationRole, Qt::ItemDataRole::DisplayRole });
    }
}
