#ifndef LUSAN_APPLICATION_SI_SICOMMON_HPP
#define LUSAN_APPLICATION_SI_SICOMMON_HPP

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
 *  \file        lusan/view/si/SICommon.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface main window.
 *
 ************************************************************************/

#include <QString>

namespace SICommon
{
    constexpr unsigned int FRAME_WIDTH      = 1080;
    constexpr unsigned int FRAME_HEIGHT     = 650;

    constexpr unsigned int  WIDGET_WIDTH    = 540;
    constexpr unsigned int  WIDGET_HEIGHT   = 600;

    /**
     * \brief   The template function to enable or disable the deprication flag and deprecated hint.
     **/
    template<class Widget, class Entry>
    inline void enableDeprecated(Widget* widget, const Entry* entry, bool enable);

    /**
     * \brief   The template function to set or reset the deprecated flag.
     **/
    template<class Widget, class Entry>
    inline void checkedDeprecated(Widget* widget, Entry* entry, bool isChecked);

    /**
     * \brief   The template function to set or reset the deprecated hint.
     **/
    template<class Widget, class Entry>
    inline void setDeprecateHint(Widget* widget, Entry* entry, const QString& newText);
}

template<class Widget, class Entry>
inline void SICommon::enableDeprecated(Widget* widget, const Entry* entry, bool enable)
{
    widget->ctrlDeprecated()->setEnabled(enable);
    if (enable)
    {
        if (entry->getIsDeprecated())
        {
            widget->ctrlDeprecateHint()->setEnabled(true);
            widget->ctrlDeprecated()->setChecked(true);
            widget->ctrlDeprecateHint()->setText(entry->getDeprecateHint());
        }
        else
        {
            widget->ctrlDeprecateHint()->setEnabled(false);
            widget->ctrlDeprecated()->setChecked(false);
            widget->ctrlDeprecateHint()->setText(QString());
        }
    }
    else
    {
        widget->ctrlDeprecateHint()->setEnabled(false);
        widget->ctrlDeprecated()->setChecked(false);
        widget->ctrlDeprecateHint()->setText(QString());
    }
}

template<class Widget, class Entry>
inline void SICommon::checkedDeprecated(Widget* widget, Entry* entry, bool isChecked)
{
    entry->setIsDeprecated(isChecked);
    widget->ctrlDeprecateHint()->setEnabled(isChecked);
    widget->ctrlDeprecateHint()->setText(isChecked ? entry->getDeprecateHint() : QString());
    widget->ctrlDeprecateHint()->setFocus();
}

template<class Widget, class Entry>
inline void SICommon::setDeprecateHint(Widget* widget, Entry* entry, const QString& newText)
{
    if (entry->getIsDeprecated())
    {
        entry->setDeprecateHint(newText);
    }
}

#endif // LUSAN_APPLICATION_SI_SICOMMON_HPP
