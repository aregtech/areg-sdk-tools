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
#include "areg/base/File.hpp"
#include "areg/logging/NELogging.hpp"
#include "lusan/model/log/LogScopeIconFactory.hpp"
#include "lusan/model/log/LogViewerFilterProxy.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/WorkspaceEntry.hpp"
#include "lusan/data/log/LogObserver.hpp"

#include <QBrush>
#include <QColor>
#include <QIcon>
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

const QList<int>& LogViewerModel::getHeaderWidths(void)
{
    static QList<int>  _widths { 50, 100, 100, 50, 100, 50, 50, 200 };
    return _widths;
}

const QList<LogViewerModel::eColumn>& LogViewerModel::getDefaultColumns(void)
{
    static QList<LogViewerModel::eColumn>   _columnIds
    {
          eColumn::LogColumnSourceId
        , eColumn::LogColumnPriority
        , eColumn::LogColumnScopeId
        , eColumn::LogColumnTimestamp
        , eColumn::LogColumnMessage
    };
    
    return _columnIds;
}

QString LogViewerModel::getFileExtension()
{
    static QString _fileExtension = QStringLiteral(".sqlog");
    return _fileExtension;
}

QString LogViewerModel::generateFileName(void)
{
    QString result{ "log_%time%.sqlog" };

    LogObserver::Component* logObserver = LogObserver::getComponent();
    if (logObserver != nullptr)
    {
        QString fileName = LogObserver::getConfigDatabaseName();
        if (fileName.isEmpty() == false)
        {
            result = fileName;
        }
    }

    return QString(File::normalizePath(result.toStdString().c_str()).getString());
}

QString LogViewerModel::newFileName(void)
{
    QString result;

    WorkspaceEntry workspace = LusanApplication::getActiveWorkspace();
    QString dir = workspace.getDirLogs();
    if (dir.isEmpty())
    {
        dir = LogObserver::getConfigDatabaseLocation();
    }

    if (dir.isEmpty() == false)
    {
        dir = File::normalizePath(dir.toStdString().c_str()).getString();
        QString fileName = generateFileName();

        std::filesystem::path fPath(dir.toStdString().c_str());
        fPath /= fileName.toStdString().c_str();
        result = QString(fPath.c_str());
    }

    return result;
}

LogViewerModel::LogViewerModel(QObject *parent)
    : QAbstractTableModel(parent)

    , mIsConnected(false)
    , mAddress()
    , mPort(NESocket::InvalidPort)
    , mDbPath()

    , mActiveColumns( LogViewerModel::getDefaultColumns() )
    , mLogs         ( )
    , mConLogger    ( )
    , mConLogs      ( )
    , mFilter       (new LogViewerFilterProxy(this))
{
}

QVariant LogViewerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Orientation::Vertical) || (section < 0) || (section >= static_cast<int>(mActiveColumns.size())))
        return QVariant();

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
        const QList<int>& widths = getHeaderWidths();
        eColumn col = mActiveColumns.at(section);
        return (static_cast<int>(col) < static_cast<int>(widths.size()) ? QVariant(QSize(widths[static_cast<int>(col)], 28)) : QVariant());
    }
    
    return QVariant();
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
            eColumn col = static_cast<eColumn>(mActiveColumns.at(index.column()));
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
                return QVariant(QString(logMessage->logModule) + " (" + QString::number(logMessage->logCookie) + ")");
            case eColumn::LogColumnSourceId:
                return QVariant((qulonglong)logMessage->logCookie);
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
        if ((logMessage != nullptr) && (static_cast<eColumn>(mActiveColumns.at(index.column())) == eColumn::LogColumnPriority))
        {
            switch (logMessage->logMessagePrio)
            {
            case NELogging::eLogPriority::PrioScope:
                if (logMessage->logMsgType == NELogging::eLogMessageType::LogMessageScopeEnter)
                    return LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioScopeEnter, true);
                else if (logMessage->logMsgType == NELogging::eLogMessageType::LogMessageScopeExit)
                    return LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioScopeExit, true);
                else
                    return LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioScope, true);
            case NELogging::eLogPriority::PrioDebug:
                return LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioDebug, true);
            case NELogging::eLogPriority::PrioInfo:
                return LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioInfo, true);
            case NELogging::eLogPriority::PrioWarning:
                return LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioWarn, true);
            case NELogging::eLogPriority::PrioError:
                return LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioError, true);
            case NELogging::eLogPriority::PrioFatal:
                return LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioFatal, true);
            default:
                return LogScopeIconFactory::getLogIcon(LogScopeIconFactory::eLogIcons::PrioNotset, false);
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
            return LogScopeIconFactory::getLogColor(*logMessage);
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

    disconnect(mConLogger);
    disconnect(mConLogs);
    if (isConnected)
    {
        LogObserver* log = LogObserver::getComponent();
        Q_ASSERT(log != nullptr);
        mConLogger = connect(log, &LogObserver::signalLogMessage         , this, &LogViewerModel::slotLogMessage);
        mConLogs   = connect(log, &LogObserver::signalLogServiceConnected, this, &LogViewerModel::slotLogServiceConnected);
    }
}

