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
 *  \file        tests/log/LogUiTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Offscreen checks of the log controls, made on what they actually paint:
 *               the priority ladder in both roles, the empty states, and the charger the
 *               scope tree draws. Saves a picture of each control when an output
 *               directory is passed.
 *
 *  Usage: lusan_log_ui_tests [grab output dir]
 *
 ************************************************************************/

#include "lusan/app/NEAppThemes.hpp"
#include "lusan/common/NELogPalette.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/view/log/LogEmptyState.hpp"
#include "lusan/view/log/LogPriorityBar.hpp"
#include "lusan/view/common/SearchLineEdit.hpp"

#include "tests/common/UiTestEnv.hpp"

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QLocale>
#include <QMouseEvent>
#include <QPalette>
#include <QPixmap>
#include <QPushButton>
#include <QComboBox>
#include <QSet>
#include <QStyleOptionFrame>
#include <QStyle>
#include <QStyleFactory>
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

    QString gGrabDir;

    void grab(QWidget& widget, const char* name)
    {
        if (gGrabDir.isEmpty() == false)
        {
            widget.grab().save(gGrabDir + QDir::separator() + QString::fromLatin1(name) + QStringLiteral(".png"));
        }
    }

    QImage renderOf(QWidget& widget)
    {
        return widget.grab().toImage().convertToFormat(QImage::Format_ARGB32);
    }

    //!< Counts the pixels painted in exactly this colour. The cell rails are filled
    //!< with antialiasing off, so a lit cell leaves its hue in the image untouched.
    int pixelsOf(const QImage& image, const QColor& color)
    {
        const QRgb wanted{ color.rgb() };
        int count{ 0 };
        for (int y = 0; y < image.height(); ++y)
        {
            for (int x = 0; x < image.width(); ++x)
            {
                if ((image.pixel(x, y) & 0x00FFFFFF) == (wanted & 0x00FFFFFF))
                {
                    ++count;
                }
            }
        }

        return count;
    }

    bool hasLevelRail(const QImage& image, NELogPalette::eLogColorRole role)
    {
        return pixelsOf(image, NELogPalette::railColor(role)) > 0;
    }

    //!< The pixels an icon actually draws on.
    int inkOf(const QIcon& icon, int extent)
    {
        const QImage image{ icon.pixmap(QSize(extent, extent)).toImage().convertToFormat(QImage::Format_ARGB32) };
        int count{ 0 };
        for (int y = 0; y < image.height(); ++y)
        {
            for (int x = 0; x < image.width(); ++x)
            {
                if (qAlpha(image.pixel(x, y)) > 16)
                {
                    ++count;
                }
            }
        }

        return count;
    }

    void clickAt(QWidget& widget, int x, int y)
    {
        const QPointF local(x, y);
        const QPointF global{ widget.mapToGlobal(QPoint(x, y)) };
        QMouseEvent press(QEvent::MouseButtonPress, local, global, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&widget, &press);
        QMouseEvent release(QEvent::MouseButtonRelease, local, global, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(&widget, &release);
        QApplication::processEvents();
    }
}

#define CHECK(cond)  check((cond), #cond)

