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
 *  \file        lusan/model/log/LoggingModelBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Logging model base class.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"
#include "lusan/model/log/LogIconFactory.hpp"

#include "areg/base/DateTime.hpp"

#include <QBrush>
#include <QColor>
#include <QIcon>
#include <QSize>

const QStringList& LoggingModelBase::getHeaderList(void)
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

const QList<int>& LoggingModelBase::getHeaderWidths(void)
{
    static QList<int>  _widths{ 50, 100, 100, 50, 100, 50, 50, 200 };
    return _widths;
}

const QList<LoggingModelBase::eColumn>& LoggingModelBase::getDefaultColumns(void)
{
    static QList<LoggingModelBase::eColumn>   _columnIds
    {
          eColumn::LogColumnSourceId
        , eColumn::LogColumnPriority
        , eColumn::LogColumnScopeId
        , eColumn::LogColumnTimestamp
        , eColumn::LogColumnMessage
    };

    return _columnIds;
}

const QString & LoggingModelBase::getFileExtension()
{
    static QString _fileExtension = QStringLiteral("sqlog");
    return _fileExtension;
}

LoggingModelBase::LoggingModelBase(LoggingModelBase::eLogging logsType, QObject* parent)
    : TableModelBase (parent)
    , mLoggingType  (logsType)
    , mDatabase     ( )
    , mStatement    (mDatabase.getDatabase())
    , mActiveColumns(getDefaultColumns())
    , mRootList     ( )
    , mLogs         ( )
    , mInstances    ( )
    , mSelectedScope( )
    , mSelectedLog  ( )
    , mScopes       ( )
    , mLogChunk     (-1)
    , mLogCount     (0)
    , mReadThread   (static_cast<IEThreadConsumer &>(self()), "_LogReadingThread_")
    , mQuitThread   (false)
{
}

LoggingModelBase::~LoggingModelBase(void)
{
    _cleanNodes();
}

QVariant LoggingModelBase::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Orientation::Vertical) || (section < 0) || (section >= static_cast<int>(mActiveColumns.size())))
        return QVariant();

    if (static_cast<Qt::ItemDataRole>(role) == Qt::ItemDataRole::DisplayRole)
    {
        return QVariant(getHeaderName(section));
    }
    else if (static_cast<Qt::ItemDataRole>(role) == Qt::ItemDataRole::UserRole)
    {
        return QVariant(static_cast<int>(mActiveColumns.at(section)));
    }
    else if (static_cast<Qt::ItemDataRole>(role) == Qt::ItemDataRole::SizeHintRole)
    {
        const QList<int>& widths = getHeaderWidths();
        eColumn col = mActiveColumns.at(section);
        return (static_cast<int>(col) < static_cast<int>(widths.size()) ? QVariant(QSize(widths[static_cast<int>(col)], 28)) : QVariant());
    }

    return QVariant();
}

int LoggingModelBase::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mLogCount;
}

int LoggingModelBase::columnCount(const QModelIndex& parent) const
{
    return static_cast<int>(mActiveColumns.size());
}

bool LoggingModelBase::insertRows(int row, int count, const QModelIndex& parent)
{
    Q_UNUSED(row)
    Q_UNUSED(count)
    Q_UNUSED(parent)
    return false;
}

bool LoggingModelBase::insertColumns(int column, int count, const QModelIndex& parent)
{
    Q_UNUSED(column)
    Q_UNUSED(count)
    Q_UNUSED(parent)
    return false;
}

bool LoggingModelBase::removeRows(int row, int count, const QModelIndex& parent)
{
    Q_UNUSED(row)
    Q_UNUSED(count)
    Q_UNUSED(parent)
    return false;
}

bool LoggingModelBase::removeColumns(int column, int count, const QModelIndex& parent)
{
    Q_UNUSED(column)
    Q_UNUSED(count)
    Q_UNUSED(parent)
    return false;
}

