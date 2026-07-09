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
 *  \file        lusan/view/sm/SMEdgeItem.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas transition edge item.
 *
 ************************************************************************/

#include "lusan/view/sm/SMEdgeItem.hpp"

#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QCoreApplication>
#include <QFont>
#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPalette>
#include <QTimer>
#include <QWidget>

#include <algorithm>
#include <cmath>

namespace
{
    inline QString translate(const char* text)
    {
        return QCoreApplication::translate("SMEdgeItem", text);
    }

    //!< The straight-line distance between two points.
    inline double distance(const QPointF& a, const QPointF& b)
    {
        return std::hypot(a.x() - b.x(), a.y() - b.y());
    }

    //!< The distance from a point to a segment, and the closest point on it.
    double segmentDistance(const QPointF& p, const QPointF& a, const QPointF& b, QPointF& closest)
    {
        const QPointF ab = b - a;
        const double  len2 = ab.x() * ab.x() + ab.y() * ab.y();
        double t = 0.0;
        if (len2 > 1e-9)
        {
            t = ((p.x() - a.x()) * ab.x() + (p.y() - a.y()) * ab.y()) / len2;
            t = std::clamp(t, 0.0, 1.0);
        }

        closest = a + ab * t;
        return distance(p, closest);
    }
}

SMEdgeItem::SMEdgeItem(uint32_t transitionId, QGraphicsItem* parent /*= nullptr*/)
    : SMCanvasItem  (transitionId, parent)
    , mSourceId     (0)
    , mTargetId     (0)
    , mTargetName   ( )
    , mSelfLoop     (false)
    , mValid        (false)
    , mShape        (SMLayoutEdge::eShape::Line)
    , mBulge        (0.0)
    , mColorName    ( )
    , mStimulusText ( )
    , mWaypoints    ( )
    , mBegin        ( )
    , mEnd          ( )
    , mPath         ( )
    , mHasLabel     (false)
    , mLabelPos     ( )
    , mDrag         (eDrag::None)
    , mDragIndex    (-1)
    , mDragPoint    ( )
    , mGesture      (0)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptedMouseButtons(Qt::LeftButton);
    // Edges paint under the state boxes so borders and badges stay readable.
    setZValue(-1.0);
}

SMEdgeItem::~SMEdgeItem()
{
}

SMScene* SMEdgeItem::getCanvas() const
{
    return qobject_cast<SMScene*>(scene());
}

QRectF SMEdgeItem::stateRect(uint32_t stateId) const
{
    SMScene* canvas = getCanvas();
    if ((canvas == nullptr) || (stateId == 0))
    {
        return QRectF();
    }

    SMStateItem* item = canvas->stateItem(stateId);
    if (item != nullptr)
    {
        return item->getBoxGeometry();
    }

    const SMLayoutNode* node = canvas->getModel().getData().getLayout().findNode(stateId);
    if (node != nullptr)
    {
        return QRectF(  node->x, node->y
                      , std::max(node->width, NESMDesign::StateMinWidth)
                      , std::max(node->height, NESMDesign::StateMinHeight));
    }

    return QRectF();
}

void SMEdgeItem::updateFromModel()
{
    SMScene* canvas = getCanvas();
    if (canvas == nullptr)
    {
        return;
    }

    StateMachineData& data = canvas->getModel().getData();
    const SMTransitionEntry* transition = data.findTransitionById(getElementId());
    if ((transition == nullptr) || (transition->isExternal() == false))
    {
        mValid = false;
        return;
    }

    const SMStateEntry* owner = data.findTransitionOwner(getElementId());
    mSourceId    = (owner != nullptr ? owner->getId() : 0);
    mTargetName  = transition->getTo();
    const SMStateEntry* target = data.findState(mTargetName);
    mTargetId    = (target != nullptr ? target->getId() : 0);
    mSelfLoop    = (mTargetId != 0) && (mTargetId == mSourceId);
    mStimulusText = transition->getStimulus();

    const SMLayoutEdge* edge = data.getLayout().findEdge(getElementId());
    mShape     = (edge != nullptr ? edge->shape : SMLayoutEdge::eShape::Line);
    mBulge     = (edge != nullptr ? edge->bulge : 0.0);
    mColorName = (edge != nullptr ? edge->color : QString());
    mHasLabel  = (edge != nullptr) && edge->hasLabel;
    mLabelPos  = (edge != nullptr ? edge->label : QPointF());

    mWaypoints.clear();
    if ((edge != nullptr) && (edge->points.size() > 2))
    {
        for (int i = 1; i < edge->points.size() - 1; ++i)
        {
            mWaypoints.append(edge->points.at(i));
        }
    }

    mValid = (mSourceId != 0);
    prepareGeometryChange();
    rebuildPath();
    update();
}

