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
 *  \file        lusan/model/log/LogViewerFilterProxy.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Filter Proxy Model.
 *
 ************************************************************************/

#include "lusan/model/log/LogViewerFilterProxy.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"
#include <QModelIndex>

LogViewerFilterProxy::LogViewerFilterProxy(QAbstractTableModel* model)
    : QSortFilterProxyModel (model)
    , mComboFilters         ( )
    , mTextFilters          ( )
    , mLogModel             (model)
{
    setSourceModel(model);
}

void LogViewerFilterProxy::setComboFilter(int logicalColumn, const QStringList& items)
{
    if (items.isEmpty())
    {
        mComboFilters.remove(logicalColumn);
    }
    else
    {
        mComboFilters[logicalColumn] = items;
    }

    invalidateFilter();
}

void LogViewerFilterProxy::setTextFilter(int logicalColumn, const QString& text)
{
    if (text.isEmpty())
    {
        mTextFilters.remove(logicalColumn);
    }
    else
    {
        mTextFilters[logicalColumn] = text;
    }

    invalidateFilter();
}

void LogViewerFilterProxy::clearFilters()
{
    mComboFilters.clear();
    mTextFilters.clear();
    invalidateFilter();
}

bool LogViewerFilterProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_UNUSED(source_parent);

    if (mLogModel == nullptr)
        return true;

    // Check if row matches all active filters
    return matchesComboFilters(source_row) && matchesTextFilters(source_row);
}

bool LogViewerFilterProxy::matchesComboFilters(int source_row) const
{
    if (mLogModel == nullptr)
        return true;

    // Check each active combo filter
    for (auto it = mComboFilters.constBegin(); it != mComboFilters.constEnd(); ++it)
    {
        int column = it.key();
        const QStringList& filterItems = it.value();

        if (filterItems.isEmpty())
            continue;

        // Get the data for this column and row
        QModelIndex index = mLogModel->index(source_row, column);
        if (!index.isValid())
            continue;

        QString cellData = mLogModel->data(index, Qt::DisplayRole).toString();

        // Check if the cell data matches any of the selected filter items
        bool matches = false;
        for (const QString& filterItem : filterItems)
        {
            if (cellData == filterItem)
            {
                matches = true;
                break;
            }
        }

        // If this column doesn't match, the row is filtered out
        if (!matches)
            return false;
    }

    return true;
}

bool LogViewerFilterProxy::matchesTextFilters(int source_row) const
{
    if (mLogModel == nullptr)
        return true;

    // Check each active text filter
    for (auto it = mTextFilters.constBegin(); it != mTextFilters.constEnd(); ++it)
    {
        int column = it.key();
        const QString& filterText = it.value();

        if (filterText.isEmpty())
            continue;

        // Get the data for this column and row
        QModelIndex index = mLogModel->index(source_row, column);
        if (!index.isValid())
            continue;

        QString cellData = mLogModel->data(index, Qt::DisplayRole).toString();

        // Check if the cell data contains the filter text (case-insensitive)
        if (!cellData.contains(filterText, Qt::CaseInsensitive))
            return false;
    }

    return true;
}
