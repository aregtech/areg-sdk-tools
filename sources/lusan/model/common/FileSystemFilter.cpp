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
 *  \file        lusan/model/common/FileSystemFilter.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, File System Model implementation.
 *
 ************************************************************************/

#include "lusan/model/common/FileSystemFilter.hpp"
#include "lusan/model/common/FileSystemModel.hpp"
#include <QDir>

FileSystemFilter::FileSystemFilter(FileSystemModel* dataSource, QObject* parent)
    : QSortFilterProxyModel(parent)

    , mFilterFlag(static_cast<uint32_t>(eFilter::ShowAll))
    , mFileFilter()
{
    Q_ASSERT(dataSource != nullptr);
    this->setSourceModel(dataSource);
    this->setDynamicSortFilter(true);
    this->setSortRole(Qt::ItemDataRole::DisplayRole);
    this->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
    this->setFilterKeyColumn(0);
}

void FileSystemFilter::setShowAll(bool showAll)
{
    if (showAll)
    {
        mFilterFlag = static_cast<uint32_t>(eFilter::ShowAll);
    }
    else
    {
        mFilterFlag &= ~static_cast<uint32_t>(eFilter::ShowHidden);
        mFilterFlag &= ~static_cast<uint32_t>(eFilter::ShowAllFiles);
    }

    invalidateFilter();
}

void FileSystemFilter::setShowHidden(bool showHidden)
{
    if (showHidden)
    {
        mFilterFlag |= static_cast<uint32_t>(eFilter::ShowHidden);
    }
    else
    {
        mFilterFlag &= ~static_cast<uint32_t>(eFilter::ShowHidden);
    }

    invalidateFilter();
}

void FileSystemFilter::setShowAllExtensions(bool showAllExtensions)
{
    if (showAllExtensions)
    {
        mFilterFlag |= static_cast<uint32_t>(eFilter::ShowAllFiles);
    }
    else
    {
        mFilterFlag &= ~static_cast<uint32_t>(eFilter::ShowAllFiles);
    }

    invalidateFilter();
}

void FileSystemFilter::setFilterExtensions(const QStringList& extensions)
{
    mFileFilter = QSet<QString>(extensions.begin(), extensions.end());
    invalidateFilter();
}

void FileSystemFilter::setShowEmptyFolders(bool showEmptyFolders)
{
    if (showEmptyFolders)
    {
        mFilterFlag |= static_cast<uint32_t>(eFilter::ShowEmptyDirs);
    }
    else
    {
        mFilterFlag &= ~static_cast<uint32_t>(eFilter::ShowEmptyDirs);
    }

    invalidateFilter();
}

bool FileSystemFilter::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    FileSystemModel* dataSource = static_cast<FileSystemModel*>(this->sourceModel());
    Q_ASSERT(dataSource != nullptr);
    QFileInfo fileInfo = dataSource->fileInfo(index);
    if (fileInfo.isRoot())
        return true;

    QString file = fileInfo.absoluteFilePath();

    if (showAll())
        return true;

    if (!showHidden() && fileInfo.isHidden())
        return false;

    if (fileInfo.isDir())
    {
        if (!showEmptyDir() && QDir(file).isEmpty())
            return false;

        return true;
    }

    if (showAllFiles())
        return true;

    return mFileFilter.contains(fileInfo.suffix());
}

bool FileSystemFilter::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    FileSystemModel* dataSource = static_cast<FileSystemModel*>(this->sourceModel());
    Q_ASSERT(dataSource != nullptr);
    QFileInfo left = dataSource->fileInfo(source_left);
    QFileInfo right = dataSource->fileInfo(source_right);

    if (left.isDir() && !right.isDir())
    {
        return false;
    }
    else if (!left.isDir() && right.isDir())
    {
        return true;
    }
    else
    {
        QString l{ left.fileName() };
        QString r{ right.fileName() };

        return l.compare(r, Qt::CaseInsensitive) > 0;
    }
}
