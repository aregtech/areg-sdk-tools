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
    , mRootIndex        ( )
{
}
    
FileSystemModel::FileSystemModel(const QMap<QString, QString> & workspaceEntries, const QStringList& extFilters, QObject* parent /*= nullptr*/)
    : QAbstractItemModel(parent)
    , mRootEntry        ( tr("Workspace") )
    , mWorkspaceDirs    (workspaceEntries)
    , mFileFilter       ( extFilters )
    , mRootIndex        ( )
{
    for (const QString & dir : mWorkspaceDirs)
    {
        FileSystemEntry* entry = new FileSystemEntry(dir, workspaceEntries[dir], FileSystemEntry::eEntryType::EntryDir, &mRootEntry);
        mRootEntry.addChild(entry);
    }

    if (mRootEntry.hasValidChildren())
    {
        mRootIndex = createIndex(0, 0, &mRootEntry);
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
    if (childEntry != nullptr)
    {
        return (childEntry != &mRootEntry ? createIndex(row, column, childEntry) : mRootIndex);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex FileSystemModel::parent(const QModelIndex& child) const
{
    if (isValidIndex(child) == false)
        return QModelIndex();

    FileSystemEntry* parentEntry = static_cast<FileSystemEntry*>(child.internalPointer())->getParent();
    if (parentEntry != nullptr)
    {
        return (parentEntry != &mRootEntry ? createIndex(parentEntry->getRow(), 0, parentEntry) : mRootIndex);
    }
    else
    {
        return QModelIndex();
    }
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

Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = QAbstractItemModel::flags(index);
    QModelIndex idxParent = parent(index);

    if (idxParent.isValid() && (index != mRootIndex) && (idxParent != mRootIndex))
    {
        result |= Qt::ItemIsEditable;
    }

    return result;
}

const QModelIndex& FileSystemModel::setRootPaths(const QMap<QString, QString>& paths)
{
    beginResetModel();
    mRootEntry.resetEntry();
    mWorkspaceDirs = paths;
    mRootEntry.setWorkspaceDirectories(paths);
    if (mRootIndex.isValid() == false)
    {
        mRootIndex = createIndex(0, 0, &mRootEntry);
    }
    endResetModel();
    return mRootIndex;
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

void FileSystemModel::refresh(void)
{
    beginResetModel();
    resetRoot();
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

QModelIndex FileSystemModel::getRootIndex(void) const
{
    return (mRootEntry.hasValidChildren() ? mRootIndex : QModelIndex());
}

bool FileSystemModel::deleteEntry(const QModelIndex & index)
{
    bool result{false};
    
    QModelIndex topIndex = this->parent(index);
    FileSystemEntry* entry = static_cast<FileSystemEntry*>(index.internalPointer());
    FileSystemEntry* parent = static_cast<FileSystemEntry*>(topIndex.internalPointer());
    if ((entry == nullptr) || (parent == nullptr) || topIndex.isValid() || parent->isRoot() || mWorkspaceDirs.contains(entry->getPath()))
        return result;
    
    QFileInfo fi{entry->getPath()};
    if (fi.isDir())
    {
        QDir dir(entry->getPath());
        result = dir.removeRecursively();
    }
    else
    {
        result = fi.dir().remove(entry->getPath());
    }
    
    if (result)
    {
        beginRemoveRows(topIndex, index.row(), index.row());
        parent->removeChild(entry);
        endRemoveRows();
    }
    
    return result;
}

QModelIndex FileSystemModel::insertDirectory(const QString& dirName, const QModelIndex& parentIndex)
{
    if (dirName.isEmpty() || (parentIndex.isValid() == false))
        return QModelIndex();

    FileSystemEntry* parentEntry = static_cast<FileSystemEntry*>(parentIndex.internalPointer());
    if (parentEntry == nullptr)
        return QModelIndex();

    QString path = parentEntry->getPath() + QDir::separator() + dirName;
    QDir dir(path);
    if (dir.exists() == false)
    {
        if (dir.mkpath(path))
        {
            QFileInfo fi(path);
            beginInsertRows(parentIndex, parentEntry->getChildCount(), parentEntry->getChildCount());
            FileSystemEntry* entry = parentEntry->addChild(fi, true);
            endInsertRows();
            return createIndex(parentEntry-getChildIndex(entry), 0, entry);
        }
    }

    return QModelIndex();
}

QModelIndex FileSystemModel::insertFile(const QString& fileName, const QModelIndex& parentIndex)
{
    if (fileName.isEmpty() || !parentIndex.isValid())
        return QModelIndex();

    FileSystemEntry* parentEntry = static_cast<FileSystemEntry*>(parentIndex.internalPointer());
    if (parentEntry == nullptr)
        return QModelIndex();

    QString path = parentEntry->getPath() + QDir::separator() + fileName;
    QFile file(path);
    if (!file.exists())
    {
        if (file.open(QIODevice::WriteOnly))
        {
            file.close();
            QFileInfo fi(path);
            beginInsertRows(parentIndex, parentEntry->getChildCount(), parentEntry->getChildCount());
            FileSystemEntry* entry = parentEntry->addChild(fi, true);
            endInsertRows();
            return createIndex(parentEntry->getChildIndex(entry), 0, entry);
        }
    }

    return QModelIndex();
}

QModelIndex FileSystemModel::renameEntry(const QString& newName, const QModelIndex& index)
{
    if (newName.isEmpty() || !isValidIndex(index))
        return QModelIndex();

    FileSystemEntry* entry = static_cast<FileSystemEntry*>(index.internalPointer());
    if (entry == nullptr || entry->isRoot() || entry->isWorkspaceDir())
        return QModelIndex();

    QString newPath = entry->getParent()->getPath() + QDir::separator() + newName;
    QFileInfo fi(newPath);

    if (fi.exists())
        return QModelIndex();

    QDir dir;
    if (dir.rename(entry->getPath(), newPath))
    {
        entry->setFilePath(newPath);
        entry->setDisplayName(newName);
        emit dataChanged(index, index);
        return index;
    }

    return QModelIndex();
}


void FileSystemModel::resetRoot(void)
{
    resetEntry(&mRootEntry);
}

void FileSystemModel::resetEntry(FileSystemEntry * entry)
{
    entry->resetEntry();
    entry->removeAll();
    QFileInfoList list = entry->fetchData(mFileFilter);
    for ( const QFileInfo & fi : list)
    {
        entry->addChild(fi, false);
    }
}
