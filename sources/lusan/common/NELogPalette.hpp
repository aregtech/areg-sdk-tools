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
        , RoleScope         //!< A scope, where the two ends are not told apart
        , RoleNotset        //!< The priority is not set on the scope
        , RoleScopeEnter    //!< The row that enters a scope
        , RoleScopeExit     //!< The row that leaves a scope
        , RoleCount         //!< Number of roles
    };

    /**
     * \brief   The colour sets the log rows can be drawn with. The rail is the same in all
     *          three: it always says which kind of row this is. They differ in how much
     *          colour the message text itself carries.
     **/
    enum class eLogPalette : uint8_t
    {
          PaletteLadder = 0 //!< Every priority owns an ink, graded from loud to quiet
        , PaletteQuiet      //!< Only Fatal, Error and Warning are coloured
        , PaletteClassic    //!< Debug takes the ordinary text colour of the theme
        , PaletteCount      //!< Number of colour sets
    };

    //!< The colour set used until the user picks another one.
    constexpr eLogPalette DefaultPalette{ eLogPalette::PaletteLadder };

    /**
     * \brief   Returns the colour set the log windows draw with.
     **/
    eLogPalette palette(void);

    /**
     * \brief   Sets the colour set the log windows draw with. The caller repaints the
     *          views: the tables hold the colours of the previous set until they do.
     **/
    void setPalette(eLogPalette newPalette);

    /**
     * \brief   Returns the human readable name of the given colour set.
     **/
    QString paletteName(eLogPalette forPalette);

    /**
     * \brief   Returns the short description of what the given colour set does.
     **/
    QString paletteHint(eLogPalette forPalette);

    /**
     * \brief   Returns the stored name of the given colour set. Never translated.
     **/
    QString paletteKey(eLogPalette forPalette);

    /**
     * \brief   Returns the colour set of the given stored name, the default when unknown.
     **/
    eLogPalette paletteFromKey(const QString& key);

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
     * \brief   The marks the log table draws over a row that a search touched.
     **/
    enum class eLogMarkRole : int
    {
          MarkSearchHit = 0 //!< Behind the matched text inside a cell.
        , MarkSearchHitText //!< The matched text itself.
        , MarkRevealedRow   //!< Behind a row a filter hides and the search brought back.
        , MarkRevealedEdge  //!< The mark drawn in the gutter of such a row.
        , MarkCount         //!< Number of roles
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
     * \brief   Returns the colour of the message text for the given role, in the active
     *          colour set. Chosen so that every value clears 4.5:1 against its own
     *          background.
     **/
    QColor textColor(eLogColorRole role);

    /**
     * \brief   Returns the colour of the message text for the given role in the given
     *          colour set and theme, whatever is active. Used to draw a sample of a set
     *          the user has not chosen yet.
     * \param   role        The role to take the colour of.
     * \param   forPalette  The colour set to read.
     * \param   dark        True to read the dark values, false for the light ones.
     **/
    QColor textColorOf(eLogColorRole role, eLogPalette forPalette, bool dark);

    /**
     * \brief   Returns the colour of the priority rail for the given role and theme,
     *          whatever theme is active.
     **/
    QColor railColorOf(eLogColorRole role, bool dark);

    /**
     * \brief   Returns the row background for the given role and theme, whatever theme
     *          is active. Only Fatal has one.
     **/
    QColor rowBackgroundOf(eLogColorRole role, bool dark);

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
     * \brief   Returns the colour of the given search mark. The hit and the revealed row
     *          share one hue, so a revealed row reads as a consequence of the search.
     **/
    QColor markColor(eLogMarkRole role);

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
