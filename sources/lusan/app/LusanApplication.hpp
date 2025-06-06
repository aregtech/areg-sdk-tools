﻿#ifndef LUSAN_APP_LUSANAPPLICATION_HPP
#define LUSAN_APP_LUSANAPPLICATION_HPP
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

#include <QApplication>
#include <QStringList>
#include "lusan/data/common/OptionsManager.hpp"

class LogCollectorClient;
class MdiMainWindow;

/**
 * \class   LusanApplication
 * \brief   Represents the main application object for managing GUI-related functionality.
 **/
class LusanApplication : public QApplication
{
    Q_OBJECT
//////////////////////////////////////////////////////////////////////////
// Types and constants
//////////////////////////////////////////////////////////////////////////
public:
    static const QStringList     ExternalExts;  //!< The list of external file extensions.
    static const QStringList     InternalExts;  //!< The list of internal file extensions.
    
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor.
     * \param   argc    The number of command-line arguments.
     * \param   argv    The array of command-line arguments.
     **/
    LusanApplication(int& argc, char** argv);

    /**
     * \brief   Destructor.
     **/
    virtual ~LusanApplication(void);
    
//////////////////////////////////////////////////////////////////////////
// Static methods
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the singleton instance of the application.
     * \return  Reference to the LusanApplication instance.
     **/
    static LusanApplication& getApplication(void);
    
    /**
     * \brief   Gets the options manager.
     * \return  Reference to the OptionsManager instance.
     **/
    static OptionsManager& getOptions(void);
    
    /**
     * \brief   Gets the active workspace.
     * \return  The active WorkspaceEntry.
     **/
    static WorkspaceEntry getActiveWorkspace(void);
    
    /**
     * \brief   Checks if the application is initialized.
     * \return  True if the application is initialized, false otherwise.
     **/
    static bool isInitialized(void);

    /**
     * \brief   Returns the list of supported file extensions.
     * \return  The list of supported file extensions.
     **/
    static QStringList getSupportedFileExtensions(void);

    /**
     * \brief   Returns the list of external file extensions.
     * \return  The list of external file extensions.
     **/
    static QStringList getExternalFileExtensions(void);

    /**
     * \brief   Returns the list of internal file extensions.
     * \return  The list of internal file extensions.
     **/
    static QStringList getInternalFileExtensions(void);

    /**
     * \brief   Returns the list of workspace directories.
     * \return  The list of workspace directories.
     **/
    static QStringList getWorkspaceDirectories(void);

    /**
     * \brief   Returns the workspace root directory.
     **/
    static QString getWorkspaceRoot(void);

    /**
     * \brief   Returns the workspace sources directory.
     **/
    static QString getWorkspaceSources(void);

    /**
     * \brief   Returns the workspace includes directory.
     **/
    static QString getWorkspaceIncludes(void);

    /**
     * \brief   Returns the workspace delivery directory.
     **/
    static QString getWorkspaceDelivery(void);

    /**
     * \brief   Returns the directory of workspace to save log files.
     **/
    static QString getWorkspaceLogs(void);

    /**
     * \brief   Returns the instance of the log observer client.
     **/
    static LogCollectorClient& getLogCollectorClient(void);
    
    /**
     * \brief   Returns the main window of the application.
     **/
    static MdiMainWindow* getMainWindow(void);
    
//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    
    int runApplication(const QString& workspace);

signals:
/************************************************************************
 * Signals
 ************************************************************************/
    /**
     * \brief   Signal emitted when the application is up and running, and ready to operate.
     **/
    void signalApplicationRunning(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    static LusanApplication *   theApp;         //!< The singleton instance of the application.
    MdiMainWindow*              mMainWindow;    //!< Main window;
    OptionsManager              mOptions;       //!< The options manager.
};

#endif // LUSAN_APP_LUSANAPPLICATION_HPP
