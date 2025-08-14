#ifndef LUSAN_DATA_LOG_LOGFILTERS_HPP
#define LUSAN_DATA_LOG_LOGFILTERS_HPP
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
#include "lusan/data/log/LogFilterBase.hpp"

class LogFilterPriority : public LogFilterBase
{
////////////////////////////////////////////////////////////////////////
// Internal types and constants
////////////////////////////////////////////////////////////////////////
public:
    LogFilterPriority(void);

    virtual ~LogFilterPriority(void) = default;

////////////////////////////////////////////////////////////////////////
// Overrides
////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Checks whether the log message passes the filter.
     * \param   logMessage  The log message to check.
     * \return  True if the log message passes the filter, false otherwise.
     **/
    virtual LogFilterBase::eMatchResult isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const override;

    virtual QList<LogFilterBase::sFilterData> filterList(void) const override;

    virtual QString filterData(void) const override;

    virtual void activateFilters(const QStringList& filters) override;
    
    virtual void activateFilter(const QString& filter, bool isCaseSensitive, bool isWholeWord, bool isRegEx) override;
    
    virtual void deactivateFilter(void) override;
    
private:
    uint16_t    mFilterMask;
    FilterList  mFilterList;
};

class LogFilterTimeCreated : public LogFilterBase
{
    
};

#endif  // LUSAN_DATA_LOG_LOGFILTERS_HPP
