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
 *  \file        lusan/model/common/WorkspaceModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include "lusan/model/common/WorkspaceModel.hpp"
#include "lusan/model/common/OptionsManager.hpp"
#include <algorithm>

const WorkspaceEntry WorkspaceModel::InvalidWorkspace{};

WorkspaceModel::WorkspaceModel(OptionsManager &options, QObject* parent)
    : QAbstractListModel(parent)
    , mItems()
{
    const std::vector<WorkspaceEntry> & workspaces = options.getWorkspaceList();
    if (workspaces.empty() == false)
    {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        mItems = workspaces;
        endInsertRows();
    }
}

WorkspaceModel::~WorkspaceModel(void)
{
}

void WorkspaceModel::addWorkspaceEntry(const WorkspaceEntry& item)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    
    int index = find(item.getWorkspaceRoot());
    if (index >= 0)
    {
        mItems.at(index) = item;
    }
    else
    {
        mItems.push_back(item);
    }
    
    std::sort(mItems.begin(), mItems.end(), std::greater<WorkspaceEntry>());
    endInsertRows();
}

WorkspaceEntry WorkspaceModel::addWorkspaceEntry(const QString& root, const QString& describe)
{
    WorkspaceEntry result(root, describe);
    
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    
    int index = find(root);
    if (index >= 0)
    {
        mItems.at(index).setWorkspaceDescription(describe);
        mItems.at(index).activate();
        result = mItems.at(index);
    }
    else
    {
        mItems.push_back(result);
    }
    
    std::sort(mItems.begin(), mItems.end(), std::greater<WorkspaceEntry>());
    endInsertRows();
    
    return result;    
}

const WorkspaceEntry & WorkspaceModel::findWorkspaceEntry(const QString& root) const
{
    for (uint32_t i = 0; i < mItems.size(); ++i)
    {
        if (mItems[i].getWorkspaceRoot() == root)
        {
            return mItems[i];
        }
    }
    
    return WorkspaceModel::InvalidWorkspace;
}

int WorkspaceModel::find(const QString& root) const
{
    int result{0};
    for (const auto& entry : mItems)
    {
        if (entry.getWorkspaceRoot() == root)
        {
            break;
        }
        
        ++result;
    }
    
    return (mItems.size() > static_cast<uint32_t>(result) ? result : -1);
}

int WorkspaceModel::find(const uint64_t key) const
{
    int result{0};
    for (const auto& entry : mItems)
    {
        if (entry.getKey() == key)
        {
            break;
        }
        
        ++result;
    }
    
    return (mItems.size() > static_cast<uint32_t>(result) ? result : -1);
}

int WorkspaceModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(mItems.size());
}

QVariant WorkspaceModel::data(const QModelIndex& index, int role) const
{
    QVariant result{};
    
    if (!index.isValid() || index.row() >= rowCount())
        return result;

    const WorkspaceEntry& item = mItems.at(index.row());
    
    switch (role)
    {
    case static_cast<int>(Qt::DisplayRole):
    case static_cast<int>(Qt::EditRole):
        if (index.column() == 0)
        {
            result = item.getWorkspaceRoot();
        }
        else if (index.column() == 1)
        {
            result = item.getWorkspaceDescription();
        }
        break;
    
    case static_cast<int>(Qt::InitialSortOrderRole):
        result = item.getKey();
        break;
        
    default:
        break;
    }
    
    return result;
}

QHash<int, QByteArray> WorkspaceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::EditRole] = "edit";
    roles[Qt::InitialSortOrderRole] = "sort";
    return roles;
}

const WorkspaceEntry& WorkspaceModel::getData(uint32_t row) const
{
    return (row < mItems.size() ? mItems[row] : WorkspaceModel::InvalidWorkspace);
}

uint64_t WorkspaceModel::activate(uint32_t row)
{
    return (row < mItems.size() ? mItems[row].activate() : 0);
}

uint64_t WorkspaceModel::activate(const QString& root)
{
    int index = find(root);
    return (index >= 0 ? activate(static_cast<uint32_t>(index)) : 0);
}
