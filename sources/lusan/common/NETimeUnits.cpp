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
 *  \file        lusan/common/NETimeUnits.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the unit every measured time is shown in.
 *
 ************************************************************************/

#include "lusan/common/NETimeUnits.hpp"

#include <QCoreApplication>
#include <QDate>
#include <QDateTime>

#include <cstdio>

namespace
{
    NETimeUnits::eTimeUnit _unit{ NETimeUnits::DefaultUnit };

    NETimeUnits::eTimeStamp _stamp{ NETimeUnits::DefaultStamp };

    //!< Microseconds in one second, and seconds in one day.
    constexpr int64_t   _microsPerSec   { 1000000 };
    constexpr int64_t   _secsPerDay     { 86400 };

    //!< The day number of 1 January 1970, so a day number converts to a calendar date.
    constexpr int64_t   _epochJulian    { 2440588 };

    //!< Divides towards minus infinity, so a timestamp before the epoch keeps the right day.
    inline int64_t floorDiv(int64_t value, int64_t by)
    {
        const int64_t whole{ value / by };
        return ((value % by) < 0) ? (whole - 1) : whole;
    }

    /**
     * \brief   The parts of a timestamp, in local time.
     **/
    struct sParts
    {
        int64_t day     { 0 };  //!< Days since the epoch, in local time.
        int     hour    { 0 };
        int     minute  { 0 };
        int     second  { 0 };
        int     micro   { 0 };  //!< Microseconds inside the second.
    };

    /**
     * \brief   Splits a timestamp into its local parts. The offset from UTC is asked of Qt
     *          once per hour of log data and kept, so a cell costs integer arithmetic alone.
     *          An hour is the step because a daylight saving change lands on one.
     **/
    sParts splitLocal(uint64_t micros)
    {
        static int64_t  _cachedHour  { INT64_MIN };
        static int64_t  _cachedOffset{ 0 };

        const int64_t stamp{ static_cast<int64_t>(micros) };
        const int64_t utcSecs{ floorDiv(stamp, _microsPerSec) };
        const int64_t hour{ floorDiv(utcSecs, 3600) };
        if (hour != _cachedHour)
        {
            _cachedHour   = hour;
            _cachedOffset = QDateTime::fromSecsSinceEpoch(utcSecs).offsetFromUtc();
        }

        const int64_t local{ utcSecs + _cachedOffset };
        const int64_t day{ floorDiv(local, _secsPerDay) };
        const int64_t inDay{ local - (day * _secsPerDay) };

        sParts parts;
        parts.day    = day;
        parts.hour   = static_cast<int>(inDay / 3600);
        parts.minute = static_cast<int>((inDay / 60) % 60);
        parts.second = static_cast<int>(inDay % 60);
        parts.micro  = static_cast<int>(stamp - (utcSecs * _microsPerSec));
        return parts;
    }

    //!< Writes the given day number as a date, in full or without the year.
    QString dayText(int64_t day, bool withYear)
    {
        const QDate date{ QDate::fromJulianDay(day + _epochJulian) };
        return date.toString(withYear ? QStringLiteral("yyyy-MM-dd") : QStringLiteral("MM-dd"));
    }

    //!< The number of microseconds in one of the units, and the decimals it is written with.
    struct sUnitShape
    {
        double  divider;
        int     decimals;
    };

    inline sUnitShape unitShape(NETimeUnits::eTimeUnit forUnit)
    {
        switch (forUnit)
        {
        case NETimeUnits::eTimeUnit::UnitMilli:
            return sUnitShape{ 1000.0, 3 };
        case NETimeUnits::eTimeUnit::UnitSecond:
            return sUnitShape{ 1000000.0, 6 };
        case NETimeUnits::eTimeUnit::UnitMicro:
        default:
            return sUnitShape{ 1.0, 0 };
        }
    }
}

NETimeUnits::eTimeUnit NETimeUnits::unit(void)
{
    return _unit;
}

void NETimeUnits::setUnit(NETimeUnits::eTimeUnit newUnit)
{
    _unit = newUnit;
}

QString NETimeUnits::unitSuffix(NETimeUnits::eTimeUnit forUnit)
{
    switch (forUnit)
    {
    case NETimeUnits::eTimeUnit::UnitMilli:
        return QCoreApplication::translate("NETimeUnits", "ms");
    case NETimeUnits::eTimeUnit::UnitSecond:
        return QCoreApplication::translate("NETimeUnits", "s");
    case NETimeUnits::eTimeUnit::UnitMicro:
    default:
        return QCoreApplication::translate("NETimeUnits", "µs");
    }
}