QVariant LoggingModelBase::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();

    if ((row < 0) || (row >= static_cast<int>(mLogCount)))
        return QVariant();

    if ((col < 0) || (col >= static_cast<int>(mActiveColumns.size())))
        return QVariant();
    
    const SharedBuffer & logData {mLogs[row]};
    Q_ASSERT(logData.isValid());
    const NELogging::sLogMessage* logMessage = reinterpret_cast<const NELogging::sLogMessage*>(logData.getBuffer());
    if (logMessage == nullptr)
        return QVariant();
    
    eColumn column = mActiveColumns.at(col);
    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::DisplayRole:
        return getDisplayData(logMessage, column);
        
    case Qt::BackgroundRole:
        return getBackgroundData(logMessage, column);
        
    case Qt::ForegroundRole:
        return getForegroundData(logMessage, column);
        
    case Qt::DecorationRole:
        return getDecorationData(logMessage, column);
        
    case Qt::TextAlignmentRole:
        return getAlignmentData(column);
        
    case Qt::UserRole:
        return QVariant::fromValue(logMessage);
        
    default:
        return QVariant();
    }
}

QString LoggingModelBase::getHeaderName(int colIndex) const
{
    if ((colIndex >= 0) && (colIndex < mActiveColumns.size()))
    {
        eColumn col = mActiveColumns.at(colIndex);
        const QStringList& header = getHeaderList();
        return header.at(static_cast<int>(col));
    }
    else
    {
        return QString();
    }
}

void LoggingModelBase::addColumn(LoggingModelBase::eColumn col, int pos /*= -1*/)
{
    if (mActiveColumns.contains(col) == false)
    {
        pos = (pos >= 0) && (pos < static_cast<int>(mActiveColumns.size())) ? pos : mActiveColumns.size() - 1;
        beginInsertColumns(QModelIndex(), pos, pos);
        mActiveColumns.insert(pos, col);
        endInsertColumns();
    }
}

void LoggingModelBase::removeColumn(LoggingModelBase::eColumn col)
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

void LoggingModelBase::setActiveColumns(const QList<LoggingModelBase::eColumn>& columns)
{
    const QList<LoggingModelBase::eColumn>& cols{ columns.empty() ? getDefaultColumns() : columns };

    beginResetModel();
    mActiveColumns = cols;
    endResetModel();
}

void LoggingModelBase::refresh(void)
{
    beginResetModel();
    endResetModel();
}

void LoggingModelBase::setupModel(void)
{
}

void LoggingModelBase::releaseModel(void)
{
}

void LoggingModelBase::openDatabase(const QString& dbPath, bool readOnly)
{
    std::string path(File::normalizePath(dbPath.toStdString().c_str()));
    if (mDatabase.getDatabasePath() != path)
    {
        mDatabase.connect(path, readOnly);
    }
}

QString LoggingModelBase::getDatabasePath(void) const
{
    return QString::fromStdString(mDatabase.getDatabasePath().getData());
}

void LoggingModelBase::closeDatabase(void)
{
    _closeDatabase();
}

bool LoggingModelBase::isOperable(void) const
{
    return mDatabase.isOperable();
}

void LoggingModelBase::getLogInstanceNames(std::vector<String>& names)
{
    const std::vector< NEService::sServiceConnectedInstance> & instances{getLogInstances()};
    names.clear();
    for (const auto& instance : instances)
    {
        names.push_back(instance.ciInstance);
    }
}

void LoggingModelBase::getLogInstanceIds(std::vector<ITEM_ID>& ids)
{
    const std::vector< NEService::sServiceConnectedInstance> & instances{getLogInstances()};
    ids.clear();
    for (const auto& instance : instances)
    {
        ids.push_back(instance.ciCookie);
    }
}

void LoggingModelBase::getLogThreadNames(std::vector<String>& names)
{
    mDatabase.getLogThreadNames(names);
}

void LoggingModelBase::getLogThreads(std::vector<ITEM_ID>& ids)
{
    mDatabase.getLogThreads(ids);
}

void LoggingModelBase::getPriorityNames(std::vector<String>& names)
{
    mDatabase.getPriorityNames(names);
}

const std::vector< NEService::sServiceConnectedInstance> & LoggingModelBase::getLogInstances(void)
{
    if (isOfflineLogging() && mInstances.empty())
    {
        mDatabase.getLogInstanceInfos(mInstances);
    }

    return mInstances;
}

