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
 *  \file        lusan/app/LusanApplication.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application object for managing GUI-related functionality.
 *
 ************************************************************************/

#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/LogCollectorClient.hpp"
#include "lusan/common/NELogPalette.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/common/NETimeUnits.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/Workspace.hpp"
#include "lusan/data/common/WorkspaceEntry.hpp"
#include "lusan/view/common/WorkspaceSetupDialog.hpp"
#include "lusan/app/NEAppThemes.hpp"

#include <QDir>

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
    , "*.fsml"
    , "*.sqlog"
};

LusanApplication *  LusanApplication::theApp{nullptr};

LusanApplication::LusanApplication(int& argc, char** argv)
    : QApplication      (argc, argv)
    , mMainWindow       (nullptr)
    , mOptions          ( )
    , mIsRestarting     (false)
    , mDefaultEnabled   (true)
    , mSwitchRoot       ( )
{
    Q_ASSERT(LusanApplication::theApp == nullptr);
    LusanApplication::theApp = this;
}

LusanApplication::~LusanApplication()
{
    LusanApplication::theApp = nullptr;
}

OptionsManager& LusanApplication::getOptions()
{
    Q_ASSERT(LusanApplication::theApp != nullptr);
    return LusanApplication::theApp->mOptions;
}

LusanApplication& LusanApplication::getApplication()
{
    Q_ASSERT(LusanApplication::theApp != nullptr);
    return (*LusanApplication::theApp);
}

WorkspaceEntry LusanApplication::getActiveWorkspace()
{
    Q_ASSERT(LusanApplication::theApp != nullptr);
    return LusanApplication::theApp->mOptions.getActiveWorkspace();
}

bool LusanApplication::isInitialized()
{
    return (LusanApplication::theApp != nullptr);
}

QStringList LusanApplication::getSupportedFileExtensions()
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

QStringList LusanApplication::getExternalFileExtensions()
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

QStringList LusanApplication::getInternalFileExtensions()
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

QString LusanApplication::buildFileFilter(const QString& label, const QStringList& extensions)
{
    return QStringLiteral("%1 (%2)").arg(label, extensions.join(QLatin1Char(' ')));
}

QStringList LusanApplication::getWorkspaceDirectories()
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

QString LusanApplication::getWorkspaceRoot()
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

QString LusanApplication::getWorkspaceSources()
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

QString LusanApplication::getWorkspaceIncludes()
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

QString LusanApplication::getWorkspaceDelivery()
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

QString LusanApplication::getWorkspaceLogs()
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

LogCollectorClient& LusanApplication::getLogCollectorClient()
{
    return LogCollectorClient::getInstance();
}

MdiMainWindow* LusanApplication::getMainWindow()
{
    return (LusanApplication::theApp != nullptr ? LusanApplication::theApp->mMainWindow : nullptr);
}

void LusanApplication::newWorkspace()
{
    LusanApplication& theApp{LusanApplication::getApplication()};
    if (theApp.mMainWindow == nullptr)
        return;

    theApp.mIsRestarting = true;
    theApp.mDefaultEnabled = false;
    if (theApp.mMainWindow->close() == false)
    {
        // A document refused to close, so the session stays on the current workspace.
        theApp.mIsRestarting = false;
        theApp.mDefaultEnabled = true;
        return;
    }

    theApp.mMainWindow = nullptr;
}

bool LusanApplication::switchWorkspace(const QString& workspaceRoot)
{
    LusanApplication& theApp{ LusanApplication::getApplication() };
    if (workspaceRoot.isEmpty() || (theApp.mMainWindow == nullptr))
        return false;

    if (workspaceRoot == theApp.mOptions.getActiveWorkspace().getWorkspaceRoot())
        return true;

    if ((theApp.mOptions.existsWorkspace(workspaceRoot) == false) || (QDir(workspaceRoot).exists() == false))
        return false;

    theApp.mSwitchRoot = workspaceRoot;
    theApp.mIsRestarting = true;
    if (theApp.mMainWindow->close() == false)
    {
        theApp.mSwitchRoot.clear();
        theApp.mIsRestarting = false;
        return false;
    }

    theApp.mMainWindow = nullptr;
    return true;
}

void LusanApplication::applyConfiguredTheme()
{
    if (LusanApplication::theApp == nullptr)
        return;
    
    NEAppThemes::applyTheme(LusanApplication::theApp->mOptions.getTheme());
    NETimeUnits::setUnit(LusanApplication::theApp->mOptions.getTimeUnit());
    NETimeUnits::setStamp(LusanApplication::theApp->mOptions.getTimeStamp());
    NELogPalette::setPalette(LusanApplication::theApp->mOptions.getLogPalette());
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
    if (mSwitchRoot.isEmpty() == false)
    {
        const QString root{ mSwitchRoot };
        mSwitchRoot.clear();

        // The description is carried over, activateWorkspace() overwrites the stored one.
        QString description;
        for (const WorkspaceEntry& entry : mOptions.getWorkspaceList())
        {
            if (entry.getWorkspaceRoot() == root)
            {
                description = entry.getWorkspaceDescription();
                break;
            }
        }

        WorkspaceEntry activated{ mOptions.activateWorkspace(root, description) };
        if (activated.isValid())
        {
            mOptions.writeOptions();
            return activated;
        }
    }

    if (enableDefault == false)
    {
        mOptions.setDefaultWorkspace(0);
    }

    // The remembered folder can be gone since the last run. Reopening it then gives an empty
    // tree with nothing to explain it, so the workspace dialog is offered instead -- the same
    // check that dialog already makes before it enables its OK button.
    const bool defaultUsable = mOptions.hasDefaultWorkspace()
                            && QDir(mOptions.getDefaultWorkspaceRoot()).exists();
    if (defaultUsable)
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

    // Every relative include location a document holds is measured from one of these, so they
    // have to be in place before the first document opens.
    NELusanCommon::setSearchRoots(LusanApplication::getWorkspaceDirectories());

    MdiMainWindow w;
    mMainWindow = &w;

    w.setWorkspaceRoot(curWorkspace.getWorkspaceRoot());
    w.showMaximized();
    w.show();
    emit signalApplicationRunning();
    int result = exec();
    mMainWindow = nullptr;
    return result;
}

int LusanApplication::runApplication()
{
    int result{ 0 };
    mOptions.readOptions();
    applyConfiguredTheme();
    
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
