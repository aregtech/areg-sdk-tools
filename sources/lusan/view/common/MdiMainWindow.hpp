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
class OfflineLogViewer;
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
    friend class MdiChild;
    Q_OBJECT
    
//////////////////////////////////////////////////////////////////////////
// Hidden static methods.
//////////////////////////////////////////////////////////////////////////
private:
    
    //!< Returns the filters to display when opening service interface files.
    inline static QString _filterServiceFiles(void);
    
    //!< Returns the filter to display when opening log database files.
    inline static QString _filterLoggingFiles(void);
    
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
     * \brief   Call to notify the main window that application is connected to log collector service and is ready to receive log messages.
     * \param   isConnected   True if connected, false if disconnected.
     * \param   address       The IP address of the log collector service.
     * \param   port          The TCP port number of the log collector service.
     * \param   dbPath        The path to the database file, if any. If empty, no database is used.
     **/
    void logCollecttorConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath);

    /**
     * \brief   Call to notify the main window that the database is created.
     * \param   dbPath    The path to the database file.
     **/
    void logDatabaseCreated(const QString& dbPath);

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

    /**
     * \brief   Returns the active MDI child window.
     **/
    inline MdiChild* getActiveWindow(void) const;

    /**
     * \brief   Call to show the options dialog with active log setting page.
     * \param   address        The IP address of the log collector service.
     * \param   hostName       The host name of the log collector service. Can be same as `address`.
     * \param   port           The TCP port number of the log collector service.
     * \param   logFile        The name of the log file.
     * \param   logLocation    The directory where the log file is stored.
     * \return  The result of the dialog, 0 if canceled, 1 if OK pressed.
     **/
    int showOptionPageLogging(const QString& address, const QString& hostName, uint16_t port, const QString& logFile, const QString& logLocation);

    /**
     * \brief   Displays the dialog to pen log database files. Loads files and returns the path of the opened database.
     **/
    QString openLogFile(void);

    /**
     * \brief   Called to setup live logging models.
     **/
    void setupLiveLogging(void);
    
//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

/************************************************************************
 * Signals
 ************************************************************************/

    /**
     * \brief   The signal triggered when the MDI child window is activated.
     * \param   mdiChild    The MDI child window that is activated.
     **/
    void signalWindowActivated(MdiChild* child);

    /**
     * \brief   The signal triggered when the MDI child window is closed.
     * \param   mdiChild    The MDI child window that is closed.
     **/
    void signalWindowClosed(MdiChild* child);

    /**
     * \brief   The signal triggered when the MDI child window is created.
     * \param   mdiChild    The MDI child window that is created.
     **/
    void signalWindowCreated(MdiChild* child);

    /**
     * \brief   The signal triggered when the options dialog is opened.
     **/
    void signalOptionsOpening(void);

    /**
     * \brief   The signal triggered when the options dialog is applied.
     **/
    void signalOptionsApplied(void);

    /**
     * \brief   The signal triggered when the options dialog is closed.
     * \param   OKpressed   True if OK button was pressed, false if Cancel button was pressed.
     **/
    void signalOptionsClosed(bool OKpressed);

    /**
     * \brief   The signal triggered when the main window is about to close.
     *          This signal is used to notify other components that the main window is closing.
     **/
    void signalMainwindowClosing(void);
    
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
     * \brief   Slot for showing the workspace navigator.
     **/
    void onViewNavigator();

    /**
     * \brief   Slot for showing the workspace view.
     **/
    void onViewWorkspace();

    /**
     * \brief   Slot for showing the logs view.
     **/
    void onViewLogs();

    /**
     * \brief   Slot for showing the status view.
     **/
    void onViewStatus();

    /**
     * \brief   Slot for showing the tools options dialog.
     **/
    void onToolsOptions(void);

    /**
     * \brief   Slot for showing the about dialog.
     **/
    void onHelpAbout();

    /**
     * \brief   Updates the recent file actions.
     **/
    void onShowMenuRecent();

    /**
     * \brief   Updates the window menu.
     **/
    void onShowMenuWindow();

    /**
     * \brief   Slot for handling the MDI child window closed event.
     * \param   mdiChild    The MDI child window that is closed.
     **/
    void onMdiChildClosed(MdiChild *mdiChild);

    /**
     * \brief   Slot for handling the MDI child window when it is created.
     * \param   mdiChild    The MDI child window that is created.
     **/
    void onMdiChildCreated(MdiChild *mdiChild);

    /**
     * \brief   Slot for handling the MDI sub-window when it is activated.
     * \param   mdiSubWindow    The MDI sub-window that is activated.
     **/
    void onSubWindowActivated(QMdiSubWindow* mdiSubWindow);

