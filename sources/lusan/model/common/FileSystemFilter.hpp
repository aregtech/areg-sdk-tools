#ifndef LUSAN_MODEL_COMMON_FILESYSTEMFILTER_HPP
#define LUSAN_MODEL_COMMON_FILESYSTEMFILTER_HPP
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
 *  \file        lusan/model/common/FileSystemFilter.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, File System filter to display supported files.
 *
 ************************************************************************/

#include <QSortFilterProxyModel>
#include <QSet>

class FileSystemModel;

/**
 * \class   FileSystemFilter
 * \brief   Model to manage and display file system entries in a QTreeView.
 **/
class FileSystemFilter : public QSortFilterProxyModel
{
    Q_OBJECT

private:
    enum class eFilter : uint32_t
    {
          FilterNone    = 0
        , ShowHidden    = 1
        , ShowEmptyDirs = 2
        , ShowAllFiles  = 4
        , ShowAll       = 7
    };
public:
    /**
     * \brief   Constructor with initialization.
     * \param   parent  The parent object.
     **/
    FileSystemFilter(FileSystemModel* dataSource, QObject* parent = nullptr);

    /**
     * \brief   Sets the filter to show all files and folders.
     * \param   showAll  If true, shows all files and folders.
     **/
    void setShowAll(bool showAll);

    /**
     * \brief   Sets the filter to hide or show hidden files and folders.
     * \param   showHidden  If true, shows hidden files and folders.
     **/
    void setShowHidden(bool showHidden);

    /**
     * \brief   Sets the filter to show files with any extension.
     * \param   showAllExtensions  If true, shows files with any extension.
     **/
    void setShowAllExtensions(bool showAllExtensions);

    /**
     * \brief   Sets the filter to show only specific file extensions.
     * \param   extensions  The list of file extensions to show.
     **/
    void setFilterExtensions(const QStringList& extensions);

    /**
     * \brief   Sets the filter to show or hide empty folders.
     * \param   showEmptyFolders  If true, shows empty folders.
     **/
    void setShowEmptyFolders(bool showEmptyFolders);

protected:
    /**
     * \brief   Filters the files and folders based on the current settings.
     * \param   path  The path to filter.
     * \return  True if the path should be shown, false otherwise.
     **/
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

    virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;

    inline bool showAll(void) const;

    inline bool showHidden(void) const;

    inline bool showEmptyDir(void) const;

    inline bool showAllFiles(void) const;

private:
    uint32_t            mFilterFlag;
    QSet<QString>       mFileFilter;    //!< Set of file extensions to filter.
};

//////////////////////////////////////////////////////////////////////////
// FileSystemFilter class inline methods
//////////////////////////////////////////////////////////////////////////

inline bool FileSystemFilter::showAll(void) const
{
    return (mFilterFlag == static_cast<uint32_t>(eFilter::ShowAll));
}

inline bool FileSystemFilter::showHidden(void) const
{
    return ((mFilterFlag & static_cast<uint32_t>(eFilter::ShowHidden)) != 0);
}

inline bool FileSystemFilter::showEmptyDir(void) const
{
    return ((mFilterFlag & static_cast<uint32_t>(eFilter::ShowEmptyDirs)) != 0);
}

inline bool FileSystemFilter::showAllFiles(void) const
{
    return ((mFilterFlag & static_cast<uint32_t>(eFilter::ShowAllFiles)) != 0);
}

#endif // LUSAN_MODEL_COMMON_FILESYSTEMFILTER_HPP
