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
 *  \file        lusan/model/log/LoggingScopesModelBase.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Base class for log scopes models.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/log/LoggingScopesModelBase.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/model/log/LogIconFactory.hpp"

#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/common/NELusanCommon.hpp"

LoggingScopesModelBase::LoggingScopesModelBase(QObject* parent)
    : QAbstractItemModel( parent )
    , mRootIndex        ( )
    , mLoggingModel     ( nullptr )
    
    , mSignalsSetup         ( false )
    , mConSvcConnected      ( )
    , mConSvcDisconnected   ( )
    , mConInstAvailable     ( )
    , mConInstUnavailable   ( )
    , mConScopesAvailable   ( )
    , mConScopesUnavailable ( )
{
    mRootIndex = createIndex(0, 0, nullptr);
}

LoggingScopesModelBase::~LoggingScopesModelBase(void)
{
    _setupSignals(false);
    clearModel(false);
    mLoggingModel = nullptr;
}

void LoggingScopesModelBase::nodeExpanded(const QModelIndex& idxNode)
{
    if (mLoggingModel != nullptr)
    {
        ScopeNodeBase* node = idxNode.isValid() ? static_cast<ScopeNodeBase*>(idxNode.internalPointer()) : nullptr;
        if (node != nullptr)
        {
            node->setNodeState(true);
        }
        else if (idxNode == mRootIndex)
        {
            LoggingModelBase::RootList & roots = mLoggingModel->getRootList();
            for (auto root : roots)
            {
                Q_ASSERT(root != nullptr);
                root->setNodeState(true);
            }
        }
    }
}

void LoggingScopesModelBase::nodeCollapsed(const QModelIndex& idxNode)
{
    if (mLoggingModel != nullptr)
    {
        ScopeNodeBase* node = idxNode.isValid() ? static_cast<ScopeNodeBase*>(idxNode.internalPointer()) : nullptr;
        if (node != nullptr)
        {
            node->setNodeState(false);
        }
        else if ((idxNode == mRootIndex))
        {
            LoggingModelBase::RootList & roots = mLoggingModel->getRootList();
            for (auto root : roots)
            {
                Q_ASSERT(root != nullptr);
                root->setNodeState(false);
            }
        }
    }
}

void LoggingScopesModelBase::nodeSelected(const QModelIndex& idxNode)
{
    if (mLoggingModel != nullptr)
    {
        mLoggingModel->setSelectedScope(idxNode);
    }
}

void LoggingScopesModelBase::nodeTreeExpanded(const QModelIndex& idxNode)
{
    if (mLoggingModel != nullptr)
    {
        ScopeNodeBase* node = idxNode.isValid() ? static_cast<ScopeNodeBase*>(idxNode.internalPointer()) : nullptr;
        if (node != nullptr)
        {
            node->setNodeTreeExpanded();
        }
        else if ((idxNode == mRootIndex))
        {
            LoggingModelBase::RootList & roots = mLoggingModel->getRootList();
            for (auto root : roots)
            {
                Q_ASSERT(root != nullptr);
                root->setNodeTreeExpanded();
            }
        }
    }
}

void LoggingScopesModelBase::nodeTreeCollapsed(const QModelIndex& idxNode)
{
    if (mLoggingModel != nullptr)
    {
        ScopeNodeBase* node = idxNode.isValid() ? static_cast<ScopeNodeBase*>(idxNode.internalPointer()) : nullptr;
        if (node != nullptr)
        {
            node->setNodeTreeCollapsed();
        }
        else if ((idxNode == mRootIndex))
        {
            LoggingModelBase::RootList & roots = mLoggingModel->getRootList();
            for (auto root : roots)
            {
                Q_ASSERT(root != nullptr);
                root->setNodeTreeCollapsed();
            }
        }
    }
}

void LoggingScopesModelBase::setLoggingModel(LoggingModelBase* model)
{
    if (model != nullptr)
    {
        if (mLoggingModel == model)
            return;

        if (mLoggingModel != nullptr)
        {
            // If the model is already set, disconnect from the previous one
            _setupSignals(false);
        }

        mLoggingModel = model;
        _setupSignals(true);        
        slotLogServiceConnected();
    }
    else if (mLoggingModel != nullptr)
    {
        _setupSignals(false);
        mLoggingModel = nullptr;
    }
}

