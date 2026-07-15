#ifndef LUSAN_VIEW_SM_SMDESIGN_HPP
#define LUSAN_VIEW_SM_SMDESIGN_HPP
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
 *  \file        lusan/view/sm/SMDesign.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM editor Design page.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QMainWindow>

#include "lusan/view/sm/NESMDesign.hpp"

#include <QHash>
#include <QList>
#include <QPoint>
#include <cstdint>
#include <memory>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QAction;
class QDockWidget;
class QHBoxLayout;
class QKeyEvent;
class QLabel;
class QLineEdit;
class QToolBar;
class QToolButton;
class SMClipboardContent;
class SMGraphicsView;
class SMOutlinePanel;
class SMPropertiesPanel;
class SMValidationPanel;
class SMScene;
class SMSceneManager;
class SMStateData;
class StateMachineModel;

/**
 * \class   SMDesign
 * \brief   The Design page: a QMainWindow whose central widget is the breadcrumb of the
 *          machine-level path and the graphics viewport over the displayed level's scene.
 *          It hosts, inside the page, its own drawing toolbar (a real QToolBar, movable to
 *          the page's edges) plus the Properties and Outline dock panels (issue #516). The
 *          viewport, grid, editing, and level-navigation commands are exposed as actions so
 *          the toolbar, Design menu, and context menus dispatch to the same set. Each level's
 *          zoom and scroll is persisted in its View layout entry and restored on return.
 **/
