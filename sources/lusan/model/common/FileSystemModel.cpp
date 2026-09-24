/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/common/FileSystemModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, File System Model implementation.
 *
 ************************************************************************/

#include "lusan/model/common/FileSystemModel.hpp"
#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/WorkspaceWatcher.hpp"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QIcon>
#include <QSet>

#include <algorithm>

 //////////////////////////////////////////////////////////////////////////
 // FileSystemModel class declaration
 //////////////////////////////////////////////////////////////////////////

FileSystemModel::FileSystemModel(QObject * parent /*= nullptr*/)
    : QAbstractItemModel(parent)
    , mRootEntry        ( tr("Workspace") )
    , mWorkspaceDirs    ( )
    , mFileFilter       ( )
    , mRootIndex        ( )
{
}
    
FileSystemModel::FileSystemModel(const WorkspaceElem & workspaceEntries, const QStringList& extFilters, QObject* parent /*= nullptr*/)
    : QAbstractItemModel(parent)
    , mRootEntry        ( tr("Workspace") )
    , mWorkspaceDirs    (workspaceEntries)
    , mFileFilter       ( extFilters )
    , mRootIndex        ( )
{
    for (WorkspaceElem::const_iterator dir = mWorkspaceDirs.constBegin(); dir != mWorkspaceDirs.constEnd(); ++dir)
    {
        FileSystemEntry* entry = new FileSystemEntry(dir->wsDir, dir->wsDisplay, FileSystemEntry::eEntryType::EntryDir, &mRootEntry);
        mRootEntry.addChild(entry);
    }

    if (mRootEntry.hasValidChildren())
    {
        mRootIndex = createIndex(0, 0, &mRootEntry);
    }
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
    if (parentEntry->hasValidChildren() == false)
    {
        parentEntry->addChildren(mFileFilter);
        emit signalLoadedDirectoriesChanged();
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

const QModelIndex& FileSystemModel::setRootPaths(const WorkspaceElem& paths)
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
    emit signalLoadedDirectoriesChanged();
    return mRootIndex;
}

bool FileSystemModel::updateRootPaths(const WorkspaceElem& paths)
{
    bool result { false };
    if (sameRoots(paths) == false)
    {
        beginResetModel();
        mRootEntry.resetEntry();
        mWorkspaceDirs = paths;
        mRootEntry.updateWorkspaceDirectories(paths);
        if (mRootIndex.isValid() == false)
        {
            mRootIndex = createIndex(0, 0, &mRootEntry);
        }
        
        result = true;
        endResetModel();
        emit signalLoadedDirectoriesChanged();
    }

    return result;
}

const WorkspaceElem& FileSystemModel::getRootPaths() const
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

void FileSystemModel::refresh()
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
        return;
    }

    emit signalLoadedDirectoriesChanged();
}

void FileSystemModel::syncPaths(const QStringList& paths)
{
    if (mRootEntry.hasFetched() == false)
        return;

    QSet<QString> unique;
    for (const QString& path : paths)
    {
        const QString changed{ WorkspaceWatcher::normalizePath(path) };
        if (changed.isEmpty())
            continue;

        unique.insert(changed);
        unique.insert(QFileInfo(changed).path());
    }

    // A directory is synchronized after the directories above it, so an entry removed with its
    // parent is never visited.
    QStringList dirs{ unique.values() };
    std::sort(dirs.begin(), dirs.end(), [](const QString& left, const QString& right) { return (left.size() < right.size()); });

    bool changed{ false };
    QList<FileSystemEntry*> entries;
    for (const QString& dir : dirs)
    {
        entries.clear();
        for (FileSystemEntry* top : mRootEntry.getChildren())
        {
            findLoadedEntries(top, dir, entries);
        }

        for (FileSystemEntry* entry : entries)
        {
            changed = syncEntry(entry) || changed;
        }
    }

    if (changed)
    {
        emit signalLoadedDirectoriesChanged();
    }
}

QStringList FileSystemModel::loadedDirectories() const
{
    QStringList result;
    if (mRootEntry.hasFetched())
    {
        for (const FileSystemEntry* top : mRootEntry.getChildren())
        {
            collectLoadedDirectories(top, result);
        }
    }

    return result;
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
        emit signalLoadedDirectoriesChanged();
    }
}

void FileSystemModel::cleanFilters()
{
    mFileFilter.clear();
    if (mRootEntry.hasFetched())
    {
        beginResetModel();
        mRootEntry.resetEntry();
        endResetModel();
        emit signalLoadedDirectoriesChanged();
    }
}

QModelIndex FileSystemModel::getRootIndex() const
{
    return (mRootEntry.hasValidChildren() ? mRootIndex : QModelIndex());
}