private:

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

    /**
     * \brief   Creates a new Log Viewer View.
     * \param   filePath   The path to the log file to view.
     * \return  A pointer to the new Log Viewer View.
     **/
    LogViewer* createLogViewerView(const QString& filePath = QString());

    /**
     * \brief   Creates a new Offline Log Viewer View for .sqlog files.
     * \param   filePath   The path to the .sqlog file to view.
     * \param   cloneLive  If true, the data from live log viewer is cloned, otherwise a new instance is created.
     * \return  A pointer to the new Offline Log Viewer View.
     **/
    OfflineLogViewer* createOfflineLogViewer(const QString& filePath = QString(), bool cloneLive = false);

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
     * \brief   Creates the tool-bars for the main window.
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
     * \brief   Appends a file to the recent files list.
     * \param   fileName    The name of the file to append.
     **/
    void appendToRecentFiles(const QString& fileName);

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
private:
    QString         mWorkspaceRoot; //!< The root directory of the workspace.
    QString         mLastFile;      //!< The current file name.
    
    MdiArea         mMdiArea;       //!< The MDI area for managing sub-windows.
    Navigation      mNavigation;    //!< The navigation dock widget.
    QDockWidget*    mStatusDock;    //!< The status dock widget.
    QListView*      mListView;      //!< The list view widget.
    QTabWidget*     mStatusTabs;    //!< The status tab widget.
    LogViewer*      mLogViewer;     //!< The log viewer for displaying live logs. There should be only one instance of this viewer.
    QMdiSubWindow*  mLiveLogWnd;    //!< The MDI sub-window for the live log viewer. There should be only one instance of this window.

    QMenu*          mFileMenu;      //!< The file menu.
    QMenu*          mEditMenu;      //!< The edit menu.
    QMenu*          mViewMenu;      //!< The view menu.
    QMenu*          mDesignMenu;    //!< The design top level menu.
    QMenu*          mLoggingMenu;   //!< The logging menu.
    QMenu*          mToolsMenu;     //!< The top level menu "Tools"
    QMenu*          mWindowMenu;    //!< The window menu.
    QMenu*          mHelpMenu;      //!< The help menu.

    QToolBar*       mFileToolBar;   //!< The file toolbar.
    QToolBar*       mEditToolBar;   //!< The edit toolbar.
    QToolBar*       mViewToolBar;   //!< The view toolbar.

    //!< Actions for File sub-menus.
    QAction         mActFileNewSI;
    QAction         mActFileNewLog;
    QAction         mActFileOpen;
    QAction         mActFileSave;
    QAction         mActFileSaveAs;
    QAction         mActFileClose;
    QAction         mActFileCloseAll;
    QAction         mActFileExit;
    QAction*        mFileSeparator;
    QAction*        mActFileRecent;

    //!< Actions for Edit sub-menus.
    QAction         mActEditCut;
    QAction         mActEditCopy;
    QAction         mActEditPaste;

    //!< Actions for View sub-menus.
    QAction         mActViewNavigator;
    QAction         mActViewWokspace;
    QAction         mActViewLogs;
    QAction         mActViewStatus;

    //!< Actions for Tools sub-menus.
    QAction         mActToolsOptions;

    //!< Actions for Window sub-menus.
    QAction         mActWindowsTile;
    QAction         mActWindowsCascade;
    QAction         mActWindowsNext;
    QAction         mActWindowsPrev;
    QAction         mActWindowMenuSeparator;

    //!< Actions for Help sub-menus.
    QAction*        mActHelpAbout;

    QAction*        mActsRecentFiles[MaxRecentFiles];   //!< Actions for opening recent files.    
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

inline MdiChild* MdiMainWindow::getActiveWindow(void) const
{
    return activeMdiChild();
}

#endif // LUSAN_VIEW_COMMON_MDIMAINWINDOW_HPP
