/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/app/NEAppThemes.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
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
    QString baseStyleSheet(void)
    {
        return QString::fromUtf8(
            "QPushButton#linkDataTypes, QPushButton#linkAttributes, QPushButton#linkMethods,"
            "QPushButton#linkConstants, QPushButton#linkIncludes"
            "{ background: transparent; border: none; color: palette(link); text-align: left; padding: 2px 4px; }");
    }

    const QString& defaultStyleName(void)
    {
        static const QString _styleName{ QApplication::style() != nullptr ? QApplication::style()->objectName() : QString() };
        return _styleName;
    }
}

QList<OptionsManager::eAppTheme> NEAppThemes::allThemes(void)
{
    return QList<OptionsManager::eAppTheme>
    {
          OptionsManager::eAppTheme::SystemDefault
        , OptionsManager::eAppTheme::ModernLight
        , OptionsManager::eAppTheme::ModernDark
        , OptionsManager::eAppTheme::MidnightBlue
        , OptionsManager::eAppTheme::Nord
    };
}

QString NEAppThemes::themeDisplayName(OptionsManager::eAppTheme theme)
{
    switch (theme)
    {
    case OptionsManager::eAppTheme::SystemDefault:
        return QCoreApplication::translate("NEAppThemes", "System (Default)");
    case OptionsManager::eAppTheme::ModernLight:
        return QCoreApplication::translate("NEAppThemes", "Modern Light");
    case OptionsManager::eAppTheme::ModernDark:
        return QCoreApplication::translate("NEAppThemes", "Modern Dark");
    case OptionsManager::eAppTheme::MidnightBlue:
        return QCoreApplication::translate("NEAppThemes", "Midnight Blue");
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

    const QString& systemStyle = defaultStyleName();
    if (theme == OptionsManager::eAppTheme::SystemDefault)
    {
        NELusanCommon::setIconsForDarkTheme(false);
        app->styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
        QApplication::setStyle(QStyleFactory::create(systemStyle));
        QApplication::setPalette(QApplication::style()->standardPalette());
        app->setStyleSheet(baseStyleSheet());
    }
    else
    {
        const sThemeColors& colors = themeColors(theme);
        NELusanCommon::setIconsForDarkTheme(colors.isDark);
        app->styleHints()->setColorScheme(colors.isDark ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light);
        QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
        QApplication::setPalette(themePalette(colors));
        app->setStyleSheet(themeStyleSheet(colors));
    }
}
