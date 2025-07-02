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
 *  \file        lusan/model/log/LogOfflineModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Offline Log Navigation Model.
 *
 ************************************************************************/

#include "lusan/model/log/LogOfflineModel.hpp"
#include "lusan/model/log/LogScopeIconFactory.hpp"
#include "lusan/model/log/LogViewerFilterProxy.hpp"

#include "areg/base/DateTime.hpp"
#include "areg/base/SharedBuffer.hpp"
#include "areg/base/File.hpp"
#include "areg/logging/NELogging.hpp"

#include <QBrush>
#include <QColor>
#include <QIcon>
#include <QSize>
#include <QFileInfo>

const QStringList& LogOfflineModel::getHeaderList(void)
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

const QList<int>& LogOfflineModel::getHeaderWidths(void)
{
    static QList<int> _widths
        {
              65   // Priority
            , 165  // Timestamp
            , 150  // Source
            , 80   // Source ID
            , 120  // Thread
            , 80   // Thread ID
            , 80   // Scope ID
            , 400  // Message
        };
    
    return _widths;
}

const QList<LogOfflineModel::eColumn>& LogOfflineModel::getDefaultColumns(void)
{
    static QList<LogOfflineModel::eColumn> _defaultColumns
        {
              LogOfflineModel::eColumn::LogColumnPriority
            , LogOfflineModel::eColumn::LogColumnTimestamp
            , LogOfflineModel::eColumn::LogColumnSource
            , LogOfflineModel::eColumn::LogColumnThread
            , LogOfflineModel::eColumn::LogColumnMessage
        };
    
    return _defaultColumns;
}

QString LogOfflineModel::getFileExtension()
{
    return QString("sqlog");
}

LogOfflineModel::LogOfflineModel(QObject *parent)
    : QAbstractTableModel(parent)
    , mDatabase ( )
    , mActiveColumns(LogOfflineModel::getDefaultColumns())
    , mLogs     ( )
    , mFilter   (new LogViewerFilterProxy(this))
{
}

LogOfflineModel::~LogOfflineModel()
{
    closeDatabase();
}

QVariant LogOfflineModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            if ((section >= 0) && (section < mActiveColumns.size()))
            {
                const QStringList& headers = LogOfflineModel::getHeaderList();
                eColumn col = mActiveColumns.at(section);
                return headers.at(static_cast<int>(col));
            }
        }
        else if (role == Qt::SizeHintRole)
        {
            if ((section >= 0) && (section < mActiveColumns.size()))
            {
                const QList<int>& widths = LogOfflineModel::getHeaderWidths();
                eColumn col = mActiveColumns.at(section);
                return QSize(widths.at(static_cast<int>(col)), 22);
            }
        }
        else if (role == Qt::ToolTipRole)
        {
            return headerData(section, orientation, Qt::DisplayRole);
        }
    }
    
    return QVariant();
}

int LogOfflineModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(mLogs.size());
}

int LogOfflineModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(mActiveColumns.size());
}

QVariant LogOfflineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    int row = index.row();
    int col = index.column();
    
    if ((row < 0) || (row >= static_cast<int>(mLogs.size())))
        return QVariant();
    
    if ((col < 0) || (col >= static_cast<int>(mActiveColumns.size())))
        return QVariant();
    
    const SharedBuffer& logData = mLogs.at(row);
    const NELogging::sLogMessage* logMessage = reinterpret_cast<const NELogging::sLogMessage*>(logData.getBuffer());
    if (logMessage == nullptr)
        return QVariant();
    
    eColumn column = mActiveColumns.at(col);
    
    switch (role)
    {
    case Qt::DisplayRole:
        return _getDisplayData(logMessage, column);
        
    case Qt::BackgroundRole:
        return _getBackgroundData(logMessage, column);
        
    case Qt::ForegroundRole:
        return _getForegroundData(logMessage, column);
        
    case Qt::DecorationRole:
        return _getDecorationData(logMessage, column);
        
    case Qt::TextAlignmentRole:
        return _getAlignmentData(column);
        
    case Qt::UserRole:
        return QVariant::fromValue(logMessage);
        
    default:
        return QVariant();
    }
}

