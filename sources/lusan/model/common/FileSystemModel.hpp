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
     * \param   workspaceEntries   The map of workspace entries.
     * \param   parent             The parent object.
     **/
    explicit FileSystemModel(const QMap<QString, QString> & workspaceEntries, QObject* parent = nullptr);

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
    virtual QModelIndex parent(const QModelIndex& child) const;

    /**
     * \brief   Returns the number of rows under the given parent.
     * \param   parent  The parent index.
     * \return  The number of rows.
     **/
    virtual int rowCount(const QModelIndex& parent) const;

    /**
     * \brief   Returns the number of columns for the children of the given parent.
     * \param   parent  The parent index.
     * \return  The number of columns.
     **/
    virtual int columnCount(const QModelIndex& parent) const;

    /**
     * \brief   Returns the data stored under the given role for the item referred to by the index.
     * \param   index   The index of the item.
     * \param   role    The role for which data is requested.
     * \return  The data for the given role and section.
     **/
    virtual QVariant data(const QModelIndex& index, int role) const;

    /**
     * \brief   Returns the data for the given role and section in the header with the specified orientation.
     * \param   section     The section of the header.
     * \param   orientation The orientation of the header.
     * \param   role        The role for which data is requested.
     * \return  The data for the given role and section.
     **/
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    /**
     * \brief   Fetches more data for the given parent index.
     * \param   parent  The parent index.
     **/
    virtual void fetchMore(const QModelIndex& parent);

    /**
     * \brief   Checks if more data can be fetched for the given parent index.
     * \param   parent  The parent index.
     * \return  True if more data can be fetched, false otherwise.
     **/
    virtual bool canFetchMore(const QModelIndex& parent) const;

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
    QModelIndex setRootPaths(const QMap<QString, QString>& paths);
    
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
     * \brief   Refreshes the data for the given index.
     * \param   index   The index of the item.
     **/
    void refresh(const QModelIndex& index);
    
    /**
     * \brief   Returns the file information for the given index.
     * \param   index   The index of the item.
     * \return  The file information.
     **/
    QFileInfo getFileInfo(const QModelIndex& index) const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    FileSystemRootEntry mRootEntry;      //!< The root entry of the file system.
    WorkspaceEntries    mWorkspaceDirs;  //!< The map of workspace directories.
};

//////////////////////////////////////////////////////////////////////////
// FileSystemModel class inline methods
//////////////////////////////////////////////////////////////////////////

inline bool FileSystemModel::isValidIndex(const QModelIndex& index) const
{
    return (index.isValid() && (index.row() >= 0) && (index.column() == 0) && (index.model() == this));
}

#endif // LUSAN_MODEL_COMMON_FILESYSTEMMODEL_HPP
