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
#include "lusan/model/log/LogScopeIconFactory.hpp"

#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/common/NELusanCommon.hpp"

LoggingScopesModelBase::LoggingScopesModelBase(QObject* parent)
    : QAbstractItemModel( parent )
    , mRootList         ( )
    , mRootIndex        ( )
{
    mRootIndex = createIndex(0, 0, nullptr);
}

LoggingScopesModelBase::~LoggingScopesModelBase(void)
{
    _clear();
}

QModelIndex LoggingScopesModelBase::index(int row, int column, const QModelIndex& parent) const
{
    if ((hasIndex(row, column, parent) == false) || (column != 0))
        return QModelIndex();
    
    if ((parent.isValid() == false) || (parent == mRootIndex))
    {
        return (row < static_cast<int>(mRootList.size()) ? createIndex(row, column, mRootList[row]) : QModelIndex());
    }
    else
    {
        ScopeNodeBase* parentNode = static_cast<ScopeNodeBase*>(parent.internalPointer());
        Q_ASSERT(parentNode != nullptr);
        ScopeNodeBase* childNode = parentNode->getChildAt(row);
        return (childNode != nullptr ? createIndex(row, column, childNode) : QModelIndex());
    }
}

QModelIndex LoggingScopesModelBase::parent(const QModelIndex& child) const
{
    if (child.isValid() == false)
        return mRootIndex;
    
    ScopeNodeBase* childNode = static_cast<ScopeNodeBase*>(child.internalPointer());
    Q_ASSERT(childNode != nullptr);
    ScopeNodeBase* parentNode = childNode->getParent();
    if (parentNode == nullptr)
    {
        return mRootIndex;
    }
    else if (parentNode->getParent() == nullptr)
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

int LoggingScopesModelBase::rowCount(const QModelIndex& parent) const
{
    if ((parent.isValid() == false) || (parent == mRootIndex))
        return static_cast<int>(mRootList.size());
    
    ScopeNodeBase* node = static_cast<ScopeNodeBase*>(parent.internalPointer());
    Q_ASSERT(node != nullptr);
    return node->getChildCount();
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
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    if ((node == nullptr) || (index == mRootIndex))
        return Qt::NoItemFlags;
    else if (node->isLeaf())
        return (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    else
        return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

inline void LoggingScopesModelBase::_clear(void)
{
    for (auto root : mRootList)
    {
        Q_ASSERT(root != nullptr);
        delete root;
    }

    mRootList.clear();
}

inline bool LoggingScopesModelBase::_exists(ITEM_ID rootId) const
{
    for (auto root : mRootList)
    {
        Q_ASSERT(root != nullptr);
        if (root->getRootId() == rootId)
            return true;
    }

    return false;
}

inline bool LoggingScopesModelBase::_appendRoot(ScopeRoot* root, bool unique /*= true*/)
{
    if ((unique == false) || (_exists(root->getRootId()) == false))
    {
        mRootList.append(root);
        return true;
    }

    return false;
}

inline int LoggingScopesModelBase::_findRoot(ITEM_ID rootId) const
{
    for (int i = 0; i < mRootList.size(); ++i)
    {
        if (mRootList[i]->getRootId() == rootId)
            return i;
    }

    return static_cast<int>(NECommon::INVALID_INDEX);
}