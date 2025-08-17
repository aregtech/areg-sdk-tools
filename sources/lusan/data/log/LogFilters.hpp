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

/**
 * \brief   This object if filtering log message by priority
 **/
class LogFilterPriorities : public LogFilterBase
{
////////////////////////////////////////////////////////////////////////
// Internal types and constants
////////////////////////////////////////////////////////////////////////
private:
    using PrioList  = QList<uint16_t>;

public:
    LogFilterPriorities(void);

    virtual ~LogFilterPriorities(void) = default;

////////////////////////////////////////////////////////////////////////
// Attributes and operations
////////////////////////////////////////////////////////////////////////
public:

    inline const QStringList& getData(void) const;

    inline const QStringList& getDisplayNames(void) const;

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

    /**
     * \brief   Deactivates the filter, clearing all active filters.
     *          After deactivation, all log messages will be accepted.
     **/
    virtual void deactivateFilter(void) override;

    /**
     * \brief   Activates the filter with the specified filter list.
     *          The filter list contains the filters to be applied.
     *          If the list of filters is empty, the filter will be deactivated
     *          and all log messages will be accepted.
     * \param   filters     The list of filters to activate.
     **/
    virtual void activateFilter(const LogFilterBase::FilterList& filters) override;
    
private:
    uint16_t    mMask;
    QStringList mData;
    PrioList    mPrios;
};

////////////////////////////////////////////////////////////////////////
// LogFilterScopes  class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterScopes : public LogFilterBase
{
////////////////////////////////////////////////////////////////////////
// Internal types and constants
////////////////////////////////////////////////////////////////////////
private:
    using   ListScopes      = std::vector<uint32_t>;
    using   MapScopes       = std::map<ITEM_ID, ListScopes>;

public:
    using   ListScopeInfo   = std::vector<NELogging::sScopeInfo>;
    using   MapScopeInfo    = std::map<ITEM_ID, ListScopeInfo>;

public:
        LogFilterScopes(void);
        virtual ~LogFilterScopes(void) = default;

////////////////////////////////////////////////////////////////////////
// Attributes and operations
////////////////////////////////////////////////////////////////////////
public:

    inline const LogFilterScopes::MapScopeInfo& getData(void) const;

    inline const LogFilterScopes::MapScopeInfo& getDisplayNames(void) const;

    inline void updateScopeData(ITEM_ID inst, const std::vector<NELogging::sScopeInfo>& listScopeInfo);

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

    /**
     * \brief   Deactivates the filter, clearing all active filters.
     *          After deactivation, all log messages will be accepted.
     **/
    virtual void deactivateFilter(void) override;

    /**
     * \brief   Activates the filter with the specified filter list.
     *          The filter list contains the filters to be applied.
     *          If the list of filters is empty, the filter will be deactivated
     *          and all log messages will be accepted.
     * \param   filters     The list of filters to activate.
     **/
    virtual void activateFilter(const LogFilterBase::FilterList& filters) override;

private:
    MapScopes       mScopes;
    MapScopeInfo    mData;
};

////////////////////////////////////////////////////////////////////////
// LogFilterInstances  class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterInstances : public LogFilterBase
{
public:
    LogFilterInstances(void);
    virtual ~LogFilterInstances(void) = default;

////////////////////////////////////////////////////////////////////////
// Attributes and operations
////////////////////////////////////////////////////////////////////////
public:

    inline const QStringList& getData(bool isNames) const;

    inline const QStringList& getDisplayNames(bool isNames) const;

    void updateInstanceData(const NEService::sServiceConnectedInstance& instance);

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

private:
    QStringList     mNames;
    QStringList     mIds;
    QStringList     mDispNames;
    QStringList     mDispIds;
};

////////////////////////////////////////////////////////////////////////
// LogFilterTimeCreated class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterTimeCreated : public LogFilterBase
{
////////////////////////////////////////////////////////////////////////
// Internal types and constants
////////////////////////////////////////////////////////////////////////
public:
    static constexpr int MIN_TIME   { 0 };  //!< The index of the minimum timestamp filter
    static constexpr int MAX_TIME   { 1 };  //!< The index of the maximum timestamp filter

public:
    LogFilterTimeCreated(void);
    virtual ~LogFilterTimeCreated(void) = default;

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
};

////////////////////////////////////////////////////////////////////////
// LogFilterTimeReceived class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterTimeReceived : public LogFilterBase
{
////////////////////////////////////////////////////////////////////////
// Internal types and constants
////////////////////////////////////////////////////////////////////////
public:
    static constexpr int MIN_TIME   { 0 };  //!< The index of the minimum timestamp filter
    static constexpr int MAX_TIME   { 1 };  //!< The index of the maximum timestamp filter

public:
    LogFilterTimeReceived(void);
    virtual ~LogFilterTimeReceived(void) = default;

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
};

////////////////////////////////////////////////////////////////////////
// LogFilterDuration class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterDuration : public LogFilterBase
{
public:
    LogFilterDuration(void);
    virtual ~LogFilterDuration(void) = default;

    inline const QStringList& getData(void) const;

    inline const QStringList& getDisplayNames(void) const;

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

    /**
     * \brief   Deactivates the filter, clearing all active filters.
     *          After deactivation, all log messages will be accepted.
     **/
    virtual void deactivateFilter(void) override;

    /**
     * \brief   Activates the filter with the specified filter list.
     *          The filter list contains the filters to be applied.
     *          If the list of filters is empty, the filter will be deactivated
     *          and all log messages will be accepted.
     * \param   filters     The list of filters to activate.
     **/
    virtual void activateFilter(const LogFilterBase::FilterList& filters) override;

private:
    uint32_t    mDuration; //!< The duration in milliseconds
    QStringList mData;
};