bool FileSystemModel::deleteEntry(const QModelIndex & index)
{
    bool result{false};
    
    QModelIndex topIndex = this->parent(index);
    FileSystemEntry* entry = static_cast<FileSystemEntry*>(index.internalPointer());
    FileSystemEntry* parent = static_cast<FileSystemEntry*>(topIndex.internalPointer());
    if ((entry == nullptr) || (parent == nullptr) || parent->isRoot() || containsRoot(entry->getPath()))
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
        emit signalLoadedDirectoriesChanged();
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
                entry = parentEntry->addChild(fi, true);
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
        emit signalLoadedDirectoriesChanged();
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

bool FileSystemModel::checkWorkspaceEntry(const QModelIndex& index) const
{
    const FileSystemEntry* entry = static_cast<const FileSystemEntry*>(index.constInternalPointer());
    const QString path      { entry != nullptr ? entry->getPath() : "" };
    const QString root      { NELusanCommon::fixPath(LusanApplication::getWorkspaceRoot())      };
    const QString sources   { NELusanCommon::fixPath(LusanApplication::getWorkspaceSources())   };
    const QString includes  { NELusanCommon::fixPath(LusanApplication::getWorkspaceIncludes())  };
    const QString delivery  { NELusanCommon::fixPath(LusanApplication::getWorkspaceDelivery())  };
    const QString logs      { NELusanCommon::fixPath(LusanApplication::getWorkspaceLogs())      };

    if (path.compare(root, Qt::CaseSensitivity::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (path.compare(sources, Qt::CaseSensitivity::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (path.compare(includes, Qt::CaseSensitivity::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (path.compare(delivery, Qt::CaseSensitivity::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (path.compare(logs, Qt::CaseSensitivity::CaseInsensitive) == 0)
    {
        return true;
    }

    return false;
}

void FileSystemModel::resetRoot()
{
    resetEntry(&mRootEntry);
}

inline bool FileSystemModel::sameRoots(const WorkspaceElem & elems) const
{
    if (elems.isEmpty() || (elems.size() != mWorkspaceDirs.size()))
    {
        return (elems.isEmpty() && mWorkspaceDirs.isEmpty());
    }
    
    for (WorkspaceElem::const_iterator dir = mWorkspaceDirs.constBegin(); dir != mWorkspaceDirs.constEnd(); ++dir)
    {
        if (elems.contains(dir.key()) == false)
            return false;
        
        const sWorkspaceElem & elem = elems[dir.key()];
        if (dir->wsDir != elem.wsDir)
            return false;
    }
    
    return true;
}

inline bool FileSystemModel::containsRoot(const QString & rootPath) const
{
    QString path{rootPath.isEmpty() == false ? NELusanCommon::fixPath(rootPath) : QString()};
    for (WorkspaceElem::const_iterator dir = mWorkspaceDirs.constBegin(); dir != mWorkspaceDirs.constEnd(); ++dir)
    {
        if (dir->wsDir == path)
            return true;
    }
    
    return false;
}

void FileSystemModel::resetEntry(FileSystemEntry * entry)
{
    entry->resetEntry();
    entry->removeAll();
    QFileInfoList list = entry->fetchData(mFileFilter);
    for ( QFileInfoList::const_iterator fi = list.constBegin(); fi != list.constEnd(); ++fi)
    {
        entry->addChild(*fi, false);
    }
}

bool FileSystemModel::syncEntry(FileSystemEntry* entry)
{
    const QFileInfoList list{ entry->fetchData(mFileFilter) };
    QHash<QString, bool> added;
    added.reserve(list.size());
    for (const QFileInfo& fi : list)
    {
        added.insert(fi.fileName(), fi.isDir());
    }

    bool changed{ false };
    const QModelIndex parentIndex{ entryIndex(entry) };
    for (int row = entry->getChildCount() - 1; row >= 0; --row)
    {
        FileSystemEntry* child{ entry->getChild(row) };
        QHash<QString, bool>::iterator pos{ added.find(child->getFileName()) };
        if ((pos != added.end()) && (pos.value() == child->isDir()))
        {
            added.erase(pos);
            continue;
        }

        beginRemoveRows(parentIndex, row, row);
        entry->removeChild(row);
        endRemoveRows();
        if (child->isValid())
        {
            delete child;
        }

        changed = true;
    }

    for (const QFileInfo& fi : list)
    {
        if (added.contains(fi.fileName()) == false)
            continue;

        FileSystemEntry* child{ entry->createChildEntry(fi) };
        if ((child == nullptr) || (child->isValid() == false))
        {
            delete child;
            continue;
        }

        const int row{ entry->insertPosition(*child) };
        beginInsertRows(parentIndex, row, row);
        entry->addChild(child, true);
        endInsertRows();
        changed = true;
    }

    return changed;
}

void FileSystemModel::findLoadedEntries(FileSystemEntry* entry, const QString& dirPath, QList<FileSystemEntry*>& result) const
{
    if ((entry == nullptr) || (entry->isValid() == false) || (entry->isDir() == false) || (entry->hasFetched() == false))
        return;

    const QString path{ QDir::cleanPath(entry->getPath()) };
    if (path.compare(dirPath, Qt::CaseSensitivity::CaseInsensitive) == 0)
    {
        result.append(entry);
    }
    else if (WorkspaceWatcher::isSameOrUnder(dirPath, path))
    {
        for (FileSystemEntry* child : entry->getChildren())
        {
            findLoadedEntries(child, dirPath, result);
        }
    }
}

void FileSystemModel::collectLoadedDirectories(const FileSystemEntry* entry, QStringList& result) const
{
    if ((entry == nullptr) || (entry->isValid() == false) || (entry->isDir() == false) || (entry->hasFetched() == false))
        return;

    result.append(entry->getPath());
    for (const FileSystemEntry* child : entry->getChildren())
    {
        collectLoadedDirectories(child, result);
    }
}

inline QModelIndex FileSystemModel::entryIndex(FileSystemEntry* entry) const
{
    return (entry == &mRootEntry ? mRootIndex : createIndex(entry->getRow(), 0, entry));
}

//////////////////////////////////////////////////////////////////////////
// GeneralFileSystemModel class implementation
//////////////////////////////////////////////////////////////////////////

GeneralFileSystemModel::GeneralFileSystemModel(QObject* parent /*= nullptr*/)
    : QFileSystemModel(parent)
{
    setFilter(QDir::Filter::AllEntries | QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot);
}

int GeneralFileSystemModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

QVariant GeneralFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}
