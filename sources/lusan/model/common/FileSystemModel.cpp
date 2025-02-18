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
 *  \file        lusan/model/common/FileSystemModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, File System Model implementation.
 *
 ************************************************************************/

#include "lusan/model/common/FileSystemModel.hpp"

#include <QDir>
#include <QFileInfo>
#include <QIcon>

FileSystemModel::FileSystemModel(QObject * parent /*= nullptr*/)
    : QAbstractItemModel(parent)
    , mRootEntry        ( tr("Workspace") )
    , mWorkspaceDirs    ( )
    , mFileFilter       ( )
{
}
    
FileSystemModel::FileSystemModel(const QMap<QString, QString> & workspaceEntries, const QStringList& extFilters, QObject* parent /*= nullptr*/)
    : QAbstractItemModel(parent)
    , mRootEntry        ( tr("Workspace") )
    , mWorkspaceDirs    (workspaceEntries)
    , mFileFilter       ( extFilters )
{
    for (const QString & dir : mWorkspaceDirs)
    {
        FileSystemEntry* entry = new FileSystemEntry(dir, workspaceEntries[dir], FileSystemEntry::eEntryType::EntryDir, &mRootEntry);
        mRootEntry.addChild(entry);
    }
}

FileSystemModel::~FileSystemModel()
{
}

QModelIndex FileSystemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (hasIndex(row, column, parent) == false)
        return QModelIndex();

    const FileSystemEntry* parentEntry = isValidIndex(parent) == false ? &mRootEntry : static_cast<FileSystemEntry*>(parent.internalPointer());
    const FileSystemEntry* childEntry = parentEntry != nullptr ? parentEntry->getChild(row) : nullptr;
    return (childEntry != nullptr ? createIndex(row, column, childEntry) : QModelIndex());
}

QModelIndex FileSystemModel::parent(const QModelIndex& child) const
{
    if (isValidIndex(child) == false)
        return QModelIndex();

    FileSystemEntry* parentEntry = static_cast<FileSystemEntry*>(child.internalPointer())->getParent();
    return ((parentEntry != nullptr) && (parentEntry != &mRootEntry) ? createIndex(parentEntry->getRow(), 0, parentEntry) : QModelIndex());
}

int FileSystemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;

    const FileSystemEntry* parentEntry = isValidIndex(parent) == false ? &mRootEntry : static_cast<FileSystemEntry*>(parent.internalPointer());
    return (parentEntry != nullptr ? parentEntry->getChildCount() : 0);
}

int FileSystemModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

QVariant FileSystemModel::data(const QModelIndex& index, int role) const
{
    if (isValidIndex(index) == false)
        return QVariant();
    
    FileSystemEntry* entry{ static_cast<FileSystemEntry*>(index.internalPointer()) };
    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
        return entry->getDiplayName();
    
    case Qt::ItemDataRole::DecorationRole:
        return entry->getIcon();
    
    case Qt::ItemDataRole::EditRole:
        return (mRootEntry.getChild(entry->getPath()) == nullptr ? entry->getFileName() : QString());
    
    case Qt::ItemDataRole::UserRole:
        return QVariant::fromValue<FileSystemEntry *>(entry);
        
    default:
        return QVariant();
    }
}

QVariant FileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

void FileSystemModel::fetchMore(const QModelIndex& parent)
{
    if (isValidIndex(parent) == false)
        return;
    
    FileSystemEntry* parentEntry = static_cast<FileSystemEntry*>(parent.internalPointer());
    if (parentEntry->hasValidChildren())
        return;
    
    QFileInfoList list = parentEntry->fetchData(mFileFilter);
    parentEntry->removeDummyEntry();
    for ( const QFileInfo & fi : list)
    {
        parentEntry->addChild(fi, false);
    }
}

bool FileSystemModel::canFetchMore(const QModelIndex& parent) const
{
    if (isValidIndex(parent) == false)
        return false;
    
    FileSystemEntry* parentEntry = static_cast<FileSystemEntry*>(parent.internalPointer());
    return (parentEntry != nullptr) && (parentEntry->hasFetched() == false);
}

QModelIndex FileSystemModel::setRootPaths(const QMap<QString, QString>& paths)
{
    beginResetModel();
    mRootEntry.resetEntry();
    mWorkspaceDirs = paths;
    mRootEntry.setWorkspaceDirectories(paths);
    QModelIndex index = createIndex(0, 0, &mRootEntry);
    endResetModel();
    return index;
}

const QMap<QString, QString>& FileSystemModel::getRootPaths(void) const
{
    return mWorkspaceDirs;
}

QString FileSystemModel::filePath(const QModelIndex& index)
{
    if (isValidIndex(index) == false)
        return "";
    
    FileSystemEntry* entry = static_cast<FileSystemEntry*>(index.internalPointer());
    return (entry != nullptr ? entry->getPath() : QString());
}

void FileSystemModel::refresh(const QModelIndex& index)
{
    if (isValidIndex(index) == false)
        return;
    
    FileSystemEntry* entry = static_cast<FileSystemEntry*>(index.internalPointer());
    if ((entry == nullptr) || (entry->isDir() == false))
        return;
    
    beginResetModel();
    entry->removeAll();
    QFileInfoList list = entry->fetchData(mFileFilter);
    for ( const QFileInfo & fi : list)
    {
        entry->addChild(fi, false);
    }    
    endResetModel();
}

QFileInfo FileSystemModel::getFileInfo(const QModelIndex& index) const
{
    if (isValidIndex(index) == false)
        return QFileInfo();
    
    FileSystemEntry* entry = static_cast<FileSystemEntry*>(index.internalPointer());
    if (entry == nullptr)
        return QFileInfo();
    
    return QFileInfo(entry->getPath());
}

void FileSystemModel::setFileFilter(const QStringList& filterList)
{
    mFileFilter = filterList;
    if (mRootEntry.hasFetched())
    {
        beginResetModel();
        mRootEntry.resetEntry();
        endResetModel();
    }
}

void FileSystemModel::cleanFilters(void)
{
    mFileFilter.clear();
    if (mRootEntry.hasFetched())
    {
        beginResetModel();
        mRootEntry.resetEntry();
        endResetModel();
    }
}
