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
 *  \file        tests/log/LogModelTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Headless checks of the log data layer: the colour table, the column
 *               keys the workspace stores, the scopes the tree keeps out of the view,
 *               the view filter and the clock-skew measure. Runs with synthetic log
 *               entries, so it needs neither a collector nor an archive.
 *
 *  Usage: lusan_log_model_tests
 *
 ************************************************************************/

#include "lusan/common/NELogPalette.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/common/NETimeUnits.hpp"
#include "lusan/model/log/LogClockSkew.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/model/log/ScopeLogViewerFilter.hpp"

#include "tests/common/UiTestEnv.hpp"

#include <QApplication>
#include <QColor>
#include <QDateTime>
#include <QPalette>
#include <QElapsedTimer>
#include <QSet>
#include <QStyle>
#include <QString>

#include <cstdio>
#include <cstring>
#include <string>

namespace
{
    int gChecks = 0;
    int gFailures = 0;

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }

    //!< Builds one log entry in the layout the archive reader produces.
    areg::SharedBuffer makeEntry( areg::LogMessageType msgType
                                , areg::LogPriority prio
                                , ITEM_ID cookie
                                , ITEM_ID threadId
                                , uint32_t scopeId
                                , uint32_t sessionId
                                , TIME64 timestamp
                                , TIME64 received
                                , const char* message
                                , uint32_t messageLen = 0u)
    {
        areg::LogEntry entry{ };
        entry.logMsgType     = msgType;
        entry.logMessagePrio = prio;
        entry.logDataType    = areg::LogDataType::Remote;
        entry.logCookie      = cookie;
        entry.logModuleId    = cookie;
        entry.logThreadId    = threadId;
        entry.logScopeId     = scopeId;
        entry.logSessionId   = sessionId;
        entry.logTimestamp   = timestamp;
        entry.logReceived    = received;

        const uint32_t textLen{ static_cast<uint32_t>(message != nullptr ? std::strlen(message) : 0u) };
        const uint32_t copyLen{ textLen < (areg::LOG_MSG_SIZE - 1u) ? textLen : (areg::LOG_MSG_SIZE - 1u) };
        if (copyLen != 0u)
        {
            std::memcpy(entry.logMessage, message, copyLen);
        }

        entry.logMessage[copyLen] = '\0';
        entry.logMessageLen = (messageLen != 0u ? messageLen : textLen);
        std::memcpy(entry.logModule, "target", 7);
        entry.logModuleLen = 6u;
        std::memcpy(entry.logThread, "worker", 7);
        entry.logThreadLen = 6u;

        constexpr uint32_t entrySize{ static_cast<uint32_t>(sizeof(areg::LogEntry)) };
        areg::SharedBuffer buffer;
        buffer.reserve(entrySize, false);
        buffer.set_size_used(entrySize);
        buffer.move_to_begin();
        std::memcpy(buffer.buffer(), &entry, entrySize);
        buffer.set_size_used(areg::log_entry_size(entry));
        return buffer;
    }

    /**
     * \brief   A log model filled by hand instead of by a collector or an archive.
     **/
    class TestLogModel : public LoggingModelBase
    {
    public:
        TestLogModel(void)
            : LoggingModelBase(LoggingModelBase::eLogging::LoggingOffline, nullptr)
        {
        }

        void addEntries(std::vector<areg::SharedBuffer>&& entries)
        {
            appendLogBatch(std::move(entries), mLoadGeneration);
        }

        QString timeOf(int row, LoggingModelBase::eColumn column) const
        {
            return getTimeDisplay(row, getLogData(row), column);
        }

        using LoggingModelBase::isDayChange;
    };

    //!< Paints the application in the values a light or a dark theme carries.
    void applyTheme(bool dark)
    {
        QPalette palette{ QApplication::style()->standardPalette() };
        palette.setColor(QPalette::Base, dark ? QColor(0x1E, 0x22, 0x27) : QColor(Qt::white));
        palette.setColor(QPalette::Window, dark ? QColor(0x25, 0x2A, 0x30) : QColor(0xF0, 0xF0, 0xF0));
        QApplication::setPalette(palette);
    }

    //!< The grey a colour becomes on a monochrome screen or a printed page.
    int greyOf(const QColor& color)
    {
        return qGray(color.red(), color.green(), color.blue());
    }
}

#define CHECK(cond)  check((cond), #cond)

