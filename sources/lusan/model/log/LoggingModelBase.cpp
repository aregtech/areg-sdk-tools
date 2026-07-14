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

#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/ScopeLogViewerFilter.hpp"
#include "areg/base/DateTime.hpp"

#include <QBrush>
#include <QColor>
#include <QIcon>
#include <QSize>

const QStringList& LoggingModelBase::getHeaderList(void)
{
    static QStringList _headers
    {
          tr("Priority")
        , tr("Time Created")
        , tr("Time Received")
        , tr("Duration, µs")
        , tr("Source")
        , tr("Source ID")
        , tr("Thread")
        , tr("Thread ID")
        , tr("Scope ID")
        , tr("Message")
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
    , mDatabase     ( )
    , mStatement    (mDatabase.database())
    , mLoggingType  (logsType)
    , mActiveColumns(getDefaultColumns())
    , mRootList     ( )
    , mLogs         ( )
    , mInstances    ( )
    , mSelectedScope( )
    , mSelectedLog  ( )
    , mScopes       ( )
    , mLogChunk     (-1)
    , mLogCount     (0)
    , mTotalLogCount(0)
    , mWindowStart  (0)
    , mReadThread   (static_cast<areg::ThreadConsumer &>(self()), "_LogReadingThread_")
    , mQuitThread   (false)
    , mScopeFilter  (nullptr)
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
    
    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
        return QVariant(getHeaderName(section));
        
    case Qt::ItemDataRole::UserRole:
        return QVariant(static_cast<int>(mActiveColumns.at(section)));
    
    case Qt::ItemDataRole::SizeHintRole:
    {
        const QList<int>& widths = getHeaderWidths();
        eColumn col = mActiveColumns.at(section);
        return (static_cast<int>(col) < static_cast<int>(widths.size()) ? QVariant(QSize(widths[static_cast<int>(col)], 28)) : QVariant());
    }
    
    default:
        return QVariant();
    }
}

int LoggingModelBase::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(mTotalLogCount);  // was mLogCount
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
    if ((index.isValid() == false) || mLogs.empty())
        return QVariant();

    int row = index.row();
    int col = index.column();

    if ((row < 0) || (row >= static_cast<int>(mLogCount)))
        return QVariant();

    if ((col < 0) || (col >= static_cast<int>(mActiveColumns.size())))
        return QVariant();
    
    const areg::SharedBuffer & logData {mLogs[row]};
    Q_ASSERT(logData.is_valid());
    const areg::LogEntry* logMessage = reinterpret_cast<const areg::LogEntry*>(logData.buffer());
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
    {
        static const QIcon _iconSelect(NELusanCommon::iconLogSelected(NELusanCommon::SizeSmall));
        return (column == eColumn::LogColumnSourceId) && (mScopeFilter != nullptr) && mScopeFilter->filterExactMatch(index) ? _iconSelect : getDecorationData(logMessage, column);
    }
        
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
    std::string path(areg::File::normalize_path(dbPath.toStdString().c_str()));
    if (mDatabase.database_path() != path)
    {
        mDatabase.connect(path, readOnly);
    }
}

void LoggingModelBase::slideWindow(uint32_t newStartRow)
{
    if (newStartRow == mWindowStart)
        return;

    // Clamp so window doesn't go past end of dataset
    uint32_t windowSize = static_cast<uint32_t>(mLogs.size());
    if (newStartRow + windowSize > mTotalLogCount)
        newStartRow = (mTotalLogCount > windowSize)
                      ? mTotalLogCount - windowSize
                      : 0u;

    mWindowStart = newStartRow;

    // Re-prepare the statement with new offset — SQLite jumps directly, no scanning
    setupLogStatement(areg::TARGET_ALL, mLogChunk, mWindowStart);

    beginResetModel();
    mLogCount = 0;

    int readCount = areg::ext::LogSqliteDatabase::fill_log_messages(
                        mLogs, mStatement, 0, mLogChunk);

    if (readCount > 0)
        mLogCount = static_cast<uint32_t>(readCount);

    endResetModel();
}

