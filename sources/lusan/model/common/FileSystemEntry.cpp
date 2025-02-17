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
 *  \file        lusan/model/common/FileSystemEntry.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, File System Model implementation.
 *
 ************************************************************************/

#include "lusan/model/common/FileSystemEntry.hpp"
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QFileIconProvider>

FileSystemEntry FileSystemEntry::EmptyEntry("..", "..", FileSystemEntry::eEntryType::EntryUnknown, QIcon(), nullptr);

FileSystemEntry::FileSystemEntry(const QString& path, FileSystemEntry* parent)
    : mId       (parent != nullptr ? parent->getNextId() : 0)
    , mFilePath (path)
    , mDispName ( )
    , mEntryType(EntryUnknown)
    , mChildren ( )
    , mIcon     ( )
    , mParent   (parent)
{
    QFileInfo fi(path);
    if (fi.exists())
    {
        mDispName   = fi.fileName();
        if (fi.isDir())
        {
            mEntryType  = EntryDir;
            addDummyEntry();
            if (mDispName.isEmpty())
            {
                mDispName   = fi.dir().dirName();
            }
        }
        else if (fi.isSymbolicLink())
        {
            mEntryType  = EntrySymlink;
        }
        else
        {
            mEntryType = EntryFile;
        }
        
        if (isValid())
        {
            QFileIconProvider fip;
            mIcon  = fip.icon(fi);
        }
    }
}

FileSystemEntry::FileSystemEntry(const QString& path, FileSystemEntry::eEntryType entryType, FileSystemEntry* parent)
    : mId       (parent != nullptr ? parent->getNextId() : 0)
    , mFilePath (path)
    , mDispName ( )
    , mEntryType(entryType)
    , mChildren ( )
    , mIcon     ( )
    , mParent   (parent)
{
    QFileInfo fi(path);
    if (fi.exists())
    {
        mDispName = fi.fileName();
        if (mDispName.isEmpty() && fi.isDir())
        {
            mDispName = fi.dir().dirName();
        }
        
        if (isDir())
        {
            addDummyEntry();
        }
        
        if (isValid())
        {
            QFileIconProvider fip;
            mIcon = fip.icon(fi);
        }
    }
}

FileSystemEntry::FileSystemEntry(const QString& path, const QString& dispName, FileSystemEntry::eEntryType entryType, FileSystemEntry* parent)
    : mId       (parent != nullptr ? parent->getNextId() : 0)
    , mFilePath (path)
    , mDispName (dispName)
    , mEntryType(entryType)
    , mChildren ( )
    , mIcon     ( )
    , mParent   (parent)
{
    QFileInfo fi(path);
    if (fi.exists())
    {
        if (dispName.isEmpty())
        {
            mDispName = fi.fileName();
            if (mDispName.isEmpty() && fi.isDir())
                mDispName = fi.dir().dirName();
        }
        
        if (isValid())
        {
            QFileIconProvider fip;
            mIcon = fip.icon(fi);
        }
    }
}

FileSystemEntry::FileSystemEntry(const QString& path, const QString& dispName, FileSystemEntry::eEntryType entryType, const QIcon& icon, FileSystemEntry* parent)
    : mId       (parent != nullptr ? parent->getNextId() : 0)
    , mFilePath (path)
    , mDispName (dispName)
    , mEntryType(entryType)
    , mChildren ( )
    , mIcon     (icon)
    , mParent   (parent)
{
    QFileInfo fi(path);
    if (fi.exists())
    {
        if (dispName.isEmpty())
        {
            mDispName = fi.fileName();
            if (mDispName.isEmpty() && fi.isDir())
                mDispName = fi.dir().dirName();
        }
        
        if (isDir())
        {
            addDummyEntry();
        }
        
        if (icon.isNull() && isValid())
        {
            QFileIconProvider fip;
            mIcon = fip.icon(fi);
        }
    }
}

FileSystemEntry::FileSystemEntry(const QFileInfo & fileInfo, FileSystemEntry* parent)
    : mId       (parent != nullptr ? parent->getNextId() : 0)
    , mFilePath (fileInfo.absoluteFilePath())
    , mDispName ( )
    , mEntryType( )
    , mChildren ( )
    , mIcon     ( )
    , mParent   (parent)
{
    mDispName = fileInfo.fileName();
    if (fileInfo.isDir())
    {
        mEntryType = EntryDir;
        if (mDispName.isEmpty())
            mDispName = fileInfo.dir().dirName();
    }
    else
    {
        mEntryType = fileInfo.isSymbolicLink() ? EntrySymlink : EntryFile;
    }
    
    if (isDir())
    {
        addDummyEntry();
    }
    
    if (isValid())
    {
        QFileIconProvider fip;
        mIcon = fip.icon(fileInfo);    
    }
}    

FileSystemEntry::FileSystemEntry(const FileSystemEntry& src)
    : mId       (src.mId)
    , mChildren (src.mChildren)
    , mFilePath (src.mFilePath)
    , mEntryType(src.mEntryType)
    , mIcon     (src.mIcon)
    , mParent   (nullptr)
{
}

FileSystemEntry::FileSystemEntry(FileSystemEntry&& src) noexcept
    : mId       (src.mId)
    , mChildren (std::move(src.mChildren))
    , mFilePath (std::move(src.mFilePath))
    , mEntryType(src.mEntryType)
    , mIcon     (std::move(src.mIcon))
    , mParent   (src.mParent)
{
    src.mChildren.clear();
    src.mParent = nullptr;
}

FileSystemEntry::~FileSystemEntry()
{
    deleteEntries();
}

