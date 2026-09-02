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
 *  \file        tests/log/LogWindowShots.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Builds the assembled log windows -- the real main window, the live viewer
 *               and the offline viewer -- checks the parts that must be there, and saves a
 *               picture of each one. It runs against the application's own widgets, so the
 *               pictures show what a reader sees.
 *
 *  Usage: lusan_log_window_shots [output directory]
 *
 *  \note    The run keeps to a test configuration path, so the reader's own settings
 *           file is never opened and never written.
 *
 ************************************************************************/

#include "tests/common/UiTestEnv.hpp"

#include "areg/logging/LoggingDefs.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/app/NEAppThemes.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/NaviFileSystem.hpp"
#include "lusan/view/common/NaviLiveLogsScopes.hpp"
#include "lusan/view/common/NaviOfflineLogsScopes.hpp"
#include "lusan/view/common/NaviToolbarWindow.hpp"
#include "lusan/view/common/SearchLineEdit.hpp"
#include "lusan/view/log/LiveLogViewer.hpp"
#include "lusan/view/log/LogEmptyState.hpp"
#include "lusan/view/log/LogFilterChips.hpp"
#include "lusan/view/log/LogFilterWidgets.hpp"
#include "lusan/view/log/LogHeaderItem.hpp"
#include "lusan/view/log/LogPriorityBar.hpp"
#include "lusan/view/log/LogSessionBar.hpp"
#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/view/log/LogTextHighlight.hpp"
#include "lusan/data/common/OptionsManager.hpp"
#include "lusan/view/log/LogViewPanels.hpp"
#include "lusan/view/log/LogViewerBase.hpp"
#include "lusan/view/log/OfflineLogViewer.hpp"
#include "lusan/view/log/ScopeOutputViewer.hpp"

