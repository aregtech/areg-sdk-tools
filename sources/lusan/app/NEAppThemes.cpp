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
 *  \file        lusan/app/NEAppThemes.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, visual themes of the application.
 *
 ************************************************************************/

#include "lusan/app/NEAppThemes.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QFile>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>
#include <QWidget>

namespace
{
    //!< The color set of one theme used in palette and stylesheet.
    struct sThemeColors
    {
        QString window;      //!< Main window and MDI area background.
        QString panel;       //!< Menu bar, toolbar, dock title background.
        QString base;        //!< Input fields and item view background.
        QString alt;         //!< Alternate row background.
        QString button;      //!< Push button background.
        QString text;        //!< Main text color.
        QString muted;       //!< Secondary text color.
        QString border;      //!< Border and separator color.
        QString hover;       //!< Subtle hover background.
        QString pressed;     //!< Pressed control background.
        QString accent;      //!< Accent and selection color.
        QString accentHover; //!< Accent hover color.
        QString onAccent;    //!< Text color on accent background.
        QString scroll;      //!< Scrollbar handle color.
        QString scrollHover; //!< Scrollbar handle hover color.
        QString link;        //!< Hyperlink color.
        bool    isDark;      //!< True for dark themes, selects the combo arrow.
    };

    const sThemeColors& themeColors(OptionsManager::eAppTheme theme)
    {
        static const sThemeColors _modernLight
        {
              "#f3f5f9", "#ffffff", "#ffffff", "#f6f8fb", "#ffffff"
            , "#1c2430", "#5b6675", "#d5dbe5", "#eaf1fd", "#dbe7fb"
            , "#2f6fed", "#245bd1", "#ffffff", "#c3ccd9", "#a9b5c6"
            , "#2f6fed", false
        };
        static const sThemeColors _modernDark
        {
              "#1b1e24", "#22262e", "#262b34", "#2b303a", "#2b303a"
            , "#dfe4ec", "#98a2b3", "#3a4150", "#313845", "#3c4554"
            , "#4f8cff", "#6ba1ff", "#0f1420", "#454e5e", "#5a6478"
            , "#6ba1ff", true
        };
        static const sThemeColors _midnightBlue
        {
              "#0d1526", "#111c33", "#142140", "#182747", "#16264a"
            , "#d7e2f5", "#8fa3c7", "#23345c", "#1b2c52", "#223a6b"
            , "#38bdf8", "#6fd0ff", "#06121f", "#2c4070", "#3a5290"
            , "#38bdf8", true
        };
        static const sThemeColors _nord
        {
              "#2e3440", "#353c4a", "#3b4252", "#404859", "#414a5c"
            , "#e5e9f0", "#aeb8ca", "#4c566a", "#434c5e", "#4c566a"
            , "#88c0d0", "#9ed0de", "#20242d", "#4c566a", "#5e81ac"
            , "#88c0d0", true
        };

        switch (theme)
        {
        case OptionsManager::eAppTheme::ModernDark:
            return _modernDark;
        case OptionsManager::eAppTheme::MidnightBlue:
            return _midnightBlue;
        case OptionsManager::eAppTheme::Nord:
            return _nord;
        case OptionsManager::eAppTheme::ModernLight:
        default:
            return _modernLight;
        }
    }

