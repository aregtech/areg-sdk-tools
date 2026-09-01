#ifndef LUSAN_COMMON_NETIMEUNITS_HPP
#define LUSAN_COMMON_NETIMEUNITS_HPP
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
 *  \file        lusan/common/NETimeUnits.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the unit every measured time is shown in.
 *
 ************************************************************************/

/************************************************************************
 * Include files.
 ************************************************************************/
#include <QString>

#include <cstdint>

/**
 * \brief   The unit every duration and every elapsed time of the application is written in.
 *
 *          Every surface asks this namespace to turn a measurement into text, so one setting
 *          changes the log table, the scope analyzer and the menus at once. The measurements
 *          themselves stay in microseconds, which is what the framework delivers.
 **/
namespace NETimeUnits
{
    /**
     * \brief   The unit a measured time is written in.
     **/
    enum class eTimeUnit : uint8_t
    {
          UnitMicro  = 0    //!< Microseconds, the unit the framework measures in
        , UnitMilli         //!< Milliseconds
        , UnitSecond        //!< Seconds
    };

    //!< The unit taken when nothing was configured.
    constexpr eTimeUnit DefaultUnit{ eTimeUnit::UnitMicro };

    /**
     * \brief   The shape a log timestamp is written in.
     **/
    enum class eTimeStamp : uint8_t
    {
          StampTime = 0     //!< Time of day with milliseconds
        , StampTimeMicro    //!< Time of day with microseconds
        , StampDateTime     //!< Date and time of day with milliseconds
        , StampElapsed      //!< Time since the first row of the window
        , StampDelta        //!< Time since the row above
    };

    //!< The shape taken when nothing was configured.
    constexpr eTimeStamp DefaultStamp{ eTimeStamp::StampTime };

    /**
     * \brief   Returns the unit the application currently writes measured times in.
     **/
    eTimeUnit unit(void);

    /**
     * \brief   Sets the unit the application writes measured times in.
     **/
    void setUnit(eTimeUnit newUnit);

    /**
     * \brief   Returns the short suffix of the unit, such as "us".
     **/
    QString unitSuffix(eTimeUnit forUnit);

    /**
     * \brief   Returns the full name of the unit, such as "Microseconds".
     **/
    QString unitName(eTimeUnit forUnit);

    /**
     * \brief   Returns the name of the unit as it is stored in the options file.
     **/
    QString unitKey(eTimeUnit forUnit);

    /**
     * \brief   Returns the unit that carries the given stored name, the default one if no unit does.
     **/
    eTimeUnit unitFromKey(const QString& key);

    /**
     * \brief   Returns the measurement as a number in the active unit, without a suffix.
     * \param   micros  The measurement in microseconds.
     **/
    QString number(uint64_t micros);

    /**
     * \brief   Returns the measurement as a number and a suffix, such as "1240 us".
     * \param   micros  The measurement in microseconds.
     **/
    QString duration(uint64_t micros);

    /**
     * \brief   Returns the measurement with its sign, such as "+1240 us". Used where the value
     *          is a distance from another point in time rather than a length.
     * \param   micros  The measurement in microseconds, negative when it precedes that point.
     **/
    QString offset(int64_t micros);

    /**
     * \brief   Returns the shape the application currently writes log timestamps in.
     **/
    eTimeStamp stamp(void);

    /**
     * \brief   Sets the shape the application writes log timestamps in.
     **/
    void setStamp(eTimeStamp newStamp);

    /**
     * \brief   Returns the full name of the shape, such as "Time of day".
     **/
    QString stampName(eTimeStamp forStamp);

    /**
     * \brief   Returns a sample timestamp written in the given shape, for a settings page.
     **/
    QString stampSample(eTimeStamp forStamp);

    /**
     * \brief   Returns the name of the shape as it is stored in the options file.
     **/
    QString stampKey(eTimeStamp forStamp);

    /**
     * \brief   Returns the shape that carries the given stored name, the default one if none does.
     **/
    eTimeStamp stampFromKey(const QString& key);

    /**
     * \brief   Returns true if the given shape counts from another row rather than from the clock.
     **/
    bool isRelative(eTimeStamp forStamp);

    /**
     * \brief   Returns the timestamp of a log row in the active shape.
     * \param   micros      The timestamp of the row, in microseconds since the epoch.
     * \param   base        The timestamp of the first row of the window, for the elapsed shape.
     * \param   previous    The timestamp of the row above, for the delta shape. Equal to
     *                      \p micros on the first row.
     * \param   withDate    True to put the day in front of the time, on the first row of a day.
     **/
    QString timestamp(uint64_t micros, uint64_t base, uint64_t previous, bool withDate);

    /**
     * \brief   Returns the date and the time of a timestamp in full, to the microsecond.
     *          Used where the text is read on its own: a tool tip, a copied line.
     * \param   micros  The timestamp, in microseconds since the epoch.
     **/
    QString fullTimestamp(uint64_t micros);

    /**
     * \brief   Returns the local calendar day of a timestamp, counted from a fixed origin.
     *          Two rows of the same day give the same number.
     * \param   micros  The timestamp, in microseconds since the epoch.
     **/
    int64_t dayOf(uint64_t micros);
}

#endif  // LUSAN_COMMON_NETIMEUNITS_HPP
