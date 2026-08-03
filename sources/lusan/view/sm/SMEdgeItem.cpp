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
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMOperationSummary.hpp"
#include "lusan/model/sm/SMGuardValidation.hpp"
#include "lusan/model/sm/SMOperationValidation.hpp"
#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QCoreApplication>
#include <QCursor>
#include <QFont>
#include <QFontMetricsF>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPalette>
#include <QTimer>
#include <QToolTip>
#include <QWidget>

#include <algorithm>
#include <cmath>

namespace
{
    inline QString translate(const char* text)
    {
        return QCoreApplication::translate("SMEdgeItem", text);
    }

    //!< The (slightly smaller) font of the on-canvas edge labels, in one place.
    inline QFont labelFont()
    {
        return NESMDesign::scaledFont(QFont(), NESMDesign::EdgeLabelFontScale);
    }

    //!< The room the stimulus kind mark reserves in front of the label text, mark plus its gap.
    //!< Zero when nothing is drawn, so the label geometry collapses to plain text by itself.
    inline double markWidth(SMKindGlyph::eGlyph glyph)
    {
        return SMKindGlyph::isDrawn(glyph) ? (SMKindGlyph::GlyphSize + 2.0) : 0.0;
    }

    //!< The straight-line distance between two points.
    inline double distance(const QPointF& a, const QPointF& b)
    {
        return std::hypot(a.x() - b.x(), a.y() - b.y());
    }

    //!< Moves one coordinate by a single keyboard step. A pixel-wise step moves exactly one
    //!< unit; otherwise the result snaps to the next multiple of \a base strictly beyond the
    //!< current value in the \a dir direction (e.g. base 5: 3 -> 5 -> 10, or 3 -> 0 -> -5).
    double nudgeAxis(double value, int dir, int base, bool pixelWise)
    {
        if (dir == 0)
        {
            return value;
        }

        if (pixelWise)
        {
            return value + static_cast<double>(dir);
        }

        constexpr double eps = 1e-6;
        const double cells = value / static_cast<double>(base);
        const double target = (dir > 0) ? (std::floor(cells + eps) + 1.0)
                                        : (std::ceil (cells - eps) - 1.0);
        return target * static_cast<double>(base);
    }

    //!< Two points that differ by less than a thousandth of a scene unit are the same point.
    //!< Anchors are re-derived from a box origin every rebuild, so bit-exact comparison would
    //!< report a move where the arithmetic merely rounded differently.
    inline bool samePoint(const QPointF& a, const QPointF& b)
    {
        return (std::abs(a.x() - b.x()) < 1e-3) && (std::abs(a.y() - b.y()) < 1e-3);
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
    , mStimulusGlyph(SMKindGlyph::eGlyph::None)
    , mGuardText    ( )
    , mGuardSeverity(-1)
    , mActionSeverity(-1)
    , mSourceIsStart(false)
    , mIsInitial    (false)
    , mHasNote      (false)
    , mWaypoints    ( )
    , mHasAnchors   (false)
    , mAnchorBegin  ( )
    , mAnchorEnd    ( )
    , mAnchorsMeasured(false)
    , mAnchorBeginRel( )
    , mAnchorEndRel ( )
    , mHasOrigins   (false)
    , mSrcOrigin    ( )
    , mTgtOrigin    ( )
    , mBegin        ( )
    , mEnd          ( )
    , mPath         ( )
    , mHasLabel     (false)
    , mLabelPos     ( )
    , mLabelActive  (false)
    , mDrag         (eDrag::None)
    , mDragIndex    (-1)
    , mSelectedPoint(-1)
    , mActiveEnd    (0)
    , mDragPoint    ( )
    , mGesture      (0)
    , mHoverLink    (LinkNone)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);     // drives the Ctrl+Shift label links (go-to-declaration).
    // A transition line always paints ABOVE the inactive state boxes so a box never hides it
    // (z = 1 vs a box's z = 0). A selected box raises to z = 2 to cover the inactive edges, and a
    // selected edge raises to z = 3 so it -- and its grab handles -- stay on top of everything.
    setZValue(1.0);
}

SMEdgeItem::~SMEdgeItem()
{
}

SMScene* SMEdgeItem::getCanvas() const
{
    return qobject_cast<SMScene*>(scene());
}

QRectF SMEdgeItem::stateBoxRect(uint32_t stateId) const
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
        return item->getVisibleGeometry();
    }

    // No live item (a box of another level): the collapsed flag has to be read from the layout.
    // Start / Final markers have no body to collapse and are never cut down (see SMStateItem).
    QRectF rect = stateBoxRect(stateId);
    const SMLayoutNode* node = canvas->getModel().getData().getLayout().findNode(stateId);
    const SMStateEntry* state = canvas->getModel().getData().findStateById(stateId);
    const bool marker = (state != nullptr) && (state->getKind() != SMStateEntry::eStateKind::Normal);
    if ((rect.isEmpty() == false) && (marker == false)
        && (node != nullptr) && node->hasExpanded && (node->expanded == false))
    {
        rect.setHeight(NESMDesign::StateHeaderHeight);
    }

    return rect;
}

QPointF SMEdgeItem::borderAnchorPoint(uint32_t stateId, const QRectF& drawn, const QPointF& anchor) const
{
    if ((drawn.width() <= 0.0) || (drawn.height() <= 0.0))
    {
        return anchor;
    }

    const double radius = stateRadius(stateId, drawn);
    const QRectF stored = stateBoxRect(stateId);
    if ((stored.width() <= 0.0) || (stored.height() <= 0.0) || (std::abs(stored.height() - drawn.height()) < 1e-6))
    {
        return NESMDesign::nearestBorderPoint(drawn, radius, anchor);
    }

    // Only the height differs, so the drawn box keeps the same four sides -- one of them shorter.
    // Take the side of the stored box the anchor belongs to and stay on it.
    const QPointF onStored = NESMDesign::nearestBorderPoint(stored, stateRadius(stateId, stored), anchor);
    const QPointF normal   = NESMDesign::borderOutwardNormal(stored, onStored);
    const double  rad      = std::clamp(radius, 0.0, std::min(drawn.width(), drawn.height()) / 2.0);
    if (normal.y() < 0.0)
    {
        return QPointF(std::clamp(onStored.x(), drawn.left() + rad, drawn.right() - rad), drawn.top());
    }
    else if (normal.y() > 0.0)
    {
        return QPointF(std::clamp(onStored.x(), drawn.left() + rad, drawn.right() - rad), drawn.bottom());
    }
    else if (normal.x() < 0.0)
    {
        return QPointF(drawn.left(), std::clamp(onStored.y(), drawn.top() + rad, drawn.bottom() - rad));
    }

    return QPointF(drawn.right(), std::clamp(onStored.y(), drawn.top() + rad, drawn.bottom() - rad));
}

QPointF SMEdgeItem::anchorFrame(uint32_t stateId) const
{
    // An unresolved box gives a null origin, which leaves the anchor where it was persisted:
    // there is no box to measure it against, so there is nothing for it to follow either.
    return stateBoxRect(stateId).topLeft();
}

void SMEdgeItem::setAnchorPoint(bool begin, const QPointF& point)
{
    if (mHasAnchors == false)
    {
        // First manual endpoint move: seed BOTH anchors from the drawn path, so the endpoint
        // that was not touched keeps the place it was drawn at instead of falling back to the
        // center-facing default.
        mAnchorBegin = mBegin;
        mAnchorEnd   = mEnd;
        mHasAnchors  = true;
    }

    if (begin)
    {
        mAnchorBegin = point;
    }
    else
    {
        mAnchorEnd = point;
    }

    // Re-measure both against their boxes: the seeding above may have written the other one too.
    mAnchorBeginRel  = mAnchorBegin - anchorFrame(mSourceId);
    mAnchorEndRel    = mAnchorEnd   - anchorFrame(mSelfLoop ? mSourceId : mTargetId);
    mAnchorsMeasured = true;
}

