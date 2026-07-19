#ifndef LUSAN_VIEW_SM_NEGUARDSTYLE_HPP
#define LUSAN_VIEW_SM_NEGUARDSTYLE_HPP
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
 *  \file        lusan/view/sm/NEGuardStyle.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard editor visual tokens (v7 B15, one place).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QColor>
#include <QString>

/**
 * \namespace   NEGuardStyle
 * \brief   The single place the guard editor's B15 visual tokens live: owner colors,
 *          severity colors, the auto-map tint, and the ASCII owner glyphs. Everything that
 *          needs a color asks here (never a literal at the call site), so U4's dark-theme
 *          pass is a change to this one file. Light and dark values are both defined and
 *          picked by the active palette (the theme sets a dark palette; system default is
 *          judged by window lightness).
 **/
namespace NEGuardStyle
{
    /**
     * \enum    eOwner
     * \brief   The owner hue of a rendered span / catalog entry (B15 owner tokens).
     **/
    enum class eOwner
    {
          Stimulus  //!< guard.owner.stim  -- a stimulus parameter.
        , Fsm       //!< guard.owner.fsm   -- an attribute, constant, or named lambda.
        , Handler   //!< guard.owner.handler-- a handler condition.
        , Literal   //!< guard.literal     -- a verbatim literal.
        , Raw       //!< guard.raw         -- a raw-C++ fragment (dotted underline).
        , Operator  //!< an operator / punctuation (uses the default text color).
    };

    /**
     * \enum    eSeverity
     * \brief   The status severity (B15 guard.ok / warn / err).
     **/
    enum class eSeverity
    {
          Ok
        , Warn
        , Err
    };

    //!< True when the active palette is dark (selects the dark token values).
    bool isDark();

    //!< The owner hue for a span/pill/glyph.
    QColor ownerColor(eOwner owner);

    //!< The severity color (status icon, squiggle, tab glyph, edge label).
    QColor severityColor(eSeverity severity);

    //!< The background tint of a pre-filled, unvisited auto-map slot (guard.automap).
    QColor autoMapTint();

    //!< The ASCII glyph for an owner/kind cue used by the catalog, pills and hovers.
    QString ownerGlyph(eOwner owner);
}

#endif  // LUSAN_VIEW_SM_NEGUARDSTYLE_HPP
