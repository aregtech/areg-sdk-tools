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
 *  \file        lusan/app/LusanApplication.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application object for managing GUI-related functionality.
 *
 ************************************************************************/

#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/LogCollectorClient.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/Workspace.hpp"
#include "lusan/data/common/WorkspaceEntry.hpp"
#include "lusan/view/common/WorkspaceSetupDialog.hpp"


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
    , "*.sqlog"
};

LusanApplication *  LusanApplication::theApp{nullptr};

LusanApplication::LusanApplication(int& argc, char** argv)
    : QApplication      (argc, argv)
    , mMainWindow       (nullptr)
    , mOptions          ( )
    , mIsRestarting     (false)
    , mDefaultEnabled   (true)
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

LogCollectorClient& LusanApplication::getLogCollectorClient(void)
{
    return LogCollectorClient::getInstance();
}

MdiMainWindow* LusanApplication::getMainWindow(void)
{
    return (LusanApplication::theApp != nullptr ? LusanApplication::theApp->mMainWindow : nullptr);
}

void LusanApplication::newWorkspace(void)
{
    LusanApplication& theApp{LusanApplication::getApplication()};
    
    theApp.mIsRestarting = true;
    theApp.mDefaultEnabled = false;
    if (theApp.mMainWindow != nullptr)
    {
        theApp.mMainWindow->close();
        theApp.mMainWindow = nullptr;
    }
}

bool LusanApplication::isWorkpacePath(const QString & path)
{
    if (LusanApplication::theApp == nullptr)
        return false;

    const WorkspaceEntry& workspace = LusanApplication::theApp->mOptions.getActiveWorkspace();
    const QStringList dirs = {
        workspace.getWorkspaceRoot(),
        workspace.getDirSources(),
        workspace.getDirDelivery(),
        workspace.getDirIncludes(),
        workspace.getDirLogs()
    };

    auto startsWith = [](const QString& base, const QString& value) -> bool {
        return !value.isEmpty() && base.startsWith(value, Qt::CaseInsensitive);
    };

    return std::any_of(dirs.begin(), dirs.end(), [&](const QString& dir) {
        return startsWith(path, dir);
    });
}

WorkspaceEntry LusanApplication::startupWorkspace(bool enableDefault)
{
    if (enableDefault == false)
    {
        mOptions.setDefaultWorkspace(0);
    }
    
    if (mOptions.hasDefaultWorkspace())
    {
        mOptions.activateDefaultWorkspace();
        return mOptions.getDefaultWorkspace();
    }
    else
    {
        Workspace workspace(mOptions);
        if (workspace.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
        {
            if (workspace.hasNewWorkspaceEntry())
            {
                WorkspaceSetupDialog setup;
                if (setup.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
                    setup.applyDirectories();
            }

            return mOptions.getActiveWorkspace();
        }
    }

    return WorkspaceEntry::InvalidWorkspace;
}

int LusanApplication::startupMainWindow(const WorkspaceEntry& curWorkspace)
{
    Q_ASSERT(curWorkspace.isValid());

    MdiMainWindow w;
    mMainWindow = &w;

    w.setWorkspaceRoot(curWorkspace.getWorkspaceRoot());
    w.showMaximized();
    w.show();
    emit signalApplicationRunning();
    setStyleSheet("{ background: palette(base); color: palette(text);}");    
    int result = exec();
    mMainWindow = nullptr;
    return result;
}

int LusanApplication::runApplication()
{
    int result{ 0 };
    mOptions.readOptions();
    
    do
    {
        mIsRestarting = false;
        WorkspaceEntry workspace = startupWorkspace(mDefaultEnabled);
        mDefaultEnabled = true;
        if (workspace.isValid() == false)
            break; // No workspace selected, exit application

        result = startupMainWindow(workspace);

    } while (mIsRestarting);

    return result;
}
