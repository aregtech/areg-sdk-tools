#ifndef LUSAN_VIEW_COMMON_NAVITOOLBARWINDOW_HPP
#define LUSAN_VIEW_COMMON_NAVITOOLBARWINDOW_HPP
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
 *  \file        lusan/view/common/NaviToolbarWindow.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The base of navigation windows built of a tool button row and a tree view.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/view/common/NavigationWindow.hpp"

#include <QSize>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class MdiMainWindow;
class QHBoxLayout;
class QIcon;
class QToolButton;
class QTreeView;
class QVBoxLayout;
class QWidget;

//////////////////////////////////////////////////////////////////////////
// NaviToolbarWindow class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The base class of the navigation windows that show a row of tool buttons
 *          above a tree view. It creates and owns both controls, and offers the
 *          helpers to fill the tool button row.
 **/
class NaviToolbarWindow : public NavigationWindow
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the tool button row and the tree view of the navigation window.
     * \param   naviWindow  The type of the navigation window.
     * \param   wndMain     The main window of the application.
     * \param   parent      The parent widget.
     **/
    NaviToolbarWindow(int naviWindow, MdiMainWindow* wndMain, QWidget* parent = nullptr);

    virtual ~NaviToolbarWindow(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the tree view of the navigation window.
     **/
    inline QTreeView* ctrlTable(void) const;

    /**
     * \brief   Returns the widget that holds the row of tool buttons.
     **/
    inline QWidget* ctrlToolbar(void) const;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Appends a tool button to the end of the tool button row.
     * \param   icon        The icon of the tool button.
     * \param   toolTip     The tool tip shown when the mouse rests on the button.
     * \param   statusTip   The text shown in the status bar of the main window.
     * \param   checkable   If true, the button keeps the checked state.
     * \return  The created tool button, owned by the tool button row.
     **/
    QToolButton* addToolButton(const QIcon& icon, const QString& toolTip, const QString& statusTip, bool checkable = false);

    /**
     * \brief   Appends a vertical separator to the end of the tool button row.
     **/
    void addToolSeparator(void);

    /**
     * \brief   Puts the given widget in the tool button row, after everything added so far.
     * \param   widget  The widget to place. It is reparented to the row.
     **/
    void addToolWidget(QWidget* widget);

    /**
     * \brief   Places a widget above the tool button row and takes its ownership.
     * \param   header  The widget to show on top of the navigation window.
     **/
    void setNaviHeader(QWidget* header);

    /**
     * \brief   Places a widget between the tool button row and the tree view, after every
     *          bar added so far, and takes its ownership.
     * \param   bar     The widget to show above the tree.
     **/
    void addNaviBar(QWidget* bar);

    /**
     * \brief   Applies the tree view setup shared by the navigation windows.
     * \param   iconSize    The size of the icons drawn in the tree items.
     **/
    void setupTreeView(const QSize& iconSize);

    /**
     * \brief   Sets one square icon size on every tool button of this window.
     *          Themed icons come in different aspects and are clipped by the fixed row height
     *          unless they share one edge. Call it after the tool buttons are created.
     * \param   iconExtent  The square icon edge in logical pixels.
     **/
    void capToolButtonIconSizes(int iconExtent = NAVI_TOOL_ICON_SMALL);

//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////
public:
    //!< The icon edge a navigation panel tool button carries.
    static constexpr int NAVI_TOOL_ICON         { 18 };

    //!< The icon edge of the panels that pack their tool buttons tighter.
    static constexpr int NAVI_TOOL_ICON_SMALL   { 16 };

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QVBoxLayout*    mNaviLayout;    //!< The layout that stacks the tool button row and the tree view.
    QWidget*        mToolbar;       //!< The widget that holds the row of tool buttons.
    QHBoxLayout*    mToolLayout;    //!< The layout of the tool button row.
    QTreeView*      mNaviTree;      //!< The tree view of the navigation window.
};

//////////////////////////////////////////////////////////////////////////
// NaviToolbarWindow class inline methods
//////////////////////////////////////////////////////////////////////////

inline QTreeView* NaviToolbarWindow::ctrlTable(void) const
{
    return mNaviTree;
}

inline QWidget* NaviToolbarWindow::ctrlToolbar(void) const
{
    return mToolbar;
}

#endif  // LUSAN_VIEW_COMMON_NAVITOOLBARWINDOW_HPP
