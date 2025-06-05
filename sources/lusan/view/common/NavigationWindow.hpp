#ifndef LUSAN_VIEW_COMMON_NAVIGATIONWINDOW_HPP
#define LUSAN_VIEW_COMMON_NAVIGATIONWINDOW_HPP
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
 *  \file        lusan/view/common/NavigationWindow.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The navigation windows elements.
 *
 ************************************************************************/

#include <QWidget>

class MdiMainWindow;

class NavigationWindow : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constants, types and static methods
//////////////////////////////////////////////////////////////////////////
public:

    //!< The enumeration of the navigation window types.
    enum eNavigationWindow
    {
          NaviUnknown       = 0 //!< Unknown navigation window type
        , NaviWorkspace         //!< Workspace navigation window type
        , NaviLiveLogs          //!< Live logs navigation window type
        , NaviOfflineLogs       //!< Offline logs navigation window type
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Constructor for NavigationWindow.
     * \param   naviWindow   The type of the navigation window.
     * \param   wndMain      Pointer to the main MDI window.
     * \param   parent       Pointer to the parent widget.
     **/
    NavigationWindow(eNavigationWindow naviWindow, MdiMainWindow* wndMain, QWidget* parent = nullptr);

    virtual ~NavigationWindow(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the type of the navigation window.
     * \return  The type of the navigation window.
     **/
    inline NavigationWindow::eNavigationWindow getNaviWindowType(void) const;

    /**
     * \brief   Checks if the navigation window is a workspace.
     * \return  True if the navigation window is a workspace, false otherwise.
     **/
    inline bool isNaviWorkspace(void) const;

    /**
     * \brief   Checks if the navigation window is for live logs.
     * \return  True if the navigation window is for live logs, false otherwise.
     **/
    inline bool isNaviLiveLogs(void) const;

    /**
     * \brief   Checks if the navigation window is for offline logs.
     * \return  True if the navigation window is for offline logs, false otherwise.
     **/
    inline bool isNaviOfflineLogs(void) const;

//////////////////////////////////////////////////////////////////////////
// NavigationWindow class inline methods
//////////////////////////////////////////////////////////////////////////
protected:

    const eNavigationWindow mNaviWindowType;    //!< The type of the navigation window
    MdiMainWindow*          mMainWindow;        //!< Pointer to the main MDI window
};

//////////////////////////////////////////////////////////////////////////
// NavigationWindow class inline methods
//////////////////////////////////////////////////////////////////////////

inline NavigationWindow::eNavigationWindow NavigationWindow::getNaviWindowType(void) const
{
    return mNaviWindowType;
}

inline bool NavigationWindow::isNaviWorkspace(void) const
{
    return (mNaviWindowType == eNavigationWindow::NaviWorkspace);
}

inline bool NavigationWindow::isNaviLiveLogs(void) const
{
    return (mNaviWindowType == eNavigationWindow::NaviLiveLogs);
}

inline bool NavigationWindow::isNaviOfflineLogs(void) const
{
    return (mNaviWindowType == eNavigationWindow::NaviOfflineLogs);
}

#endif  // LUSAN_VIEW_COMMON_NAVIGATIONWINDOW_HPP
