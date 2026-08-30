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
 *  \file        lusan/model/log/ScopeLogViewerFilter.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope Output Viewer Filter Proxy Model.
 *
 ************************************************************************/

#include "lusan/model/log/ScopeLogViewerFilter.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"

#include "areg/logging/areg_log.h"
#include "areg/component/ServiceDefs.hpp"

ScopeLogViewerFilter::ScopeLogViewerFilter(uint32_t scopeId /*= 0u*/, LoggingModelBase* model /*= nullptr*/)
    : LogViewerFilter(model)

    , mSelScopeData     ( )
    , mScopeData        ( )
    , mSelSessionData   ( )
    , mSessionData      ( )
    , mSelThreadData    ( )
    , mThreadData       ( )
    , mSelInstanceData  ( )
    , mInstanceData     ( )
    , mSelPriorityData  ( )
    , mPriorityData     ( )
    , mActiveFilter     (eDataFilter::NoFilter)
    , mIndexStart       ( )
    , mIndexEnd         ( )
    , mCalls            ( )
    , mStacks           ( )
    , mFolded           ( )
    , mCallsRead        (0)
    , mAutoFold         (false)
    , mInteresting      (false)
    , mSlowUs           (0)
    , mRelativeTime     (false)
{
}

ScopeLogViewerFilter::~ScopeLogViewerFilter()
{
    _clearData();
}

void ScopeLogViewerFilter::setScopeFilter(LoggingModelBase *model, const QModelIndex& index)
{
    const areg::LogEntry* logMessage = index.data(Qt::ItemDataRole::UserRole).value<const areg::LogEntry *>();
    if ((logMessage != nullptr) && (model != nullptr))
    {
        setScopeFilter(model, logMessage->logScopeId, logMessage->logSessionId, logMessage->logThreadId, logMessage->logCookie);
    }
    else
    {
        setScopeFilter(nullptr, 0u, 0u, 0u, 0u);
    }
}

void ScopeLogViewerFilter::setScopeFilter(LoggingModelBase* model, uint32_t scopeId, uint32_t sessionId, ITEM_ID threadId, ITEM_ID instanceId)
{
    setSourceModel(nullptr);
    clearFilters();

    if (model != nullptr)
    {
        mSelScopeData.data = scopeId;
        mSelScopeData.isSet = true;
        mScopeData = mSelScopeData;

        mSelSessionData.data = sessionId;
        mSelSessionData.isSet = true;
        mSessionData = mSelSessionData;

        mSelThreadData.data = threadId;
        mSelThreadData.isSet = true;
        mThreadData = mSelThreadData;

        mSelInstanceData.data = instanceId;
        mSelInstanceData.isSet = true;
        mInstanceData = mSelInstanceData;
        
        setSourceModel(model);
    }
}

QModelIndex ScopeLogViewerFilter::getIndexNextScope(const QModelIndex& startAt, bool asSource) const
{
    QAbstractItemModel* model = sourceModel();
    int count = model != nullptr ? rowCount() : 0;
    if (count == 0)
        return QModelIndex();

    if (startAt.isValid() == false)
    {
        QModelIndex idx = index(0, 0);
        return (asSource ? mapToSource(idx) : idx);
    }

    QModelIndex idxTarget;
    idxTarget = asSource ? mapFromSource(startAt) : startAt;
    int row = idxTarget.row();
    if (row >= (count - 1))
        return QModelIndex();

    const areg::LogEntry* log = data(idxTarget, static_cast<int>(Qt::UserRole)).value<const areg::LogEntry *>();
    uint32_t scopeId    = log->logScopeId;
    uint32_t sessionId  = log->logSessionId;
    uint32_t moduleId   = log->logCookie;
    
    for ( row += 1; row < count; ++ row)
    {
        QModelIndex idx = index(row, 0);
        log = data(idx, static_cast<int>(Qt::UserRole)).value<const areg::LogEntry *>();
        if ((log->logScopeId != scopeId) || (log->logSessionId != sessionId) || (log->logCookie != moduleId))
        {
            return (asSource ? mapToSource(idx) : idx);
        }
    }

    return QModelIndex();
}

