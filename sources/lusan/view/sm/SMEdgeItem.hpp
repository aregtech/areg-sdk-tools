#ifndef LUSAN_VIEW_SM_SMEDGEITEM_HPP
#define LUSAN_VIEW_SM_SMEDGEITEM_HPP
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
 *  \file        lusan/view/sm/SMEdgeItem.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas transition edge item.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/sm/SMCanvasItem.hpp"

#include "lusan/data/sm/SMLayoutData.hpp"
#include "lusan/view/sm/SMNoteEditor.hpp"

#include <QList>
#include <QPointF>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMScene;
class SMStateEntry;

/**
 * \class   SMEdgeItem
 * \brief   A transition edge on the design canvas: a begin dot on the source state border,
 *          a polyline (waypoints) or arc to the target state border, an arrowhead, and a
 *          draggable stimulus label. The begin/end anchors are re-derived from the current
 *          state box geometry, so moving a state only moves its anchor; interior waypoints
 *          stay put. Only external (and self) transitions have an edge -- internal ones are
 *          shown as a state-body row. The element ID is the item's single model link, and
 *          every edit it produces goes through an undo command.
 **/
class SMEdgeItem : public SMCanvasItem
{
//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \enum    eDrag
     * \brief   The part of the edge grabbed by the current mouse drag.
     **/
    enum class eDrag
    {
          None
        , Begin     //!< The begin anchor (source-endpoint reconnection).
        , End       //!< The end anchor (target-endpoint reconnection).
        , Waypoint  //!< An interior waypoint.
        , Label     //!< The stimulus label.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the edge item of a transition element.
     * \param   transitionId    The transition's document element ID.
     * \param   parent          The parent graphics item.
     **/
    explicit SMEdgeItem(uint32_t transitionId, QGraphicsItem* parent = nullptr);
    virtual ~SMEdgeItem();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The source (owner) state's element ID, or 0 when unresolved.
     **/
    inline uint32_t getSourceId() const;

    /**
     * \brief   The target state's element ID, or 0 when unresolved.
     **/
    inline uint32_t getTargetId() const;

    /**
     * \brief   Recomputes the begin/end anchors from the current source/target box
     *          geometry and repaints. Called when a connected state moves or resizes.
     **/
    void refreshAnchors();

    /**
     * \brief   The drawn polyline in scene coordinates (begin, waypoints, end).
     **/
    inline const QList<QPointF>& getPath() const;

    /**
     * \brief   True when a scene position lands on one of the selected edge's grab
     *          handles (endpoints, waypoints, or the label). The scene consults this
     *          before starting a border-band transition drag: the handle wins.
     **/
    bool hitsHandle(const QPointF& scenePos) const;

    /**
     * \brief   Opens the in-place note editor near the edge label for the note bound to this
     *          transition; commit (focus-out) pushes an undoable text change. No-op with no
     *          bound note.
     **/
    void startNoteEdit();

    /**
     * \brief   True when a note is bound to this transition (a note badge is shown).
     **/
    inline bool hasNote() const;

    /**
     * \brief   True when one interior waypoint is the active (keyboard-movable) point.
     **/
    inline bool hasSelectedPoint() const;

    /**
     * \brief   True while the label block is in reposition mode: a double-click on the trigger or
     *          operation text framed it and tethered it to the line, so mouse or arrow keys move it.
     **/
    inline bool hasActiveLabel() const;

    /**
     * \brief   True while a begin/end anchor is the active (keyboard-movable) endpoint.
     **/
    inline bool hasActiveEnd() const;

    /**
     * \brief   Moves the movable label block by one keyboard step, clamped so it stays within
     *          \ref NESMDesign::EdgeLabelMaxOffset of the line, and commits it as one undo step.
     *          Steps match \ref nudgeSelectedPoint (5 units, Ctrl = 10, Shift = 1 pixel).
     * \return  True when the label was active and moved (the event is consumed).
     **/
    bool nudgeLabel(int dx, int dy, bool coarse, bool pixelWise);

    /**
     * \brief   Moves the active begin/end endpoint by one keyboard step along the state border,
     *          re-sticking it to the nearest grid-aligned border position, and commits one undo
     *          step. Steps match \ref nudgeSelectedPoint.
     * \return  True when an endpoint was active and moved (the event is consumed).
     **/
    bool nudgeActiveEnd(int dx, int dy, bool coarse, bool pixelWise);

