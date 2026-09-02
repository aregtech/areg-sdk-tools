/************************************************************************
 *  Temporary probe: application palette vs widget palette across themes.
 ************************************************************************/

#include "lusan/app/LusanApplication.hpp"
#include "lusan/app/NEAppThemes.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"

#include "tests/common/UiTestEnv.hpp"

#include <QApplication>
#include <QMdiArea>
#include <QPalette>
#include <QStandardPaths>

#include <cstdio>

int main(int argc, char* argv[])
{
    LusanTest::prepareUiEnvironment();
    QStandardPaths::setTestModeEnabled(true);
    LusanApplication app(argc, argv);

    NEAppThemes::applyTheme(OptionsManager::eAppTheme::ModernLight);

    MdiMainWindow window;
    window.resize(1400, 900);
    window.show();
    QApplication::processEvents();

    QMdiArea* mdi{ window.findChild<QMdiArea*>() };

    std::printf("%-18s %-10s %-10s %-10s %-10s\n", "theme", "app[Dark]", "mdi[Dark]", "app[Win]", "mdi[Win]");
    for (OptionsManager::eAppTheme t : { OptionsManager::eAppTheme::ModernDark
                                       , OptionsManager::eAppTheme::SystemFusion
                                       , OptionsManager::eAppTheme::ModernLight
                                       , OptionsManager::eAppTheme::SystemDefault
                                       , OptionsManager::eAppTheme::MidnightBlue
                                       , OptionsManager::eAppTheme::Nord })
    {
        NEAppThemes::applyTheme(t);
        QApplication::processEvents();
        std::printf("%-18s %-10s %-10s %-10s %-10s\n"
                   , qPrintable(NEAppThemes::themeDisplayName(t))
                   , qPrintable(QApplication::palette().color(QPalette::ColorRole::Dark).name())
                   , qPrintable(mdi->palette().color(QPalette::ColorRole::Dark).name())
                   , qPrintable(QApplication::palette().color(QPalette::ColorRole::Window).name())
                   , qPrintable(mdi->palette().color(QPalette::ColorRole::Window).name()));
        std::fflush(stdout);
    }

    return 0;
}
