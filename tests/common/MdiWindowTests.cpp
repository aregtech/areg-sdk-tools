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
 *  \file        tests/common/MdiWindowTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       What the assembled main window promises its document and log windows:
 *
 *                 1. Every document window keeps its page tabs inside the editor area,
 *                    whatever height the Output dock leaves for it. The three document
 *                    kinds behave the same way.
 *                 2. The live log window stops following the newest log as soon as the
 *                    user picks a row, and follows again when the end button is pressed.
 *
 *  Usage: lusan_mdi_window_tests
 *
 ************************************************************************/

#include "lusan/app/LusanApplication.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/view/common/MdiChild.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/log/LiveLogViewer.hpp"
#include "lusan/view/log/LogSessionBar.hpp"

#include "tests/common/UiTestEnv.hpp"

#include "areg/logging/LoggingDefs.hpp"
#include "aregextend/db/LogSqliteDatabase.hpp"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QItemSelectionModel>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QStandardPaths>
#include <QTabBar>
#include <QTabWidget>
#include <QTableView>
#include <QToolButton>

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

    //!< The tab bar a document window carries under its pages.
    QTabBar* pageTabs(MdiChild* child)
    {
        QTabWidget* tabs{ child != nullptr
                          ? child->findChild<QTabWidget*>(QString(), Qt::FindDirectChildrenOnly)
                          : nullptr };
        return (tabs != nullptr ? tabs->tabBar() : nullptr);
    }

    //!< True when the whole tab bar lies inside what the editor area shows.
    bool tabsInSight(MdiChild* child, QMdiArea& area)
    {
        QTabBar* bar{ pageTabs(child) };
        if (bar == nullptr)
        {
            return false;
        }

        const int top{ bar->mapTo(area.viewport(), QPoint(0, 0)).y() };
        return (top >= 0) && ((top + bar->height()) <= area.viewport()->height());
    }

    //!< Writes a small archive of text logs, so a window has rows to select.
    bool writeArchive(const QString& path, int count)
    {
        areg::ext::LogSqliteDatabase db;
        if (db.connect(areg::String(path.toUtf8().constData()), false) == false)
        {
            return false;
        }

        for (int i = 0; i < count; ++i)
        {
            areg::LogEntry entry{ };
            entry.logMsgType     = areg::LogMessageType::MessageText;
            entry.logMessagePrio = areg::LogPriority::PrioInfo;
            entry.logDataType    = areg::LogDataType::Remote;
            entry.logCookie      = 1u;
            entry.logModuleId    = 1u;
            entry.logThreadId    = 2u;
            entry.logScopeId     = 3u;
            entry.logSessionId   = 0u;
            entry.logTimestamp   = 1000 + i;
            entry.logReceived    = 1000 + i;

            char text[64];
            std::snprintf(text, sizeof(text), "line %d", i);
            const uint32_t len{ static_cast<uint32_t>(std::strlen(text)) };
            std::memcpy(entry.logMessage, text, len);
            entry.logMessage[len] = '\0';
            entry.logMessageLen = len;
            std::memcpy(entry.logModule, "target", 7);
            entry.logModuleLen = 6u;
            std::memcpy(entry.logThread, "worker", 7);
            entry.logThreadLen = 6u;

            db.log_message(entry);
        }

        db.commit(true);
        db.disconnect();
        return true;
    }

    //!< Reaches the window's model, which the window itself owns and keeps to itself.
    class ArchiveLiveViewer : public LiveLogViewer
    {
    public:
        explicit ArchiveLiveViewer(MdiMainWindow* wndMain)
            : LiveLogViewer(wndMain)
        {
        }

        LoggingModelBase* logModel(void)
        {
            return mLogModel;
        }
    };

    //!< Runs the loop until the table has rows, or until the wait is over.
    bool waitForRows(QTableView& table, int msec)
    {
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < msec)
        {
            QApplication::processEvents();
            if ((table.model() != nullptr) && (table.model()->rowCount() > 0))
            {
                return true;
            }
        }

        return false;
    }
}