double SMEdgeItem::stateRadius(uint32_t stateId, const QRectF& rect) const
{
    SMScene* canvas = getCanvas();
    if ((canvas == nullptr) || (stateId == 0))
    {
        return NESMDesign::StateCornerRadius;
    }

    SMStateItem* item = canvas->stateItem(stateId);
    if (item != nullptr)
    {
        return item->boxCornerRadius();
    }

    const SMStateEntry* state = canvas->getModel().getData().findStateById(stateId);
    const bool marker = (state != nullptr) && (state->getKind() != SMStateEntry::eStateKind::Normal);
    return (marker ? std::min(rect.width(), rect.height()) / 2.0 : NESMDesign::StateCornerRadius);
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
    if ((transition == nullptr) || (transition->hasTarget() == false))
    {
        mValid = false;
        return;
    }

    const SMStateEntry* owner = data.findTransitionOwner(getElementId());
    mSourceId    = (owner != nullptr ? owner->getId() : 0);
    // A transition out of the Start pseudo-state fires automatically: it never carries a stimulus,
    // so no `<stimulus>` placeholder is shown for it.
    mSourceIsStart = (owner != nullptr) && (owner->getKind() == SMStateEntry::eStateKind::Start);
    mIsInitial     = transition->isInitial();
    mTargetId    = transition->getToId();
    const SMStateEntry* target = data.findStateById(mTargetId);
    mTargetName  = (target != nullptr ? target->getName() : QString());
    mSelfLoop    = (mTargetId != 0) && (mTargetId == mSourceId);

    // Show the guard next to the stimulus (`stimulus[summary]`); the full guard is the tooltip.
    // No guard -> the stimulus alone (no empty brackets). Summary only, never rotated.
    // The label carries the guard's severity color + glyph when the guard is not ok.
    const QString summary = SMGuardRender::guardText(data, getElementId(), transition->getGuard()).simplified();
    mGuardSeverity = -1;
    QString guardIssue;
    const QList<SMGuardValidation::Finding> findings = SMGuardValidation::validateTransition(data, getElementId());
    SMGuardValidation::eSeverity worst = SMGuardValidation::eSeverity::Info;
    for (const SMGuardValidation::Finding& finding : findings)
    {
        if (static_cast<int>(finding.severity) > static_cast<int>(worst))
        {
            worst = finding.severity;
        }
    }

    if ((findings.isEmpty() == false) && (worst != SMGuardValidation::eSeverity::Info))
    {
        mGuardSeverity = static_cast<int>((worst == SMGuardValidation::eSeverity::Error)
                                          ? NEGuardStyle::eSeverity::Err
                                          : NEGuardStyle::eSeverity::Warn);
        // The canvas says WHAT is wrong, not merely that something is: the same sentence the
        // Conditions status line shows, so the two surfaces never seem to disagree.
        for (const SMGuardValidation::Finding& finding : findings)
        {
            if (finding.severity == worst)
            {
                guardIssue = finding.message;
                break;
            }
        }
    }

    // Only a trigger reads as a method signature (`walk(count)`); an event and a timer are named
    // bare and say WHAT they are through a mark drawn in front of the text -- a lightning bolt or a
    // clock -- rather than through empty brackets or a generated `on_event_` prefix (issue #543).
    // The guard clause is kept separate so paintLabels can tint stimulus and condition distinctly.
    const QString signature = SMOperationSummary::stimulusSignature(data, *transition);
    mStimulusText  = SMKindGlyph::prefix(SMKindGlyph::stimulusGlyph(*transition)) + signature;
    mStimulusGlyph = SMKindGlyph::stimulusGlyph(*transition);
    if (summary.isEmpty())
    {
        mGuardText.clear();
        setToolTip(QString());
    }
    else
    {
        constexpr int MAX_SUMMARY = SMGuardRender::ChipEdge;
        // One glyph per severity: an error and a warning must not look alike on the canvas while
        // the status line calls them by different names. The glyph is the grayscale/color-blind
        // channel -- the severity color alone would carry the whole distinction.
        QString glyph;
        if (mGuardSeverity == static_cast<int>(NEGuardStyle::eSeverity::Err))
        {
            glyph = QStringLiteral("(x) ");
        }
        else if (mGuardSeverity >= 0)
        {
            glyph = QStringLiteral("(!) ");
        }

        // The shared one-line renderer: full text, else a structural summary, else a cut taken at a
        // token boundary. It lives in SMGuardRender because the internal-transition picker, the
        // state-box rows and the transition list must speak the same shorthand as this label; only
        // the budget differs. The tooltip always carries the whole text.
        const QString shortSummary = SMGuardRender::chipText(data, getElementId(), transition->getGuard(), MAX_SUMMARY);
        mGuardText = QChar('[') + glyph + shortSummary + QChar(']');
        // A tooltip cannot carry a drawn mark, so it spells the kind out instead.
        const QString kindWord = SMKindGlyph::word(mStimulusGlyph);
        QString tip = (kindWord.isEmpty() ? QString() : (kindWord + QChar(' ')))
                    + signature + QChar('[') + summary + QChar(']');
        if (guardIssue.isEmpty() == false)
        {
            tip += QChar('\n')
                 + translate((mGuardSeverity == static_cast<int>(NEGuardStyle::eSeverity::Err))
                             ? "err: %1" : "warn: %1").arg(guardIssue);
        }

        setToolTip(tip);
    }

    // An action/event whose arguments are not fully mapped warns on the canvas, so the developer
    // sees which transitions a method edit broke without opening each Properties panel; the glyph
    // clears the instant every argument is mapped.
    mActionSeverity = -1;
    DocIssue::eSeverity opSeverity = DocIssue::eSeverity::Info;
    if (SMOperationValidation::worstForTransition(data, getElementId(), opSeverity))
    {
        mActionSeverity = static_cast<int>((opSeverity == DocIssue::eSeverity::Error)
                                           ? NEGuardStyle::eSeverity::Err
                                           : NEGuardStyle::eSeverity::Warn);
    }

    // The transition's operations are summarized below the line on ONE line -- action, event, and
    // timer(s) in order, joined with a thin separator. As many as fit the width budget are shown;
    // the rest collapse into a trailing `(+N)` rather than spilling onto new lines (issue #532).
    const SMOperationList& ops = transition->getOperations();
    mActionText.clear();
    if (ops.getCount() > 0)
    {
        constexpr int MAX_ACTION = 44;
        const QString SEP = QStringLiteral(" | ");
        QString line;
        int shown = 0;
        for (int i = 0; i < ops.getCount(); ++i)
        {
            const QString token = SMOperationSummary::text(data, *ops.at(i)).simplified();
            const int sepLen = line.isEmpty() ? 0 : SEP.length();
            // Always show at least the first op; stop once another would overflow the budget.
            if ((shown > 0) && ((line.length() + sepLen + token.length()) > MAX_ACTION))
            {
                break;
            }

            line += (line.isEmpty() ? token : (SEP + token));
            ++shown;
        }

        const int hidden = ops.getCount() - shown;
        if (hidden > 0)
        {
            line += QStringLiteral(" (+%1)").arg(hidden);
        }
        else if (line.length() > MAX_ACTION)
        {
            line = line.left(MAX_ACTION - 3) + QStringLiteral("...");
        }

        mActionText = line;

        // A leading `(!)` marks an incomplete mapping; the tint is applied in paintLabels.
        if (mActionSeverity >= 0)
        {
            mActionText = QStringLiteral("(!) ") + mActionText;
        }
    }

    mHasNote     = (data.getLayout().findNoteByOwner(getElementId()) != nullptr);

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

    // The persisted first/last points are the user-placed border anchors; the drawn begin/end are
    // these projected onto the live border. They are re-measured against their boxes on the next
    // rebuild (mAnchorsMeasured), from where on they follow those boxes rather than the stored
    // scene coordinates -- otherwise moving a box far enough would let the projection pick the
    // opposite side and the transition would jump around the box (issue #550 bug 3).
    mHasAnchors      = (edge != nullptr) && (edge->points.size() >= 2);
    mAnchorsMeasured = false;
    mHasOrigins      = false;   // the waypoints just read belong to the boxes as they stand now
    if (mHasAnchors)
    {
        mAnchorBegin = edge->points.first();
        mAnchorEnd   = edge->points.last();
    }

    // A removed/merged waypoint (or a re-read that shrank the list) invalidates the active point.
    if (mSelectedPoint >= mWaypoints.size())
    {
        mSelectedPoint = -1;
    }

    mValid = (mSourceId != 0);
    prepareGeometryChange();
    rebuildPath();
    update();
}

void SMEdgeItem::refreshAnchors(bool fromModel /*= false*/)
{
    if (mValid)
    {
        if (fromModel)
        {
            // The document moved the box, so its stored anchors belong to where it stands now.
            // One undo step may carry a box and its transitions as separate children, which reach
            // the canvas one after the other; re-measuring on each of them leaves the last -- and
            // therefore consistent -- pair as the one that counts.
            mAnchorsMeasured = false;
        }

        prepareGeometryChange();
        rebuildPath(fromModel == false);
        update();
    }
}

