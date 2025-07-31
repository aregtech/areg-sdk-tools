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

    if ((logMessage->logScopeId != mScopeData.value()) && mScopeData.valid())
        return eMatchType::NoMatch;
    else if ((logMessage->logScopeId == 0) && (mScopeData.valid() == false))
        return eMatchType::NoMatch;
    else if ((logMessage->logScopeId != mSelScopeData.value()) && (mScopeData.valid() == false))
        return eMatchType::NoMatch;
    
    if ((logMessage->logThreadId != mThreadData.value()) && mThreadData.valid())
        return eMatchType::NoMatch;
    else if ((logMessage->logThreadId == 0) && (mThreadData.valid() == false))
        return eMatchType::NoMatch;
    else if (logMessage->logThreadId != mSelThreadData.value())
        return eMatchType::PartialOutput;

    if ((logMessage->logCookie != mInstanceData.value()) && mInstanceData.valid())
        return eMatchType::NoMatch;
    else if ((logMessage->logCookie <= NEService::COOKIE_ANY) && (mInstanceData.valid() == false))
        return eMatchType::NoMatch;
    else if (logMessage->logCookie != mSelInstanceData.value())
        return eMatchType::PartialOutput;

    if ((logMessage->logSessionId != mSessionData.value()) && mSessionData.valid())
        return eMatchType::NoMatch;
    else if (logMessage->logSessionId != mSelSessionData.value())
        return eMatchType::PartialOutput;

    return eMatchType::ExactMatch;
}

void ScopeLogViewerFilter::ignoreSession(bool doIgnore)
{
    if (sourceModel() == nullptr)
        return;

    if (doIgnore && mSessionData.valid())
    {
        mSessionData.clear();
        invalidateFilter();
    }
    else if ((doIgnore == false) && (mSessionData.valid() == false))
    {
        mSessionData = mSelSessionData;
        invalidateFilter();
    }
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
}
