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

#include <QList>
#include <QSize>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class MdiMainWindow;
class QHBoxLayout;
class QIcon;
class QResizeEvent;
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
     * \brief   Marks an entry of the tool button row as one that must stay in the row at
     *          every width. The row gives up its entries from the right, so marking an entry
     *          also keeps every entry before it.
     * \param   widget  An entry already added to the row.
     **/
    void setToolFixed(QWidget* widget);

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
     * \brief   Sets one square icon size on every tool button of this window, including the
     *          buttons a derived class created on its own.
     * \param   iconExtent  The square icon edge in logical pixels.
     **/
    void capToolButtonIconSizes(int iconExtent = NAVI_TOOL_ICON);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual void resizeEvent(QResizeEvent* event) override;

    virtual bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////
public:
    //!< The icon edge every navigation panel tool button carries.
    static constexpr int NAVI_TOOL_ICON         { 16 };

    //!< The air the tool row keeps above and below its buttons.
    static constexpr int NAVI_TOOL_AIR          { 1 };

    //!< The gap the tool row keeps between two entries.
    static constexpr int NAVI_TOOL_GAP          { 1 };

    //!< The air an input row keeps around one line of text.
    static constexpr int NAVI_INPUT_AIR         { 3 };

    /**
     * \brief   Returns the height one navigation tool button takes at the current style
     *          and font. Every panel gives its tool row the same one.
     **/
    static int toolButtonHeight(void);

    /**
     * \brief   Returns the width one navigation tool button takes at the current style and
     *          font. A widget placed in the row takes it as its own cell width, so the row
     *          reads as one grid instead of a button run beside a wider stranger.
     **/
    static int toolButtonWidth(void);

    /**
     * \brief   Returns the height of the tool row: one tool button with a pixel of air
     *          above and below it.
     **/
    static int toolRowHeight(void);

    /**
     * \brief   Returns the height of an input row placed under the tool row. The workspace
     *          selector and the scope filter both take it, so the tree below starts on the
     *          same line whichever navigation panel the user is on.
     * \param   owner   The widget the row is built for. Its font sets the height.
     **/
    static int naviInputHeight(const QWidget& owner);

//////////////////////////////////////////////////////////////////////////
// Internal types and methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< One entry of the tool button row.
    struct sToolItem
    {
        QWidget*    widget  { nullptr };     //!< The button, separator or custom widget.
        bool        fixed   { false };       //!< True if the entry must stay in the row.
        bool        divider { false };       //!< True for a separator.
    };

    /**
     * \brief   Distributes the entries between the row and the overflow for the current width.
     **/
    void updateToolOverflow(void);

    /**
     * \brief   Shows the overflow entries as a second row under the chevron.
     **/
    void showToolOverflow(void);

    /**
     * \brief   Returns the entries back to the row and closes the second row.
     **/
    void closeToolOverflow(void);

    /**
     * \brief   Hides a separator that has no visible entry after it.
     **/
    void _hideDanglingSeparators(void);

    /**
     * \brief   Gives a tool row its margins and its gap. The row under the chevron takes the
     *          same ones as the row itself, so an entry does not change size when it moves.
     * \param   row     The layout of the row to measure.
     **/
    static void _applyRowMetrics(QHBoxLayout& row);

    /**
     * \brief   Puts an entry at the end of the tool row, before the slack that holds the
     *          overflow button against the right edge.
     * \param   widget  The entry to place.
     **/
    void _insertTool(QWidget* widget);

    /**
     * \brief   Returns the size hint of a tool button built the way addToolButton() builds
     *          one, which is what the row measures itself against.
     **/
    static QSize _toolButtonHint(void);

    /**
     * \brief   Returns the width the overflow button is held to. Its mark is three dots on
     *          one line, so it needs less room than a button carrying a full icon.
     **/
    static int _overflowWidth(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QVBoxLayout*        mNaviLayout;    //!< The layout that stacks the tool button row and the tree view.
    QWidget*            mToolbar;       //!< The widget that holds the row of tool buttons.
    QHBoxLayout*        mToolLayout;    //!< The layout of the tool button row.
    QTreeView*          mNaviTree;      //!< The tree view of the navigation window.
    QToolButton*        mToolOverflow;  //!< The chevron that opens the entries the row cannot fit.
    QWidget*            mOverflowRow;   //!< The second row shown by the chevron.
    QHBoxLayout*        mOverflowLayout;//!< The layout of the second row.
    QList<sToolItem>    mToolItems;     //!< Every entry of the row, in the order it was added.
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
