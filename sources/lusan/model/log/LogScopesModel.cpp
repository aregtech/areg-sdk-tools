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

#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/data/log/LogObserver.hpp"

LogScopesModel::LogScopesModel(QObject* parent)
    : QAbstractItemModel( parent )
    , mRootList         ( )
    , mRootIndex        ( )
{
    mRootIndex = createIndex(0, 0, nullptr);
}

LogScopesModel::~LogScopesModel(void)
{
    _clear();
}

bool LogScopesModel::initialize(void)
{
    _clear();
    LogObserver* log = LogObserver::getComponent();
    if ((log == nullptr) || (log->isConnected() == false))
        return false;

    connect(log, &LogObserver::signalLogInstancesConnect    , this, &LogScopesModel::slotLogInstancesConnect);
    connect(log, &LogObserver::signalLogInstancesDisconnect , this, &LogScopesModel::slotLogInstancesDisconnect);
    connect(log, &LogObserver::signalLogServiceDisconnected , this, &LogScopesModel::slotLogServiceDisconnected);
    connect(log, &LogObserver::signalLogRegisterScopes      , this, &LogScopesModel::slotLogRegisterScopes);
    connect(log, &LogObserver::signalLogUpdateScopes        , this, &LogScopesModel::slotLogUpdateScopes);
    
    return LogObserver::requestInstances();
}

void LogScopesModel::release(void)
{
    LogObserver* log = LogObserver::getComponent();
    if (log != nullptr)
    {
        disconnect(log, &LogObserver::signalLogInstancesConnect     , this, &LogScopesModel::slotLogInstancesConnect);
        disconnect(log, &LogObserver::signalLogInstancesDisconnect  , this, &LogScopesModel::slotLogInstancesDisconnect);
        disconnect(log, &LogObserver::signalLogServiceDisconnected  , this, &LogScopesModel::slotLogServiceDisconnected);
        disconnect(log, &LogObserver::signalLogRegisterScopes       , this, &LogScopesModel::slotLogRegisterScopes);
        disconnect(log, &LogObserver::signalLogUpdateScopes         , this, &LogScopesModel::slotLogUpdateScopes);
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
    
    if ((node->getPriority() & static_cast<uint32_t>(prio)) == 0)
    {
        node->addPriority(static_cast<uint32_t>(prio));        
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

QModelIndex LogScopesModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((hasIndex(row, column, parent) == false) || (column != 0))
        return QModelIndex();
    
    if (parent.isValid() == false)
        return mRootIndex;
    
    if (parent == mRootIndex)
    {
        ScopeRoot* root = mRootList[row];
        return createIndex(row, column, root);
    }
    else
    {
        ScopeNodeBase* parentNode = static_cast<ScopeNodeBase*>(parent.internalPointer());
        Q_ASSERT(parentNode != nullptr);
        ScopeNodeBase* childNode = parentNode->getChildAt(row);
        return createIndex(row, column, childNode);
    }
}

QModelIndex LogScopesModel::parent(const QModelIndex& child) const
{
    if ((child.isValid() == false) || (child == mRootIndex))
        return QModelIndex();
    
    ScopeNodeBase* childNode = static_cast<ScopeNodeBase *>(child.internalPointer());
    Q_ASSERT(childNode != nullptr);
    if (childNode->isRoot())
        return mRootIndex;
    
    ScopeNodeBase* parentNode= childNode->getParent();
    Q_ASSERT(parentNode != nullptr);
    if (parentNode->isRoot())
    {
        int pos = _findRoot(static_cast<ScopeRoot *>(parentNode)->getRootId());
        Q_ASSERT(pos != static_cast<int>(NECommon::INVALID_INDEX));
        return createIndex(pos, 0, parentNode);
    }
    else
    {
        ScopeNodeBase* grandParent = parentNode->getParent();
        int pos = grandParent->getChildPosition(parentNode->getNodeName());
        Q_ASSERT(pos != static_cast<int>(NECommon::INVALID_INDEX));
        return createIndex(pos, 0, parentNode);
    }
}

int LogScopesModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() == false)
        return 1;
    
    if (parent == mRootIndex)
        return static_cast<int>(mRootList.size());
    
    ScopeNodeBase* parentNode = static_cast<ScopeNodeBase*>(parent.internalPointer());
    Q_ASSERT(parentNode != nullptr);
    return parentNode->getChildCount();
}

int LogScopesModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant LogScopesModel::data(const QModelIndex& index, int role) const
{
    if (isValidIndex(index) == false)
        return QVariant();
    
    if (index == mRootIndex)
    {
        return (static_cast<Qt::ItemDataRole>(role) == Qt::ItemDataRole::DisplayRole ? QVariant(tr("Live Logs")) : QVariant());
    }
    
    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
    {
        ScopeNodeBase* entry{ static_cast<ScopeNodeBase*>(index.internalPointer()) };
        return entry->getNodeName();
    }
    
    case Qt::ItemDataRole::DecorationRole:
    {
        ScopeNodeBase* entry{ static_cast<ScopeNodeBase*>(index.internalPointer()) };
        // return entry->getIcon();
        return LogScopeIconFactory::getIcon(entry->getPriority());
    }
    
    case Qt::ItemDataRole::UserRole:
    {
        ScopeNodeBase* entry{ static_cast<ScopeNodeBase*>(index.internalPointer()) };
        return QVariant::fromValue<ScopeNodeBase *>(entry);
    }
        
    default:
        return QVariant();
    }
}

QVariant LogScopesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

Qt::ItemFlags LogScopesModel::flags(const QModelIndex& index) const
{
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    if ((node == nullptr) || (index == mRootIndex))
        return Qt::NoItemFlags;
    else if (node->isLeaf())
        return (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    else
        return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

inline void LogScopesModel::_clear(void)
{
    for (auto root : mRootList)
    {
        Q_ASSERT(root != nullptr);
        delete root;
    }

    mRootList.clear();
}

inline bool LogScopesModel::_exists(ITEM_ID rootId) const
{
    for (auto root : mRootList)
    {
        Q_ASSERT(root != nullptr);
        if (root->getRootId() == rootId)
            return true;
    }

    return false;
}

inline bool LogScopesModel::_appendRoot(ScopeRoot* root, bool unique /*= true*/)
{
    if ((unique == false) || (_exists(root->getRootId()) == false))
    {
        mRootList.append(root);
        return true;
    }

    return false;
}

inline int LogScopesModel::_findRoot(ITEM_ID rootId) const
{
    for (int i = 0; i < mRootList.size(); ++i)
    {
        if (mRootList[i]->getRootId() == rootId)
            return i;
    }

    return static_cast<int>(NECommon::INVALID_INDEX);
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

        endInsertRows();
        emit signalScopesInserted(idxInstance);
    }
}

void LogScopesModel::slotLogUpdateScopes(ITEM_ID cookie, const QList<sLogScope*>& scopes)
{
    int pos = scopes.isEmpty() == false ? _findRoot(cookie) : static_cast<int>(NECommon::INVALID_INDEX);
    if (pos != static_cast<int>(NECommon::INVALID_INDEX))
    {
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
        emit dataChanged(entry, entry, { Qt::ItemDataRole::DecorationRole, Qt::ItemDataRole::DisplayRole });
    }
}