const std::vector<NELogging::sScopeInfo> & LoggingModelBase::getLogInstScopes(ITEM_ID instId)
{
    static std::vector<NELogging::sScopeInfo> _dummy;
    if (isOfflineLogging() && mScopes.empty())
    {
        mDatabase.getLogInstScopes(mScopes[instId], instId);
    }
    
    return (mScopes.find(instId) != mScopes.end() ? mScopes.at(instId) : _dummy); 
}

const std::vector<SharedBuffer>& LoggingModelBase::getLogMessages()
{
    if (isOfflineLogging() && (mLogCount == 0))
    {
        mDatabase.getLogMessages(mLogs);
    }

    return mLogs;
}

void LoggingModelBase::getLogInstMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId /*= NEService::COOKIE_ANY*/)
{
    mDatabase.getLogInstMessages(messages, instId);
}

void LoggingModelBase::getLogScopeMessages(std::vector<SharedBuffer>& messages, uint32_t scopeId /*= 0*/)
{
    mDatabase.getLogScopeMessages(messages, scopeId);
}

void LoggingModelBase::getLogMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId, uint32_t scopeId)
{
    mDatabase.getLogMessages(messages, instId, scopeId);
}

int LoggingModelBase::findInstanceEntry(ITEM_ID instId)
{
    int result{ NECommon::INVALID_INDEX };
    const std::vector<NEService::sServiceConnectedInstance> & instances = getLogInstances();
    for (int i = 0; i < static_cast<int>(instances.size()); ++ i)
    {
        if (instances[i].ciCookie == instId)
        {
            result = i;
            break;
        }
    }

    return result;
}

const NEService::sServiceConnectedInstance& LoggingModelBase::getInstanceEntry(ITEM_ID instId)
{
    static const NEService::sServiceConnectedInstance _instInvalid;
    int pos = findInstanceEntry(instId);
    const std::vector<NEService::sServiceConnectedInstance>& instances = getLogInstances();
    return (pos != NECommon::INVALID_INDEX ? instances[pos] : _instInvalid);
}

bool LoggingModelBase::addInstanceEntry(const NEService::sServiceConnectedInstance& instance, bool unique)
{
    bool result{ false };
    int pos = findInstanceEntry(instance.ciCookie);
    if ((pos == NECommon::INVALID_INDEX) || (unique == false))
    {
        mInstances.push_back(instance);
        result = true;
    }
    else
    {
        mInstances[pos] = instance;
    }

    return result;
}

int LoggingModelBase::removeInstanceEntry(ITEM_ID instId)
{
    int result{ NECommon::INVALID_INDEX };
    for (int i = 0; i < static_cast<int>(mInstances.size()); ++i)
    {
        if (mInstances[i].ciCookie == instId)
        {
            result = i;
            mInstances.erase(mInstances.begin() + i);
            break;
        }
    }

    return result;
}

int LoggingModelBase::addInstances(const std::vector<NEService::sServiceConnectedInstance>& instances, bool unique)
{
    int result{ 0 };
    for (const auto& instance : instances)
    {
        result += addInstanceEntry(instance, unique) ? 1 : 0;
    }

    return result;
}

int LoggingModelBase::removeInstances(const std::vector<NEService::sServiceConnectedInstance>& instances)
{
    int result{ 0 };
    for (const auto& instance : instances)
    {
        result += removeInstanceEntry(instance.ciCookie) != NECommon::INVALID_INDEX ? 1 : 0;
    }

    return result;
}

void LoggingModelBase::dataTransfer(LoggingModelBase& logModel)
{
    mActiveColumns.clear();
    mActiveColumns = std::move(logModel.mActiveColumns);
    logModel.mActiveColumns.clear();

    mLogs.clear();
    mLogs = std::move(logModel.mLogs);
    mLogChunk = logModel.mLogChunk;
    mLogCount = logModel.mLogCount;
    logModel.mLogs.clear();
    logModel.mLogCount = 0;

    mInstances.clear();
    mInstances = std::move(logModel.mInstances);
    logModel.mInstances.clear();

    mScopes.clear();
    mScopes = std::move(logModel.mScopes);
    logModel.mScopes.clear();

    _cleanNodes();
    mRootList = std::move(logModel.mRootList);
    logModel._cleanNodes();

    mSelectedScope = std::move(logModel.mSelectedScope);
    logModel.mSelectedScope = QModelIndex();

    mSelectedLog = std::move(logModel.mSelectedLog);
    logModel.mSelectedLog = QModelIndex();

    mDatabase.disconnect();
    if (logModel.mDatabase.isOperable())
    {
        mDatabase.connect(logModel.mDatabase.getDatabasePath(), true);
    }
    logModel.mDatabase.disconnect();
}