#include <QDir>
#include <QHeaderView>
#include <QMouseEvent>
#include <QLabel>
#include <QListWidget>
#include <QFontMetrics>
#include <QRadioButton>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTableView>
#include <QHash>
#include <QLayout>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>
#include <QMenu>
#include <QTimer>

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

    QString gShotDir;

    void shoot(QWidget& widget, const char* name)
    {
        if (gShotDir.isEmpty() == false)
        {
            widget.grab().save(gShotDir + QDir::separator() + QString::fromLatin1(name) + QStringLiteral(".png"));
        }
    }

    //!< Saves the top strip of a window, the band the session bar and the table header take.
    void shootTop(QWidget& widget, const char* name)
    {
        if (gShotDir.isEmpty() == false)
        {
            widget.grab(QRect(0, 0, widget.width(), 72))
                  .save(gShotDir + QDir::separator() + QString::fromLatin1(name) + QStringLiteral(".png"));
        }
    }

    //!< Presses the left mouse button on the header at the given viewport point.
    //!< \note   Only the press is sent. A press and a release with no move between them puts
    //!<         a movable QHeaderView through a section move it never started, and the debug
    //!<         build of Qt asserts on it.
    void pressHeader(LogTableHeader& header, const QPoint& where)
    {
        const QPointF local{ where };
        QMouseEvent press( QEvent::Type::MouseButtonPress, local, header.viewport()->mapToGlobal(local)
                         , Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
        QApplication::sendEvent(header.viewport(), &press);
        QApplication::processEvents();
    }

    //!< Finds the row of the given column in a list that carries one row per column.
    const QListWidgetItem* findColumnRow(const QListWidget& list, LoggingModelBase::eColumn column)
    {
        for (int i = 0; i < list.count(); ++i)
        {
            const QListWidgetItem* item{ list.item(i) };
            if (item->text() == LoggingModelBase::getHeaderList().at(static_cast<int>(column)))
                return item;
        }

        return nullptr;
    }

    //!< Builds one log entry in the layout the archive reader produces.
    areg::SharedBuffer makeEntry(areg::LogPriority prio, TIME64 timestamp, const char* message
                                , uint32_t scopeId = 7u, uint32_t sessionId = 0u)
    {
        areg::LogEntry entry{ };
        entry.logMsgType     = areg::LogMessageType::MessageText;
        entry.logMessagePrio = prio;
        entry.logDataType    = areg::LogDataType::Remote;
        entry.logCookie      = 1u;
        entry.logModuleId    = 1u;
        entry.logThreadId    = 42u;
        entry.logScopeId     = scopeId;
        entry.logSessionId   = sessionId;
        entry.logTimestamp   = timestamp;
        entry.logReceived    = timestamp;

        const uint32_t textLen{ static_cast<uint32_t>(message != nullptr ? std::strlen(message) : 0u) };
        const uint32_t copyLen{ textLen < (areg::LOG_MSG_SIZE - 1u) ? textLen : (areg::LOG_MSG_SIZE - 1u) };
        if (copyLen != 0u)
        {
            std::memcpy(entry.logMessage, message, copyLen);
        }

        entry.logMessage[copyLen] = '\0';
        entry.logMessageLen = textLen;
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

    //!< A log model filled by hand instead of by a collector or an archive.
    class ShotLogModel : public LoggingModelBase
    {
    public:
        ShotLogModel(void)
            : LoggingModelBase(LoggingModelBase::eLogging::LoggingOffline, nullptr)
        {
        }

        void addEntries(std::vector<areg::SharedBuffer>&& entries)
        {
            appendLogBatch(std::move(entries), mLoadGeneration);
        }
    };

    //!< The words the empty panel of a window is showing, joined into one string.
    QString emptyStateText(const QWidget& window)
    {
        const LogEmptyState* empty{ window.findChild<LogEmptyState*>() };
        if (empty == nullptr)
            return QString();

        QString text;
        for (const QLabel* label : empty->findChildren<QLabel*>())
        {
            if (label->text().isEmpty() == false)
            {
                text += label->text() + QLatin1Char('\n');
            }
        }

        return text;
    }
    //!< The control of @p panel whose accessible name is @p name, or nullptr.
    QWidget* toolNamed(const QWidget& panel, const QString& name)
    {
        for (QWidget* child : panel.findChildren<QWidget*>())
        {
            if (child->accessibleName() == name)
            {
                return child;
            }
        }

        return nullptr;
    }

    //!< Gives a panel the width asked for and lays it out. A window that was never shown
    //!< leaves its controls at their default size, and every measure taken from one is
    //!< the same meaningless number.
    void layoutAt(QWidget& panel, int width)
    {
        panel.setFixedWidth(width);
        panel.ensurePolished();
        QApplication::processEvents();
        // Rendering the panel is what lays it out. A window that was never shown does not do
        // it on its own, and activating the layout alone does not reach the nested rows.
        (void)panel.grab();
    }

    //!< The button that drops the chip standing at position @p at in the row, or nullptr.
    QToolButton* chipDropButton(LogFilterChips& row, int at)
    {
        QLayout* layout{ row.layout() };
        int seen{ 0 };
        for (int i = 0; (layout != nullptr) && (i < layout->count()); ++i)
        {
            QWidget* widget{ layout->itemAt(i)->widget() };
            if ((widget != nullptr) && (widget->objectName() == QLatin1String("logFilterChip")))
            {
                if (seen == at)
                    return widget->findChild<QToolButton*>();

                ++seen;
            }
        }

        return nullptr;
    }

    //!< True when the given column is among the filters the proxy has on.
    bool filtersColumn(const LogViewerFilter& proxy, LoggingModelBase::eColumn column)
    {
        const LogViewerFilter::ListActiveFilters active{ proxy.activeFilters() };
        for (const LogViewerFilter::sActiveFilter& entry : active)
        {
            if (entry.column == static_cast<int>(column))
                return true;
        }

        return false;
    }

    //!< Where a control starts, measured from the left edge of the panel holding it.
    int toolLeft(const QWidget& panel, const QString& name)
    {
        const QWidget* tool{ toolNamed(panel, name) };
        return (tool != nullptr) ? tool->mapTo(&panel, QPoint(0, 0)).x() : -1;
    }
}

#define CHECK(cond)  check((cond), #cond)

int main(int argc, char* argv[])
{
    LusanTest::prepareUiEnvironment();

    // Keep every settings path inside the test tree: the run must not read or rewrite
    // the reader's own options file.
    QStandardPaths::setTestModeEnabled(true);

    LusanApplication app(argc, argv);

    if (argc > 1)
    {
        gShotDir = QString::fromLocal8Bit(argv[1]);
        QDir().mkpath(gShotDir);
    }

    MdiMainWindow window;
    window.resize(1600, 1000);
    QApplication::processEvents();

    std::printf("[main window] the frame the log windows live in\n");
    {
        CHECK(window.centralWidget() != nullptr);
        CHECK(window.menuBar() != nullptr);
        shoot(window, "main-window");
    }

    std::printf("[navigation] the file system panel with no active workspace\n");
    {
        // A run can reach the panel before a workspace is active. It must show an
        // empty tree, not end the process.
        NaviFileSystem& files{ window.getNaviFileSystem() };
        QApplication::processEvents();
        CHECK(files.findChild<QTreeView*>() != nullptr);
        shoot(files, "navigation-no-workspace");
    }

    std::printf("[offline] the archive window, with no archive open\n");
    {
        OfflineLogViewer offline(&window);
        offline.resize(1400, 820);
        QApplication::processEvents();

        // The offline window used to carry no controls at all. It has its own bar now.
        LogSessionBar* bar{ offline.findChild<LogSessionBar*>() };
        CHECK(bar != nullptr);

        // The table is there, with headers, before any archive is read.
        QTableView* table{ offline.findChild<QTableView*>() };
        CHECK(table != nullptr);
        if (table != nullptr)
        {
            CHECK(table->model() != nullptr);
            CHECK(table->horizontalHeader() != nullptr);
            CHECK(table->horizontalHeader()->count() > 0);
        }

        // The window says what is missing instead of showing an empty grid.
        const QString words{ emptyStateText(offline) };
        CHECK(words.isEmpty() == false);
        CHECK(words.contains(QStringLiteral("archive"), Qt::CaseInsensitive));

        shoot(offline, "offline-empty");
    }

    std::printf("[scope toolbars] the two explorers carry their shared tools on one line\n");
    {
        // The window owns both explorers, and the live one keeps a pointer to itself, so a
        // second instance must not be built here.
        NaviLiveLogsScopes&    live   { window.getNaviLiveScopes() };
        NaviOfflineLogsScopes& offline{ window.getNaviOfflineScopes() };

        layoutAt(live, 600);
        layoutAt(offline, 600);

        // Everything the two explorers have in common, in the order the row carries it.
        const QStringList shared{ QStringLiteral("Scope priority")
                                , QStringLiteral("Find a scope")
                                , QStringLiteral("Show only the selected scopes")
                                , QStringLiteral("Hide the selected scopes")
                                , QStringLiteral("Show all scopes")
                                , QStringLiteral("Collapse or expand log scopes") };

        bool everyToolPresent{ true };
        bool everyToolAligned{ true };
        for (const QString& name : shared)
        {
            const int liveLeft   { toolLeft(live, name) };
            const int offlineLeft{ toolLeft(offline, name) };
            everyToolPresent = everyToolPresent && (liveLeft >= 0) && (offlineLeft >= 0);
            everyToolAligned = everyToolAligned && (liveLeft == offlineLeft);
            if (liveLeft != offlineLeft)
            {
                std::printf("  %-34s live x=%d, offline x=%d\n", qPrintable(name), liveLeft, offlineLeft);
            }
        }

        CHECK(everyToolPresent);
        CHECK(everyToolAligned);

        // The button that opens the log source heads both rows.
        CHECK(toolLeft(live, QStringLiteral("Connect to log collector")) == toolLeft(offline, QStringLiteral("Open log file")));

        shoot(live, "scopes-live-toolbar");
        shoot(offline, "scopes-offline-toolbar");
    }

    std::printf("[scope toolbars] the row under the chevron keeps the size of the row itself\n");
    {
        NaviLiveLogsScopes& live{ window.getNaviLiveScopes() };
        layoutAt(live, 600);

        // What every tool measures while the row still holds it.
        QHash<QString, QSize> inRow;
        for (QToolButton* button : live.findChildren<QToolButton*>())
        {
            if (button->accessibleName().isEmpty() == false)
            {
                inRow.insert(button->accessibleName(), button->size());
            }
        }

        CHECK(inRow.isEmpty() == false);

        // A tool of the row is as tall as the row less the air it keeps. Reading the
        // default size of a control instead means the panel was never laid out, and every
        // measure taken after it would be the same meaningless number.
        const QSize findButton{ inRow.value(QStringLiteral("Find a scope")) };
        std::printf("  a tool of the row is %dx%d, the row is %d px\n"
                    , findButton.width(), findButton.height(), NaviToolbarWindow::toolRowHeight());
        CHECK(findButton.height() == (NaviToolbarWindow::toolRowHeight() - (NaviToolbarWindow::NAVI_TOOL_AIR * 2)));

        // Narrow enough that the row has to give entries up.
        layoutAt(live, 220);

        QToolButton* chevron{ qobject_cast<QToolButton*>(toolNamed(live, QStringLiteral("More tools"))) };
        CHECK(chevron != nullptr);
        if (chevron != nullptr)
        {
            // The window this runs in was never shown, so every entry counts as not visible
            // and the second row receives them all. That is what the sizes are read from.
            chevron->click();
            QApplication::processEvents();

            QWidget* popup{ live.findChild<QWidget*>(QStringLiteral("naviToolOverflow")) };
            CHECK(popup != nullptr);
            if (popup != nullptr)
            {
                bool everyButtonKeptItsSize{ true };
                int moved{ 0 };
                for (QToolButton* button : popup->findChildren<QToolButton*>())
                {
                    ++moved;
                    const QSize was{ inRow.value(button->accessibleName()) };
                    if (was.isValid() && (button->size() != was))
                    {
                        everyButtonKeptItsSize = false;
                        std::printf("  %-34s was %dx%d in the row, %dx%d under the chevron\n"
                                    , qPrintable(button->accessibleName())
                                    , was.width(), was.height(), button->width(), button->height());
                    }
                }

                std::printf("  %d tool(s) moved, the row under the chevron is %d px, the row is %d px\n"
                            , moved, popup->height(), NaviToolbarWindow::toolRowHeight());
                CHECK(moved > 0);
                CHECK(everyButtonKeptItsSize);

                // The second row is the tool row plus the frame it draws around itself.
                CHECK(popup->height() >= NaviToolbarWindow::toolRowHeight());
                CHECK(popup->height() <= NaviToolbarWindow::toolRowHeight() + 6);
                shoot(*popup, "scopes-toolbar-overflow");
                popup->hide();
                QApplication::processEvents();
            }
        }
    }

    std::printf("[live] the collector window, with nothing connected\n");
    {
        LiveLogViewer live(&window);
        live.resize(1400, 820);
        QApplication::processEvents();

        LogSessionBar* bar{ live.findChild<LogSessionBar*>() };
        CHECK(bar != nullptr);

        QTableView* table{ live.findChild<QTableView*>() };
        CHECK(table != nullptr);

        // The view ladder belongs to the log window, and it is the view copy, not the
        // one that sets what a target produces.
        LogPriorityBar* ladder{ live.findChild<LogPriorityBar*>() };
        CHECK(ladder != nullptr);
        if (ladder != nullptr)
        {
            CHECK(ladder->role() == LogPriorityBar::eBarRole::RoleView);
        }

        const QString words{ emptyStateText(live) };
        CHECK(words.isEmpty() == false);
        CHECK(words.contains(QStringLiteral("collector"), Qt::CaseInsensitive));

        shoot(live, "live-empty");
    }

    std::printf("[filters] dropping a chip switches off the header control it stands for\n");
    {
        LiveLogViewer live(&window);
        live.resize(1400, 820);
        QApplication::processEvents();

        LogTableHeader*  header{ live.findChild<LogTableHeader*>() };
        LogFilterChips*  row   { live.findChild<LogFilterChips*>() };
        QTableView*      table { live.findChild<QTableView*>() };
        LogViewerFilter* proxy { table != nullptr ? qobject_cast<LogViewerFilter*>(table->model()) : nullptr };

        CHECK(header != nullptr);
        CHECK(row    != nullptr);
        CHECK(proxy  != nullptr);

        if ((header != nullptr) && (row != nullptr) && (proxy != nullptr))
        {
            // A phrase in the Message filter and a priority in the Priority filter, both set
            // through the controls the reader uses.
            LogHeaderItem* message{ header->getHeaderItem(LoggingModelBase::eColumn::LogColumnMessage) };
            CHECK(message != nullptr);
            if (message != nullptr)
            {
                message->setFilterData(NELusanCommon::FilterString{ QStringLiteral("timeout"), false, false, false });
            }

            LogPrioComboFilter* prio{ header->findChild<LogPrioComboFilter*>() };
            QListWidget*        list{ prio != nullptr ? prio->findChild<QListWidget*>() : nullptr };
            CHECK(prio != nullptr);
            if (prio != nullptr)
            {
                prio->setDataItems(QStringList{ QStringLiteral("Debug"), QStringLiteral("Error") }
                                  , NELusanCommon::AnyList{ std::make_any<uint16_t>(static_cast<uint16_t>(areg::LogPriority::PrioDebug))
                                                          , std::make_any<uint16_t>(static_cast<uint16_t>(areg::LogPriority::PrioError)) });
                list = prio->findChild<QListWidget*>();
                CHECK(list != nullptr);
                if (list != nullptr)
                {
                    CHECK(list->count() == 2);
                    list->item(1)->setCheckState(Qt::CheckState::Checked);
                }
            }

            QApplication::processEvents();
            shootTop(live, "header-filtered");
            if ((gShotDir.isEmpty() == false) && (header != nullptr))
            {
                const QPixmap band{ header->grab() };
                band.scaled(band.width() * 3, band.height() * 3, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::FastTransformation)
                    .save(gShotDir + QDir::separator() + QStringLiteral("header-zoom.png"));
            }

            CHECK(filtersColumn(*proxy, LoggingModelBase::eColumn::LogColumnMessage));
            CHECK(filtersColumn(*proxy, LoggingModelBase::eColumn::LogColumnPriority));
            CHECK(row->chips().size() == 2);

            // The chip that stands for the Message filter names the column it acts on.
            int atMessage{ -1 };
            for (int i = 0; i < row->chips().size(); ++i)
            {
                if (row->chips().at(i).column == static_cast<int>(LoggingModelBase::eColumn::LogColumnMessage))
                {
                    atMessage = i;
                }
            }

            CHECK(atMessage >= 0);
            if (atMessage >= 0)
            {
                CHECK(row->chips().at(atMessage).label.startsWith(QStringLiteral("Message: ")));
            }

            // Dropping that chip clears the Message filter and the header control behind it,
            // and leaves the Priority filter exactly where it was.
            QToolButton* drop{ atMessage >= 0 ? chipDropButton(*row, atMessage) : nullptr };
            CHECK(drop != nullptr);
            if (drop != nullptr)
            {
                drop->click();
                QApplication::processEvents();
            }

            CHECK(filtersColumn(*proxy, LoggingModelBase::eColumn::LogColumnMessage) == false);
            CHECK(filtersColumn(*proxy, LoggingModelBase::eColumn::LogColumnPriority));
            CHECK((message == nullptr) || message->getFilterData().isEmpty());
            CHECK(row->chips().size() == 1);

            // The same for the priority chip, which is set in a list and not in a text box.
            QToolButton* dropPrio{ chipDropButton(*row, 0) };
            CHECK(dropPrio != nullptr);
            if (dropPrio != nullptr)
            {
                dropPrio->click();
                QApplication::processEvents();
            }

            CHECK(proxy->hasColumnFilters() == false);
            CHECK(row->chips().isEmpty());
            CHECK((list == nullptr) || (list->item(1)->checkState() == Qt::CheckState::Unchecked));
        }
    }

    std::printf("[filters] a column opens the same panel after its filter was dropped\n");
    {
        LiveLogViewer live(&window);
        live.resize(1400, 820);
        QApplication::processEvents();

        LogTableHeader* header{ live.findChild<LogTableHeader*>() };
        CHECK(header != nullptr);
        if (header != nullptr)
        {
            LogHeaderItem* message{ header->getHeaderItem(LoggingModelBase::eColumn::LogColumnMessage) };
            LogHeaderItem* prio   { header->getHeaderItem(LoggingModelBase::eColumn::LogColumnPriority) };
            LogMessageEditFilter* box { header->findChild<LogMessageEditFilter*>() };
            LogPrioComboFilter*   plate{ header->findChild<LogPrioComboFilter*>() };
            SearchLineEdit* phrase{ box   != nullptr ? box->findChild<SearchLineEdit*>() : nullptr };
            QListWidget*    values{ plate != nullptr ? plate->findChild<QListWidget*>()  : nullptr };

            CHECK(message != nullptr);
            CHECK(prio    != nullptr);
            CHECK(phrase  != nullptr);
            CHECK(values  != nullptr);

            // Opening a panel, setting a filter and dropping it again must leave the panel
            // exactly as it was. Hiding the control inside the panel instead of the panel
            // itself left an empty square behind for every following open.
            if ((message != nullptr) && (box != nullptr) && (phrase != nullptr))
            {
                message->showFilters();
                QApplication::processEvents();
                shoot(*box, "filter-panel-phrase");
                const QSize opened{ box->size() };
                CHECK(phrase->isHidden() == false);
                CHECK(opened.height() > 8);

                message->setFilterData(NELusanCommon::FilterString{ QStringLiteral("timeout"), false, false, false });
                message->resetFilter();
                QApplication::processEvents();

                message->showFilters();
                QApplication::processEvents();
                CHECK(phrase->isHidden() == false);
                CHECK(box->size() == opened);
                box->hide();
            }

            if ((prio != nullptr) && (plate != nullptr) && (values != nullptr))
            {
                plate->setDataItems(QStringList{ QStringLiteral("Fatal"), QStringLiteral("Error"), QStringLiteral("Warning")
                                               , QStringLiteral("Information"), QStringLiteral("Debug") }
                                   , NELusanCommon::AnyList{ std::make_any<uint16_t>(static_cast<uint16_t>(areg::LogPriority::PrioFatal))
                                                           , std::make_any<uint16_t>(static_cast<uint16_t>(areg::LogPriority::PrioError))
                                                           , std::make_any<uint16_t>(static_cast<uint16_t>(areg::LogPriority::PrioWarning))
                                                           , std::make_any<uint16_t>(static_cast<uint16_t>(areg::LogPriority::PrioInfo))
                                                           , std::make_any<uint16_t>(static_cast<uint16_t>(areg::LogPriority::PrioDebug)) });
                values = plate->findChild<QListWidget*>();
                CHECK(values != nullptr);
                if (values != nullptr)
                {
                    values->item(1)->setCheckState(Qt::CheckState::Checked);
                }

                prio->showFilters();
                QApplication::processEvents();
                shoot(*plate, "filter-panel-values");
                const QSize opened{ plate->size() };
                CHECK(values->isHidden() == false);
                CHECK(opened.height() > 8);

                prio->resetFilter();
                QApplication::processEvents();

                prio->showFilters();
                QApplication::processEvents();
                CHECK(values->isHidden() == false);
                CHECK(plate->size() == opened);
                plate->hide();
            }
        }
    }

    std::printf("[search] the search box and the Message filter are one control doing two jobs\n");
    {
        LiveLogViewer live(&window);
        live.resize(1400, 820);
        QApplication::processEvents();

        LogTableHeader*  header{ live.findChild<LogTableHeader*>() };
        LogSessionBar*   bar   { live.findChild<LogSessionBar*>() };
        LogFilterChips*  row   { live.findChild<LogFilterChips*>() };
        QTableView*      table { live.findChild<QTableView*>() };
        LogViewerFilter* proxy { table != nullptr ? qobject_cast<LogViewerFilter*>(table->model()) : nullptr };

        CHECK(header != nullptr);
        CHECK(bar    != nullptr);
        CHECK(proxy  != nullptr);

        LogMessageEditFilter* filter{ header != nullptr ? header->findChild<LogMessageEditFilter*>() : nullptr };
        SearchLineEdit* inFilter{ filter != nullptr ? filter->findChild<SearchLineEdit*>() : nullptr };
        SearchLineEdit* inBar   { bar    != nullptr ? bar->ctrlSearch() : nullptr };

        CHECK(inFilter != nullptr);
        CHECK(inBar    != nullptr);
        CHECK(inFilter != inBar);

        // Both carry the same match options, and only the search walks in a direction.
        if ((inFilter != nullptr) && (inBar != nullptr))
        {
            CHECK(inFilter->buttonMatchCase() != nullptr);
            CHECK(inFilter->buttonMatchWord() != nullptr);
            CHECK(inFilter->buttonWildCard()  != nullptr);
            CHECK(inBar->buttonMatchCase()    != nullptr);
            CHECK(inBar->buttonSearchBackward()     != nullptr);
            CHECK(inFilter->buttonSearchBackward()  == nullptr);
        }

        // Typing in the search box removes no row and raises no chip.
        if ((inBar != nullptr) && (proxy != nullptr) && (row != nullptr))
        {
            inBar->setText(QStringLiteral("timeout"));
            QApplication::processEvents();
            CHECK(proxy->hasColumnFilters() == false);
            CHECK(row->chips().isEmpty());
        }

        // Handing the phrase over to the filter is what removes them, and it says so with a chip.
        if ((bar != nullptr) && (proxy != nullptr) && (row != nullptr))
        {
            CHECK(bar->ctrlFilterMatches()->isEnabled());
            bar->ctrlFilterMatches()->click();
            QApplication::processEvents();
            CHECK(filtersColumn(*proxy, LoggingModelBase::eColumn::LogColumnMessage));
            CHECK(row->chips().size() == 1);
            CHECK((inFilter == nullptr) || (inFilter->text() == QStringLiteral("timeout")));

            // The search box keeps the phrase: the filter took it over, it did not move out.
            CHECK((inBar == nullptr) || (inBar->text() == QStringLiteral("timeout")));
        }
    }

    std::printf("[header] the funnel of a column stands beside its title, not at the far end\n");
    {
        LiveLogViewer live(&window);
        live.resize(1400, 820);
        QApplication::processEvents();

        LogTableHeader* header{ live.findChild<LogTableHeader*>() };
        CHECK(header != nullptr);

        const int logical{ header != nullptr ? header->getColumnIndex(LoggingModelBase::eColumn::LogColumnPriority) : -1 };
        CHECK(logical >= 0);

        LogPrioComboFilter* panel{ header != nullptr ? header->findChild<LogPrioComboFilter*>() : nullptr };
        CHECK(panel != nullptr);

        if ((header != nullptr) && (panel != nullptr) && (logical >= 0))
        {
            const int left { header->sectionViewportPosition(logical) };
            const int size { header->sectionSize(logical) };
            const int middle{ header->height() / 2 };

            // The pointer lands where the funnel now is, and the panel of the column opens.
            pressHeader(*header, QPoint(left + 10, middle));
            CHECK(panel->isVisible());
            panel->hide();
            QApplication::processEvents();

            // The far end of the section is where the funnel used to be, clear of the grip
            // that resizes a column. Nothing opens there any more.
            pressHeader(*header, QPoint(left + size - 14, middle));
            CHECK(panel->isVisible() == false);
        }

        // Every column that can be narrowed says so at rest, and the two time columns
        // are told apart by name at any width.
        if (header != nullptr)
        {
            CHECK(header->canFilter(LoggingModelBase::eColumn::LogColumnPriority));
            CHECK(header->canFilter(LoggingModelBase::eColumn::LogColumnMessage));
            CHECK(header->canFilter(LoggingModelBase::eColumn::LogColumnTimestamp) == false);
        }
    }

    std::printf("[by value] a row narrows its column to what it carries, or drops that value\n");
    {
        LiveLogViewer live(&window);
        live.resize(1400, 820);
        QApplication::processEvents();

        LogTableHeader* header{ live.findChild<LogTableHeader*>() };
        CHECK(header != nullptr);

        areg::LogEntry entry{ };
        entry.logMessagePrio = areg::LogPriority::PrioError;
        entry.logCookie      = 262u;
        entry.logThreadId    = 4180u;

        // Each column knows which field of a row it is narrowed by.
        if (header != nullptr)
        {
            LogHeaderItem* prio  { header->getHeaderItem(LoggingModelBase::eColumn::LogColumnPriority) };
            LogHeaderItem* source{ header->getHeaderItem(LoggingModelBase::eColumn::LogColumnSource)   };
            LogHeaderItem* thread{ header->getHeaderItem(LoggingModelBase::eColumn::LogColumnThread)   };
            LogHeaderItem* stamp { header->getHeaderItem(LoggingModelBase::eColumn::LogColumnTimestamp)};

            CHECK((prio   != nullptr) && (std::any_cast<uint16_t>(prio->valueOf(entry))  == static_cast<uint16_t>(areg::LogPriority::PrioError)));
            CHECK((source != nullptr) && (std::any_cast<ITEM_ID>(source->valueOf(entry)) == static_cast<ITEM_ID>(262u)));
            CHECK((thread != nullptr) && (std::any_cast<ITEM_ID>(thread->valueOf(entry)) == static_cast<ITEM_ID>(4180u)));

            // A column with no filter panel carries no value to narrow by.
            CHECK((stamp != nullptr) && (stamp->valueOf(entry).has_value() == false));
        }

        LogPrioComboFilter* panel{ header != nullptr ? header->findChild<LogPrioComboFilter*>() : nullptr };
        CHECK(panel != nullptr);

        if (panel != nullptr)
        {
            const uint16_t debug{ static_cast<uint16_t>(areg::LogPriority::PrioDebug) };
            const uint16_t info { static_cast<uint16_t>(areg::LogPriority::PrioInfo)  };
            const uint16_t error{ static_cast<uint16_t>(areg::LogPriority::PrioError) };

            panel->setDataItems( QStringList{ QStringLiteral("Debug"), QStringLiteral("Info"), QStringLiteral("Error") }
                               , NELusanCommon::AnyList{ std::make_any<uint16_t>(debug)
                                                       , std::make_any<uint16_t>(info)
                                                       , std::make_any<uint16_t>(error) });

            // Keeping one value picks it and nothing else.
            CHECK(panel->pickValue(std::make_any<uint16_t>(error), false));
            QList<NELusanCommon::FilterData> picked{ panel->getSelectedData() };
            CHECK(picked.size() == 1);
            CHECK((picked.isEmpty() == false) && (std::any_cast<uint16_t>(picked[0].data) == error));

            // Dropping one value keeps every other, which is what the reader asked for.
            CHECK(panel->pickValue(std::make_any<uint16_t>(error), true));
            picked = panel->getSelectedData();
            CHECK(picked.size() == 1);
            CHECK((picked.isEmpty() == false) && (std::any_cast<uint16_t>(picked[0].data) == static_cast<uint16_t>(debug | info)));

            // A value the column never saw changes nothing.
            CHECK(panel->pickValue(std::make_any<uint16_t>(static_cast<uint16_t>(areg::LogPriority::PrioFatal)), false) == false);
            CHECK(panel->pickValue(NELusanCommon::AnyData(), false) == false);
        }
    }

    std::printf("[columns] the panel that chooses the columns keeps them in one place\n");
    {
        LogColumnPicker picker;
        const LogColumnPicker::ListColumns active{ LoggingModelBase::getDefaultColumns() };
        picker.setColumns(active);
        picker.resize(300, picker.sizeHint().height());
        QApplication::processEvents();

        QListWidget* list{ picker.findChild<QListWidget*>() };
        CHECK(list != nullptr);

        // The rail is placed by the table and is never offered, so it is not counted here.
        const int offered{ static_cast<int>(LoggingModelBase::eColumn::LogColumnCount) - 1 };
        const int shown{ static_cast<int>(active.size()) - 1 };

        if (list != nullptr)
        {
            // Every column is offered, the shown ones first and in their own order.
            CHECK(list->count() == offered);
            CHECK(findColumnRow(*list, LoggingModelBase::eColumn::LogColumnRail) == nullptr);
            for (int i = 0; i < shown; ++i)
            {
                CHECK(list->item(i)->checkState() == Qt::CheckState::Checked);
            }

            for (int i = shown; i < list->count(); ++i)
            {
                CHECK(list->item(i)->checkState() == Qt::CheckState::Unchecked);
            }

            // The message stays, and its row says so rather than being absent.
            const QListWidgetItem* message{ findColumnRow(*list, LoggingModelBase::eColumn::LogColumnMessage) };
            CHECK(message != nullptr);
            CHECK((message == nullptr) || (message->checkState() == Qt::CheckState::Checked));
            CHECK((message == nullptr) || ((message->flags() & Qt::ItemFlag::ItemIsUserCheckable) == 0));
        }

        shoot(picker, "columns-picker");

        // What the panel reports is what the table is asked to show.
        LogColumnPicker::ListColumns reported;
        int reports{ 0 };
        QObject::connect(&picker, &LogColumnPicker::signalColumnsChanged, &picker
                        , [&reported, &reports](const LogColumnPicker::ListColumns& columns) {
                            reported = columns;
                            ++reports;
                        });

        if (list != nullptr)
        {
            // Checking a column adds it, and the report waits for the event loop.
            QListWidgetItem* duration{ const_cast<QListWidgetItem *>(findColumnRow(*list, LoggingModelBase::eColumn::LogColumnTimeDuration)) };
            CHECK(duration != nullptr);
            if (duration != nullptr)
            {
                duration->setCheckState(Qt::CheckState::Checked);
            }

            QApplication::processEvents();
            CHECK(reports == 1);
            CHECK(reported.size() == (shown + 1));

            // The column lands where the table would put it, which is the same place the row
            // menu gives it: before the message, and after every column that comes first.
            const int placed{ static_cast<int>(reported.indexOf(LoggingModelBase::eColumn::LogColumnTimeDuration)) };
            const int message{ static_cast<int>(reported.indexOf(LoggingModelBase::eColumn::LogColumnMessage)) };
            CHECK((placed >= 0) && (message >= 0) && (placed < message));
            CHECK(list->row(duration) < list->row(findColumnRow(*list, LoggingModelBase::eColumn::LogColumnMessage)));

            // The row menu takes the same road, so the two ways of showing a column agree.
            LoggingModelBase::ListColumns byMenu{ active };
            byMenu.insert( LoggingModelBase::placeOfColumn(byMenu, LoggingModelBase::eColumn::LogColumnTimeDuration)
                         , LoggingModelBase::eColumn::LogColumnTimeDuration);
            CHECK(LoggingModelBase::shapeColumns(reported) == byMenu);

            // A moved row reaches the model in either of two shapes, and the panel has to
            // answer to both. First shape: the row is taken out and put back, which is what
            // QListWidget::dropEvent does for an internal move. Two signals, one report.
            reports = 0;
            const LoggingModelBase::eColumn first{ static_cast<LoggingModelBase::eColumn>(list->item(0)->data(Qt::ItemDataRole::UserRole + 1).toInt()) };
            QListWidgetItem* const moved{ list->takeItem(0) };
            list->insertItem(2, moved);
            QApplication::processEvents();
            CHECK(reports == 1);
            CHECK(reported.size() == (shown + 1));
            CHECK((reported.size() > 2) && (reported.at(2) == first));

            // Second shape: one row move. The list model carries it, so it has to be watched
            // as well, or a move made this way would never reach the table.
            reports = 0;
            CHECK(list->model()->moveRows(QModelIndex(), 0, 1, QModelIndex(), 3));
            QApplication::processEvents();
            CHECK(reports == 1);
            CHECK(reported.size() == (shown + 1));
        }
    }

    std::printf("[rail] the leading column carries the rail, and nothing moves it\n");
    {
        // The rail opens every set of columns, whatever it was built from.
        CHECK(LoggingModelBase::getDefaultColumns().first() == LoggingModelBase::eColumn::LogColumnRail);
        CHECK(LoggingModelBase::isPinnedColumn(LoggingModelBase::eColumn::LogColumnRail));
        CHECK(LoggingModelBase::isPinnedColumn(LoggingModelBase::eColumn::LogColumnMessage));

        const LoggingModelBase::ListColumns messy
        {
              LoggingModelBase::eColumn::LogColumnMessage
            , LoggingModelBase::eColumn::LogColumnThread
            , LoggingModelBase::eColumn::LogColumnRail
            , LoggingModelBase::eColumn::LogColumnThread
            , LoggingModelBase::eColumn::LogColumnPriority
        };

        const LoggingModelBase::ListColumns shaped{ LoggingModelBase::shapeColumns(messy) };
        CHECK(shaped.first() == LoggingModelBase::eColumn::LogColumnRail);
        CHECK(shaped.last() == LoggingModelBase::eColumn::LogColumnMessage);
        CHECK(shaped.count(LoggingModelBase::eColumn::LogColumnThread) == 1);
        CHECK(shaped.size() == 4);

        // A list with no data column at all is not a table, so the default comes back.
        CHECK(LoggingModelBase::shapeColumns(LoggingModelBase::ListColumns{}) == LoggingModelBase::getDefaultColumns());

        // The rail cannot be dropped, and the model refuses to place a second one.
        LoggingModelBase model(LoggingModelBase::eLogging::LoggingOffline);
        model.setActiveColumns(LoggingModelBase::getDefaultColumns());
        model.removeColumn(LoggingModelBase::eColumn::LogColumnRail);
        model.removeColumn(LoggingModelBase::eColumn::LogColumnMessage);
        CHECK(model.getActiveColumns() == LoggingModelBase::getDefaultColumns());
        CHECK(model.fromIndexToColumn(0) == LoggingModelBase::eColumn::LogColumnRail);

        // Showing a column from the model puts it in the same place the panel reported.
        model.addColumn(LoggingModelBase::eColumn::LogColumnTimeDuration);
        const int placed{ model.fromColumnToIndex(LoggingModelBase::eColumn::LogColumnTimeDuration) };
        CHECK(placed > 0);
        CHECK(placed < model.fromColumnToIndex(LoggingModelBase::eColumn::LogColumnMessage));
        CHECK(model.fromIndexToColumn(0) == LoggingModelBase::eColumn::LogColumnRail);

        // The rail carries no title, so nothing is drawn over the marks it holds.
        CHECK(LoggingModelBase::getHeaderList().at(static_cast<int>(LoggingModelBase::eColumn::LogColumnRail)).isEmpty());
    }

    std::printf("[table] the rail stays first, the table scrolls sideways, and rows can wrap\n");
    {
        LiveLogViewer live(&window);
        live.resize(1400, 820);
        QApplication::processEvents();

        LogTableHeader* header{ live.findChild<LogTableHeader*>() };
        QTableView* table{ live.findChild<QTableView*>() };
        CHECK(header != nullptr);
        CHECK(table != nullptr);

        if ((header != nullptr) && (table != nullptr))
        {
            // The rail opens the table, at its own width, and no drag takes it out of place.
            const int rail{ header->getColumnIndex(LoggingModelBase::eColumn::LogColumnRail) };
            CHECK(rail == 0);
            CHECK(header->visualIndex(rail) == 0);
            CHECK(header->sectionSize(rail) == LoggingModelBase::RailWidth);
            CHECK(header->sectionResizeMode(rail) == QHeaderView::ResizeMode::Fixed);

            header->moveSection(2, 0);
            QApplication::processEvents();
            QApplication::processEvents();
            CHECK(header->visualIndex(rail) == 0);
            CHECK(header->sectionSize(rail) == LoggingModelBase::RailWidth);

            // Widening the message column past the window is what a long line asks for, so
            // the last section is not stretched and the bar appears instead.
            CHECK(table->horizontalScrollBarPolicy() == Qt::ScrollBarPolicy::ScrollBarAsNeeded);
            CHECK(header->stretchLastSection() == false);

            const int message{ header->getColumnIndex(LoggingModelBase::eColumn::LogColumnMessage) };
            CHECK(message > 0);
            table->setColumnWidth(message, table->viewport()->width() + 400);
            QApplication::processEvents();
            CHECK(table->columnWidth(message) > table->viewport()->width());
            CHECK(table->horizontalScrollBar()->maximum() > 0);

            // The right button opens the menu of the table. The menu was written but the
            // request never reached it, so the connection itself is what is checked. Taking
            // it apart is the only way to ask, and this window is thrown away after.
            CHECK(table->contextMenuPolicy() == Qt::ContextMenuPolicy::CustomContextMenu);
            CHECK(QObject::disconnect(table, SIGNAL(customContextMenuRequested(QPoint)), &live, nullptr));
        }

        // A wrapped row is as tall as the lines its message takes, up to the given count.
        const QFontMetrics metrics{ table != nullptr ? table->font() : QApplication::font() };
        const QString single{ QStringLiteral("connect failed") };
        const QString many  { QString(QStringLiteral("payload 0x41 0x42 0x43 0x44 0x45 ")).repeated(20) };

        const int one { LogTextHighlight::wrappedHeight(single, metrics, 200, 4) };
        const int four{ LogTextHighlight::wrappedHeight(many  , metrics, 200, 4) };
        const int two { LogTextHighlight::wrappedHeight(many  , metrics, 200, 2) };
        CHECK(one < four);
        CHECK(two < four);
        CHECK(four <= (metrics.lineSpacing() * 4) + 4);
        CHECK(LogTextHighlight::wrappedHeight(QString(), metrics, 200, 4) == one);
    }

    std::printf("[rows] the rail opens every row, and a long message wraps when asked to\n");
    {
        ShotLogModel model;
        std::vector<areg::SharedBuffer> entries;
        entries.push_back(makeEntry(areg::LogPriority::PrioInfo   , 1000, "component started, waiting for the router"));
        entries.push_back(makeEntry(areg::LogPriority::PrioDebug  , 2000, "cache warm, 128 entries"));
        entries.push_back(makeEntry(areg::LogPriority::PrioWarning, 3000, "router answered late, 850 ms"));
        entries.push_back(makeEntry(areg::LogPriority::PrioError  , 4000
                                   , "connect failed to 10.0.0.5:8181, retry in 500 ms, attempt 3 of 10, "
                                     "last error was ECONNREFUSED and the socket was closed by the peer"));
        entries.push_back(makeEntry(areg::LogPriority::PrioFatal  , 5000, "out of memory, giving up"));
        model.addEntries(std::move(entries));

        QTableView table;
        LogTableHeader* header = new LogTableHeader(&table, &model);
        LogSearchModel::sFoundPos nothing{ };
        LogTextHighlight* paint = new LogTextHighlight(nothing, &table);

        table.setHorizontalHeader(header);
        table.setItemDelegate(paint);
        table.verticalHeader()->hide();
        table.setShowGrid(false);
        table.setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        LogViewerBase::applyRowHeight(&table);
        table.setModel(&model);
        table.resize(900, 220);

        const int message{ header->getColumnIndex(LoggingModelBase::eColumn::LogColumnMessage) };
        table.setColumnWidth(message, 380);
        QApplication::processEvents();

        CHECK(header->getColumnIndex(LoggingModelBase::eColumn::LogColumnRail) == 0);
        CHECK(header->sectionSize(0) == LoggingModelBase::RailWidth);
        shoot(table, "rows-rail");

        // The same rows, wrapped. The long one grows, the short ones keep their height.
        const int before{ table.rowHeight(3) };
        table.setWordWrap(true);
        paint->setWordWrap(true, 4);
        LogViewerBase::measureShownRows(&table, message, OptionsManager::LogRowHeightDefault, 4);
        QApplication::processEvents();

        CHECK(table.rowHeight(3) > before);
        CHECK(table.rowHeight(0) == before);
        shoot(table, "rows-wrapped");
    }

    std::printf("[filters] the panel names every column that can be narrowed, shown or not\n");
    {
        LogFilterPanel panel;
        LogFilterPanel::ListEntries entries;
        entries.append(LogFilterPanel::sEntry{ LoggingModelBase::eColumn::LogColumnPriority, QStringLiteral("Error"), true });
        entries.append(LogFilterPanel::sEntry{ LoggingModelBase::eColumn::LogColumnSource  , QString()             , true });
        entries.append(LogFilterPanel::sEntry{ LoggingModelBase::eColumn::LogColumnThread  , QString()             , false });
        entries.append(LogFilterPanel::sEntry{ LoggingModelBase::eColumn::LogColumnMessage , QStringLiteral("timeout"), true });
        panel.setEntries(entries);
        panel.resize(320, panel.sizeHint().height());
        QApplication::processEvents();

        QListWidget* list{ panel.findChild<QListWidget*>() };
        CHECK(list != nullptr);

        if (list != nullptr)
        {
            CHECK(list->count() == entries.size());

            // A narrowed column carries what it keeps, and says it in bold.
            CHECK(list->item(0)->text().contains(QStringLiteral("Error")));
            CHECK(list->item(0)->font().bold());
            CHECK(list->item(1)->font().bold() == false);

            // A column the table does not show is still reachable, and is marked apart.
            CHECK(list->item(2)->toolTip().isEmpty() == false);
        }

        shoot(panel, "filter-panel");
    }

    std::printf("[select] the menu marks every row of one call, one scope and one process\n");
    {
        // The live window fills its own model, so the rows are handed to it the way the
        // collector hands them over.
        qRegisterMetaType<areg::SharedBuffer>("areg::SharedBuffer");

        LiveLogViewer* live{ new LiveLogViewer(&window) };
        live->resize(1200, 600);
        QApplication::processEvents();

        LoggingModelBase* model{ live->getLoggingModel() };
        CHECK(model != nullptr);

        const areg::SharedBuffer rows[]
            { makeEntry(areg::LogPriority::PrioInfo, 1000, "first run" , 7u, 1u)
            , makeEntry(areg::LogPriority::PrioInfo, 1100, "second run", 7u, 2u)
            , makeEntry(areg::LogPriority::PrioInfo, 1200, "first run" , 7u, 1u)
            , makeEntry(areg::LogPriority::PrioInfo, 1300, "other"     , 9u, 3u)
            };

        bool handed{ true };
        for (const areg::SharedBuffer& row : rows)
        {
            handed = handed && QMetaObject::invokeMethod(model, "slotLogMessage"
                                                        , Qt::ConnectionType::DirectConnection
                                                        , Q_ARG(areg::SharedBuffer, row));
        }

        CHECK(handed);
        QApplication::processEvents();
        CHECK((model != nullptr) && (model->rowCount() == 4));

        QTableView* table{ live->getLoggingTable() };
        CHECK(table != nullptr);

        LogViewerFilter::sIsolation pick;
        pick.cookie    = 1u;
        pick.thread    = 42u;
        pick.scopeId   = 7u;
        pick.sessionId = 1u;

        // One call of a scope: the two rows of that run, and neither the second run nor the
        // other scope. The rows do not stand next to each other, which is the case the run
        // gathering has to get right.
        pick.kind = LogViewerFilter::eIsolation::IsolationCall;
        live->selectMatching(pick);
        QApplication::processEvents();
        CHECK((table != nullptr) && (table->selectionModel()->selectedRows().size() == 2));

        // The scope is every run of it.
        pick.kind = LogViewerFilter::eIsolation::IsolationScope;
        live->selectMatching(pick);
        CHECK((table != nullptr) && (table->selectionModel()->selectedRows().size() == 3));

        // The process is all of it, and one call of one scope is not.
        pick.kind = LogViewerFilter::eIsolation::IsolationProcess;
        live->selectMatching(pick);
        CHECK((table != nullptr) && (table->selectionModel()->selectedRows().size() == 4));

        // The Analyzer reads exactly the rows that are marked.
        ScopeOutputViewer& viewScope{ window.getOutputScopeLogs() };
        pick.kind = LogViewerFilter::eIsolation::IsolationCall;
        live->selectMatching(pick);
        live->analyzeSelection();
        QApplication::processEvents();

        QTableView* analyzed{ viewScope.findChild<QTableView*>() };
        CHECK((analyzed != nullptr) && (analyzed->model() != nullptr) && (analyzed->model()->rowCount() == 2));

        live->close();
        QApplication::processEvents();
    }

    std::printf("[scope analyzer] the table answers the right button, and reads what it is told to\n");
    {
        ShotLogModel model;
        std::vector<areg::SharedBuffer> entries;
        entries.push_back(makeEntry(areg::LogPriority::PrioInfo, 1000, "first run",  7u, 1u));
        entries.push_back(makeEntry(areg::LogPriority::PrioInfo, 1100, "first run",  7u, 1u));
        entries.push_back(makeEntry(areg::LogPriority::PrioInfo, 1200, "second run", 7u, 2u));
        entries.push_back(makeEntry(areg::LogPriority::PrioInfo, 1300, "second run", 7u, 2u));
        entries.push_back(makeEntry(areg::LogPriority::PrioInfo, 1400, "other scope", 9u, 3u));
        model.addEntries(std::move(entries));

        LiveLogViewer* live{ new LiveLogViewer(&window) };
        ScopeOutputViewer& scope{ window.getOutputScopeLogs() };
        scope.bindWindow(*live);

        QTableView* table{ scope.findChild<QTableView*>() };
        CHECK(table != nullptr);

        // Reading exactly the rows the reader picked. None of the radios names that, so
        // they all go quiet.
        scope.analyzeRows(&model, QList<int>{ 0, 4 });
        QApplication::processEvents();
        CHECK((table != nullptr) && (table->model() != nullptr) && (table->model()->rowCount() == 2));

        bool anyRadioOn{ false };
        for (const QRadioButton* radio : scope.findChildren<QRadioButton*>())
        {
            anyRadioOn = anyRadioOn || radio->isEnabled();
        }

        CHECK(anyRadioOn == false);

        // A row the reader takes out of the view goes, and the row is named in the
        // coordinates of the table rather than of the log.
        if (table != nullptr)
        {
            scope.hideRows(QList<int>{ 1 });
            QApplication::processEvents();
            CHECK(table->model()->rowCount() == 1);
        }

        // Reading a scope names the scope, and the control says so.
        scope.analyzeAt(&model, model.index(0, 0), ScopeLogViewerFilter::eDataFilter::FilterScope);
        QApplication::processEvents();
        const QRadioButton* onScope{ scope.findChild<QRadioButton*>(QStringLiteral("radioScope")) };
        CHECK((onScope != nullptr) && onScope->isEnabled() && onScope->isChecked());

        scope.analyzeAt(&model, model.index(0, 0), ScopeLogViewerFilter::eDataFilter::FilterProcess);
        QApplication::processEvents();
        const QRadioButton* onProcess{ scope.findChild<QRadioButton*>(QStringLiteral("radioProcess")) };
        CHECK((onProcess != nullptr) && onProcess->isChecked());

        // The right button opens the menu of the analyzer table. Taking the connection apart
        // is the only way to ask for it, and no later section opens that menu.
        if (table != nullptr)
        {
            CHECK(table->contextMenuPolicy() == Qt::ContextMenuPolicy::CustomContextMenu);
            CHECK(QObject::disconnect(table, SIGNAL(customContextMenuRequested(QPoint)), &scope, nullptr));
        }

        scope.releaseWindow(*live);
        live->close();
        QApplication::processEvents();
    }

    std::printf("[scope analyzer] the window it follows closes, and its own table stays whole\n");
    {
        ShotLogModel model;
        std::vector<areg::SharedBuffer> entries;
        for (int i = 0; i < 200; ++i)
        {
            entries.push_back(makeEntry(areg::LogPriority::PrioInfo, 1000 + i, "scope entered"));
        }

        model.addEntries(std::move(entries));

        LiveLogViewer* live{ new LiveLogViewer(&window) };
        live->resize(1200, 600);
        live->show();
        QApplication::processEvents();

        window.show();
        ScopeOutputViewer& scope{ window.getOutputScopeLogs() };
        scope.bindWindow(*live);
        scope.setupFilter(&model, model.index(0, 0));
        scope.resize(900, 200);
        scope.show();
        QApplication::processEvents();

        // The table has to stand on the screen with a scroll bar of its own. Losing that bar
        // is what resizes the viewport, and the resize is what shapes the table again.
        QTableView* shown{ scope.findChild<QTableView*>() };
        CHECK(shown != nullptr);
        if (shown != nullptr)
        {
            CHECK(shown->model() != nullptr);
            CHECK(shown->horizontalHeader()->count() > 0);
            shown->scrollToBottom();
            QApplication::processEvents();
            CHECK(shown->verticalScrollBar()->value() > 0);
        }

        // The log window is closing, so the analyzer lets the log go. Its own table loses
        // every column at that moment and must not be shaped against the columns of a log
        // it no longer reads.
        live->close();
        QApplication::processEvents();
        CHECK(shown == nullptr || shown->model() == nullptr);
    }

    std::printf("[fit] the table fills the window, and the rail keeps the left edge\n");
    {
        qRegisterMetaType<areg::SharedBuffer>("areg::SharedBuffer");

        LiveLogViewer* live{ new LiveLogViewer(&window) };
        live->resize(1400, 500);
        live->show();
        QApplication::processEvents();

        QTableView* table{ live->getLoggingTable() };
        LoggingModelBase* model{ live->getLoggingModel() };
        CHECK(table != nullptr);
        CHECK(model != nullptr);

        const auto taken = [table]() {
                int width{ 0 };
                for (int i = 0; i < table->model()->columnCount(); ++i)
                {
                    width += table->columnWidth(i);
                }

                return width;
            };

        // With no row the table has no vertical bar, and the columns fill the viewport.
        CHECK(table->verticalScrollBar()->isVisible() == false);
        CHECK(taken() == table->viewport()->width());
        CHECK(table->horizontalScrollBar()->maximum() == 0);

        for (int i = 0; i < 200; ++i)
        {
            const areg::SharedBuffer row{ makeEntry(areg::LogPriority::PrioInfo, 1000 + i
                                                   , "a log line long enough to fill the message column") };
            QMetaObject::invokeMethod(model, "slotLogMessage", Qt::ConnectionType::DirectConnection
                                     , Q_ARG(areg::SharedBuffer, row));
        }

        QApplication::processEvents();

        // The rows bring the vertical bar up, and it takes its own width out of the viewport.
        // The message column gives that width back, so no sideways bar appears for it.
        CHECK(table->verticalScrollBar()->isVisible());
        CHECK(taken() == table->viewport()->width());
        CHECK(table->horizontalScrollBar()->maximum() == 0);

        // Reading a row never moves the table sideways: the rail opens every row and has to
        // stay on the left edge.
        const int message{ model->fromColumnToIndex(LoggingModelBase::eColumn::LogColumnMessage) };
        CHECK(message > 0);
        table->setCurrentIndex(table->model()->index(199, message, QModelIndex()));
        table->scrollToBottom();
        QApplication::processEvents();
        CHECK(table->horizontalScrollBar()->value() == 0);
        CHECK(table->columnViewportPosition(0) == 0);

        // A column the reader widens is kept, and the table scrolls sideways instead.
        const int wide{ table->columnWidth(message) + 300 };
        table->setColumnWidth(message, wide);
        QApplication::processEvents();
        CHECK(table->columnWidth(message) == wide);
        CHECK(table->horizontalScrollBar()->maximum() == 300);

        // Narrowed back, the column takes the room that is left over again.
        table->setColumnWidth(message, wide - 300);
        QApplication::processEvents();
        CHECK(taken() == table->viewport()->width());
        CHECK(table->horizontalScrollBar()->maximum() == 0);

        // The room another column takes is not taken back out of the message column.
        const int stamp{ model->fromColumnToIndex(LoggingModelBase::eColumn::LogColumnTimestamp) };
        CHECK(stamp > 0);
        const int held{ table->columnWidth(message) };
        table->setColumnWidth(stamp, table->columnWidth(stamp) + 250);
        QApplication::processEvents();
        CHECK(table->columnWidth(message) == held);
        CHECK(table->horizontalScrollBar()->maximum() == 250);
        CHECK(table->columnViewportPosition(0) == 0);

        live->close();
        QApplication::processEvents();
    }

    std::printf("[row menu] the menu of a row names each subject once and marks its values\n");
    {
        qRegisterMetaType<areg::SharedBuffer>("areg::SharedBuffer");

        // The menu is drawn under the style sheet a theme installs, which is what decides
        // whether the group heading survives. This is the last section, so the theme stays.
        NEAppThemes::applyTheme(OptionsManager::eAppTheme::ModernLight);
        QApplication::processEvents();

        LiveLogViewer* live{ new LiveLogViewer(&window) };
        live->resize(1200, 600);
        live->show();
        QApplication::processEvents();

        LoggingModelBase* model{ live->getLoggingModel() };
        QTableView* table{ live->getLoggingTable() };
        CHECK(model != nullptr);
        CHECK(table != nullptr);

        const areg::SharedBuffer entry{ makeEntry(areg::LogPriority::PrioError, 1000, "connect failed", 7u, 1u) };
        QMetaObject::invokeMethod(model, "slotLogMessage", Qt::ConnectionType::DirectConnection
                                 , Q_ARG(areg::SharedBuffer, entry));
        QApplication::processEvents();
        CHECK(model->rowCount() == 1);

        QStringList opened;
        QStringList strong;
        int marked{ 0 };
        int coloured{ 0 };
        QTimer::singleShot(0, &window, [&opened, &strong, &marked, &coloured]() {
                QMenu* shown{ qobject_cast<QMenu *>(QApplication::activePopupWidget()) };
                if (shown == nullptr)
                    return;

                for (QAction* action : shown->actions())
                {
                    if (action->menu() != nullptr)
                    {
                        opened.append(action->text());
                        marked += action->icon().isNull() ? 0 : 1;
                        if (action->font().bold())
                        {
                            strong.append(action->text());
                        }
                    }
                    else if (action->text().startsWith(QStringLiteral("Filter:")) && (action->icon().isNull() == false))
                    {
                        ++coloured;
                    }
                }

                shoot(*shown, "row-menu");
                shown->close();
            });

        // The priority cell is the one clicked, so its filter entry stands at the top level.
        const int prio{ model->fromColumnToIndex(LoggingModelBase::eColumn::LogColumnPriority) };
        CHECK(prio > 0);
        const QPoint where{ table->visualRect(table->model()->index(0, prio, QModelIndex())).center() };
        QContextMenuEvent ask(QContextMenuEvent::Reason::Mouse, where, table->viewport()->mapToGlobal(where));
        QApplication::sendEvent(table->viewport(), &ask);
        QApplication::processEvents();

        // One entry per subject, and the old three verb submenus are gone.
        CHECK(opened.contains(QStringLiteral("This call")));
        CHECK(opened.contains(QStringLiteral("worker")));
        CHECK(opened.contains(QStringLiteral("target")));
        CHECK(opened.contains(QStringLiteral("Isolate")) == false);
        CHECK(opened.contains(QStringLiteral("Analyze")) == false);

        // A title that is a value the row carries is bold; a title that is words of the
        // application is not.
        CHECK(strong.contains(QStringLiteral("worker")));
        CHECK(strong.contains(QStringLiteral("target")));
        CHECK(strong.contains(QStringLiteral("This call")) == false);

        // Every subject entry carries its icon, and the priority entry carries its colour.
        CHECK(marked >= 4);
        CHECK(coloured == 1);

        live->close();
        QApplication::processEvents();
    }

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
