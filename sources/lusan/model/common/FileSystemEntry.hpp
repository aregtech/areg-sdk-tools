#ifndef LUSAN_MODEL_COMMON_FILESYSTEMENTRY_HPP
#define LUSAN_MODEL_COMMON_FILESYSTEMENTRY_HPP
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
 *  \file        lusan/model/common/FileSystemEntry.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, File System Model for QTreeView.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QList>
#include <QString>

//////////////////////////////////////////////////////////////////////////
// FileSystemEntry class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   FileSystemEntry
 * \brief   Represents an entry in the file system, which can be a file, directory, root, or symlink.
 **/
class FileSystemEntry
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
    friend class FileSystemRootEntry;
    using ChildEntries = QList<FileSystemEntry *>;

public:
    /**
     * \enum    eEntryType
     * \brief   Defines the type of the file system entry.
     **/
    enum eEntryType : uint16_t
    {
          EntryUnknown      = 0     //!< bits: 0000 0000, Unknown entry type.
        , EntryFile         = 1     //!< bits: 0000 0001, File entry type.
        , EntryDir          = 2     //!< bits: 0000 0010, Directory entry type.
        , EntrySymlink      = 17    //!< bits: 0001 0001, Symlink entry type.
        , EntryRoot         = 34    //!< bits: 0010 0010, Root entry type.
        , EntryWorkspace    = 66    //!< bits: 0100 0010, Workspace entry type.
    };
    
    static FileSystemEntry EmptyEntry;

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor with initialization.
     * \param   path    The path of the file system entry.
     * \param   parent  The parent entry.
     **/
    FileSystemEntry(const QString& path, FileSystemEntry* parent);
    
    /**
     * \brief   Constructor with initialization.
     * \param   path        The path of the file system entry.
     * \param   entryType   The type of the file system entry.
     * \param   parent      The parent entry.
     **/
    FileSystemEntry(const QString& path, FileSystemEntry::eEntryType entryType, FileSystemEntry* parent);
    
    /**
     * \brief   Constructor with initialization.
     * \param   path        The path of the file system entry.
     * \param   dispName    The display name of the file system entry.
     * \param   entryType   The type of the file system entry.
     * \param   parent      The parent entry.
     **/
    FileSystemEntry(const QString& path, const QString& dispName, FileSystemEntry::eEntryType entryType, FileSystemEntry* parent);

    /**
     * \brief   Constructor with initialization.
     * \param   path        The path of the file system entry.
     * \param   dispName    The display name of the file system entry.
     * \param   entryType   The type of the file system entry.
     * \param   icon        The icon of the file system entry.
     * \param   parent      The parent entry.
     **/
    FileSystemEntry(const QString& path, const QString& dispName, FileSystemEntry::eEntryType entryType, const QIcon& icon, FileSystemEntry* parent);
    
    /**
     * \brief   Constructor with initialization.
     * \param   fileInfo    The file information of the file system entry.
     * \param   parent      The parent entry.
     **/
    FileSystemEntry(const QFileInfo & fileInfo, FileSystemEntry* parent);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    FileSystemEntry(const FileSystemEntry& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    FileSystemEntry(FileSystemEntry&& src) noexcept;

    /**
     * \brief   Destructor.
     **/
    virtual ~FileSystemEntry();

//////////////////////////////////////////////////////////////////////////
// operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    FileSystemEntry& operator = (const FileSystemEntry& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    FileSystemEntry& operator = (FileSystemEntry&& other) noexcept;

    /**
     * \brief   Equality operator.
     * \param   other   The other object to compare with.
     * \return  True if equal, false otherwise.
     **/
    bool operator == (const FileSystemEntry& other) const;

    /**
     * \brief   Inequality operator.
     * \param   other   The other object to compare with.
     * \return  True if not equal, false otherwise.
     **/
    bool operator != (const FileSystemEntry& other) const;

    /**
     * \brief   Less than operator.
     * \param   other   The other object to compare with.
     * \return  True if less than, false otherwise.
     **/
    bool operator < (const FileSystemEntry& other) const;

    /**
     * \brief   Greater than operator.
     * \param   other   The other object to compare with.
     * \return  True if greater than, false otherwise.
     **/
    bool operator > (const FileSystemEntry& other) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Create and returns the child entry. The entry will not be added to the children list.
     * \param   path    The path of the file system entry.
     **/
    virtual FileSystemEntry* createChildEntry(const QString& path) const;

    /**
     * \brief   Create and returns the child entry. The entry will not be added to the children list.
     * \param   fileInfo    The file information of the file system entry.
     **/
    virtual FileSystemEntry* createChildEntry(const QFileInfo& fileInfo) const;

    /**
     * \brief   Checks if the file system entry has fetched data.
     * \return  True if the file system entry has fetched data, false otherwise.
     **/
    virtual bool hasFetched(void) const;
    
    /**
     * \brief   Fetches data for the file system entry.
     * \param   filter  The list of file extension filters.
     * \return  The list of file information.
     **/
    virtual QFileInfoList fetchData(const QStringList & filter = QStringList()) const;
    
    /**
     * \brief   Checks if the file system entry has valid children.
     * \return  True if the file system entry has valid children, false otherwise.
     **/
    virtual bool hasValidChildren(void) const;

    /**
     * \brief   Refreshes the children of the file system entry.
     * \param   filter  The list of file extension filters.
     * \return  The number of refreshed children.
     **/
    virtual int refreshChildren(const QStringList& filter = QStringList());

    /**
     * \brief   Sorts the child entries. The sorting is done in ascending or descending order.
     *          The first entries are directories, followed by files.
     * \param   ascending   Flag, indicating whether the sorting is done ascending or descending.
     **/
    virtual void sort(bool ascending);
    
//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the list of child elements.
     **/
    inline const QList<FileSystemEntry *>& getChildren(void) const;

    /**
     * \brief   Returns the child element at the given index.
     * \param   index   The index of the child element.
     **/
    inline FileSystemEntry* getChild(int index);
    inline const FileSystemEntry* getChild(int index) const;

    /**
     * \brief   Returns the child element with the given path.
     * \param   path    The path of the child element.
     **/
    inline FileSystemEntry* getChild(const QString& path);
    inline const FileSystemEntry* getChild(const QString& path) const;

    /**
     * \brief   Returns the child element with the given ID.
     * \param   id  The ID of the child element.
     **/
    inline FileSystemEntry* getChild(uint32_t id);
    inline const FileSystemEntry* getChild(uint32_t id) const;

    /**
     * \brief   Returns the number of child elements.
     **/
    inline int getChildCount(void) const;

    /**
     * \brief   Returns the path of the file system entry.
     **/
    inline const QString& getPath(void) const;
    
    /**
     * \brief   Returns the file name of the file system entry.
     **/
    inline QString getFileName(void) const;
    
    /**
     * \brief   Returns the display name of the file system entry.
     **/
    inline const QString& getDisplayName(void) const;
    
    /**
     * \brief   Returns the icon of the file system entry.
     **/
    inline const QIcon& getIcon(void) const;

    /**
     * \brief   Returns the row of the file system entry in the parent's child list.
     **/
    inline int getRow(void) const;
    
    /**
     * \brief   Returns the parent of the file system entry.
     **/
    inline FileSystemEntry* getParent(void);

    /**
     * \brief   Returns the parent of the file system entry.
     **/
    inline const FileSystemEntry* getParent(void) const;

    /**
     * \brief   Sets the type of the file system entry.
     * \param   entryType   The type of the file system entry.
     **/
    inline void setEntryType(FileSystemEntry::eEntryType entryType);

    /**
     * \brief   Checks if the file system entry is a directory.
     * \return  True if the file system entry is a directory, false otherwise.
     **/
    inline bool isDir(void) const;

    /**
     * \brief   Checks if the file system entry is a file.
     * \return  True if the file system entry is a file, false otherwise.
     **/
    inline bool isFile(void) const;

    /**
     * \brief   Checks if the file system entry is a symbolic link.
     * \return  True if the file system entry is a symbolic link, false otherwise.
     **/
    inline bool isSymlink(void) const;

    /**
     * \brief   Checks if the file system entry is a root.
     * \return  True if the file system entry is a root, false otherwise.
     **/
    inline bool isRoot(void) const;

    /**
     * \brief   Checks if the file system entry is a workspace directory.
     * \return  True if the file system entry is a workspace directory, false otherwise.
     **/
    inline bool isWorkspaceDir(void) const;

    /**
     * \brief   Checks if the file system entry is valid.
     * \return  True if the file system entry is valid, false otherwise.
     **/
    inline bool isValid(void) const;
    
    /**
     * \brief   Returns the ID of the file system entry.
     **/
    inline uint32_t getId(void) const;

    /**
     * \brief   Adds a child entry to the file system entry.
     * \param   child   The child entry to add.
     * \param   sort    If true, sorts the child entries after adding.
     * \return  True if the child entry was added, false otherwise.
     **/
    bool addChild(FileSystemEntry * child, bool sort = true);
    
    /**
     * \brief   Adds a child entry to the file system entry.
     * \param   path        The path of the child entry.
     * \param   entryType   The type of the child entry.
     * \param   sort        If true, sorts the child entries after adding.
     * \return  The added child entry.
     **/
    inline FileSystemEntry* addChild(const QString& path, FileSystemEntry::eEntryType entryType, bool sort = true);

    /**
     * \brief   Adds a child entry to the file system entry.
     * \param   path        The path of the child entry.
     * \param   entryType   The type of the child entry.
     * \param   icon        The icon of the child entry.
     * \param   sort        If true, sorts the child entries after adding.
     * \return  The added child entry.
     **/
    inline FileSystemEntry* addChild(const QString& path, FileSystemEntry::eEntryType entryType, const QIcon& icon, bool sort = true);
    
    /**
     * \brief   Adds a child entry to the file system entry.
     * \param   fi      The file information of the child entry.
     * \param   sort    If true, sorts the child entries after adding.
     * \return  The added child entry.
     **/
    inline FileSystemEntry* addChild(const QFileInfo &fi, bool sort = true);
    
    /**
     * \brief   Adds a dummy entry to the file system entry.
     * \return  True if the dummy entry was added, false otherwise.
     *          The dummy entry cannot be added if the entry is either not a directory
     *          or it container list of child elements.
     **/
    inline bool addDummyEntry(void);

    /**
     * \brief   Removes the dummy element from the list.
     **/
    inline void removeDummyEntry(void);
    
    /**
     * \brief   Removes a child entry from the file system entry.
     * \param   entry   The child entry to remove.
     **/
    inline void removeChild(FileSystemEntry* entry);

    /**
     * \brief   Removes a child entry from the file system entry.
     * \param   row     The row of the child entry to remove.
     **/
    inline void removeChild(int row);

    /**
     * \brief   Removes a child entry from the file system entry.
     * \param   path    The path of the child entry to remove.
     **/
    inline void removeChild(const QString& path);

    /**
     * \brief   Removes a child entry from the file system entry.
     * \param   id  The ID of the child entry to remove.
     **/
    inline void removeChild(uint32_t id);

    /**
     * \brief   Removes all child entries from the file system entry.
     **/
    inline void removeAll(void);

    /**
     * \brief   Resets the file system entry.
     **/
    inline void resetEntry(void);

    /**
     * \brief   Sets the file path, updates the display name, if it is not the root entry
     *          of workspace entry (child or root).
     * \param   newPath     The new path to set.
     */
    void setFilePath(const QString& newPath);

    
    /**
     * \brief   Sets the file path of a child entry.
     * \param   oldPath     The old path of the child entry.
     * \param   newPath     The new path of the child entry.
     **/
    void setChildFilePath(const QString& oldPath, const QString& newPath);

    
    /**
     * \brief   Checks if the file system entry contains a child with the given file name.
     * \param   fileName    The name of the file to check.
     * \return  True if the file system entry contains a child with the given file name, false otherwise.
     **/
    bool containsEntryName(const QString& fileName) const;

//////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Returns the next ID for the file system entry.
     * \return  The next ID for the file system entry.
     **/
    virtual uint32_t getNextId(void) const;
    
    /**
     * \brief   Deletes all child entries.
     **/
    void deleteEntries(void);
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    uint32_t            mId;        //!< The ID of the file system entry.
    QString             mFilePath;  //!< The path of the file system entry.
    QString             mDispName;  //!< The display name of the file system entry.
    eEntryType          mEntryType; //!< The type of the file system entry.
    ChildEntries        mChildren;  //!< The list of child entries.
    QIcon               mIcon;      //!< The icon of the file system entry.
    FileSystemEntry*    mParent;    //!< The parent entry.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    FileSystemEntry(void) = delete;
};

