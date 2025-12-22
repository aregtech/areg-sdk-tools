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
 *  \file        lusan/app/main.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application main function.
 *
 ************************************************************************/

#include "lusan/app/LusanApplication.hpp"
#include "areg/appbase/Application.hpp"

#include <QLocale>
#include <QTranslator>
#include <QCommonStyle>
#include <QStyleFactory>

namespace
{
    const QString _organization {"Aregtech"};   //!< Name of organization
    const QString _application  {"lusan"};      //!< Name of applicatin
    const QString _version      {"1.0.0"};      //!< Application version
}

int main(int argc, char *argv[])
{
    LusanApplication a(argc, argv);
    LusanApplication::setOrganizationName(_organization);
    LusanApplication::setApplicationName(_application);
    LusanApplication::setApplicationVersion(_version);
    Application::setWorkingDirectory(nullptr);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "Lusan_" + QLocale(locale).name();
        if (translator.load(":/res/" + baseName))
        {
            a.installTranslator(&translator);
            break;
        }
    }
    
    return a.runApplication();
}
