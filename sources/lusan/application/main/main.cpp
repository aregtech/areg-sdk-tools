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
 *  \file        lusan/application/main/main.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application main function.
 *
 ************************************************************************/

#include "lusan/application/main/LusanApplication.hpp"
#include "lusan/application/main/Workspace.hpp"
#include "lusan/application/main/MdiMainWindow.hpp"

#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    LusanApplication a(argc, argv);
    LusanApplication::setOrganizationName("Aregtech");
    LusanApplication::setApplicationName("Lusan, GUI application for AREG Framework");
    LusanApplication::setApplicationVersion(QT_VERSION_STR);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Lusan_" + QLocale(locale).name();
        if (translator.load(":/ts/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    
    MdiMainWindow w;
    Workspace workspace;
    if (workspace.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        w.setWorkspaceRoot(workspace.getRootDirectory());
        w.show();
        return a.exec();
    }
    else
    {
        w.hide();
        return 0;
    }
}