QModelIndex ScopeLogViewerFilter::getIndexPrevScope(const QModelIndex& startAt, bool asSource) const
{
    QAbstractItemModel* model = sourceModel();
    int count = model != nullptr ? rowCount() : 0;
    if (count == 0)
        return QModelIndex();

    if (startAt.isValid() == false)
    {
        QModelIndex idx = index(0, count - 1);
        return (asSource ? mapToSource(idx) : idx);
    }

    QModelIndex idxTarget;
    idxTarget = asSource ? mapFromSource(startAt) : startAt;
    int row = idxTarget.row();
    if (row == 0)
        return QModelIndex();

    const areg::LogEntry* log = data(idxTarget, static_cast<int>(Qt::UserRole)).value<const areg::LogEntry *>();
    uint32_t scopeId = log->logScopeId;
    uint32_t sessionId  = log->logSessionId;
    uint32_t moduleId   = log->logCookie;
    
    for ( row -= 1; row >= 0; -- row)
    {
        QModelIndex idx = index(row, 0);
        log = data(idx, static_cast<int>(Qt::UserRole)).value<const areg::LogEntry *>();
        if ((log->logScopeId != scopeId) || (log->logSessionId != sessionId) || (log->logCookie != moduleId))
        {
            return (asSource ? mapToSource(idx) : idx);
        }
    }
    
    return QModelIndex();
}

void ScopeLogViewerFilter::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (sourceModel != nullptr)
    {
        static_cast<LoggingModelBase *>(sourceModel)->setScopeFiler(this);
    }
    else if (this->sourceModel() != nullptr)
    {
        static_cast<LoggingModelBase *>(this->sourceModel())->setScopeFiler(nullptr);
    }
    
    LogViewerFilter::setSourceModel(sourceModel);
    _resetCalls();
}

void ScopeLogViewerFilter::clearFilters()
{
    _clearData();
    _resetCalls();
    LogViewerFilter::clearFilters();
}

bool ScopeLogViewerFilter::filterExactMatch(const QModelIndex& index) const
{
    return (matchesScopeFilter(index) == NELusanCommon::eMatchType::ExactMatch);
}

bool ScopeLogViewerFilter::filterAcceptsRow(int row, const QModelIndex& parent) const
{
    QModelIndex index = sourceModel() != nullptr ? sourceModel()->index(row, 0, parent) : QModelIndex();
    if ((matchesScopeFilter(index) == NELusanCommon::eMatchType::NoMatch) || (LogViewerFilter::filterAcceptsRow(row, parent) == false))
        return false;

    if (_isHiddenByFold(row))
        return false;

    return (mInteresting == false) || _isInteresting(row);
}

