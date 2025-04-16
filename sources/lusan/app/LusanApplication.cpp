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
 *  \file        lusan/app/LusanApplication.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application object for managing GUI-related functionality.
 *
 ************************************************************************/

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/WorkspaceEntry.hpp"
#include "lusan/common/LogObserverClient.hpp"

const QStringList   LusanApplication::ExternalExts
{
      "*.c"
    , "*.cc"
    , "*.cpp"
    , "*.cxx"
    , "*.c++"
    , "*.cppm"
    , "*.ixx"

    , "*.h"
    , "*.h++"
    , "*.hh"
    , "*.hpp"
    , "*.hxx"
    , "*.inl"
    , "*.ipp"
    , "*.tlh"
    , "*.tli"
    , "*.inc"
};

const QStringList   LusanApplication::InternalExts
{
      "*.siml"
    , "*.dtml"
    , "*.coml"
    , "*.logs"
};

LusanApplication *  LusanApplication::theApp{nullptr};

LusanApplication::LusanApplication(int& argc, char** argv)
    : QApplication  (argc, argv)
    , mOptions      ( )
{
    Q_ASSERT(LusanApplication::theApp == nullptr);
    LusanApplication::theApp = this;
}

LusanApplication::~LusanApplication(void)
{
    LusanApplication::theApp = nullptr;
}

OptionsManager& LusanApplication::getOptions(void)
{
    Q_ASSERT(LusanApplication::theApp != nullptr);
    return LusanApplication::theApp->mOptions;
}

LusanApplication& LusanApplication::getApplication(void)
{
    Q_ASSERT(LusanApplication::theApp != nullptr);
    return (*LusanApplication::theApp);
}

WorkspaceEntry LusanApplication::getActiveWorkspace(void)
{
    Q_ASSERT(LusanApplication::theApp != nullptr);
    return LusanApplication::theApp->mOptions.getActiveWorkspace();
}

bool LusanApplication::isInitialized(void)
{
    return (LusanApplication::theApp != nullptr);
}

QStringList LusanApplication::getSupportedFileExtensions(void)
{
    QString result(tr("Supported Files"));
    result += " (";

    int count = 0;
    for (const QString& ext : ExternalExts)
    {
        if (count++ > 0)
        {
            result += " ";
        }

        result += ext;
    }

    for (const QString& ext : InternalExts)
    {
        if (count++ > 0)
        {
            result += " ";
        }

        result += ext;
    }

    result += ")";
    return QStringList(result);
}

QStringList LusanApplication::getExternalFileExtensions(void)
{
    QString externals(tr("External Files"));
    externals += " (";

    int count = 0;
    for (const QString& ext : ExternalExts)
    {
        if (count ++ > 0)
        {
            externals += " ";
        }

        externals += ext;
    }

    externals += ")";
    return QStringList(externals);
}

QStringList LusanApplication::getInternalFileExtensions(void)
{
    QString internals(tr("Internal Files"));
    internals += " (";

    int count = 0;
    for (const QString& ext : InternalExts)
    {
        if (count++ > 0)
        {
            internals += " ";
        }

        internals += ext;
    }

    internals += ")";
    return QStringList(internals);
}

QStringList LusanApplication::getWorkspaceDirectories(void)
{
    QStringList result;
    if (LusanApplication::theApp != nullptr)
    {
        const WorkspaceEntry & workspace = LusanApplication::theApp->mOptions.getActiveWorkspace();
        
        Q_ASSERT(workspace.getWorkspaceRoot().isEmpty() == false);
        result.append(workspace.getWorkspaceRoot());
        
        if (workspace.getDirSources().isEmpty() == false)
        {
            result.append(workspace.getDirSources());
        }
        
        if (workspace.getDirIncludes().isEmpty() == false)
        {
            result.append(workspace.getDirIncludes());
        }
        
        if (workspace.getDirDelivery().isEmpty() == false)
        {
            result.append(workspace.getDirDelivery());
        }
    }
    
    return result;
}

QString LusanApplication::getWorkspaceRoot(void)
{
    if (LusanApplication::theApp != nullptr)
    {
        const WorkspaceEntry& workspace = LusanApplication::theApp->mOptions.getActiveWorkspace();
        return workspace.getWorkspaceRoot();
    }
    else
    {
        return QString();
    }
}

QString LusanApplication::getWorkspaceSources(void)
{
    if (LusanApplication::theApp != nullptr)
    {
        const WorkspaceEntry& workspace = LusanApplication::theApp->mOptions.getActiveWorkspace();
        return workspace.getDirSources();
    }
    else
    {
        return QString();
    }
}

QString LusanApplication::getWorkspaceIncludes(void)
{
    if (LusanApplication::theApp != nullptr)
    {
        const WorkspaceEntry& workspace = LusanApplication::theApp->mOptions.getActiveWorkspace();
        return workspace.getDirIncludes();
    }
    else
    {
        return QString();
    }
}

QString LusanApplication::getWorkspaceDelivery(void)
{
    if (LusanApplication::theApp != nullptr)
    {
        const WorkspaceEntry& workspace = LusanApplication::theApp->mOptions.getActiveWorkspace();
        return workspace.getDirDelivery();
    }
    else
    {
        return QString();
    }
}

QString LusanApplication::getWorkspaceLogs(void)
{
    if (LusanApplication::theApp != nullptr)
    {
        const WorkspaceEntry& workspace = LusanApplication::theApp->mOptions.getActiveWorkspace();
        return workspace.getDirLogs();
    }
    else
    {
        return QString();
    }
}

LogObserverClient& LusanApplication::getLogObserverClient(void)
{
    return LogObserverClient::getInstance();
}