class SMDesign : public QMainWindow
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eDeclareKind
     * \brief   The registry entry kind of one "Add Declaration" dropdown entry
     *          (spec 9.2). Submachine imports are not listed - no page exists yet for
     *          them (SM-29).
     **/
    enum class eDeclareKind
    {
          Trigger
        , Action
        , Condition
        , Event
        , Timer
        , Attribute
        , Constant
        , DataType
    };

    /**
     * \struct  ToolGroup
     * \brief   One named group of drawing-toolbar actions, ordered by importance. The
     *          navigation dock's Design Toolbar tab lays these out as collapsible sections.
     **/
    struct ToolGroup
    {
        QString         title;      //!< The group heading.
        QList<QAction*> actions;    //!< The group's actions, in display order.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMDesign(StateMachineModel& model, QWidget* parent = nullptr);
    virtual ~SMDesign();

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Shows or hides the in-page drawing toolbar (spec 9.2/9.3: hideable via the
     *          application View menu; every toolbar command remains reachable via the
     *          Design menu, context menus, and shortcuts).
     **/
    void setToolbarVisible(bool visible);
    bool isToolbarVisible() const;

    /**
     * \brief   Shows or hides the in-page Properties / Outline dock panels. Driven by the
     *          global widget placement (issue #516): when a panel is moved to the Navigation
     *          Window or hidden, its in-page dock is hidden so it never shows in both places.
     **/
    void setPropertiesVisible(bool visible);
    bool isPropertiesVisible() const;
    void setOutlineVisible(bool visible);
    bool isOutlineVisible() const;

    /**
     * \brief   Records where each of the three design widgets (toolbar, Properties, Outline)
     *          currently lives, so the canvas context menu's View submenu can show the correct
     *          check marks. Values match MdiMainWindow's eDesignWidget / eDesignPlace ints.
     * \param   toolbar     Placement of the drawing toolbar (0 hidden, 1 in design, 2 in nav).
     * \param   properties  Placement of the Properties panel.
     * \param   outline     Placement of the Outline panel.
     **/
    void setPlacementState(int toolbar, int properties, int outline);

    /**
     * \brief   Applies the toolbutton display style (icon only / text beside / text under) to
     *          the in-page drawing toolbar. Driven by the application View menu's Toolbutton
     *          Mode submenu; the chosen style is persisted and seeded on the next page's toolbar.
     **/
    void setToolbarStyle(Qt::ToolButtonStyle style);
    /**
     * \brief   Returns the scene of the displayed machine level.
     **/
    inline SMScene& getScene() const;

    /**
     * \brief   Returns the canvas viewport.
     **/
    inline SMGraphicsView& getView() const;

    /**
     * \brief   Returns the machine-level scene manager (navigation and scene cache).
     **/
    inline SMSceneManager& getSceneManager() const;

    /**
     * \brief   Returns the document facade. Exposed so the main window can bind the global
     *          Outline/Properties docks to the active Design page's model (issue #516).
     **/
    inline StateMachineModel& getModel() const;

    /**
     * \brief   Returns the breadcrumb bar widget (the clickable ancestor buttons and the
     *          bold current-level label). Exposed so tests can scope a `QToolButton`
     *          search to the breadcrumb without also matching the drawing toolbar.
     **/
    inline QWidget* getBreadcrumb() const;

    /**
     * \brief   The viewport and grid command actions.
     **/
    inline QAction* actionZoomIn() const;
    inline QAction* actionZoomOut() const;
    inline QAction* actionZoomReset() const;
    inline QAction* actionZoomFit() const;
    inline QAction* actionToggleGrid() const;
    inline QAction* actionGridDots() const;
    inline QAction* actionGridDotSize() const;
    inline QAction* actionToggleSnap() const;
    inline QAction* actionGridSize() const;
    inline QAction* actionSelectAll() const;

    /**
     * \brief   The undo/redo actions (no shortcut of their own - Ctrl+Z/Ctrl+Y are the
     *          global Edit-menu shortcut; these exist so the canvas toolbar/Design menu
     *          carry the "General" group's Undo/Redo per spec 9.2).
     **/
    inline QAction* actionUndo() const;
    inline QAction* actionRedo() const;

    /**
     * \brief   The state editing actions: placement tools, delete, and rename.
     **/
    inline QAction* actionAddState() const;
    inline QAction* actionAddFinalState() const;
    inline QAction* actionAddTransition() const;
    inline QAction* actionAddNote() const;
    inline QAction* actionDelete() const;
    inline QAction* actionRename() const;

    /**
     * \brief   The clipboard actions: cut, copy, paste, and duplicate.
     **/
    inline QAction* actionCut() const;
    inline QAction* actionCopy() const;
    inline QAction* actionPaste() const;
    inline QAction* actionDuplicate() const;

    /**
     * \brief   The appearance actions: color swatches, alignment, and distribution.
     **/
    inline QAction* actionStateColor() const;
    inline QAction* actionEdgeColor() const;
    inline QAction* actionNoteColor() const;
    /**
     * \brief   The single "Set Color" action for the toolbar: applies a picked color to the
     *          current selection (states, transitions, and/or notes), disabled when nothing
     *          is selected. The per-kind color actions above remain for the context menus.
     **/
    inline QAction* actionSetColor() const;
    inline QAction* actionAlignLeft() const;
    inline QAction* actionAlignRight() const;
    inline QAction* actionAlignTop() const;
    inline QAction* actionAlignBottom() const;
    inline QAction* actionDistributeHorizontal() const;
    inline QAction* actionDistributeVertical() const;

    /**
     * \brief   The transition editing actions: internal transition, stimulus picker,
     *          and priority reorder.
     **/
    inline QAction* actionAddInternal() const;
    inline QAction* actionSetStimulus() const;
    inline QAction* actionRaisePriority() const;
    inline QAction* actionLowerPriority() const;

    /**
     * \brief   The hierarchy actions: convert to composite, descend, and ascend.
     **/
    inline QAction* actionAddSubstate() const;
    inline QAction* actionEnterSubmachine() const;
    inline QAction* actionGoToParent() const;
    inline QAction* actionCenterMachine() const;

    /**
     * \brief   Deletes the current selection after confirmation: selected states (with
     *          painted substates and connected transitions), or selected transitions, as
     *          one undo step. Start states are never deleted.
     **/
    void deleteSelection();

    /**
     * \brief   Copies the selection (states with their subtrees, notes, registry
     *          entries) to the system clipboard; no-op when nothing is copyable.
     **/
    void copySelection();

    /**
     * \brief   Copies the selection to the clipboard and deletes the copied states and
     *          notes as one undo step, without confirmation. Registry entries are
     *          copied but never deleted here.
     **/
    void cutSelection();

    /**
     * \brief   Pastes the clipboard payload into the displayed level as one undo step:
     *          fresh IDs, `Name_<id>` collision renames, registry merge by name, and a
     *          grid-step position offset. The pasted elements become the selection.
     **/
    void pasteClipboard();

    /**
     * \brief   Duplicates the selection in place (copy + paste in one step, without
     *          touching the system clipboard).
     **/
    void duplicateSelection();

    /**
     * \brief   The "Add Declaration" dropdown entries, in display order, each requesting
     *          signalDeclareRequested(kind) when triggered.
     **/
    QList<QAction*> declareActions() const;

    /**
     * \brief   Returns the drawing commands grouped and ordered by importance (Design,
     *          Declare, Alignment, Navigate, Color, Grid, Edit, Zoom). The navigation dock's
     *          Design Toolbar tab builds its collapsible sections from this.
     **/
    QList<ToolGroup> toolGroups() const;

    /**
     * \brief   Builds the same group/action structure as toolGroups(), but with fresh,
     *          disabled display-only actions parented to \p owner. The navigation dock's
     *          Design Toolbar tab shows these while no State Machine document is active,
     *          so the toolbar always presents its buttons (issue #514).
     **/
    static QList<ToolGroup> placeholderToolGroups(QObject& owner);

    /**
     * \brief   True when \p action activates a placement tool (Add State/Final/Transition/
     *          Note); on true, \p toolOut receives the tool so the toolbar can arm it sticky
     *          on a double-click (spec 9.2 rule 1).
     **/
    bool placementToolFor(QAction* action, NESMDesign::eCanvasTool& toolOut) const;

    /**
     * \brief   Arms the given placement tool sticky on the live scene (double-click gesture).
     **/
    void armStickyTool(NESMDesign::eCanvasTool tool);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   Emitted by a Declare-dropdown entry; StateMachine switches to the owning
     *          page and starts a new entry of the requested kind (spec 9.2).
     **/
    void signalDeclareRequested(SMDesign::eDeclareKind kind);

    /**
     * \brief   Emitted by the canvas context menu's View submenu; the owning window moves the
     *          given design widget to the requested home (issue #516). The arguments match
     *          MdiMainWindow's eDesignWidget / eDesignPlace ints (widget: 0 toolbar, 1
     *          Properties, 2 Outline; place: 1 in Design page, 2 in Navigation Window).
     **/
    void signalPlaceDesignWidget(int widget, int place);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Claims the page's shortcuts while the canvas has focus, so
     *          window-wide shortcuts of other pages cannot swallow them.
     **/
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onDocumentReloaded();

    /**
     * \brief   Builds and shows the context-sensitive right-click menu (canvas / state /
     *          transition / note, spec 9.3 rule 2) at the given viewport position.
     **/
    void onViewContextMenuRequested(const QPoint& pos);

    /**
     * \brief   Reacts to a level switch: swaps the view's scene, applies the grid
     *          settings, restores the level's viewport, and refreshes the breadcrumb.
     **/
    void onLevelChanged(uint32_t levelId);

    /**
     * \brief   Persists the displayed level's zoom/scroll into its View layout entry
     *          through a coalesced undo command; no-op while a restore is in progress
     *          or when the stored entry already matches.
     **/
    void onViewportChanged();

    /**
     * \brief   Re-applies the persisted viewport when an undo/redo changed the displayed
     *          level's View entry.
     **/
    void onModelLayoutChanged(const QList<uint32_t>& ownerIds);

private:
    void setupActions();

    /**
     * \brief   Builds the in-page drawing toolbar: a real QToolBar (icon-only by default,
     *          seeded from the persisted style), one button per action of toolGroups() with
     *          a separator between groups, movable to any of the page's four edges.
     **/
    void buildDesignToolbar();

    /**
     * \brief   Builds the in-page Properties (right, top) and Outline (right, below) dock
     *          panels, bound to this page's model and scene manager, and wires the outline's
     *          rename/delete context actions to this page's own actions (issue #516).
     **/
    void buildDesignPanels();

    /**
     * \brief   Rebuilds the breadcrumb from the current level path: clickable ancestor
     *          buttons and the bold current-level label.
     **/
    void rebuildBreadcrumb();

    /**
     * \brief   Updates the enabled state of the hierarchy actions from the selection
     *          and the current level.
     **/
    void updateNavActions();

    /**
     * \brief   Applies a level's persisted View entry to the viewport; without an entry
     *          falls back to 100% zoom centered on the content when \p applyDefault.
     **/
    void restoreViewport(uint32_t levelId, bool applyDefault);

    /**
     * \brief   Converts the single selected Normal state into a painted composite
     *          (nested StateList with its Start state) and enters the new level.
     **/
    void addSubstateToSelection();

    /**
     * \brief   Descends into the single selected composite state's submachine.
     **/
    void enterSelectedSubmachine();

    /**
     * \brief   Brings the level's painted content back into view (issue #514): centers it
     *          when it fits the viewport, otherwise anchors its top-left near the viewport's
     *          top-left corner (about 64 device pixels in), keeping the current zoom.
     **/
    void centerMachine();

    /**
     * \brief   Deletes the selected transition edges after confirmation, one undo step.
     **/
    void deleteSelectedEdges();

    /**
     * \brief   Deletes the selected notes (no confirmation - diagram-only annotations),
     *          one undo step.
     **/
    void deleteSelectedNotes();

    /**
     * \brief   Reorders the single selected transition by one step in document order
     *          (priority); \p raise moves it earlier (higher priority).
     **/
    void reorderSelectedTransition(bool raise);

    /**
     * \brief   Opens a picker over the shared registries to set the selected transition's
     *          stimulus (kind + name).
     **/
    void setStimulusOfSelection();

    /**
     * \brief   Adds an internal transition (no target; runs its operations without
     *          exit/entry) to the single selected state.
     **/
    void addInternalToSelection();

    /**
     * \brief   Adds the state/transition's note-management entries (Add Note, or Edit Note +
     *          Remove Note when one exists) to a context menu for the given owner element.
     * \param   menu    The menu to extend.
     * \param   ownerId The state or transition element ID.
     * \param   isState True for a state owner, false for a transition owner.
     **/
    void addNoteMenuEntries(QMenu& menu, uint32_t ownerId, bool isState);

    /**
     * \brief   Creates a note bound to the given owner (if none exists) and opens its editor.
     **/
    void addOwnedNote(uint32_t ownerId, bool isState);

    /**
     * \brief   Opens the note editor of the given owner's state/transition item.
     **/
    void editOwnedNote(uint32_t ownerId);

    /**
     * \brief   Removes the note bound to the given owner, as one undo step.
     **/
    void removeOwnedNote(uint32_t ownerId);

    /**
     * \enum    eColorTarget
     * \brief   Which kind of selected item a color-apply action targets.
     **/
    enum class eColorTarget { State, Edge, Note };

    /**
     * \brief   Opens a color picker and applies the chosen color to every selected item of
     *          the given kind, as one undo step (spec 9.2 "State Color"/"analogous" edge
     *          color); no-op when the selection has none of that kind.
     **/
    void applyColorToSelection(eColorTarget target);

    /**
     * \brief   Opens a color picker and applies the chosen color to every selected item of
     *          any kind (states, transitions, notes) as one undo step; no-op on an empty
     *          selection. Backs the toolbar's single "Set Color" action.
     **/
    void applyColorToCurrentSelection();

    /**
     * \enum    eAlign
     * \brief   The alignment edge for alignSelection().
     **/
    enum class eAlign { Left, Right, Top, Bottom };

    /**
     * \brief   Aligns every selected state/note box to the given edge of the selection's
     *          bounding box, as one undo step; no-op on a single-element selection.
     **/
    void alignSelection(eAlign align);

    /**
     * \enum    eDistribute
     * \brief   The axis for distributeSelection().
     **/
    enum class eDistribute { Horizontal, Vertical };

    /**
     * \brief   Spaces every selected state/note box evenly (by center) along the given axis
     *          between the first and last (by position), as one undo step.
     **/
    void distributeSelection(eDistribute axis);

    /**
     * \brief   Builds and pushes the paste command for a parsed clipboard payload and
     *          selects the pasted elements.
     **/
    void pushPaste(std::unique_ptr<SMClipboardContent> content, const QString& text);

    /**
     * \brief   Returns the page action bound to the pressed key, or nullptr.
     **/
    QAction* matchAction(const QKeyEvent& event) const;

    /**
     * \brief   Rebuilds the scene cache for a (re)loaded document and shows its root level.
     **/
    void rebuildScene();

    /**
     * \brief   Gives every state that carries no Node layout entry a free, non-overlapping
     *          place on its level, as one undoable edit; no-op when the layout is complete.
     **/
    void autoPlaceMissingNodes();

    /**
     * \brief   Fills the scene with synthetic nodes when the
     *          LUSAN_SM_CANVAS_STRESS environment variable is set (performance check).
     **/
    void populateStressContent();

    /**
     * \struct  SearchHit
     * \brief   One canvas-search match: the machine level that draws it and the matched
     *          element (a state box or a transition edge).
     **/
    struct SearchHit
    {
        uint32_t    level;      //!< The level owner ID the element is drawn on.
        uint32_t    elementId;  //!< The matched state / transition element ID.
        bool        isState;    //!< True for a state, false for a transition.
    };

    /**
     * \brief   Recomputes the canvas-search matches for the current query and jumps to the
     *          first (SM-21-08). Empty query clears the affordance; no match leaves the
     *          canvas unchanged with a "No match" note.
     **/
    void onSearchTextChanged();

    /**
     * \brief   Cycles to the next match (Enter / repeated search), recomputing first when
     *          the cache is empty.
     **/
    void advanceSearch();

    /**
     * \brief   Navigates to the match's level, selects it (reusing the highlight/selection
     *          path), and centers the viewport on it.
     **/
    void focusSearchHit(int index);

    //!< Updates the "current / total" match counter next to the search box.
    void updateSearchStatus();

    /**
     * \brief   Collects the matches of \p query over state names, transition stimuli, and
     *          transition targets, recursing into painted submachines. A local name scan
     *          until SM-26's shared search index lands (one index, two surfaces).
     **/
    void collectSearchHits(const QString& query, const SMStateData& level, uint32_t levelId, QList<SearchHit>& out) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;         //!< The document facade.
    SMGraphicsView*     mView;          //!< The canvas viewport.
    SMSceneManager*     mSceneManager;  //!< The level navigation and scene cache.
    SMScene*            mScene;         //!< The displayed level's scene.
    QWidget*            mBreadcrumb;    //!< The level-path bar above the viewport.
    QHBoxLayout*        mBreadcrumbLayout; //!< The breadcrumb content layout.
    QLineEdit*          mSearchEdit;    //!< The canvas search box (find state / transition).
    QLabel*             mSearchStatus;  //!< The "current / total" match counter (or "No match").
    QList<SearchHit>    mSearchHits;    //!< The current query's matches, in document order.
    int                 mSearchIndex;   //!< The focused match index, or -1.
    QToolBar*           mToolBar;       //!< The in-page drawing toolbar (movable to the page edges).
    QDockWidget*        mPropertiesDock;//!< The Properties dock (right, top) inside the Design page.
    QDockWidget*        mOutlineDock;   //!< The Outline dock (right, below Properties) inside the Design page.
    QDockWidget*        mValidationDock;//!< The Validation results dock (bottom, S15) inside the Design page.
    SMPropertiesPanel*  mProperties;    //!< The Properties editor hosted in mPropertiesDock.
    SMOutlinePanel*     mOutline;       //!< The Outline tree hosted in mOutlineDock.
    SMValidationPanel*  mValidation;    //!< The validation results list hosted in mValidationDock.
    QAction*            mActZoomIn;     //!< Zoom one step in.
    QAction*            mActZoomOut;    //!< Zoom one step out.
    QAction*            mActZoomReset;  //!< Zoom to 100%.
    QAction*            mActZoomFit;    //!< Zoom to fit the level content.
    QAction*            mActToggleGrid; //!< Show / hide the grid.
    QAction*            mActGridDots;   //!< Render the grid as dots instead of lines.
    QAction*            mActGridDotSize;//!< Opens a dialog to change the dotted-grid dot size.
    QAction*            mActToggleSnap; //!< Toggle snap-to-grid.
    QAction*            mActGridSize;   //!< Opens a dialog to change the grid cell size.
    QAction*            mActSelectAll;  //!< Select every element on the level.
    QAction*            mActUndo;       //!< Undo (no shortcut; the Edit menu owns Ctrl+Z).
    QAction*            mActRedo;       //!< Redo (no shortcut; the Edit menu owns Ctrl+Y).
    QAction*            mActAddState;   //!< Activate the Add State tool.
    QAction*            mActAddFinal;   //!< Activate the Add Final State tool.
    QAction*            mActAddTransition; //!< Activate the Add Transition tool.
    QAction*            mActAddNote;    //!< Activate the Add Note tool.
    QAction*            mActDelete;     //!< Delete the selection with confirmation.
    QAction*            mActRename;     //!< Rename the selected state in place.
    QAction*            mActCut;        //!< Cut the selection to the clipboard.
    QAction*            mActCopy;       //!< Copy the selection to the clipboard.
    QAction*            mActPaste;      //!< Paste the clipboard into the shown level.
    QAction*            mActDuplicate;  //!< Duplicate the selection in place.
    QAction*            mActStateColor; //!< Apply a picked color to the selected states.
    QAction*            mActEdgeColor;  //!< Apply a picked color to the selected transitions.
    QAction*            mActNoteColor;  //!< Apply a picked color to the selected notes.
    QAction*            mActSetColor;   //!< Toolbar: apply a picked color to the whole selection.
    QAction*            mActAlignLeft;  //!< Align selected boxes to the leftmost edge.
    QAction*            mActAlignRight; //!< Align selected boxes to the rightmost edge.
    QAction*            mActAlignTop;   //!< Align selected boxes to the topmost edge.
    QAction*            mActAlignBottom;//!< Align selected boxes to the bottommost edge.
    QAction*            mActDistributeH;//!< Distribute selected boxes evenly, horizontally.
    QAction*            mActDistributeV;//!< Distribute selected boxes evenly, vertically.
    QAction*            mActAddInternal;//!< Add an internal transition to the selected state.
    QAction*            mActSetStimulus;//!< Set the selected transition's stimulus.
    QAction*            mActRaisePriority; //!< Raise the selected transition's priority.
    QAction*            mActLowerPriority; //!< Lower the selected transition's priority.
    QAction*            mActAddSubstate;   //!< Convert the selected state to a composite.
    QAction*            mActEnterSubmachine; //!< Descend into the selected submachine.
    QAction*            mActGoToParent;    //!< Ascend to the parent level.
    QAction*            mActCenterMachine; //!< Bring the painted machine back into view.
    QAction*            mActNewTrigger; //!< Declare dropdown: new trigger method.
    QAction*            mActNewAction;  //!< Declare dropdown: new action method.
    QAction*            mActNewCondition; //!< Declare dropdown: new condition method.
    QAction*            mActNewEvent;   //!< Declare dropdown: new event.
    QAction*            mActNewTimer;   //!< Declare dropdown: new timer.
    QAction*            mActNewAttribute; //!< Declare dropdown: new attribute.
    QAction*            mActNewConstant;  //!< Declare dropdown: new constant.
    QAction*            mActNewDataType;  //!< Declare dropdown: new data type.
    bool                mToolbarVisible;//!< Requested visibility of the in-page drawing toolbar.
    int                 mPlaceToolbar;  //!< Cached toolbar placement (for the context menu check marks).
    int                 mPlaceProperties; //!< Cached Properties placement (for the context menu check marks).
    int                 mPlaceOutline;  //!< Cached Outline placement (for the context menu check marks).
    uint32_t            mShownLevel;    //!< The level the viewport currently shows.
    uint32_t            mViewGesture;   //!< The coalescing key of this level visit's viewport writes.
    bool                mRestoringView; //!< A viewport restore is in progress; do not persist.
    bool                mSyncingGrid;   //!< A programmatic grid-action resync is in progress; do not push a command.
};