void SMEdgeItem::rebuildPath(bool followBoxes /*= false*/)
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

    // The anchors live in their boxes, not in the scene: measure them once against the stored box
    // (the collapsed body must not move them), then re-derive them from wherever the box stands
    // now. Moving a box therefore only changes the length and the angle of the line.
    const QPointF srcFrame = anchorFrame(mSourceId);
    const QPointF tgtFrame = anchorFrame(mSelfLoop ? mSourceId : mTargetId);
    if (mHasAnchors)
    {
        if (mAnchorsMeasured == false)
        {
            // Measure the point the DOCUMENT holds against the box as it now stands. The member
            // anchors are not used here: one undo step reaches the canvas as several changes, and
            // in between them an anchor may already have been re-derived from a box that has not
            // caught up yet -- measuring that would freeze the mismatch instead of ending it.
            const SMScene* owner = getCanvas();
            const SMLayoutEdge* stored = (owner != nullptr)
                    ? owner->getModel().getData().getLayout().findEdge(getElementId()) : nullptr;
            if ((stored != nullptr) && (stored->points.size() >= 2))
            {
                mAnchorBegin = stored->points.first();
                mAnchorEnd   = stored->points.last();
            }

            mAnchorBeginRel  = mAnchorBegin - srcFrame;
            mAnchorEndRel    = mAnchorEnd   - tgtFrame;
            mAnchorsMeasured = true;
        }
        else
        {
            mAnchorBegin = srcFrame + mAnchorBeginRel;
            mAnchorEnd   = tgtFrame + mAnchorEndRel;
        }
    }

    // Both boxes travelled by the same step: the whole transition was carried along by a group
    // move, so its interior waypoints and its label travel with it and the drawn line keeps its
    // shape. One box moving alone leaves them where they are and reshapes the line instead.
    if (followBoxes && mHasOrigins)
    {
        const QPointF delta = srcFrame - mSrcOrigin;
        const QPointF drift = delta - (tgtFrame - mTgtOrigin);
        const bool    same  = (std::abs(drift.x()) < 1e-6) && (std::abs(drift.y()) < 1e-6);
        const bool    moved = (std::abs(delta.x()) > 1e-6) || (std::abs(delta.y()) > 1e-6);
        if (same && moved)
        {
            for (QPointF& point : mWaypoints)
            {
                point += delta;
            }

            if (mHasLabel)
            {
                mLabelPos += delta;
            }

            mSrcOrigin = srcFrame;
            mTgtOrigin = tgtFrame;
        }

        // The two boxes disagree: keep the origins until the second one catches up. A group move
        // reaches the items one by one, so the steps only match after the last of them arrived.
    }
    else
    {
        mSrcOrigin  = srcFrame;
        mTgtOrigin  = tgtFrame;
        mHasOrigins = true;
    }

    // A self-loop with no stored waypoints gets a default loop above the box. An ARC self-loop gets
    // none: its two border anchors and the bulge already describe the whole loop, and a seeded
    // waypoint here would be written straight back into the layout as if the user had placed it.
    if (mSelfLoop && mWaypoints.isEmpty() && (mShape != SMLayoutEdge::eShape::Arc))
    {
        const double off = NESMDesign::EdgeSelfLoopStandoff;
        mWaypoints.append(QPointF(src.center().x() - NESMDesign::EdgeSelfLoopHalfSpan, src.top() - off));
        mWaypoints.append(QPointF(src.center().x() + NESMDesign::EdgeSelfLoopHalfSpan, src.top() - off));
    }

    const double srcRad = stateRadius(mSourceId, src);
    const double tgtRad = (mSelfLoop ? srcRad : stateRadius(mTargetId, tgt));

    // A default (never-dragged) endpoint sticks to the border facing the other box; with snap-to-grid
    // on it also lands on a grid-aligned border position (crossing or midpoint), so it stops jittering
    // as either box moves (issue #532).
    SMScene* canvas = getCanvas();
    const bool snap = (canvas != nullptr) && canvas->isSnapToGrid();
    const int  grid = (canvas != nullptr) ? canvas->getGridSize() : NESMDesign::GridSizeDefault;
    const auto defaultBorder = [&](const QRectF& rect, double rad, const QPointF& towards) -> QPointF
    {
        const QPointF bp = NESMDesign::borderPoint(rect, rad, towards);
        return snap ? NESMDesign::gridAlignedBorderPoint(rect, rad, bp, grid) : bp;
    };

    if (mShape == SMLayoutEdge::eShape::Arc)
    {
        // A self-loop faces no other box, so center-to-center gives no direction at all: without
        // anchors of its own it falls back to the symmetric default pair on its top border.
        QPointF loopBegin;
        QPointF loopEnd;
        if (mSelfLoop && (mHasAnchors == false))
        {
            selfLoopEnds(src, loopBegin, loopEnd);
        }

        mBegin = (mDrag == eDrag::Begin) ? mDragPoint
               : mHasAnchors ? borderAnchorPoint(mSourceId, src, mAnchorBegin)
               : mSelfLoop   ? loopBegin
                             : defaultBorder(src, srcRad, tc);
        mEnd   = (mDrag == eDrag::End)   ? mDragPoint
               : mHasAnchors ? borderAnchorPoint((mSelfLoop ? mSourceId : mTargetId), tgt, mAnchorEnd)
               : mSelfLoop   ? loopEnd
                             : defaultBorder(tgt, tgtRad, sc);
        mPath  = NESMDesign::arcPolyline(mBegin, mEnd, mBulge, NESMDesign::EdgeArcSamples);
    }
    else
    {
        // Without waypoints the endpoint faces the other box; with them it lines up with the
        // adjacent corner, so the first and last legs leave the border square instead of fanning
        // back toward the box center.
        const bool    poly     = (mWaypoints.isEmpty() == false);
        const QPointF beginRef = poly ? mWaypoints.first() : tc;
        const QPointF endRef   = poly ? mWaypoints.last()  : sc;
        mBegin = (mDrag == eDrag::Begin) ? mDragPoint
               : mHasAnchors ? borderAnchorPoint(mSourceId, src, mAnchorBegin)
               : poly        ? NESMDesign::polylineBorderPoint(src, srcRad, beginRef)
                             : defaultBorder(src, srcRad, beginRef);
        mEnd   = (mDrag == eDrag::End)   ? mDragPoint
               : mHasAnchors ? borderAnchorPoint((mSelfLoop ? mSourceId : mTargetId), tgt, mAnchorEnd)
               : poly        ? NESMDesign::polylineBorderPoint(tgt, tgtRad, endRef)
                             : defaultBorder(tgt, tgtRad, endRef);

        mPath.append(mBegin);
        mPath.append(mWaypoints);
        mPath.append(mEnd);
    }
}

QPointF SMEdgeItem::labelAnchor() const
{
    if (mHasLabel)
    {
        return mLabelPos;
    }

    // Halfway along the drawn polyline.
    double total = 0.0;
    for (int i = 1; i < mPath.size(); ++i)
    {
        total += distance(mPath.at(i - 1), mPath.at(i));
    }

    double half = total / 2.0;
    QPointF anchor = mPath.first();
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

    return anchor;
}

bool SMEdgeItem::labelOnVerticalRun() const
{
    if (mPath.size() < 2)
    {
        return false;
    }

    // The segment the midpoint falls on, found the way labelAnchor() finds the point itself.
    double total = 0.0;
    for (int i = 1; i < mPath.size(); ++i)
    {
        total += distance(mPath.at(i - 1), mPath.at(i));
    }

    double half = total / 2.0;
    int index = 1;
    for (int i = 1; i < mPath.size(); ++i)
    {
        index = i;
        const double seg = distance(mPath.at(i - 1), mPath.at(i));
        if (seg >= half)
        {
            break;
        }

        half -= seg;
    }

    const QPointF delta = mPath.at(index) - mPath.at(index - 1);
    return (qAbs(delta.y()) > qAbs(delta.x()));
}

QString SMEdgeItem::labelText() const
{
    const QString text = mStimulusText + mGuardText;
    if (text.isEmpty() == false)
    {
        return text;
    }

    // No stimulus and no guard: a Start-state transition shows nothing (it fires automatically);
    // any other transition shows a subtle hint that a stimulus can be set.
    return (mIsInitial || mSourceIsStart) ? QString() : translate("<stimulus>");
}