////////////////////////////////////////////////////////////////////////
// LogFilterThread class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterThread : public LogFilterBase
{
////////////////////////////////////////////////////////////////////////
// Internal types and constants
////////////////////////////////////////////////////////////////////////
public:
    using   ListThreads = FilterList;
    using   MapThreads  = QMap<ITEM_ID, ListThreads>;

public:
    LogFilterThread(void);
    virtual ~LogFilterThread(void) = default;

////////////////////////////////////////////////////////////////////////
// Attributes and operations
////////////////////////////////////////////////////////////////////////
public:

    inline const LogFilterThread::MapThreads& getData(void) const;

    inline const QStringList& getDisplayNames(bool names) const;

    inline void updateThreadData(const NELogging::sLogMessage& logMessage);

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
    
    /**
     * \brief   Deactivates the filter, clearing all active filters.
     *          After deactivation, all log messages will be accepted.
     **/
    virtual void deactivateFilter(void) override;
    
    /**
     * \brief   Activates the filter with the specified filter list.
     *          The filter list contains the filters to be applied.
     *          If the list of filters is empty, the filter will be deactivated
     *          and all log messages will be accepted.
     * \param   filters     The list of filters to activate.
     **/
    virtual void activateFilter(const LogFilterBase::FilterList& filters) override;
    
private:
    MapThreads  mThreads;       //!< Map of thread IDs to thread names
    MapThreads  mDataThreads;       //!< Map of thread IDs to thread names
    QStringList mDispNames;
    QStringList mDispIds;
};

////////////////////////////////////////////////////////////////////////
// LogFilterText class declaration
////////////////////////////////////////////////////////////////////////

class LogFilterText : public LogFilterBase
{
public:
    LogFilterText(void);
    virtual ~LogFilterText(void) = default;

    inline const QString getDisplayName(bool isNames) const;

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

    /**
     * \brief   Deactivates the filter, clearing all active filters.
     *          After deactivation, all log messages will be accepted.
     **/
    virtual void deactivateFilter(void) override;

    /**
     * \brief   Activates the filter with the specified filter list.
     *          The filter list contains the filters to be applied.
     *          If the list of filters is empty, the filter will be deactivated
     *          and all log messages will be accepted.
     * \param   filters     The list of filters to activate.
     **/
    virtual void activateFilter(const LogFilterBase::FilterList& filters) override;

////////////////////////////////////////////////////////////////////////
// Hidden methods
////////////////////////////////////////////////////////////////////////
private:

    inline int containsExact(const char* str) const;

    inline int containsInsensitive(const char * str) const;

    inline int containsWord(const char* str) const;

    inline int containsWildCard(const QString & text) const;
    
////////////////////////////////////////////////////////////////////////
// Member variables
////////////////////////////////////////////////////////////////////////
private:
    std::string         mSearch;
    bool                mIsSensitive;
    bool                mIsWholeWord;
    bool                mIsRegularEx;
    QRegularExpression  mRegularEx;     //<!< Regular expression for the filter, if applicable
};

/************************************************************************
 * Inline methods.
 ************************************************************************/

////////////////////////////////////////////////////////////////////////
// LogFilterPriorities inline methods
////////////////////////////////////////////////////////////////////////

inline const QStringList& LogFilterPriorities::getData(void) const
{
    return mData;
}

inline const QStringList& LogFilterPriorities::getDisplayNames(void) const
{
    return mData;
}

////////////////////////////////////////////////////////////////////////
// LogFilterScopes inline methods
////////////////////////////////////////////////////////////////////////

inline const LogFilterScopes::MapScopeInfo& LogFilterScopes::getData(void) const
{
    return mData;
}

inline const LogFilterScopes::MapScopeInfo& LogFilterScopes::getDisplayNames(void) const
{
    return mData;
}

inline void LogFilterScopes::updateScopeData(ITEM_ID inst, const std::vector<NELogging::sScopeInfo>& listScopeInfo)
{
    mData[inst] = listScopeInfo;
}

////////////////////////////////////////////////////////////////////////
// LogFilterInstances inline methods
////////////////////////////////////////////////////////////////////////

inline const QStringList& LogFilterInstances::getData(bool isNames) const
{
    return (isNames ? mNames : mIds);
}

const QStringList& LogFilterInstances::getDisplayNames(bool isNames) const
{
    return (isNames ? mDispNames : mDispIds);
}

////////////////////////////////////////////////////////////////////////
// LogFilterThread inline methods
////////////////////////////////////////////////////////////////////////

inline const LogFilterThread::MapThreads& LogFilterThread::getData(void) const
{
    return mDataThreads;
}

////////////////////////////////////////////////////////////////////////
// LogFilterDuration inline methods
////////////////////////////////////////////////////////////////////////

inline const QStringList& LogFilterDuration::getData(void) const
{
    return mData;
}

inline const QStringList& LogFilterDuration::getDisplayNames(void) const
{
    return mData;
}

////////////////////////////////////////////////////////////////////////
// LogFilterText inline methods
////////////////////////////////////////////////////////////////////////

inline const QString LogFilterText::getDisplayName(bool isNames) const
{
    return (mSearch.empty() ? QString() : QString::fromStdString(mSearch));
}

#endif  // LUSAN_DATA_LOG_LOGFILTERS_HPP