QString NETimeUnits::unitName(NETimeUnits::eTimeUnit forUnit)
{
    switch (forUnit)
    {
    case NETimeUnits::eTimeUnit::UnitMilli:
        return QCoreApplication::translate("NETimeUnits", "Milliseconds");
    case NETimeUnits::eTimeUnit::UnitSecond:
        return QCoreApplication::translate("NETimeUnits", "Seconds");
    case NETimeUnits::eTimeUnit::UnitMicro:
    default:
        return QCoreApplication::translate("NETimeUnits", "Microseconds");
    }
}

QString NETimeUnits::unitKey(NETimeUnits::eTimeUnit forUnit)
{
    switch (forUnit)
    {
    case NETimeUnits::eTimeUnit::UnitMilli:
        return QStringLiteral("Milliseconds");
    case NETimeUnits::eTimeUnit::UnitSecond:
        return QStringLiteral("Seconds");
    case NETimeUnits::eTimeUnit::UnitMicro:
    default:
        return QStringLiteral("Microseconds");
    }
}

NETimeUnits::eTimeUnit NETimeUnits::unitFromKey(const QString& key)
{
    if (key.compare(QStringLiteral("Milliseconds"), Qt::CaseSensitivity::CaseInsensitive) == 0)
        return NETimeUnits::eTimeUnit::UnitMilli;
    if (key.compare(QStringLiteral("Seconds"), Qt::CaseSensitivity::CaseInsensitive) == 0)
        return NETimeUnits::eTimeUnit::UnitSecond;

    return NETimeUnits::eTimeUnit::UnitMicro;
}

QString NETimeUnits::number(uint64_t micros)
{
    const sUnitShape shape{ unitShape(_unit) };
    return shape.decimals == 0
                ? QString::number(micros)
                : QString::number(static_cast<double>(micros) / shape.divider, 'f', shape.decimals);
}

QString NETimeUnits::duration(uint64_t micros)
{
    return NETimeUnits::number(micros) + QChar(' ') + NETimeUnits::unitSuffix(_unit);
}

QString NETimeUnits::offset(int64_t micros)
{
    const QString sign{ micros < 0 ? QStringLiteral("-") : QStringLiteral("+") };
    return sign + NETimeUnits::duration(static_cast<uint64_t>(micros < 0 ? -micros : micros));
}

NETimeUnits::eTimeStamp NETimeUnits::stamp(void)
{
    return _stamp;
}

void NETimeUnits::setStamp(NETimeUnits::eTimeStamp newStamp)
{
    _stamp = newStamp;
}

QString NETimeUnits::stampName(NETimeUnits::eTimeStamp forStamp)
{
    switch (forStamp)
    {
    case NETimeUnits::eTimeStamp::StampTimeMicro:
        return QCoreApplication::translate("NETimeUnits", "Time, microseconds");
    case NETimeUnits::eTimeStamp::StampDateTime:
        return QCoreApplication::translate("NETimeUnits", "Date and time");
    case NETimeUnits::eTimeStamp::StampElapsed:
        return QCoreApplication::translate("NETimeUnits", "Elapsed since first row");
    case NETimeUnits::eTimeStamp::StampDelta:
        return QCoreApplication::translate("NETimeUnits", "Since the row above");
    case NETimeUnits::eTimeStamp::StampTime:
    default:
        return QCoreApplication::translate("NETimeUnits", "Time");
    }
}

QString NETimeUnits::stampSample(NETimeUnits::eTimeStamp forStamp)
{
    switch (forStamp)
    {
    case NETimeUnits::eTimeStamp::StampTimeMicro:
        return QStringLiteral("10:22:04.121408");
    case NETimeUnits::eTimeStamp::StampDateTime:
        return QStringLiteral("2026-09-01 10:22:04.121");
    case NETimeUnits::eTimeStamp::StampElapsed:
        return QStringLiteral("+00:01:23.456");
    case NETimeUnits::eTimeStamp::StampDelta:
        return NETimeUnits::offset(12480);
    case NETimeUnits::eTimeStamp::StampTime:
    default:
        return QStringLiteral("10:22:04.121");
    }
}