QString LoggingModelBase::getDatabasePath(void) const
{
    return QString::fromStdString(mDatabase.database_path().data());
}

void LoggingModelBase::closeDatabase(void)
{
    _closeDatabase();
}

bool LoggingModelBase::isOperable(void) const
{
    return mDatabase.is_operable();
}

void LoggingModelBase::getLogInstanceNames(std::vector<areg::String>& names)
{
    const std::vector< areg::ConnectedInstance> & instances{getLogInstances()};
    names.clear();
    for (const auto& instance : instances)
    {
        names.push_back(instance.ciInstance);
    }
}

void LoggingModelBase::getLogInstanceIds(std::vector<ITEM_ID>& ids)
{
    const std::vector< areg::ConnectedInstance> & instances{getLogInstances()};
    ids.clear();
    for (const auto& instance : instances)
    {
        ids.push_back(instance.ciCookie);
    }
}

void LoggingModelBase::getLogInstances(std::vector<areg::String>&names, std::vector<std::any>& ids)
{
    const std::vector< areg::ConnectedInstance> & instances{getLogInstances()};
    for (const auto& instance : instances)
    {
        names.push_back(instance.ciInstance);
        ids.push_back(std::make_any<ITEM_ID>(instance.ciCookie));
    }
}
    
void LoggingModelBase::getLogThreadNames(std::vector<areg::String>& names)
{
    mDatabase.log_thread_names(names);
}

void LoggingModelBase::getLogThreads(std::vector<ITEM_ID>& ids)
{
    mDatabase.log_threads(ids);
}

void LoggingModelBase::getLogThreadValues(std::vector<areg::String>& names, std::vector<std::any>& ids)
{
    std::vector<ITEM_ID> tids;
    mDatabase.log_thread_names(names);
    mDatabase.log_threads(tids);
    for (auto id : tids)
    {
        ids.push_back(std::make_any<ITEM_ID>(id));
    }
}

void LoggingModelBase::getPriorityNames(std::vector<areg::String>& names)
{
    mDatabase.log_priority_names(names);
}

void LoggingModelBase::getPriorityValues(std::vector<areg::String>& names, std::vector<std::any>& values)
{
    mDatabase.log_priority_names(names);
    for (const auto & name : names)
    {
        areg::LogPriority prio = name.is_empty() ? areg::LogPriority::PrioAny : areg::string_to_priority(name);
        values.push_back(std::make_any<uint16_t>(static_cast<uint16_t>(prio)));
    }
}

const std::vector< areg::ConnectedInstance> & LoggingModelBase::getLogInstances(void)
{
    if (isOfflineLogging() && mInstances.empty())
    {
        mDatabase.log_instance_infos(mInstances);
    }

    return mInstances;
}

const std::vector<areg::ScopeEntry> & LoggingModelBase::getLogInstScopes(ITEM_ID instId)
{
    static std::vector<areg::ScopeEntry> _dummy;
    if (isOfflineLogging() && mScopes.empty())
    {
        mDatabase.log_inst_scopes(mScopes[instId], instId);
    }
    
    return (mScopes.find(instId) != mScopes.end() ? mScopes.at(instId) : _dummy); 
}

const std::vector<areg::SharedBuffer>& LoggingModelBase::getLogMessages()
{
    if (isOfflineLogging() && (mLogCount == 0))
    {
        mDatabase.log_messages(mLogs);
    }

    return mLogs;
}

void LoggingModelBase::getLogInstMessages(std::vector<areg::SharedBuffer>& messages, ITEM_ID instId /*= areg::COOKIE_ANY*/)
{
    mDatabase.log_inst_messages(messages, instId);
}

void LoggingModelBase::getLogScopeMessages(std::vector<areg::SharedBuffer>& messages, uint32_t scopeId /*= 0*/)
{
    mDatabase.log_scope_messages(messages, scopeId);
}

void LoggingModelBase::getLogMessages(std::vector<areg::SharedBuffer>& messages, ITEM_ID instId, uint32_t scopeId)
{
    mDatabase.log_messages(messages, instId, scopeId);
}