void LoggingModelBase::readLogsAsynchronous(int maxEntries)
{
    _quitThread();
    beginResetModel();
    mLogs.clear();
    mLogCount = 0;
    endResetModel();
    mLogChunk = maxEntries;
    mReadThread.createThread(NECommon::DO_NOT_WAIT);
}

QString LoggingModelBase::getDisplayData(const NELogging::sLogMessage* logMessage, eColumn column) const
{
    Q_ASSERT(logMessage != nullptr);

    switch (column)
    {
    case eColumn::LogColumnPriority:
        return QString::fromStdString(NELogging::logPrioToString(logMessage->logMessagePrio).getData());

    case eColumn::LogColumnTimestamp:
        return QString::fromStdString(DateTime(logMessage->logTimestamp).formatTime().getData());

    case eColumn::LogColumnSource:
        return QString(logMessage->logModule) + " (" + QString::number(logMessage->logCookie) + ")";

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
        return QString();
    }
}

QBrush LoggingModelBase::getBackgroundData(const NELogging::sLogMessage* logMessage, eColumn column) const
{
    Q_UNUSED(column)
    Q_ASSERT(logMessage != nullptr);
    return QBrush(LogIconFactory::getLogBackgroundColor(*logMessage));
}

QColor LoggingModelBase::getForegroundData(const NELogging::sLogMessage* logMessage, eColumn column) const
{
    Q_UNUSED(column)
    Q_ASSERT(logMessage != nullptr);
    return LogIconFactory::getLogColor(*logMessage);
}

QIcon LoggingModelBase::getDecorationData(const NELogging::sLogMessage* logMessage, eColumn column) const
{
    Q_ASSERT(logMessage != nullptr);
    if (column != eColumn::LogColumnPriority)
        return QIcon();

    switch (logMessage->logMessagePrio)
    {
    case NELogging::eLogPriority::PrioScope:
        if (logMessage->logMsgType == NELogging::eLogMessageType::LogMessageScopeEnter)
            return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioScopeEnter, true);
        else if (logMessage->logMsgType == NELogging::eLogMessageType::LogMessageScopeExit)
            return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioScopeExit, true);
        else
            return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioScope, true);
    case NELogging::eLogPriority::PrioDebug:
        return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioDebug, true);
    case NELogging::eLogPriority::PrioInfo:
        return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioInfo, true);
    case NELogging::eLogPriority::PrioWarning:
        return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioWarn, true);
    case NELogging::eLogPriority::PrioError:
        return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioError, true);
    case NELogging::eLogPriority::PrioFatal:
        return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioFatal, true);
    default:
        return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioNotset, false);
    }
}

int LoggingModelBase::getAlignmentData(eColumn column) const
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

void LoggingModelBase::onThreadRuns(void)
{
    uint32_t    nextStart   { 0 };
    int         readCount   { 0 };
    
    if (mDatabase.setupStatementReadLogs(mStatement, NEService::TARGET_ALL) == false)
        return;

    uint32_t count = mDatabase.countLogEntries();
    if (count == 0)
        return;
    
    Q_ASSERT(mLogCount == 0);
    mLogs.resize(count);

    do
    {
        readCount = -1;
        if (mQuitThread.tryLock() == false)
            break;
        
        mQuitThread.unlock();
        readCount   = mDatabase.fillLogMessages(mLogs, mStatement, nextStart, mLogChunk);
        if (readCount != 0)
        {
            beginInsertRows(QModelIndex(), static_cast<int>(nextStart), static_cast<int>(nextStart) + readCount - 1);
            nextStart += static_cast<uint32_t>(readCount);
            mLogCount = nextStart;
            endInsertRows();
        }
    } while ((readCount > 0) && (readCount == mLogChunk));
    
    Q_ASSERT((mLogCount == static_cast<int>(mLogs.size())) || (readCount == -1));
}
