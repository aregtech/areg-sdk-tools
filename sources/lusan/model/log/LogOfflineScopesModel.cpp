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

#include <QIcon>

LogOfflineScopesModel::LogOfflineScopesModel(QObject* parent)
    : QAbstractItemModel( parent )
    , mRootList         ( )
    , mRootIndex        ( )
    , mOfflineModel     ( nullptr )
{
    mRootIndex = createIndex(0, 0, nullptr);
}

LogOfflineScopesModel::~LogOfflineScopesModel(void)
{
    _clear();
}

bool LogOfflineScopesModel::initialize(LogOfflineModel* offlineModel)
{
    _clear();
    
    if (offlineModel == nullptr || !offlineModel->isDatabaseOpen())
        return false;

    mOfflineModel = offlineModel;
    _buildScopeTree(offlineModel);
    
    return true;
}

void LogOfflineScopesModel::release(void)
{
    _clear();
    mOfflineModel = nullptr;
}

QModelIndex LogOfflineScopesModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((row < 0) || (column < 0) || (column > 0))
        return QModelIndex();

    if (parent.isValid() == false)
    {
        // Root level - return instances
        return ((row >= 0) && (row < mRootList.size())) ? createIndex(row, column, mRootList[row]) : QModelIndex();
    }
    else
    {
        // Child level - get from the node
        ScopeNodeBase* node = static_cast<ScopeNodeBase*>(parent.internalPointer());
        if (node != nullptr)
        {
            ScopeNodeBase* child = node->getChildAt(row);
            return (child != nullptr) ? createIndex(row, column, child) : QModelIndex();
        }
    }

    return QModelIndex();
}

QModelIndex LogOfflineScopesModel::parent(const QModelIndex& child) const
{
    if (child.isValid() == false)
        return QModelIndex();

    ScopeNodeBase* node = static_cast<ScopeNodeBase*>(child.internalPointer());
    if (node == nullptr)
        return QModelIndex();

    ScopeNodeBase* parentNode = node->getParent();
    if (parentNode == nullptr)
    {
        // This is a root node, return invalid index
        return QModelIndex();
    }

    // Find the parent's row in its parent
    ScopeNodeBase* grandParent = parentNode->getParent();
    if (grandParent == nullptr)
    {
        // Parent is a root node, find its index in mRootList
        for (int i = 0; i < mRootList.size(); ++i)
        {
            if (mRootList[i] == parentNode)
                return createIndex(i, 0, parentNode);
        }
    }
    else
    {
        // Parent is a regular node, find its index in grandparent's children
        int row = grandParent->getChildPosition(parentNode->getNodeName());
        if (row >= 0)
            return createIndex(row, 0, parentNode);
    }

    return QModelIndex();
}

int LogOfflineScopesModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() == false)
    {
        // Root level - return number of instances
        return mRootList.size();
    }
    else
    {
        // Child level - get from the node
        ScopeNodeBase* node = static_cast<ScopeNodeBase*>(parent.internalPointer());
        return (node != nullptr) ? node->getChildCount() : 0;
    }
}

int LogOfflineScopesModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1; // Only one column for scope names
}

QVariant LogOfflineScopesModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() == false)
        return QVariant();

    ScopeNodeBase* node = static_cast<ScopeNodeBase*>(index.internalPointer());
    if (node == nullptr)
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
        return node->getDisplayName();

    case Qt::DecorationRole:
        return LogScopeIconFactory::getScopeIcon(node);

    case Qt::ToolTipRole:
        return node->makePath();

    default:
        break;
    }

    return QVariant();
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
        return Qt::NoItemFlags;

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

void LogOfflineScopesModel::_buildScopeTree(LogOfflineModel* offlineModel)
{
    if (offlineModel == nullptr)
        return;

    beginResetModel();

    // Get list of instances from the offline model
    std::vector<NEService::sServiceConnectedInstance> instances;
    offlineModel->getLogInstanceInfos(instances);

    // Create root nodes for each instance
    for (const auto& instance : instances)
    {
        ScopeRoot* root = new ScopeRoot(instance);
        if (root != nullptr)
        {
            // Get scopes for this instance
            std::vector<NELogging::sScopeInfo> scopes;
            offlineModel->getLogInstScopes(scopes, instance.ciCookie);
            
            // Create scope nodes
            _createScopeNodes(root, scopes);
            
            // Add root to the model
            _appendRoot(root, true);
        }
    }

    endResetModel();
    
    // Emit signal that root is updated
    emit signalRootUpdated(mRootIndex);
}

void LogOfflineScopesModel::_createScopeNodes(ScopeRoot* root, const std::vector<NELogging::sScopeInfo>& scopes)
{
    if (root == nullptr)
        return;

    // Build the scope tree by parsing scope names and creating appropriate nodes
    for (const auto& scopeInfo : scopes)
    {
        QString scopeName = QString::fromStdString(scopeInfo.siScopeName.getData());
        QStringList scopeParts = scopeName.split("::", Qt::SkipEmptyParts);
        
        if (scopeParts.isEmpty())
            continue;
            
        // Create the child node hierarchy using the makeChildNode method
        QStringList pathCopy = scopeParts;
        ScopeNodeBase* childNode = root->makeChildNode(pathCopy, scopeInfo.siPriority);
        if (childNode != nullptr)
        {
            root->addChildNode(childNode);
        }
    }
}