int LoggingModelBase::findInstanceEntry(ITEM_ID instId)
{
    int result{ areg::INVALID_INDEX };
    const std::vector<areg::ConnectedInstance> & instances = getLogInstances();
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

const areg::ConnectedInstance& LoggingModelBase::getInstanceEntry(ITEM_ID instId)
{
    static const areg::ConnectedInstance _instInvalid;
    int pos = findInstanceEntry(instId);
    const std::vector<areg::ConnectedInstance>& instances = getLogInstances();
    return (pos != areg::INVALID_INDEX ? instances[pos] : _instInvalid);
}

bool LoggingModelBase::addInstanceEntry(const areg::ConnectedInstance& instance, bool unique)
{
    bool result{ false };
    int pos = findInstanceEntry(instance.ciCookie);
    if ((pos == areg::INVALID_INDEX) || (unique == false))
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
    int result{ areg::INVALID_INDEX };
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

int LoggingModelBase::addInstances(const std::vector<areg::ConnectedInstance>& instances, bool unique)
{
    int result{ 0 };
    for (const auto& instance : instances)
    {
        result += addInstanceEntry(instance, unique) ? 1 : 0;
    }

    return result;
}

int LoggingModelBase::removeInstances(const std::vector<areg::ConnectedInstance>& instances)
{
    int result{ 0 };
    for (const auto& instance : instances)
    {
        result += removeInstanceEntry(instance.ciCookie) != areg::INVALID_INDEX ? 1 : 0;
    }

    return result;
}

void LoggingModelBase::dataTransfer(LoggingModelBase& logModel)
{
    mActiveColumns.clear();
    mActiveColumns = std::move(logModel.mActiveColumns);
    logModel.mActiveColumns.clear();

    cleanLogs();
    mLogs = std::move(logModel.mLogs);
    mLogChunk = logModel.mLogChunk;
    mLogCount = logModel.mLogCount;
    logModel.mLogCount = 0;
    logModel.mLogs.clear();

    mInstances.clear();
    mInstances = std::move(logModel.mInstances);
    logModel.mInstances.clear();

    mScopes.clear();
    mScopes = std::move(logModel.mScopes);
    logModel.mScopes.clear();

    _cleanNodes();
    mRootList = std::move(logModel.mRootList);

    mSelectedScope = std::move(logModel.mSelectedScope);
    logModel.mSelectedScope = QModelIndex();

    mSelectedLog = std::move(logModel.mSelectedLog);
    logModel.mSelectedLog = QModelIndex();

    mDatabase.disconnect();
    if (logModel.mDatabase.is_operable())
    {
        mDatabase.connect(logModel.mDatabase.database_path(), true);
    }

    logModel.mDatabase.disconnect();
}

void LoggingModelBase::readLogsAsynchronous(int maxEntries)
{
    _quitThread();
    beginResetModel();
    cleanLogs();
    endResetModel();
    mLogChunk = maxEntries;
    mLogCount = 0u;
    mWindowStart = 0u;           
    
    uint32_t count = setupLogStatement(areg::TARGET_ALL, mLogChunk, 0u);
    if (count == 0)
        return;

    mTotalLogCount = count;

    // If maxEntries <= 0, no windowing limit is applied (e.g. scopes model loads all)
    uint32_t windowSize = (maxEntries > 0)
                          ? static_cast<uint32_t>(maxEntries)
                          : count;
    mLogs.resize(std::min(count, windowSize));
    mReadThread.start(areg::DO_NOT_WAIT);
}

// New — add limit and offset params
uint32_t LoggingModelBase::setupLogStatement(ITEM_ID instId, int32_t limit, uint32_t offset)
{
    return mDatabase.setup_statement_read_logs(mStatement, instId, limit, offset);
}

bool LoggingModelBase::applyFilters(uint32_t instId, const areg::ArrayList<areg::ext::LogSqliteDatabase::ScopeFilter>& filter)
{
    return mDatabase.setup_filter_logs(instId, filter);
}

bool LoggingModelBase::resetFilters(uint32_t instId)
{
    return mDatabase.reset(instId);
}

bool LoggingModelBase::disableFilters(uint32_t instId)
{
    return mDatabase.disable_filter_mask(instId);
}

QString LoggingModelBase::getDisplayData(const areg::LogEntry* logMessage, eColumn column) const
{
    Q_ASSERT(logMessage != nullptr);

    switch (column)
    {
    case eColumn::LogColumnPriority:
        return QString::fromStdString(areg::priority_to_string(logMessage->logMessagePrio).data());

    case eColumn::LogColumnTimestamp:
        return QString::fromStdString(areg::DateTime(logMessage->logTimestamp).format_time().data());

    case eColumn::LogColumnTimeReceived:
        return QString::fromStdString(areg::DateTime(logMessage->logReceived).format_time().data());
    
    case eColumn::LogColumnTimeDuration:
        return QString::number(logMessage->logDuration);
        
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

QBrush LoggingModelBase::getBackgroundData(const areg::LogEntry* logMessage, eColumn column) const
{
    Q_UNUSED(column)
    Q_ASSERT(logMessage != nullptr);
    return QBrush(LogIconFactory::getLogBackgroundColor(*logMessage));
}

QColor LoggingModelBase::getForegroundData(const areg::LogEntry* logMessage, eColumn column) const
{
    Q_UNUSED(column)
    Q_ASSERT(logMessage != nullptr);
    return LogIconFactory::getLogColor(*logMessage);
}

QIcon LoggingModelBase::getDecorationData(const areg::LogEntry* logMessage, eColumn column) const
{
    Q_ASSERT(logMessage != nullptr);
    if (column != eColumn::LogColumnPriority)
        return QIcon();

    switch (logMessage->logMessagePrio)
    {
    case areg::LogPriority::PrioScope:
        if (logMessage->logMsgType == areg::LogMessageType::ScopeEnter)
            return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioScopeEnter, true);
        else if (logMessage->logMsgType == areg::LogMessageType::ScopeExit)
            return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioScopeExit, true);
        else
            return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioScope, true);
    case areg::LogPriority::PrioDebug:
        return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioDebug, true);
    case areg::LogPriority::PrioInfo:
        return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioInfo, true);
    case areg::LogPriority::PrioWarning:
        return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioWarn, true);
    case areg::LogPriority::PrioError:
        return LogIconFactory::getLogIcon(LogIconFactory::eLogIcons::PrioError, true);
    case areg::LogPriority::PrioFatal:
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
    case eColumn::LogColumnTimeDuration:
        return static_cast<int>(Qt::AlignCenter | Qt::AlignVCenter);

    case eColumn::LogColumnTimestamp:
    case eColumn::LogColumnTimeReceived:
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);

    default:
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }
}

inline void LoggingModelBase::_cleanNodes(void)
{
    for (ScopeRoot* root : mRootList)
    {
        Q_ASSERT(root != nullptr);
        delete root;
    }
    
    mRootList.clear();
}

void LoggingModelBase::on_run(void)
{
    uint32_t    nextStart   { 0 };
    int         readCount   { 0 };
    
    Q_ASSERT(mLogCount == 0);

    do
    {
        readCount = -1;
        if (mQuitThread.try_lock() == false)
            break;
        
        mQuitThread.unlock();
        readCount   = areg::ext::LogSqliteDatabase::fill_log_messages(mLogs, mStatement, nextStart, mLogChunk);
        if (readCount != 0)
        {
            beginInsertRows(QModelIndex(), static_cast<int>(nextStart), static_cast<int>(nextStart) + readCount - 1);
            nextStart += static_cast<uint32_t>(readCount);
            mLogCount = nextStart;
            endInsertRows();
        }
    } while ((readCount > 0) && (readCount == mLogChunk));
    
    Q_ASSERT((mLogCount == static_cast<uint32_t>(mLogs.size())) || (readCount == -1));
}