QVariant ScopeLogViewerFilter::data(const QModelIndex& index, int role) const
{
    if (index.isValid() == false)
        return QVariant();

    if ((role < ScopeLogViewerFilter::RoleCallDepth) || (role > ScopeLogViewerFilter::RoleCallElapsed))
    {
        if (role == Qt::ItemDataRole::DisplayRole)
        {
            // A scope enter carries no duration of its own, so the row that opens a call shows
            // how long the call took. A folded call is then still readable.
            const LoggingModelBase* logs{ static_cast<const LoggingModelBase *>(sourceModel()) };
            if ((logs != nullptr) && (logs->fromIndexToColumn(index.column()) == LoggingModelBase::eColumn::LogColumnTimeDuration))
            {
                const_cast<ScopeLogViewerFilter *>(this)->_readCalls();
                const ScopeLogViewerFilter::sCallRow& own{ _callOf(mapToSource(index).row()) };
                if ((own.closer >= 0) && (own.elapsed != 0))
                    return QVariant(QString("%1 ms").arg(static_cast<double>(own.elapsed) / 1000.0, 0, 'f', 3));
            }
        }

        if ((mRelativeTime == false) || (role != Qt::ItemDataRole::DisplayRole))
            return LogViewerFilter::data(index, role);

        const LoggingModelBase* model{ static_cast<const LoggingModelBase *>(sourceModel()) };
        if (model == nullptr)
            return LogViewerFilter::data(index, role);

        const LoggingModelBase::eColumn column{ model->fromIndexToColumn(index.column()) };
        if (column != LoggingModelBase::eColumn::LogColumnTimestamp)
            return LogViewerFilter::data(index, role);

        const_cast<ScopeLogViewerFilter *>(this)->_readCalls();
        const int srcRow{ mapToSource(index).row() };
        const ScopeLogViewerFilter::sCallRow& call{ _callOf(srcRow) };
        const areg::LogEntry* entry{ model->getLogData(srcRow) };
        const areg::LogEntry* opener{ call.opener >= 0 ? model->getLogData(call.opener) : nullptr };
        if ((entry == nullptr) || (opener == nullptr))
            return LogViewerFilter::data(index, role);

        const qint64 sinceUs{ (static_cast<qint64>(entry->logTimestamp) - static_cast<qint64>(opener->logTimestamp)) / 1000 };
        return QVariant(QString("+%1 ms").arg(static_cast<double>(sinceUs) / 1000.0, 0, 'f', 3));
    }

    const_cast<ScopeLogViewerFilter *>(this)->_readCalls();
    const int srcRow{ mapToSource(index).row() };
    const ScopeLogViewerFilter::sCallRow& call{ _callOf(srcRow) };

    switch (role)
    {
    case ScopeLogViewerFilter::RoleCallDepth:
        return QVariant(call.depth);

    case ScopeLogViewerFilter::RoleCallFold:
        return QVariant(static_cast<int>( call.closer < 0
                                            ? ScopeLogViewerFilter::eCallFold::FoldNone
                                            : mFolded.contains(srcRow) ? ScopeLogViewerFilter::eCallFold::FoldClosed
                                                                       : ScopeLogViewerFilter::eCallFold::FoldOpen));

    case ScopeLogViewerFilter::RoleCallBracket:
    {
        const LoggingModelBase* model{ static_cast<const LoggingModelBase *>(sourceModel()) };
        const areg::LogEntry* entry{ model != nullptr ? model->getLogData(srcRow) : nullptr };
        if (entry == nullptr)
            return QVariant(static_cast<int>(ScopeLogViewerFilter::eCallBracket::BracketNone));
        else if (entry->logMsgType == areg::LogMessageType::ScopeEnter)
            return QVariant(static_cast<int>(ScopeLogViewerFilter::eCallBracket::BracketOpen));
        else if (entry->logMsgType == areg::LogMessageType::ScopeExit)
            return QVariant(static_cast<int>(ScopeLogViewerFilter::eCallBracket::BracketClose));

        return QVariant(static_cast<int>( call.opener >= 0 ? ScopeLogViewerFilter::eCallBracket::BracketInside
                                                           : ScopeLogViewerFilter::eCallBracket::BracketNone));
    }

    case ScopeLogViewerFilter::RoleCallElapsed:
        return QVariant(call.elapsed);

    default:
        break;
    }

    return QVariant();
}

bool ScopeLogViewerFilter::toggleFold(const QModelIndex& index)
{
    const int srcRow{ mapToSource(index).row() };
    if ((srcRow < 0) || (srcRow >= mCalls.size()) || (mCalls.at(srcRow).closer < 0))
        return false;

    if (mFolded.remove(srcRow) == false)
    {
        mFolded.insert(srcRow);
    }

    invalidateRowsFilter();
    return true;
}

void ScopeLogViewerFilter::setAutoFold(bool enable)
{
    mAutoFold = enable;
    mFolded.clear();
    if (enable)
    {
        _readCalls();
        for (int row = 0; row < mCalls.size(); ++row)
        {
            const ScopeLogViewerFilter::sCallRow& call{ mCalls.at(row) };
            if ((call.closer >= 0) && (call.problem == false))
            {
                mFolded.insert(row);
            }
        }
    }

    invalidateRowsFilter();
}

