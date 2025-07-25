﻿#ifndef LUSAN_VIEW_COMMON_NAVIGATIONDOCK_HPP
#define LUSAN_VIEW_COMMON_NAVIGATIONDOCK_HPP
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
 *  \file        lusan/view/common/NavigationDock.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The navigation docking widget of lusan.
 *
 ************************************************************************/

#include "lusan/view/common/NavigationWindow.hpp"
#include "lusan/view/common/NaviFileSystem.hpp"
#include "lusan/view/common/NaviLiveLogsScopes.hpp"
#include "lusan/view/common/NaviOfflineLogsScopes.hpp"

#include <QDockWidget>
#include <QIcon>
#include <QSize>
#include <QTabWidget>
#include "OutputDock.hpp"

class MdiMainWindow;

class NavigationDock : public QDockWidget
{
//////////////////////////////////////////////////////////////////////////
// Constants, types and static methods
//////////////////////////////////////////////////////////////////////////
public:

    //!< The enumeration of the navigation window types.
    enum eNaviWindow
    {
          NaviUnknown       = 0 //!< Unknown navigation window type
        , NaviWorkspace         //!< Workspace navigation window type
        , NaviLiveLogs          //!< Live logs navigation window type
        , NaviOfflineLogs       //!< Offline logs navigation window type
    };

    static QString  TabNameFileSystem;      //!< The name of the tab for workspace explorer.
    static QString  TabLiveLogsExplorer;    //!< The name of the tab for live logs explorer.
    static QString  TabOfflineLogsExplorer; //!< The name of the tab for offline logs explorer.
    static QSize    IconSize;               //<! The size of icons.

    //!< Returns the icon for the workspace explorer tab.
    static QIcon getWorkspaceExplorerIcon(void);

    //!< Returns the icon for the live logs explorer tab.
    static QIcon getLiveLogIcon(void);

    //!< Returns the icon for the offline logs explorer tab.
    static QIcon getOfflineLogIcon(void);

    //!< Returns the tab name of the specified navigation window
    static const QString& getTabName(NavigationDock::eNaviWindow navi);
    
    //!< Returns the navigation window type by specified tab name.
    static NavigationDock::eNaviWindow getNaviWindow(const QString & tabName);
    
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    NavigationDock(MdiMainWindow* parent);
    
//////////////////////////////////////////////////////////////////////////
// Actions and attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the tab widget of the navigation.
     **/
    inline QTabWidget& getTabWidget(void);

    /**
     * \brief   Returns the file system widget.
     **/
    inline NaviFileSystem& getFileSystem(void);

    /**
     * \brief   Returns the live mode log explorer widget.
     **/
    inline NaviLiveLogsScopes& getLiveScopes(void);

    /**
     * \brief   Returns the offline log explorer widget.
     **/
    inline NaviOfflineLogsScopes& getOfflineScopes(void);

    /**
     * \brief   Adds a new tab with the widget to the tab-control.
     * \param   widget      The widget to add to the tab-control.
     * \brief   tabName     The name of the tab to add.
     * \return  The index of the new added tab.
     **/
    inline int addTab(NavigationWindow& widget, const QString& tabName);
    inline int addTab(NavigationWindow& widget, NavigationDock::eNaviWindow navi);

    /**
     * \brief   Returns the pointer to the widget of the given tab name.
     *          Returns nullptr if the name does not exist.
     * \param   tabName     The name of tab to return the widget.
     * \return  Returns the valid pointer of the Widget of the given tab name.
     *          Returns nullptr if tab name does not exist.
     **/
    NavigationWindow* getTab(const QString& tabName) const;
    NavigationWindow* getTab(NavigationDock::eNaviWindow navi) const;
    

    /**
     * \brief   Check if the tab with the given name exists.
     * @param   tabName     The name of the tab to check.
     * @return  Returns true if the tab with the given name exists. False, otherwise.
     **/
    bool tabExists(const QString& tabName) const;
    bool tabExists(NavigationDock::eNaviWindow navi) const;

    /**
     * \brief   Show tab with specified unique name.
     * \param   tabName     The unique name of the tab to show.
     **/
    bool showTab(const QString& tabName);
    bool showTab(NavigationDock::eNaviWindow navi);
    
//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Returns the instance of NavigationDock window.
     **/
    inline NavigationDock& self(void);

    /**
     * \brief   Initializes the size of tab widgets.
     **/
    void initSize();

private slots:

    /**
     * \brief   Slot is triggered when options dialog is opened.
     **/
    void onOptionsOpening(void);

    /**
     * \brief   Slot is triggered when apply button in options dialog is pressed.
     **/
    void onOptionsApplied(void);

    /**
     * \brief   Slot is triggered when options dialog is closed.
     * \param   pressedOK   If true, OK button was pressed. Otherwise, Cancel button was pressed.
     **/
    void onOptionsClosed(bool pressedOK);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    MdiMainWindow*          mMainWindow;    //!< Main window
    QTabWidget              mTabs;          //!< The tab widget of the navigation.
    NaviLiveLogsScopes      mLiveScopes;    //!< The log explorer widget.
    NaviOfflineLogsScopes   mOfflineScopes; //!< The offline scopes explorer.
    NaviFileSystem          mFileSystem;    //!< The file system widget.
};

//////////////////////////////////////////////////////////////////////////
// NavigationDock class inline methods
//////////////////////////////////////////////////////////////////////////

inline QTabWidget& NavigationDock::getTabWidget(void)
{
    return mTabs;
}

inline NaviFileSystem& NavigationDock::getFileSystem(void)
{
    return mFileSystem;
}

inline NaviLiveLogsScopes& NavigationDock::getLiveScopes(void)
{
    return mLiveScopes;
}

inline NaviOfflineLogsScopes& NavigationDock::getOfflineScopes(void)
{
    return mOfflineScopes;
}

inline int NavigationDock::addTab(NavigationWindow& widget, const QString& tabName)
{
    return mTabs.addTab(&widget, tabName);
}

inline int NavigationDock::addTab(NavigationWindow& widget, NavigationDock::eNaviWindow navi)
{
    return (navi != NavigationDock::eNaviWindow::NaviUnknown ? addTab(widget, NavigationDock::getTabName(navi)) : -1);
}

inline NavigationDock& NavigationDock::self(void)
{
    return (*this);
}

#endif  // LUSAN_VIEW_COMMON_NAVIGATIONDOCK_HPP
