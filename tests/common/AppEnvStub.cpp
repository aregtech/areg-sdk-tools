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
 *  \file        tests/common/AppEnvStub.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The application environment a headless page harness needs, and nothing else.
 *
 *               Two of the shared pages ask the application for the workspace directories and
 *               the file filters when the author presses Browse. The real answers live in
 *               LusanApplication, whose translation unit drags in the main window and every
 *               dock with it. A harness that builds pages over a document opens no browse
 *               dialog, so these three answers are supplied here instead, and the cascade
 *               stays out of the test target.
 *
 ************************************************************************/

#include "lusan/app/LusanApplication.hpp"

#include <QDir>

QStringList LusanApplication::getExternalFileExtensions()
{
    return QStringList{ QStringLiteral("External Files (*.h *.hpp *.hxx)") };
}

QString LusanApplication::buildFileFilter(const QString& label, const QStringList& extensions)
{
    return QStringLiteral("%1 (%2)").arg(label, extensions.join(QLatin1Char(' ')));
}

QStringList LusanApplication::getWorkspaceDirectories()
{
    return QStringList{ QDir::tempPath() };
}
