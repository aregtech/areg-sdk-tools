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
#include "lusan/app/LusanApplication.hpp"

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
        return entry->getDisplayName();
    
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
    refresh(&mRootEntry);
}

void FileSystemModel::refresh(const QModelIndex & index)
{
    FileSystemEntry* entry = static_cast<FileSystemEntry*>(index.internalPointer());
    refresh(entry);
}

void FileSystemModel::refresh(FileSystemEntry* entry)
{
    if (entry == nullptr)
        return;
    
    if (entry == &mRootEntry)
    {
        beginResetModel();
        resetRoot();
        endResetModel();
    }
    else if (entry->isDir())
    {
        resetEntry(entry);
        int childCount = entry->getChildCount();
        beginInsertRows(createIndex(entry->getRow(), 0, entry), 0, childCount == 0 ? 0 : childCount - 1);
        endInsertRows();
    }
    else
    {
        FileSystemEntry* parent = entry->getParent();
        refresh(parent);
    }
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
    if ((entry == nullptr) || (parent == nullptr) || parent->isRoot() || mWorkspaceDirs.contains(entry->getPath()))
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
    if ((parentEntry == nullptr) || (parentEntry->isRoot()))
        return QModelIndex();
    
    QModelIndex result;
    QString path = parentEntry->getPath() + QDir::separator() + dirName;
    QDir dir(path);
    if (dir.exists() == false)
    {
        if (dir.mkpath(path))
        {
            FileSystemEntry* entry{nullptr};
            QFileInfo fi(path);
            if (parentEntry->hasFetched())
            {
                beginInsertRows(parentIndex, 0, parentEntry->getChildCount() + 1);
                entry = parentEntry->addChild(fi, true);
                endInsertRows();
            }
            else
            {
                refresh(parentEntry);
                entry = parentEntry->getChild(fi.filePath());
                Q_ASSERT(entry != nullptr);
            }
            
            result = createIndex(entry->getRow(), 0, entry);
        }
    }

    return result;
}