void ScopeLogViewerFilter::setInterestingOnly(bool enable, uint32_t slowUs)
{
    mInteresting = enable;
    mSlowUs = slowUs;
    invalidateRowsFilter();
}

void ScopeLogViewerFilter::setRelativeTime(bool enable)
{
    if (mRelativeTime == enable)
        return;

    mRelativeTime = enable;
    const int rows{ rowCount() };
    const int cols{ columnCount() };
    if ((rows > 0) && (cols > 0))
    {
        emit dataChanged(index(0, 0), index(rows - 1, cols - 1), QList<int>{ Qt::ItemDataRole::DisplayRole });
    }
}

void ScopeLogViewerFilter::_resetCalls(void)
{
    mCalls.clear();
    mStacks.clear();
    mFolded.clear();
    mCallsRead = 0;
}

void ScopeLogViewerFilter::_readCalls(void)
{
    const LoggingModelBase* model{ static_cast<const LoggingModelBase *>(sourceModel()) };
    if (model == nullptr)
    {
        _resetCalls();
        return;
    }

    const int rows{ model->rowCount() };
    if (rows < mCallsRead)
    {
        // Rows were dropped under the walk, so the recorded positions no longer point at
        // the entries they were read from.
        _resetCalls();
    }

    if (rows == mCallsRead)
        return;

    mCalls.resize(rows);
    for (int row = mCallsRead; row < rows; ++row)
    {
        const areg::LogEntry* entry{ model->getLogData(row) };
        ScopeLogViewerFilter::sCallRow& rec{ mCalls[row] };
        rec = ScopeLogViewerFilter::sCallRow{ };
        if (entry == nullptr)
            continue;

        const quint64 key{ (static_cast<quint64>(entry->logCookie) << 32) | static_cast<quint64>(entry->logThreadId & 0xFFFFFFFFu) };
        QVector<int32_t>& stack{ mStacks[key] };

        if (entry->logMsgType == areg::LogMessageType::ScopeExit)
        {
            if (stack.isEmpty() == false)
            {
                const int32_t opened{ stack.takeLast() };
                ScopeLogViewerFilter::sCallRow& call{ mCalls[opened] };
                call.closer = row;
                call.elapsed = entry->logDuration;
                rec.depth = call.depth;
                // The exit belongs to the call it closes, so folding that call takes the
                // closing line with it and leaves the opening line alone on screen.
                rec.opener = opened;
            }
            else
            {
                rec.opener = -1;
            }
        }
        else
        {
            rec.depth = static_cast<int32_t>(stack.size());
            rec.opener = stack.isEmpty() ? -1 : stack.last();
            if (entry->logMsgType == areg::LogMessageType::ScopeEnter)
            {
                stack.append(row);
            }
        }

        if (LoggingModelBase::isProblemEntry(entry))
        {
            // A call is worth opening when anything it holds, at any depth, went wrong.
            for (int32_t open : stack)
            {
                mCalls[open].problem = true;
            }
        }
    }

    mCallsRead = rows;
}

bool ScopeLogViewerFilter::_isHiddenByFold(int srcRow) const
{
    if (mFolded.isEmpty())
        return false;

    const_cast<ScopeLogViewerFilter *>(this)->_readCalls();
    int32_t at{ _callOf(srcRow).opener };
    while (at >= 0)
    {
        if (mFolded.contains(at))
            return true;

        at = _callOf(at).opener;
    }

    return false;
}