QRectF SMEdgeItem::labelRect() const
{
    const QString text = labelText();
    if ((mValid == false) || mPath.isEmpty() || text.isEmpty())
    {
        return QRectF();
    }

    const QPointF anchor = labelAnchor();
    const QFontMetricsF metrics{ labelFont() };
    const QSizeF size = metrics.size(0, text) + QSizeF(4.0 + markWidth(mStimulusGlyph), 1.0);

    // Default edges lift the stimulus above the line so a horizontal edge does not strike
    // through it; a user-dragged label centers on its point (the user placed it deliberately).
    constexpr double GAP = 3.0;
    double top = anchor.y() - size.height() - GAP;
    if (mHasLabel)
    {
        top = anchor.y() - size.height() / 2.0;
    }
    else if (labelOnVerticalRun())
    {
        // The line runs alongside the text instead of between the two rows, so nothing has to be
        // kept clear: stimulus and action read as one block, centred on the anchor.
        const double action = mActionText.isEmpty() ? 0.0 : (metrics.size(0, mActionText).height() + 1.0);
        top = anchor.y() - (size.height() + action) / 2.0;
    }

    return QRectF(anchor.x() - size.width() / 2.0, top, size.width(), size.height());
}

QRectF SMEdgeItem::actionRect() const
{
    if ((mValid == false) || mPath.isEmpty() || mActionText.isEmpty())
    {
        return QRectF();
    }

    const QPointF anchor = labelAnchor();
    const QFontMetricsF metrics{ labelFont() };
    const QSizeF size = metrics.size(0, mActionText) + QSizeF(4.0, 1.0);

    // The action reads below the line; when the user dragged the stimulus label, or when the line
    // runs vertically past both rows, it tucks directly beneath the stimulus instead.
    constexpr double GAP = 3.0;
    double top = anchor.y() + GAP;
    if (mHasLabel)
    {
        top = labelRect().bottom() + 1.0;
    }
    else if (labelOnVerticalRun())
    {
        const QRectF label = labelRect();
        top = label.isNull() ? (anchor.y() - size.height() / 2.0) : label.bottom();
    }

    return QRectF(anchor.x() - size.width() / 2.0, top, size.width(), size.height());
}

QRectF SMEdgeItem::noteBadgeRect() const
{
    if ((mValid == false) || mPath.isEmpty())
    {
        return QRectF();
    }

    // Sit just right of the label; with no label (a Start transition) hang off the midpoint.
    const QRectF label = labelRect();
    if (label.isNull() == false)
    {
        return QRectF(label.right() + 3.0, label.center().y() - 7.0, 13.0, 14.0);
    }

    const QPointF anchor = labelAnchor();
    return QRectF(anchor.x() + 4.0, anchor.y() - 7.0, 13.0, 14.0);
}

QRectF SMEdgeItem::stimulusLinkRect() const
{
    if (mStimulusText.isEmpty())
    {
        return QRectF();
    }

    const QRectF label = labelRect();
    if (label.isNull())
    {
        return QRectF();
    }

    // Mirror paintLabels: the stimulus is drawn at label.left() + 2, its mark and text wide. The
    // mark is part of the link -- it names the same declaration the text does.
    const QFontMetricsF metrics{ labelFont() };
    const double advance = markWidth(mStimulusGlyph) + metrics.horizontalAdvance(mStimulusText);
    return QRectF(label.left() + 2.0, label.top(), advance, label.height());
}

QRectF SMEdgeItem::guardLinkRect() const
{
    if (mGuardText.isEmpty())
    {
        return QRectF();
    }

    const QRectF label = labelRect();
    if (label.isNull())
    {
        return QRectF();
    }

    // The guard clause is drawn just right of the stimulus (or at the label start when there is none).
    const QFontMetricsF metrics{ labelFont() };
    double x = label.left() + 2.0;
    if (mStimulusText.isEmpty() == false)
    {
        x += markWidth(mStimulusGlyph) + metrics.horizontalAdvance(mStimulusText);
    }

    const double advance = metrics.horizontalAdvance(mGuardText);
    return QRectF(x, label.top(), advance, label.height());
}

