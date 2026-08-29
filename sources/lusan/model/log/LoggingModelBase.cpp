/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/log/LoggingModelBase.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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
#include "lusan/common/NELogPalette.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/ScopeLogViewerFilter.hpp"
#include "areg/base/DateTime.hpp"

#include <QBrush>
#include <QColor>
#include <QIcon>
#include <QSize>

#include <iterator>

const QStringList& LoggingModelBase::getHeaderList()
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

const QList<int>& LoggingModelBase::getHeaderWidths()
{
    static QList<int>  _widths{ 50, 100, 100, 50, 100, 50, 50, 200 };
    return _widths;
}

const QList<LoggingModelBase::eColumn>& LoggingModelBase::getDefaultColumns()
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
    , mLoadGeneration(0)
    , mReadThread   (static_cast<areg::ThreadConsumer &>(self()), "_LogReadingThread_")
    , mQuitThread   (false)
    , mScopeFilter  (nullptr)
{
}

LoggingModelBase::~LoggingModelBase()
{
    // The reading thread works on the database and the statement of this object, stop it first.
    _quitThread();
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
    // Only the entries the model actually holds are addressable. Reporting the number of
    // rows the database has would hand out indexes that data() cannot answer.
    return parent.isValid() ? 0 : static_cast<int>(mLogCount);
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
        
    case Qt::ToolTipRole:
        return getTooltipData(logMessage, column);

    case Qt::TextAlignmentRole:
        return getAlignmentData(column);
        
    case Qt::UserRole:
        return QVariant::fromValue(logMessage);
        
    default:
        return QVariant();
    }
}

QString LoggingModelBase::getScopeName(ITEM_ID target, uint32_t scopeId) const
{
    const auto entry{ mScopes.find(target) };
    if (entry == mScopes.end())
        return QString();

    for (const areg::ScopeEntry& scope : entry->second)
    {
        if (scope.scopeId == scopeId)
            return QString(scope.scopeName.as_string());
    }

    return QString();
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

void LoggingModelBase::refresh()
{
    beginResetModel();
    endResetModel();
}

void LoggingModelBase::setupModel()
{
}

void LoggingModelBase::releaseModel()
{
}

void LoggingModelBase::openDatabase(const QString& dbPath, bool readOnly)
{
    std::string path(areg::File::normalize_path(dbPath.toStdString().c_str()));
    if (mDatabase.database_path() != path)
    {
        // Another database is another session, and a refused span names a target of the old one.
        clearRefusedScopes();
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

    _quitThread();
    mWindowStart = newStartRow;

    // Re-prepare the statement with the new offset, SQLite jumps directly without scanning.
    setupLogStatement(areg::TARGET_ALL, mLogChunk, mWindowStart);

    beginResetModel();
    mLogCount = 0;

    int readCount = areg::ext::LogSqliteDatabase::fill_log_messages(
                        mLogs, mStatement, 0, mLogChunk);

    if (readCount > 0)
        mLogCount = static_cast<uint32_t>(readCount);

    endResetModel();
}

QString LoggingModelBase::getDatabasePath() const
{
    return QString::fromStdString(mDatabase.database_path().data());
}

void LoggingModelBase::closeDatabase()
{
    _closeDatabase();
}

bool LoggingModelBase::isOperable() const
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

const std::vector< areg::ConnectedInstance> & LoggingModelBase::getLogInstances()
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
    // Both models may have a reading thread on their database, stop them before the data moves.
    _quitThread();
    logModel._quitThread();

    mActiveColumns.clear();
    mActiveColumns = std::move(logModel.mActiveColumns);
    logModel.mActiveColumns.clear();

    cleanLogs();
    mLogs = std::move(logModel.mLogs);
    mLogChunk       = logModel.mLogChunk;
    mLogCount       = logModel.mLogCount;
    mTotalLogCount  = logModel.mTotalLogCount;
    mWindowStart    = logModel.mWindowStart;
    logModel.cleanLogs();

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

    uint32_t count = setupLogStatement(areg::TARGET_ALL, mLogChunk, 0u);
    if (count == 0)
        return;

    mTotalLogCount = count;
    mLogs.reserve(count);
    mReadThread.start(areg::DO_NOT_WAIT);
}

void LoggingModelBase::setScopesRefused(ITEM_ID target, const QSet<uint32_t>& scopeIds, bool refuse, TIME64 since)
{
    if (scopeIds.isEmpty())
        return;

    bool changed{ false };
    MapScopeSpans& scopes = mRefused[target];
    for (uint32_t scopeId : scopeIds)
    {
        ListSpans& spans = scopes[scopeId];
        const bool open{ spans.isEmpty() == false && spans.last().to == 0 };
        if (refuse && (open == false))
        {
            spans.append(sRefusedSpan{ since, 0 });
            changed = true;
        }
        else if ((refuse == false) && open)
        {
            // A span that covers the whole session leaves nothing behind when it is lifted.
            if (spans.last().from == 0)
                spans.removeLast();
            else
                spans.last().to = since;

            changed = true;
        }
    }

    if (changed)
    {
        emit signalRefusedScopesChanged();
    }
}

bool LoggingModelBase::isEntryRefused(const areg::LogEntry* entry) const
{
    if ((entry == nullptr) || mRefused.isEmpty())
        return false;

    const auto target = mRefused.constFind(entry->logCookie);
    if (target == mRefused.constEnd())
        return false;

    const auto scope = target.value().constFind(entry->logScopeId);
    if (scope == target.value().constEnd())
        return false;

    for (const sRefusedSpan& span : scope.value())
    {
        if ((entry->logTimestamp >= span.from) && ((span.to == 0) || (entry->logTimestamp < span.to)))
            return true;
    }

    return false;
}

bool LoggingModelBase::hasRefusedScopes(void) const
{
    for (auto target = mRefused.constBegin(); target != mRefused.constEnd(); ++target)
    {
        for (auto scope = target.value().constBegin(); scope != target.value().constEnd(); ++scope)
        {
            if (scope.value().isEmpty() == false)
                return true;
        }
    }

    return false;
}

void LoggingModelBase::clearRefusedScopes(void)
{
    if (mRefused.isEmpty())
        return;

    mRefused.clear();
    emit signalRefusedScopesChanged();
}

void LoggingModelBase::requestShowAllScopes(void)
{
    emit signalShowAllScopesRequested();
}

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
        // A scope row names the event, not the category: the reader already knows it is
        // a scope, what they need is which end of it.
        if (logMessage->logMessagePrio == areg::LogPriority::PrioScope)
        {
            if (logMessage->logMsgType == areg::LogMessageType::ScopeEnter)
                return tr("Enter");
            else if (logMessage->logMsgType == areg::LogMessageType::ScopeExit)
                return tr("Exit");
        }

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
        {
            // logMessageLen is the length before the message was cut, so it can exceed
            // what the entry holds. When it does, say how much is missing.
            constexpr uint32_t maxLen{ areg::LOG_MSG_SIZE - 1u };
            QString text{ QString::fromUtf8(logMessage->logMessage) };
            if (logMessage->logMessageLen > maxLen)
            {
                text += QString("  [+%1 B]").arg(logMessage->logMessageLen - maxLen);
            }

            return text;
        }

    default:
        return QString();
    }
}

