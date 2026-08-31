#ifndef LUSAN_VIEW_COMMON_NAVILOGSCOPEBASE_HPP
#define LUSAN_VIEW_COMMON_NAVILOGSCOPEBASE_HPP
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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/common/NaviLogScopeBase.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The base class of the log explorer view.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/view/common/NaviToolbarWindow.hpp"
#include "areg/logging/areg_log.h"

#include <QList>
#include <QPersistentModelIndex>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LoggingScopesModelBase;
class LoggingModelBase;
class LogPriorityBar;
class MdiMainWindow;
class ScopeNameDelegate;
class ScopeNodeBase;
class SearchLineEdit;
class QAction;
class QEvent;
class QLabel;
class QLineEdit;
class QMenu;
class QItemSelectionModel;
class QModelIndex;
class QPoint;
class QToolButton;

//////////////////////////////////////////////////////////////////////////
// NaviLogScopeBase class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The base class of the live and offline log scope explorers. It builds the tool
 *          button row shared by both explorers, keeps the scope tree in sync with the scope
 *          model, and shows the context menu of the scope tree.
 **/
class NaviLogScopeBase : public NaviToolbarWindow
{
//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////
public:
    //!< The icon edge of a scope tree node. The charger is drawn to be read at this size.
    static constexpr int    ScopeIconExtent { 16 };

    //!< How long a raise that goes back on its own stays in effect.
    static constexpr int    TempRaiseMs     { 5 * 60 * 1000 };

    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
protected:

    //!< The entries of the scope tree context menu. The value travels in QAction::data().
    enum eScopeMenu
    {
          MenuNone          = 0 //!< No entry
        , MenuLevelOff          //!< Generate nothing
        , MenuLevelError        //!< Generate fatal and error
        , MenuLevelWarning      //!< and warning
        , MenuLevelInfo         //!< and information
        , MenuLevelDebug        //!< and debug
        , MenuScopeLines        //!< Switch the enter and exit lines
        , MenuTempRaise         //!< Let the next raise go back on its own
        , MenuReachScope        //!< Apply a priority to the clicked scope
        , MenuReachBranch       //!< Apply a priority to the clicked node and everything under it
        , MenuReachProcess      //!< Apply a priority to every scope of the process
        , MenuShowOnlyThis      //!< Draw the rows of this branch and of nothing else
        , MenuHideThis          //!< Stop drawing the rows of this branch
        , MenuShowAll           //!< Draw the rows of every scope again
        , MenuExpandSelected    //!< Expand selected node
        , MenuCollapseSelected  //!< Collapse selected node
        , MenuExpandAll         //!< Expand all nodes
        , MenuCollapseAll       //!< Collapse all nodes
        , MenuSavePrioTarget    //!< Save priority settings of the selected target
        , MenuSavePrioAll       //!< Save priority settings of all targets
        , MenuRestorePrioTarget //!< Make the selected target apply the priorities it has saved
        , MenuRestorePrioAll    //!< Make every target apply the priorities it has saved
        , MenuTargetStop        //!< Stop the selected target producing any log
        , MenuTargetPause       //!< Let the selected target produce its logs and hold them
        , MenuTargetResume      //!< Let the selected target produce and send its logs again
        , MenuTargetResumeAll   //!< Let every held or stopped target produce and send again
        , MenuCopyScopePath     //!< Copy the full path of the clicked node
    };

