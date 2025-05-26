#ifndef LUSAN_VIEW_COMMON_MDIMAINWINDOW_HPP
#define LUSAN_VIEW_COMMON_MDIMAINWINDOW_HPP
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
 *  \file        lusan/view/common/MdiMainWindow.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MdiMainWindow setup.
 *
 ************************************************************************/

#include <QMainWindow>
#include <QAction>

#include "lusan/view/common/MdiArea.hpp"
#include "lusan/view/common/NaviFileSystem.hpp"
#include "lusan/view/common/Navigation.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class MdiChild;
class ServiceInterface;
class LogViewer;
QT_BEGIN_NAMESPACE
class QDockWidget;
class QListView;
class QMdiArea;
class QMdiSubWindow;
class QMenu;
class QTabWidget;
class QToolBar;
class QTreeView;
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

    void logCollecttorConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath);

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

    /**
     * \brief   Returns the last file opened or saved.
     **/
    inline const QString getLastFile(void) const;

    /**
     * \brief   Sets the last file opened or saved.
     * \param   lastFile    The last file opened or saved.
     **/
    inline void setLastFile(const QString& lastFile);

    /**
     * \brief   Returns a pointer to the live log viewer window.
     *          Returns nullptr if does not receive logs in live mode.
     **/
    inline LogViewer* getLiveLogViewer(void) const;

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

    void onViewNavigator();

    void onViewWorkspace();

    void onViewLogs();

    void onViewStatus();

    void onToolsOptions(void);
    
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
    MdiChild* createMdiChild(const QString& filePath = QString());

    /**
     * \brief   Creates a new Service Interface View.
     * \return  A pointer to the new Service Interface View.
     **/
    ServiceInterface* createServiceInterfaceView(const QString& filePath = QString());
    
    LogViewer* createLogViewerView(const QString& filePath = QString());

    /**
     * \brief   Returns the file filter string, which contains the list of supported extensions.
     *          The string is used to open files supported by lusan application.
     **/
    const QString& fileFilters(void) const;

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
     * \brief   Creates the menus for the main window.
     **/
    void _createMenus();

    /**
     * \brief   Creates the toolbars for the main window.
     **/
    void _createToolBars();

    /**
     * \brief   Creates the status bar for the main window.
     **/
    void _createStatusBar();

    /**
     * \brief   Creates the dock windows for the main window.
     **/
    void _createDockWindows();

    /**
     * \brief   Creates the MDI area for managing sub-windows.
     **/
    void _createMdiArea();

    /**
     * \brief   Reads the settings for the main window.
     **/
    void readSettings();

    /**
     * \brief   Writes the settings for the main window.
     **/
    void writeSettings();

    /**
     * \brief   Loads a file.
     * \param   fileName    The name of the file to load.
     * \return  True if the file was successfully loaded, false otherwise.
     **/
    bool loadFile(const QString& fileName);

    /**
     * \brief   Checks if there are recent files.
     * \return  True if there are recent files, false otherwise.
     **/
    static bool hasRecentFiles();

    /**
     * \brief   Prepends a file to the recent files list.
     * \param   fileName    The name of the file to prepend.
     **/
    void prependToRecentFiles(const QString& fileName);

    /**
     * \brief   Sets the visibility of the recent files.
     * \param   visible     True to make recent files visible, false otherwise.
     **/
    void setRecentFilesVisibility(bool visible);

    /**
     * \brief   Gets the active MDI child window.
     * \return  A pointer to the active MDI child window.
     **/
    MdiChild* activeMdiChild() const;

    /**
     * \brief   Finds an MDI child window by file name.
     * \param   fileName    The name of the file.
     * \return  A pointer to the MDI sub-window containing the file.
     **/
    QMdiSubWindow* findMdiChild(const QString& fileName) const;

    /**
     * \brief   Returns a reference to the current instance of MdiMainWindow.
     * \return  A reference to the current instance of MdiMainWindow.
     **/
    inline MdiMainWindow& self(void);

    /**
     * \brief   Initializes an action with the given parameters.
     * \param   act     The action to initialize.
     * \param   icon    The icon for the action.
     * \param   txt     The text for the action.
     **/
    inline void initAction(QAction& act, const QIcon & icon, QString txt);