//////////////////////////////////////////////////////////////////////////
// FileSystemRootEntry class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   FileSystemRootEntry
 * \brief   Represents the root entry in the file system.
 **/
class FileSystemRootEntry : public FileSystemEntry
{
public:
    /**
     * \brief   Constructor with initialization.
     * \param   name    The name of the root entry.
     **/
    FileSystemRootEntry(const QString & name);
    
    /**
     * \brief   Destructor.
     **/
    virtual ~FileSystemRootEntry(void) = default;
    
    /**
     * \brief   Sets the workspace directories for the root entry.
     * \param   workspaceDirs   The list of workspace directories that contains file path and display name pair.
     **/
    void setWorkspaceDirectories(const QMap<QString, QString>& workspaceDirs);
    
//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Returns the next ID for the root entry.
     * \return  The next ID for the root entry.
     **/
    virtual uint32_t  getNextId(void) const override;
    
    /**
     * \brief   Fetches data for the root entry.
     * \param   filter  The list of file extension filters.
     * \return  The list of file information.
     **/
    virtual QFileInfoList fetchData(const QStringList & filter = QStringList()) const override;
    
    /**
     * \brief   Create and returns the child entry. The entry will not be added to the children list.
     * \param   path    The path of the file system entry.
     **/
    virtual FileSystemEntry* createChildEntry(const QString& path) const override;

