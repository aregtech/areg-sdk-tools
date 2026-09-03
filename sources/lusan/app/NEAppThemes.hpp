#ifndef LUSAN_APPLICATION_NEAPPTHEMES_HPP
#define LUSAN_APPLICATION_NEAPPTHEMES_HPP
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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/app/NEAppThemes.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, visual themes of the application.
 *
 ************************************************************************/

#include "lusan/data/common/OptionsManager.hpp"

#include <QList>
#include <QString>

class QWidget;

/**
 * \namespace   NEAppThemes
 * \brief       Application visual themes: style, palette and stylesheet management.
 **/
namespace NEAppThemes
{
    /**
     * \brief   Returns the list of all available application themes.
     **/
    QList<OptionsManager::eAppTheme> allThemes();

    /**
     * \brief   Returns the human readable name of the given theme.
     * \param   theme   The theme to get the display name for.
     **/
    QString themeDisplayName(OptionsManager::eAppTheme theme);

    /**
     * \brief   Applies the given theme to the whole application.
     *          Sets the widget style, the color palette and the stylesheet.
     * \param   theme   The theme to apply.
     **/
    void applyTheme(OptionsManager::eAppTheme theme);

    /**
     * \brief   Puts the style sheet of the active theme on a window that stands on its own.
     *          A widget takes the style sheet of its parent chain, so a window created with a
     *          parent is already covered by that parent and this does nothing for it. Call it
     *          from the constructor of every window that is created without one.
     * \param   window  The window to hand the active theme's style sheet to.
     **/
    void applyThemeToWindow(QWidget& window);
}

#endif // LUSAN_APPLICATION_NEAPPTHEMES_HPP