int main(int argc, char* argv[])
{
    LusanTest::prepareUiEnvironment();

    QApplication app(argc, argv);

    std::printf("[palette] the colour table, both themes\n");
    {
        applyTheme(false);
        CHECK(NELogPalette::isDarkTheme() == false);

        applyTheme(true);
        CHECK(NELogPalette::isDarkTheme());

        for (int pass = 0; pass < 2; ++pass)
        {
            const bool dark{ pass == 1 };
            applyTheme(dark);

            // A greyscale screenshot must still separate Error from Warning. The rail is the
            // channel that carries it: the two message-text colours are near neighbours in grey.
            const int greyError  { greyOf(NELogPalette::railColor(NELogPalette::eLogColorRole::RoleError)) };
            const int greyWarning{ greyOf(NELogPalette::railColor(NELogPalette::eLogColorRole::RoleWarning)) };
            CHECK(qAbs(greyError - greyWarning) >= 24);

            // Fatal is the only role with a row background.
            CHECK(NELogPalette::rowBackground(NELogPalette::eLogColorRole::RoleFatal).alpha() == 255);
            CHECK(NELogPalette::rowBackground(NELogPalette::eLogColorRole::RoleError).alpha() == 0);
            CHECK(NELogPalette::rowBackground(NELogPalette::eLogColorRole::RoleScope).alpha() == 0);

            // No two priorities share a rail colour or a text colour.
            const NELogPalette::eLogColorRole roles[]
            {
                  NELogPalette::eLogColorRole::RoleFatal
                , NELogPalette::eLogColorRole::RoleError
                , NELogPalette::eLogColorRole::RoleWarning
                , NELogPalette::eLogColorRole::RoleInformation
                , NELogPalette::eLogColorRole::RoleDebug
                , NELogPalette::eLogColorRole::RoleScope
            };

            bool railsDiffer{ true };
            bool textsDiffer{ true };
            for (int i = 0; i < 6; ++i)
            {
                for (int j = i + 1; j < 6; ++j)
                {
                    railsDiffer = railsDiffer && (NELogPalette::railColor(roles[i]) != NELogPalette::railColor(roles[j]));
                    textsDiffer = textsDiffer && (NELogPalette::textColor(roles[i]) != NELogPalette::textColor(roles[j]));
                }
            }

            CHECK(railsDiffer);
            CHECK(textsDiffer);

            // Every opacity is a real transparency, and the tint carries more ink than the hover.
            const qreal tint { NELogPalette::opacity(NELogPalette::eLogOpacity::OpacityTint) };
            const qreal hover{ NELogPalette::opacity(NELogPalette::eLogOpacity::OpacityHover) };
            const qreal ghost{ NELogPalette::opacity(NELogPalette::eLogOpacity::OpacityGhost) };
            CHECK((tint > 0.0) && (tint < 1.0));
            CHECK((hover > 0.0) && (hover < 1.0));
            CHECK((ghost > 0.0) && (ghost < 1.0));

            const QColor tinted{ NELogPalette::withOpacity(NELogPalette::railColor(NELogPalette::eLogColorRole::RoleError)
                                                         , NELogPalette::eLogOpacity::OpacityTint) };
            CHECK(tinted.alpha() == static_cast<int>(qRound(tint * 255.0)));
        }

        // The ghost track is a per-theme value, not one number used twice.
        applyTheme(false);
        const qreal ghostLight{ NELogPalette::opacity(NELogPalette::eLogOpacity::OpacityGhost) };
        applyTheme(true);
        const qreal ghostDark{ NELogPalette::opacity(NELogPalette::eLogOpacity::OpacityGhost) };
        CHECK(ghostLight != ghostDark);

        // The Fatal band keeps the same strength in both themes.
        applyTheme(false);
        const QColor bandLight{ NELogPalette::rowBackground(NELogPalette::eLogColorRole::RoleFatal) };
        applyTheme(true);
        const QColor bandDark{ NELogPalette::rowBackground(NELogPalette::eLogColorRole::RoleFatal) };
        CHECK(bandLight != bandDark);

        // A priority maps to the role that carries its colours.
        CHECK(NELogPalette::roleOf(areg::LogPriority::PrioFatal)   == NELogPalette::eLogColorRole::RoleFatal);
        CHECK(NELogPalette::roleOf(areg::LogPriority::PrioError)   == NELogPalette::eLogColorRole::RoleError);
        CHECK(NELogPalette::roleOf(areg::LogPriority::PrioWarning) == NELogPalette::eLogColorRole::RoleWarning);
        CHECK(NELogPalette::roleOf(areg::LogPriority::PrioInfo)    == NELogPalette::eLogColorRole::RoleInformation);
        CHECK(NELogPalette::roleOf(areg::LogPriority::PrioDebug)   == NELogPalette::eLogColorRole::RoleDebug);
        CHECK(NELogPalette::roleOf(areg::LogPriority::PrioScope)   == NELogPalette::eLogColorRole::RoleScope);

        applyTheme(false);
    }

    std::printf("[columns] the keys the workspace stores\n");
    {
        QSet<QString> keys;
        bool roundTrip{ true };
        bool ascii{ true };
        for (int index = 0; index < static_cast<int>(LoggingModelBase::eColumn::LogColumnCount); ++index)
        {
            const LoggingModelBase::eColumn column{ static_cast<LoggingModelBase::eColumn>(index) };
            const QString key{ LoggingModelBase::getColumnKey(column) };
            roundTrip = roundTrip && (key.isEmpty() == false) && (LoggingModelBase::getColumnByKey(key) == column);
            for (const QChar& one : key)
            {
                ascii = ascii && (one.unicode() < 128);
            }

            keys.insert(key);
        }

        CHECK(roundTrip);
        CHECK(ascii);
        CHECK(keys.size() == static_cast<int>(LoggingModelBase::eColumn::LogColumnCount));
        CHECK(LoggingModelBase::getColumnByKey(QStringLiteral("no-such-column")) == LoggingModelBase::eColumn::LogColumnInvalid);
        CHECK(LoggingModelBase::getColumnByKey(QString()) == LoggingModelBase::eColumn::LogColumnInvalid);
        CHECK(LoggingModelBase::getDefaultColumns().isEmpty() == false);
    }

    std::printf("[entry] the cut message reports both lengths\n");
    {
        const areg::SharedBuffer shortMsg{ makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo
                                                   , 1u, 2u, 3u, 0u, 1000, 1000, "a short message") };
        const areg::LogEntry* shortEntry{ reinterpret_cast<const areg::LogEntry*>(shortMsg.buffer()) };
        CHECK(areg::is_log_message_cut(*shortEntry) == false);
        CHECK(areg::log_message_size(*shortEntry) == 15u);

        // The target cut the text and reported the length it had before the cut.
        const areg::SharedBuffer cutMsg{ makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo
                                                 , 1u, 2u, 3u, 0u, 1000, 1000, "cut", 823u) };
        const areg::LogEntry* cutEntry{ reinterpret_cast<const areg::LogEntry*>(cutMsg.buffer()) };
        CHECK(areg::is_log_message_cut(*cutEntry));
        CHECK(areg::log_message_size(*cutEntry) == (areg::LOG_MSG_SIZE - 1u));
        CHECK(cutEntry->logMessageLen == 823u);
    }

    std::printf("[time] the shapes a log row writes its time in\n");
    {
        const NETimeUnits::eTimeUnit  keepUnit { NETimeUnits::unit()  };
        const NETimeUnits::eTimeStamp keepStamp{ NETimeUnits::stamp() };

        // The measurement is built from a local reading, so every expected string below holds
        // in any time zone the checks run in.
        const QDateTime when{ QDate(2026, 9, 1), QTime(10, 22, 4, 121) };
        const uint64_t stamp{ (static_cast<uint64_t>(when.toMSecsSinceEpoch()) * 1000ull) + 408ull };
        const uint64_t oneDay{ 24ull * 60ull * 60ull * 1000000ull };

        NETimeUnits::setUnit(NETimeUnits::eTimeUnit::UnitMicro);

        NETimeUnits::setStamp(NETimeUnits::eTimeStamp::StampTime);
        CHECK(NETimeUnits::timestamp(stamp, stamp, stamp, false) == QStringLiteral("10:22:04.121"));
        CHECK(NETimeUnits::timestamp(stamp, stamp, stamp, true)  == QStringLiteral("09-01 10:22:04.121"));

        NETimeUnits::setStamp(NETimeUnits::eTimeStamp::StampTimeMicro);
        CHECK(NETimeUnits::timestamp(stamp, stamp, stamp, false) == QStringLiteral("10:22:04.121408"));

        NETimeUnits::setStamp(NETimeUnits::eTimeStamp::StampDateTime);
        CHECK(NETimeUnits::timestamp(stamp, stamp, stamp, false) == QStringLiteral("2026-09-01 10:22:04.121"));

        // A relative shape counts from another row and never carries a day.
        NETimeUnits::setStamp(NETimeUnits::eTimeStamp::StampElapsed);
        CHECK(NETimeUnits::timestamp(stamp + 83456000ull, stamp, stamp, true) == QStringLiteral("+00:01:23.456"));

        NETimeUnits::setStamp(NETimeUnits::eTimeStamp::StampDelta);
        CHECK(NETimeUnits::timestamp(stamp + 12480ull, stamp, stamp, true) == NETimeUnits::offset(12480));

        // The whole reading is what the tool tip and the copied line carry.
        CHECK(NETimeUnits::fullTimestamp(stamp) == QStringLiteral("2026-09-01 10:22:04.121408"));
        CHECK(NETimeUnits::fullTimestamp(0) == QString());

        CHECK(NETimeUnits::dayOf(stamp + oneDay) == (NETimeUnits::dayOf(stamp) + 1));
        CHECK(NETimeUnits::dayOf(stamp + 3600000000ull) == NETimeUnits::dayOf(stamp));
        CHECK(NETimeUnits::isRelative(NETimeUnits::eTimeStamp::StampDelta));
        CHECK(NETimeUnits::isRelative(NETimeUnits::eTimeStamp::StampTime) == false);

        NETimeUnits::setStamp(NETimeUnits::eTimeStamp::StampTime);

        TestLogModel model;
        std::vector<areg::SharedBuffer> entries;
        entries.push_back(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo, 1u, 10u, 100u, 0u
                                   , stamp, stamp + 500ull, "first of the day"));
        entries.push_back(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo, 1u, 10u, 100u, 0u
                                   , stamp + 3600000000ull, stamp + 3600000500ull, "one hour later"));
        entries.push_back(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo, 1u, 10u, 100u, 0u
                                   , stamp + oneDay, stamp + oneDay + 500ull, "the next day"));
        model.addEntries(std::move(entries));
        CHECK(model.rowCount() == 3);

        // The day is written on the row that opens it, and on nothing else.
        CHECK(model.timeOf(0, LoggingModelBase::eColumn::LogColumnTimestamp) == QStringLiteral("09-01 10:22:04.121"));
        CHECK(model.timeOf(1, LoggingModelBase::eColumn::LogColumnTimestamp) == QStringLiteral("11:22:04.121"));
        CHECK(model.timeOf(2, LoggingModelBase::eColumn::LogColumnTimestamp) == QStringLiteral("09-02 10:22:04.121"));

        // The first row opens no day, it has no row above it to differ from.
        CHECK(model.isDayChange(0) == false);
        CHECK(model.isDayChange(1) == false);
        CHECK(model.isDayChange(2));

        // The received column answers with its own measurement, not with the created one.
        NETimeUnits::setStamp(NETimeUnits::eTimeStamp::StampTimeMicro);
        CHECK(model.timeOf(1, LoggingModelBase::eColumn::LogColumnTimestamp)    == QStringLiteral("11:22:04.121408"));
        CHECK(model.timeOf(1, LoggingModelBase::eColumn::LogColumnTimeReceived) == QStringLiteral("11:22:04.121908"));

        // The elapsed shape counts from the first row of the window, in either column.
        NETimeUnits::setStamp(NETimeUnits::eTimeStamp::StampElapsed);
        CHECK(model.timeOf(0, LoggingModelBase::eColumn::LogColumnTimestamp) == QStringLiteral("+00:00:00.000"));
        CHECK(model.timeOf(1, LoggingModelBase::eColumn::LogColumnTimestamp) == QStringLiteral("+01:00:00.000"));

        // The delta shape counts from the row above.
        NETimeUnits::setStamp(NETimeUnits::eTimeStamp::StampDelta);
        CHECK(model.timeOf(0, LoggingModelBase::eColumn::LogColumnTimestamp) == NETimeUnits::offset(0));
        CHECK(model.timeOf(1, LoggingModelBase::eColumn::LogColumnTimestamp) == NETimeUnits::offset(3600000000ll));

        // The header names the two columns apart at any width. Both used to begin with "Time".
        CHECK(LoggingModelBase::getHeaderList().at(static_cast<int>(LoggingModelBase::eColumn::LogColumnTimestamp))
              != LoggingModelBase::getHeaderList().at(static_cast<int>(LoggingModelBase::eColumn::LogColumnTimeReceived)));
        CHECK(LoggingModelBase::getHeaderList().at(static_cast<int>(LoggingModelBase::eColumn::LogColumnTimeReceived)).startsWith(
              LoggingModelBase::getHeaderList().at(static_cast<int>(LoggingModelBase::eColumn::LogColumnTimestamp))) == false);

        NETimeUnits::setUnit(keepUnit);
        NETimeUnits::setStamp(keepStamp);
    }

    std::printf("[problem] the predicate the marks, F8 and the output window share\n");
    {
        const areg::SharedBuffer fatal  { makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioFatal  , 1u, 2u, 3u, 0u, 10, 10, "f") };
        const areg::SharedBuffer error  { makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioError  , 1u, 2u, 3u, 0u, 10, 10, "e") };
        const areg::SharedBuffer warning{ makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioWarning, 1u, 2u, 3u, 0u, 10, 10, "w") };
        const areg::SharedBuffer info   { makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo   , 1u, 2u, 3u, 0u, 10, 10, "i") };
        const areg::SharedBuffer debug  { makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioDebug  , 1u, 2u, 3u, 0u, 10, 10, "d") };

        CHECK(LoggingModelBase::isProblemEntry(reinterpret_cast<const areg::LogEntry*>(fatal.buffer())));
        CHECK(LoggingModelBase::isProblemEntry(reinterpret_cast<const areg::LogEntry*>(error.buffer())));
        CHECK(LoggingModelBase::isProblemEntry(reinterpret_cast<const areg::LogEntry*>(warning.buffer())));
        CHECK(LoggingModelBase::isProblemEntry(reinterpret_cast<const areg::LogEntry*>(info.buffer())) == false);
        CHECK(LoggingModelBase::isProblemEntry(reinterpret_cast<const areg::LogEntry*>(debug.buffer())) == false);
        CHECK(LoggingModelBase::isProblemEntry(nullptr) == false);
    }

    std::printf("[refused] the scope tree keeps rows out from the moment it was unchecked\n");
    {
        TestLogModel model;
        std::vector<areg::SharedBuffer> entries;
        entries.push_back(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo, 1u, 10u, 100u, 0u, 1000, 1000, "before"));
        entries.push_back(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo, 1u, 10u, 100u, 0u, 3000, 3000, "after"));
        entries.push_back(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo, 1u, 10u, 200u, 0u, 3000, 3000, "other scope"));
        entries.push_back(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo, 2u, 10u, 100u, 0u, 3000, 3000, "other target"));
        model.addEntries(std::move(entries));
        CHECK(model.rowCount() == 4);

        CHECK(model.hasRefusedScopes() == false);
        model.setScopesRefused(1u, QSet<uint32_t>{ 100u }, true, 2000);
        CHECK(model.hasRefusedScopes());

        // The rows already collected stay; the ones the scope produced afterwards do not.
        CHECK(model.isEntryRefused(model.getLogData(0)) == false);
        CHECK(model.isEntryRefused(model.getLogData(1)));
        CHECK(model.isEntryRefused(model.getLogData(2)) == false);
        CHECK(model.isEntryRefused(model.getLogData(3)) == false);

        // Letting the scope back in closes the span, and the rows of that stretch stay out.
        model.setScopesRefused(1u, QSet<uint32_t>{ 100u }, false, 4000);
        CHECK(model.isEntryRefused(model.getLogData(1)));

        model.clearRefusedScopes();
        CHECK(model.hasRefusedScopes() == false);
        CHECK(model.isEntryRefused(model.getLogData(1)) == false);

        // An archive refuses the whole session, and lifting it leaves nothing behind.
        model.setScopesRefused(1u, QSet<uint32_t>{ 100u }, true, 0);
        CHECK(model.isEntryRefused(model.getLogData(0)));
        CHECK(model.isEntryRefused(model.getLogData(1)));
        model.setScopesRefused(1u, QSet<uint32_t>{ 100u }, false, 5000);
        CHECK(model.isEntryRefused(model.getLogData(0)) == false);
        CHECK(model.isEntryRefused(model.getLogData(1)) == false);
    }

    std::printf("[filter] what the log window draws\n");
    {
        TestLogModel model;
        std::vector<areg::SharedBuffer> entries;
        //                                                                    prio                          cookie thread scope session time
        entries.push_back(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioDebug  , 1u, 10u, 100u, 7u, 1000, 1000, "alpha debug"));
        entries.push_back(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioWarning, 1u, 10u, 100u, 7u, 1100, 1100, "alpha warning"));
        entries.push_back(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioError  , 1u, 11u, 101u, 8u, 1200, 1200, "beta error"));
        entries.push_back(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo   , 2u, 12u, 102u, 9u, 1300, 1300, "gamma info"));
        model.addEntries(std::move(entries));

        LogViewerFilter filter(&model);
        CHECK(filter.rowCount() == 4);
        CHECK(filter.hasWindowFilters() == false);

        // The priority bar as a view filter: at least Warning.
        const uint16_t atLeastWarning{ static_cast<uint16_t>(static_cast<uint16_t>(areg::LogPriority::PrioWarning)
                                                           | static_cast<uint16_t>(areg::LogPriority::PrioError)
                                                           | static_cast<uint16_t>(areg::LogPriority::PrioFatal)) };
        filter.setViewPriority(atLeastWarning);
        CHECK(filter.viewPriority() == atLeastWarning);
        CHECK(filter.rowCount() == 2);
        CHECK(filter.hasWindowFilters());

        filter.setViewPriority(0);
        CHECK(filter.rowCount() == 4);

        // Isolating one process, one thread, one scope and one call.
        LogViewerFilter::sIsolation isolation;
        isolation.kind   = LogViewerFilter::eIsolation::IsolationProcess;
        isolation.cookie = 1u;
        filter.setIsolation(isolation);
        CHECK(filter.hasIsolation());
        CHECK(filter.rowCount() == 3);

        isolation.kind   = LogViewerFilter::eIsolation::IsolationThread;
        isolation.thread = 10u;
        filter.setIsolation(isolation);
        CHECK(filter.rowCount() == 2);

        isolation.kind    = LogViewerFilter::eIsolation::IsolationScope;
        isolation.scopeId = 100u;
        filter.setIsolation(isolation);
        CHECK(filter.rowCount() == 2);

        isolation.kind      = LogViewerFilter::eIsolation::IsolationCall;
        isolation.sessionId = 7u;
        filter.setIsolation(isolation);
        CHECK(filter.rowCount() == 2);

        isolation.kind = LogViewerFilter::eIsolation::IsolationNone;
        filter.setIsolation(isolation);
        CHECK(filter.hasIsolation() == false);
        CHECK(filter.rowCount() == 4);

        // A scope the tree unchecked leaves the window even with no column filter on.
        model.setScopesRefused(1u, QSet<uint32_t>{ 100u }, true, 0);
        CHECK(filter.rowCount() == 2);

        // A search hit the filters hide comes back, and says it was brought back.
        filter.revealRow(0);
        CHECK(filter.hasRevealedRows());
        CHECK(filter.rowCount() == 3);

        bool revealedMarked{ false };
        for (int row = 0; row < filter.rowCount(); ++row)
        {
            const QModelIndex index{ filter.index(row, 0) };
            if (filter.mapToSource(index).row() == 0)
            {
                revealedMarked = static_cast<QAbstractItemModel*>(&filter)->data(index, LogViewerFilter::RevealedRole).toBool();
            }
        }

        CHECK(revealedMarked);

        filter.clearRevealedRows();
        CHECK(filter.hasRevealedRows() == false);
        CHECK(filter.rowCount() == 2);

        model.clearRefusedScopes();
        CHECK(filter.rowCount() == 4);

        // The message column filter, and the phrase that matches nothing.
        const int messageIndex{ model.fromColumnToIndex(LoggingModelBase::eColumn::LogColumnMessage) };
        CHECK(messageIndex >= 0);

        NELusanCommon::FilterString phrase;
        phrase.text = QStringLiteral("alpha");
        filter.setTextFilter(messageIndex, phrase);
        CHECK(filter.rowCount() == 2);
        CHECK(filter.hasColumnFilters());

        phrase.text = QStringLiteral("nothing matches this");
        filter.setTextFilter(messageIndex, phrase);
        CHECK(filter.rowCount() == 0);

        phrase.text = QString();
        filter.setTextFilter(messageIndex, phrase);
        CHECK(filter.rowCount() == 4);
        CHECK(filter.hasColumnFilters() == false);
    }

    std::printf("[skew] the clocks that disagree\n");
    {
        constexpr qint64 aheadMs{ 300 };

        // A source ahead of the collector on every entry, over the sample floor.
        LogClockSkew skew;
        for (uint32_t i = 0; i < LogClockSkew::MIN_SAMPLES; ++i)
        {
            const areg::SharedBuffer entry{ makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo
                                                    , 1u, 10u, 100u, 0u
                                                    , static_cast<TIME64>(1000000 + i * 1000 + aheadMs * 1000)
                                                    , static_cast<TIME64>(1000000 + i * 1000), "m") };
            skew.feed(*reinterpret_cast<const areg::LogEntry*>(entry.buffer()));
        }

        CHECK(skew.hasSkew());
        CHECK(skew.report().offsetUs >= LogClockSkew::MIN_OFFSET_US);
        CHECK(skew.report().source.isEmpty() == false);

        skew.reset();
        CHECK(skew.hasSkew() == false);

        // One entry short of the floor is not judged, however far ahead it is.
        LogClockSkew tooFew;
        for (uint32_t i = 0; i < (LogClockSkew::MIN_SAMPLES - 1u); ++i)
        {
            const areg::SharedBuffer entry{ makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo
                                                    , 1u, 10u, 100u, 0u
                                                    , static_cast<TIME64>(1000000 + i * 1000 + aheadMs * 1000)
                                                    , static_cast<TIME64>(1000000 + i * 1000), "m") };
            tooFew.feed(*reinterpret_cast<const areg::LogEntry*>(entry.buffer()));
        }

        CHECK(tooFew.hasSkew() == false);

        // Ahead by less than the smallest offset worth a warning: the clocks agree in practice.
        LogClockSkew tooClose;
        for (uint32_t i = 0; i < (LogClockSkew::MIN_SAMPLES * 2u); ++i)
        {
            const areg::SharedBuffer entry{ makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo
                                                    , 1u, 10u, 100u, 0u
                                                    , static_cast<TIME64>(1000000 + i * 1000 + 100000)
                                                    , static_cast<TIME64>(1000000 + i * 1000), "m") };
            tooClose.feed(*reinterpret_cast<const areg::LogEntry*>(entry.buffer()));
        }

        CHECK(tooClose.hasSkew() == false);

        // Ahead on half the entries only, which is below the share a warning needs.
        LogClockSkew halfAhead;
        for (uint32_t i = 0; i < (LogClockSkew::MIN_SAMPLES * 2u); ++i)
        {
            const TIME64 received { static_cast<TIME64>(1000000 + i * 1000) };
            const TIME64 timestamp{ (i % 2u) == 0u ? received + (aheadMs * 1000) : received - (aheadMs * 1000) };
            const areg::SharedBuffer entry{ makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo
                                                    , 1u, 10u, 100u, 0u, timestamp, received, "m") };
            halfAhead.feed(*reinterpret_cast<const areg::LogEntry*>(entry.buffer()));
        }

        CHECK(halfAhead.hasSkew() == false);
    }

    // The cost probe walks a table of a hundred thousand rows several times over. It is not
    // part of the run the suite makes, and it is asked for by name.
    if ((argc > 1) && (std::strcmp(argv[1], "--cost") == 0))
    {
        std::printf("[cost] what one filter change costs on a table that already holds rows\n");

        constexpr int rows { 100000 };
        constexpr int every{ 7 };

        TestLogModel model;
        std::vector<areg::SharedBuffer> entries;
        entries.reserve(rows);
        for (int i = 0; i < rows; ++i)
        {
            // "timeout" lands on every seventh row, "leading" on an unbroken block of the
            // same size. The two keep the same number of rows and cost the same to test.
            std::string text{ "connection " + std::to_string(i) };
            text += ((i % every) == 0) ? " timeout while reading" : " established";
            if (i < (rows / every))
            {
                text += " leading";
            }

            entries.push_back(makeEntry(areg::LogMessageType::MessageText
                                       , (i % 5) == 0 ? areg::LogPriority::PrioError : areg::LogPriority::PrioInfo
                                       , static_cast<ITEM_ID>(1 + (i % 4)), static_cast<ITEM_ID>(10 + (i % 9))
                                       , static_cast<uint32_t>(100 + (i % 25)), 0u
                                       , static_cast<TIME64>(1000000 + i), static_cast<TIME64>(1000000 + i)
                                       , text.c_str()));
        }

        model.addEntries(std::move(entries));
        LogViewerFilter filter(&model);
        CHECK(filter.rowCount() == rows);

        const int message{ model.fromColumnToIndex(LoggingModelBase::eColumn::LogColumnMessage) };
        QElapsedTimer clock;

        // The row count is asked for inside the measure. A proxy that drops its mapping does
        // the work on the first question put to it, and a timer stopped before that reads zero.
        const auto measure = [&](const char* what, const QString& phrase) {
                clock.start();
                filter.setTextFilter(message, NELusanCommon::FilterString{ phrase, false, false, false });
                const int kept{ filter.rowCount() };
                const qint64 applied{ clock.elapsed() };

                clock.start();
                filter.setTextFilter(message, NELusanCommon::FilterString{ });
                const int back{ filter.rowCount() };
                const qint64 dropped{ clock.elapsed() };

                std::printf("  %-22s kept %6d of %d | apply %6lld ms | drop %6lld ms\n"
                           , what, kept, rows
                           , static_cast<long long>(applied), static_cast<long long>(dropped));
                CHECK(kept > 0);
                CHECK(back == rows);
            };

        measure("scattered (every 7th)", QStringLiteral("timeout"));
        measure("one unbroken block"   , QStringLiteral("leading"));
    }

    std::printf("[select] the predicate the Isolate and the Select menus share\n");
    {
        //                                                           prio                          cookie thread scope session time
        const areg::SharedBuffer one{ makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo, 1u, 10u, 100u, 7u, 1000, 1000, "alpha") };
        const areg::LogEntry* entry{ reinterpret_cast<const areg::LogEntry*>(one.buffer()) };

        LogViewerFilter::sIsolation pick;
        CHECK(LogViewerFilter::matchesIsolation(pick, entry));   // IsolationNone takes every row
        CHECK(LogViewerFilter::matchesIsolation(pick, nullptr));

        pick.kind      = LogViewerFilter::eIsolation::IsolationCall;
        pick.cookie    = 1u;
        pick.thread    = 10u;
        pick.scopeId   = 100u;
        pick.sessionId = 7u;
        CHECK(LogViewerFilter::matchesIsolation(pick, entry));
        CHECK(LogViewerFilter::matchesIsolation(pick, nullptr) == false);

        // The call is one run of one scope: another run of the same scope is not it.
        pick.sessionId = 8u;
        CHECK(LogViewerFilter::matchesIsolation(pick, entry) == false);

        // The scope is every run of it, the thread is every scope of it, the process is all.
        pick.kind = LogViewerFilter::eIsolation::IsolationScope;
        CHECK(LogViewerFilter::matchesIsolation(pick, entry));
        pick.kind = LogViewerFilter::eIsolation::IsolationThread;
        CHECK(LogViewerFilter::matchesIsolation(pick, entry));
        pick.kind = LogViewerFilter::eIsolation::IsolationProcess;
        CHECK(LogViewerFilter::matchesIsolation(pick, entry));

        // Every kind is bounded by the process the row came from.
        pick.cookie = 2u;
        CHECK(LogViewerFilter::matchesIsolation(pick, entry) == false);
    }

    std::printf("[analyzer] the picked rows, and the rows taken out of the view\n");
    {
        TestLogModel model;
        std::vector<areg::SharedBuffer> entries;
        for (int i = 0; i < 6; ++i)
        {
            entries.push_back(makeEntry( areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo
                                       , 1u, 10u, 100u, 7u
                                       , 1000 + i, 1000 + i, "row"));
        }

        model.addEntries(std::move(entries));

        ScopeLogViewerFilter filter;
        filter.setRowFilter(&model, QList<int>{ 1, 3, 5 });
        CHECK(filter.isRowFilter());
        CHECK(filter.rowCount() == 3);

        // The picked rows are the ones the log window marks as read apart.
        CHECK(filter.filterExactMatch(model.index(1, 0)));
        CHECK(filter.filterExactMatch(model.index(2, 0)) == false);

        // A row taken out of the view goes, and comes back whole.
        filter.hideRows(QList<int>{ 3 });
        CHECK(filter.hasHiddenRows());
        CHECK(filter.rowCount() == 2);
        filter.showHiddenRows();
        CHECK(filter.hasHiddenRows() == false);
        CHECK(filter.rowCount() == 3);

        // Reading a call again drops the picked set with everything else.
        filter.setScopeFilter(&model, 100u, 7u, 10u, 1u);
        CHECK(filter.isRowFilter() == false);
        CHECK(filter.rowCount() == 6);
    }

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
