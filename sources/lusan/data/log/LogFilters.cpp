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

LogFilterPriority::LogFilterPriority(void)
    : LogFilterBase(LogFilterBase::eFilterType::FilterPriority)
    , mFilterMask   ( 0u )
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

QList<LogFilterBase::sFilterData> LogFilterPriority::filterList(void) const
{
    return mFilterList;
}

QString LogFilterPriority::filterData(void) const
{
    return QString();
}

void LogFilterPriority::activateFilters(const QStringList& filters)
{
    mFilterMask = 0u;
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

LogFilterBase::eMatchResult LogFilterPriority::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    return ((static_cast<uint16_t>(logMessage.logMessagePrio) & mFilterMask) != 0) ? LogFilterBase::eMatchResult::MatchExact : LogFilterBase::eMatchResult::NoMatch;
}

void LogFilterPriority::activateFilter(const QString& /*filter*/, bool /*isCaseSensitive*/, bool /*isWholeWord*/, bool /*isRegEx*/)
{
}

void LogFilterPriority::deactivateFilter(void)
{
    mFilterMask = 0u;
    for (auto & filter : mFilterList)
    {
        filter.active = false;
    }
}