    /**
     * \brief   Create and returns the child entry. The entry will not be added to the children list.
     * \param   fileInfo    The file information of the file system entry.
     **/
    virtual FileSystemEntry* createChildEntry(const QFileInfo& fileInfo) const override;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    mutable uint32_t        mNextId;        //!< The next ID for the root entry.
    QMap<QString, QString>  mWorkspaceDirs; //!< The list of workspace directories.
    
//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    FileSystemRootEntry(void) = delete;
};

//////////////////////////////////////////////////////////////////////////
// FileSystemEntry class inline methods
//////////////////////////////////////////////////////////////////////////

inline const QList<FileSystemEntry*>& FileSystemEntry::getChildren(void) const
{
    return mChildren;
}

inline FileSystemEntry* FileSystemEntry::getChild(int index)
{
    return ((index >= 0) && (index < mChildren.size()) ? mChildren[index] : nullptr);
}

inline const FileSystemEntry* FileSystemEntry::getChild(int index) const
{
    return ((index >= 0) && (index < mChildren.size()) ? mChildren[index] : nullptr);
}

inline FileSystemEntry* FileSystemEntry::getChild(const QString& path)
{
    for (auto entry : mChildren )
    {
        if (entry->getPath() == path)
            return entry;
    }
    
    return nullptr;
}