    QPalette themePalette(const sThemeColors& colors)
    {
        QPalette palette;
        const QColor window(colors.window);
        const QColor base(colors.base);
        const QColor alt(colors.alt);
        const QColor button(colors.button);
        const QColor text(colors.text);
        const QColor muted(colors.muted);
        const QColor accent(colors.accent);
        const QColor onAccent(colors.onAccent);

        palette.setColor(QPalette::Window, window);
        palette.setColor(QPalette::WindowText, text);
        // QMdiArea paints its background with the Dark role.
        palette.setColor(QPalette::Dark, window);
        palette.setColor(QPalette::Base, base);
        palette.setColor(QPalette::AlternateBase, alt);
        palette.setColor(QPalette::Text, text);
        palette.setColor(QPalette::PlaceholderText, muted);
        palette.setColor(QPalette::Button, button);
        palette.setColor(QPalette::ButtonText, text);
        palette.setColor(QPalette::BrightText, Qt::red);
        palette.setColor(QPalette::Highlight, accent);
        palette.setColor(QPalette::HighlightedText, onAccent);
        palette.setColor(QPalette::Link, QColor(colors.link));
        palette.setColor(QPalette::LinkVisited, QColor(colors.accentHover));
        palette.setColor(QPalette::ToolTipBase, base);
        palette.setColor(QPalette::ToolTipText, text);
        palette.setColor(QPalette::Disabled, QPalette::WindowText, muted);
        palette.setColor(QPalette::Disabled, QPalette::Text, muted);
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, muted);
        palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(colors.border));
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, muted);
        return palette;
    }

    QString themeStyleSheet(const sThemeColors& colors)
    {
        QFile file(QStringLiteral(":/styles/theme-template.qss"));
        if (file.open(QIODevice::ReadOnly) == false)
            return QString();

        QString qss = QString::fromUtf8(file.readAll());
        const QString arrow{ colors.isDark ? QStringLiteral(":/styles/arrow-down-light.svg")
                                           : QStringLiteral(":/styles/arrow-down-dark.svg") };
        qss.replace(QStringLiteral("${window}")     , colors.window);
        qss.replace(QStringLiteral("${panel}")      , colors.panel);
        qss.replace(QStringLiteral("${base}")       , colors.base);
        qss.replace(QStringLiteral("${alt}")        , colors.alt);
        qss.replace(QStringLiteral("${button}")     , colors.button);
        qss.replace(QStringLiteral("${text}")       , colors.text);
        qss.replace(QStringLiteral("${muted}")      , colors.muted);
        qss.replace(QStringLiteral("${border}")     , colors.border);
        qss.replace(QStringLiteral("${hover}")      , colors.hover);
        qss.replace(QStringLiteral("${pressed}")    , colors.pressed);
        qss.replace(QStringLiteral("${accent}")     , colors.accent);
        qss.replace(QStringLiteral("${accentHover}"), colors.accentHover);
        qss.replace(QStringLiteral("${onAccent}")   , colors.onAccent);
        qss.replace(QStringLiteral("${scroll}")     , colors.scroll);
        qss.replace(QStringLiteral("${scrollHover}"), colors.scrollHover);
        qss.replace(QStringLiteral("${arrow}")      , arrow);
        return qss;
    }

    //!< Minimal stylesheet used with the system default theme, keeps overview links consistent.
    QString baseStyleSheet()
    {
        // The dock drop guides need explicit colors: their palette fallback paints the arrow in the
        // icon's own background and leaves the guides blank. A fixed accent reads on both themes.
        return QString::fromUtf8(
            "QPushButton#linkDataTypes, QPushButton#linkAttributes, QPushButton#linkMethods,"
            "QPushButton#linkConstants, QPushButton#linkIncludes"
            "{ background: transparent; border: none; color: palette(link); text-align: left; padding: 2px 4px; }"
            "ads--CDockOverlayCross"
            "{ qproperty-iconColors: \"Frame=#ff2f6fed Background=#ffffffff Overlay=#ff2f6fed"
            " Arrow=#ff2f6fed Shadow=#ff000000\"; }"
            // The dock resize grip is invisible with the stock style; mark it with a seam
            // line that lights up on hover (see the matching rule in theme-template.qss).
            "ads--CDockSplitter::handle:horizontal"
            "{ width: 6px; border-left: 1px solid palette(mid); }"
            "ads--CDockSplitter::handle:vertical"
            "{ height: 6px; border-top: 1px solid palette(mid); }"
            "ads--CDockSplitter::handle:hover"
            "{ background: #ff2f6fed; }");
    }

    //!< The palette the desktop started the application with. Latches on the first call,
    //!< which happens before any theme has replaced it.
    const QPalette& defaultPalette()
    {
        static const QPalette _palette{ QApplication::palette() };
        return _palette;
    }

    const QString& defaultStyleName()
    {
        static const QString _styleName{ QApplication::style() != nullptr ? QApplication::style()->objectName() : QString() };
        return _styleName;
    }

    //!< The style this namespace installed last. Empty until the first theme is applied.
    QString& appliedStyleName()
    {
        static QString _applied;
        return _applied;
    }

    //!< The style sheet of the theme in force. Empty until the first theme is applied.
    QString& activeStyleSheet()
    {
        static QString _sheet;
        return _sheet;
    }

    //!< Hands the sheet to every window that carries its own style chain.
    void installStyleSheet(const QString& sheet)
    {
        const QWidgetList tops{ QApplication::topLevelWidgets() };
        for (QWidget* top : tops)
        {
            if ((top != nullptr) && (top->parentWidget() == nullptr))
            {
                top->setStyleSheet(sheet);
            }
        }
    }
}

