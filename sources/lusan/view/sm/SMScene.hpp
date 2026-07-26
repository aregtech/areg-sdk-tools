#ifndef LUSAN_VIEW_SM_SMSCENE_HPP
#define LUSAN_VIEW_SM_SMSCENE_HPP
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
 *  \file        lusan/view/sm/SMScene.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas scene of one machine level.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QGraphicsScene>

#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMCanvasTool.hpp"

#include "lusan/data/sm/SMReferences.hpp"

#include <QHash>
#include <QList>
#include <memory>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMCanvasItem;
class SMEdgeItem;
class SMNoteItem;
class SMStateItem;
class SMSubmachinePeek;
class StateMachineModel;
struct SMLayoutEdge;
enum class eDocElementKind;

/**
 * \class   SMScene
 * \brief   The graphics scene of one machine level. It renders the grid, owns the
 *          per-level item lookup (element ID to graphics item), runs the mode-based
 *          tool controller, applies grid snapping to interactive moves, and keeps the
 *          scene selection and the document selection model in sync.
 **/
class SMScene : public QGraphicsScene
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the scene of one machine level.
     * \param   model   The document facade.
     * \param   levelId The level owner's element ID (the Overview ID for the root level).
     * \param   parent  The owning object.
     **/
    SMScene(StateMachineModel& model, uint32_t levelId, QObject* parent = nullptr);
    virtual ~SMScene();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the level owner's element ID this scene displays.
     **/
    inline uint32_t getLevelId() const;

    /**
     * \brief   Returns the document facade.
     **/
    inline StateMachineModel& getModel() const;

    /**
     * \brief   The grid cell size in scene units; clamped to the allowed minimum.
     **/
    inline int getGridSize() const;
    void setGridSize(int gridSize);

    /**
     * \brief   The grid visibility.
     **/
    inline bool isGridVisible() const;
    void setGridVisible(bool visible);

    /**
     * \brief   The grid rendering style: lines (cell squares) or dots at the crossings.
     **/
    inline NESMDesign::eGridStyle getGridStyle() const;
    void setGridStyle(NESMDesign::eGridStyle style);

    /**
     * \brief   The dot diameter (device pixels) of the dotted grid style; clamped to the
     *          allowed range.
     **/
    inline int getGridDotSize() const;
    void setGridDotSize(int dotSize);

    /**
     * \brief   The snap-to-grid mode applied to interactive moves and resizes.
     **/
    inline bool isSnapToGrid() const;
    void setSnapToGrid(bool snap);

    /**
     * \brief   Returns true while a mouse drag is in progress and snapping is on;
     *          items consult this to snap their interactive position changes.
     **/
    inline bool isInteractiveSnap() const;

    /**
     * \brief   Snaps a point to the grid when snapping is enabled; identity otherwise.
     **/
    QPointF snappedPosition(const QPointF& position) const;

    /**
     * \brief   Returns the active tool mode.
     **/
    inline NESMDesign::eCanvasTool getActiveTool() const;

    /**
     * \brief   Activates a tool mode.
     * \param   tool    The tool mode; modes without an implementation fall back to Select.
     * \param   sticky  True keeps the tool active after a finished gesture,
     *                  false reverts to Select (single-shot).
     **/
    void setActiveTool(NESMDesign::eCanvasTool tool, bool sticky = false);

    /**
     * \brief   Returns true while the active tool is armed sticky (double-clicked button).
     **/
    inline bool isToolSticky() const;

    /**
     * \brief   Cancels the in-progress gesture and returns to the Select tool (Esc).
     **/
    void cancelActiveGesture();

    /**
     * \brief   Called by the active tool when its gesture completed; single-shot
     *          tools revert to Select. Holding Ctrl through the gesture repeats the
     *          tool once more (issue #541) without arming it sticky.
     **/
    void finishToolGesture();

    /**
     * \brief   Returns the graphics item of an element, or nullptr.
     **/
    inline SMCanvasItem* findCanvasItem(uint32_t elementId) const;

    /**
     * \brief   Returns the bounding rectangle of the level content (all items).
     **/
    QRectF contentBounds() const;

    /**
     * \brief   Selects every element on this level.
     **/
    void selectAll();

    /**
     * \brief   Returns the selected state box items of this level.
     **/
    QList<SMStateItem*> selectedStateItems() const;

    /**
     * \brief   Returns the selected transition edge items of this level.
     **/
    QList<SMEdgeItem*> selectedEdgeItems() const;

    /**
     * \brief   Returns the selected note items of this level.
     **/
    QList<SMNoteItem*> selectedNoteItems() const;

    /**
     * \brief   Returns the note item of an element, or nullptr.
     **/
    SMNoteItem* noteItem(uint32_t noteId) const;

    /**
     * \brief   Pushes one undo step moving/resizing every selected state box and note whose
     *          item position/size differs from its layout entry - the finished drag gesture
     *          of a (possibly mixed) multi-selection. Called by SMStateItem/SMNoteItem on a
     *          plain (non-resize) drag release.
     **/
    void commitSelectionMove(const QString& text);

    /**
     * \brief   Returns the state box item of an element, or nullptr.
     **/
    SMStateItem* stateItem(uint32_t stateId) const;

    /**
     * \brief   Returns the topmost state box item at a scene position, or nullptr.
     **/
    SMStateItem* stateAt(const QPointF& scenePos) const;

    /**
     * \brief   Returns the state box at a scene position or, failing that, the nearest one whose
     *          border lies within \p margin. Aiming at the exact border of a box is not a
     *          reasonable thing to ask of a pointing device, so a press that lands just outside
     *          one still starts (or finishes) a transition on it.
     **/
    SMStateItem* stateNear(const QPointF& scenePos, double margin) const;

    /**
     * \brief   Re-anchors every edge connected to a state after its box moved or resized.
     **/
    void updateEdgesForState(uint32_t stateId);

    /**
     * \brief   Applies a target-endpoint reconnection: retargets the transition to the state
     *          under the drop (empty drop offers making it internal). Deferred by the edge
     *          item so the resulting command may recreate the edge safely. The finished drop
     *          geometry is persisted in the same undo step so the endpoint lands where the mouse
     *          was released and the edge never flashes back to its old anchor first.
     * \param   transitionId    The reconnected transition's element ID.
     * \param   targetStateId   The new target state's element ID.
     * \param   geometry        The finished edge geometry from the drop (dragged endpoint pinned
     *                          to the drop border position, label reset to re-centre on the line).
     **/
    void reconnectTransitionTarget(uint32_t transitionId, uint32_t targetStateId, const SMLayoutEdge& geometry);

    /**
     * \brief   Applies a begin-endpoint reconnection: moves the transition to a new source
     *          state. A zero or unchanged source is ignored. The finished drop geometry is
     *          persisted in the same undo step (see \ref reconnectTransitionTarget).
     * \param   transitionId        The reconnected transition's element ID.
     * \param   newSourceStateId    The new source state's element ID.
     * \param   geometry            The finished edge geometry from the drop.
     **/
    void reparentTransition(uint32_t transitionId, uint32_t newSourceStateId, const SMLayoutEdge& geometry);

    /**
     * \brief   Opens the in-place name editor when exactly one state is selected (F2).
     **/
    void startRenameOfSelection();

    /**
     * \brief   True while a proxy-backed inline editor (state rename / note edit) holds the
     *          scene focus. The Design page consults this so its single-key tool shortcuts
     *          (S, F, T, N, Backspace, Delete, ...) never fire while the user types into the
     *          embedded editor, which would otherwise swallow the keystroke and spawn items.
     **/
    bool isInlineEditorActive() const;

    /**
     * \brief   Ends every open in-place editor, committing each as a focus-out would.
     **/
    void closeInlineEditors();

    /**
     * \brief   Requests descending into a state's painted submachine (double-click,
     *          Enter, context menu); ignored when the state owns none.
     **/
    void requestEnterSubmachine(uint32_t stateId);

    /**
     * \brief   Requests descending into a state's submachine, creating one on the fly when the
     *          state is a plain normal state with none (body double-click). Unlike
     *          requestEnterSubmachine, this is not gated on hasNestedStates: the owning Design
     *          page decides whether to create-and-enter or just enter.
     **/
    void requestSubstate(uint32_t stateId);

    /**
     * \brief   Requests the guard editor for a transition (double-click on the edge
     *          label): the owning page selects it and focuses the Conditions tab field.
     **/
    void requestGuardEdit(uint32_t transitionId);

    /**
     * \enum    eGotoScope
     * \brief   Which referenced declarations a go-to-declaration request targets. A Ctrl+Shift
     *          click on an edge label scopes to the part clicked; the whole-element triggers
     *          (F12, context menu) use \c GotoAll.
     **/
    enum eGotoScope
    {
          GotoAll = 0   //!< Every declaration the element references (whole-element trigger).
        , GotoStimulus  //!< Only the transition's stimulus (the trigger / event / timer that fires it).
        , GotoAction    //!< Only the transition's operations (action calls, sent events, started/stopped timers).
    };

    /**
     * \brief   Requests go-to-declaration for a canvas element (a Ctrl+Shift link click, or the
     *          whole-element F12 / context-menu trigger): the owning page resolves the registry
     *          declarations the element references and navigates (directly when there is one,
     *          picker when many).
     * \param   elementId   The state or transition the user acted on.
     * \param   isState     True when \p elementId is a state, false for a transition.
     * \param   scope       Which part of the element to resolve (see \ref eGotoScope). Carried as
     *                      int so it forwards cleanly through the scene-manager passthrough signal.
     **/
    void requestGotoDefinition(uint32_t elementId, bool isState, int scope = GotoAll);

    /**
     * \brief   Requests go-to-declaration for an explicit set of references (a Ctrl+Shift click on
     *          one state-body operation row): the owning page resolves them to declarations and
     *          navigates (directly when there is one, picker when several). No-op for an empty set.
     * \param   refs    The references the clicked row makes (kind + name).
     **/
    void requestGotoRefs(const QList<SMReferences::Ref>& refs);

    /**
     * \brief   Shows the submachine quick view over the composite state \p stateId (the Ctrl+Alt
     *          hover on its corner hint). The scene builds it, not the item: reading the nested
     *          level is a model read, and a canvas item holds no model data. Nothing is shown for
     *          a state without a real submachine.
     * \param   stateId     The composite state whose level is previewed.
     * \param   globalPos   The pointer position, in screen coordinates.
     **/
    void showSubmachinePeek(uint32_t stateId, const QPoint& globalPos);

    //!< Hides the submachine quick view; harmless when none is open.
    void hideSubmachinePeek();