void SMEdgeItem::refreshAnchors()
{
    if (mValid)
    {
        prepareGeometryChange();
        rebuildPath();
        update();
    }
}

void SMEdgeItem::rebuildPath()
{
    mPath.clear();
    if (mValid == false)
    {
        return;
    }

    QRectF src = stateRect(mSourceId);
    if ((src.width() <= 0.0) || (src.height() <= 0.0))
    {
        return;
    }

    const QPointF sc = src.center();
    QRectF tgt = (mSelfLoop ? src : stateRect(mTargetId));
    if ((tgt.width() <= 0.0) || (tgt.height() <= 0.0))
    {
        // Dangling target: draw toward a placeholder box beside the source.
        tgt = QRectF(sc + QPointF(160.0, 0.0) - QPointF(src.width() / 2.0, src.height() / 2.0), src.size());
    }

    const QPointF tc = tgt.center();

    // A self-loop with no stored waypoints gets a default loop above the box.
    if (mSelfLoop && mWaypoints.isEmpty())
    {
        const double off = 44.0;
        mWaypoints.append(QPointF(src.center().x() - 22.0, src.top() - off));
        mWaypoints.append(QPointF(src.center().x() + 22.0, src.top() - off));
    }

    if ((mShape == SMLayoutEdge::eShape::Arc) && (mSelfLoop == false))
    {
        mBegin = (mDrag == eDrag::Begin) ? mDragPoint : NESMDesign::borderPoint(src, tc);
        mEnd   = (mDrag == eDrag::End)   ? mDragPoint : NESMDesign::borderPoint(tgt, sc);
        mPath  = NESMDesign::arcPolyline(mBegin, mEnd, mBulge, NESMDesign::EdgeArcSamples);
    }
    else
    {
        const QPointF beginRef = mWaypoints.isEmpty() ? tc : mWaypoints.first();
        const QPointF endRef   = mWaypoints.isEmpty() ? sc : mWaypoints.last();
        mBegin = (mDrag == eDrag::Begin) ? mDragPoint : NESMDesign::borderPoint(src, beginRef);
        mEnd   = (mDrag == eDrag::End)   ? mDragPoint : NESMDesign::borderPoint(tgt, endRef);

        mPath.append(mBegin);
        mPath.append(mWaypoints);
        mPath.append(mEnd);
    }
}

QRectF SMEdgeItem::labelRect() const
{
    if ((mValid == false) || mPath.isEmpty())
    {
        return QRectF();
    }

    QPointF anchor;
    if (mHasLabel)
    {
        anchor = mLabelPos;
    }
    else
    {
        // Halfway along the drawn polyline.
        double total = 0.0;
        for (int i = 1; i < mPath.size(); ++i)
        {
            total += distance(mPath.at(i - 1), mPath.at(i));
        }

        double half = total / 2.0;
        anchor = mPath.first();
        for (int i = 1; i < mPath.size(); ++i)
        {
            const double seg = distance(mPath.at(i - 1), mPath.at(i));
            if (seg >= half)
            {
                const double t = (seg > 1e-6 ? half / seg : 0.0);
                anchor = mPath.at(i - 1) + (mPath.at(i) - mPath.at(i - 1)) * t;
                break;
            }

            half -= seg;
        }
    }

    const QString text = mStimulusText.isEmpty() ? translate("<stimulus>") : mStimulusText;
    const QFontMetricsF metrics{ QFont() };
    const QSizeF size = metrics.size(0, text) + QSizeF(6.0, 2.0);
    return QRectF(anchor - QPointF(size.width() / 2.0, size.height() / 2.0), size);
}