//////////////////////////////////////////////////////////////////////////
// SMDesign inline methods
//////////////////////////////////////////////////////////////////////////

inline SMScene& SMDesign::getScene() const
{
    return *mScene;
}

inline SMGraphicsView& SMDesign::getView() const
{
    return *mView;
}

inline SMSceneManager& SMDesign::getSceneManager() const
{
    return *mSceneManager;
}

inline StateMachineModel& SMDesign::getModel() const
{
    return mModel;
}

inline QWidget* SMDesign::getBreadcrumb() const
{
    return mBreadcrumb;
}

inline QAction* SMDesign::actionZoomIn() const
{
    return mActZoomIn;
}

inline QAction* SMDesign::actionZoomOut() const
{
    return mActZoomOut;
}

inline QAction* SMDesign::actionZoomReset() const
{
    return mActZoomReset;
}

inline QAction* SMDesign::actionZoomFit() const
{
    return mActZoomFit;
}

inline QAction* SMDesign::actionToggleGrid() const
{
    return mActToggleGrid;
}

inline QAction* SMDesign::actionGridDots() const
{
    return mActGridDots;
}

inline QAction* SMDesign::actionGridDotSize() const
{
    return mActGridDotSize;
}

inline QAction* SMDesign::actionToggleSnap() const
{
    return mActToggleSnap;
}

