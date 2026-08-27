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
 *  \file        lusan/common/NELogPalette.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the colours of the log windows.
 *
 ************************************************************************/
#ifndef LUSAN_COMMON_NELOGPALETTE_HPP
#define LUSAN_COMMON_NELOGPALETTE_HPP

/************************************************************************
 * Include files.
 ************************************************************************/
#include "areg/logging/LoggingDefs.hpp"

#include <QColor>

/**
 * \brief   The colours the log windows draw with.
 *
 *          Every colour of the log tree, the log table and the priority controls is
 *          named here and nowhere else. Each name has a light value and a dark value,
 *          and the pair is chosen by the theme in use.
 *
 *          Change a colour here and it changes on every log surface at once.
 **/
namespace NELogPalette
{
    /**
     * \brief   The role a colour plays, independent of the theme.
     **/
    enum class eLogColorRole : int
    {
          RoleFatal = 0     //!< Fatal, the loudest row on screen
        , RoleError         //!< Error
        , RoleWarning       //!< Warning
        , RoleInformation   //!< Information
        , RoleDebug         //!< Debug
        , RoleScope         //!< Scope enter and exit
        , RoleNotset        //!< The priority is not set on the scope
        , RoleCount         //!< Number of roles
    };

    /**
     * \brief   What a running target knows about the priorities set in the tree.
     **/
    enum class eLogStateRole : int
    {
          StatePending = 0  //!< Changed here and not in effect on the target
        , StateSent         //!< The change is on its way, no answer yet
        , StateSaved        //!< The target was asked to keep the priorities across a restart
        , StateCount        //!< Number of roles
    };

    /**
     * \brief   The transparency the log controls paint with.
     **/
    enum class eLogOpacity : int
    {
          OpacityTint = 0   //!< Fill of a chosen cell of the priority ladder
        , OpacityHover      //!< Wash over the cell under the pointer
        , OpacityGhost      //!< Track behind the cells that are not chosen
        , OpacityCount      //!< Number of values
    };

    /**
     * \brief   Returns true if the log windows should draw with their dark values.
     *          Decided from the palette in use, so it follows every theme without
     *          naming any of them.
     **/
    bool isDarkTheme(void);

    /**
     * \brief   Returns the alpha of the given use for the active theme.
     *          The values differ per theme: the same alpha recedes further on white
     *          than it does on a dark base.
     **/
    qreal opacity(eLogOpacity use);

    /**
     * \brief   Returns the given colour carrying the alpha of the given use.
     **/
    QColor withOpacity(const QColor & color, eLogOpacity use);

    /**
     * \brief   Returns the colour of the message text for the given role.
     *          Chosen so that every value clears 4.5:1 against its own background.
     **/
    QColor textColor(eLogColorRole role);

    /**
     * \brief   Returns the colour of the priority rail for the given role.
     *          Stronger than the text colour, because the rail is a 4 px bar
     *          rather than a letter shape.
     **/
    QColor railColor(eLogColorRole role);

    /**
     * \brief   Returns the colour of the mark that says what the target knows.
     *          The three values are the amber, the blue and the teal of the rail table,
     *          so the application carries one of each rather than two near neighbours.
     **/
    QColor stateColor(eLogStateRole role);

    /**
     * \brief   Returns the row background for the given role. Only Fatal has one;
     *          every other role returns a transparent colour so the row stays neutral.
     **/
    QColor rowBackground(eLogColorRole role);

    /**
     * \brief   Maps a log entry to the role its colours come from.
     **/
    eLogColorRole roleOf(const areg::LogEntry & entry);

    /**
     * \brief   Maps a log priority to the role its colours come from.
     **/
    eLogColorRole roleOf(areg::LogPriority prio);
}

#endif  // LUSAN_COMMON_NELOGPALETTE_HPP