QRectF SMEdgeItem::boundingRect() const
{
    if (mPath.isEmpty())
    {
        return QRectF();
    }

    double minX = mPath.first().x();
    double minY = mPath.first().y();
    double maxX = minX;
    double maxY = minY;
    for (const QPointF& point : mPath)
    {
        minX = std::min(minX, point.x());
        minY = std::min(minY, point.y());
        maxX = std::max(maxX, point.x());
        maxY = std::max(maxY, point.y());
    }

    QRectF rect{ QPointF(minX, minY), QPointF(maxX, maxY) };
    rect = rect.united(labelRect());

    const double margin = std::max({ NESMDesign::EdgeArrowLength, NESMDesign::WaypointHandleSize, NESMDesign::EndpointPickRadius }) + 2.0;
    return rect.adjusted(-margin, -margin, margin, margin);
}

QPainterPath SMEdgeItem::shape() const
{
    QPainterPath path;
    if (mPath.size() >= 2)
    {
        path.moveTo(mPath.first());
        for (int i = 1; i < mPath.size(); ++i)
        {
            path.lineTo(mPath.at(i));
        }
    }

    QPainterPathStroker stroker;
    stroker.setWidth(8.0);
    QPainterPath result = stroker.createStroke(path);
    result.addRect(labelRect());
    result.addEllipse(mBegin, NESMDesign::EndpointPickRadius, NESMDesign::EndpointPickRadius);
    result.addEllipse(mEnd, NESMDesign::EndpointPickRadius, NESMDesign::EndpointPickRadius);
    for (const QPointF& wp : mWaypoints)
    {
        result.addEllipse(wp, NESMDesign::EndpointPickRadius, NESMDesign::EndpointPickRadius);
    }

    return result;
}

QColor SMEdgeItem::strokeColor(const QPalette& palette) const
{
    if (isSelected())
    {
        return NESMDesign::selectionColor(palette);
    }

    switch (getConnHighlight())
    {
    case eConnHighlight::Outgoing:
        return NESMDesign::edgeOutgoingColor(palette);
    case eConnHighlight::Incoming:
    case eConnHighlight::Both:
        return NESMDesign::edgeIncomingColor(palette);
    case eConnHighlight::None:
    default:
        break;
    }

    QColor custom{ mColorName };
    return (custom.isValid() ? custom : NESMDesign::edgeColor(palette));
}

void SMEdgeItem::paintArrowHead(QPainter* painter, const QPointF& from, const QPointF& tip, const QColor& color)
{
    QPointF dir = tip - from;
    const double len = std::hypot(dir.x(), dir.y());
    if (len < 1e-6)
    {
        return;
    }

    dir /= len;
    const QPointF normal{ -dir.y(), dir.x() };
    const QPointF base = tip - dir * NESMDesign::EdgeArrowLength;
    const QPointF left  = base + normal * NESMDesign::EdgeArrowHalfWidth;
    const QPointF right = base - normal * NESMDesign::EdgeArrowHalfWidth;

    QPainterPath head;
    head.moveTo(tip);
    head.lineTo(left);
    head.lineTo(right);
    head.closeSubpath();

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPath(head);
}

void SMEdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* widget)
{
    if ((mValid == false) || (mPath.size() < 2))
    {
        return;
    }

    const QPalette palette{ (widget != nullptr) ? widget->palette() : QPalette() };
    const QColor   color = strokeColor(palette);

    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen pen{ color, NESMDesign::EdgeLineWidth };
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    if (isSelected() || (getConnHighlight() != eConnHighlight::None))
    {
        pen.setWidthF(NESMDesign::EdgeLineWidth + 0.8);
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(mPath.constData(), mPath.size());

    // Begin dot on the source border.
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawEllipse(mBegin, NESMDesign::EdgeBeginDotRadius, NESMDesign::EdgeBeginDotRadius);

    paintArrowHead(painter, mPath.at(mPath.size() - 2), mEnd, color);

    // Stimulus label.
    const QRectF label = labelRect();
    painter->setPen(color);
    painter->setBrush(Qt::NoBrush);
    painter->drawText(label, Qt::AlignCenter, mStimulusText.isEmpty() ? translate("<stimulus>") : mStimulusText);

    if (isSelected())
    {
        painter->setPen(QPen(palette.color(QPalette::Base), 1.0));
        painter->setBrush(NESMDesign::selectionColor(palette));
        const double h = NESMDesign::WaypointHandleSize;
        for (const QPointF& wp : mWaypoints)
        {
            painter->drawRect(QRectF(wp.x() - h / 2.0, wp.y() - h / 2.0, h, h));
        }

        painter->setBrush(palette.color(QPalette::Base));
        painter->setPen(QPen(NESMDesign::selectionColor(palette), 1.4));
        painter->drawEllipse(mBegin, h / 2.0, h / 2.0);
        painter->drawEllipse(mEnd, h / 2.0, h / 2.0);
    }
}

int SMEdgeItem::hitWaypoint(const QPointF& point) const
{
    for (int i = 0; i < mWaypoints.size(); ++i)
    {
        if (distance(point, mWaypoints.at(i)) <= NESMDesign::EndpointPickRadius)
        {
            return i;
        }
    }

    return -1;
}

int SMEdgeItem::hitSegment(const QPointF& point, QPointF& projected) const
{
    int    best = -1;
    double bestDistance = NESMDesign::SegmentPickTolerance;
    for (int i = 1; i < mPath.size(); ++i)
    {
        QPointF closest;
        const double dist = segmentDistance(point, mPath.at(i - 1), mPath.at(i), closest);
        if (dist <= bestDistance)
        {
            bestDistance = dist;
            best = i - 1;   // interior insert position for a Line path
            projected = closest;
        }
    }

    return best;
}

SMLayoutEdge SMEdgeItem::buildGeometry() const
{
    SMLayoutEdge edge;
    edge.owner = getElementId();
    edge.shape = mShape;
    edge.bulge = mBulge;
    edge.color = mColorName;
    edge.points.append(mBegin);
    edge.points.append(mWaypoints);
    edge.points.append(mEnd);
    edge.hasLabel = mHasLabel;
    edge.label    = mLabelPos;
    return edge;
}

void SMEdgeItem::commitGeometry(const QString& text)
{
    SMScene* canvas = getCanvas();
    if (canvas == nullptr)
    {
        return;
    }

    StateMachineModel& model = canvas->getModel();
    model.getUndoStack().push(new SMSetEdgeGeometryCommand(  model.getData(), model.getNotifier()
                                                          , getElementId(), mGesture, buildGeometry(), text));
}

void SMEdgeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if ((event->button() == Qt::LeftButton) && isSelected())
    {
        const QPointF p = event->pos();
        if (distance(p, mBegin) <= NESMDesign::EndpointPickRadius)
        {
            mDrag = eDrag::Begin;
            mDragPoint = mBegin;
            mGesture = SMMoveNodeCommand::takeNextGesture();
            event->accept();
            return;
        }

        if (distance(p, mEnd) <= NESMDesign::EndpointPickRadius)
        {
            mDrag = eDrag::End;
            mDragPoint = mEnd;
            mGesture = SMMoveNodeCommand::takeNextGesture();
            event->accept();
            return;
        }

        const int wp = hitWaypoint(p);
        if (wp >= 0)
        {
            mDrag = eDrag::Waypoint;
            mDragIndex = wp;
            mGesture = SMMoveNodeCommand::takeNextGesture();
            event->accept();
            return;
        }

        if (labelRect().contains(p))
        {
            mDrag = eDrag::Label;
            mGesture = SMMoveNodeCommand::takeNextGesture();
            event->accept();
            return;
        }
    }

    SMCanvasItem::mousePressEvent(event);
}

void SMEdgeItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (mDrag == eDrag::None)
    {
        SMCanvasItem::mouseMoveEvent(event);
        return;
    }

    SMScene* canvas = getCanvas();
    const QPointF snapped = (canvas != nullptr ? canvas->snappedPosition(event->scenePos()) : event->scenePos());

    prepareGeometryChange();
    switch (mDrag)
    {
    case eDrag::Waypoint:
        mWaypoints[mDragIndex] = snapped;
        rebuildPath();
        break;

    case eDrag::Begin:
    case eDrag::End:
        // Free follow for reconnection feedback (endpoints do not grid-snap).
        mDragPoint = event->scenePos();
        rebuildPath();
        break;

    case eDrag::Label:
        mLabelPos = snapped;
        mHasLabel = true;
        break;

    default:
        break;
    }

    update();
    event->accept();
}

void SMEdgeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (mDrag == eDrag::None)
    {
        SMCanvasItem::mouseReleaseEvent(event);
        return;
    }

    const eDrag drag = mDrag;
    mDrag = eDrag::None;
    mDragIndex = -1;

    SMScene* canvas = getCanvas();
    switch (drag)
    {
    case eDrag::Waypoint:
        commitGeometry(translate("Move waypoint"));
        break;

    case eDrag::Label:
        commitGeometry(translate("Move edge label"));
        break;

    case eDrag::End:
    case eDrag::Begin:
        if (canvas != nullptr)
        {
            SMStateItem* over = canvas->stateAt(event->scenePos());
            const uint32_t overId = (over != nullptr ? over->getElementId() : 0);
            const uint32_t tid = getElementId();
            // Revert the drag feedback; the scene applies the reconnection deferred so
            // this item can be safely deleted/recreated by the resulting command.
            updateFromModel();
            if (drag == eDrag::End)
            {
                QTimer::singleShot(0, canvas, [canvas, tid, overId]() { canvas->reconnectTransitionTarget(tid, overId); });
            }
            else
            {
                QTimer::singleShot(0, canvas, [canvas, tid, overId]() { canvas->reparentTransition(tid, overId); });
            }
        }
        break;

    default:
        break;
    }

    event->accept();
}

void SMEdgeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const QPointF p = event->pos();

        // Merge (remove) a waypoint within the small hit threshold; begin/end are never hit.
        for (int i = 0; i < mWaypoints.size(); ++i)
        {
            if (distance(p, mWaypoints.at(i)) <= NESMDesign::WaypointMergeRadius)
            {
                prepareGeometryChange();
                mWaypoints.removeAt(i);
                rebuildPath();
                mGesture = SMMoveNodeCommand::takeNextGesture();
                commitGeometry(translate("Remove waypoint"));
                event->accept();
                return;
            }
        }

        // Break (insert) a waypoint on the nearest segment.
        QPointF projected;
        const int segment = hitSegment(p, projected);
        if (segment >= 0)
        {
            SMScene* canvas = getCanvas();
            const QPointF drop = (canvas != nullptr ? canvas->snappedPosition(projected) : projected);
            prepareGeometryChange();
            if (mShape == SMLayoutEdge::eShape::Arc)
            {
                // An arc uses exactly two points; a waypoint turns it into a polyline.
                mShape = SMLayoutEdge::eShape::Line;
                mBulge = 0.0;
                mWaypoints.clear();
                mWaypoints.append(drop);
            }
            else
            {
                mWaypoints.insert(segment, drop);
            }

            rebuildPath();
            mGesture = SMMoveNodeCommand::takeNextGesture();
            commitGeometry(translate("Add waypoint"));
            event->accept();
            return;
        }
    }

    SMCanvasItem::mouseDoubleClickEvent(event);
}
