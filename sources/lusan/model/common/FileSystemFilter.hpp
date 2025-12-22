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
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/model/common/FileSystemFilter.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, File System filter to display supported files.
 *
 ************************************************************************/

#include <QSortFilterProxyModel>

class QFileSystemModel;

/**
 * \class   FileSystemFilter
 * \brief   Model to manage and display file system entries in a QTreeView.
 **/
class FileSystemFilter : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    /**
     * \brief   Constructor with initialization.
     * \param   parent  The parent object.
     **/
    FileSystemFilter(QFileSystemModel* dataSource, QObject* parent = nullptr);

protected:
    /**
     * \brief   Sorts the files and folders based on the current settings.
     * \param   source_left The left side index to compare.
     * \param   source_right    The right side index to compare.
     * \return  True if the left side index element is greater than the right side index.
     **/
    virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
};

#endif // LUSAN_MODEL_COMMON_FILESYSTEMFILTER_HPP
