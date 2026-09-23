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
 *                 2. Switching documents neither restores nor resizes any document window,
 *                    and the Properties panel of a Design page keeps the width dragged for it,
 *                    also after a narrow main window squeezed it.
 *                 3. Closing the active window activates the window active before it,
 *                    closing another window keeps the active one, also after a tab move.
 *                 4. The live log window stops following the newest log as soon as the
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
#include <QDockWidget>
#include <QElapsedTimer>
#include <QHash>
#include <QItemSelectionModel>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMouseEvent>
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

    //!< Runs the event loop for the given time, so queued layout work lands.
    void settle(int msec)
    {
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < msec)
        {
            QApplication::processEvents();
        }
    }

    //!< Drags the separator on the left edge of a dock by the given distance, as a mouse does.
    void dragDockEdge(QMainWindow& page, QDockWidget& dock, int distance)
    {
        const QPointF from(dock.x() - 1, dock.y() + 40);
        const QPointF to(from.x() + distance, from.y());
        QMouseEvent press(QEvent::Type::MouseButtonPress, from, page.mapToGlobal(from), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&page, &press);
        QMouseEvent move(QEvent::Type::MouseMove, to, page.mapToGlobal(to), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&page, &move);
        // The page moves the separator on its next event loop turn, not on the move itself.
        settle(100);
        QMouseEvent release(QEvent::Type::MouseButtonRelease, to, page.mapToGlobal(to), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(&page, &release);
        settle(100);
    }

    //!< Closes a document window without the question about unsaved changes.
    void closeWindow(QMdiSubWindow* sub)
    {
        MdiChild* child{ qobject_cast<MdiChild*>(sub->widget()) };
        if (child != nullptr)
        {
            child->setModified(false);
        }

        sub->close();
        QApplication::processEvents();
    }

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

    std::printf("[document windows] switching documents leaves every window and the Properties panel as they were\n");
    if (area != nullptr)
    {
        const QList<QMdiSubWindow*> windows{ area->subWindowList() };
        QMdiSubWindow* fsm{ nullptr };
        for (QMdiSubWindow* sub : windows)
        {
            if (sub->widget()->inherits("StateMachine"))
            {
                fsm = sub;
            }
        }

        check(fsm != nullptr, "a state machine window is open");
        if (fsm != nullptr)
        {
            area->setActiveSubWindow(fsm);
            QTabWidget* tabs{ fsm->widget()->findChild<QTabWidget*>(QString(), Qt::FindDirectChildrenOnly) };
            if (tabs != nullptr)
            {
                tabs->setCurrentIndex(tabs->count() - 1);
            }

            QDockWidget* dock{ fsm->findChild<QDockWidget*>(QStringLiteral("SMPropertiesDock")) };
            QMainWindow* page{ dock != nullptr ? qobject_cast<QMainWindow*>(dock->parentWidget()) : nullptr };
            settle(500);
            const bool docked{ (page != nullptr) && dock->isVisible() };
            if (docked)
            {
                const int before{ dock->width() };
                dragDockEdge(*page, *dock, -80);
                check(dock->width() == (before + 80), "dragging the separator widens the Properties panel");
            }
            else
            {
                std::printf("  the Properties panel is placed outside the Design page, its width is not checked\n");
            }

            QHash<QMdiSubWindow*, QSize> sizes;
            for (QMdiSubWindow* sub : windows)
            {
                sizes.insert(sub, sub->size());
            }

            const int panelWidth{ docked ? dock->width() : -1 };
            for (QMdiSubWindow* sub : windows)
            {
                if (sub == fsm)
                    continue;

                area->setActiveSubWindow(sub);
                settle(100);
                bool unchanged{ true };
                for (QMdiSubWindow* other : windows)
                {
                    unchanged = unchanged && other->isMaximized() && (other->size() == sizes.value(other));
                }

                check(unchanged, "activating a document neither restores nor resizes another window");

                area->setActiveSubWindow(fsm);
                settle(100);
                check((docked == false) || (dock->width() == panelWidth), "the Properties panel keeps the width it was given");
            }

            if (docked)
            {
                const QSize full{ window.size() };
                window.resize(700, full.height());
                settle(200);
                check(dock->width() < panelWidth, "a narrow main window squeezes the Properties panel");

                window.resize(full);
                settle(200);
                check(dock->width() == panelWidth, "the Properties panel gets its width back when the window grows");
            }
        }
    }

    std::printf("[document windows] closing a window returns to the one active before it\n");
    if (area != nullptr)
    {
        QMetaObject::invokeMethod(&window, "onFileNewSI");
        QMetaObject::invokeMethod(&window, "onFileNewSI");
        QApplication::processEvents();

        // Creation order is SI, FSM, DT, SI, SI.
        const QList<QMdiSubWindow*> windows{ area->subWindowList() };
        check(windows.size() == 5, "five document windows are open");
        if (windows.size() == 5)
        {
            area->setActiveSubWindow(windows.at(0));
            area->setActiveSubWindow(windows.at(2));
            closeWindow(windows.at(2));
            check(area->activeSubWindow() == windows.at(0), "closing the active window activates the one active before it");

            closeWindow(windows.at(3));
            check(area->activeSubWindow() == windows.at(0), "closing an inactive window keeps the active one");

            // Open: SI0, FSM1, SI4. A tab drag moves the active window to the end.
            QTabBar* bar{ area->findChild<QTabBar*>(QString(), Qt::FindDirectChildrenOnly) };
            check(bar != nullptr, "the editor area shows its windows as tabs");
            if (bar != nullptr)
            {
                area->setActiveSubWindow(windows.at(4));
                area->setActiveSubWindow(windows.at(0));
                bar->moveTab(0, 2);
                QApplication::processEvents();
                closeWindow(windows.at(1));
                check(area->activeSubWindow() == windows.at(0), "closing an inactive window after a tab move keeps the active one");

                closeWindow(windows.at(0));
                check(area->activeSubWindow() == windows.at(4), "the order of activation survives a tab move");
            }
        }
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