bool ScopeLogViewerFilter::_isInteresting(int srcRow) const
{
    const LoggingModelBase* model{ static_cast<const LoggingModelBase *>(sourceModel()) };
    const areg::LogEntry* entry{ model != nullptr ? model->getLogData(srcRow) : nullptr };
    if (entry == nullptr)
        return false;
    else if (LoggingModelBase::isProblemEntry(entry))
        return true;

    const_cast<ScopeLogViewerFilter *>(this)->_readCalls();

    // The pair that opens and closes a call worth reading stays, so the rows kept still
    // sit in the calls they came from.
    int32_t owner{ -1 };
    if (entry->logMsgType == areg::LogMessageType::ScopeEnter)
    {
        owner = srcRow;
    }
    else if (entry->logMsgType == areg::LogMessageType::ScopeExit)
    {
        owner = _callOf(srcRow).opener;
    }

    if (owner < 0)
        return false;

    const ScopeLogViewerFilter::sCallRow& call{ _callOf(owner) };
    return (call.problem || ((mSlowUs != 0) && (call.elapsed >= mSlowUs)));
}

NELusanCommon::eMatchType ScopeLogViewerFilter::matchesScopeFilter(const QModelIndex& index) const
{
    if ((mSelScopeData.valid() == false) || (sourceModel() == nullptr))
        return NELusanCommon::eMatchType::PartialMatch; // No scope filter applied
    else if (index.isValid() == false)
        return NELusanCommon::eMatchType::NoMatch;
    
    const areg::LogEntry* logMessage = index.data(Qt::ItemDataRole::UserRole).value<const areg::LogEntry*>();
    if (logMessage == nullptr)
        return NELusanCommon::eMatchType::NoMatch;
    
    if ((logMessage->logCookie != mInstanceData.value()) && mInstanceData.valid())
        return NELusanCommon::eMatchType::NoMatch;
    else if ((logMessage->logCookie <= areg::COOKIE_ANY) && (mInstanceData.valid() == false))
        return NELusanCommon::eMatchType::NoMatch;
    else if (logMessage->logCookie != mSelInstanceData.value())
        return NELusanCommon::eMatchType::PartialOutput;

    if ((logMessage->logThreadId != mThreadData.value()) && mThreadData.valid())
        return NELusanCommon::eMatchType::NoMatch;
    else if ((logMessage->logThreadId == 0) && (mThreadData.valid() == false))
        return NELusanCommon::eMatchType::NoMatch;
    else if ((logMessage->logThreadId != mSelThreadData.value()) && (mThreadData.valid() == false))
    {
        if (mInstanceData.valid() && mSessionData.valid())
            return NELusanCommon::eMatchType::NoMatch;
    }

    if ((logMessage->logScopeId != mScopeData.value()) && mScopeData.valid())
    {
        if (mActiveFilter == eDataFilter::FilterSublogs)
        {
            if ((mIndexStart.isValid() && (mIndexEnd.isValid() == false)) || ((mIndexStart.row() < index.row()) && (index.row() < mIndexEnd.row())))
                return NELusanCommon::eMatchType::PartialOutput;
        }

        return NELusanCommon::eMatchType::NoMatch;
    }
    else if ((logMessage->logScopeId == 0) && (mScopeData.valid() == false))
    {
        return NELusanCommon::eMatchType::NoMatch;
    }
    else if ((logMessage->logScopeId != mSelScopeData.value()) && (mScopeData.valid() == false))
    {
        if (mThreadData.valid() && mInstanceData.valid() && mSessionData.valid())
            return NELusanCommon::eMatchType::NoMatch;

        return NELusanCommon::eMatchType::PartialOutput;
    }

    if ((logMessage->logSessionId != mSessionData.value()) && mSessionData.valid())
    {
        if (mActiveFilter == eDataFilter::FilterSublogs)
        {
            Q_ASSERT(logMessage->logThreadId == mThreadData.value());
            if ((mIndexStart.isValid() && (mIndexEnd.isValid() == false)) || ((mIndexStart.row() < index.row()) && (index.row() < mIndexEnd.row())))
                return NELusanCommon::eMatchType::PartialOutput;
        }
        
        return NELusanCommon::eMatchType::NoMatch;
    }
    else if (logMessage->logSessionId != mSelSessionData.value())
    {
        return NELusanCommon::eMatchType::PartialOutput;
    }

    Q_ASSERT(logMessage->logSessionId == mSelSessionData.value());
    if (logMessage->logMsgType == areg::LogMessageType::ScopeEnter)
    {
        mIndexStart = index;
        if (mIndexEnd.isValid() == false)
        {
            emit const_cast<ScopeLogViewerFilter *>(this)->signalFilterSelected(mIndexStart, mIndexEnd);
        }
    }
    else if (logMessage->logMsgType == areg::LogMessageType::MessageText)
    {
        if (mIndexStart.isValid() == false)
        {
            mIndexStart = index;
            if (mIndexEnd.isValid() == false)
            {
                emit const_cast<ScopeLogViewerFilter *>(this)->signalFilterSelected(mIndexStart, mIndexEnd);
            }
        }
    }
    else if (logMessage->logMsgType == areg::LogMessageType::ScopeExit)
    {
        mIndexEnd = index;
        emit const_cast<ScopeLogViewerFilter *>(this)->signalFilterSelected(mIndexStart, mIndexEnd);
    }
        
    return NELusanCommon::eMatchType::ExactMatch;
}