FileSystemEntry& FileSystemEntry::operator = (const FileSystemEntry& other)
{
    if (this != &other)
    {
        deleteEntries();

        mId         = other.mId;
        mFilePath   = other.mFilePath;
        mEntryType  = other.mEntryType;
        mIcon       = other.mIcon;
        mParent     = other.mParent;
        mChildren   = other.mChildren;
    }

    return (*this);
}

FileSystemEntry& FileSystemEntry::operator = (FileSystemEntry&& other) noexcept
{
    if (this != &other)
    {
        deleteEntries();

        mId         = other.mId;
        mFilePath   = std::move(other.mFilePath);
        mEntryType  = other.mEntryType;
        mIcon       = std::move(other.mIcon);
        mParent     = other.mParent;
        mChildren   = std::move(other.mChildren);
        other.mParent = nullptr;
    }

    return (*this);
}

bool FileSystemEntry::operator == (const FileSystemEntry& other) const
{
    return ((mId == other.mId) || (mFilePath == other.mFilePath));
}

bool FileSystemEntry::operator != (const FileSystemEntry& other) const
{
    return ((mId != other.mId) && (mFilePath != other.mFilePath));
}

bool FileSystemEntry::operator < (const FileSystemEntry& other) const
{
    if (isDir() && other.isFile())
        return true;
    else if (isFile() && other.isDir())
        return false;
    else
        return (mFilePath < other.mFilePath);
}

bool FileSystemEntry::operator > (const FileSystemEntry& other) const
{
    if (isDir() && other.isFile())
        return false;
    else if (isFile() && other.isDir())
        return true;
    else
        return (mFilePath > other.mFilePath);
}

uint32_t FileSystemEntry::getNextId(void) const
{
    return (mParent != nullptr ? mParent->getNextId() : 0);
}

void FileSystemEntry::deleteEntries(void)
{
    if (mChildren.size() == 1)
    {
        if (*mChildren[0] != EmptyEntry)
            delete mChildren[0];
    }
    else
    {
        for (FileSystemEntry * entry : mChildren)
        {
            delete entry;
        }
    }
    
    mChildren.clear();
}

bool FileSystemEntry::addChild(FileSystemEntry* child, bool sort /*= true*/)
{
    bool result{ false };
    
    if ((child != nullptr) && child->isValid())
    {
        if ((mChildren.size() > 0) && (mChildren[0]->isValid() == false))
        {
            deleteEntries();
        }
        
        int pos = sort ? 0 : mChildren.size();
        for ( ; pos < mChildren.size(); ++ pos)
        {
            Q_ASSERT((*mChildren[pos]) != (*child));
            if (*mChildren[pos] > (*child))
                break;
        }
        
        mChildren.insert(pos, child);
        result = true;
    }
    
    return result;
}

FileSystemEntry* FileSystemEntry::createChildEntry(const QString& path) const
{
    return new FileSystemEntry(path, const_cast<FileSystemEntry *>(this));
}

FileSystemEntry* FileSystemEntry::createChildEntry(const QFileInfo& fileInfo) const
{
    return new FileSystemEntry(fileInfo, const_cast<FileSystemEntry *>(this));
}

bool FileSystemEntry::hasFetched(void) const
{
    return ((mChildren.size() != 1) || mChildren[0]->isValid());
}

QFileInfoList FileSystemEntry::fetchData(void) const
{
    if (isDir())
    {
        QDir dir(mFilePath);
        return std::move(dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden, QDir::Name | QDir::DirsFirst | QDir::IgnoreCase));
    }
    
    return QFileInfoList();    
}

bool FileSystemEntry::hasValidChildren(void) const
{
    if (mChildren.size() == 1)
    {
        const FileSystemEntry* entry = mChildren[0];
        return (*entry != FileSystemEntry::EmptyEntry);
    }
    else
    {
        return true;
    }
}

FileSystemRootEntry::FileSystemRootEntry(const QString& name)
    : FileSystemEntry(name, name, FileSystemEntry::eEntryType::EntryRoot, QIcon(), nullptr)
    , mNextId ( 0 )
    , mWorkspaceDirs( )
{
}

void FileSystemRootEntry::setWorkspaceDirectories(const QMap<QString, QString>& workspaceDirs)
{
    mWorkspaceDirs = workspaceDirs;
    removeAll();
    addDummyEntry();
}

uint32_t FileSystemRootEntry::getNextId(void) const
{
    return (mParent != nullptr ? mParent->getNextId() : ++ mNextId);
}

QFileInfoList FileSystemRootEntry::fetchData(void) const
{
    QFileInfoList result;
    QList<QString> list(mWorkspaceDirs.keys());
    for (const auto & path : list)
    {
        result.push_back(QFileInfo(path));
    }
    
    return std::move(result);
}

FileSystemEntry* FileSystemRootEntry::createChildEntry(const QString& path) const
{
    FileSystemEntry* result = FileSystemEntry::createChildEntry(path);
    if (result != nullptr)
    {
        Q_ASSERT(mWorkspaceDirs.contains(result->getPath()));
        result->mEntryType = FileSystemEntry::eEntryType::EntryWorkspace;
        result->mDispName = mWorkspaceDirs[result->getPath()];
    }
    
    return result;
}

FileSystemEntry* FileSystemRootEntry::createChildEntry(const QFileInfo& fileInfo) const
{
    FileSystemEntry* result = FileSystemEntry::createChildEntry(fileInfo);
    if (result != nullptr)
    {
        Q_ASSERT(mWorkspaceDirs.contains(result->getPath()));
        result->mEntryType = FileSystemEntry::eEntryType::EntryWorkspace;
        result->mDispName = mWorkspaceDirs[result->getPath()];
    }
    
    return result;
}
