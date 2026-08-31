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
}

#endif  // LUSAN_COMMON_NETIMEUNITS_HPP