bool LogOfflineModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // Note: This model is read-only for offline viewing
    endInsertRows();
    return true;
}

bool LogOfflineModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    endInsertColumns();
    return true;
}

bool LogOfflineModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // Note: This model is read-only for offline viewing
    endRemoveRows();
    return true;
}

bool LogOfflineModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // Note: This model is read-only for offline viewing
    endRemoveColumns();
    return true;
}

QString LogOfflineModel::getHeaderName(int colIndex) const
{
    if ((colIndex >= 0) && (colIndex < mActiveColumns.size()))
    {
        eColumn col = mActiveColumns.at(colIndex);
        const QStringList& header = LogOfflineModel::getHeaderList();
        return header.at(static_cast<int>(col));
    }
    else
    {
        return QString();
    }
}

bool LogOfflineModel::openDatabase(const QString& filePath)
{
    _closeDatabase(); // Close any existing database
    
    if (filePath.isEmpty())
    {
        return false;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile())
    {
        return false;
    }
    
    if (mDatabase.connect(filePath.toStdString()))
    {
        emit signalDatabaseIsOpened(QString::fromStdString(mDatabase.getDatabasePath().getData()));
        // Load initial log messages for display
        beginResetModel();
        mLogs.clear();
        
        std::vector<SharedBuffer> messages;
        getLogMessages(messages);
        
        for (const auto& msg : messages)
        {
            mLogs.append(msg);
        }
        
        endResetModel();
        emit signalLogsAvailable();
        return true;
    }
    else
    {
        return false;
    }
}

void LogOfflineModel::closeDatabase(void)
{
    _closeDatabase();
    emit signalDatabaseIsClosed(QString::fromStdString(mDatabase.getDatabasePath().getData()));
}

void LogOfflineModel::addColumn(LogOfflineModel::eColumn col, int pos /*= -1*/)
{
    if (mActiveColumns.contains(col))
        return; // Column already exists
    
    if ((pos < 0) || (pos >= mActiveColumns.size()))
    {
        // Add before message column or at the end
        pos = mActiveColumns.indexOf(eColumn::LogColumnMessage);
        if (pos < 0)
        {
            pos = mActiveColumns.size();
        }
    }
    
    beginInsertColumns(QModelIndex(), pos, pos);
    mActiveColumns.insert(pos, col);
    endInsertColumns();
}

void LogOfflineModel::removeColumn(LogOfflineModel::eColumn col)
{
    int pos = mActiveColumns.indexOf(col);
    if (pos >= 0)
    {
        beginRemoveColumns(QModelIndex(), pos, pos);
        mActiveColumns.removeAt(pos);
        endRemoveColumns();
    }
}

void LogOfflineModel::setActiveColumns(const QList<LogOfflineModel::eColumn>& columns)
{
    beginResetModel();
    if (columns.isEmpty())
    {
        mActiveColumns = LogOfflineModel::getDefaultColumns();
    }
    else
    {
        mActiveColumns = columns;
    }
    
    endResetModel();
}

void LogOfflineModel::getLogInstanceNames(std::vector<String>& names)
{
    mDatabase.getLogInstanceNames(names);
}

void LogOfflineModel::getLogInstances(std::vector<ITEM_ID>& ids)
{
    mDatabase.getLogInstances(ids);
}

void LogOfflineModel::getLogThreadNames(std::vector<String>& names)
{
    mDatabase.getLogThreadNames(names);
}

void LogOfflineModel::getLogThreads(std::vector<ITEM_ID>& ids)
{
    mDatabase.getLogThreads(ids);
}

void LogOfflineModel::getPriorityNames(std::vector<String>& names)
{
    mDatabase.getPriorityNames(names);
}

void LogOfflineModel::getLogInstanceInfos(std::vector<NEService::sServiceConnectedInstance>& infos)
{
    mDatabase.getLogInstanceInfos(infos);
}

void LogOfflineModel::getLogInstScopes(std::vector<NELogging::sScopeInfo>& scopes, ITEM_ID instId)
{
    mDatabase.getLogInstScopes(scopes, instId);
}

void LogOfflineModel::getLogMessages(std::vector<SharedBuffer>& messages)
{
    mDatabase.getLogMessages(messages);
}

