﻿/************************************************************************
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
 *  \file        lusan/model/log/LogViewerModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Model.
 *
 ************************************************************************/

#include "lusan/model/log/LogViewerModel.hpp"
#include "lusan/data/log/LogObserverComp.hpp"
#include "lusan/data/log/LogObserver.hpp"
#include "lusan/data/log/NELogObserver.hpp"

#include "areg/base/DateTime.hpp"
#include "areg/logging/NELogging.hpp"

#include <QSize>

const QStringList& LogViewerModel::getHeaderList(void)
{
    static QStringList _headers
        {
              "Priority"
            , "Timestamp"
            , "Source"
            , "Source ID"
            , "Thread"
            , "Thread ID"
            , "Scope ID"
            , "Message"
        };
    
    return _headers;
}

const QList<int>& LogViewerModel::getDefaultColumns(void)
{
    static QList<int>   _columnIds
    {
          static_cast<int>(eColumn::LogColumnPriority)
        , static_cast<int>(eColumn::LogColumnTimestamp)
        , static_cast<int>(eColumn::LogColumnMessage)
    };
    
    return _columnIds;
}

LogViewerModel::LogViewerModel(QObject *parent)
    : QAbstractTableModel(parent)

    , mLogObserver  ( nullptr )
    , mActiveColumns( )
{
    const QList<int>& list = LogViewerModel::getDefaultColumns();
    for (int col : list)
    {
        mActiveColumns.append(static_cast<eColumn>(col));
    }
}

QVariant LogViewerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Vertical)
        return QVariant();

#if 0
    if (static_cast<Qt::ItemDataRole>(role) == Qt::ItemDataRole::UserRole)
    {
        return QVariant( static_cast<int>(mActiveColumns.at(section)));
    }
    else
    {
        QString header(getHeaderName(section));
        return QVariant(header);
    }        
#else
    if (static_cast<Qt::ItemDataRole>(role) == Qt::ItemDataRole::DisplayRole)
    {
        return QVariant(getHeaderName(section));
    }
    else if (static_cast<Qt::ItemDataRole>(role) == Qt::ItemDataRole::UserRole)
    {
        return QVariant( static_cast<int>(mActiveColumns.at(section)));
    }
    else if (static_cast<Qt::ItemDataRole>(role) == Qt::ItemDataRole::SizeHintRole)
    {
        eColumn col = mActiveColumns.at(section);
        switch (col)
        {
        case eColumn::LogColumnMessage:
            return QVariant(QSize(200, 28));
        case eColumn::LogColumnPriority:
        case eColumn::LogColumnScopeId:
        case eColumn::LogColumnSourceId:
        case eColumn::LogColumnThreadId:
            return QVariant(QSize(50, 28));
        case eColumn::LogColumnSource:
        case eColumn::LogColumnThread:
        case eColumn::LogColumnTimestamp:
            return QVariant(QSize(100, 28));
        
        default:
            break;
        }
    }
    
    return QVariant();
#endif
}

int LogViewerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return (mLogObserver != nullptr ? mLogObserver->getLogObserver().getLogMessages().getSize() : 0);
}

int LogViewerModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    
    return mActiveColumns.size();
}

QVariant LogViewerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::ItemDataRole::DisplayRole)
    {
        const sLogMessage* logMessage = mLogObserver != nullptr ? mLogObserver->getLogMessage(index.row()) : nullptr;
        if (logMessage != nullptr)
        {
            eColumn col = static_cast<eColumn>(getDefaultColumns().at(index.column()));
            switch (col)
            {
            case eColumn::LogColumnPriority:
                return QVariant( QString(NELogging::getString(static_cast<NELogging::eLogPriority>(logMessage->msgPriority))) );
            case eColumn::LogColumnTimestamp:
            {
                DateTime timestamp(logMessage->msgTimestamp);
                return QVariant( QString(timestamp.formatTime().getString()) );
            }
            case eColumn::LogColumnSource:
                return QVariant(QString(logMessage->msgModule));
            case eColumn::LogColumnSourceId:
                return QVariant((qulonglong)logMessage->msgModuleId);
            case eColumn::LogColumnThread:
                return QVariant( QString(logMessage->msgThread) );
            case eColumn::LogColumnThreadId:
                return QVariant((qulonglong)logMessage->msgThreadId);
            case eColumn::LogColumnScopeId:
                return QVariant(logMessage->msgScopeId);
            case eColumn::LogColumnMessage:
                return QVariant( QString(logMessage->msgLogText) );
            default:
                break;
            }
        }
    }

    return QVariant(QString());
}

bool LogViewerModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return true;
}

bool LogViewerModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    endInsertColumns();
    return true;
}

bool LogViewerModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
    return true;
}

bool LogViewerModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return true;
}

QString LogViewerModel::getHeaderName(int colIndex) const
{
    if ((colIndex >= 0) && (colIndex < mActiveColumns.size()))
    {
        eColumn col = mActiveColumns.at(colIndex);
        const QStringList& header = LogViewerModel::getHeaderList();
        return header.at(static_cast<int>(col));
    }
    else
    {
        return QString();
    }
}

bool LogViewerModel::connect(const QString& hostName /*= ""*/, unsigned short portNr /*= 0u*/)
{
    mLogObserver = NELogObserver::startLobObserver() ? NELogObserver::getLogObserver() : nullptr;
    return (mLogObserver != nullptr);
}

void LogViewerModel::disconnect(void)
{
    NELogObserver::stopLogObserver();
    mLogObserver = nullptr;
}