QString LoggingModelBase::getTooltipData(const areg::LogEntry* logMessage, eColumn column) const
{
    Q_ASSERT(logMessage != nullptr);
    if (column != eColumn::LogColumnMessage)
        return QString();

    const QString message{ QString::fromUtf8(logMessage->logMessage, static_cast<int>(areg::log_message_size(*logMessage))) };
    if (areg::is_log_message_cut(*logMessage) == false)
        return message;

    // The row shows what arrived; the tooltip says what was sent.
    return message + QString("\n\n") +
           tr("Cut after %1 of %2 characters.").arg(areg::log_message_size(*logMessage)).arg(logMessage->logMessageLen);
}

QBrush LoggingModelBase::getBackgroundData(const areg::LogEntry* logMessage, eColumn column) const
{
    Q_UNUSED(column)
    Q_ASSERT(logMessage != nullptr);
    // The row stays neutral, so that only Fatal pulls the eye.
    return QBrush(NELogPalette::rowBackground(NELogPalette::roleOf(*logMessage)));
}

QColor LoggingModelBase::getForegroundData(const areg::LogEntry* logMessage, eColumn column) const
{
    Q_UNUSED(column)
    Q_ASSERT(logMessage != nullptr);
    return NELogPalette::textColor(NELogPalette::roleOf(*logMessage));
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

inline void LoggingModelBase::_cleanNodes()
{
    for (ScopeRoot* root : mRootList)
    {
        Q_ASSERT(root != nullptr);
        delete root;
    }
    
    mRootList.clear();
}

void LoggingModelBase::appendLogBatch(std::vector<areg::SharedBuffer>&& logs, uint32_t generation)
{
    if ((generation != mLoadGeneration) || logs.empty())
        return;

    const int first{ static_cast<int>(mLogs.size()) };
    const int last { first + static_cast<int>(logs.size()) - 1 };

    beginInsertRows(QModelIndex(), first, last);
    mLogs.insert(mLogs.end(), std::make_move_iterator(logs.begin()), std::make_move_iterator(logs.end()));
    mLogCount = static_cast<uint32_t>(mLogs.size());
    endInsertRows();
}

void LoggingModelBase::on_run()
{
    // Runs in the reading thread. It reads from the database only, the entries are handed
    // over to the thread that owns the model, which is the single writer of the row list.
    const uint32_t generation{ mLoadGeneration };
    const int32_t  chunk{ mLogChunk > 0 ? mLogChunk : LoggingModelBase::READ_CHUNK_SIZE };
    int32_t        readCount{ 0 };

    do
    {
        if (mQuitThread.try_lock() == false)
            break;

        mQuitThread.unlock();

        std::vector<areg::SharedBuffer> batch(static_cast<size_t>(chunk));
        readCount = areg::ext::LogSqliteDatabase::fill_log_messages(batch, mStatement, 0, chunk);
        if (readCount <= 0)
            break;

        batch.resize(static_cast<size_t>(readCount));
        QMetaObject::invokeMethod(this
                                 , [this, generation, batch = std::move(batch)]() mutable
                                   {
                                       appendLogBatch(std::move(batch), generation);
                                   }
                                 , Qt::ConnectionType::QueuedConnection);

    } while (readCount == chunk);
}

