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
#include <QDir>
#include <QFileSystemModel>

FileSystemFilter::FileSystemFilter(QFileSystemModel* dataSource, QObject* parent)
    : QSortFilterProxyModel(parent)
{
    Q_ASSERT(dataSource != nullptr);
    this->setSourceModel(dataSource);
    this->setDynamicSortFilter(true);
    this->setSortRole(Qt::ItemDataRole::DisplayRole);
    this->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
    this->setFilterKeyColumn(0);
}

bool FileSystemFilter::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    QFileSystemModel* dataSource = static_cast<QFileSystemModel*>(this->sourceModel());
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
