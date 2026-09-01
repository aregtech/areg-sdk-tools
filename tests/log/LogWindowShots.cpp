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
#include "lusan/view/log/OfflineLogViewer.hpp"

#include <QDir>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QStandardPaths>
#include <QTableView>
#include <QHash>
#include <QLayout>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>

#include <cstdio>

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

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