inline QAction* SMDesign::actionGridSize() const
{
    return mActGridSize;
}

inline QAction* SMDesign::actionSelectAll() const
{
    return mActSelectAll;
}

inline QAction* SMDesign::actionUndo() const
{
    return mActUndo;
}

inline QAction* SMDesign::actionRedo() const
{
    return mActRedo;
}

inline QAction* SMDesign::actionAddState() const
{
    return mActAddState;
}

inline QAction* SMDesign::actionAddFinalState() const
{
    return mActAddFinal;
}

inline QAction* SMDesign::actionAddTransition() const
{
    return mActAddTransition;
}

inline QAction* SMDesign::actionAddNote() const
{
    return mActAddNote;
}

inline QAction* SMDesign::actionDelete() const
{
    return mActDelete;
}

inline QAction* SMDesign::actionRename() const
{
    return mActRename;
}

inline QAction* SMDesign::actionCut() const
{
    return mActCut;
}

inline QAction* SMDesign::actionCopy() const
{
    return mActCopy;
}

inline QAction* SMDesign::actionPaste() const
{
    return mActPaste;
}

inline QAction* SMDesign::actionDuplicate() const
{
    return mActDuplicate;
}

inline QAction* SMDesign::actionStateColor() const
{
    return mActStateColor;
}

