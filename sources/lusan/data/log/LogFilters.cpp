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
 *  \file        lusan/data/log/LogFilters.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log viewer filter elements
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/data/log/LogFilters.hpp"
#include "LogFilters.hpp"

////////////////////////////////////////////////////////////////////////
// LogFilterPriorities class implementation
////////////////////////////////////////////////////////////////////////

LogFilterPriorities::LogFilterPriorities(void)
    : LogFilterBase(LogFilterBase::eFilterType::FilterPriority)
    , mFilterMask   ( 0x0000u )
    , mFilterList   ( )
{
    mFilterList.push_back({""       , static_cast<uint64_t>(NELogging::eLogPriority::PrioAny)       , false});
    mFilterList.push_back({"SCOPE"  , static_cast<uint64_t>(NELogging::eLogPriority::PrioScope)     , false});
    mFilterList.push_back({"DEBUG"  , static_cast<uint64_t>(NELogging::eLogPriority::PrioDebug)     , false});
    mFilterList.push_back({"INFO"   , static_cast<uint64_t>(NELogging::eLogPriority::PrioInfo)      , false});
    mFilterList.push_back({"WARN"   , static_cast<uint64_t>(NELogging::eLogPriority::PrioWarning)   , false});
    mFilterList.push_back({"ERROR"  , static_cast<uint64_t>(NELogging::eLogPriority::PrioError)     , false});
    mFilterList.push_back({"FATAL"  , static_cast<uint64_t>(NELogging::eLogPriority::PrioFatal)     , false});
}

LogFilterBase::eMatchResult LogFilterPriorities::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    return (mFilterMask == 0u) || ((static_cast<uint16_t>(logMessage.logMessagePrio) & mFilterMask) != 0) ? LogFilterBase::eMatchResult::MatchExact : LogFilterBase::eMatchResult::NoMatch;
}

void LogFilterPriorities::deactivateFilter(void)
{
    mFilterMask = 0x0000u;
    for (auto & filter : mFilterList)
    {
        filter.active = false;
    }
}

const QList<LogFilterBase::sFilterData>& LogFilterPriorities::getFilterList(void) const
{
    return mFilterList;
}

void LogFilterPriorities::activateFilters(const QStringList& filters)
{
    mFilterMask = 0x0000u;
    for (auto & filter : mFilterList)
    {
        filter.active = false;
        if (filters.indexOf(filter.data) >= 0)
        {
            filter.active = true;
            mFilterMask |= static_cast<uint16_t>(filter.value);
        }
    }
}

////////////////////////////////////////////////////////////////////////
// LogFilterScopes class implementation
////////////////////////////////////////////////////////////////////////

LogFilterScopes::LogFilterScopes(void)
    : LogFilterBase(LogFilterBase::eFilterType::FilterScope)
    , mScopes   ()
    , mFilters  ()
{
}

LogFilterBase::eMatchResult LogFilterScopes::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    constexpr uint32_t scopeMask = static_cast<uint32_t>(NELogging::eLogPriority::PrioLogs);

    if (mFilters.empty())
        return LogFilterBase::eMatchResult::MatchExact; // filter is not set, accept all

    auto it = mFilters.find(logMessage.logCookie);
    if ((it == mFilters.end()) || (it->second.empty()))
        return LogFilterBase::eMatchResult::MatchExact;    // no filter for this instance, accept all

    const ListScopes& scopes = it->second;
    for (const auto& scope : scopes)
    {
        if ((scope.scopeId == logMessage.logScopeId) && ((scope.scopePrio & static_cast<uint32_t>(logMessage.logMessagePrio) & scopeMask) != 0))
            return LogFilterBase::eMatchResult::MatchExact;
    }

    return LogFilterBase::eMatchResult::NoMatch;
}

void LogFilterScopes::deactivateFilter(void)
{
    mFilters.clear();
}

void LogFilterScopes::setData(ITEM_ID instId, const ListScopes& scopes)
{
    mScopes[instId] = scopes;
}

void LogFilterScopes::removeData(ITEM_ID instId)
{
    mScopes.erase(instId);
    mFilters.erase(instId);
}

void LogFilterScopes::activateFilters(ITEM_ID instId, const QList<uint32_t>& scopes)
{
    mFilters.erase(instId);
    if ((scopes.isEmpty()) || (mScopes.find(instId) == mScopes.end()))
        return;

    ListScopes& filterScopes = mFilters[instId];
    const ListScopes& allScopes = mScopes[instId];

    for (const auto& scope : allScopes)
    {
        if (scopes.indexOf(scope.scopeId) >= 0)
        {
            filterScopes.push_back(scope);
        }
    }
}

void LogFilterScopes::activateFilters(ITEM_ID instId, const std::vector<std::pair<uint32_t, uint32_t>>& scopes)
{
    mFilters.erase(instId);
    if ((scopes.empty()) || (mScopes.find(instId) == mScopes.end()))
        return;

    ListScopes& filterScopes = mFilters[instId];
    const ListScopes& allScopes = mScopes[instId];

    for (const auto& scope : allScopes)
    {
        for (const auto& scopePair : scopes)
        {
            if (scope.scopeId == scopePair.first)
            {
                NELogging::sScopeInfo filter{ scope };
                filter.scopePrio = scopePair.second; // update priority if needed
                filterScopes.push_back(filter);
                break;  // found, no need to check further
            }
        }
    }
}