QList<OptionsManager::eAppTheme> NEAppThemes::allThemes()
{
    // The designed themes come first and the one the application starts on leads them. The
    // native system theme is last: it is the only entry that swaps the widget style, which
    // takes far longer than every other switch.
    return QList<OptionsManager::eAppTheme>
    {
          OptionsManager::eAppTheme::ModernLight
        , OptionsManager::eAppTheme::ModernDark
        , OptionsManager::eAppTheme::MidnightBlue
        , OptionsManager::eAppTheme::Nord
        , OptionsManager::eAppTheme::SystemFusion
        , OptionsManager::eAppTheme::SystemDefault
    };
}

QString NEAppThemes::themeDisplayName(OptionsManager::eAppTheme theme)
{
    switch (theme)
    {
    case OptionsManager::eAppTheme::SystemDefault:
        return QCoreApplication::translate("NEAppThemes", "System (Default)");
    case OptionsManager::eAppTheme::SystemFusion:
        return QCoreApplication::translate("NEAppThemes", "System (Fusion)");
    case OptionsManager::eAppTheme::ModernLight:
        return QCoreApplication::translate("NEAppThemes", "Light");
    case OptionsManager::eAppTheme::ModernDark:
        return QCoreApplication::translate("NEAppThemes", "Dark");
    case OptionsManager::eAppTheme::MidnightBlue:
        return QCoreApplication::translate("NEAppThemes", "Blue");
    case OptionsManager::eAppTheme::Nord:
        return QCoreApplication::translate("NEAppThemes", "Nord");
    default:
        return QCoreApplication::translate("NEAppThemes", "Unknown");
    }
}

void NEAppThemes::applyTheme(OptionsManager::eAppTheme theme)
{
    QApplication* app = qobject_cast<QApplication*>(QCoreApplication::instance());
    if (app == nullptr)
        return;

    // Read unconditionally: it latches the style the desktop started the application with,
    // and it has to latch before the first theme replaces that style.
    const QString& systemStyle = defaultStyleName();
    const QPalette& systemPalette = defaultPalette();
    const bool native{ theme == OptionsManager::eAppTheme::SystemDefault };
    const bool system{ native || (theme == OptionsManager::eAppTheme::SystemFusion) };
    const QString wanted{ native ? systemStyle : QStringLiteral("Fusion") };

    // Installing a style walks and repolishes every widget of the application through the
    // style sheet that is still on, which costs about as much as the sheet itself. The four
    // built-in themes all run on Fusion, so the style is only swapped when it really changes.
    if (wanted != appliedStyleName())
    {
        QApplication::setStyle(QStyleFactory::create(wanted));
        appliedStyleName() = wanted;
    }

    // The sheet goes on the windows, not on the application. Handing it to the application makes
    // Qt walk and rebuild the rules of every widget there is, which on a session with a document
    // open takes several times longer than handing the same sheet to the windows themselves. A
    // widget reads the sheets of its own parent chain, so one call per parentless window reaches
    // everything under it, dialogs, menus and tool tips included.
    if (system)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        app->styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
#endif
        // The native variant reads the colours back from the style the desktop gave it. The
        // Fusion variant cannot: Fusion answers with its own light palette, so it takes the
        // palette the desktop handed over at startup instead.
        QApplication::setPalette(native ? QApplication::style()->standardPalette() : systemPalette);
        activeStyleSheet() = baseStyleSheet();
        // The system theme takes its colours from the desktop, so the ink of the icons is
        // read back from the palette that was just installed.
        NELusanCommon::setIconsForDarkTheme(QApplication::palette().color(QPalette::ColorRole::Window).lightness() < 128);
    }
    else
    {
        const sThemeColors& colors = themeColors(theme);
        NELusanCommon::setIconsForDarkTheme(colors.isDark);
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        app->styleHints()->setColorScheme(colors.isDark ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light);
#endif
        QApplication::setPalette(themePalette(colors));
        activeStyleSheet() = themeStyleSheet(colors);
    }

    installStyleSheet(activeStyleSheet());
}

void NEAppThemes::applyThemeToWindow(QWidget& window)
{
    if (window.parentWidget() == nullptr)
    {
        window.setStyleSheet(activeStyleSheet());
    }
}
