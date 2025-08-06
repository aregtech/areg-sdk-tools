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
 *  \file        lusan/model/log/ScopeLogViewerFilter.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope Output Viewer Filter Proxy Model.
 *
 ************************************************************************/

#include "lusan/model/log/ScopeLogViewerFilter.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"

#include "areg/logging/NELogging.hpp"
#include "areg/component/NEService.hpp"

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
{
}

ScopeLogViewerFilter::~ScopeLogViewerFilter(void)
{
    _clearData();
}

void ScopeLogViewerFilter::setScopeFilter(LoggingModelBase *model, const QModelIndex& index)
{
    const NELogging::sLogMessage* logMessage = index.data(Qt::ItemDataRole::UserRole).value<const NELogging::sLogMessage *>();
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
}

void ScopeLogViewerFilter::clearFilters(void)
{
    _clearData();
    LogViewerFilter::clearFilters();
}

bool ScopeLogViewerFilter::filterExactMatch(const QModelIndex& index) const
{
    return (matchesScopeFilter(index) == eMatchType::ExactMatch);
}

bool ScopeLogViewerFilter::filterAcceptsRow(int row, const QModelIndex& parent) const
{
    QModelIndex index = sourceModel() != nullptr ? sourceModel()->index(row, 0, parent) : QModelIndex();
    return ((matchesScopeFilter(index) != eMatchType::NoMatch) && LogViewerFilter::filterAcceptsRow(row, parent));
}

LogViewerFilter::eMatchType ScopeLogViewerFilter::matchesScopeFilter(const QModelIndex& index) const
{
    if ((mSelScopeData.valid() == false) || (sourceModel() == nullptr))
        return eMatchType::PartialMatch; // No scope filter applied
    else if (index.isValid() == false)
        return eMatchType::NoMatch;
    
    const NELogging::sLogMessage* logMessage = index.data(Qt::ItemDataRole::UserRole).value<const NELogging::sLogMessage*>();
    if (logMessage == nullptr)
        return eMatchType::NoMatch;
    
    if ((logMessage->logCookie != mInstanceData.value()) && mInstanceData.valid())
        return eMatchType::NoMatch;
    else if ((logMessage->logCookie <= NEService::COOKIE_ANY) && (mInstanceData.valid() == false))
        return eMatchType::NoMatch;
    else if (logMessage->logCookie != mSelInstanceData.value())
        return eMatchType::PartialOutput;

    if ((logMessage->logThreadId != mThreadData.value()) && mThreadData.valid())
        return eMatchType::NoMatch;
    else if ((logMessage->logThreadId == 0) && (mThreadData.valid() == false))
        return eMatchType::NoMatch;
    else if ((logMessage->logThreadId != mSelThreadData.value()) && (mThreadData.valid() == false))
    {
        if (mInstanceData.valid() && mSessionData.valid())
            return eMatchType::NoMatch;
    }

    if ((logMessage->logScopeId != mScopeData.value()) && mScopeData.valid())
    {
        if (mActiveFilter == eDataFilter::FilterSublogs)
        {
            if ((mIndexStart.isValid() && (mIndexEnd.isValid() == false)) || ((mIndexStart.row() < index.row()) && (index.row() < mIndexEnd.row())))
                return eMatchType::PartialOutput;
        }

        return eMatchType::NoMatch;
    }
    else if ((logMessage->logScopeId == 0) && (mScopeData.valid() == false))
    {
        return eMatchType::NoMatch;
    }
    else if ((logMessage->logScopeId != mSelScopeData.value()) && (mScopeData.valid() == false))
    {
        if (mThreadData.valid() && mInstanceData.valid() && mSessionData.valid())
            return eMatchType::NoMatch;

        return eMatchType::PartialOutput;
    }

    if ((logMessage->logSessionId != mSessionData.value()) && mSessionData.valid())
    {
        if (mActiveFilter == eDataFilter::FilterSublogs)
        {
            Q_ASSERT(logMessage->logThreadId == mThreadData.value());
            if ((mIndexStart.isValid() && (mIndexEnd.isValid() == false)) || ((mIndexStart.row() < index.row()) && (index.row() < mIndexEnd.row())))
                return eMatchType::PartialOutput;
        }
        
        return eMatchType::NoMatch;
    }
    else if (logMessage->logSessionId != mSelSessionData.value())
    {
        return eMatchType::PartialOutput;
    }

    Q_ASSERT(logMessage->logSessionId == mSelSessionData.value());
    if (logMessage->logMsgType == NELogging::eLogMessageType::LogMessageScopeEnter)
    {
        mIndexStart = index;
        if (mIndexEnd.isValid() == false)
        {
            emit signalFilterSelected(mIndexStart, mIndexEnd);
        }
    }
    else if (logMessage->logMsgType == NELogging::eLogMessageType::LogMessageText)
    {
        if (mIndexStart.isValid() == false)
        {
            mIndexStart = index;
            if (mIndexEnd.isValid() == false)
            {
                emit signalFilterSelected(mIndexStart, mIndexEnd);
            }
        }
    }
    else if (logMessage->logMsgType == NELogging::eLogMessageType::LogMessageScopeExit)
    {
        mIndexEnd = index;
        emit signalFilterSelected(mIndexStart, mIndexEnd);
    }
        
    return eMatchType::ExactMatch;
}

void ScopeLogViewerFilter::filterData(ScopeLogViewerFilter::eDataFilter dataFilter)
{
    mIndexStart = QModelIndex();
    mIndexEnd   = QModelIndex();
    QAbstractItemModel* model = sourceModel();
    
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
    invalidateFilter();
}

inline void ScopeLogViewerFilter::_clearData(void)
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
