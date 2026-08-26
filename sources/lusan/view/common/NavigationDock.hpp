#ifndef LUSAN_VIEW_COMMON_NAVIGATIONDOCK_HPP
#define LUSAN_VIEW_COMMON_NAVIGATIONDOCK_HPP
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
 *  \file        lusan/view/common/NavigationDock.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The navigation docking widget of lusan.
 *
 ************************************************************************/

#include "lusan/view/common/NavigationWindow.hpp"
#include "lusan/view/common/NaviFileSystem.hpp"
#include "lusan/view/common/NaviLiveLogsScopes.hpp"
#include "lusan/view/common/NaviOfflineLogsScopes.hpp"
#include "lusan/view/common/NaviTabRail.hpp"

#include <QElapsedTimer>
#include <QHash>
#include <QKeySequence>
#include <QSize>
#include <QStackedWidget>
#include <QWidget>

class MdiMainWindow;
class QHBoxLayout;
class QLabel;

/**
 * \brief   The body of the navigation panel: a rail of icons on the window-facing edge
 *          picks which navigator fills the rest. It is a plain content widget hosted in
 *          a Qt-Advanced-Docking-System dock widget, which provides the title bar,
 *          floating and cross-window drag behavior.
 **/
class NavigationDock : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constants, types and static methods
//////////////////////////////////////////////////////////////////////////
public:

    //!< The navigators of the panel. The order is also the order of the rail.
    enum eNaviWindow
    {
          NaviUnknown       = 0 //!< Unknown navigation window type
        , NaviWorkspace         //!< Workspace navigation window type
        , NaviLiveLogs          //!< Live logs navigation window type
        , NaviOfflineLogs       //!< Offline logs navigation window type
        , NaviDesignToolbar     //!< FSM design toolbar navigation window type
        , NaviDesignProperties  //!< FSM design Properties panel navigation window type
        , NaviDesignOutline     //!< FSM design Outline panel navigation window type
    };

    /**
     * \brief   Returns the display name of the navigator. It is translated on every call,
     *          so it must never be used as the identity of a panel; the enumeration is.
     **/
    static QString panelName(NavigationDock::eNaviWindow navi);

    /**
     * \brief   Returns the one line description of the navigator, shown in tool tips.
     **/
    static QString panelHint(NavigationDock::eNaviWindow navi);

    /**
     * \brief   Returns the resource path of the monochrome rail icon of the navigator.
     **/
    static QString panelIcon(NavigationDock::eNaviWindow navi);

    /**
     * \brief   Returns the key sequence that brings the navigator up.
     **/
    static QKeySequence panelShortcut(NavigationDock::eNaviWindow navi);

    /**
     * \brief   True for the three design widgets that the Design page may lend to the panel.
     **/
    static bool isDesignPanel(NavigationDock::eNaviWindow navi);

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
     * \brief   Returns the navigator of the given type, or nullptr when it is not hosted here.
     **/
    NavigationWindow* getPanel(NavigationDock::eNaviWindow navi) const;

    /**
     * \brief   True when the given navigator is hosted in the panel, visible or not.
     **/
    bool hasPanel(NavigationDock::eNaviWindow navi) const;

    /**
     * \brief   Brings the given navigator to the front, showing it again when the user had
     *          hidden it. Returns false when the navigator is not hosted in the panel.
     **/
    bool showPanel(NavigationDock::eNaviWindow navi);

    /**
     * \brief   Returns the navigator shown at the moment, NaviUnknown when the panel is empty.
     **/
    NavigationDock::eNaviWindow currentPanel(void) const;

    /**
     * \brief   Adds (if absent) the movable design widget to the panel and gives it a rail
     *          entry. The content stays owned by the main window. The current navigator is
     *          left alone, so a document re-sync does not steal the panel from the user.
     * \param   navi        One of NaviDesignToolbar / NaviDesignProperties / NaviDesignOutline.
     * \param   content     The navigation window to host.
     **/
    void showDesignPanel(NavigationDock::eNaviWindow navi, NavigationWindow* content);

    /**
     * \brief   Drops the design widget from the panel, detaching its content instead of
     *          deleting it, so the Design page can take it back.
     **/
    void hideDesignPanel(NavigationDock::eNaviWindow navi);

    /**
     * \brief   True when the given design widget is hosted in the panel.
     **/
    bool isDesignPanelShown(NavigationDock::eNaviWindow navi) const;

    /**
     * \brief   Shows or hides a navigator without dropping it, backing the View menu.
     **/
    void setPanelVisible(NavigationDock::eNaviWindow navi, bool visible);

    /**
     * \brief   True when the navigator is hosted in the panel and drawn in the rail.
     **/
    bool isPanelVisible(NavigationDock::eNaviWindow navi) const;

    /**
     * \brief   Marks the Live Logs rail entry while a log source is connected.
     **/
    void setLiveLogsConnected(bool connected);

    /**
     * \brief   Turns the rail captions on or off and remembers the choice.
     **/
    void setRailLabels(bool labels);

    /**
     * \brief   True when the rail is set to draw captions under the icons.
     **/
    bool railLabels(void) const;

    /**
     * \brief   Hides the navigator body and leaves the rail alone, or brings the body back.
     **/
    void setContentCollapsed(bool collapsed);

    /**
     * \brief   True while the panel shows the rail without a navigator body.
     **/
    inline bool isContentCollapsed(void) const;

    /**
     * \brief   Returns the width of the rail, which is the width of a collapsed panel.
     **/
    inline int railWidth(void) const;