SMEdgeItem::eLink SMEdgeItem::linkRegionAt(const QPointF& pos) const
{
    const QRectF stimulus = stimulusLinkRect();
    if ((stimulus.isNull() == false) && stimulus.contains(pos))
    {
        return LinkStimulus;
    }

    const QRectF guard = guardLinkRect();
    if ((guard.isNull() == false) && guard.contains(pos))
    {
        return LinkGuard;
    }

    const QRectF action = actionRect();
    if ((action.isNull() == false) && action.contains(pos))
    {
        return LinkAction;
    }

    return LinkNone;
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
    if (actionRect().isNull() == false)
    {
        rect = rect.united(actionRect());
    }
    if (mHasNote)
    {
        rect = rect.united(noteBadgeRect());
    }

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
    if (mHasNote)
    {
        result.addRect(noteBadgeRect());
    }
    result.addEllipse(mBegin, NESMDesign::EndpointPickRadius, NESMDesign::EndpointPickRadius);
    result.addEllipse(mEnd, NESMDesign::EndpointPickRadius, NESMDesign::EndpointPickRadius);
    for (const QPointF& wp : mWaypoints)
    {
        result.addEllipse(wp, NESMDesign::EndpointPickRadius, NESMDesign::EndpointPickRadius);
    }

    if (mShape == SMLayoutEdge::eShape::Arc)
    {
        result.addEllipse(arcApex(), NESMDesign::EndpointPickRadius, NESMDesign::EndpointPickRadius);
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
        // Keep the selected/highlighted line clearly thin -- selection is already signalled by hue,
        // waypoint handles and endpoint rings, so only a slight width bump is needed for emphasis.
        pen.setWidthF(NESMDesign::EdgeLineWidth + 0.2);
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(mPath.constData(), mPath.size());

    // Begin dot on the source border -- doubled into the filled entry ball for an INITIAL
    // transition. That is the one drawing difference between the two kinds that carry an edge, and
    // it is the notation an author already reads as "this is where the level begins"; the absent
    // stimulus label is the other half of it.
    const double beginDot = mIsInitial
                            ? (NESMDesign::EdgeBeginDotRadius * 2.0)
                            : NESMDesign::EdgeBeginDotRadius;
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawEllipse(mBegin, beginDot, beginDot);

    paintArrowHead(painter, mPath.at(mPath.size() - 2), mEnd, color);

    // The labels (stimulus, guard, action summary) and the note badge are painted by the scene's
    // foreground pass (paintLabels), so they stay above the state boxes and readable.

    if (isSelected())
    {
        const double h = NESMDesign::WaypointHandleSize;
        for (int i = 0; i < mWaypoints.size(); ++i)
        {
            const QPointF& wp = mWaypoints.at(i);
            if (i == mSelectedPoint)
            {
                // The active (keyboard-movable) waypoint: a larger filled marker so the user
                // sees which point the arrow keys move.
                painter->setPen(QPen(NESMDesign::selectionColor(palette), 1.4));
                painter->setBrush(palette.color(QPalette::Base));
                painter->drawRect(QRectF(wp.x() - h / 2.0 - 1.5, wp.y() - h / 2.0 - 1.5, h + 3.0, h + 3.0));
            }
            else
            {
                painter->setPen(QPen(palette.color(QPalette::Base), 1.0));
                painter->setBrush(NESMDesign::selectionColor(palette));
                painter->drawRect(QRectF(wp.x() - h / 2.0, wp.y() - h / 2.0, h, h));
            }
        }

        if (mShape == SMLayoutEdge::eShape::Arc)
        {
            // The curvature handle: a diamond at the apex, so it never reads as one more waypoint
            // (squares) or an endpoint (circles) -- dragging it bends the arc, it does not move a point.
            const QPointF apex = arcApex();
            const double  d    = h / 2.0 + 1.0;
            const QPointF diamond[4] = { QPointF(apex.x(), apex.y() - d), QPointF(apex.x() + d, apex.y())
                                       , QPointF(apex.x(), apex.y() + d), QPointF(apex.x() - d, apex.y()) };
            painter->setPen(QPen(NESMDesign::selectionColor(palette), 1.4));
            painter->setBrush(palette.color(QPalette::Base));
            painter->drawPolygon(diamond, 4);
        }

        painter->setBrush(palette.color(QPalette::Base));
        painter->setPen(QPen(NESMDesign::selectionColor(palette), 1.4));
        // The active (keyboard-movable) endpoint gets a slightly larger ring so the user sees which
        // one the arrow keys move along the border.
        const double br = (mActiveEnd == 1) ? (h / 2.0 + 1.5) : (h / 2.0);
        const double er = (mActiveEnd == 2) ? (h / 2.0 + 1.5) : (h / 2.0);
        painter->drawEllipse(mBegin, br, br);
        painter->drawEllipse(mEnd, er, er);
    }
}

QRectF SMEdgeItem::labelBounds() const
{
    QRectF rect = labelRect();
    if (actionRect().isNull() == false)
    {
        rect = rect.united(actionRect());
    }

    if (mHasNote)
    {
        rect = rect.united(noteBadgeRect());
    }

    // In reposition mode the tether runs from the label down to the line; include its target so the
    // scene's foreground pass repaints the whole tether, not just the framed block.
    if (mLabelActive && (rect.isNull() == false))
    {
        const QPointF to = nearestPathPoint(labelAnchor());
        rect = rect.united(QRectF(to.x() - 2.0, to.y() - 2.0, 4.0, 4.0));
    }

    return rect;
}

void SMEdgeItem::paintLabels(QPainter* painter, const QPalette& palette)
{
    if ((mValid == false) || mPath.isEmpty())
    {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(Qt::NoBrush);
    painter->setFont(labelFont());

    // Reposition mode: a 1px tether from the label to the nearest point of its transition line so
    // the reader sees which edge the text belongs to, and a dashed frame around the movable block.
    if (mLabelActive)
    {
        const QColor  accent = NESMDesign::selectionColor(palette);
        const QPointF from   = labelAnchor();
        const QPointF to     = nearestPathPoint(from);
        painter->setPen(QPen(accent, 1.0));
        painter->drawLine(from, to);
        painter->drawEllipse(to, 1.6, 1.6);

        QRectF frame = labelRect();
        if (actionRect().isNull() == false)
        {
            frame = frame.united(actionRect());
        }

        painter->setPen(QPen(accent, 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        // A tight 1px margin so the frame hugs the text without covering it.
        painter->drawRect(frame.adjusted(-1.0, -1.0, 1.0, 1.0));
    }

    // Stimulus (its own hue) then the guard clause (a distinct condition hue / severity tint),
    // so the two read apart at a glance (issue: differentiate stimulus vs condition).
    const QRectF label = labelRect();
    if (label.isNull() == false)
    {
        const QFontMetricsF metrics{ labelFont() };
        if (mStimulusText.isEmpty() && mGuardText.isEmpty())
        {
            painter->setPen(palette.color(QPalette::Disabled, QPalette::Text));
            painter->drawText(label, Qt::AlignCenter, labelText());
        }
        else
        {
            double x = label.left() + 2.0;
            if (mStimulusText.isEmpty() == false)
            {
                const QColor stimColor = NEGuardStyle::ownerColor(NEGuardStyle::eOwner::Stimulus);
                const double mark = markWidth(mStimulusGlyph);
                if (mark > 0.0)
                {
                    // The mark takes the stimulus hue too, so mark and name read as one token.
                    SMKindGlyph::paint(*painter, QRectF(x, label.top() + 1.0
                                                      , SMKindGlyph::GlyphSize, label.height() - 2.0)
                                      , mStimulusGlyph, stimColor);
                    x += mark;
                }

                painter->setPen(stimColor);
                const double advance = metrics.horizontalAdvance(mStimulusText);
                painter->drawText(QRectF(x, label.top(), advance, label.height()), Qt::AlignVCenter | Qt::AlignLeft, mStimulusText);
                x += advance;
            }

            if (mGuardText.isEmpty() == false)
            {
                painter->setPen((mGuardSeverity >= 0)
                                ? NEGuardStyle::severityColor(static_cast<NEGuardStyle::eSeverity>(mGuardSeverity))
                                : NEGuardStyle::ownerColor(NEGuardStyle::eOwner::Handler));
                const double advance = metrics.horizontalAdvance(mGuardText);
                painter->drawText(QRectF(x, label.top(), advance, label.height()), Qt::AlignVCenter | Qt::AlignLeft, mGuardText);
            }
        }
    }

    // Operation summary below the line, in a third hue distinct from stimulus and guard.
    const QRectF action = actionRect();
    if (action.isNull() == false)
    {
        painter->setPen((mActionSeverity >= 0)
                        ? NEGuardStyle::severityColor(static_cast<NEGuardStyle::eSeverity>(mActionSeverity))
                        : NEGuardStyle::ownerColor(NEGuardStyle::eOwner::Fsm));
        painter->drawText(action, Qt::AlignCenter, mActionText);
    }

    // Note badge just right of the label: a small folded-page glyph signalling a bound note.
    if (mHasNote)
    {
        const QColor color = strokeColor(palette);
        const QRectF badge = noteBadgeRect();
        const double fold = 4.0;
        QPainterPath page;
        page.moveTo(badge.left(), badge.top());
        page.lineTo(badge.right() - fold, badge.top());
        page.lineTo(badge.right(), badge.top() + fold);
        page.lineTo(badge.right(), badge.bottom());
        page.lineTo(badge.left(), badge.bottom());
        page.closeSubpath();

        QColor fill{ color };
        fill.setAlphaF(0.16);
        painter->setBrush(fill);
        painter->setPen(QPen(color, 1.0));
        painter->drawPath(page);
        painter->drawLine(QPointF(badge.right() - fold, badge.top()), QPointF(badge.right() - fold, badge.top() + fold));
        painter->drawLine(QPointF(badge.right() - fold, badge.top() + fold), QPointF(badge.right(), badge.top() + fold));
        painter->drawLine(QPointF(badge.left() + 2.5, badge.top() + 6.0), QPointF(badge.right() - 2.5, badge.top() + 6.0));
        painter->drawLine(QPointF(badge.left() + 2.5, badge.top() + 9.5), QPointF(badge.right() - 4.5, badge.top() + 9.5));
    }

    // Ctrl+Shift link feedback: underline the label part under the pointer so the user sees exactly
    // what a click opens (the stimulus, the guard editor, or the operations), matching the editor's
    // go-to-definition affordance. Drawn last so the rule sits above the text.
    if (mHoverLink != LinkNone)
    {
        QRectF region;
        switch (mHoverLink)
        {
        case LinkStimulus:  region = stimulusLinkRect(); break;
        case LinkGuard:     region = guardLinkRect();    break;
        case LinkAction:    region = actionRect();       break;
        default:            break;
        }

        if (region.isNull() == false)
        {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(NESMDesign::selectionColor(palette), 1.0));
            const double y = region.bottom() - 1.0;
            painter->drawLine(QPointF(region.left(), y), QPointF(region.right(), y));
        }
    }
}

void SMEdgeItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    // Ctrl+Shift held turns the referenced label parts into links: underline the part under the
    // pointer and switch to a link cursor. Without both modifiers the label behaves normally.
    const Qt::KeyboardModifiers mods = event->modifiers();
    const bool linkMode = mods.testFlag(Qt::ControlModifier) && mods.testFlag(Qt::ShiftModifier);
    const eLink region = linkMode ? linkRegionAt(event->pos()) : LinkNone;
    if (region != mHoverLink)
    {
        mHoverLink = region;
        if (region == LinkNone)
        {
            unsetCursor();
        }
        else
        {
            setCursor(Qt::PointingHandCursor);
        }

        update();
    }

    SMCanvasItem::hoverMoveEvent(event);
}

void SMEdgeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    if (mHoverLink != LinkNone)
    {
        mHoverLink = LinkNone;
        unsetCursor();
        update();
    }

    SMCanvasItem::hoverLeaveEvent(event);
}

void SMEdgeItem::setSelectedPoint(int index)
{
    if (index >= mWaypoints.size())
    {
        index = -1;
    }

    if (index != mSelectedPoint)
    {
        mSelectedPoint = index;
        update();
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

QPointF SMEdgeItem::arcApex() const
{
    // Must agree with NESMDesign::arcPolyline, which places the apex the same way.
    const QPointF chord = mEnd - mBegin;
    const double  c     = std::hypot(chord.x(), chord.y());
    if (c < 1e-6)
    {
        return mBegin;
    }

    const QPointF normal{ -chord.y() / c, chord.x() / c };
    return ((mBegin + mEnd) / 2.0) + normal * (mBulge * c / 2.0);
}

double SMEdgeItem::bulgeFor(const QPointF& point) const
{
    const QPointF chord = mEnd - mBegin;
    const double  c     = std::hypot(chord.x(), chord.y());
    if (c < 1e-6)
    {
        return 0.0;
    }

    // The bulge is the signed sagitta as a fraction of half the chord, so the apex tracks the
    // pointer's distance from the chord and its side decides which way the arc bends.
    const QPointF normal{ -chord.y() / c, chord.x() / c };
    const QPointF fromMid = point - ((mBegin + mEnd) / 2.0);
    const double  sagitta = (fromMid.x() * normal.x()) + (fromMid.y() * normal.y());
    const double  limit   = (mSelfLoop ? NESMDesign::EdgeArcSelfBulgeMax : NESMDesign::EdgeArcBulgeMax);
    return std::clamp(2.0 * sagitta / c, -limit, limit);
}

void SMEdgeItem::selfLoopEnds(const QRectF& box, QPointF& begin, QPointF& end) const
{
    begin = QPointF(box.center().x() - NESMDesign::EdgeSelfLoopHalfSpan, box.top());
    end   = QPointF(box.center().x() + NESMDesign::EdgeSelfLoopHalfSpan, box.top());
}

void SMEdgeItem::adoptSelfLoopEnds()
{
    const QRectF box = stateRect(mSourceId);
    if ((box.width() <= 0.0) || (box.height() <= 0.0))
    {
        return;
    }

    if (distance(mBegin, mEnd) < 1e-3)
    {
        selfLoopEnds(box, mBegin, mEnd);    // a chord of zero length is not a curve
    }

    // Pin them: without anchors the arc branch would re-derive the endpoints and lose the side of
    // the box the user had the loop leaving from.
    setAnchorPoint(true , mBegin);
    setAnchorPoint(false, mEnd);
}

double SMEdgeItem::selfLoopBulge() const
{
    const QRectF  box   = stateRect(mSourceId);
    const QPointF chord = mEnd - mBegin;
    const double  c     = std::hypot(chord.x(), chord.y());
    if ((c < 1e-6) || (box.width() <= 0.0) || (box.height() <= 0.0))
    {
        return NESMDesign::EdgeArcBulgeDefault;
    }

    // Stand the apex the same distance off the border as the polyline loop's corners, so switching
    // a loop between the two shapes keeps it the same size, and bow it AWAY from the box: a loop
    // curving back through its own state reads as a line crossing it, not as a transition.
    const QPointF normal { -chord.y() / c, chord.x() / c };
    const QPointF outward = ((mBegin + mEnd) / 2.0) - box.center();
    const double  side    = (((normal.x() * outward.x()) + (normal.y() * outward.y())) < 0.0) ? -1.0 : 1.0;
    return side * std::min(2.0 * NESMDesign::EdgeSelfLoopStandoff / c, NESMDesign::EdgeArcSelfBulgeMax);
}

QList<QPointF> SMEdgeItem::selfLoopCorners() const
{
    QList<QPointF> corners;
    const QRectF box = stateRect(mSourceId);
    if ((box.width() <= 0.0) || (box.height() <= 0.0))
    {
        return corners;
    }

    corners.append(mBegin + (NESMDesign::borderOutwardNormal(box, mBegin) * NESMDesign::EdgeSelfLoopStandoff));
    corners.append(mEnd   + (NESMDesign::borderOutwardNormal(box, mEnd)   * NESMDesign::EdgeSelfLoopStandoff));
    return corners;
}

void SMEdgeItem::setShape(SMLayoutEdge::eShape shape)
{
    const bool arc = (shape == SMLayoutEdge::eShape::Arc);
    if (mShape == shape)
    {
        return;
    }

    prepareGeometryChange();
    mShape = shape;
    if (arc)
    {
        // An arc is its two endpoints plus a bulge; the corners of a polyline describe nothing
        // on a curve. This mirrors the Arc -> Line downgrade that adding a waypoint performs.
        mWaypoints.clear();
        setSelectedPoint(-1);
        if (mSelfLoop)
        {
            adoptSelfLoopEnds();
        }

        if (std::abs(mBulge) < 1e-6)
        {
            // A zero bulge would still draw straight.
            mBulge = (mSelfLoop ? selfLoopBulge() : NESMDesign::EdgeArcBulgeDefault);
        }
    }
    else
    {
        mBulge = 0.0;
        if (mSelfLoop && mWaypoints.isEmpty())
        {
            // Both anchors of a self-loop sit on the same box, so a two-point run between them has
            // nothing to draw. Straightening gives it the rectangle the arc stood in: one corner
            // off each anchor, at the standoff the default loop uses.
            mWaypoints = selfLoopCorners();
        }
    }

    rebuildPath();
    mGesture = SMMoveNodeCommand::takeNextGesture();
    commitGeometry(arc ? translate("Curve transition") : translate("Straighten transition"));
    update();
}

SMLayoutEdge SMEdgeItem::buildGeometry() const
{
    SMLayoutEdge edge;
    edge.owner = getElementId();
    edge.shape = mShape;
    edge.bulge = mBulge;
    edge.color = mColorName;
    // The ANCHORS are persisted, not the drawn endpoints: the two differ only while a box is
    // collapsed, and then it is the anchor that has to survive -- expanding the box must give the
    // endpoint its old place back rather than the header edge it was clamped to (issue #550 bug 4).
    edge.points.append(mHasAnchors ? mAnchorBegin : mBegin);
    edge.points.append(mWaypoints);
    edge.points.append(mHasAnchors ? mAnchorEnd : mEnd);
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

bool SMEdgeItem::hasGeometryDrift() const
{
    SMScene* canvas = getCanvas();
    if ((mValid == false) || (canvas == nullptr))
    {
        return false;
    }

    const SMLayoutEdge* stored = canvas->getModel().getData().getLayout().findEdge(getElementId());
    if ((stored == nullptr) || (stored->points.size() < 2))
    {
        // Nothing persisted yet: the endpoints are derived from the live boxes and follow them by
        // themselves, so there is nothing to write back -- unless a gesture just pinned anchors.
        return mHasAnchors;
    }

    if (mHasAnchors && ((samePoint(stored->points.first(), mAnchorBegin) == false)
                        || (samePoint(stored->points.last(), mAnchorEnd) == false)))
    {
        return true;
    }

    // Only persisted waypoints count: the corners a self-loop seeds for itself are re-derived
    // from the box on every re-read, so they must not turn a plain click into an undo step.
    if (stored->points.size() > 2)
    {
        if (stored->points.size() != (mWaypoints.size() + 2))
        {
            return true;
        }

        for (int i = 0; i < mWaypoints.size(); ++i)
        {
            if (samePoint(stored->points.at(i + 1), mWaypoints.at(i)) == false)
            {
                return true;
            }
        }
    }

    return (stored->hasLabel && mHasLabel && (samePoint(stored->label, mLabelPos) == false));
}

bool SMEdgeItem::nudgeGeometry(const QPointF& delta)
{
    if ((mValid == false) || ((delta.x() == 0.0) && (delta.y() == 0.0)))
    {
        return false;
    }

    const QRectF srcBox = stateRect(mSourceId);
    const QRectF tgtBox = (mSelfLoop ? srcBox : stateRect(mTargetId));
    if ((srcBox.width() <= 0.0) || (srcBox.height() <= 0.0))
    {
        return false;
    }

    // An endpoint may only travel along the border it is stuck to, and only within that border's
    // straight span: the step that runs across the border is dropped, so the transition never
    // leaves the state it connects. Everything else -- the interior waypoints and the label --
    // takes the whole step, so the run between the two ends moves as one.
    const auto slide = [this, &delta](uint32_t stateId, const QRectF& box, const QPointF& anchor) -> QPointF
    {
        if ((box.width() <= 0.0) || (box.height() <= 0.0))
        {
            return anchor;
        }

        const double  radius = stateRadius(stateId, box);
        const QPointF border = NESMDesign::nearestBorderPoint(box, radius, anchor);
        return NESMDesign::slideBorderPoint(box, radius, border, border + delta, 0, false);
    };

    const QPointF begin = slide(mSourceId, srcBox, mHasAnchors ? mAnchorBegin : mBegin);
    const QPointF end   = slide((mSelfLoop ? mSourceId : mTargetId), tgtBox, mHasAnchors ? mAnchorEnd : mEnd);

    prepareGeometryChange();
    for (QPointF& point : mWaypoints)
    {
        point += delta;
    }

    if (mHasLabel)
    {
        mLabelPos += delta;
    }

    setAnchorPoint(true , begin);
    setAnchorPoint(false, end);
    rebuildPath();
    if (mHasLabel)
    {
        mLabelPos = clampLabelPos(mLabelPos);   // the line moved too; keep the label tethered to it
    }

    update();
    return true;
}

bool SMEdgeItem::nudgeSelectedPoint(int dx, int dy, bool coarse, bool pixelWise)
{
    if ((mSelectedPoint < 0) || (mSelectedPoint >= mWaypoints.size()))
    {
        return false;
    }

    // Shift => exact single-pixel step; Ctrl => 10-unit coarse step; otherwise 5-unit step.
    const int base = (coarse ? 10 : 5);
    QPointF point = mWaypoints.at(mSelectedPoint);
    point.setX(nudgeAxis(point.x(), dx, base, pixelWise));
    point.setY(nudgeAxis(point.y(), dy, base, pixelWise));
    if (point == mWaypoints.at(mSelectedPoint))
    {
        return true;    // consumed, but the point did not move
    }

    prepareGeometryChange();
    mWaypoints[mSelectedPoint] = point;
    rebuildPath();
    update();

    // One undo step per key press (a fresh gesture keeps consecutive presses separate).
    mGesture = SMMoveNodeCommand::takeNextGesture();
    commitGeometry(translate("Move waypoint"));
    return true;
}

QPointF SMEdgeItem::nearestPathPoint(const QPointF& point) const
{
    if (mPath.isEmpty())
    {
        return point;
    }

    QPointF best = mPath.first();
    double bestDist = distance(point, best);
    for (int i = 1; i < mPath.size(); ++i)
    {
        QPointF closest;
        const double d = segmentDistance(point, mPath.at(i - 1), mPath.at(i), closest);
        if (d < bestDist)
        {
            bestDist = d;
            best = closest;
        }
    }

    return best;
}

QPointF SMEdgeItem::clampLabelPos(const QPointF& candidate) const
{
    if (mPath.size() < 2)
    {
        return candidate;
    }

    const QPointF anchor = nearestPathPoint(candidate);
    const double d = distance(candidate, anchor);
    if (d <= NESMDesign::EdgeLabelMaxOffset)
    {
        return candidate;
    }

    return anchor + (candidate - anchor) * (NESMDesign::EdgeLabelMaxOffset / d);
}

void SMEdgeItem::setLabelActive(bool active)
{
    if (active && (mValid == false))
    {
        return;
    }

    if (active && (mHasLabel == false))
    {
        // Freeze the label at its CURRENT on-screen position -- the default placement sits ABOVE the
        // line, so seeding from the line anchor would shift the text down onto the transition. Read
        // labelRect() while mHasLabel is still false to get that above-line rect and keep its centre.
        const QRectF current = labelRect();
        mLabelPos = current.isNull() ? labelAnchor() : current.center();
        mHasLabel = true;
    }

    if (mLabelActive != active)
    {
        mLabelActive = active;
        prepareGeometryChange();
        update();
    }
}

void SMEdgeItem::setActiveEnd(int which)
{
    if (which != mActiveEnd)
    {
        mActiveEnd = which;
        update();
    }
}

bool SMEdgeItem::nudgeLabel(int dx, int dy, bool coarse, bool pixelWise)
{
    if (mLabelActive == false)
    {
        return false;
    }

    if (mHasLabel == false)
    {
        mHasLabel = true;
        mLabelPos = labelAnchor();
    }

    const int base = (coarse ? 10 : 5);
    QPointF point = mLabelPos;
    point.setX(nudgeAxis(point.x(), dx, base, pixelWise));
    point.setY(nudgeAxis(point.y(), dy, base, pixelWise));
    point = clampLabelPos(point);
    if (point == mLabelPos)
    {
        return true;    // consumed, but the label did not move (already at the clamp radius)
    }

    prepareGeometryChange();
    mLabelPos = point;
    update();

    mGesture = SMMoveNodeCommand::takeNextGesture();
    commitGeometry(translate("Move edge label"));
    return true;
}

bool SMEdgeItem::nudgeActiveEnd(int dx, int dy, bool coarse, bool pixelWise)
{
    if (mActiveEnd == 0)
    {
        return false;
    }

    const bool     begin   = (mActiveEnd == 1);
    const uint32_t stateId = (begin ? mSourceId : mTargetId);
    const QRectF   box     = stateRect(stateId);
    if ((box.width() <= 0.0) || (box.height() <= 0.0))
    {
        return true;
    }

    QPointF anchor = (begin ? mAnchorBegin : mAnchorEnd);
    if (mHasAnchors == false)
    {
        anchor = (begin ? mBegin : mEnd);    // first manual endpoint move: start from the drawn path
    }

    const int base = (coarse ? 10 : 5);
    anchor.setX(nudgeAxis(anchor.x(), dx, base, pixelWise));
    anchor.setY(nudgeAxis(anchor.y(), dy, base, pixelWise));

    SMScene* canvas = getCanvas();
    const int grid  = (canvas != nullptr) ? canvas->getGridSize() : NESMDesign::GridSizeDefault;
    const QPointF glued = NESMDesign::gridAlignedBorderPoint(box, stateRadius(stateId, box), anchor, grid);

    if (mHasAnchors && (glued == (begin ? mAnchorBegin : mAnchorEnd)))
    {
        return true;
    }

    prepareGeometryChange();
    setAnchorPoint(begin, glued);
    rebuildPath();
    update();

    mGesture = SMMoveNodeCommand::takeNextGesture();
    commitGeometry(translate("Move endpoint"));
    return true;
}

void SMEdgeItem::startNoteEdit()
{
    SMScene* canvas = getCanvas();
    if ((canvas == nullptr) || mNoteEditor.isActive())
    {
        return;
    }

    StateMachineData& data = canvas->getModel().getData();
    const SMLayoutNote* note = data.getLayout().findNoteByOwner(getElementId());
    if (note == nullptr)
    {
        return;
    }

    const uint32_t noteId = note->id;

    // The editor floats near the label (item-local coordinates are scene coordinates for an
    // edge); focus-out commits and collapses back to the label note badge.
    const QRectF label = labelRect();
    const QRectF area{ label.center().x() - 75.0, label.bottom() + 4.0, 150.0, 80.0 };
    mNoteEditor.open(this, area, note->text, [this, noteId](const QString& text) {
        SMScene* c = getCanvas();
        if (c == nullptr)
        {
            return;
        }

        const SMLayoutNote* n = c->getModel().getData().getLayout().findNote(noteId);
        if ((n != nullptr) && (n->text != text))
        {
            c->getModel().getUndoStack().push(new SMSetNoteTextCommand(  c->getModel().getData(), c->getModel().getNotifier()
                                                                       , noteId, text, translate("Edit note")));
        }
    });
}

bool SMEdgeItem::hitsHandle(const QPointF& scenePos) const
{
    if ((mValid == false) || (isSelected() == false))
    {
        return false;
    }

    const QPointF p = mapFromScene(scenePos);
    if ((distance(p, mBegin) <= NESMDesign::EndpointPickRadius)
        || (distance(p, mEnd) <= NESMDesign::EndpointPickRadius))
    {
        return true;
    }

    return (hitWaypoint(p) >= 0) || labelRect().contains(p);
}

QVariant SMEdgeItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        // A selected edge raises to the top (above even a selected state box, z = 2) so its
        // endpoint and waypoint handles stay grabbable where they overlap a box; an unselected
        // edge falls back to z = 1, still above the inactive boxes (see the constructor).
        setZValue(value.toBool() ? 3.0 : 1.0);
        if (value.toBool() == false)
        {
            setSelectedPoint(-1);   // a deselected edge has no active point to nudge
            setActiveEnd(0);
            setLabelActive(false);
        }
    }

    return SMCanvasItem::itemChange(change, value);
}

void SMEdgeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // Ctrl+Shift + primary click on a referenced label part is a link: the stimulus and the
    // operations jump to their declarations; the guard opens the transition's guard editor. Plain
    // clicks are untouched, so clicking a label to select or drag the transition still works.
    const Qt::KeyboardModifiers mods = event->modifiers();
    const bool linkMode = mods.testFlag(Qt::ControlModifier) && mods.testFlag(Qt::ShiftModifier);
    if ((event->button() == Qt::LeftButton) && linkMode)
    {
        const eLink region = linkRegionAt(event->pos());
        if (region != LinkNone)
        {
            if (SMScene* canvas = getCanvas())
            {
                if (region == LinkStimulus)
                {
                    canvas->requestGotoDefinition(getElementId(), false, SMScene::GotoStimulus);
                }
                else if (region == LinkGuard)
                {
                    canvas->requestGuardEdit(getElementId());
                }
                else
                {
                    canvas->requestGotoDefinition(getElementId(), false, SMScene::GotoAction);
                }
            }

            event->accept();
            return;
        }
    }

    if ((event->button() == Qt::LeftButton) && mHasNote && noteBadgeRect().contains(event->pos()))
    {
        startNoteEdit();
        event->accept();
        return;
    }

    if ((event->button() == Qt::LeftButton) && isSelected())
    {
        const QPointF p = event->pos();
        if (distance(p, mBegin) <= NESMDesign::EndpointPickRadius)
        {
            mDrag = eDrag::Begin;
            mDragPoint = mBegin;
            setSelectedPoint(-1);
            setLabelActive(false);
            setActiveEnd(1);
            mGesture = SMMoveNodeCommand::takeNextGesture();
            event->accept();
            return;
        }

        if (distance(p, mEnd) <= NESMDesign::EndpointPickRadius)
        {
            mDrag = eDrag::End;
            mDragPoint = mEnd;
            setSelectedPoint(-1);
            setLabelActive(false);
            setActiveEnd(2);
            mGesture = SMMoveNodeCommand::takeNextGesture();
            event->accept();
            return;
        }

        if ((mShape == SMLayoutEdge::eShape::Arc) && (distance(p, arcApex()) <= NESMDesign::EndpointPickRadius))
        {
            mDrag = eDrag::Bulge;
            setSelectedPoint(-1);
            setLabelActive(false);
            setActiveEnd(0);
            mGesture = SMMoveNodeCommand::takeNextGesture();
            event->accept();
            return;
        }

        const int wp = hitWaypoint(p);
        if (wp >= 0)
        {
            mDrag = eDrag::Waypoint;
            mDragIndex = wp;
            // Grabbing a waypoint also makes it the active point for keyboard nudging.
            setSelectedPoint(wp);
            setLabelActive(false);
            setActiveEnd(0);
            mGesture = SMMoveNodeCommand::takeNextGesture();
            event->accept();
            return;
        }

        if (labelRect().contains(p) || (mLabelActive && actionRect().contains(p)))
        {
            mDrag = eDrag::Label;
            setSelectedPoint(-1);
            setActiveEnd(0);
            mGesture = SMMoveNodeCommand::takeNextGesture();
            event->accept();
            return;
        }
    }

    // A press elsewhere on the edge (selecting the line) drops any active highlight / reposition mode.
    setSelectedPoint(-1);
    setActiveEnd(0);
    setLabelActive(false);
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

    case eDrag::Bulge:
        // Curvature follows the pointer freely: snapping the apex to the grid would quantise the
        // curve into visible steps, and the arc is judged by eye, not by coordinate.
        mBulge = bulgeFor(event->scenePos());
        rebuildPath();
        break;

    case eDrag::Begin:
    case eDrag::End:
    {
        // Free follow for reconnection feedback; near a state box the endpoint sticks to the
        // border AND snaps to a grid-aligned border position (crossing or midpoint), so it settles
        // on stable steps instead of jittering under the cursor (issue #532).
        QPointF point = event->scenePos();
        SMStateItem* over = (canvas != nullptr ? canvas->stateAt(point) : nullptr);
        if (over != nullptr)
        {
            const int grid = (canvas != nullptr) ? canvas->getGridSize() : NESMDesign::GridSizeDefault;
            point = canvas->isSnapToGrid()
                    ? NESMDesign::gridAlignedBorderPoint(over->getVisibleGeometry(), over->boxCornerRadius(), point, grid)
                    : NESMDesign::nearestBorderPoint(over->getVisibleGeometry(), over->boxCornerRadius(), point);
        }

        mDragPoint = point;
        rebuildPath();
        break;
    }

    case eDrag::Label:
        // The label moves freely in any direction (no grid snap) within the 30px band around the
        // line, so it can be pulled right up to the transition; clampLabelPos enforces the limit.
        mLabelPos = clampLabelPos(event->scenePos());
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

    case eDrag::Bulge:
        commitGeometry(translate("Curve transition"));
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
            const uint32_t ownId  = (drag == eDrag::End ? mTargetId : mSourceId);
            const uint32_t tid    = getElementId();
            const QRectF   ownBox = stateRect(ownId);
            if ((overId != 0) && (overId != ownId))
            {
                // a Start state is a source only, no transition may END on it, and a Final state is a target only -- no
                // transition may BEGIN on it. Reject such a drop: restore the edge to its stored
                // geometry (undo the drag feedback) and warn briefly at the cursor.
                const SMStateEntry* overState = canvas->getModel().getData().findStateById(overId);
                const SMStateEntry::eStateKind overKind =
                        (overState != nullptr ? overState->getKind() : SMStateEntry::eStateKind::Normal);
                const bool rejectEnd   = (drag == eDrag::End)   && (overKind == SMStateEntry::eStateKind::Start);
                // A Start is a source only for its OWN initial transitions, which are taken on
                // entering the level and name no stimulus. Dragging an ordinary transition's
                // begin onto one would hand a marker a stimulus to react to, and dragging an
                // initial one off it would leave an ordinary state with a transition nothing can
                // trigger -- so the begin endpoint may neither land on a Start nor leave one.
                const bool startSource = (overKind == SMStateEntry::eStateKind::Start) || mSourceIsStart;
                const bool rejectBegin = (drag == eDrag::Begin)
                                      && ((overKind == SMStateEntry::eStateKind::Final) || startSource);
                if (rejectEnd || rejectBegin)
                {
                    updateFromModel();      // snap the endpoint back to the unchanged connection
                    const QList<QGraphicsView*> viewList = canvas->views();
                    const QString reason = rejectEnd
                            ? translate("A transition cannot enter a Start state.")
                            : (startSource ? translate("An initial transition belongs to its Start state and cannot be moved.")
                                           : translate("A transition cannot leave a Final state."));
                    QToolTip::showText(QCursor::pos(), reason, (viewList.isEmpty() ? nullptr : viewList.first()));
                    break;
                }

                // Dropped on another state: reconnect. Pin the dragged endpoint to the border of
                // the drop state at the release position, reset the label so it re-centres on the
                // new line, and hand the finished geometry to the deferred reconnection. The drag
                // feedback is NOT reverted here: keeping the dropped path drawn until the command
                // redraws the identical geometry removes the one-frame flash back to the old anchor.
                const QRectF  overBox = over->getVisibleGeometry();
                const double  overRad = over->boxCornerRadius();
                const int     grid    = canvas->getGridSize();
                const QPointF glued   = canvas->isSnapToGrid()
                        ? NESMDesign::gridAlignedBorderPoint(overBox, overRad, event->scenePos(), grid)
                        : NESMDesign::nearestBorderPoint(overBox, overRad, canvas->snappedPosition(event->scenePos()));

                mHasLabel = false;      // re-derive the label at the new line's midpoint
                if (drag == eDrag::End)
                {
                    mEnd = glued;
                }
                else
                {
                    mBegin = glued;
                }

                // Seeds both anchors from the drawn path when there were none, so the endpoint
                // that was not dragged survives the reconnection.
                setAnchorPoint(drag != eDrag::End, glued);

                const SMLayoutEdge geometry = buildGeometry();
                if (drag == eDrag::End)
                {
                    QTimer::singleShot(0, canvas, [canvas, tid, overId, geometry]() { canvas->reconnectTransitionTarget(tid, overId, geometry); });
                }
                else
                {
                    QTimer::singleShot(0, canvas, [canvas, tid, overId, geometry]() { canvas->reparentTransition(tid, overId, geometry); });
                }
            }
            else if ((ownBox.width() > 0.0) && (ownBox.height() > 0.0))
            {
                // Dropped on the own state or empty canvas: stick the endpoint to its own state's
                // border at a grid-aligned position (crossing or midpoint) and persist the move.
                const int grid = canvas->getGridSize();
                const QPointF glued = canvas->isSnapToGrid()
                        ? NESMDesign::gridAlignedBorderPoint(ownBox, stateRadius(ownId, ownBox), event->scenePos(), grid)
                        : NESMDesign::nearestBorderPoint(ownBox, stateRadius(ownId, ownBox), canvas->snappedPosition(event->scenePos()));
                prepareGeometryChange();
                setAnchorPoint(drag != eDrag::End, glued);
                rebuildPath();
                update();
                commitGeometry(translate("Move endpoint"));

                // Keep this endpoint the active one so the arrow keys go on moving it along the
                // border without a re-grab (issue #532 -- "then with the keys the user may move it").
                setActiveEnd(drag == eDrag::End ? 2 : 1);
            }
            else
            {
                updateFromModel();
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

        // Double-click on the trigger/guard text or the operation summary toggles label reposition
        // mode: the block is framed and tethered to the line so the user can move it with the mouse
        // or the arrow keys (issue #532). Conditions are still edited from the Properties panel.
        if (labelRect().contains(p) || actionRect().contains(p))
        {
            setLabelActive(mLabelActive == false);
            setSelectedPoint(-1);
            setActiveEnd(0);
            event->accept();
            return;
        }

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