void ScopeLogViewerFilter::filterData(ScopeLogViewerFilter::eDataFilter dataFilter)
{
    mIndexStart = QModelIndex();
    mIndexEnd   = QModelIndex();
    
    switch (dataFilter)
    {
    case ScopeLogViewerFilter::eDataFilter::FilterSession:
        mSessionData = mSelSessionData;
        mScopeData   = mSelScopeData;
        mThreadData  = mSelThreadData;
        mInstanceData= mSelInstanceData;
        mActiveFilter= ScopeLogViewerFilter::eDataFilter::FilterSession;
        break;
        
    case ScopeLogViewerFilter::eDataFilter::FilterSublogs:
        mSessionData = mSelSessionData;
        mScopeData   = mSelScopeData;
        mThreadData  = mSelThreadData;
        mInstanceData= mSelInstanceData;
        mActiveFilter= ScopeLogViewerFilter::eDataFilter::FilterSublogs;
        break;
        
    case ScopeLogViewerFilter::eDataFilter::FilterScope:
        mSessionData.clear();
        mScopeData   = mSelScopeData;
        mThreadData  = mSelThreadData;
        mInstanceData= mSelInstanceData;
        mActiveFilter= ScopeLogViewerFilter::eDataFilter::FilterScope;
        break;

    case ScopeLogViewerFilter::eDataFilter::FilterThread:
        mSessionData.clear();
        mScopeData.clear();
        mThreadData  = mSelThreadData;
        mInstanceData= mSelInstanceData;
        mActiveFilter= ScopeLogViewerFilter::eDataFilter::FilterThread;
        break;

    case ScopeLogViewerFilter::eDataFilter::FilterProcess:
        mSessionData.clear();
        mScopeData.clear();
        mThreadData.clear();
        mInstanceData= mSelInstanceData;
        mActiveFilter= ScopeLogViewerFilter::eDataFilter::FilterProcess;
        break;

    case ScopeLogViewerFilter::eDataFilter::NoFilter:
    default:
        mSessionData.clear();
        mScopeData.clear();
        mThreadData.clear();
        mInstanceData.clear();
        mActiveFilter= ScopeLogViewerFilter::eDataFilter::NoFilter;
        break;
    }

    emit signalFilterSelected(mIndexStart, mIndexEnd);
    invalidateRowFilter();
}

inline void ScopeLogViewerFilter::_clearData()
{
    mSelScopeData.clear();
    mScopeData.clear();
    mSelSessionData.clear();
    mSessionData.clear();
    mSelThreadData.clear();
    mThreadData.clear();
    mSelInstanceData.clear();
    mInstanceData.clear();
    mSelPriorityData.clear();
    mPriorityData.clear();
    
    mActiveFilter = eDataFilter::NoFilter;
    mIndexStart = QModelIndex();
    mIndexEnd   = QModelIndex();
    emit signalFilterSelected(mIndexStart, mIndexEnd);
}