inline QAction* SMDesign::actionEdgeColor() const
{
    return mActEdgeColor;
}

inline QAction* SMDesign::actionNoteColor() const
{
    return mActNoteColor;
}

inline QAction* SMDesign::actionSetColor() const
{
    return mActSetColor;
}

inline QAction* SMDesign::actionAlignLeft() const
{
    return mActAlignLeft;
}

inline QAction* SMDesign::actionAlignRight() const
{
    return mActAlignRight;
}

inline QAction* SMDesign::actionAlignTop() const
{
    return mActAlignTop;
}

inline QAction* SMDesign::actionAlignBottom() const
{
    return mActAlignBottom;
}

inline QAction* SMDesign::actionDistributeHorizontal() const
{
    return mActDistributeH;
}

inline QAction* SMDesign::actionDistributeVertical() const
{
    return mActDistributeV;
}

inline QAction* SMDesign::actionAddInternal() const
{
    return mActAddInternal;
}

inline QAction* SMDesign::actionSetStimulus() const
{
    return mActSetStimulus;
}

inline QAction* SMDesign::actionRaisePriority() const
{
    return mActRaisePriority;
}

inline QAction* SMDesign::actionLowerPriority() const
{
    return mActLowerPriority;
}

inline QAction* SMDesign::actionAddSubstate() const
{
    return mActAddSubstate;
}

inline QAction* SMDesign::actionEnterSubmachine() const
{
    return mActEnterSubmachine;
}

inline QAction* SMDesign::actionGoToParent() const
{
    return mActGoToParent;
}

inline QAction* SMDesign::actionCenterMachine() const
{
    return mActCenterMachine;
}

#endif  // LUSAN_VIEW_SM_SMDESIGN_HPP
