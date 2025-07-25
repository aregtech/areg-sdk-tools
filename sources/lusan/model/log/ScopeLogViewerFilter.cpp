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
    , mScopeId       (scopeId)
    , mPriorities    (0)
    , mSessionIds    ( )
    , mInstanceIds   ( )
{
}

ScopeLogViewerFilter::~ScopeLogViewerFilter(void)
{
    _clearData();
}

void ScopeLogViewerFilter::setScopeFilter(LoggingModelBase *model, const QModelIndex& index)
{
    const NELogging::sLogMessage* logMessage = index.data(Qt::ItemDataRole::UserRole).value<const NELogging::sLogMessage *>();
    if (logMessage != nullptr)
    {
        std::vector<uint32_t> sessionIds;
        sessionIds.push_back(logMessage->logSessionId);
        std::vector<ITEM_ID> instances;
        instances.push_back(logMessage->logCookie);
        setScopeFilter(model, logMessage->logScopeId, sessionIds, instances, 0);
    }
    else
    {
        setScopeFilter(nullptr, 0, std::vector<uint32_t>(), std::vector<ITEM_ID>(), 0);
    }
}

void ScopeLogViewerFilter::setScopeFilter(LoggingModelBase * model, uint32_t scopeId, const std::vector<uint32_t>& sessionids, const std::vector<ITEM_ID>& instances, uint32_t priorities)
{
    setSourceModel(nullptr);
    clearFilters();
    mScopeId       = scopeId;
    mSessionIds    = sessionids;
    mInstanceIds   = instances;
    mPriorities    = priorities;
    
    if (std::find(mInstanceIds.begin(), mInstanceIds.end(), NEService::COOKIE_ANY) != mInstanceIds.end())
        mInstanceIds.clear();
    
    setSourceModel(model);
    invalidateFilter();
}

void ScopeLogViewerFilter::clearFilters(void)
{
    _clearData();
    LogViewerFilter::clearFilters();
}

bool ScopeLogViewerFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    return (sourceModel() == nullptr) ||
           (matchesScopeFilter(source_row, source_parent) && LogViewerFilter::filterAcceptsRow(source_row, source_parent));
}

bool ScopeLogViewerFilter::matchesScopeFilter(int source_row, const QModelIndex& source_parent) const
{
    if (mScopeId == 0)
        return true; // No scope filter applied

    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if (!index.isValid())
        return false;

    const NELogging::sLogMessage* logMessage = index.data(Qt::ItemDataRole::UserRole).value<const NELogging::sLogMessage*>();
    if (logMessage == nullptr)
        return false;

    if (logMessage->logScopeId != mScopeId)
        return false;
    
    if ((mSessionIds.empty() == false) && (std::find(mSessionIds.begin(), mSessionIds.end(), logMessage->logSessionId) == mSessionIds.end()))
        return false;
    
    if ((mInstanceIds.empty() == false) && (std::find(mInstanceIds.begin(), mInstanceIds.end(), logMessage->logCookie) == mInstanceIds.end()))
        return false;
    
    return (mPriorities == 0) || ((mPriorities & static_cast<uint32_t>(logMessage->logMessagePrio)) != 0);
}

inline void ScopeLogViewerFilter::_clearData(void)
{
    mSessionIds.clear();
    mInstanceIds.clear();
    mPriorities = 0u;
}
