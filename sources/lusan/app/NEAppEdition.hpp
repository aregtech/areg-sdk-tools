#ifndef LUSAN_APPLICATION_NEAPPEDITION_HPP
#define LUSAN_APPLICATION_NEAPPEDITION_HPP
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
 *  \file        lusan/app/NEAppEdition.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the edition the build reports about itself.
 *
 ************************************************************************/

#include <QString>

#ifndef LUSAN_EDITION
    #define LUSAN_EDITION   "Free"
#endif  // LUSAN_EDITION

/**
 * \namespace   NEAppEdition
 * \brief       The edition name of the running build, set when the application is configured.
 **/
namespace NEAppEdition
{
    /**
     * \brief   Returns the edition name of the running build.
     **/
    inline QString edition(void)
    {
        return QString::fromLatin1(LUSAN_EDITION);
    }

    /**
     * \brief   Returns the application name followed by the edition, for a window title or an
     *          About box.
     * \param   appName     The application name to prefix.
     **/
    inline QString nameWithEdition(const QString& appName)
    {
        return appName + QStringLiteral(" ") + NEAppEdition::edition();
    }
}

#endif  // LUSAN_APPLICATION_NEAPPEDITION_HPP
