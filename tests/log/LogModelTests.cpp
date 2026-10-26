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
#include "lusan/model/log/OfflineLogsModel.hpp"
#include "lusan/model/log/ScopeLogViewerFilter.hpp"

#include "tests/common/UiTestEnv.hpp"

#include "areg/base/DateTime.hpp"
#include "aregextend/db/LogSqliteDatabase.hpp"
#include "aregextend/db/SqliteDatabase.hpp"
#include "aregextend/db/SqliteStatement.hpp"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QColor>
#include <QDateTime>
#include <QPalette>
#include <QElapsedTimer>
#include <QSet>
#include <QStyle>
#include <QString>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

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
            std::vector<uint32_t> ids(entries.size(), 0u);
            appendLogBatch(std::move(entries), std::move(ids), mLoadGeneration);
        }

        void addEntry(const areg::SharedBuffer& entry, uint32_t rowId)
        {
            appendLogEntry(entry, rowId);
        }

        void dropOldest(uint32_t count)
        {
            evictOldest(count);
        }

        void setWindow(int32_t rows)
        {
            mLogChunk = rows;
        }

        void setFiltered(bool filtered)
        {
            mFiltered = filtered;
        }

        bool readsFilteredLogs() const override
        {
            return mFiltered;
        }

        bool mFiltered{ false };

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

    using LogDb = areg::ext::LogSqliteDatabase;

    //!< One row of a log file in the order a viewer reads it.
    struct FileRow
    {
        uint32_t    id      { 0u };
        uint32_t    scope   { 0u };
        uint32_t    prio    { 0u };
        std::string message { };
    };

    QString tempFile(const char* name)
    {
        const QString path{ QDir(QDir::tempPath()).filePath(QString::fromLatin1(name)) };
        QFile::remove(path);
        QFile::remove(path + QStringLiteral("-wal"));
        QFile::remove(path + QStringLiteral("-shm"));
        return path;
    }

    //!< The scope of a row: 1 row in 100 has no scope, the others cycle through scopes 1 to 4.
    uint32_t scopeOfRow(uint32_t i)
    {
        return ((i % 100u) == 0u ? 0u : 1u + (i % 4u));
    }

    /**
     * \brief   Writes a log file of the given number of rows through the writer of the log
     *          library. Many rows share a timestamp and the times do not follow the row ids.
     *          With markers, a markers table holds 5 markers set on rows of the file.
     **/
    bool writeLogFile(const QString& path, uint32_t rows, bool markers)
    {
        constexpr areg::LogPriority prios[]{ areg::LogPriority::PrioDebug, areg::LogPriority::PrioInfo, areg::LogPriority::PrioWarning, areg::LogPriority::PrioError };

        LogDb writer;
        if (writer.connect(areg::String(path.toStdString().c_str()), false) == false)
            return false;

        const areg::DateTime now{ areg::DateTime::now() };
        bool result{ writer.begin() };
        for (uint32_t cookie = 1u; result && (cookie <= 2u); ++cookie)
        {
            for (uint32_t scope = 1u; result && (scope <= 4u); ++scope)
            {
                result = writer.log_scope_activate(areg::String("scope_") + areg::String::make_string(scope), scope, static_cast<uint32_t>(areg::LogPriority::PrioScopeLogs), cookie, now);
            }
        }

        for (uint32_t i = 0u; result && (i < rows); ++i)
        {
            const std::string text{ "row " + std::to_string(i) };
            const areg::SharedBuffer buf{ makeEntry( areg::LogMessageType::MessageText, prios[i % 4u], 1u + (i % 2u), 10u
                                                   , scopeOfRow(i), 0u
                                                   , static_cast<TIME64>(1000000u + ((static_cast<uint64_t>(i) * 7919u) % (rows / 7u + 1u)))
                                                   , 0, text.c_str()) };
            result = writer.log_message(*reinterpret_cast<const areg::LogEntry*>(buf.buffer()));
            if (result && ((i % 10000u) == 9999u))
            {
                result = writer.commit(true) && writer.begin();
            }
        }

        result = writer.commit(result) && result;
        for (uint32_t i = 0u; result && markers && (i < 5u); ++i)
        {
            result = writer.add_marker(10u + i * 7u, areg::LogPriority::PrioMarkerInfo, "tester", areg::String("note ") + areg::String::make_string(i)) != 0u;
        }

        writer.disconnect();
        return result;
    }

    //!< Reads the rows of the file in the order a viewer reads them, with a connection of its own.
    std::vector<FileRow> readFileRows(const QString& path)
    {
        std::vector<FileRow> result;
        areg::ext::SqliteDatabase db;
        if (db.connect(areg::String(path.toStdString().c_str()), areg::DatabaseRole::Viewer))
        {
            areg::ext::SqliteStatement stmt(db, "SELECT id, scope_id, msg_prio, msg_log FROM logs ORDER BY time_created, id;");
            while (stmt.next() == areg::ext::SqliteStatement::QueryResult::HasMore)
            {
                result.push_back(FileRow{ stmt.as_uint32(0), stmt.as_uint32(1), stmt.as_uint32(2), stmt.as_text(3).as_string() });
            }

            stmt.finalize();
            db.disconnect();
        }

        return result;
    }

    //!< Returns the text of one query on a connection of its own, the values of each row joined.
    std::string queryText(const QString& path, const char* sql)
    {
        std::string result;
        areg::ext::SqliteDatabase db;
        if (db.connect(areg::String(path.toStdString().c_str()), areg::DatabaseRole::Viewer))
        {
            areg::ext::SqliteStatement stmt(db, sql);
            while (stmt.next() == areg::ext::SqliteStatement::QueryResult::HasMore)
            {
                for (int32_t col = 0; col < stmt.column_count(); ++col)
                {
                    result += stmt.as_text(col).as_string();
                    result += "|";
                }

                result += "\n";
            }

            stmt.finalize();
            db.disconnect();
        }

        return result;
    }

    //!< True if the file is one plain file: no `-wal` or `-shm` beside it, journal mode `delete`.
    bool isPlainFile(const QString& path)
    {
        const bool noWal{ (QFile::exists(path + QStringLiteral("-wal")) == false) && (QFile::exists(path + QStringLiteral("-shm")) == false) };
        return noWal && (queryText(path, "PRAGMA journal_mode;") == "delete|\n");
    }

    //!< True if the loaded rows of the model are the rows of the file from `start` on, ids included.
    bool windowMatches(const LoggingModelBase& model, const std::vector<FileRow>& rows, uint32_t start)
    {
        const int count{ model.rowCount() };
        if ((start > rows.size()) || (count == 0) || (start + static_cast<uint32_t>(count) > rows.size()))
            return false;

        for (int row = 0; row < count; ++row)
        {
            const FileRow& expected{ rows[start + static_cast<uint32_t>(row)] };
            const areg::LogEntry* entry{ model.getLogData(row) };
            if ((entry == nullptr) || (model.getRowId(row) != expected.id) || (expected.message != entry->logMessage))
                return false;
        }

        return true;
    }

    //!< Waits until the model holds the given number of rows, or the time is out.
    bool waitForModelRows(const LoggingModelBase& model, int rows, int timeoutMs)
    {
        QElapsedTimer clock;
        clock.start();
        while ((model.rowCount() < rows) && (clock.elapsed() < timeoutMs))
        {
            QApplication::processEvents(QEventLoop::AllEvents, 20);
        }

        QApplication::processEvents();
        return (model.rowCount() == rows);
    }

    //!< The median of the measured times, in milliseconds.
    double medianMs(std::vector<qint64> nanos)
    {
        if (nanos.empty())
            return 0.0;

        std::sort(nanos.begin(), nanos.end());
        return static_cast<double>(nanos[nanos.size() / 2u]) / 1000000.0;
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

        // The message column filter, and the phrase that matches nothing. The proxy is told
        // the column, so it holds a filter whether or not the table shows that column.
        const LoggingModelBase::eColumn messageColumn{ LoggingModelBase::eColumn::LogColumnMessage };
        CHECK(model.getActiveColumns().contains(messageColumn));

        NELusanCommon::FilterString phrase;
        phrase.text = QStringLiteral("alpha");
        filter.setTextFilter(messageColumn, phrase);
        CHECK(filter.rowCount() == 2);
        CHECK(filter.hasColumnFilters());

        phrase.text = QStringLiteral("nothing matches this");
        filter.setTextFilter(messageColumn, phrase);
        CHECK(filter.rowCount() == 0);

        phrase.text = QString();
        filter.setTextFilter(messageColumn, phrase);
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

        const LoggingModelBase::eColumn message{ LoggingModelBase::eColumn::LogColumnMessage };
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

    // LUSAN_LOG_PERF set: a file of 760 000 rows and the window times at rows 0, 100 000,
    // 400 000 and 750 000. Otherwise a file of 20 000 rows.
    const bool perfRun{ qEnvironmentVariableIsSet("LUSAN_LOG_PERF") };
    const QString windowFile{ tempFile("lusan-window-test.sqlog") };
    const uint32_t fileRows{ perfRun ? 760000u : 20000u };
    QElapsedTimer writeClock;
    writeClock.start();
    const bool fileWritten{ writeLogFile(windowFile, fileRows, false) };
    std::printf("[windows] a log file of %u rows written in %lld ms\n", fileRows, static_cast<long long>(writeClock.elapsed()));
    CHECK(fileWritten);
    const std::vector<FileRow> fileAll{ readFileRows(windowFile) };
    CHECK(fileAll.size() > fileRows);

    std::printf("[windows] keyset windows hold the rows of the file in order, each with its row id\n");
    {
        TestLogModel model;
        model.setWindow(1000);
        model.openDatabase(windowFile, true);
        CHECK(model.isOperable());

        const uint32_t total{ static_cast<uint32_t>(fileAll.size()) };
        model.slideWindow(0u);
        CHECK(model.rowCount() == 1000);
        CHECK(model.getTotalLogCount() == total);
        CHECK(windowMatches(model, fileAll, 0u));

        // Shorter than a window: keyset forward, keyset backward.
        model.slideWindow(300u);
        CHECK(model.getWindowStart() == 300u);
        CHECK(windowMatches(model, fileAll, 300u));
        model.slideWindow(120u);
        CHECK(model.getWindowStart() == 120u);
        CHECK(windowMatches(model, fileAll, 120u));

        // Longer than a window: one read at the row number.
        model.slideWindow(total / 2u);
        CHECK(model.getWindowStart() == total / 2u);
        CHECK(windowMatches(model, fileAll, total / 2u));
        model.slideWindow(total / 2u + 999u);
        CHECK(windowMatches(model, fileAll, total / 2u + 999u));

        // Past the end: the window holds the last rows.
        model.slideWindow(total + 10u);
        CHECK(model.getWindowStart() == total - 1000u);
        CHECK(windowMatches(model, fileAll, total - 1000u));
        model.slideWindow(total - 1999u);
        CHECK(windowMatches(model, fileAll, total - 1999u));

        // The role answers the id as well, and an id belongs to its row.
        CHECK(model.data(model.index(5, 0), LoggingModelBase::RowIdRole).toUInt() == fileAll[total - 1999u + 5u].id);
        CHECK(model.getRowId(-1) == 0u);
        CHECK(model.getRowId(model.rowCount()) == 0u);

        // Filtered: scopes 1 and 2 pass, 3 and 4 do not, rows of no scope always pass.
        areg::ArrayList<LogDb::ScopeFilter> half;
        half.add(LogDb::ScopeFilter{ 1u, static_cast<uint32_t>(areg::LogPriority::PrioScopeLogs) });
        half.add(LogDb::ScopeFilter{ 2u, static_cast<uint32_t>(areg::LogPriority::PrioScopeLogs) });
        half.add(LogDb::ScopeFilter{ 3u, 0u });
        half.add(LogDb::ScopeFilter{ 4u, 0u });
        model.setFiltered(true);
        CHECK(model.applyFilters(areg::TARGET_ALL, half));
        model.dataReset();

        std::vector<FileRow> passed;
        for (const FileRow& row : fileAll)
        {
            if ((row.scope == 0u) || (row.scope == 1u) || (row.scope == 2u))
                passed.push_back(row);
        }

        const uint32_t filtered{ static_cast<uint32_t>(passed.size()) };
        model.slideWindow(0u);
        CHECK(model.getTotalLogCount() == filtered);
        CHECK(windowMatches(model, passed, 0u));
        model.slideWindow(500u);
        CHECK(windowMatches(model, passed, 500u));
        model.slideWindow(filtered / 3u);
        CHECK(windowMatches(model, passed, filtered / 3u));
        model.slideWindow(filtered / 3u - 700u);
        CHECK(windowMatches(model, passed, filtered / 3u - 700u));

        model.closeDatabase();
        CHECK(model.rowCount() == 0);
    }

    std::printf("[windows] the row id follows its row through sorting and filtering\n");
    {
        TestLogModel model;
        model.setWindow(1000);
        model.openDatabase(windowFile, true);
        model.slideWindow(2000u);

        LogViewerFilter filter(&model);
        const int messageColumn{ model.findColumn(LoggingModelBase::eColumn::LogColumnMessage) };
        CHECK(messageColumn >= 0);
        filter.sort(messageColumn, Qt::SortOrder::DescendingOrder);
        filter.setTextFilter(LoggingModelBase::eColumn::LogColumnMessage, NELusanCommon::FilterString{ QStringLiteral("7"), false, false, false });
        CHECK((filter.rowCount() > 0) && (filter.rowCount() < model.rowCount()));

        std::vector<std::string> byId(static_cast<size_t>(fileAll.size()) + 2u);
        for (const FileRow& row : fileAll)
        {
            if (row.id < byId.size())
                byId[row.id] = row.message;
        }

        bool sorted{ true };
        bool idsMatch{ true };
        QString previous;
        for (int row = 0; row < filter.rowCount(); ++row)
        {
            const QModelIndex idx{ filter.index(row, messageColumn) };
            const uint32_t rowId{ idx.data(LoggingModelBase::RowIdRole).toUInt() };
            const areg::LogEntry* entry{ idx.data(Qt::ItemDataRole::UserRole).value<const areg::LogEntry*>() };
            idsMatch = idsMatch && (entry != nullptr) && (rowId != 0u) && (rowId < byId.size()) && (byId[rowId] == entry->logMessage);
            const QString text{ idx.data(Qt::ItemDataRole::DisplayRole).toString() };
            sorted = sorted && ((row == 0) || (QString::compare(previous, text) >= 0));
            previous = text;
        }

        CHECK(sorted);
        CHECK(idsMatch);
        model.closeDatabase();
    }

    std::printf("[windows] received rows keep their ids in step when the oldest are dropped\n");
    {
        TestLogModel model;
        for (uint32_t i = 0u; i < 250u; ++i)
        {
            const std::string text{ "live " + std::to_string(i) };
            model.addEntry(makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo, 1u, 10u, 0u, 0u, 1000 + i, 1000 + i, text.c_str()), 5000u + i);
        }

        model.dropOldest(100u);
        CHECK(model.rowCount() == 150);
        bool inStep{ true };
        for (int row = 0; row < model.rowCount(); ++row)
        {
            const std::string text{ "live " + std::to_string(row + 100) };
            inStep = inStep && (model.getRowId(row) == 5100u + static_cast<uint32_t>(row)) && (text == model.getLogData(row)->logMessage);
        }

        CHECK(inStep);
        model.dropOldest(1000u);
        CHECK(model.rowCount() == 0);
        CHECK(model.getRowId(0) == 0u);
    }

    std::printf("[windows] the offline model reads the whole file by keyset, every row with its id\n");
    {
        OfflineLogsModel model;
        model.openDatabase(windowFile, true);
        const int all{ static_cast<int>(fileAll.size()) };
        CHECK(waitForModelRows(model, all, 120000));
        bool idsMatch{ model.rowCount() == all };
        for (int row = 0; idsMatch && (row < all); ++row)
        {
            idsMatch = (model.getRowId(row) == fileAll[static_cast<size_t>(row)].id);
        }

        CHECK(idsMatch);
        model.closeDatabase();
    }

    std::printf("[windows] times of a window of 1000 rows, ms, medians (%s build)\n",
#ifdef _DEBUG
                "Debug"
#else
                "Release"
#endif
                );
    {
        const uint32_t total{ static_cast<uint32_t>(fileAll.size()) };
        const std::vector<uint32_t> places{ perfRun ? std::vector<uint32_t>{ 0u, 100000u, 400000u, 750000u }
                                                    : std::vector<uint32_t>{ 0u, 2500u, 10000u, 18000u } };
        QElapsedTimer clock;

        // The reading Lusan had before: LIMIT and OFFSET, no row id.
        LogDb before;
        before.connect(areg::String(windowFile.toStdString().c_str()), true);
        areg::ext::SqliteStatement beforeStmt(before.database());

        const auto measure = [&](TestLogModel& model, uint32_t count, const char* what, bool withOffset)
            {
                std::printf("  %-14s %10s %10s %10s %10s\n", what, "row", "next", "previous", "jump");
                for (uint32_t share : places)
                {
                    const uint32_t place{ (count == total) ? share : static_cast<uint32_t>((static_cast<uint64_t>(share) * count) / total) };
                    if (place + 2000u > count)
                        continue;

                    std::vector<qint64> next, previous, jump, offset;
                    for (int i = 0; i < 15; ++i)
                    {
                        model.slideWindow(place);
                        clock.start();
                        model.slideWindow(place + 999u);
                        next.push_back(clock.nsecsElapsed());
                        clock.start();
                        model.slideWindow(place);
                        previous.push_back(clock.nsecsElapsed());
                        if (model.mFiltered == false)
                        {
                            CHECK(windowMatches(model, fileAll, place));
                        }
                    }

                    for (int i = 0; i < 5; ++i)
                    {
                        model.slideWindow(place < count / 2u ? count : 0u);
                        clock.start();
                        model.slideWindow(place);
                        jump.push_back(clock.nsecsElapsed());

                        if (withOffset)
                        {
                            std::vector<areg::SharedBuffer> logs(1000u);
                            clock.start();
                            before.setup_statement_read_logs(beforeStmt, areg::TARGET_ALL, 1000, place);
                            LogDb::fill_log_messages(logs, beforeStmt, 0u, 1000);
                            beforeStmt.reset();
                            offset.push_back(clock.nsecsElapsed());
                        }
                    }

                    std::printf("  %-14s %10u %10.2f %10.2f %10.2f", what, place, medianMs(next), medianMs(previous), medianMs(jump));
                    if (withOffset)
                        std::printf("   before: OFFSET %.2f", medianMs(offset));
                    std::printf("\n");
                }
            };

        TestLogModel model;
        model.setWindow(1000);
        model.openDatabase(windowFile, true);
        measure(model, total, "plain", true);

        areg::ArrayList<LogDb::ScopeFilter> half;
        half.add(LogDb::ScopeFilter{ 1u, static_cast<uint32_t>(areg::LogPriority::PrioScopeLogs) });
        half.add(LogDb::ScopeFilter{ 2u, static_cast<uint32_t>(areg::LogPriority::PrioScopeLogs) });
        half.add(LogDb::ScopeFilter{ 3u, 0u });
        half.add(LogDb::ScopeFilter{ 4u, 0u });
        model.setFiltered(true);
        model.applyFilters(areg::TARGET_ALL, half);
        model.dataReset();
        model.slideWindow(0u);
        measure(model, model.getTotalLogCount(), "filter ~50%", false);

        areg::ArrayList<LogDb::ScopeFilter> rare;
        for (uint32_t scope = 1u; scope <= 4u; ++scope)
        {
            rare.add(LogDb::ScopeFilter{ scope, 0u });
        }

        model.applyFilters(areg::TARGET_ALL, rare);
        model.dataReset();
        model.slideWindow(0u);
        std::printf("  filter ~1%% passes %u of %u rows\n", model.getTotalLogCount(), total);
        measure(model, model.getTotalLogCount(), "filter ~1%", false);
        model.closeDatabase();

        // The offline load of the whole file: the keyset steps against the stream Lusan had before.
        LogDb reader;
        reader.connect(areg::String(windowFile.toStdString().c_str()), true);
        reader.setup_filter_logs(areg::TARGET_ALL, areg::ArrayList<LogDb::ScopeFilter>{});
        areg::ext::SqliteStatement stmt(reader.database());
        for (int round = 0; round < 2; ++round)
        {
            uint32_t rowsNew{ 0u }, rowsOld{ 0u };
            qint64 firstNew{ 0 }, firstOld{ 0 };
            clock.start();
            LogDb::LogPosition place{};
            for (int32_t read = 1000; read == 1000; )
            {
                std::vector<areg::SharedBuffer> logs;
                std::vector<uint32_t> ids;
                reader.setup_statement_read_filter_window(stmt, areg::TARGET_ALL, place, 1000);
                read = LogDb::fill_log_window(logs, stmt, 1000, place, ids);
                rowsNew += static_cast<uint32_t>(read);
                firstNew = (firstNew == 0) ? clock.nsecsElapsed() : firstNew;
            }

            const qint64 allNew{ clock.nsecsElapsed() };
            clock.start();
            reader.setup_statement_read_filter_logs(stmt, areg::TARGET_ALL);
            for (int32_t read = 1000; read == 1000; )
            {
                std::vector<areg::SharedBuffer> logs(1000u);
                read = LogDb::fill_log_messages(logs, stmt, 0u, 1000);
                rowsOld += static_cast<uint32_t>(read);
                firstOld = (firstOld == 0) ? clock.nsecsElapsed() : firstOld;
            }

            const qint64 allOld{ clock.nsecsElapsed() };
            stmt.reset();
            std::printf("  offline load of %u rows: keyset %.0f ms (first 1000 in %.2f ms), before: stream of %u rows %.0f ms (first 1000 in %.2f ms)\n"
                       , rowsNew, allNew / 1e6, firstNew / 1e6, rowsOld, allOld / 1e6, firstOld / 1e6);
            CHECK(rowsNew == rowsOld);
        }

        stmt.finalize();
        reader.disconnect();
        beforeStmt.finalize();
        before.disconnect();
    }

    QFile::remove(windowFile);

    std::printf("[markers] a file with a markers table opens, scrolls, filters and closes; the table stays as it was\n");
    {
        const QString path{ tempFile("lusan-markers-test.sqlog") };
        CHECK(writeLogFile(path, 5000u, true));
        const std::string markersBefore{ queryText(path, "SELECT id, master_id, marker_prio, author, note, time_set FROM markers ORDER BY id;") };
        const std::string schemaBefore{ queryText(path, "SELECT type, name, sql FROM sqlite_master WHERE tbl_name = 'markers' ORDER BY name;") };
        CHECK(std::count(markersBefore.begin(), markersBefore.end(), '\n') == 5);
        CHECK(isPlainFile(path));
        const std::vector<FileRow> rows{ readFileRows(path) };

        OfflineLogsModel model;
        model.openDatabase(path, true);
        CHECK(waitForModelRows(model, static_cast<int>(rows.size()), 30000));
        CHECK(model.getRowId(0) == rows.front().id);

        model.slideWindow(1200u);
        CHECK(windowMatches(model, rows, 1200u));
        model.slideWindow(1500u);
        CHECK(windowMatches(model, rows, 1500u));

        areg::ArrayList<LogDb::ScopeFilter> filter;
        filter.add(LogDb::ScopeFilter{ 3u, 0u });
        CHECK(model.applyFilters(areg::TARGET_ALL, filter));
        model.readLogsAsynchronous(-1);
        int passed{ 0 };
        for (const FileRow& row : rows)
        {
            passed += (row.scope != 3u) ? 1 : 0;
        }

        CHECK(waitForModelRows(model, passed, 30000));
        model.closeDatabase();

        CHECK(isPlainFile(path));
        CHECK(queryText(path, "SELECT id, master_id, marker_prio, author, note, time_set FROM markers ORDER BY id;") == markersBefore);
        CHECK(queryText(path, "SELECT type, name, sql FROM sqlite_master WHERE tbl_name = 'markers' ORDER BY name;") == schemaBefore);
        QFile::remove(path);
    }

    std::printf("[close order] the reader closes before the writer: the writer leaves one plain file\n");
    {
        const QString path{ tempFile("lusan-close-test.sqlog") };
        CHECK(writeLogFile(path, 3000u, false));

        // The writer stays open as the log observer does while a session runs.
        LogDb writer;
        CHECK(writer.connect(areg::String(path.toStdString().c_str()), false));
        CHECK(writer.begin());
        const areg::SharedBuffer extra{ makeEntry(areg::LogMessageType::MessageText, areg::LogPriority::PrioInfo, 1u, 10u, 0u, 0u, 5000000, 0, "last") };
        CHECK(writer.log_message(*reinterpret_cast<const areg::LogEntry*>(extra.buffer())));
        CHECK(writer.commit(true));

        TestLogModel model;
        model.setWindow(1000);
        model.openDatabase(path, true);
        model.slideWindow(500u);
        CHECK(model.rowCount() == 1000);

        model.closeDatabase();
        writer.disconnect();
        CHECK(isPlainFile(path));

        const std::vector<FileRow> rows{ readFileRows(path) };
        const uint32_t lastStart{ static_cast<uint32_t>(rows.size()) - 1000u };
        CHECK(std::count_if(rows.begin(), rows.end(), [](const FileRow& row) { return row.message == "last"; }) == 1);
        model.openDatabase(path, true);
        model.slideWindow(lastStart);
        CHECK(windowMatches(model, rows, lastStart));
        model.closeDatabase();
        CHECK(isPlainFile(path));

        // The control: a reader still open when the writer closes leaves the file in WAL.
        LogDb second;
        CHECK(second.connect(areg::String(path.toStdString().c_str()), false));
        model.openDatabase(path, true);
        model.slideWindow(0u);
        second.disconnect();
        model.closeDatabase();
        CHECK(isPlainFile(path) == false);
        QFile::remove(path);
        QFile::remove(path + QStringLiteral("-wal"));
        QFile::remove(path + QStringLiteral("-shm"));
    }

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
