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
#include "lusan/model/log/LogClockSkew.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"

#include "tests/common/UiTestEnv.hpp"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QSet>
#include <QStyle>
#include <QString>

#include <cstdio>
#include <cstring>

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

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
