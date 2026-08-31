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
 *  \file        tests/common/UiTestEnv.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The environment a widget test runs in. Header only, so a target adds it
 *               by including it and nothing else.
 *
 ************************************************************************/
#ifndef LUSAN_TESTS_COMMON_UITESTENV_HPP
#define LUSAN_TESTS_COMMON_UITESTENV_HPP

#include <QByteArray>
#include <QDir>
#include <QString>

#ifdef Q_OS_WIN
    #include <crtdbg.h>
    #include <cstdlib>
#endif  // Q_OS_WIN

namespace LusanTest
{
    /**
     * \brief   Stops a failed assertion from waiting for a person.
     *
     *          A debug build answers a failed Q_ASSERT with a modal dialog. Nobody is
     *          there to close it in a headless run, so the harness hangs instead of
     *          reporting. This makes the process die at once and carry the failure out
     *          in its exit code.
     **/
    inline void silenceAbortDialog(void)
    {
#ifdef Q_OS_WIN
        _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif  // Q_OS_WIN
    }

    /**
     * \brief   Prepares the platform a widget test draws on. Call before QApplication.
     *
     *          Chooses the offscreen platform unless the caller named one, and gives that
     *          platform a font directory. The offscreen plugin carries no font database of
     *          its own on Windows: with nothing to read it draws every character as an
     *          empty box, and a saved picture cannot be read by anyone.
     *
     * \note    Call it before the QApplication is constructed. Afterwards the platform
     *          plugin is already chosen and the font database is already built.
     **/
    inline void prepareUiEnvironment(void)
    {
        if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        {
            qputenv("QT_QPA_PLATFORM", "offscreen");
        }

#ifdef Q_OS_WIN
        if (qEnvironmentVariableIsEmpty("QT_QPA_FONTDIR"))
        {
            const QString root{ qEnvironmentVariable("SystemRoot", QStringLiteral("C:/Windows")) };
            const QString fonts{ QDir::fromNativeSeparators(root) + QStringLiteral("/Fonts") };
            if (QDir(fonts).exists())
            {
                qputenv("QT_QPA_FONTDIR", fonts.toLocal8Bit());
            }
        }
#endif  // Q_OS_WIN

        silenceAbortDialog();
    }
}

#endif  // LUSAN_TESTS_COMMON_UITESTENV_HPP