QModelIndex LoggingScopesModelBase::index(int row, int column, const QModelIndex& parent) const
{
    if ((hasIndex(row, column, parent) == false) || (column != 0) || (mLoggingModel == nullptr))
        return QModelIndex();

    const LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
    ScopeNodeBase* parentNode = parent.isValid() ? static_cast<ScopeNodeBase*>(parent.internalPointer()) : nullptr;
    
    if (parentNode == nullptr)
    {
        ScopeRoot* root = (row >= 0) && (row < static_cast<int>(roots.size())) ? roots[row] : nullptr;
        return (root != nullptr ? createIndex(row, column, root) : mRootIndex);
    }
    else
    {
        ScopeNodeBase* childNode = parentNode->getChildAt(row);
        return (childNode != nullptr ? createIndex(row, column, childNode) : QModelIndex());
    }
}

QModelIndex LoggingScopesModelBase::parent(const QModelIndex& child) const
{
    if ((child.isValid() == false) || (child == mRootIndex) || (mLoggingModel == nullptr))
        return QModelIndex();

    ScopeNodeBase* childNode = static_cast<ScopeNodeBase*>(child.internalPointer());
    Q_ASSERT(childNode != nullptr);
    if (childNode->isRoot())
        return mRootIndex;

    ScopeNodeBase* parentNode = childNode->getParent();
    Q_ASSERT(parentNode != nullptr);
    if (parentNode->isRoot())
    {
        int pos = findRoot(static_cast<ScopeRoot*>(parentNode)->getRootId());
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

int LoggingScopesModelBase::rowCount(const QModelIndex& parent) const
{
    ScopeNodeBase* node = parent.isValid() ? static_cast<ScopeNodeBase*>(parent.internalPointer()) : nullptr;
    if (node == nullptr)
    {
        // Root level - return number of instances
        return (mLoggingModel != nullptr ? static_cast<int>(mLoggingModel->getRootList().size()) : 0);
    }
    else
    {
        // Child level - get from the node
        return node->getChildCount();
    }
}

int LoggingScopesModelBase::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant LoggingScopesModelBase::data(const QModelIndex& index, int role) const
{
    if (isValidIndex(index) == false)
        return QVariant();
    
    if (index == mRootIndex)
    {
        return (static_cast<Qt::ItemDataRole>(role) == Qt::ItemDataRole::DisplayRole ? QVariant(tr("Scopes")) : QVariant());
    }
    
    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
    {
        ScopeNodeBase* entry{ static_cast<ScopeNodeBase*>(index.internalPointer()) };
        return entry->getDisplayName();
    }
    
    case Qt::ItemDataRole::DecorationRole:
    {
        ScopeNodeBase* entry{ static_cast<ScopeNodeBase*>(index.internalPointer()) };
        return LogIconFactory::getIcon(entry->getPriority());
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

QVariant LoggingScopesModelBase::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole) && (section == 0))
    {
        return QString("Scopes");
    }

    return QVariant();
}

Qt::ItemFlags LoggingScopesModelBase::flags(const QModelIndex& index) const
{
    if (index.isValid() == false)
    {
        return Qt::NoItemFlags;
    }
    else
    {
        ScopeNodeBase* node = static_cast<ScopeNodeBase*>(index.internalPointer());
        if ((node != nullptr) && (node->isLeaf()))
        {
            return (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
        }
        else
        {
            return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        }
    }
}

void LoggingScopesModelBase::buildScope(ScopeRoot& root, QString& scopePath, uint32_t scopePrio, uint32_t scopeId)
{
    root.addChildRecursive(scopePath, scopePrio, scopeId);
}

void LoggingScopesModelBase::clearModel(bool notify /*= false*/)
{
    if (notify)
    {
        beginResetModel();
        endResetModel();
    }
}

bool LoggingScopesModelBase::existsRoot(ITEM_ID rootId) const
{
    if (mLoggingModel != nullptr)
    {
        LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        for (auto root : roots)
        {
            Q_ASSERT(root != nullptr);
            if (root->getRootId() == rootId)
                return true;
        }
    }

    return false;
}

bool LoggingScopesModelBase::appendRoot(ScopeRoot* root, bool unique /*= true*/)
{
    if ((mLoggingModel != nullptr) && ((unique == false) || (existsRoot(root->getRootId()) == false)))
    {
        mLoggingModel->getRootList().push_back(root);
        return true;
    }

    return false;
}

int LoggingScopesModelBase::findRoot(ITEM_ID rootId) const
{
    if (mLoggingModel != nullptr)
    {
        const LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        for (int i = 0; i < static_cast<int>(roots.size()); ++i)
        {
            if (roots[i]->getRootId() == rootId)
                return i;
        }
    }

    return static_cast<int>(NECommon::INVALID_INDEX);
}

void LoggingScopesModelBase::slotLogServiceConnected(void)
{
    clearModel(false);
}

void LoggingScopesModelBase::slotLogServiceDisconnected(void)
{
    clearModel(false);
}

bool LoggingScopesModelBase::slotInstancesAvailable(const std::vector<NEService::sServiceConnectedInstance> & instances)
{
    bool result {false};
    beginResetModel();
    for (const auto & instance : instances)
    {
        if ((instance.ciSource != NEService::eMessageSource::MessageSourceObserver) && (existsRoot(instance.ciCookie) == false))
        {
            result = true;
            ScopeRoot* root = new ScopeRoot(instance);
            if (appendRoot(root, false) == false)
                delete root;
        }
    }
    
    endResetModel();
    
    if (result)
    {
        emit signalRootUpdated(mRootIndex);
    }
    
    return result;
}

void LoggingScopesModelBase::slotInstancesUnavailable(const std::vector<ITEM_ID>& instIds)
{
    bool removed{false};

    if (mLoggingModel != nullptr)
    {
        LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        for (auto rootId : instIds)
        {
            for (int i = 0; i < static_cast<int>(roots.size()); ++i)
            {
                ScopeRoot* root = roots[i];
                Q_ASSERT(root != nullptr);
                if (root->getRootId() == rootId)
                {
                    removed = true;
                    beginRemoveRows(mRootIndex, i, i);
                    roots.erase(roots.begin() + i);
                    endRemoveRows();
                    break;
                }
            }
        }
    }
    
    if (removed)
    {
        emit signalRootUpdated(mRootIndex);
    }
}

void LoggingScopesModelBase::slotScopesAvailable(ITEM_ID instId, const std::vector<NELogging::sScopeInfo>& scopes)
{
    int pos = scopes.empty() == false ? findRoot(instId) : static_cast<int>(NECommon::INVALID_INDEX);
    if (pos != static_cast<int>(NECommon::INVALID_INDEX))
    {
        Q_ASSERT(mLoggingModel != nullptr);
        LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        int count = static_cast<int>(scopes.size());
        QModelIndex idxInstance = index(pos, 0, mRootIndex);
        beginInsertRows(idxInstance, 0, count);
        // beginResetModel();

        ScopeRoot* root = roots[pos];
        Q_ASSERT(root != nullptr);
        root->resetPrioritiesRecursive(false);
        for (int i = 0; i < count; ++i)
        {
            QString scopePath(QString::fromStdString(scopes[i].scopeName.getData()));
            buildScope(*root, scopePath, scopes[i].scopePrio, scopes[i].scopeId);
        }

        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();

        endInsertRows();
        // endResetModel();
        emit signalScopesInserted(idxInstance);
    }
}

void LoggingScopesModelBase::slotScopesUpdated(ITEM_ID instId, const std::vector<NELogging::sScopeInfo>& scopes)
{
    int pos = scopes.empty() == false ? findRoot(instId) : static_cast<int>(NECommon::INVALID_INDEX);
    if (pos != static_cast<int>(NECommon::INVALID_INDEX))
    {
        Q_ASSERT(mLoggingModel != nullptr);
        LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        QModelIndex idxInstance = index(pos, 0, mRootIndex);
        int count = static_cast<int>(scopes.size());
        ScopeRoot* root = roots[pos];
        Q_ASSERT(root != nullptr);
        for (int i = 0; i < count; ++i)
        {
            const NELogging::sScopeInfo & scope = scopes[i];
            QString scopeName{ scope.scopeName };
            root->addChildPriorityRecursive(scopeName, scope.scopePrio);
        }

        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();

        QModelIndex entry = index(pos, 0, mRootIndex);
        emit signalScopesUpdated(idxInstance);
        emit dataChanged(entry, entry, { Qt::ItemDataRole::DecorationRole, Qt::ItemDataRole::DisplayRole });
    }
}

void LoggingScopesModelBase::_setupSignals(bool doSetup)
{
    if (doSetup)
    {
        if (mSignalsSetup)
            return;
        
        mSignalsSetup = true;
        mConSvcConnected = connect(mLoggingModel, &LoggingModelBase::signalLogServiceConnected      , this, [this]() {
            this->slotLogServiceConnected();
        });
        mConSvcDisconnected = connect(mLoggingModel, &LoggingModelBase::signalLogServiceDisconnected, this, [this]() {
            this->slotLogServiceDisconnected();
        });
        mConInstAvailable = connect(mLoggingModel, &LoggingModelBase::signalInstanceAvailable       , this, [this](const std::vector<NEService::sServiceConnectedInstance>& instances) {
            this->slotInstancesAvailable(instances);
        });
        mConInstUnavailable = connect(mLoggingModel, &LoggingModelBase::signalInstanceUnavailable   , this, [this](const std::vector<ITEM_ID>& instIds) {
            this->slotInstancesUnavailable(instIds);
        });
        mConScopesAvailable = connect(mLoggingModel, &LoggingModelBase::signalScopesAvailable       , this, [this](ITEM_ID instId, const std::vector<NELogging::sScopeInfo>& scopes) {
            this->slotScopesAvailable(instId, scopes);
        });
        mConScopesUnavailable = connect(mLoggingModel, &LoggingModelBase::signalScopesUpdated       , this, [this](ITEM_ID instId, const std::vector<NELogging::sScopeInfo>& scopes) {
            this->slotScopesUpdated(instId, scopes);
        });
    }
    else if (mSignalsSetup)
    {
        disconnect(mConSvcConnected);
        disconnect(mConSvcDisconnected);
        disconnect(mConInstAvailable);
        disconnect(mConInstUnavailable);
        disconnect(mConScopesAvailable);
        disconnect(mConScopesUnavailable);
        
        mSignalsSetup = false;
    }
}

void LoggingScopesModelBase::buildScopes(void)
{
    if (mLoggingModel == nullptr)
        return;
    
    beginResetModel();
    const std::vector<NEService::sServiceConnectedInstance>& instances = mLoggingModel->getLogInstances();
    slotInstancesAvailable(instances);
    
    for (const auto& inst : instances)
    {
        const std::vector<NELogging::sScopeInfo> & scopes = mLoggingModel->getLogInstScopes(inst.ciCookie);
        slotScopesAvailable(inst.ciCookie, scopes);
    }
    
    endResetModel();
}

void LoggingScopesModelBase::setupModel(void)
{
    clearModel(false);
    if (mLoggingModel != nullptr)
    {
        mLoggingModel->setupModel();
    }
}

void LoggingScopesModelBase::releaseModel(void)
{
    clearModel(true);
    if (mLoggingModel != nullptr)
    {
        mLoggingModel->releaseModel();
    }
}

void LoggingScopesModelBase::dataTransfer(LoggingScopesModelBase& scopeModel)
{
    beginResetModel();
    _setupSignals(false);
    mLoggingModel = scopeModel.mLoggingModel;
    if (mLoggingModel != nullptr)
    {
        if (scopeModel.mLoggingModel != nullptr)
        {
            mLoggingModel->dataTransfer(*scopeModel.mLoggingModel);
        }

        _setupSignals(true);
        slotLogServiceConnected();
    }
    
    mRootIndex = std::move(scopeModel.mRootIndex);
    scopeModel.mRootIndex = QModelIndex();

    endResetModel();
}

void LoggingScopesModelBase::refresh(void)
{
    beginResetModel();
    endResetModel();
}
