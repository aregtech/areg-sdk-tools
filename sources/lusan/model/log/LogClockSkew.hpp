#ifndef LUSAN_MODEL_LOG_LOGCLOCKSKEW_HPP
#define LUSAN_MODEL_LOG_LOGCLOCKSKEW_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/log/LogClockSkew.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, detection of sources whose clock disagrees.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "areg/logging/LoggingDefs.hpp"

#include <QHash>
#include <QString>

//////////////////////////////////////////////////////////////////////////
// LogClockSkew class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   Watches the log entries of every source and reports the one whose clock
 *          disagrees with the log collector. A source is reported when most of its
 *          entries carry a timestamp later than the moment the collector received
 *          them, which is a time that never happened.
 **/
class LogClockSkew
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The source whose clock disagrees, and by how much.
     **/
    struct sSkewReport
    {
        QString source;     //!< The module name of the source.
        qint64  offsetUs;   //!< Microseconds the source is ahead of the collector.
    };

    //!< The number of entries a source must produce before it can be judged.
    static constexpr uint32_t   MIN_SAMPLES     { 64u };

    //!< The share of entries that must be ahead of the collector, in percent.
    static constexpr uint32_t   AHEAD_PERCENT   { 80u };

    //!< The smallest offset worth a warning. Below it the two clocks agree in practice.
    static constexpr qint64     MIN_OFFSET_US   { 250000 };

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    LogClockSkew(void);

//////////////////////////////////////////////////////////////////////////
// Operations and attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Forgets every source and the report made so far.
     **/
    void reset(void);

    /**
     * \brief   Takes one log entry into account.
     * \param   entry   The entry to measure.
     **/
    void feed(const areg::LogEntry& entry);

    /**
     * \brief   Returns true if a source was found whose clock disagrees.
     **/
    inline bool hasSkew(void) const;

    /**
     * \brief   Returns the source with the largest disagreement.
     *          Meaningful only when hasSkew() returns true.
     **/
    inline const LogClockSkew::sSkewReport& report(void) const;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< What is known about the clock of one source.
    struct sSource
    {
        QString     name    { };    //!< The module name.
        uint32_t    count   { 0 };  //!< The entries taken into account.
        uint32_t    ahead   { 0 };  //!< The entries stamped later than they were received.
        qint64      sum     { 0 };  //!< The sum of the offsets, in microseconds.
    };

    //!< Picks the source with the largest offset that clears every threshold.
    void _evaluate(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QHash<ITEM_ID, sSource> mSources;   //!< One entry per source, keyed by its cookie.
    sSkewReport             mReport;    //!< The source reported, valid while mHasSkew is true.
    bool                    mHasSkew;   //!< True when a source clears every threshold.
};

//////////////////////////////////////////////////////////////////////////
// LogClockSkew class inline methods
//////////////////////////////////////////////////////////////////////////

inline bool LogClockSkew::hasSkew(void) const
{
    return mHasSkew;
}

inline const LogClockSkew::sSkewReport& LogClockSkew::report(void) const
{
    return mReport;
}

#endif  // LUSAN_MODEL_LOG_LOGCLOCKSKEW_HPP
