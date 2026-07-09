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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
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
 *          stay put. Only external (and self) transitions have an edge — internal ones are
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

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

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
     * \brief   The label rectangle in item (scene) coordinates.
     **/
    QRectF labelRect() const;

    /**
     * \brief   Paints the arrowhead pointing from \p from to \p tip.
     **/
    void paintArrowHead(QPainter* painter, const QPointF& from, const QPointF& tip, const QColor& color);

    /**
     * \brief   The stroke color for the current state (theme, selection, or highlight).
     **/
    QColor strokeColor(const QPalette& palette) const;

    /**
     * \brief   Finds the interior waypoint within pick range of \p point, or -1.
     **/
    int hitWaypoint(const QPointF& point) const;

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
    QString                 mStimulusText;  //!< The stimulus label text.
    QList<QPointF>          mWaypoints;     //!< The interior waypoints, in order.
    QPointF                 mBegin;         //!< The begin anchor on the source border.
    QPointF                 mEnd;           //!< The end anchor on the target border.
    QList<QPointF>          mPath;          //!< The full drawn polyline (begin, waypoints, end).
    bool                    mHasLabel;      //!< Whether a label position is persisted.
    QPointF                 mLabelPos;      //!< The persisted label anchor.
    eDrag                   mDrag;          //!< The active drag part.
    int                     mDragIndex;     //!< The dragged waypoint index.
    QPointF                 mDragPoint;     //!< The live free position of the dragged endpoint.
    uint32_t                mGesture;       //!< The coalescing gesture ID of the active drag.
};

//////////////////////////////////////////////////////////////////////////
// SMEdgeItem inline methods
//////////////////////////////////////////////////////////////////////////

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
