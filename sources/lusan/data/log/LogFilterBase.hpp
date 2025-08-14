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

    enum eFilterData
    {
          NoData            = 0 //!< No data for filter 
        , DataInteger       = 1 //!< Integer data for filter
        , DataString        = 2 //!< String data for filter
        , DataStringList    = 3 //!< List of strings for filter
        , DataIntegerList   = 4 //!< List of integers for filter
    };

    enum eFilterFields
    {
          FiledInvalid      = 0xFFFFu   //!< Invalid field index
        , FieldPriority     = 0x0000u   //!< Filter by priority
        , FieldTimeCreated  = 0x0001u   //!< Filter by time created
        , FieldTimeReceived = 0x0002u   //!< Filter by time received
        , FieldDuration     = 0x0003u   //!< Filter by duration
        , FieldInstance     = 0x0004u   //!< Filter by instance
        , FieldScope        = 0x0005u   //!< Filter by scope
        , FieldThread       = 0x0006u   //!< Filter by thread
        , FieldMessage      = 0x0007u   //!< Filter by message text

        , FiledCount        = 0x0008u   //!< Total number of fields in the filter
    };

    enum eFieldMask
    {
          NoMask            = 0x0000u   //!< No filter mask
        , MaskPriority      = 0x0003u   //!< Filter by priority
        , NaskTimeCreated   = 0x000Cu   //!< Filter by time created
        , MaskTimeReceived  = 0x0030u   //!< Filter by time received
        , MaskDuration      = 0x00C0u   //!< Filter by duration
        , MaskInstance      = 0x0300u   //!< Filter by instance
        , MaskScope         = 0x0C00u   //!< Filter by scope
        , MaskThread        = 0x3000u   //!< Filter by thread
        , MaskMessage       = 0xC000u   //!< Filter by message text
    };

    enum eFieldReset
    {
          NoReset           = 0xFFFFu   //!< No reset
        , ResetPriority     = 0xFFFCu   //!< Reset priority filter
        , ResetTimeCreated  = 0xFFF3u   //!< Reset time created filter
        , ResetTimeReceived = 0xFFCFu   //!< Reset time received filter
        , ResetDuration     = 0xFF3Fu   //!< Reset duration filter
        , ResetInstance     = 0xFCFFu   //!< Reset instance filter
        , ResetScope        = 0xCFFFu   //!< Reset scope filter
        , ResetThread       = 0x3FFFu   //!< Reset thread filter
        , ResetMessage      = 0x0FFFu   //!< Reset message text filter
    };

    enum eFieldChecked
    {
          NoFieldChecked    = 0x0000u   //!< Field is not checked
        , CheckPriority     = 0x0002u   //!< Field is checked for priority
        , CheckTimeCreated  = 0x0008u   //!< Field is checked for time created
        , CheckTimeReceived = 0x0020u   //!< Field is checked for time received
        , CheckDuration     = 0x0080u   //!< Field is checked for duration
        , CheckInstance     = 0x0200u   //!< Field is checked for instance
        , CheckScope        = 0x0800u   //!< Field is checked for scope
        , CheckThread       = 0x2000u   //!< Field is checked for thread
        , CheckMessage      = 0x8000u   //!< Field is checked for message text
    };

    enum eFieldMatch
    {
          NoValueMatch      = 0x0000u   //!< No value match
        , MatchPriority     = 0x0001u   //!< Value matches exactly
        , MatchTimeCreated  = 0x0004u   //!< Value matches partially
        , MatchTimeReceived = 0x0010u   //!< Value contains the filter text
        , MatchDuration     = 0x0040u   //!< Value matches duration
        , MatchInstance     = 0x0100u   //!< Value matches instance
        , MatchScope        = 0x0400u   //!< Value matches scope
        , MatchThread       = 0x1000u   //!< Value matches thread
        , MatchMessage      = 0x4000u   //!< Value matches message text
    };

    struct sFieldFilter
    {
        uint16_t    field   { 0u }; //!< The field index of the filter
        uint16_t    mask    { 0u };
        uint16_t    reset   { 0u };
        uint16_t    checked { 0u };
        uint16_t    match   { 0u };
    };

    struct sFilterData
    {
        QString     data  {       };//!< The filter data as a string
        uint64_t    value { 0     };//!< The filter digital value  
        bool        active{ false };//!< Flag indicating if the data is active
    };

    using FilterList = QList<sFilterData>;

protected:
////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
////////////////////////////////////////////////////////////////////////
    /**
     * \brief   Default constructor.
     **/
    LogFilterBase(eFilterType filter);

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

    virtual QList<LogFilterBase::sFilterData> filterList(void) const = 0;

    virtual QString filterData(void) const = 0;

    /**
     * \brief   Sets the filtering data.
     * \param   data            The data to set for filtering. It can be a string, wildcard, regular expression or digit as a string.
     * \param   isCaseSensitive Flag indicating whether the filter is case sensitive. Valid only for filtering by text.
     * \param   isWholeWord     Flag indicating whether the filter is for whole words only. Ignored for digits.
     * \param   isRegEx         Flag indicating whether the filter is a regular expression. Ignored for digits.
     **/
    virtual void setData(const QString& data, bool isCaseSensitive, bool isWholeWord, bool isRegEx) = 0;

    /**
     * \brief   Sets the filtering data as a list.
     * \param   data    The list of strings to set for filtering. It can be a list of strings, enumeration.
     **/
    virtual void setData(const QStringList& data) = 0;

    virtual void activateFilter(const QString& filter, bool isCaseSensitive, bool isWholeWord, bool isRegEx) = 0;
    
    virtual void activateFilters(const QStringList& filters) = 0;

    virtual void deactivateFilter(void) = 0;

////////////////////////////////////////////////////////////////////////
// Attributes and operations
////////////////////////////////////////////////////////////////////////
public:

    inline LogFilterBase::eFilterType filterType(void) const;

////////////////////////////////////////////////////////////////////////
// Member variables
////////////////////////////////////////////////////////////////////////
protected:

    const eFilterType   mFilterType; //!< The type of the filter
};

////////////////////////////////////////////////////////////////////////
// LogFilterBase inline methods
////////////////////////////////////////////////////////////////////////

inline LogFilterBase::eFilterType LogFilterBase::filterType(void) const
{
    return mFilterType;
}

#endif  // LUSAN_DATA_LOG_LOGFILTERBASE_HPP