//////////////////////////////////////////////////////////////////////////
// Internal: item registry (called by SMCanvasItem on scene changes)
//////////////////////////////////////////////////////////////////////////
public:
    void registerCanvasItem(SMCanvasItem& item);
    void unregisterCanvasItem(SMCanvasItem& item);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   Emitted when the grid size, visibility, or snapping changed.
     **/
    void signalGridChanged();

    /**
     * \brief   Emitted when the active tool mode changed.
     **/
    void signalToolChanged(NESMDesign::eCanvasTool tool);

    /**
     * \brief   Emitted to descend into a composite state's painted submachine.
     * \param   stateId The composite state's element ID.
     **/
    void signalEnterSubmachine(uint32_t stateId);

    /**
     * \brief   Emitted to descend into a state's submachine, creating one on the fly when the
     *          state has none (body double-click). The Design page handles the create-or-enter.
     * \param   stateId The state's element ID.
     **/
    void signalRequestSubstate(uint32_t stateId);

    /**
     * \brief   Emitted to ascend to the parent machine level (Backspace, Alt+double-click).
     **/
    void signalGoToParent();

    /**
     * \brief   Emitted to focus a transition's Conditions tab field (edge label double-click).
     **/
    void signalGuardEditRequested(uint32_t transitionId);

    /**
     * \brief   Emitted when a Ctrl+Shift link on a canvas element is clicked; the owning page
     *          navigates to the declaration(s) the scoped part references.
     **/
    void signalGotoDefinitionRequested(uint32_t elementId, bool isState, int scope);

    /**
     * \brief   Emitted when a Ctrl+Shift link on a single state-body operation row is clicked; the
     *          owning page resolves the row's references and navigates.
     **/
    void signalGotoRefsRequested(const QList<SMReferences::Ref>& refs);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

    /**
     * \brief   Paints the transition edge labels (stimulus, guard, operation summary) on top of
     *          every item, so a label that overlaps a state box stays readable regardless of the
     *          edge's z-order (the line itself still draws under the boxes).
     **/
    virtual void drawForeground(QPainter* painter, const QRectF& rect) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onSceneSelectionChanged();
    void onModelSelectionChanged(const QList<uint32_t>& selected);
    void onElementAdded(uint32_t id, eDocElementKind kind);
    void onElementRemoved(uint32_t id, eDocElementKind kind);
    void onElementChanged(uint32_t id, eDocElementKind kind);
    void onListReordered(uint32_t ownerId, eDocElementKind kind);
    void onNameChanged(uint32_t id, const QString& oldName, const QString& newName);
    void onLayoutChanged(const QList<uint32_t>& ownerIds);

