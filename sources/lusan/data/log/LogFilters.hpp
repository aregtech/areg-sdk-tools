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

#include <QList>
#include <QMap>
#include <QPair>
#include <QRegularExpression>

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

////////////////////////////////////////////////////////////////////////
// LogFilterDuration class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterDuration : public LogFilterBase
{
public:
    LogFilterDuration(void);
    virtual ~LogFilterDuration(void) = default;

public:
    /**
     * \brief   Checks whether the log message passes the filter.
     * \param   logMessage  The log message to check.
     * \return  True if the log message passes the filter, false otherwise.
     **/
    virtual LogFilterBase::eMatchResult isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const override;

    virtual void deactivateFilter(void) override;

public:
    void activateFilter(uint32_t duration);

    void activateFilter(const QString& duration);

    QString getFilter(void) const;

private:
    uint32_t    mDuration; //!< The duration in milliseconds
};

////////////////////////////////////////////////////////////////////////
// LogFilterThread class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterThread : public LogFilterBase
{
    using   ListThreads = QList<LogFilterBase::sFilterData>;
    using   MapThreads  = QMap<ITEM_ID, ListThreads>;

public:
    LogFilterThread(void);
    virtual ~LogFilterThread(void) = default;

public:
    /**
     * \brief   Checks whether the log message passes the filter.
     * \param   logMessage  The log message to check.
     * \return  True if the log message passes the filter, false otherwise.
     **/
    virtual LogFilterBase::eMatchResult isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const override;

    virtual void deactivateFilter(void) override;

public:

    void setData(ITEM_ID source, ITEM_ID threadId, const QString& threadName);

    void setData(const NELogging::sLogMessage& logMessage);

    void activateFilter(const QList<QString>& threadNames);

    void activateFilter(const QList<ITEM_ID>& threadIds);

    QMap<ITEM_ID, QList<LogFilterBase::sFilterData>> getFilterNames(void) const;

private:
    MapThreads  mThreads; //!< Map of thread IDs to thread names
    ListThreads mFilters; //!< Map of active thread IDs to thread names
};

////////////////////////////////////////////////////////////////////////
// LogFilterText class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterText : public LogFilterBase
{
public:
    struct sFilterData
    {
        std::string data        {       };  //!< The filter data as a string
        bool        isSensitive { false };  //!< Flag indicating if the filter is case sensitive
        bool        isWholeWord { false };  //!< Flag indicating if the filter is for whole words only
        bool        isRegEx     { false };  //!< Flag indicating if the filter is a regular expression
    };

public:
    LogFilterText(void);
    virtual ~LogFilterText(void) = default;

public:
    /**
     * \brief   Checks whether the log message passes the filter.
     * \param   logMessage  The log message to check.
     * \return  True if the log message passes the filter, false otherwise.
     **/
    virtual LogFilterBase::eMatchResult isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const override;

    virtual void deactivateFilter(void) override;

public:

    void activateFilter(const QString & filter, bool isCaseSensitive, bool isWholeWord, bool isRegEx);

    const sFilterData & getFilter(void) const;

private:

    inline int containsExact(const char* str) const;

    inline int containsInsensitive(const char * str) const;

    inline int containsWord(const char* str) const;

    inline int containsWildCard(const QString & text) const;

private:
    sFilterData         mFilter;//!< The filter data
    QRegularExpression  mRegEx; //<!< Regular expression for the filter, if applicable
};

#endif  // LUSAN_DATA_LOG_LOGFILTERS_HPP
