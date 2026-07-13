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
class StateMachineModel;
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
     * \brief   Cancels the in-progress gesture and returns to the Select tool (Esc).
     **/
    void cancelActiveGesture();

    /**
     * \brief   Called by the active tool when its gesture completed; single-shot
     *          tools revert to Select.
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
     * \brief   Re-anchors every edge connected to a state after its box moved or resized.
     **/
    void updateEdgesForState(uint32_t stateId);

    /**
     * \brief   Applies a target-endpoint reconnection: retargets the transition to the state
     *          under the drop (empty drop offers making it internal). Deferred by the edge
     *          item so the resulting command may recreate the edge safely.
     **/
    void reconnectTransitionTarget(uint32_t transitionId, uint32_t targetStateId);

    /**
     * \brief   Applies a begin-endpoint reconnection: moves the transition to a new source
     *          state. A zero or unchanged source is ignored.
     **/
    void reparentTransition(uint32_t transitionId, uint32_t newSourceStateId);

    /**
     * \brief   Opens the in-place name editor when exactly one state is selected (F2).
     **/
    void startRenameOfSelection();

    /**
     * \brief   Requests descending into a state's painted submachine (double-click,
     *          Enter, context menu); ignored when the state owns none.
     **/
    void requestEnterSubmachine(uint32_t stateId);

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
     * \brief   Emitted to ascend to the parent machine level (Backspace, Alt+double-click).
     **/
    void signalGoToParent();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
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
    int                             mGridSize;      //!< The grid cell size in scene units.
    bool                            mGridVisible;   //!< The grid visibility.
    NESMDesign::eGridStyle          mGridStyle;     //!< The grid rendering style (lines or dots).
    int                             mGridDotSize;   //!< The dotted-grid dot diameter (device pixels).
    bool                            mSnapToGrid;    //!< Snap interactive moves to the grid.
    bool                            mMouseDrag;     //!< A mouse drag is in progress.
    bool                            mSyncSelection; //!< Guards the two-way selection sync.
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

inline SMCanvasItem* SMScene::findCanvasItem(uint32_t elementId) const
{
    return mItems.value(elementId, nullptr);
}

#endif  // LUSAN_VIEW_SM_SMSCENE_HPP