QString NETimeUnits::stampKey(NETimeUnits::eTimeStamp forStamp)
{
    switch (forStamp)
    {
    case NETimeUnits::eTimeStamp::StampTimeMicro:
        return QStringLiteral("TimeMicro");
    case NETimeUnits::eTimeStamp::StampDateTime:
        return QStringLiteral("DateTime");
    case NETimeUnits::eTimeStamp::StampElapsed:
        return QStringLiteral("Elapsed");
    case NETimeUnits::eTimeStamp::StampDelta:
        return QStringLiteral("Delta");
    case NETimeUnits::eTimeStamp::StampTime:
    default:
        return QStringLiteral("Time");
    }
}

NETimeUnits::eTimeStamp NETimeUnits::stampFromKey(const QString& key)
{
    if (key.compare(QStringLiteral("TimeMicro"), Qt::CaseSensitivity::CaseInsensitive) == 0)
        return NETimeUnits::eTimeStamp::StampTimeMicro;
    if (key.compare(QStringLiteral("DateTime"), Qt::CaseSensitivity::CaseInsensitive) == 0)
        return NETimeUnits::eTimeStamp::StampDateTime;
    if (key.compare(QStringLiteral("Elapsed"), Qt::CaseSensitivity::CaseInsensitive) == 0)
        return NETimeUnits::eTimeStamp::StampElapsed;
    if (key.compare(QStringLiteral("Delta"), Qt::CaseSensitivity::CaseInsensitive) == 0)
        return NETimeUnits::eTimeStamp::StampDelta;

    return NETimeUnits::eTimeStamp::StampTime;
}

bool NETimeUnits::isRelative(NETimeUnits::eTimeStamp forStamp)
{
    return (forStamp == NETimeUnits::eTimeStamp::StampElapsed)
        || (forStamp == NETimeUnits::eTimeStamp::StampDelta);
}

int64_t NETimeUnits::dayOf(uint64_t micros)
{
    return splitLocal(micros).day;
}

QString NETimeUnits::timestamp(uint64_t micros, uint64_t base, uint64_t previous, bool withDate)
{
    if (micros == 0)
        return QString();

    switch (_stamp)
    {
    case NETimeUnits::eTimeStamp::StampElapsed:
        {
            const int64_t span{ static_cast<int64_t>(micros) - static_cast<int64_t>(base) };
            const int64_t abs { span < 0 ? -span : span };
            const int64_t secs{ abs / _microsPerSec };
            char buffer[32];
            const int len{ std::snprintf(buffer, sizeof(buffer), "%c%02d:%02d:%02d.%03d"
                                       , span < 0 ? '-' : '+'
                                       , static_cast<int>(secs / 3600)
                                       , static_cast<int>((secs / 60) % 60)
                                       , static_cast<int>(secs % 60)
                                       , static_cast<int>((abs % _microsPerSec) / 1000)) };
            return QString::fromLatin1(buffer, len > 0 ? len : 0);
        }

    case NETimeUnits::eTimeStamp::StampDelta:
        return NETimeUnits::offset(static_cast<int64_t>(micros) - static_cast<int64_t>(previous));

    default:
        break;
    }

    const sParts parts{ splitLocal(micros) };
    char buffer[32];
    int len{ 0 };
    if (_stamp == NETimeUnits::eTimeStamp::StampTimeMicro)
    {
        len = std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%06d"
                          , parts.hour, parts.minute, parts.second, parts.micro);
    }
    else
    {
        len = std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%03d"
                          , parts.hour, parts.minute, parts.second, parts.micro / 1000);
    }

    const QString time{ QString::fromLatin1(buffer, len > 0 ? len : 0) };
    if (_stamp == NETimeUnits::eTimeStamp::StampDateTime)
        return dayText(parts.day, true) + QChar(' ') + time;

    // The day is written once, on the row that opens it. Every other row of that day
    // carries the time alone, which is what the reader compares.
    return withDate ? (dayText(parts.day, false) + QChar(' ') + time) : time;
}

QString NETimeUnits::fullTimestamp(uint64_t micros)
{
    if (micros == 0)
        return QString();

    const sParts parts{ splitLocal(micros) };
    char buffer[32];
    const int len{ std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%06d"
                               , parts.hour, parts.minute, parts.second, parts.micro) };
    return dayText(parts.day, true) + QChar(' ') + QString::fromLatin1(buffer, len > 0 ? len : 0);
}