void LogViewerModel::addColumn(LogViewerModel::eColumn col, int pos /*= -1*/)
{
    int found = findColumn(col);
    if (found == -1)
    {
        pos = (pos >= 0) && (pos < static_cast<int>(mActiveColumns.size())) ? pos : mActiveColumns.size() - 1;
        beginInsertColumns(QModelIndex(), pos, pos);
        mActiveColumns.insert(pos, col);        
        endInsertColumns();
    }
}

void LogViewerModel::removeColumn(LogViewerModel::eColumn col)
{
    int found = findColumn(col);
    if (found >= 0)
    {
        Q_ASSERT(found < static_cast<int>(mActiveColumns.size()));
        beginRemoveColumns(QModelIndex(), found, found);
        mActiveColumns.remove(found, 1);
        endRemoveColumns();
    }
}

void LogViewerModel::setActiveColumns(const QList<LogViewerModel::eColumn>& columns)
{
    const QList<LogViewerModel::eColumn>& cols{columns.empty() ? getDefaultColumns() : columns};
    
    beginResetModel();
    mActiveColumns = cols;
    endResetModel();
}

void LogViewerModel::pauseLogging(void)
{
    LogObserver::pause();
}

void LogViewerModel::resumeLogging(void)
{
    LogObserver::resume();
}

void LogViewerModel::stopLogging(void)
{
    LogObserver::stop();
}

void LogViewerModel::restartLogging(const QString& dbName /*= QString()*/)
{
    beginResetModel();
    mLogs.clear();
    LogObserver::restart(dbName);
    endResetModel();
}

void LogViewerModel::slotLogServiceConnected(bool isConnected, const QString& address, uint16_t port)
{
    if (isConnected == false)
    {
        mIsConnected = false;
        disconnect(mConLogger);
        disconnect(mConLogs);
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

void LogViewerModel::getLogInstanceNames(std::vector<String>& names)
{
    LogObserver::queryLogInstanceNames(names);
}

void LogViewerModel::getLogInstances(std::vector<ITEM_ID>& ids)
{
    LogObserver::queryLogInstances(ids);
}

void LogViewerModel::getLogThreadNames(std::vector<String>& names)
{
    LogObserver::queryLogThreadNames(names);
}

void LogViewerModel::getLogThreads(std::vector<ITEM_ID>& ids)
{
    LogObserver::queryLogThreads(ids);
}

void LogViewerModel::getPriorityNames(std::vector<String>& names)
{
    LogObserver::queryPriorityNames(names);
}

void LogViewerModel::getLogInstanceInfos(std::vector< NEService::sServiceConnectedInstance>& infos)
{
    LogObserver::queryLogInstanceInfos(infos);
}

void LogViewerModel::getLogInstScopes(std::vector<NELogging::sScopeInfo>& scopes, ITEM_ID instId)
{
    LogObserver::queryLogInstScopes(scopes, instId);
}

void LogViewerModel::getLogMessages(std::vector<SharedBuffer>& messages)
{
    LogObserver::queryLogMessages(messages);
}

void LogViewerModel::getLogInstMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId /*= NEService::COOKIE_ANY*/)
{
    LogObserver::queryLogInstMessages(messages, instId);
}

void LogViewerModel::getLogScopeMessages(std::vector<SharedBuffer>& messages, uint32_t scopeId /*= 0*/)
{
    LogObserver::queryLogScopeMessages(messages, scopeId);
}

void LogViewerModel::getLogMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId, uint32_t scopeId)
{
    LogObserver::queryLogMessages(messages, instId, scopeId);
}
