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

/**
 * \brief   Base class for navigation windows in the Lusan application.
 **/
class NavigationWindow : public QWidget
{
    Q_OBJECT

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
    NavigationWindow(int naviWindow, MdiMainWindow* wndMain, QWidget* parent = nullptr);

    virtual ~NavigationWindow(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   This method is called when the options dialog is opened.
     **/
    virtual void optionOpenning(void);

    /**
     * \brief   This method is called when the apply button in options dialog is pressed.
     *          It can be used to apply changes made in the options dialog.
     **/
    virtual void optionApplied(void);

    /**
     * \brief   This method is called when the options dialog is closed.
     * \param   OKpressed   True if OK button was pressed, false if Cancel button was pressed.
     **/
    virtual void optionClosed(bool OKpressed);

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the type of the navigation window.
     * \return  The type of the navigation window.
     **/
    inline int getNaviWindowType(void) const;

    /**
     * \brief   Checks if the navigation window is a workspace.
     * \return  True if the navigation window is a workspace, false otherwise.
     **/
    bool isNaviWorkspace(void) const;

    /**
     * \brief   Checks if the navigation window is for live logs.
     * \return  True if the navigation window is for live logs, false otherwise.
     **/
    bool isNaviLiveLogs(void) const;

    /**
     * \brief   Checks if the navigation window is for offline logs.
     * \return  True if the navigation window is for offline logs, false otherwise.
     **/
    bool isNaviOfflineLogs(void) const;

//////////////////////////////////////////////////////////////////////////
// NavigationWindow class inline methods
//////////////////////////////////////////////////////////////////////////
protected:

    const int       mNaviWindowType;    //!< The type of the navigation window
    MdiMainWindow*  mMainWindow;        //!< Pointer to the main MDI window
};

//////////////////////////////////////////////////////////////////////////
// NavigationWindow class inline methods
//////////////////////////////////////////////////////////////////////////

inline int NavigationWindow::getNaviWindowType(void) const
{
    return mNaviWindowType;
}

#endif  // LUSAN_VIEW_COMMON_NAVIGATIONWINDOW_HPP
