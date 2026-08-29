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
 *  \file        lusan/model/log/LogClockSkew.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, detection of sources whose clock disagrees.
 *
 ************************************************************************/

#include "lusan/model/log/LogClockSkew.hpp"

LogClockSkew::LogClockSkew(void)
    : mSources  ( )
    , mReport   ( )
    , mHasSkew  (false)
{
}

void LogClockSkew::reset(void)
{
    mSources.clear();
    mReport = sSkewReport{};
    mHasSkew = false;
}

void LogClockSkew::feed(const areg::LogEntry& entry)
{
    if ((entry.logTimestamp == 0) || (entry.logReceived == 0))
        return;

    const qint64 offset{ static_cast<qint64>(entry.logTimestamp) - static_cast<qint64>(entry.logReceived) };
    sSource& source{ mSources[entry.logCookie] };
    if (source.name.isEmpty())
    {
        source.name = QString::fromUtf8(entry.logModule);
    }

    ++source.count;
    source.sum += offset;
    if (offset > 0)
    {
        ++source.ahead;
    }

    // Judging on every entry would run the whole table on every arriving row.
    if ((source.count % LogClockSkew::MIN_SAMPLES) == 0)
    {
        _evaluate();
    }
}

void LogClockSkew::_evaluate(void)
{
    qint64 worst{ 0 };
    const sSource* found{ nullptr };

    for (auto it = mSources.cbegin(); it != mSources.cend(); ++it)
    {
        const sSource& source{ it.value() };
        if (source.count < LogClockSkew::MIN_SAMPLES)
            continue;
        if ((source.ahead * 100u) < (source.count * LogClockSkew::AHEAD_PERCENT))
            continue;

        const qint64 offset{ source.sum / static_cast<qint64>(source.count) };
        if ((offset >= LogClockSkew::MIN_OFFSET_US) && (offset > worst))
        {
            worst = offset;
            found = &source;
        }
    }

    mHasSkew = (found != nullptr);
    if (mHasSkew)
    {
        mReport.source = found->name;
        mReport.offsetUs = worst;
    }
}
