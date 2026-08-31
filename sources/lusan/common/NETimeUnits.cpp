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

namespace
{
    NETimeUnits::eTimeUnit _unit{ NETimeUnits::DefaultUnit };

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
