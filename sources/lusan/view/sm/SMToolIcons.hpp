#ifndef LUSAN_VIEW_SM_SMTOOLICONS_HPP
#define LUSAN_VIEW_SM_SMTOOLICONS_HPP
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
 *  \file        lusan/view/sm/SMToolIcons.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM drawing-toolbar vector icons.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QIcon>

/**
 * \namespace   SMToolIcons
 * \brief       Draws the FSM drawing-toolbar icons as simple monochrome vector glyphs, so
 *              the icon-only toolbar is usable without external image assets and the glyphs
 *              follow the active theme's text color. One glyph per canvas command; the style
 *              matches the thin-stroke glyphs already used in the state-body rows.
 **/
namespace SMToolIcons
{
    /**
     * \enum    eIcon
     * \brief   The drawing-toolbar command each glyph represents.
     **/
    enum class eIcon
    {
          AddState
        , AddFinalState
        , AddTransition
        , AddNote
        , StateColor
        , EdgeColor
        , NoteColor
        , AlignLeft
        , AlignRight
        , AlignTop
        , AlignBottom
        , DistributeHorizontal
        , DistributeVertical
        , ToggleSnap
        , ToggleGrid
        , GridDots
        , GridDotSize
        , GridSize
        , EnterSubmachine
        , GoToParent
        , CenterMachine
        , ZoomIn
        , ZoomOut
        , ZoomReset
        , ZoomFit
        , Undo
        , Redo
        , SelectAll
        , Cut
        , Copy
        , Paste
        , Duplicate
        , Declare
        , NewTrigger
        , NewAction
        , NewCondition
        , NewEvent
        , NewTimer
        , NewAttribute
        , NewConstant
        , NewDataType
    };

    /**
     * \brief   Returns the icon of the given command, drawn with the active application
     *          palette's text color (theme-aware).
     **/
    QIcon icon(eIcon kind);
}

#endif  // LUSAN_VIEW_SM_SMTOOLICONS_HPP
