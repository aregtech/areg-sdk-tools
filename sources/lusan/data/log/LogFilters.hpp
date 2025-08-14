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
#include "areg/component/NEService.hpp"
#include <QPair>

////////////////////////////////////////////////////////////////////////
// LogFilterPriorities  class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterPriorities : public LogFilterBase
{
////////////////////////////////////////////////////////////////////////
// Internal types and constants
////////////////////////////////////////////////////////////////////////
public:
    LogFilterPriorities(void);

    virtual ~LogFilterPriorities(void) = default;

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

    virtual void deactivateFilter(void) override;

////////////////////////////////////////////////////////////////////////
// Attributes and operations
////////////////////////////////////////////////////////////////////////
public:

    const QList<LogFilterBase::sFilterData>& getFilterList(void) const;

    void activateFilters(const QStringList& filters);
    
private:
    uint16_t    mFilterMask;
    FilterList  mFilterList;
};

////////////////////////////////////////////////////////////////////////
// LogFilterScopes  class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterScopes : public LogFilterBase
{
    using   ListScopes = std::vector< NELogging::sScopeInfo>;
    using   MapScopes = std::map<ITEM_ID, ListScopes>;

public:
        LogFilterScopes(void);
        virtual ~LogFilterScopes(void) = default;

public:
    /**
     * \brief   Checks whether the log message passes the filter.
     * \param   logMessage  The log message to check.
     * \return  True if the log message passes the filter, false otherwise.
     **/
    virtual LogFilterBase::eMatchResult isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const override;

    virtual void deactivateFilter(void) override;

public:
    void setData(ITEM_ID instId, const ListScopes& scopes);

    void removeData(ITEM_ID instId);

    void activateFilters(ITEM_ID instId, const QList<uint32_t>& scopes);

    void activateFilters(ITEM_ID instId, const std::vector<std::pair<uint32_t, uint32_t>>& scopes);

    const std::map<ITEM_ID, std::vector<NELogging::sScopeInfo>>& getFilterList(void) const;

private:
    MapScopes   mScopes;
    MapScopes   mFilters;
};

////////////////////////////////////////////////////////////////////////
// LogFilterInstances  class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterInstances : public LogFilterBase
{
public:
    LogFilterInstances(void);
    virtual ~LogFilterInstances(void) = default;

public:
    /**
     * \brief   Checks whether the log message passes the filter.
     * \param   logMessage  The log message to check.
     * \return  True if the log message passes the filter, false otherwise.
     **/
    virtual LogFilterBase::eMatchResult isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const override;

    virtual void deactivateFilter(void) override;

public:
    void setData(const std::vector< NEService::sServiceConnectedInstance> & instances);

    void setData(const NEService::sServiceConnectedInstance& instance);

    void setData(ITEM_ID instId, const QString & instName);

    void removeData(ITEM_ID instId);

    void activateFilters(const QList<ITEM_ID> & instId);

    const QList<LogFilterBase::sFilterData> & getFilterList(void) const;

private:
    QList<sFilterData>  mInstances;
    QList<ITEM_ID>      mFilters;
};

////////////////////////////////////////////////////////////////////////
// LogFilterTimestamp class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterTimestamp : public LogFilterBase
{
protected:
    LogFilterTimestamp(bool isTimeCreate);
    virtual ~LogFilterTimestamp(void) = default;

public:
    /**
     * \brief   Checks whether the log message passes the filter.
     * \param   logMessage  The log message to check.
     * \return  True if the log message passes the filter, false otherwise.
     **/
    virtual LogFilterBase::eMatchResult isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const override = 0;

    virtual void deactivateFilter(void) override;

public:
    void activateFilters(TIME64 minTime, TIME64 maxTime);

    QPair<QString, QString> getFilterList(void) const;

protected:
    TIME64  mMinTime;
    TIME64  mMaxTime;
};

////////////////////////////////////////////////////////////////////////
// LogFilterTimeCreated class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterTimeCreated : public LogFilterTimestamp
{
public:
    LogFilterTimeCreated(void);
    virtual ~LogFilterTimeCreated(void) = default;

public:
    /**
     * \brief   Checks whether the log message passes the filter.
     * \param   logMessage  The log message to check.
     * \return  True if the log message passes the filter, false otherwise.
     **/
    virtual LogFilterBase::eMatchResult isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const override;
};

////////////////////////////////////////////////////////////////////////
// LogFilterTimeReceived class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterTimeReceived : public LogFilterTimestamp
{
public:
    LogFilterTimeReceived(void);
    virtual ~LogFilterTimeReceived(void) = default;

public:
    /**
     * \brief   Checks whether the log message passes the filter.
     * \param   logMessage  The log message to check.
     * \return  True if the log message passes the filter, false otherwise.
     **/
    virtual LogFilterBase::eMatchResult isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const override;
};

#endif  // LUSAN_DATA_LOG_LOGFILTERS_HPP