//////////////////////////////////////////////////////////////////////////
// Member variables
    //////////////////////////////////////////////////////////////////////////
    /// \brief mWorkspaceRoot
    ///
private:
    //!< The root directory of the workspace.
    QString     mWorkspaceRoot;
    //!< The current file name.
    QString         mLastFile;       
    
    //!< The MDI area for managing sub-windows.
    MdiArea         mMdiArea;
    //!< The navigation dock widget.
    Navigation      mNavigation;
    //!< The status dock widget.
    QDockWidget*    mStatusDock;
    //!< The list view widget.
    QListView*      mListView;
    //!< The status tab widget.
    QTabWidget*     mStatusTabs;
    LogViewer*      mLogViewer;
    QMdiSubWindow*  mLiveLogWnd;
    //!< The file menu.
    QMenu*          mFileMenu;
    //!< The edit menu.
    QMenu*          mEditMenu;
    //!< The view menu.
    QMenu*          mViewMenu;
    //!< The design top level menu.
    QMenu*          mDesignMenu;
    //!< The design service interface menu
    QMenu*          mDesignMenuSI;
    //!< The top level menu "Tools"
    QMenu*          mToolsMenu;
    //!< The window menu.
    QMenu*          mWindowMenu;
    //!< The help menu.
    QMenu*          mHelpMenu;
    //!< The file toolbar.
    QToolBar*       mFileToolBar;
    //!< The edit toolbar.
    QToolBar*       mEditToolBar;
    //!< The view toolbar.
    QToolBar*       mViewToolBar;

    //!< Action for creating a new SI file.
    QAction         mActFileNewSI;
    //!< Action for creating a new log file.
    QAction         mActFileNewLog;
    //!< Action for opening file.
    QAction         mActFileOpen;
    //!< Action for saving the current file.
    QAction         mActFileSave;
    //!< Action for saving the current file with a new name.
    QAction         mActFileSaveAs;
    //!< Action for closing the current file.
    QAction         mActFileClose;
    //!< Action for closing all files.
    QAction         mActFileCloseAll;
    //!< Action for exiting the application.
    QAction         mActFileExit;

    //!< Action for cutting text.
    QAction         mActEditCut;
    //!< Action for copying text.
    QAction         mActEditCopy;
    //!< Action for pasting text.
    QAction         mActEditPaste;

    //!< View Workspace Explorer
    QAction         mActViewNavigator;
    QAction         mActViewWokspace;
    QAction         mActViewLogs;
    QAction         mActViewStatus;

    //!< Action for showing the options dialog.
    QAction         mActToolsOptions;

    //!< Action for tiling windows.
    QAction         mActWindowsTile;
    //!< Action for cascading windows.
    QAction         mActWindowsCascade;
    //!< Action for switching to the next window.
    QAction         mActWindowsNext;
    //!< Action for switching to the previous window.
    QAction         mActWindowsPrev;
    //!< Separator for the window menu.
    QAction         mActWindowMenuSeparator;
    //!< Action for showing the about dialog.
    QAction*        mActHelpAbout;
    //!< Submenu for recent files.
    QAction*        mActRecentFilesSubMenu;
    //!< Separator for the file menu.
    QAction*        mFileSeparator;
    //!< Actions for opening recent files.
    QAction*        mActsRecentFiles[MaxRecentFiles];
    
};

//////////////////////////////////////////////////////////////////////////
// MdiMainWindow class inline methods
//////////////////////////////////////////////////////////////////////////

inline void MdiMainWindow::setWorkspaceRoot(const QString& workspace)
{
    mWorkspaceRoot = workspace;
}

inline const QString& MdiMainWindow::getWorkspaceRoot(void) const
{
    return mWorkspaceRoot;
}

inline MdiMainWindow& MdiMainWindow::self(void)
{
    return (*this);
}

inline const QString MdiMainWindow::getLastFile(void) const
{
    return mLastFile;
}

inline void MdiMainWindow::setLastFile(const QString& lastFile)
{
    mLastFile = lastFile;
}

inline LogViewer* MdiMainWindow::getLiveLogViewer(void) const
{
    return mLogViewer;
}

#endif // LUSAN_VIEW_COMMON_MDIMAINWINDOW_HPP