int main(int argc, char* argv[])
{
    LusanTest::prepareUiEnvironment();
    QStandardPaths::setTestModeEnabled(true);
    LusanApplication app(argc, argv);

    MdiMainWindow window;
    window.resize(1600, 1000);
    window.show();
    QApplication::processEvents();

    QMdiArea* area{ window.findChild<QMdiArea*>() };
    check(area != nullptr, "the main window carries an editor area");

    std::printf("[document windows] the page tabs stay in sight while the editor area shrinks\n");
    if (area != nullptr)
    {
        // One document of each kind, with every page built: the design page of a state machine
        // is the tallest thing any of them holds.
        QMetaObject::invokeMethod(&window, "onFileNewSI");
        QMetaObject::invokeMethod(&window, "onFileNewFSM");
        QMetaObject::invokeMethod(&window, "onFileNewDT");
        QApplication::processEvents();

        const QList<QMdiSubWindow*> windows{ area->subWindowList() };
        check(windows.size() == 3, "three document windows are open");

        for (QMdiSubWindow* sub : windows)
        {
            area->setActiveSubWindow(sub);
            QApplication::processEvents();

            MdiChild* child{ qobject_cast<MdiChild*>(sub->widget()) };
            QTabWidget* tabs{ child != nullptr
                              ? child->findChild<QTabWidget*>(QString(), Qt::FindDirectChildrenOnly)
                              : nullptr };
            if (tabs != nullptr)
            {
                // Reach every page, so no page is left unbuilt and out of the measurement.
                for (int i = 0; i < tabs->count(); ++i)
                {
                    tabs->setCurrentIndex(i);
                    QApplication::processEvents();
                }

                tabs->setCurrentIndex(0);
                QApplication::processEvents();
            }
        }

        // The shortest editor area the Output dock and the window frame can leave behind.
        const int shortest{ 300 };
        for (QMdiSubWindow* sub : windows)
        {
            MdiChild* child{ qobject_cast<MdiChild*>(sub->widget()) };
            const int wanted{ child != nullptr ? child->minimumSizeHint().height() : -1 };
            std::printf("  %-18s asks for at least %d px\n", sub->widget()->metaObject()->className(), wanted);
            check((wanted > 0) && (wanted <= shortest), "the document window fits a short editor area");
        }

        area->setFixedHeight(shortest);
        QApplication::processEvents();
        for (QMdiSubWindow* sub : windows)
        {
            area->setActiveSubWindow(sub);
            QApplication::processEvents();
            check(tabsInSight(qobject_cast<MdiChild*>(sub->widget()), *area)
                 , "the page tabs are inside the editor area");
        }

        area->setMinimumHeight(0);
        area->setMaximumHeight(QWIDGETSIZE_MAX);
        QApplication::processEvents();
    }

    std::printf("[live logs] picking a row stops the table following the newest log\n");
    {
        const QString archive{ QDir(QDir::tempPath()).filePath(QStringLiteral("lusan-follow-test.sqlog")) };
        QFile::remove(archive);
        check(writeArchive(archive, 40), "the test archive is written");

        ArchiveLiveViewer live(&window);
        live.resize(1200, 600);
        QApplication::processEvents();

        LogSessionBar* bar{ live.findChild<LogSessionBar*>() };
        QTableView* table{ live.findChild<QTableView*>() };
        check(bar != nullptr, "the live window carries a session bar");
        check(table != nullptr, "the live window carries a table");

        if ((bar != nullptr) && (table != nullptr) && (live.logModel() != nullptr))
        {
            live.logModel()->openDatabase(archive, true);
            live.logModel()->readLogsAsynchronous();
            check(waitForRows(*table, 5000), "the window shows the archived rows");

            // The window opens holding the end of the log.
            check(bar->isFollowing(), "a new live window follows the newest log");

            table->selectRow(0);
            QApplication::processEvents();
            check(bar->isFollowing() == false, "a picked row releases the follow");

            QToolButton* toEnd{ bar->ctrlMoveBottom() };
            check(toEnd != nullptr, "the session bar carries the end button");
            if (toEnd != nullptr)
            {
                toEnd->click();
                QApplication::processEvents();
                check(bar->isFollowing(), "the end button takes the follow back");
            }
        }

        live.logModel()->closeDatabase();
        QFile::remove(archive);
    }

    std::printf("%d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