QModelIndex FileSystemModel::insertFile(const QString& fileName, const QModelIndex& parentIndex)
{
    if (fileName.isEmpty() || !parentIndex.isValid())
        return QModelIndex();

    FileSystemEntry* parentEntry = static_cast<FileSystemEntry*>(parentIndex.internalPointer());
    if ((parentEntry == nullptr) || (parentEntry->isRoot()))
        return QModelIndex();

    QString newPath = parentEntry->getPath() + QDir::separator() + fileName;
    QFile file(newPath);
    if (!file.exists())
    {
        if (file.open(QIODevice::WriteOnly))
        {
            file.close();
            QFileInfo fi(newPath);
            FileSystemEntry* entry {nullptr};
            if (parentEntry->hasFetched())
            {
                beginInsertRows(parentIndex, 0, parentEntry->getChildCount());
                FileSystemEntry* entry = parentEntry->addChild(fi, true);
                endInsertRows();
            }
            else
            {
                refresh(parentEntry);
                entry = parentEntry->getChild(fi.filePath());
                Q_ASSERT(entry != nullptr);
            }
            
            return createIndex(entry->getRow(), 0, entry);
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
    
    FileSystemEntry* parent = entry->getParent();
    Q_ASSERT(parent != nullptr);

    QString newPath = parent->getPath() + QDir::separator() + newName;
    QFileInfo fi(newPath);

    if (fi.exists())
        return QModelIndex();

    QDir dir;
    if (dir.rename(entry->getPath(), fi.filePath()))
    {
        beginInsertRows(index.parent(), 0, parent->getChildCount());
        entry->setFilePath(fi.filePath());
        parent->refreshChildren(mFileFilter);
        entry = parent->getChild(fi.filePath());
        Q_ASSERT(entry != nullptr);
        int pos = entry->getRow();
        endInsertRows();
        return createIndex(pos, 0, entry);
    }

    return QModelIndex();
}

bool FileSystemModel::containsChildEntry(const QModelIndex & parentIndex, const QString& fileName) const
{
    const FileSystemEntry * entry = static_cast<const FileSystemEntry *>(parentIndex.constInternalPointer());
    return (entry != nullptr ? entry->containsEntryName(fileName) : false);
}

bool FileSystemModel::existsDirectory(const QString & dirPath) const
{
    QFileInfo fi(dirPath);
    return fi.exists();
}

bool FileSystemModel::existsDirectory(const QModelIndex & parentIndex, const QString & subdirName) const
{
    const FileSystemEntry * entry = static_cast<const FileSystemEntry *>(parentIndex.constInternalPointer());
    return (entry != nullptr) && entry->isValid() && existsDirectory(entry->getPath() + QDir::separator() + subdirName) ;
}

bool FileSystemModel::existsFile(const QString& filePath) const
{
    QFileInfo fi(filePath);
    return fi.exists();
}

bool FileSystemModel::existsFile(const QModelIndex & parentIndex, const QString& fileName) const
{
    const FileSystemEntry * entry = static_cast<const FileSystemEntry *>(parentIndex.constInternalPointer());
    return (entry != nullptr) && entry->isValid() && existsFile(entry->getPath() + QDir::separator() + fileName) ;
}

bool FileSystemModel::isFile(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    return ((entry != nullptr) && entry->isFile());
}

bool FileSystemModel::isDir(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    return ((entry != nullptr) && entry->isDir());
}

bool FileSystemModel::isWorkspaceEntry(const QModelIndex& index) const
{
    return (mRootIndex == index) || (index.isValid() && (parent(index) == mRootIndex));
}

bool FileSystemModel::isWorkspaceProject(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    const QString dir{ LusanApplication::getWorkspaceRoot() };
    const QString path{ entry != nullptr ? entry->getPath() : "" };
    return (path.isEmpty() == false) && (path.compare(dir, Qt::CaseSensitivity::CaseInsensitive) == 0);
}

bool FileSystemModel::isWorkspaceProjectSubdirEntry(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    const QString dir{ LusanApplication::getWorkspaceRoot() };
    const QString path{ entry != nullptr ? entry->getPath() : "" };
    return (path.isEmpty() == false) && (dir.isEmpty() == false) && path.startsWith(dir, Qt::CaseSensitivity::CaseInsensitive);
}

bool FileSystemModel::isWorkspaceSource(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    const QString dir{ LusanApplication::getWorkspaceSources() };
    const QString path{ entry != nullptr ? entry->getPath() : "" };
    return (path.isEmpty() == false) && (path.compare(dir, Qt::CaseSensitivity::CaseInsensitive) == 0);
}

bool FileSystemModel::isWorkspaceSourceSubdirEntry(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    const QString dir{ LusanApplication::getWorkspaceSources() };
    const QString path{ entry != nullptr ? entry->getPath() : "" };
    return (path.isEmpty() == false) && (dir.isEmpty() == false) && path.startsWith(dir, Qt::CaseSensitivity::CaseInsensitive);
}

bool FileSystemModel::isWorkspaceDelivery(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    const QString dir{ LusanApplication::getWorkspaceDelivery() };
    const QString path{ entry != nullptr ? entry->getPath() : "" };
    return (path.isEmpty() == false) && (path.compare(dir, Qt::CaseSensitivity::CaseInsensitive) == 0);
}

bool FileSystemModel::isWorkspaceDeliverySubdirEntry(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    const QString dir{ LusanApplication::getWorkspaceDelivery() };
    const QString path{ entry != nullptr ? entry->getPath() : "" };
    return (path.isEmpty() == false) && (dir.isEmpty() == false) && path.startsWith(dir, Qt::CaseSensitivity::CaseInsensitive);
}

bool FileSystemModel::isWorkspaceInclude(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    const QString dir{ LusanApplication::getWorkspaceIncludes() };
    const QString path{ entry != nullptr ? entry->getPath() : "" };
    return (path.isEmpty() == false) && (path.compare(dir, Qt::CaseSensitivity::CaseInsensitive) == 0);
}

bool FileSystemModel::isWorkspaceIncludeSubdirEntry(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    const QString dir{ LusanApplication::getWorkspaceIncludes() };
    const QString path{ entry != nullptr ? entry->getPath() : "" };
    return (path.isEmpty() == false) && (dir.isEmpty() == false) && path.startsWith(dir, Qt::CaseSensitivity::CaseInsensitive);
}

bool FileSystemModel::isLusanFile(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    if ((entry == nullptr) || entry->isDir())
        return false;

    for (const QString& ext : LusanApplication::InternalExts)
    {
        if (entry->getPath().endsWith(ext, Qt::CaseSensitivity::CaseInsensitive))
            return true;
    }

    return false;
}

bool FileSystemModel::isRoot(const QModelIndex & index) const
{
    return (index.isValid() && (index == mRootIndex));
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
