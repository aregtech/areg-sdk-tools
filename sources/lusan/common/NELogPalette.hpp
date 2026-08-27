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
     * \brief   Returns true if the log windows should draw with their dark values.
     *          Decided from the palette in use, so it follows every theme without
     *          naming any of them.
     **/
    bool isDarkTheme(void);

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
