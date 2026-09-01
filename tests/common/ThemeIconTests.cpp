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
 *  \file        tests/common/ThemeIconTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       An icon follows the theme after it has been handed out, and the unit a
 *               measured time is written in follows the setting.
 *
 *  Self-contained (no external test framework), matching DocSchemaTests.cpp.
 *
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/common/NETimeUnits.hpp"

#include <QAction>
#include <QApplication>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <cstdio>

//////////////////////////////////////////////////////////////////////////
// Minimal assertion harness
//////////////////////////////////////////////////////////////////////////

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

#define CHECK(expr)     check((expr), #expr)

    //!< An icon the application ships and draws on a toolbar. The name is the alias the
    //!< resource file gives it, which is what every call site uses.
    const QString ToolbarIcon{ QStringLiteral(":/icons/Update Item") };

    //!< The mean lightness of the ink of a rendered icon. Fully clear pixels are skipped,
    //!< so the value describes the mark rather than the empty room around it.
    int inkLightness(const QIcon& icon, int extent)
    {
        const QImage image{ icon.pixmap(QSize(extent, extent), 1.0).toImage().convertToFormat(QImage::Format_ARGB32) };
        qint64 total{ 0 };
        qint64 count{ 0 };
        for (int y = 0; y < image.height(); ++y)
        {
            for (int x = 0; x < image.width(); ++x)
            {
                const QColor pixel{ image.pixelColor(x, y) };
                if (pixel.alpha() < 32)
                    continue;

                total += pixel.lightness();
                ++count;
            }
        }

        return count != 0 ? static_cast<int>(total / count) : -1;
    }
}

//////////////////////////////////////////////////////////////////////////
// The icon follows the theme after it was handed out
//////////////////////////////////////////////////////////////////////////

// The defect: the ink used to be baked into the pixmaps when the icon was loaded, so an icon
// already sitting on a QAction kept the ink of the theme that was active at that moment.
void testIconFollowsThemeAfterHandOut()
{
    std::printf("[icons] one icon, two themes, no reload\n");

    NELusanCommon::setIconsForDarkTheme(false);
    QAction action;
    action.setIcon(NELusanCommon::loadIcon(ToolbarIcon, NELusanCommon::SizeToolbar));

    const int onLight{ inkLightness(action.icon(), NELusanCommon::SizeToolbar.height()) };
    CHECK(onLight >= 0);

    // Nothing is loaded again and nothing is assigned: only the theme changes.
    NELusanCommon::setIconsForDarkTheme(true);
    const int onDark{ inkLightness(action.icon(), NELusanCommon::SizeToolbar.height()) };
    CHECK(onDark >= 0);

    CHECK(onDark > onLight);
    CHECK(onLight < 128);
    CHECK(onDark > 128);

    // And back, so the light theme is not a one-way door.
    NELusanCommon::setIconsForDarkTheme(false);
    CHECK(inkLightness(action.icon(), NELusanCommon::SizeToolbar.height()) == onLight);
}

// The extent named when the icon is loaded does not fix what it can be drawn at. An icon
// loaded small still renders at every size the tool asks for, in both themes.
// The marks are drawn on a 24 grid, so a request above that returns the grid size; the
// check is against what the icon reports, not against the request.
void testIconRendersAtEveryExtent()
{
    std::printf("[icons] the load extent does not fix the draw extent\n");

    const QIcon icon{ NELusanCommon::loadIcon(ToolbarIcon, NELusanCommon::SizeSmall) };
    CHECK(icon.isNull() == false);

    for (bool isDark : { false, true })
    {
        NELusanCommon::setIconsForDarkTheme(isDark);
        for (int extent : { 12, 16, 20, 24, 32, 48, 64 })
        {
            const QSize asked{ extent, extent };
            const QPixmap pixmap{ icon.pixmap(asked) };
            // Sizes are compared in device independent pixels: on a scaled display the pixmap
            // itself carries more physical pixels than the extent that was asked for.
            const QSize given{ pixmap.deviceIndependentSize().toSize() };
            CHECK(pixmap.isNull() == false);
            CHECK(given.width() <= extent);
            CHECK(given.width() == given.height());
        }

        // The sizes the toolbars, the menus and the item views draw at come back exact.
        for (const QSize& size : { NELusanCommon::SizeSmall, NELusanCommon::SizeToolbar, NELusanCommon::SizeMiddle })
        {
            CHECK(icon.pixmap(size).deviceIndependentSize().toSize() == size);
        }
    }

    NELusanCommon::setIconsForDarkTheme(false);
}

// A file that is not there stays a null icon rather than becoming an empty engine that
// every view then asks to draw.
void testMissingFileStaysNull()
{
    std::printf("[icons] a missing file stays a null icon\n");
    CHECK(NELusanCommon::loadIcon(QStringLiteral(":/icons/there is no such icon")).isNull());
}

//////////////////////////////////////////////////////////////////////////
// The unit a measured time is written in
//////////////////////////////////////////////////////////////////////////

void testTimeUnitFormatting()
{
    std::printf("[units] one measurement, three units\n");

    const uint64_t measured{ 1240u };

    // The reader sees the real symbol of the unit, so the number is checked against the
    // suffix the unit itself reports rather than against a spelling written out here.
    NETimeUnits::setUnit(NETimeUnits::eTimeUnit::UnitMicro);
    const QString micro{ NETimeUnits::unitSuffix(NETimeUnits::eTimeUnit::UnitMicro) };
    CHECK(NETimeUnits::duration(measured) == (QStringLiteral("1240 ") + micro));
    CHECK(NETimeUnits::offset(-measured) == (QStringLiteral("-1240 ") + micro));

    NETimeUnits::setUnit(NETimeUnits::eTimeUnit::UnitMilli);
    CHECK(NETimeUnits::duration(measured) == QStringLiteral("1.240 ms"));

    NETimeUnits::setUnit(NETimeUnits::eTimeUnit::UnitSecond);
    CHECK(NETimeUnits::duration(measured) == QStringLiteral("0.001240 s"));

    NETimeUnits::setUnit(NETimeUnits::DefaultUnit);
    CHECK(NETimeUnits::unit() == NETimeUnits::eTimeUnit::UnitMicro);
    CHECK(NETimeUnits::offset(0) == (QStringLiteral("+0 ") + micro));
}

// The stored name survives a round trip, and an options file written by an older build,
// which carries no unit at all, opens on microseconds.
void testTimeUnitKeys()
{
    std::printf("[units] the stored name round trips\n");

    for (NETimeUnits::eTimeUnit unit : { NETimeUnits::eTimeUnit::UnitMicro
                                       , NETimeUnits::eTimeUnit::UnitMilli
                                       , NETimeUnits::eTimeUnit::UnitSecond })
    {
        CHECK(NETimeUnits::unitFromKey(NETimeUnits::unitKey(unit)) == unit);
        CHECK(NETimeUnits::unitName(unit).isEmpty() == false);
    }

    CHECK(NETimeUnits::unitFromKey(QStringLiteral("MILLISECONDS")) == NETimeUnits::eTimeUnit::UnitMilli);
    CHECK(NETimeUnits::unitFromKey(QString()) == NETimeUnits::DefaultUnit);
    CHECK(NETimeUnits::unitFromKey(QStringLiteral("Fortnights")) == NETimeUnits::DefaultUnit);
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    std::printf("Theme icon and time unit tests\n");
    testIconFollowsThemeAfterHandOut();
    testIconRendersAtEveryExtent();
    testMissingFileStaysNull();
    testTimeUnitFormatting();
    testTimeUnitKeys();

    std::printf("\n%d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
