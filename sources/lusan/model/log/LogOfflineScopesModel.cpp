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
 *  \file        lusan/model/log/LogOfflineScopesModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Offline Log Scopes model.
 *
 ************************************************************************/

 /************************************************************************
  * Includes
  ************************************************************************/

#include "lusan/model/log/LogOfflineScopesModel.hpp"
#include "lusan/model/log/LogScopeIconFactory.hpp"
#include "lusan/model/log/LogOfflineModel.hpp"

#include "lusan/data/log/ScopeNodes.hpp"
#include "areg/base/NECommon.hpp"

#include <QIcon>

LogOfflineScopesModel::LogOfflineScopesModel(QObject* parent)
    : QAbstractItemModel(parent)
    , mRootList ()
    , mRootIndex()
    , mLogModel (nullptr)
{
    mRootIndex = createIndex(0, 0, nullptr);
}

LogOfflineScopesModel::~LogOfflineScopesModel(void)
{
    _clear();
    mLogModel = nullptr;
}

bool LogOfflineScopesModel::setScopeModel(LogOfflineModel* model)
{
    _clear();
    mLogModel = nullptr;

    if ((model == nullptr) || (model->isOperable() == false))
    {
        return false;
    }

    mLogModel = model;
    _buildScopeTree();

    return true;
}

void LogOfflineScopesModel::release(void)
{
    _clear();
    mLogModel = nullptr;
}

QModelIndex LogOfflineScopesModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((hasIndex(row, column, parent) == false) || (column != 0))
        return QModelIndex();

    if ((parent.isValid() == false) || (parent == mRootIndex))
    {
        ScopeRoot* root = (row >= 0) && (row < static_cast<int>(mRootList.size())) ? mRootList[row] : nullptr;
        return (root != nullptr ? createIndex(row, column, root) : mRootIndex);
    }
    else
    {
        ScopeNodeBase* parentNode = static_cast<ScopeNodeBase*>(parent.internalPointer());
        Q_ASSERT(parentNode != nullptr);
        ScopeNodeBase* childNode = parentNode->getChildAt(row);
        return createIndex(row, column, childNode);
    }
}

QModelIndex LogOfflineScopesModel::parent(const QModelIndex& child) const
{
    if ((child.isValid() == false) || (child == mRootIndex))
        return QModelIndex();

    ScopeNodeBase* childNode = static_cast<ScopeNodeBase*>(child.internalPointer());
    Q_ASSERT(childNode != nullptr);
    if (childNode->isRoot())
        return mRootIndex;

    ScopeNodeBase* parentNode = childNode->getParent();
    Q_ASSERT(parentNode != nullptr);
    if (parentNode->isRoot())
    {
        int pos = _findRoot(static_cast<ScopeRoot*>(parentNode)->getRootId());
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

int LogOfflineScopesModel::rowCount(const QModelIndex& parent) const
{
    if ((parent.isValid() == false) || (parent == mRootIndex))
    {
        // Root level - return number of instances
        return mRootList.size();
    }
    else
    {
        // Child level - get from the node
        ScopeNodeBase* node = static_cast<ScopeNodeBase*>(parent.internalPointer());
        Q_ASSERT(node != nullptr);
        return node->getChildCount();
    }
}

int LogOfflineScopesModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1; // Only one column for scope names
}

QVariant LogOfflineScopesModel::data(const QModelIndex& index, int role) const
{
    if (isValidIndex(index) == false)
        return QVariant();

    if (index == mRootIndex)
    {
        return (static_cast<Qt::ItemDataRole>(role) == Qt::ItemDataRole::DisplayRole ? QVariant(tr("Offline Logs")) : QVariant());
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
        return LogScopeIconFactory::getIcon(entry->getPriority());
    }

    case Qt::ItemDataRole::UserRole:
    {
        ScopeNodeBase* entry{ static_cast<ScopeNodeBase*>(index.internalPointer()) };
        return QVariant::fromValue<ScopeNodeBase*>(entry);
    }

    default:
        return QVariant();
    }
}

QVariant LogOfflineScopesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole) && (section == 0))
    {
        return QString("Offline Scopes");
    }

    return QVariant();
}

Qt::ItemFlags LogOfflineScopesModel::flags(const QModelIndex& index) const
{
    if (index.isValid() == false)
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

//////////////////////////////////////////////////////////////////////////
// Private helper methods
//////////////////////////////////////////////////////////////////////////

inline void LogOfflineScopesModel::_clear(void)
{
    beginResetModel();

    for (ScopeRoot* root : mRootList)
    {
        delete root;
    }

    mRootList.clear();
    endResetModel();
}

inline bool LogOfflineScopesModel::_exists(ITEM_ID rootId) const
{
    for (auto root : mRootList)
    {
        Q_ASSERT(root != nullptr);
        if (root->getRootId() == rootId)
            return true;
    }

    return false;
}

inline bool LogOfflineScopesModel::_appendRoot(ScopeRoot* root, bool unique)
{
    if ((unique == false) || (_exists(root->getRootId()) == false))
    {
        mRootList.append(root);
        return true;
    }

    return false;
}

inline int LogOfflineScopesModel::_findRoot(ITEM_ID rootId) const
{
    for (int i = 0; i < mRootList.size(); ++i)
    {
        if (mRootList[i]->getRootId() == rootId)
            return i;
    }

    return static_cast<int>(NECommon::INVALID_INDEX);
}

void LogOfflineScopesModel::_buildScopeTree(void)
{
    if (mLogModel == nullptr)
        return;

    beginResetModel();
    _createRootScopes();
    endResetModel();

    // Emit signal that root is updated
    emit signalRootUpdated(mRootIndex);
}

void LogOfflineScopesModel::_createRootScopes(void)
{
    std::vector<NEService::sServiceConnectedInstance> instances;
    mLogModel->getLogInstanceInfos(instances);
    for (const NEService::sServiceConnectedInstance& instance : instances)
    {
        if (instance.ciSource == NEService::eMessageSource::MessageSourceObserver)
            continue;

        ScopeRoot* root = new ScopeRoot(instance);
        if (_appendRoot(root, true))
        {
            _createScopeNodes(*root, instance.ciCookie);
        }
        else
        {
            delete root;
        }
    }
}

void LogOfflineScopesModel::_createScopeNodes(ScopeRoot& root, ITEM_ID instId)
{
    std::vector<NELogging::sScopeInfo> scopes;
    mLogModel->getLogInstScopes(scopes, instId);
    int pos = scopes.empty() == false ? _findRoot(instId) : static_cast<int>(NECommon::INVALID_INDEX);
    if (pos != static_cast<int>(NECommon::INVALID_INDEX))
    {
        int count = static_cast<int>(scopes.size());
        QModelIndex idxInstance = index(pos, 0, mRootIndex);
        root.resetPrioritiesRecursive(false);

        // Build the scope tree by parsing scope names and creating appropriate nodes
        for (const auto& scope : scopes)
        {
            QString scopePath{ scope.scopeName};
            root.addChildRecursive(scopePath, scope.scopePrio);
        }

        root.resetPrioritiesRecursive(true);
        root.refreshPrioritiesRecursive();
    }
}