    /**
     * \brief   Moves the active interior waypoint by one keyboard step and commits it as one
     *          undo step. The begin/end anchors stay glued to the state borders and are never
     *          moved this way. Steps snap the moved coordinate to a grid of the step size: a
     *          normal step is 5 units (lands on the next multiple of 5 in the key direction),
     *          a coarse step (Ctrl) is 10 units (next multiple of 10), and a pixel step (Shift)
     *          moves exactly one unit without snapping (issue #516 bug 4).
     * \param   dx          The horizontal direction: -1, 0, or +1.
     * \param   dy          The vertical direction: -1, 0, or +1.
     * \param   coarse      True for the 10-unit coarse step (Ctrl held).
     * \param   pixelWise   True for the exact 1-unit step (Shift held); overrides \p coarse.
     * \return  True when an active waypoint was moved (the event is consumed).
     **/
    bool nudgeSelectedPoint(int dx, int dy, bool coarse, bool pixelWise);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    /**
     * \brief   Paints the edge's labels (stimulus signature, guard, and operation summary) plus
     *          the note badge, each in its own hue so the three read distinctly. Called by the
     *          scene's foreground pass so the labels stay above the state boxes and readable.
     **/
    void paintLabels(QPainter* painter, const QPalette& palette);

    /**
     * \brief   The scene-coordinate bounds of everything paintLabels draws (label + action +
     *          note badge), used by the scene to clip its foreground pass.
     **/
    QRectF labelBounds() const;

    /**
     * \brief   Re-reads the transition and its Edge layout: stimulus label, shape, bulge,
     *          waypoints, endpoints, and connected states.
     **/
    virtual void updateFromModel() override;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Returns the owning canvas scene, or nullptr.
     **/
    SMScene* getCanvas() const;

    /**
     * \brief   Returns the box geometry of a state (live item geometry when present,
     *          otherwise its Node layout entry); an empty rect when unresolved.
     **/
    QRectF stateRect(uint32_t stateId) const;

    /**
     * \brief   Returns the drawn corner radius of a state's box: the pill radius for
     *          Start / Final marker boxes, the standard corner radius otherwise.
     **/
    double stateRadius(uint32_t stateId, const QRectF& rect) const;

    /**
     * \brief   Rebuilds the drawn path and anchors from the cached model state, optionally
     *          forcing the dragged endpoint to a free point for live feedback.
     **/
    void rebuildPath();

    /**
     * \brief   The current geometry as a layout entry (begin, waypoints, end, shape, bulge,
     *          color, label) for persistence into a command.
     **/
    SMLayoutEdge buildGeometry() const;

    /**
     * \brief   Pushes the current geometry as one coalesced layout command.
     **/
    void commitGeometry(const QString& text);

    /**
     * \brief   The anchor point (item/scene coordinates) the labels hang from: the persisted
     *          label position when the user placed one, otherwise the polyline midpoint.
     **/
    QPointF labelAnchor() const;

    /**
     * \brief   The stimulus label rectangle in item (scene) coordinates. For a default
     *          (un-dragged) edge the label sits just above the line so a horizontal edge does
     *          not strike through the text; once the user drags it, it centers on that point.
     **/
    QRectF labelRect() const;

    /**
     * \brief   The action-summary rectangle just below the transition line (empty when the
     *          transition has no operations).
     **/
    QRectF actionRect() const;

    /**
     * \brief   The label text shown for the stimulus + guard: the stimulus signature followed by
     *          the `[guard]` clause. Empty for a transition from the Start pseudo-state (which
     *          fires automatically and has no stimulus); a `<stimulus>` hint otherwise when unset.
     **/
    QString labelText() const;

    /**
     * \brief   The note-badge rectangle (item/scene coordinates), just right of the label;
     *          valid only when the transition has a bound note.
     **/
    QRectF noteBadgeRect() const;

    /**
     * \brief   Paints the arrowhead pointing from \p from to \p tip.
     **/
    void paintArrowHead(QPainter* painter, const QPointF& from, const QPointF& tip, const QColor& color);

    /**
     * \brief   The stroke color for the current state (theme, selection, or highlight).
     **/
    QColor strokeColor(const QPalette& palette) const;

    /**
     * \brief   Sets the active interior waypoint for keyboard nudging (or -1 to clear) and
     *          repaints when it changed.
     **/
    void setSelectedPoint(int index);

    /**
     * \brief   Finds the interior waypoint within pick range of \p point, or -1.
     **/
    int hitWaypoint(const QPointF& point) const;

    /**
     * \brief   The point on the drawn polyline closest to \p point (the tether target of the
     *          movable label, and the clamp reference that keeps the label near its line).
     **/
    QPointF nearestPathPoint(const QPointF& point) const;

