#ifndef LUSAN_APPLICATION_SI_SICOMMON_HPP
#define LUSAN_APPLICATION_SI_SICOMMON_HPP

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
 *  \file        lusan/view/si/SICommon.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface main window.
 *
 ************************************************************************/

#include <QString>

namespace SICommon
{
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