    /**
     * \brief   What a priority chosen in the context menu applies to. The choice is kept
     *          between menus, so the same reach holds until it is changed.
     **/
    enum eScopeReach
    {
          ReachScope    = 0 //!< The clicked scope alone. Only a leaf is a single scope
        , ReachBranch       //!< The clicked node and everything under it
        , ReachProcess      //!< Every scope of the process the clicked node belongs to
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    NaviLogScopeBase(int naviWindow, MdiMainWindow* wndMain, QWidget* parent = nullptr);
    virtual ~NaviLogScopeBase(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns true if root entries are collapsed.
     **/
    bool areRootsCollapsed(void) const;

    /**
     * \brief   Returns the tree entry of the given scope of the given target.
     * \param   target  The ID of the target the scope belongs to.
     * \param   scopeId The ID of the scope.
     * \return  The index of the scope, invalid when the tree does not carry it.
     **/
    QModelIndex findScopeIndex(ITEM_ID target, uint32_t scopeId) const;

    /**
     * \brief   Fills the given menu with the scope entries of the given tree row, and makes
     *          that row the current one so a following action acts on it.
     * \param   menu    The menu to fill.
     * \param   node    The tree entry the menu belongs to.
     * \return  True if the menu was filled.
     * \note    Pair it with `applyScopeMenu`, which runs what the reader chose.
     **/
    bool populateScopeMenu(QMenu& menu, const QModelIndex& node);

    /**
     * \brief   Runs the entry the reader chose in a menu filled by `populateScopeMenu`.
     * \param   action  The chosen entry.
     * \param   node    The tree entry the menu was filled for.
     * \return  True if the entry belonged to the scope menu and was run.
     **/
    bool applyScopeMenu(const QAction& action, const QModelIndex& node);

    /**
     * \brief   Sets the scope model and binds it to the scope tree.
     * \param   model   The scope model to show in the tree.
     **/
    void setupModel(LoggingScopesModelBase* model);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Puts the priority ladder and the show and hide buttons in the state the given
     *          selection calls for. With nothing selected the ladder shows no level at all.
     **/
    virtual void enableButtons(const QModelIndex& selection);

    /**
     * \brief   Puts the priority ladder and the show and hide buttons in the state the scope
     *          the user has selected calls for.
     * \note    Every caller that reacts to the tree changing under the user, rather than to
     *          the user picking another scope, must use this. Passing the index the change
     *          arrived on would make the ladder describe a scope nobody selected.
     **/
    void refreshButtons(void);

    /**
     * \brief   Updates the expanded of the log scopes model based on the current index.
     * \param   current    The current index to update expanded.
     **/
    virtual void updateExpanded(const QModelIndex& current);

    /**
     * \brief   Updates the priority of the log scope at the given index.
     * \param   node       The index of the log scope to update priority.
     * \param   addPrio    If true, adds the priority. Otherwise, removes the priority.
     * \param   prio       The log priority to set or remove.
     * \return  Returns true if succeeded the request to update the priority. Otherwise, returns false.
     **/
    virtual bool updatePriority(const QModelIndex& node, bool addPrio, areg::LogPriority prio);

    /**
     * \brief   Returns the priority bits a ladder level stands for. The levels are cumulative:
     *          warning means fatal, error and warning.
     * \param   level   The level of the ladder.
     * \return  The combination of priority bits, without the scope lines flag.
     **/
    static uint32_t priorityOfLevel(int level);

    /**
     * \brief   Returns the ladder level the given priority bits stand for.
     * \param   prio    The combination of priority bits.
     **/
    static int levelOfPriority(uint32_t prio);

    /**
     * \brief   Returns the node a priority change applies to, following the kept reach.
     * \param   node    The index the context menu was opened on.
     * \return  The index to change, or an invalid index if there is none.
     **/
    QModelIndex reachTarget(const QModelIndex& node) const;

    /**
     * \brief   Sets the scope priority of the reach target to the given ladder level,
     *          keeping the enter and exit lines as they are.
     * \param   node    The index the context menu was opened on.
     * \param   level   The level of the ladder to set.
     **/
    void applyPriorityLevel(const QModelIndex& node, int level);

    /**
     * \brief   Puts the full path of the given node on the clipboard.
     **/
    void copyScopePath(const QModelIndex& node) const;

    /**
     * \brief   Builds the scope context menu on the given node.
     * \param   menu    The menu to fill.
     * \param   node    The index the menu was opened on.
     * \param   entry   The scope node the index points to.
     **/
    void buildScopeMenu(QMenu& menu, const QModelIndex& node, const ScopeNodeBase& entry);

    /**
     * \brief   Runs one of the show and hide entries on the given node.
     * \param   node    The node to show or hide.
     * \param   entry   One of the show and hide entries. Anything else is ignored.
     **/
    void applyScopeVisibility(const QModelIndex& node, eScopeMenu entry);

    /**
     * \brief   Runs the entry the user picked in the scope context menu.
     * \param   action  The chosen entry, never nullptr.
     * \param   node    The index the menu was opened on.
     * \return  True if the entry was handled here.
     **/
    bool runScopeMenu(const QAction& action, const QModelIndex& node);

    /**
     * \brief   Expands the child nodes of the specified scope tree recursively.
     * \param   idxNode    The index of the node to expand.
     * \param   node       The scope node to check and expand child nodes.
     **/
    virtual void expandChildNodesRecursive(const QModelIndex& idxNode, const ScopeNodeBase& node);

    /**
     * \brief   Collapses the root entries.
     **/
    virtual void collapseRoots(void);

    /**
     * \brief   Sets the pointer of associated live logs model.
     * \param   logModel    The pointer to the live logs model.
     *                      Can be nullptr if no live logs are available.
     **/
    virtual void setLoggingModel(LoggingModelBase* logModel);

    /**
     * \brief   Returns the pointer to the live logs model used by live logging scope navigation view.
     *          If no live logs are available, returns nullptr.
     **/
    virtual LoggingModelBase* getLoggingModel(void) const;

    /**
     * \brief   Collapses the root entries or expands the complete tree.
     * \param   checked     If true, only the root entry stays expanded.
     **/
    virtual void onCollapseClicked(bool checked);

//////////////////////////////////////////////////////////////////////////
// Operations for the derived classes
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Creates the tool button row and the scope tree of the explorer. Call it from
     *          the constructor of the derived class, before the scope model is set.
     * \note    It calls addSourceTool() and addExtraTools(), so the derived object must be
     *          constructed before the call.
     **/
    void setupScopeToolbar(void);

    /**
     * \brief   Connects the tool buttons and the scope tree. Call it after setupModel().
     **/
    void setupScopeControls(void);

    /**
     * \brief   Builds the two rows that look for a scope: the filter box that narrows the
     *          tree, and the find box that walks it. Called from setupScopeToolbar().
     **/
    void setupScopeSearch(void);

    /**
     * \brief   Builds the scope actions that carry a keyboard shortcut. They act on the row
     *          the cursor is on, so the menu and the key do the same thing.
     **/
    void setupScopeActions(void);

    /**
     * \brief   Builds the row that names what is standing, above the filter row. Called
     *          from setupScopeToolbar().
     **/
    void setupSafeguards(void);

    /**
     * \brief   Leaves in the tree only the scopes whose name carries the filter text, with
     *          the nodes above and below them. An empty text brings the whole tree back.
     **/
    void applyScopeFilter(void);

    /**
     * \brief   Opens or closes the find row and moves the focus into it or back to the tree.
     * \param   show    True to open the row.
     **/
    void showScopeFind(bool show);

    /**
     * \brief   Selects the next scope whose name carries the find text, opening the nodes
     *          above it and scrolling it into view. The tree is left whole.
     * \param   step    1 walks forward, -1 walks back.
     **/
    void findScope(int step);

    /**
     * \brief   Tells the tree which text to mark inside the scope names. The find text wins
     *          while the find row is open; otherwise the filter text is marked.
     **/
    void updateMatchMark(void);

    /**
     * \brief   Shows how many raises go back on their own. Hides itself when none do.
     **/
    void refreshSafeguards(void);

    /**
     * \brief   Redraws the controls that act on the target of the given tree entry.
     * \param   selection   The current tree entry, invalid when there is none.
     **/
    virtual void refreshTargetControls(const QModelIndex& selection);

    /**
     * \brief   Asks the target of the given tree entry to take a state. A stop is confirmed
     *          first, naming the target, because it turns every scope priority off.
     * \param   node    Any entry of the target.
     * \param   state   The state the target should take.
     **/
    void applyTargetState(const QModelIndex& node, areg::LogSourceState state);

    /**
     * \brief   Closes the find row when Escape is pressed inside its box.
     **/
    bool eventFilter(QObject* watched, QEvent* event) override;

    /**
     * \brief   Gives the leading show and hide column its fixed width and takes the tree
     *          indentation out of it, so every box sits at the same place.
     **/
    void setupShowColumn(void);

    /**
     * \brief   Gives the show and hide column its resize mode and width.
     * \note    A header section exists only while a model is attached, and attaching a model
     *          rebuilds the sections. Call it after every model change; it does nothing while
     *          the tree has fewer columns than the show and hide column needs.
     **/
    void applyShowColumnLayout(void);

    /**
     * \brief   Returns the width of the show and hide column, measured from the active style
     *          rather than fixed, so it stays tight at any scaling.
     **/
    int showColumnWidth(void) const;

    /**
     * \brief   Creates the single button that opens the log source of the explorer. It heads
     *          the tool row and stays in it at every width.
     * \return  The created button, or nullptr when the explorer has no such button.
     **/
    virtual QToolButton* addSourceTool(void);

    /**
     * \brief   Adds the remaining tool buttons of the explorer. They close the tool row,
     *          after every button the two explorers share.
     **/
    virtual void addExtraTools(void);

    /**
     * \brief   Returns true if the context menu offers the entries to save the priorities on
     *          the logging targets.
     **/
    virtual bool hasSavePrioMenu(void) const;

    /**
     * \brief   Returns true if the priorities can be saved on the logging targets right now.
     **/
    virtual bool canSavePrio(void) const;

    /**
     * \brief   Returns true if the context menu offers the entry to select all priorities.
     **/
    virtual bool hasSelectAllPrioMenu(void) const;

    /**
     * \brief   Shows the context menu of the scope tree and runs the chosen entry.
     * \param   pos     The cursor position in the coordinates of the tree viewport.
     **/
    void showScopeContextMenu(const QPoint& pos);

    /**
     * \brief   Expands the given node and its direct children.
     * \param   node            The index of the node to expand.
     * \param   markExpanded    If true, the model keeps the expanded state of the nodes.
     **/
    void expandNodeAndChildren(const QModelIndex& node, bool markExpanded);

    /**
     * \brief   Expands the given node if it is collapsed.
     * \param   node            The index of the node to expand.
     * \param   markExpanded    If true, the model keeps the expanded state of the node.
     **/
    void expandNode(const QModelIndex& node, bool markExpanded);

//////////////////////////////////////////////////////////////////////////
// Tool button controls
//////////////////////////////////////////////////////////////////////////
protected:

    //!< Returns the control object to expand or collapse entries of scopes.
    inline QToolButton* ctrlCollapse(void) const;

    //!< Returns the control object to find a string.
    inline QToolButton* ctrlFind(void) const;

    //!< Returns the ladder that sets the priority of the selected scopes.
    inline LogPriorityBar* ctrlPriorityBar(void) const;

    //!< Returns the control object to show the selected scope and hide every other one.
    inline QToolButton* ctrlShowOnly(void) const;

    //!< Returns the control object to hide the selected scope.
    inline QToolButton* ctrlHide(void) const;

    //!< Returns the control object to show every scope again.
    inline QToolButton* ctrlShowAll(void) const;

    //!< Returns the box that narrows the tree to the scopes whose name matches.
    inline SearchLineEdit* ctrlScopeFilter(void) const;

    //!< Returns the box that walks the tree from one matching scope to the next.
    inline SearchLineEdit* ctrlScopeFind(void) const;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Slot, triggered when a level is chosen on the toolbar ladder. It applies the
     *          level to the current selection, following the kept reach.
     * \param   level   The chosen position of the ladder.
     **/
    virtual void onPriorityLevelChosen(int level);

    /**
     * \brief   Slot, triggered when the enter and exit lines are switched on the toolbar ladder.
     * \param   enabled     True if the lines are to be written.
     **/
    virtual void onScopeLinesToggled(bool enabled);

    /**
     * \brief   Slot, triggered when a show and hide tool button is clicked.
     * \param   entry   The entry the button stands for.
     **/
    virtual void onScopeVisibilityClicked(eScopeMenu entry);

    /**
     * \brief   Slot, triggered when a navigation node is expanded or collapsed.
     * \param   index       The index of the navigation node that was expanded or collapsed.
     * \param   expanded    The flag indicating whether the node is expanded or collapsed.
     **/
    virtual void onNodeExpanded(const QModelIndex& index, bool expanded);

    //!< Slot, triggered when the selection in the log scopes navigation is changed.
    virtual void onRowChanged(const QModelIndex& current, const QModelIndex& previous);

private:
    //!< Validates the object by checking with assertions.
    inline void validateControls(void) const;

    /**
     * \brief   Hides the children of the given node that neither match nor hold a match.
     * \param   parent      The node whose children are looked at.
     * \param   needle      The text to look for.
     * \param   inKept      True when a node above already matched, so everything below stays.
     * \param   matches     Grows by the number of names that carry the text.
     * \return  True if the node itself or anything under it stays in the tree.
     **/
    bool filterBranch(const QModelIndex& parent, const QString& needle, bool inKept, int& matches);

    //!< Opens and closes the children of the given node the way the model remembers them.
    void restoreExpanded(const QModelIndex& parent);

    //!< Collects, from top to bottom, every scope whose name carries the given text.
    void collectMatches(const QModelIndex& parent, const QString& needle, Qt::CaseSensitivity sensitivity, QList<QModelIndex>& matches) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    LoggingScopesModelBase* mScopesModel;   //!< The model of the log scopes
    QItemSelectionModel*    mSelModel;      //!< Selection model

private:
    QToolButton*            mToolCollapse;  //!< The tool button to collapse or expand the scope tree
    QToolButton*            mToolFind;      //!< The tool button to find a log message
    LogPriorityBar*         mPrioBar;       //!< The ladder that sets the priority of the selected scopes
    QToolButton*            mToolShowOnly;  //!< The tool button that shows the selection and hides the rest
    QToolButton*            mToolHide;      //!< The tool button that hides the selection
    QToolButton*            mToolShowAll;   //!< The tool button that shows every scope again
    eScopeReach             mScopeReach;    //!< What a priority chosen in the context menu applies to

    QWidget*                mFilterBar;     //!< The row that carries the scope name filter box
    SearchLineEdit*         mFilterEdit;    //!< The box that narrows the tree
    QWidget*                mFindBar;       //!< The row that carries the find box, closed until it is asked for
    SearchLineEdit*         mFindEdit;      //!< The box that walks from one matching scope to the next
    ScopeNameDelegate*     mHighlight;     //!< The delegate that marks the matched part of a scope name
    QPersistentModelIndex   mFindAt;        //!< The scope the find box stopped on last

    QWidget*                mGuardBar;      //!< The row that names what is standing, hidden when nothing is
    QWidget*                mRaiseRow;      //!< The line about the raises that go back on their own
    QLabel*                 mRaiseText;     //!< How many raises go back on their own
    QWidget*                mStopRow;       //!< The line about the targets that are not sending
    QLabel*                 mStopText;      //!< Which targets are not sending
    QWidget*                mQuietRow;      //!< The line about the scopes turned down and left that way
    QLabel*                 mQuietText;     //!< How many scopes generate less than their target reported
    bool                    mTempRaise;     //!< True while a raise is meant to go back on its own
    QString                 mPrioTip;       //!< The tool tip of the priority bar while the target is reachable

    QAction*                mActShowOnly;   //!< Show only the scope under the cursor, hide the rest
    QAction*                mActHide;       //!< Hide the scope under the cursor
    QAction*                mActShowAll;    //!< Show every scope again
    QAction*                mActCopyPath;   //!< Copy the path of the scope under the cursor
};

//////////////////////////////////////////////////////////////////////////
// NaviLogScopeBase class inline methods
//////////////////////////////////////////////////////////////////////////

inline QToolButton* NaviLogScopeBase::ctrlCollapse(void) const
{
    return mToolCollapse;
}

inline QToolButton* NaviLogScopeBase::ctrlFind(void) const
{
    return mToolFind;
}

inline LogPriorityBar* NaviLogScopeBase::ctrlPriorityBar(void) const
{
    return mPrioBar;
}

inline QToolButton* NaviLogScopeBase::ctrlShowOnly(void) const
{
    return mToolShowOnly;
}

inline QToolButton* NaviLogScopeBase::ctrlHide(void) const
{
    return mToolHide;
}

inline QToolButton* NaviLogScopeBase::ctrlShowAll(void) const
{
    return mToolShowAll;
}

inline SearchLineEdit* NaviLogScopeBase::ctrlScopeFilter(void) const
{
    return mFilterEdit;
}

inline SearchLineEdit* NaviLogScopeBase::ctrlScopeFind(void) const
{
    return mFindEdit;
}

#endif  // LUSAN_VIEW_COMMON_NAVILOGSCOPEBASE_HPP