void LogOfflineModel::getLogInstMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId /*= NEService::COOKIE_ANY*/)
{
    mDatabase.getLogInstMessages(messages, instId);
}

void LogOfflineModel::getLogScopeMessages(std::vector<SharedBuffer>& messages, uint32_t scopeId /*= 0*/)
{
    mDatabase.getLogScopeMessages(messages, scopeId);
}

void LogOfflineModel::getLogMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId, uint32_t scopeId)
{
    mDatabase.getLogMessages(messages, instId, scopeId);
}

//////////////////////////////////////////////////////////////////////////
// Private helper methods
//////////////////////////////////////////////////////////////////////////

inline void LogOfflineModel::_closeDatabase(void)
{
    mDatabase.disconnect();
    dataReset();
}

QVariant LogOfflineModel::_getDisplayData(const NELogging::sLogMessage* logMessage, eColumn column) const
{
    if (logMessage == nullptr)
        return QVariant();
    
    switch (column)
    {
    case eColumn::LogColumnPriority:
        return QString::fromStdString(NELogging::logPrioToString(static_cast<NELogging::eLogPriority>(logMessage->logMessagePrio)).getData());
        
    case eColumn::LogColumnTimestamp:
    {
        DateTime dt(logMessage->logTimestamp);
        return QString::fromStdString(dt.formatTime().getData());
    }
        
    case eColumn::LogColumnSource:
        return QString(logMessage->logModule);
        
    case eColumn::LogColumnSourceId:
        return QString::number(logMessage->logCookie);
        
    case eColumn::LogColumnThread:
        return QString(logMessage->logThread);
        
    case eColumn::LogColumnThreadId:
        return QString::number(logMessage->logThreadId);
        
    case eColumn::LogColumnScopeId:
        return QString::number(logMessage->logScopeId);
        
    case eColumn::LogColumnMessage:
        return QString(logMessage->logMessage);
        
    default:
        return QVariant();
    }
}

QVariant LogOfflineModel::_getBackgroundData(const NELogging::sLogMessage* logMessage, eColumn column) const
{
    Q_UNUSED(column)
    
    if (logMessage == nullptr)
        return QVariant();
    
    NELogging::eLogPriority prio = static_cast<NELogging::eLogPriority>(logMessage->logMessagePrio);
    switch (prio)
    {
    case NELogging::eLogPriority::PrioFatal:
        return QBrush(QColor(220, 20, 60));  // Crimson
        
    case NELogging::eLogPriority::PrioError:
        return QBrush(QColor(255, 99, 71));  // Tomato
        
    case NELogging::eLogPriority::PrioWarning:
        return QBrush(QColor(255, 215, 0));  // Gold
        
    default:
        return QVariant();
    }
}

QVariant LogOfflineModel::_getForegroundData(const NELogging::sLogMessage* logMessage, eColumn column) const
{
    Q_UNUSED(column)
    
    if (logMessage == nullptr)
        return QVariant();
    
    NELogging::eLogPriority prio = static_cast<NELogging::eLogPriority>(logMessage->logMessagePrio);
    switch (prio)
    {
    case NELogging::eLogPriority::PrioFatal:
    case NELogging::eLogPriority::PrioError:
        return QBrush(QColor(Qt::white));
        
    case NELogging::eLogPriority::PrioWarning:
        return QBrush(QColor(Qt::black));
        
    default:
        return QVariant();
    }
}

QVariant LogOfflineModel::_getDecorationData(const NELogging::sLogMessage* logMessage, eColumn column) const
{
    if (logMessage == nullptr || column != eColumn::LogColumnPriority)
        return QVariant();
    
    NELogging::eLogPriority prio = static_cast<NELogging::eLogPriority>(logMessage->logMessagePrio);
    return LogScopeIconFactory::getIcon(prio);
}

QVariant LogOfflineModel::_getAlignmentData(eColumn column) const
{
    switch (column)
    {
    case eColumn::LogColumnPriority:
    case eColumn::LogColumnSourceId:
    case eColumn::LogColumnThreadId:
    case eColumn::LogColumnScopeId:
        return static_cast<int>(Qt::AlignCenter | Qt::AlignVCenter);
        
    case eColumn::LogColumnTimestamp:
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        
    default:
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }
}