    /**
     * \brief   Clamps a candidate label anchor so it never sits farther than
     *          \ref NESMDesign::EdgeLabelMaxOffset from the nearest point of the line.
     **/
    QPointF clampLabelPos(const QPointF& candidate) const;

    /**
     * \brief   Enters (or leaves) label reposition mode: frames the label and tethers it to the
     *          line so mouse and arrow keys can move it. Seeds a label anchor on first entry.
     **/
    void setLabelActive(bool active);

    /**
     * \brief   Sets the active (keyboard-movable) endpoint: 0 none, 1 begin, 2 end.
     **/
    void setActiveEnd(int which);

    /**
     * \brief   Finds the path segment within tolerance of \p point and the projected point
     *          on it; returns the interior insert position, or -1 when none is close.
     **/
    int hitSegment(const QPointF& point, QPointF& projected) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    uint32_t                mSourceId;      //!< The source (owner) state ID.
    uint32_t                mTargetId;      //!< The target state ID (== source for self).
    QString                 mTargetName;    //!< The target state name (`To`).
    bool                    mSelfLoop;      //!< The transition targets its own source.
    bool                    mValid;         //!< The transition resolved to an external edge.
    SMLayoutEdge::eShape    mShape;         //!< The edge shape (Line or Arc).
    double                  mBulge;         //!< The arc bulge factor.
    QString                 mColorName;     //!< The persisted edge color (empty = theme).
    QString                 mStimulusText;  //!< The stimulus signature label text (`walk(count)`).
    QString                 mGuardText;     //!< The `[guard]` clause drawn after the stimulus, or empty.
    QString                 mActionText;    //!< The operation summary drawn below the line.
    int                     mGuardSeverity; //!< The guard's NEGuardStyle severity for the label tint, or -1 (clean).
    int                     mActionSeverity;//!< The operation mapping's NEGuardStyle severity for the action tint, or -1 (clean).
    bool                    mSourceIsStart; //!< The source is the Start pseudo-state (no stimulus placeholder).
    bool                    mHasNote;       //!< A note is bound to this transition (badge shown).
    SMNoteEditor            mNoteEditor;    //!< The open in-place note editor (if any).
    QList<QPointF>          mWaypoints;     //!< The interior waypoints, in order.
    bool                    mHasAnchors;    //!< Persisted begin/end anchor points exist.
    QPointF                 mAnchorBegin;   //!< The persisted begin anchor (projected onto the live border).
    QPointF                 mAnchorEnd;     //!< The persisted end anchor (projected onto the live border).
    QPointF                 mBegin;         //!< The begin anchor on the source border.
    QPointF                 mEnd;           //!< The end anchor on the target border.
    QList<QPointF>          mPath;          //!< The full drawn polyline (begin, waypoints, end).
    bool                    mHasLabel;      //!< Whether a label position is persisted.
    QPointF                 mLabelPos;      //!< The persisted label anchor.
    bool                    mLabelActive;   //!< The label block is in reposition mode (framed + tethered).
    eDrag                   mDrag;          //!< The active drag part.
    int                     mDragIndex;     //!< The dragged waypoint index.
    int                     mSelectedPoint; //!< The active interior waypoint for keyboard nudging, or -1.
    int                     mActiveEnd;     //!< The active endpoint for keyboard nudging: 0 none, 1 begin, 2 end.
    QPointF                 mDragPoint;     //!< The live free position of the dragged endpoint.
    uint32_t                mGesture;       //!< The coalescing gesture ID of the active drag.
};

//////////////////////////////////////////////////////////////////////////
// SMEdgeItem inline methods
//////////////////////////////////////////////////////////////////////////

inline bool SMEdgeItem::hasNote() const
{
    return mHasNote;
}

inline bool SMEdgeItem::hasSelectedPoint() const
{
    return (mSelectedPoint >= 0);
}

inline bool SMEdgeItem::hasActiveLabel() const
{
    return mLabelActive;
}

inline bool SMEdgeItem::hasActiveEnd() const
{
    return (mActiveEnd != 0);
}

inline uint32_t SMEdgeItem::getSourceId() const
{
    return mSourceId;
}

inline uint32_t SMEdgeItem::getTargetId() const
{
    return mTargetId;
}

inline const QList<QPointF>& SMEdgeItem::getPath() const
{
    return mPath;
}

#endif  // LUSAN_VIEW_SM_SMEDGEITEM_HPP
