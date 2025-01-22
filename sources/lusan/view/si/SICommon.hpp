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
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/si/SICommon.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface main window.
 *
 ************************************************************************/


namespace SICommon
{
    constexpr unsigned int FRAME_WIDTH      = 1080;
    constexpr unsigned int FRAME_HEIGHT     = 650;

    constexpr unsigned int  WIDGET_WIDTH    = 540;
    constexpr unsigned int  WIDGET_HEIGHT   = 600;

    template<class Widget, class Entry>
    inline void enableDeprecated(Widget* widget, Entry* entry, bool enable);

    template<class Widget, class Entry>
    inline void checkedDeprecated(Widget* widget, Entry* entry, bool isChecked);

    template<class Widget, class Entry>
    inline void setDeprecateHint(Widget* widget, Entry* entry, const QString& newText);
}

template<class Widget, class Entry>
inline void SICommon::enableDeprecated(Widget* widget, Entry* entry, bool enable)
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
    if (isChecked)
    {
        widget->ctrlDeprecateHint()->setEnabled(true);
        widget->ctrlDeprecateHint()->setText(entry->getDeprecateHint());
    }
    else
    {
        widget->ctrlDeprecateHint()->setEnabled(false);
        widget->ctrlDeprecateHint()->setText(QString());
    }
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