private:
    /**
     * \brief   Moves the selected top-level items by one step (keyboard nudge) as one
     *          undo step; items without model backing are moved directly.
     * \return  True when a selection was moved.
     **/
    bool nudgeSelection(int dx, int dy, bool pixelWise);

    /**
     * \brief   When a single transition edge is selected and one of its interior waypoints is
     *          the active point, moves that waypoint by one keyboard step (issue #516 bug 4).
     * \param   dx      The horizontal direction: -1, 0, or +1.
     * \param   dy      The vertical direction: -1, 0, or +1.
     * \param   coarse  True for the 10-unit coarse step (Ctrl held).
     * \param   pixel   True for the exact 1-unit step (Shift held).
     * \return  True when an edge waypoint was moved (arrow keys are consumed by the edge).
     **/
    bool nudgeSelectedEdgePoint(int dx, int dy, bool coarse, bool pixel);

    /**
     * \brief   Creates the graphics items of every state of this level.
     **/
    void populateFromModel();

    /**
     * \brief   Creates the box item of one state (no-op when it already exists).
     **/
    void createStateItem(uint32_t stateId);

    /**
     * \brief   Creates the edge item of one external transition (no-op when it already
     *          exists or the transition is not external and on this level).
     **/
    void createEdgeItem(uint32_t transitionId);

    /**
     * \brief   Creates the box item of one free note (no-op when it already exists, is not
     *          on this level, or is bound to an owner - owned notes are drawn as badges).
     **/
    void createNoteItem(uint32_t noteId);

    /**
     * \brief   Re-reads every state box and transition edge so their note badges reflect a
     *          note add/remove/change on the owner (the note's own ID never names the owner).
     **/
    void refreshNoteBadges();

    /**
     * \brief   Returns the edge item of a transition, or nullptr.
     **/
    SMEdgeItem* edgeItem(uint32_t transitionId) const;

    /**
     * \brief   Re-reads every state box body (behaviour rows change when transitions do).
     **/
    void refreshStateBodies();

    /**
     * \brief   Re-reads every transition edge (a transition's operation summary is drawn on the
     *          edge, so an operation add/remove/change must refresh the edges, not only the boxes).
     **/
    void refreshEdges();

    /**
     * \brief   Re-reads every composite state box (the submachine miniature goes stale
     *          when elements of a nested level change).
     **/
    void refreshCompositeBoxes();

    /**
     * \brief   True when the state is a direct child of this scene's level.
     **/
    bool isOnThisLevel(uint32_t stateId) const;

    /**
     * \brief   Recomputes the incoming/outgoing highlight of every transition item
     *          connected to the selected states.
     **/
    void updateConnHighlights();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&              mModel;         //!< The document facade.
    const uint32_t                  mLevelId;       //!< The displayed level's owner element ID.
    QHash<uint32_t, SMCanvasItem*>  mItems;         //!< Element ID to graphics item.
    std::unique_ptr<SMCanvasTool>   mTool;          //!< The active tool strategy.
    std::unique_ptr<SMCanvasTool>   mRetiredTool;   //!< The replaced tool, kept alive until the
                                                    //!< next switch: a tool may retire itself from
                                                    //!< inside its own event handler.
    bool                            mToolSticky;    //!< Keep the tool after a finished gesture.
    Qt::KeyboardModifiers           mToolModifiers; //!< Modifiers of the last mouse event, read by
                                                    //!< finishToolGesture(): a tool completes from
                                                    //!< inside its own handler and has no event there.
    int                             mGridSize;      //!< The grid cell size in scene units.
    bool                            mGridVisible;   //!< The grid visibility.
    NESMDesign::eGridStyle          mGridStyle;     //!< The grid rendering style (lines or dots).
    int                             mGridDotSize;   //!< The dotted-grid dot diameter (device pixels).
    bool                            mSnapToGrid;    //!< Snap interactive moves to the grid.
    bool                            mMouseDrag;     //!< A mouse drag is in progress.
    bool                            mSyncSelection; //!< Guards the two-way selection sync.
    SMSubmachinePeek*               mPeek;          //!< The submachine quick view, built on first use.
};

//////////////////////////////////////////////////////////////////////////
// SMScene inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t SMScene::getLevelId() const
{
    return mLevelId;
}

inline StateMachineModel& SMScene::getModel() const
{
    return mModel;
}

inline int SMScene::getGridSize() const
{
    return mGridSize;
}

inline bool SMScene::isGridVisible() const
{
    return mGridVisible;
}

inline NESMDesign::eGridStyle SMScene::getGridStyle() const
{
    return mGridStyle;
}

inline int SMScene::getGridDotSize() const
{
    return mGridDotSize;
}

inline bool SMScene::isSnapToGrid() const
{
    return mSnapToGrid;
}

inline bool SMScene::isInteractiveSnap() const
{
    return mSnapToGrid && mMouseDrag;
}

inline NESMDesign::eCanvasTool SMScene::getActiveTool() const
{
    return (mTool != nullptr ? mTool->getKind() : NESMDesign::eCanvasTool::Select);
}

inline bool SMScene::isToolSticky() const
{
    return mToolSticky;
}

inline SMCanvasItem* SMScene::findCanvasItem(uint32_t elementId) const
{
    return mItems.value(elementId, nullptr);
}

#endif  // LUSAN_VIEW_SM_SMSCENE_HPP
