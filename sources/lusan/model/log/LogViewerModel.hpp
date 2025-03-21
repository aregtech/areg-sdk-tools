#ifndef LUSAN_MODEL_LOG_LOGVIEWERMODEL_HPP
#define LUSAN_MODEL_LOG_LOGVIEWERMODEL_HPP
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
 *  \copyright   � 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/model/log/LogViewerModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include <QAbstractTableModel>

class LogViewerModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit LogViewerModel(QObject *parent = nullptr);

    // Header:
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Fetch data dynamically:
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    virtual bool canFetchMore(const QModelIndex &parent) const override;
    virtual void fetchMore(const QModelIndex &parent) override;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Add data:
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

private:
};

#endif // LUSAN_MODEL_LOG_LOGVIEWERMODEL_HPP