int main(int argc, char* argv[])
{
    LusanTest::prepareUiEnvironment();

    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QApplication::setPalette(QApplication::style()->standardPalette());

    if (argc > 1)
    {
        gGrabDir = QString::fromLocal8Bit(argv[1]);
        QDir().mkpath(gGrabDir);
    }

    std::printf("[ladder] what the priority bar paints\n");
    {
        LogPriorityBar bar;
        bar.resize(bar.sizeHint());

        // Nothing is selected: the bar shows no level at all, not a disabled lit ladder.
        bar.setIdle(true);
        CHECK(bar.isIdle());
        const QImage idle{ renderOf(bar) };
        grab(bar, "ladder-idle");
        CHECK(hasLevelRail(idle, NELogPalette::eLogColorRole::RoleError) == false);
        CHECK(hasLevelRail(idle, NELogPalette::eLogColorRole::RoleWarning) == false);
        CHECK(hasLevelRail(idle, NELogPalette::eLogColorRole::RoleInformation) == false);
        CHECK(hasLevelRail(idle, NELogPalette::eLogColorRole::RoleDebug) == false);

        // Idle is a channel of its own: the owner sets the level and the idle state
        // together, so leaving idle is an explicit act and never a side effect of a level.
        bar.setIdle(false);

        // Silence is not a priority either: the off cell lights no severity.
        bar.setLevel(LogPriorityBar::eLogLevel::LevelOff);
        CHECK(bar.isIdle() == false);
        const QImage off{ renderOf(bar) };
        grab(bar, "ladder-off");
        CHECK(hasLevelRail(off, NELogPalette::eLogColorRole::RoleError) == false);
        CHECK(hasLevelRail(off, NELogPalette::eLogColorRole::RoleDebug) == false);
        CHECK(off != idle);

        // The fill is cumulative: a level lights every cell up to it and none above.
        bar.setLevel(LogPriorityBar::eLogLevel::LevelError);
        const QImage error{ renderOf(bar) };
        grab(bar, "ladder-error");
        CHECK(hasLevelRail(error, NELogPalette::eLogColorRole::RoleError));
        CHECK(hasLevelRail(error, NELogPalette::eLogColorRole::RoleWarning) == false);

        bar.setLevel(LogPriorityBar::eLogLevel::LevelWarning);
        const QImage warning{ renderOf(bar) };
        grab(bar, "ladder-warning");
        CHECK(hasLevelRail(warning, NELogPalette::eLogColorRole::RoleError));
        CHECK(hasLevelRail(warning, NELogPalette::eLogColorRole::RoleWarning));
        CHECK(hasLevelRail(warning, NELogPalette::eLogColorRole::RoleInformation) == false);
        CHECK(hasLevelRail(warning, NELogPalette::eLogColorRole::RoleDebug) == false);

        bar.setLevel(LogPriorityBar::eLogLevel::LevelInformation);
        const QImage information{ renderOf(bar) };
        CHECK(hasLevelRail(information, NELogPalette::eLogColorRole::RoleInformation));
        CHECK(hasLevelRail(information, NELogPalette::eLogColorRole::RoleDebug) == false);

        bar.setLevel(LogPriorityBar::eLogLevel::LevelDebug);
        const QImage debug{ renderOf(bar) };
        grab(bar, "ladder-debug");
        CHECK(hasLevelRail(debug, NELogPalette::eLogColorRole::RoleError));
        CHECK(hasLevelRail(debug, NELogPalette::eLogColorRole::RoleWarning));
        CHECK(hasLevelRail(debug, NELogPalette::eLogColorRole::RoleInformation));
        CHECK(hasLevelRail(debug, NELogPalette::eLogColorRole::RoleDebug));

        // A range some scopes disagree about keeps the rail and drops the fill, so the
        // ground shows through where a single level would have tinted the cell.
        const QColor ground{ bar.palette().color(QPalette::Base) };
        const int groundFull{ pixelsOf(debug, ground) };
        bar.setLevelRange(LogPriorityBar::eLogLevel::LevelError, LogPriorityBar::eLogLevel::LevelDebug);
        const QImage mixed{ renderOf(bar) };
        grab(bar, "ladder-mixed");
        CHECK(hasLevelRail(mixed, NELogPalette::eLogColorRole::RoleDebug));
        CHECK(pixelsOf(mixed, ground) > groundFull);
    }

    std::printf("[ladder] the cells answer the mouse across the whole width\n");
    {
        LogPriorityBar bar;
        bar.resize(bar.sizeHint());
        bar.setLevel(LogPriorityBar::eLogLevel::LevelOff);

        int emitted{ 0 };
        QObject::connect(&bar, &LogPriorityBar::signalLevelChanged, [&emitted](LogPriorityBar::eLogLevel) { ++emitted; });

        QSet<int> reached;
        int previous{ -1 };
        bool ascending{ true };
        for (int x = 0; x < bar.width(); ++x)
        {
            clickAt(bar, x, bar.height() / 2);
            const int level{ static_cast<int>(bar.level()) };
            reached.insert(level);
            ascending = ascending && (level >= previous);
            previous = level;
        }

        // Every level is reachable, and the ladder never runs backwards from left to right.
        CHECK(reached.size() == 5);
        CHECK(ascending);
        CHECK(emitted > 0);
        CHECK(static_cast<int>(bar.level()) == static_cast<int>(LogPriorityBar::eLogLevel::LevelDebug));
    }

    std::printf("[ladder] the view role is told apart from the target role\n");
    {
        LogPriorityBar target;
        target.setRole(LogPriorityBar::eBarRole::RoleTarget);
        target.setLevel(LogPriorityBar::eLogLevel::LevelWarning);
        target.resize(target.sizeHint());

        LogPriorityBar view;
        view.setRole(LogPriorityBar::eBarRole::RoleView);
        view.setLevel(LogPriorityBar::eLogLevel::LevelWarning);
        view.resize(target.size());

        CHECK(target.role() == LogPriorityBar::eBarRole::RoleTarget);
        CHECK(view.role() == LogPriorityBar::eBarRole::RoleView);

        const QImage targetImage{ renderOf(target) };
        const QImage viewImage{ renderOf(view) };
        grab(target, "ladder-role-target");
        grab(view, "ladder-role-view");

        // The two never look alike: they sit in the same window and mean different things.
        CHECK(targetImage != viewImage);

        // The leading cell reports the lowest rung, and the window that owns a view bar
        // turns that into every row again. The bar itself only has to say it was pressed.
        LogPriorityBar reset;
        reset.setRole(LogPriorityBar::eBarRole::RoleView);
        reset.resize(reset.sizeHint());
        reset.setLevel(LogPriorityBar::eLogLevel::LevelError);

        int leading{ 0 };
        QObject::connect(&reset, &LogPriorityBar::signalLevelChanged, [&leading](LogPriorityBar::eLogLevel level) {
                leading += (level == LogPriorityBar::eLogLevel::LevelOff ? 1 : 0);
            });

        clickAt(reset, 2, reset.height() / 2);
        CHECK(leading == 1);
    }

    std::printf("[empty] the five states and the way out of the filtered one\n");
    {
        QWidget host;
        host.resize(640, 400);
        LogEmptyState empty(&host);
        empty.resize(host.size());

        const LogEmptyState::eEmptyReason reasons[]
        {
              LogEmptyState::eEmptyReason::ReasonNotConnected
            , LogEmptyState::eEmptyReason::ReasonNoArchive
            , LogEmptyState::eEmptyReason::ReasonNoLiveLogs
            , LogEmptyState::eEmptyReason::ReasonEmptyArchive
            , LogEmptyState::eEmptyReason::ReasonFiltered
        };

        QSet<QString> headlines;
        bool everyStateVisible{ true };
        for (const LogEmptyState::eEmptyReason reason : reasons)
        {
            empty.setReason(reason, 0, false, false);
            everyStateVisible = everyStateVisible && (empty.isHidden() == false);
            for (const QLabel* label : empty.findChildren<QLabel*>())
            {
                if (label->text().isEmpty() == false)
                {
                    headlines.insert(label->text());
                }
            }
        }

        CHECK(everyStateVisible);
        // Each state says something of its own, headline and details alike.
        CHECK(headlines.size() >= 10);

        empty.setReason(LogEmptyState::eEmptyReason::ReasonNone, 0, false, false);
        CHECK(empty.isHidden());

        // The way out is offered only where there is one: a column filter or a hidden scope.
        QPushButton* action{ empty.findChild<QPushButton*>() };
        CHECK(action != nullptr);
        if (action != nullptr)
        {
            empty.setReason(LogEmptyState::eEmptyReason::ReasonFiltered, 1234, false, false);
            CHECK(action->isHidden());

            empty.setReason(LogEmptyState::eEmptyReason::ReasonFiltered, 1234, false, true);
            CHECK(action->isHidden() == false);

            empty.setReason(LogEmptyState::eEmptyReason::ReasonFiltered, 1234, true, false);
            CHECK(action->isHidden() == false);
            grab(empty, "empty-filtered");

            // The rows the filters hold are named, so the reader knows what is waiting.
            bool namesHeld{ false };
            for (const QLabel* label : empty.findChildren<QLabel*>())
            {
                namesHeld = namesHeld || label->text().contains(QLocale::system().toString(1234));
            }

            CHECK(namesHeld);

            int cleared{ 0 };
            QObject::connect(&empty, &LogEmptyState::signalClearFilters, [&cleared]() { ++cleared; });
            action->click();
            QApplication::processEvents();
            CHECK(cleared == 1);
        }
    }

    std::printf("[charger] the scope node icon at every size it is drawn\n");
    {
        const int extents[]{ 12, 14, 16, 20, 24 };

        LogIconFactory::sCharger quiet{ };
        quiet.level  = 1u;
        quiet.mixed  = 0u;
        quiet.lines  = LogIconFactory::eScopeLines::LinesOff;
        quiet.frozen = false;

        LogIconFactory::sCharger loud{ quiet };
        loud.level = 4u;

        bool everySizeDraws{ true };
        bool moreLevelsMoreInk{ true };
        for (const int extent : extents)
        {
            const QIcon quietIcon{ LogIconFactory::chargerIcon(quiet, static_cast<uint32_t>(extent)) };
            const QIcon loudIcon { LogIconFactory::chargerIcon(loud , static_cast<uint32_t>(extent)) };
            CHECK(quietIcon.isNull() == false);
            CHECK(loudIcon.isNull() == false);

            const int quietInk{ inkOf(quietIcon, extent) };
            const int loudInk { inkOf(loudIcon , extent) };
            everySizeDraws    = everySizeDraws && (quietInk > 0) && (loudInk > 0);
            moreLevelsMoreInk = moreLevelsMoreInk && (loudInk > quietInk);
        }

        CHECK(everySizeDraws);
        CHECK(moreLevelsMoreInk);

        // A gone process greys its charger, and the scope lines change the brackets.
        LogIconFactory::sCharger frozen{ loud };
        frozen.frozen = true;
        LogIconFactory::sCharger lined{ loud };
        lined.lines = LogIconFactory::eScopeLines::LinesOn;
        LogIconFactory::sCharger broken{ loud };
        broken.lines = LogIconFactory::eScopeLines::LinesPartial;

        const QImage loudImage  { LogIconFactory::chargerIcon(loud  , 24u).pixmap(24, 24).toImage().convertToFormat(QImage::Format_ARGB32) };
        const QImage frozenImage{ LogIconFactory::chargerIcon(frozen, 24u).pixmap(24, 24).toImage().convertToFormat(QImage::Format_ARGB32) };
        const QImage linedImage { LogIconFactory::chargerIcon(lined , 24u).pixmap(24, 24).toImage().convertToFormat(QImage::Format_ARGB32) };
        const QImage brokenImage{ LogIconFactory::chargerIcon(broken, 24u).pixmap(24, 24).toImage().convertToFormat(QImage::Format_ARGB32) };

        CHECK(loudImage != frozenImage);
        CHECK(loudImage != linedImage);
        CHECK(linedImage != brokenImage);

        // A level some scopes below disagree about is drawn apart from one they all carry.
        LogIconFactory::sCharger disagreed{ loud };
        disagreed.mixed = 0x0Fu;
        const QImage mixedImage{ LogIconFactory::chargerIcon(disagreed, 24u).pixmap(24, 24).toImage().convertToFormat(QImage::Format_ARGB32) };
        CHECK(loudImage != mixedImage);

        if (gGrabDir.isEmpty() == false)
        {
            LogIconFactory::chargerIcon(loud  , 24u).pixmap(24, 24).save(gGrabDir + QDir::separator() + QStringLiteral("charger-full.png"));
            LogIconFactory::chargerIcon(frozen, 24u).pixmap(24, 24).save(gGrabDir + QDir::separator() + QStringLiteral("charger-frozen.png"));
            LogIconFactory::chargerIcon(broken, 24u).pixmap(24, 24).save(gGrabDir + QDir::separator() + QStringLiteral("charger-broken.png"));
        }
    }

    // ---- every one-line input takes one height, and shows a whole line of text ----------
    std::printf("[inputs] one height for every input row, and room for a whole line\n");
    {
        NEAppThemes::applyTheme(OptionsManager::eAppTheme::ModernLight);

        SearchLineEdit find(QList<SearchLineEdit::eToolButton>{ SearchLineEdit::eToolButton::ToolButtonMatchCase });
        SearchLineEdit filter(QList<SearchLineEdit::eToolButton>{ });
        QComboBox workspace;
        workspace.setObjectName(QStringLiteral("naviWorkspaceSelector"));
        workspace.setFixedHeight(NELusanCommon::inputRowHeight(workspace));

        const int rowHeight{ NELusanCommon::inputRowHeight(find) };
        const int lineHeight{ QFontMetrics(find.font()).height() };
        std::printf("  input row height = %d px, a line of text is %d px\n", rowHeight, lineHeight);

        // The find box, the filter box and the workspace selector stand on one line.
        CHECK(find.height() == rowHeight);
        CHECK(filter.height() == rowHeight);
        CHECK(workspace.height() == rowHeight);

        // What is left for the text once the style took its frame and its padding is at least
        // a whole line. Below that the letters that reach under the line are cut off.
        QStyleOptionFrame option;
        option.initFrom(&find);
        option.lineWidth = find.style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, &find);
        const QRect textArea{ find.style()->subElementRect(QStyle::SE_LineEditContents, &option, &find) };
        const int above{ textArea.top() };
        const int below{ find.height() - textArea.bottom() - 1 };
        std::printf("  the text area is %d px, with %d px above it and %d px below\n"
                    , textArea.height(), above, below);

        CHECK(textArea.height() >= lineHeight);
        // The same room above the line as below it, so the text sits in the middle.
        CHECK(qAbs(above - below) <= 1);
    }

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
