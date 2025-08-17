#ifndef LUSAN_DATA_LOG_LOGFILTERBASE_HPP
#define LUSAN_DATA_LOG_LOGFILTERBASE_HPP
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
 *  \file        lusan/data/log/LogFilterBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log viewer filter element base class.
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"
#include "areg/logging/NELogging.hpp"

#include <QString>
#include <QList>

class LogFilterBase
{
////////////////////////////////////////////////////////////////////////
// Internal types and constants
////////////////////////////////////////////////////////////////////////
public:

    enum eFilterType
    {
          FilterUnknown         = 0x00u //!< Unknown filter type
        , FilterPriority        = 0x01u //!< Filter by priority
        , FilterScope           = 0x02u //!< Filter by scope
        , FilterInstance        = 0x03u //!< Filter by instance
        , FilterTimeCreated     = 0x04u //!< Filter by date created
        , FilterTimeReceived    = 0x05u //!< Filter by date received
        , FilterDuration        = 0x06u //!< Filter by duration
        , FilterThread          = 0x07u //!< Filter by thread
        , FilterMessage         = 0x08u //!< Filter by message text
        , FilterSession         = 0x09u //!< Filter by session
    };

    enum eMatchResult
    {
          NoMatch           = 0 //!< Unknown filter match
        , MatchExact        = 1 //!< Exact match
        , MatchPartial      = 2 //!< Contains match
    };
    
    enum eVisualType
    {
          VisualUndefined   = 0
        , VisualText        = 1
        , VisualList        = 2
        , VisualTree        = 3
        , VisualRange       = 4
    };

    struct sTextSrch
    {
        bool    isSensitive { false };
        bool    isWholeWord { false };
        bool    isRegularEx { false };
    };
    
    union uValue
    {
        sTextSrch   srch;
        uint64_t    digit;
    };

    struct sFilterData
    {
        ITEM_ID     source{ 0u };   //!< The source of the filter, e.g., instance ID
        QString     data  {    };   //!< The filter data as a string
        uValue      value {    };   //!< The filter digital value  
    };

    using FilterList = QList<sFilterData>;

protected:
////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
////////////////////////////////////////////////////////////////////////
    /**
     * \brief   Default constructor.
     **/
    LogFilterBase(eFilterType filter, eVisualType visual);

    /**
     * \brief   Destructor.
     **/
    virtual ~LogFilterBase(void) = default;

////////////////////////////////////////////////////////////////////////
// Overrides
////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Checks whether the log message passes the filter.
     * \param   logMessage  The log message to check.
     * \return  True if the log message passes the filter, false otherwise.
     **/
    virtual LogFilterBase::eMatchResult isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const = 0;

    /**
     * \brief   Deactivates the filter, clearing all active filters.
     *          After deactivation, all log messages will be accepted.
     **/
    virtual void deactivateFilter(void);

    /**
     * \brief   Activates the filter with the specified filter list.
     *          The filter list contains the filters to be applied.
     *          If the list of filters is empty, the filter will be deactivated
     *          and all log messages will be accepted.
     * \param   filters     The list of filters to activate.
     **/
    virtual void activateFilter(const LogFilterBase::FilterList & filters);

////////////////////////////////////////////////////////////////////////
// Attributes and operations
////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the type of the filter.
     **/
    inline LogFilterBase::eFilterType filterType(void) const;
    
    inline const LogFilterBase::eVisualType filterVisual(void) const;
        
    /**
     * \brief   Returns the list of active filters.
     *          If no filters are set, it returns an empty list.
     **/
    inline const LogFilterBase::FilterList & getFilter(void) const;
    
////////////////////////////////////////////////////////////////////////
// Member variables
////////////////////////////////////////////////////////////////////////
protected:
    const eFilterType   mFilterType;//!< The type of the filter
    const eVisualType   mVisualType;//!< The visual type of the filter
    FilterList          mFilters;   //!< The list of active filters
};

////////////////////////////////////////////////////////////////////////
// LogFilterBase inline methods
////////////////////////////////////////////////////////////////////////

inline LogFilterBase::eFilterType LogFilterBase::filterType(void) const
{
    return mFilterType;
}

inline const LogFilterBase::eVisualType LogFilterBase::filterVisual(void) const
{
    return mVisualType;
}

inline const LogFilterBase::FilterList & LogFilterBase::getFilter(void) const
{
    return mFilters;
}

#endif  // LUSAN_DATA_LOG_LOGFILTERBASE_HPP
