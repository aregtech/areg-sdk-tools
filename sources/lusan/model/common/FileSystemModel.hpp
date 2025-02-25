#ifndef LUSAN_MODEL_COMMON_FILESYSTEMMODEL_HPP
#define LUSAN_MODEL_COMMON_FILESYSTEMMODEL_HPP
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
 *  \file        lusan/model/common/FileSystemModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, File System Model for QTreeView.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/FileSystemEntry.hpp"
#include <QAbstractItemModel>
#include <QFileSystemModel>
#include <QFileInfo>

/**
 * \class   FileSystemModel
 * \brief   Model to manage and display file system entries in a QTreeView.
 **/
class FileSystemModel : public QAbstractItemModel
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
private:
    using WorkspaceEntries  = QMap<QString, QString>;
    
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors / destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor with initialization.
     * \param   parent  The parent object.
     **/
    FileSystemModel(QObject * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   workspaceEntries    The map of workspace entries.
     * \param   extFilters          The list of file extension filters.
     * \param   parent              The parent object.
     **/
    explicit FileSystemModel(const QMap<QString, QString> & workspaceEntries, const QStringList & extFilters, QObject* parent = nullptr);

    /**
     * \brief   Destructor.
     **/
    virtual ~FileSystemModel();
    
//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the index of the item in the model specified by the given row, column and parent index.
     * \param   row     The row of the item.
     * \param   column  The column of the item.
     * \param   parent  The parent index.
     * \return  The index of the item.
     **/
    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    /**
     * \brief   Returns the parent index of the given child index.
     * \param   child   The child index.
     * \return  The parent index.
     **/
    virtual QModelIndex parent(const QModelIndex& child) const override;

    /**
     * \brief   Returns the number of rows under the given parent.
     * \param   parent  The parent index.
     * \return  The number of rows.
     **/
    virtual int rowCount(const QModelIndex& parent) const override;

    /**
     * \brief   Returns the number of columns for the children of the given parent.
     * \param   parent  The parent index.
     * \return  The number of columns.
     **/
    virtual int columnCount(const QModelIndex& parent) const override;

    /**
     * \brief   Returns the data stored under the given role for the item referred to by the index.
     * \param   index   The index of the item.
     * \param   role    The role for which data is requested.
     * \return  The data for the given role and section.
     **/
    virtual QVariant data(const QModelIndex& index, int role) const override;

    /**
     * \brief   Returns the data for the given role and section in the header with the specified orientation.
     * \param   section     The section of the header.
     * \param   orientation The orientation of the header.
     * \param   role        The role for which data is requested.
     * \return  The data for the given role and section.
     **/
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * \brief   Fetches more data for the given parent index.
     * \param   parent  The parent index.
     **/
    virtual void fetchMore(const QModelIndex& parent) override;

    /**
     * \brief   Checks if more data can be fetched for the given parent index.
     * \param   parent  The parent index.
     * \return  True if more data can be fetched, false otherwise.
     **/
    virtual bool canFetchMore(const QModelIndex& parent) const override;

    /**
     * \brief   Returns the flags for the item at the given index.
     * \param   index   The index of the item.
     * \return  The flags of the item.
     **/
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Checks if the given index is valid.
     * \param   index   The index to check.
     * \return  True if the index is valid, false otherwise.
     **/
    inline bool isValidIndex(const QModelIndex& index) const;

    /**
     * \brief   Sets the root paths for the file system model.
     * \param   paths   The map of root paths.
     * \return  The index of the root item.
     **/
    const QModelIndex& setRootPaths(const QMap<QString, QString>& paths);
    
    /**
     * \brief   Returns the root paths of the file system model.
     * \return  The map of root paths.
     **/
    const QMap<QString, QString>& getRootPaths(void) const;

    /**
     * \brief   Returns the file path for the given index.
     * \param   index   The index of the item.
     * \return  The file path.
     **/
    QString filePath(const QModelIndex& index);

    /**
     * \brief   Resets the model and refreshes the model starting from root entry.
     **/
    void refresh(void);

    /**
     * \brief   Resets and refreshes the entries of specified index.
     **/
    void refresh(const QModelIndex & index);
    
    /**
     * \brief   Resets and refreshes the entries of specified entry.
     **/
    void refresh(FileSystemEntry* entry);
    
    /**
     * \brief   Returns the file information for the given index.
     * \param   index   The index of the item.
     * \return  The file information.
     **/
    QFileInfo getFileInfo(const QModelIndex& index) const;

    /**
     * \brief   Sets the file filter to display file system data.
     * \param   filterList  The list of file extension filters.
     **/
    void setFileFilter(const QStringList& filterList);

    /**
     * \brief   Cleans the file filter to display all file system data.
     **/
    void cleanFilters(void);

    /**
     * \brief   Deletes a file system entry for the given index.
     * \param   index   The index of the item.
     * \return  Returns true if the entry is deleted successfully.
     **/
    bool deleteEntry(const QModelIndex & index);

    /**
     * \brief   Returns the root index of the file system model.
     **/
    QModelIndex getRootIndex(void) const;
    
    /**
     * \brief   Inserts a new directory into the file system model.
     * \param   dirName     The name of the directory to insert.
     * \param   parentIndex The index of the parent item where the directory should be inserted.
     * \return  The index of the newly inserted directory.
     **/
    QModelIndex insertDirectory(const QString& dirName, const QModelIndex& parentIndex);

    /**
     * \brief   Inserts a new file into the file system model.
     * \param   fileName    The name of the file to insert.
     * \param   parentIndex The index of the parent item where the file should be inserted.
     * \return  The index of the newly inserted file.
     **/
    QModelIndex insertFile(const QString& fileName, const QModelIndex& parentIndex);

    /**
     * \brief   Renames a file system entry.
     * \param   newName The new name for the file system entry.
     * \param   index   The index of the entry to rename.
     * \return  The index of the renamed entry.
     **/
    QModelIndex renameEntry(const QString& newName, const QModelIndex& index);

    /**
     * \brief   Checks if the given parent index contains a child entry with the specified file name.
     * \param   parentIndex The index of the parent item.
     * \param   fileName    The name of the file to check.
     * \return  True if the parent index contains a child entry with the specified file name, false otherwise.
     **/
    bool containsChildEntry(const QModelIndex & parentIndex, const QString& fileName) const;

    /**
     * \brief   Checks if a directory exists at the given path.
     * \param   dirPath     The full path of the directory to check.
     * \return  True if the directory exists, false otherwise.
     **/
    bool existsDirectory(const QString & dirPath) const;

    /**
     * \brief   Checks if a subdirectory exists within the given parent directory.
     * \param   parentIndex The index of the parent directory.
     * \param   subdirName  The name of the subdirectory to check.
     * \return  True if the subdirectory exists, false otherwise.
     **/
    bool existsDirectory(const QModelIndex & parentIndex, const QString & subdirName) const;

    /**
     * \brief   Checks if a file exists at the given file path.
     * \param   filePath    The full path of the file to check.
     * \return  True if the file exists, false otherwise.
     **/
    bool existsFile(const QString& filePath) const;

    /**
     * \brief   Checks if a file exists within the given parent directory.
     * \param   parentIndex The index of the parent directory.
     * \param   fileName    The name of the file to check.
     * \return  True if the file exists, false otherwise.
     **/
    bool existsFile(const QModelIndex & parentIndex, const QString& fileName) const;

    /**
     * \brief   Checks if the given index is a file.
     * \param   index   The index of the item.
     * \return  True if the index is a file, false otherwise.
     **/
    bool isFile(const QModelIndex & index) const;

    /**
     * \brief   Checks if the given index is a directory.
     * \param   index   The index of the item.
     * \return  True if the index is a directory, false otherwise.
     **/
    bool isDir(const QModelIndex & index) const;

    /**
     * \brief   Checks if the given index is a workspace entry.
     * \param   index   The index of the item.
     * \return  True if the index is a workspace entry, false otherwise.
     **/
    bool isWorkspaceEntry(const QModelIndex & index) const;

    /**
     * \brief   Checks if the given index is a workspace project.
     * \param   index   The index of the item.
     * \return  True if the index is a workspace project, false otherwise.
     **/
    bool isWorkspaceProject(const QModelIndex & index) const;

    /**
     * \brief   Checks if the given index is a workspace project subdirectory entry.
     * \param   index   The index of the item.
     * \return  True if the index is a workspace project subdirectory entry, false otherwise.
     **/
    bool isWorkspaceProjectSubdirEntry(const QModelIndex& index) const;

    /**
     * \brief   Checks if the given index is a workspace source directory.
     * \param   index   The index of the item.
     * \return  True if the index is a workspace source directory, false otherwise.
     **/
    bool isWorkspaceSource(const QModelIndex & index) const;

    /**
     * \brief   Checks if the given index is a workspace source subdirectory entry.
     * \param   index   The index of the item.
     * \return  True if the index is a workspace source subdirectory entry, false otherwise.
     **/
    bool isWorkspaceSourceSubdirEntry(const QModelIndex& index) const;

    /**
     * \brief   Checks if the given index is a workspace delivery directory.
     * \param   index   The index of the item.
     * \return  True if the index is a workspace delivery directory, false otherwise.
     **/
    bool isWorkspaceDelivery(const QModelIndex & index) const;

    /**
     * \brief   Checks if the given index is a workspace delivery subdirectory entry.
     * \param   index   The index of the item.
     * \return  True if the index is a workspace delivery subdirectory entry, false otherwise.
     **/
    bool isWorkspaceDeliverySubdirEntry(const QModelIndex& index) const;

    /**
     * \brief   Checks if the given index is a workspace include directory.
     * \param   index   The index of the item.
     * \return  True if the index is a workspace include directory, false otherwise.
     **/
    bool isWorkspaceInclude(const QModelIndex & index) const;

    /**
     * \brief   Checks if the given index is a workspace include subdirectory entry.
     * \param   index   The index of the item.
     * \return  True if the index is a workspace include subdirectory entry, false otherwise.
     **/
    bool isWorkspaceIncludeSubdirEntry(const QModelIndex& index) const;

    /**
     * \brief   Checks if the given index is a file that can be handled by Lusan application.
     * \param   index   The index of the item.
     * \return  True if the index is a file to be handled by Lusan application, false otherwise.
     **/
    bool isLusanFile(const QModelIndex & index) const;

    /**
     * \brief   Checks if the given index is a root entry.
     * \param   index   The index of the item.
     * \return  True if the index is a root entry, false otherwise.
     **/
    bool isRoot(const QModelIndex & index) const;
    
//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Resets the given entry, removes all children.
     * \param   entry   The entry to reset.
     **/
    void resetEntry(FileSystemEntry * entry);

    /**
     * \brief   Resets the root entry of the file system.
     **/
    void resetRoot(void);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    FileSystemRootEntry mRootEntry;     //!< The root entry of the file system.
    WorkspaceEntries    mWorkspaceDirs; //!< The map of workspace directories.
    QStringList         mFileFilter;    //!< The filter to set when display file system data.
    QModelIndex         mRootIndex;     //!< The index of the root.
};

//////////////////////////////////////////////////////////////////////////
// GeneralFileSystemModel class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   GeneralFileSystemModel
 * \brief   General file system model to display file system entries in a QTreeView.
 **/
class GeneralFileSystemModel    : public QFileSystemModel
{
//////////////////////////////////////////////////////////////////////////
// Constructors / destructor
//////////////////////////////////////////////////////////////////////////
public:
    GeneralFileSystemModel(QObject* parent = nullptr);

    virtual ~GeneralFileSystemModel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the number of columns for the children of the given parent.
     * \param   parent  The parent index.
     * \return  The number of columns.
     **/
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * \brief   Returns the header data stored under the given role.
     * \param   section The section.
     * \param   orientation The orientation.
     * \param   role        The role for which data is requested.
     * \return  The header data for the given role and section.
     **/
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};

//////////////////////////////////////////////////////////////////////////
// FileSystemModel class inline methods
//////////////////////////////////////////////////////////////////////////

inline bool FileSystemModel::isValidIndex(const QModelIndex& index) const
{
    return (index.isValid() && (index.row() >= 0) && (index.column() == 0) && (index.model() == this));
}

#endif // LUSAN_MODEL_COMMON_FILESYSTEMMODEL_HPP