inline const FileSystemEntry* FileSystemEntry::getChild(const QString& path) const
{
    for (const auto entry : mChildren )
    {
        if (entry->getPath() == path)
            return entry;
    }
    
    return nullptr;
}

inline FileSystemEntry* FileSystemEntry::getChild(uint32_t id)
{
    for (auto entry : mChildren )
    {
        if (entry->getId() == id)
            return entry;
    }
    
    return nullptr;
}

inline const FileSystemEntry* FileSystemEntry::getChild(uint32_t id) const
{
    for (const auto entry : mChildren )
    {
        if (entry->getId() == id)
            return entry;
    }
    
    return nullptr;
}

inline int FileSystemEntry::getChildCount(void) const
{
    return mChildren.size();
}

inline const QString& FileSystemEntry::getPath(void) const
{
    return mFilePath;
}

inline QString FileSystemEntry::getFileName(void) const
{
    QFileInfo fi(mFilePath);
    QString fileName(fi.fileName());
    return (fileName.isEmpty() && fi.isDir() ? fi.dir().dirName() : fileName);
}

inline const QString& FileSystemEntry::getDisplayName(void) const
{
    return mDispName;
}

inline const QIcon& FileSystemEntry::getIcon(void) const
{
    return mIcon;
}

inline int FileSystemEntry::getRow(void) const
{
    return (mParent != nullptr ? mParent->mChildren.indexOf(this) : -1);
}

