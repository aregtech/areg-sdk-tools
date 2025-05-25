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
 *  \file        lusan/model/log/LogViewerModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Model.
 *
 ************************************************************************/

#include "lusan/model/log/LogViewerModel.hpp"
#include "lusan/data/log/LogObserver.hpp"

#include "areg/base/DateTime.hpp"
#include "areg/base/NESocket.hpp"
#include "areg/base/SharedBuffer.hpp"
#include "areg/logging/NELogging.hpp"
#include "areglogger/client/LogObserverApi.h"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/log/LogObserver.hpp"

#include <QBrush>
#include <QColor>
#include <QIcon>
#include <QSize>

const QColor LogViewerModel::LogColors[static_cast<int>(ePrio::PrioTotal)]
{
      QColorConstants::Transparent
    , QColorConstants::Black
    , QColorConstants::Gray
    , QColorConstants::DarkGreen
    , QColorConstants::DarkCyan
    , QColorConstants::DarkBlue
    , QColorConstants::DarkRed
    , QColorConstants::Magenta
};


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

    , mIsConnected(false)
    , mAddress()
    , mPort(NESocket::InvalidPort)
    , mDbPath()

    , mActiveColumns( )
    , mLogs         ( )
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

    return static_cast<int>(mLogs.size());
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
    
    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
    {
        int row {index.row()};
        if (row >= mLogs.size())
            return QVariant();
        
        const SharedBuffer data{mLogs.at(row)};
        const NELogging::sLogMessage* logMessage = reinterpret_cast<const NELogging::sLogMessage*>(data.getBuffer());
        if (logMessage != nullptr)
        {
            eColumn col = static_cast<eColumn>(getDefaultColumns().at(index.column()));
            switch (col)
            {
            case eColumn::LogColumnPriority:
                return QVariant( QString(NELogging::logPrioToString(static_cast<NELogging::eLogPriority>(logMessage->logMessagePrio))) );
            case eColumn::LogColumnTimestamp:
            {
                DateTime timestamp(logMessage->logTimestamp);
                return QVariant( QString(timestamp.formatTime().getString()) );
            }
            case eColumn::LogColumnSource:
                return QVariant(QString(logMessage->logModule));
            case eColumn::LogColumnSourceId:
                return QVariant((qulonglong)logMessage->logModuleId);
            case eColumn::LogColumnThread:
                return QVariant( QString(logMessage->logThread) );
            case eColumn::LogColumnThreadId:
                return QVariant((qulonglong)logMessage->logThreadId);
            case eColumn::LogColumnScopeId:
                return QVariant(logMessage->logScopeId);
            case eColumn::LogColumnMessage:
                return QVariant( QString(logMessage->logMessage) );
            default:
                break;
            }
        }
    }
    break;
        
    case Qt::ItemDataRole::DecorationRole:
    {
        int row {index.row()};
        if (row >= mLogs.size())
            return QVariant();
        
        const SharedBuffer data{mLogs.at(row)};
        const NELogging::sLogMessage* logMessage = reinterpret_cast<const NELogging::sLogMessage*>(data.getBuffer());
        if ((logMessage != nullptr) && (static_cast<eColumn>(getDefaultColumns().at(index.column())) == eColumn::LogColumnPriority))
        {
            switch (logMessage->logMessagePrio)
            {
            case NELogging::eLogPriority::PrioScope:
                if (logMessage->logMsgType == NELogging::eLogMessageType::LogMessageScopeEnter)
                    return QIcon::fromTheme(QString::fromUtf8("media-seek-forward"));
                else if (logMessage->logMsgType == NELogging::eLogMessageType::LogMessageScopeExit)
                    return QIcon::fromTheme(QString::fromUtf8("media-seek-backward"));
                else
                    return QIcon::fromTheme(QString::fromUtf8("window-close"));
            case NELogging::eLogPriority::PrioDebug:
                return QIcon::fromTheme(QString::fromUtf8("format-justify-left"));
            case NELogging::eLogPriority::PrioInfo:
                return QIcon::fromTheme(QString::fromUtf8("dialog-information"));
            case NELogging::eLogPriority::PrioWarning:
                return QIcon::fromTheme(QString::fromUtf8("dialog-warning"));
            case NELogging::eLogPriority::PrioError:
            case NELogging::eLogPriority::PrioFatal:
                return QIcon::fromTheme(QString::fromUtf8("dialog-error"));
            default:
                return QIcon::fromTheme(QString::fromUtf8("window-close"));
            }
        }        
    }
    break;
        
    case Qt::ItemDataRole::ForegroundRole:
    {
        int row {index.row()};
        if (row >= mLogs.size())
            return QVariant();
        
        const SharedBuffer data{mLogs.at(row)};
        const NELogging::sLogMessage* logMessage = reinterpret_cast<const NELogging::sLogMessage*>(data.getBuffer());
        if (logMessage != nullptr)
        {
            switch (logMessage->logMessagePrio)
            {
            case NELogging::eLogPriority::PrioScope:
                return QBrush(LogColors[static_cast<int>(PrioScope)]);
            case NELogging::eLogPriority::PrioDebug:
                return QBrush(LogColors[static_cast<int>(PrioDebug)]);
            case NELogging::eLogPriority::PrioInfo:
                return QBrush(LogColors[static_cast<int>(PrioInfo)]);
            case NELogging::eLogPriority::PrioWarning:
                return QBrush(LogColors[static_cast<int>(PrioWarn)]);
            case NELogging::eLogPriority::PrioError:
                return QBrush(LogColors[static_cast<int>(PrioError)]);
            case NELogging::eLogPriority::PrioFatal:
                return QBrush(LogColors[static_cast<int>(PrioFatal)]);
            default:
                return QBrush(LogColors[static_cast<int>(PrioDefault)]);
            }
        }
    }
    break;
    
    case Qt::ItemDataRole::UserRole:
    {
        int row {index.row()};
        if (row >= mLogs.size())
            return QVariant();
        
        const SharedBuffer data{mLogs.at(row)};
        const NELogging::sLogMessage* logMessage = reinterpret_cast<const NELogging::sLogMessage*>(data.getBuffer());
        return QVariant::fromValue(logMessage);
    }
    
    default:
        return QVariant();
    }

    return QVariant();
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

bool LogViewerModel::connectService(const QString& hostName /*= ""*/, unsigned short portNr /*= 0u*/)
{
    return false;
}

void LogViewerModel::disconnectService(void)
{
}

void LogViewerModel::serviceConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath)
{
    mIsConnected = isConnected;
    mAddress     = address;
    mPort        = port;
    mDbPath      = dbPath;

    LogObserver* log = LogObserver::getComponent();
    Q_ASSERT(log != nullptr);
    if (isConnected)
    {
        connect(log, &LogObserver::signalLogMessage, this, &LogViewerModel::slotLogMessage);
    }
    else
    {
        disconnect(log, &LogObserver::signalLogMessage, this, &LogViewerModel::slotLogMessage);
    }
}

void LogViewerModel::slotLogMessage(const SharedBuffer& logMessage)
{
    if (logMessage.isEmpty() == false)
    {
        beginInsertRows(QModelIndex(), static_cast<int>(mLogs.size()), static_cast<int>(mLogs.size()));
        mLogs.append(logMessage);
        endInsertRows();
    }
}
