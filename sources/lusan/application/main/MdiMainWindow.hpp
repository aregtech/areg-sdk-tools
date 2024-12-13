#ifndef LUSAN_APPLICATION_MAIN_MDIMAINWINDOW_HPP
#define LUSAN_APPLICATION_MAIN_MDIMAINWINDOW_HPP
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
 *  \file        lusan/application/main/MdiMainWindow.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MdiMainWindow setup.
 *
 ************************************************************************/

#include <QMainWindow>

/************************************************************************
 * Dependencies
 ************************************************************************/
class MdiChild;
QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
QT_END_NAMESPACE

/**
 * \brief   The main window class for the Lusan application.
 *          This class manages the main window of the Lusan application,
 *          providing functionalities for file operations, editing, and
 *          window management within a Multiple Document Interface (MDI).
 **/
class MdiMainWindow : public QMainWindow
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief  Constructor for MdiMainWindow.
     **/
    MdiMainWindow();

    /**
     * \brief   Opens a file.
     * \param   fileName    The name of the file to open.
     * \return  True if the file was successfully opened, false otherwise.
     **/
    bool openFile(const QString& fileName);

    /**
     * \brief   Sets the workspace root directory.
     * \param   workspace    The path to the workspace root.
     **/
    inline void setWorkspaceRoot(const QString& workspace);

    /**
     * \brief   Gets the workspace root directory.
     * \return  The path to the workspace root.
     **/
    inline const QString& getWorkspaceRoot(void) const;

//////////////////////////////////////////////////////////////////////////
// protected methods
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Handles the close event.
     * \param   event   The close event.
     **/
    void closeEvent(QCloseEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    /**
     * \brief   Slot for creating a new SI file.
     **/
    void onFileNewSI(void);

    /**
     * \brief   Slot for creating a new log file.
     **/
    void onFileNewLog(void);

    /**
     * \brief   Slot for saving the current file.
     **/
    void onFileSave(void);

    /**
     * \brief   Slot for saving the current file with a new name.
     **/
    void onFileSaveAs(void);

    /**
     * \brief   Slot for opening a file.
     **/
    void onFileOpen(void);

    /**
     * \brief   Slot for exiting the application.
     **/
    void onFileExit(void);

    /**
     * \brief   Slot for opening a recent file.
     **/
    void onFileOpenRecent();

    /**
     * \brief   Slot for cutting text.
     **/
    void onEditCut();

    /**
     * \brief   Slot for copying text.
     **/
    void onEditCopy();

    /**
     * \brief   Slot for pasting text.
     **/
    void onEditPaste();

    /**
     * \brief   Slot for showing the about dialog.
     **/
    void onHelpAbout();

    /**
     * \brief   Updates the recent file actions.
     **/
    void updateRecentFileActions();

    /**
     * \brief   Updates the menus.
     **/
    void updateMenus();

    /**
     * \brief   Updates the window menu.
     **/
    void updateWindowMenu();

    /**
     * \brief   Creates a new MDI child window.
     * \return  A pointer to the new MDI child window.
     **/
    MdiChild* createMdiChild();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    enum { MaxRecentFiles = 5 };

    /**
     * \brief   Creates the actions for the main window.
     **/
    void _createActions();

    /**
     * \brief   Creates the status bar for the main window.
     **/
    void _createStatusBar();

    /**
     * \brief   Reads the settings for the main window.
     **/
    void _readSettings();

    /**
     * \brief   Writes the settings for the main window.
     **/
    void _writeSettings();

    /**
     * \brief   Loads a file.
     * \param   fileName    The name of the file to load.
     * \return  True if the file was successfully loaded, false otherwise.
     **/
    bool _loadFile(const QString& fileName);

    /**
     * \brief   Checks if there are recent files.
     * \return  True if there are recent files, false otherwise.
     **/
    static bool _hasRecentFiles();

    /**
     * \brief   Prepends a file to the recent files list.
     * \param   fileName    The name of the file to prepend.
     **/
    void _prependToRecentFiles(const QString& fileName);

    /**
     * \brief   Sets the visibility of the recent files.
     * \param   visible     True to make recent files visible, false otherwise.
     **/
    void _setRecentFilesVisibility(bool visible);

    /**
     * \brief   Gets the active MDI child window.
     * \return  A pointer to the active MDI child window.
     **/
    MdiChild* _activeMdiChild() const;

    /**
     * \brief   Finds an MDI child window by file name.
     * \param   fileName    The name of the file.
     * \return  A pointer to the MDI sub-window containing the file.
     **/
    QMdiSubWindow* _findMdiChild(const QString& fileName) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    //!< The root directory of the workspace.
    QString     mWorkspaceRoot;

    //!< The MDI area for managing sub-windows.
    QMdiArea*   mMdiArea;
    //!< The window menu.
    QMenu*      mMenuWindow;
    //!< Action for creating a new SI file.
    QAction*    mActFileNewSI;
    //!< Action for creating a new log file.
    QAction*    mActFileNewLog;
    //!< Action for saving the current file.
    QAction*    mActFileSave;
    //!< Action for saving the current file with a new name.
    QAction*    mActFileSaveAs;
    //!< Actions for opening recent files.
    QAction*    mActsRecentFiles[MaxRecentFiles];
    //!< Separator for the file menu.
    QAction*    mFileSeparator;
    //!< Submenu for recent files.
    QAction*    mActRecentFilesSubMenu;
    //!< Action for closing the current file.
    QAction*    mActFileClose;
    //!< Action for closing all files.
    QAction*    mActFileCloseAll;
    //!< Action for cutting text.
    QAction*    mActEditCut;
    //!< Action for copying text.
    QAction*    mActEditCopy;
    //!< Action for pasting text.
    QAction*    mActEditPaste;
    //!< Action for tiling windows.
    QAction*    mActWindowsTile;
    //!< Action for cascading windows.
    QAction*    mActWindowsCascade;
    //!< Action for switching to the next window.
    QAction*    mActWindowsNext;
    //!< Action for switching to the previous window.
    QAction*    mActWindowsPrev;
    //!< Separator for the window menu.
    QAction*    mActWindowMenuSeparator;
};

inline void MdiMainWindow::setWorkspaceRoot(const QString& workspace)
{
    mWorkspaceRoot = workspace;
}

inline const QString& MdiMainWindow::getWorkspaceRoot(void) const
{
    return mWorkspaceRoot;
}

#endif // LUSAN_APPLICATION_MAIN_MDIMAINWINDOW_HPP