signals:

    /**
     * \brief   The navigator filling the panel changed.
     **/
    void signalPanelChanged(NavigationDock::eNaviWindow navi);

    /**
     * \brief   The user asked to close the whole navigation panel.
     **/
    void signalCollapseRequested(void);

    /**
     * \brief   The navigator body was hidden or brought back.
     **/
    void signalContentCollapsed(bool collapsed);

    /**
     * \brief   The user dropped a design widget from the rail context menu.
     **/
    void signalDesignPanelHidden(NavigationDock::eNaviWindow navi);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void moveEvent(QMoveEvent* event) override;
    virtual void showEvent(QShowEvent* event) override;
    virtual QSize sizeHint(void) const override;
    virtual QSize minimumSizeHint(void) const override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Puts a navigator in the rail and in the page stack.
     **/
    void registerPanel(NavigationDock::eNaviWindow navi, NavigationWindow* content);

    /**
     * \brief   Makes the given navigator fill the panel and tells the owner about it.
     **/
    void setCurrentPanel(NavigationDock::eNaviWindow navi);

    /**
     * \brief   Picks the navigator to show when the current one leaves or is hidden.
     **/
    void selectFallbackPanel(void);

    /**
     * \brief   Initializes the size of the panel.
     **/
    void initSize(void);

    /**
     * \brief   Puts the rail on the panel edge that faces the window frame.
     **/
    void updateRailSide(void);

    /**
     * \brief   True shortly after a click opened the panel, used to swallow the double-click
     *          that the same gesture produces.
     **/
    bool justExpanded(void) const;

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
    QHBoxLayout*            mLayout;        //!< Places the rail and the page stack side by side.
    NaviTabRail*            mRail;          //!< The icon strip that picks the navigator.
    QStackedWidget*         mStack;         //!< The pages of the hosted navigators.
    QLabel*                 mEmptyHint;     //!< The page shown while every navigator is hidden.
    NaviLiveLogsScopes      mLiveScopes;    //!< The log explorer widget.
    NaviOfflineLogsScopes   mOfflineScopes; //!< The offline scopes explorer.
    NaviFileSystem          mFileSystem;    //!< The file system widget.
    QHash<int, NavigationWindow*> mPanels;  //!< The hosted navigators by navigator type.
    bool                    mRailOrdered;   //!< False until the rail side is applied the first time.
    bool                    mCollapsed;     //!< True while only the rail is shown.
    QElapsedTimer           mExpandStamp;   //!< Marks the last time a click opened the panel.
};

//////////////////////////////////////////////////////////////////////////
// NavigationDock class inline methods
//////////////////////////////////////////////////////////////////////////

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

inline bool NavigationDock::isContentCollapsed(void) const
{
    return mCollapsed;
}

inline int NavigationDock::railWidth(void) const
{
    return mRail->width();
}

#endif  // LUSAN_VIEW_COMMON_NAVIGATIONDOCK_HPP
