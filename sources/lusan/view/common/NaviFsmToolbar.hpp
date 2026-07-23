#ifndef LUSAN_VIEW_COMMON_NAVIFSMTOOLBAR_HPP
#define LUSAN_VIEW_COMMON_NAVIFSMTOOLBAR_HPP
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
 *  \file        lusan/view/common/NaviFsmToolbar.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The FSM design toolbar navigation window.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/NavigationWindow.hpp"

#include <QHash>
#include <QList>
#include <QPointer>
#include <QSet>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class MdiMainWindow;
class SMDesign;
class QAction;
class QScrollArea;
class QToolButton;
class QVBoxLayout;
class QWidget;

/**
 * \class   NaviFsmToolbar
 * \brief   The navigation-dock tab hosting the State Machine designer's drawing toolbar.
 *          The toolbar is per-document (its actions live on each State Machine's Design
 *          page), so this tab reflects whichever State Machine window is currently active:
 *          it re-binds to the active document's Design page on activation. Without an
 *          active State Machine it shows the same buttons as disabled stand-ins, so the
 *          toolbar always presents its full tool set (issue #514).
 *
 *          The commands are laid out as collapsible, scrollable groups ordered by
 *          importance (Design, Declare, Alignment, Navigate, Color, Grid, Edit, Zoom). The
 *          toolbutton display mode (icon only / text only / icon and text) is an
 *          application-wide preference set from the View menu (default icon only): with text
 *          the buttons are stacked as left-aligned rows; icon-only they flow into 2-3
 *          columns. The tools stay visible whenever a State Machine is active but are only
 *          enabled while its Design page is the current inner tab.
 **/
class NaviFsmToolbar : public NavigationWindow
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit NaviFsmToolbar(MdiMainWindow* wndMain, QWidget* parent = nullptr);
    virtual ~NaviFsmToolbar() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Binds the toolbar to the given Design page and re-populates it with that
     *          page's grouped action set; nullptr shows the disabled stand-in buttons.
     **/
    void bindDesign(SMDesign* design);

    /**
     * \brief   Sets the toolbutton display style (Qt::ToolButtonIconOnly / TextOnly /
     *          TextUnderIcon / TextBesideIcon) and rebuilds the layout to match.
     **/
    void setDisplayStyle(Qt::ToolButtonStyle style);

    /**
     * \brief   The current toolbutton display style.
     **/
    Qt::ToolButtonStyle getDisplayStyle() const;

    /**
     * \brief   Enables or disables the whole tool set. The tools remain visible (so the user
     *          can see what is available) but only act while the Design page is current.
     **/
    void setToolsActive(bool active);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Arms a placement tool sticky when its button is double-clicked.
     **/
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Rebuilds the group sections from the bound Design page (or from the disabled
     *          stand-in action set when no page is bound).
     **/
    void rebuild();

    /**
     * \brief   Builds one collapsible group section with the given title and actions.
     **/
    QWidget* buildGroup(const QString& title, const QList<QAction*>& actions);

    /**
     * \brief   Builds one tool button bound to the given action, wiring the sticky-tool
     *          double-click when the action is a placement tool.
     **/
    QToolButton* buildToolButton(QAction* action);

    /**
     * \brief   True when the current display style shows text (row layout), false for icon-only.
     **/
    bool showsText() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QScrollArea*                mScroll;        //!< Scrolls the group sections when they overflow.
    QWidget*                    mGroups;        //!< The container of the collapsible group sections.
    QVBoxLayout*                mGroupsLayout;  //!< The vertical layout of the group sections.
    QObject*                    mFallback;      //!< Owns the disabled stand-in actions shown while unbound.
    QPointer<SMDesign>          mBound;         //!< The currently bound Design page, if any.
    bool                        mBuilt;         //!< The sections were built at least once (guards the re-bind no-op).
    Qt::ToolButtonStyle         mStyle;         //!< The toolbutton display style.
    bool                        mActive;        //!< Whether the tools are currently enabled.
    QHash<QObject*, int>        mStickyButtons; //!< Placement-tool button to its eCanvasTool (as int).
    QSet<QString>               mCollapsed;     //!< Titles of the groups the user collapsed.
};

#endif  // LUSAN_VIEW_COMMON_NAVIFSMTOOLBAR_HPP
