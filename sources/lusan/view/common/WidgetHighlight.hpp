#ifndef LUSAN_VIEW_COMMON_WIDGETHIGHLIGHT_HPP
#define LUSAN_VIEW_COMMON_WIDGETHIGHLIGHT_HPP
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
 *  \file        lusan/view/common/WidgetHighlight.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, a short-lived accent on the control a navigation landed on.
 *
 ************************************************************************/

/************************************************************************
 * Dependencies
 ************************************************************************/
class QWidget;

/**
 * \brief   Points the eye at one control after a jump that crossed pages.
 *
 *          Selecting a row tells the author which element is meant, not which of its fields is.
 *          When the jump came from something that knows the field -- a finding whose check names
 *          it -- the control gets scrolled into view, focused, and tinted for a moment, so the
 *          landing needs no second search. The tint restores itself and never becomes state.
 **/
namespace WidgetHighlight
{
    /**
     * \brief   Scrolls \p field into view inside its scroll area, gives it keyboard focus, and
     *          tints it for a couple of seconds. A null \p field is ignored, so a caller may
     *          pass a control a page does not have. Re-entrant: a second call on a control that
     *          is still tinted restarts the accent without leaking the tint into its style.
     **/
    void reveal(QWidget* field);
}

#endif  // LUSAN_VIEW_COMMON_WIDGETHIGHLIGHT_HPP