inline FileSystemEntry* FileSystemEntry::getParent(void)
{
    return mParent;
}

inline const FileSystemEntry* FileSystemEntry::getParent(void) const
{
    return mParent;
}

inline void FileSystemEntry::setEntryType(FileSystemEntry::eEntryType entryType)
{
    mEntryType = entryType;
}

inline bool FileSystemEntry::isDir(void) const
{
    return ((static_cast<uint16_t>(mEntryType) & static_cast<uint16_t>(EntryDir)) != 0);
}

inline bool FileSystemEntry::isFile(void) const
{
    return ((static_cast<uint16_t>(mEntryType) & static_cast<uint16_t>(EntryFile)) != 0);
}

inline bool FileSystemEntry::isSymlink(void) const
{
    return (mEntryType == EntrySymlink);
}

inline bool FileSystemEntry::isRoot(void) const
{
    return (mEntryType == EntryRoot);
}

inline bool FileSystemEntry::isWorkspaceDir(void) const
{
    return (mEntryType == EntryWorkspace);
}

inline bool FileSystemEntry::isValid(void) const
{
    return (mEntryType != EntryUnknown);
}

inline uint32_t FileSystemEntry::getId(void) const
{
    return mId;
}

inline FileSystemEntry* FileSystemEntry::addChild(const QString& path, FileSystemEntry::eEntryType entryType, bool sort /*= true*/)
{
    FileSystemEntry* entry = new FileSystemEntry(path, entryType, this);
    if (addChild(entry, sort) == false)
    {
        delete entry;
        entry = nullptr;
    }
    
    return entry;
}

inline FileSystemEntry* FileSystemEntry::addChild(const QString& path, FileSystemEntry::eEntryType entryType, const QIcon& icon, bool sort /*= true*/)
{
    FileSystemEntry* entry = new FileSystemEntry(path, QString(), entryType, icon, this);
    if (addChild(entry, sort) == false)
    {
        delete entry;
        entry = nullptr;
    }
    
    return entry;
}

inline FileSystemEntry* FileSystemEntry::addChild(const QFileInfo &fi, bool sort /*= true*/)
{
    FileSystemEntry* entry = createChildEntry(fi);
    if (addChild(entry, sort) == false)
    {
        delete entry;
        entry = nullptr;
    }
    
    return entry;
}

inline bool FileSystemEntry::addDummyEntry(void)
{
    bool result {false};
    if (mChildren.isEmpty())
    {
        mChildren.insert(0, &EmptyEntry);
        result = true;
    }
    else
    {
        result = (mChildren.size() == 1) && (mChildren[0]->isValid() == false);
    }
    
    return result;
}

inline void FileSystemEntry::removeDummyEntry(void)
{
    if ((mChildren.size() == 1) && (mChildren[0]->isValid() == false))
    {
        mChildren.removeAt(0);
    }
}
    
inline void FileSystemEntry::removeChild(FileSystemEntry* entry)
{
    mChildren.removeOne(entry);
}

inline void FileSystemEntry::removeChild(int row)
{
    mChildren.removeAt(row);
}

inline void FileSystemEntry::removeChild(const QString& path)
{
    for (int i = 0; i < mChildren.size(); ++i)
    {
        if (mChildren[i]->getPath() == path)
        {
            mChildren.removeAt(i);
            break;
        }
    }
}

inline void FileSystemEntry::removeChild(uint32_t id)
{
    for (int i = 0; i < mChildren.size(); ++i)
    {
        if (mChildren[i]->getId() == id)
        {
            mChildren.removeAt(i);
            break;
        }
    }
}

inline void FileSystemEntry::removeAll(void)
{
    deleteEntries();
}

inline void FileSystemEntry::resetEntry(void)
{
    deleteEntries();
    addDummyEntry();
}

#endif // LUSAN_MODEL_COMMON_FILESYSTEMENTRY_HPP