const std::map<ITEM_ID, std::vector<NELogging::sScopeInfo>>& LogFilterScopes::getFilterList(void) const
{
    return mFilters;
}

////////////////////////////////////////////////////////////////////////
// LogFilterInstances  class declaration
////////////////////////////////////////////////////////////////////////

LogFilterInstances::LogFilterInstances(void)
    : LogFilterBase(LogFilterBase::eFilterType::FilterInstance)
    , mInstances( )
    , mFilters  ( )
{
}

LogFilterBase::eMatchResult LogFilterInstances::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    return (mFilters.isEmpty() || mFilters.indexOf(logMessage.logCookie) >= 0) ? LogFilterBase::eMatchResult::MatchExact : LogFilterBase::eMatchResult::NoMatch;
}

void LogFilterInstances::deactivateFilter(void)
{
    mFilters.clear();
}

void LogFilterInstances::setData(const std::vector<NEService::sServiceConnectedInstance>& instances)
{
    for (const auto& instance : instances)
    {
        setData(instance);
    }
}

void LogFilterInstances::setData(const NEService::sServiceConnectedInstance& instance)
{
    setData(instance.ciCookie, QString::fromStdString(instance.ciInstance));
}

void LogFilterInstances::setData(ITEM_ID instId, const QString& instName)
{
    for (const auto& inst : mInstances)
    {
        if (inst.value == static_cast<uint64_t>(instId))
            return; // already exists, no need to add
    }

    mInstances.push_back(LogFilterBase::sFilterData{ instName, instId, false });
}

void LogFilterInstances::removeData(ITEM_ID instId)
{
    for (auto it = mInstances.constBegin(); it != mInstances.constEnd(); ++ it)
    {
        if (instId == static_cast<ITEM_ID>(it->value))
        {
            mInstances.erase(it);
            break;
        }
    }

    for (auto it = mFilters.constBegin(); it != mFilters.constEnd(); ++ it)
    {
        if ((*it) == instId)
        {
            mFilters.erase(it);
            break;
        }
    }
}

void LogFilterInstances::activateFilters(const QList<ITEM_ID>& instId)
{
    mFilters.clear();
    for (auto& inst : mInstances)
    {
        inst.active = false; // deactivate all instances
        if (instId.indexOf(static_cast<ITEM_ID>(inst.value)) >= 0)
        {
            inst.active = true; // activate only those instances that are in the filter list
            mFilters.push_back(static_cast<ITEM_ID>(inst.value));
        }
    }
}

const QList<LogFilterBase::sFilterData>& LogFilterInstances::getFilterList(void) const
{
    return mInstances;
}

////////////////////////////////////////////////////////////////////////
// LogFilterTimestamp class implementation
////////////////////////////////////////////////////////////////////////

LogFilterTimestamp::LogFilterTimestamp(bool isTimeCreate)
    : LogFilterBase(isTimeCreate ? LogFilterBase::eFilterType::FilterTimeCreated : LogFilterBase::eFilterType::FilterTimeReceived)
    , mMinTime  (std::numeric_limits<uint64_t>::min())
    , mMaxTime  (std::numeric_limits<uint64_t>::max())
{
}

void LogFilterTimestamp::deactivateFilter(void)
{
    mMinTime = std::numeric_limits<uint64_t>::min();
    mMaxTime = std::numeric_limits<uint64_t>::max();
}

void LogFilterTimestamp::activateFilters(TIME64 minTime, TIME64 maxTime)
{
    mMinTime = minTime;
    mMaxTime = maxTime;
}

QPair<QString, QString> LogFilterTimestamp::getFilterList(void) const
{
    return QPair<QString, QString>(QString::number(mMinTime), QString::number(mMaxTime));
}

////////////////////////////////////////////////////////////////////////
// LogFilterTimeCreated class implementation
////////////////////////////////////////////////////////////////////////

LogFilterTimeCreated::LogFilterTimeCreated(void)
    : LogFilterTimestamp(true)
{
}

LogFilterBase::eMatchResult LogFilterTimeCreated::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    return (mMinTime <= logMessage.logTimestamp) && (logMessage.logTimestamp <= mMaxTime) ? LogFilterBase::eMatchResult::MatchExact : LogFilterBase::eMatchResult::NoMatch;
}

////////////////////////////////////////////////////////////////////////
// LogFilterTimeReceived class implementation
////////////////////////////////////////////////////////////////////////

LogFilterTimeReceived::LogFilterTimeReceived(void)
    : LogFilterTimestamp(false)
{
}

LogFilterBase::eMatchResult LogFilterTimeReceived::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    return (mMinTime <= logMessage.logReceived) && (logMessage.logReceived <= mMaxTime) ? LogFilterBase::eMatchResult::MatchExact : LogFilterBase::eMatchResult::NoMatch;
}
