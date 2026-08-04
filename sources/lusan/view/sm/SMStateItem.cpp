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
 *  \file        lusan/view/sm/SMStateItem.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas state box item.
 *
 ************************************************************************/

#include "lusan/view/sm/SMStateItem.hpp"

#include "lusan/data/sm/SMLayoutData.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMOperationSummary.hpp"
#include "lusan/model/sm/SMOperationValidation.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMEdgeItem.hpp"
#include "lusan/view/sm/SMScene.hpp"

#include <QCoreApplication>
#include <QCursor>
#include <QFontMetricsF>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSignalBlocker>
#include <QStringList>
#include <QToolTip>

#include <algorithm>
#include <cmath>
#include <functional>
#include <utility>

namespace
{
    inline QString translate(const char* text)
    {
        return QCoreApplication::translate("SMStateItem", text);
    }

    /**
     * \brief   The in-place name editor: Esc cancels, Enter is swallowed while the
     *          current text is invalid (the reason is shown as a tooltip).
     **/
    class RenameEdit : public QLineEdit
    {
    public:
        std::function<QString(const QString&)>  mValidate;  //!< Returns the rejection reason, empty = valid.
        std::function<void()>                   mCancel;    //!< Abandons the edit.

        explicit RenameEdit(const QString& text)
            : QLineEdit(text)
        {
        }

    protected:
        virtual bool event(QEvent* event) override
        {
            if (event->type() == QEvent::ShortcutOverride)
            {
                const QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
                switch (keyEvent->key())
                {
                case Qt::Key_Backspace:
                case Qt::Key_Delete:
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_Return:
                case Qt::Key_Enter:
                case Qt::Key_Escape:
                    event->accept();
                    return true;

                default:
                    break;
                }
            }

            return QLineEdit::event(event);
        }

        virtual void keyPressEvent(QKeyEvent* event) override
        {
            const Qt::KeyboardModifiers navMods = event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);
            const bool plainOrShift = (navMods == Qt::NoModifier) || (navMods == Qt::ShiftModifier);

            if (event->key() == Qt::Key_Escape)
            {
                event->accept();
                if (mCancel)
                {
                    mCancel();
                }

                return;
            }

            if ((event->key() == Qt::Key_Left) && plainOrShift && hasSelectedText())
            {
                home(navMods.testFlag(Qt::ShiftModifier));
                event->accept();
                return;
            }

            if ((event->key() == Qt::Key_Right) && plainOrShift && hasSelectedText())
            {
                end(navMods.testFlag(Qt::ShiftModifier));
                event->accept();
                return;
            }

            if (event->key() == Qt::Key_Up)
            {
                home(event->modifiers().testFlag(Qt::ShiftModifier));
                event->accept();
                return;
            }

            if (event->key() == Qt::Key_Down)
            {
                end(event->modifiers().testFlag(Qt::ShiftModifier));
                event->accept();
                return;
            }

            if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter))
            {
                const QString reason = (mValidate ? mValidate(text().trimmed()) : QString());
                if (reason.isEmpty() == false)
                {
                    QToolTip::showText(mapToGlobal(QPoint(0, height())), reason, this);
                    event->accept();
                    return;
                }
            }

            QLineEdit::keyPressEvent(event);
        }
    };

    //!< The placeholder for a group's first row when the group has no action method. The row keeps
    //!< the band mark on a line of its own, so it never displaces the mark of the row below.
    const QString NoActionText { QStringLiteral("...") };

    //!< The text of one behavior row for an operation.
    QString operationText(const SMOperationBase& op)
    {
        switch (op.getOperationType())
        {
        case SMOperationBase::eOperation::ActionCall:
            return op.getName() + QStringLiteral("()");
        case SMOperationBase::eOperation::AttributeSet:
            return op.getName() + QStringLiteral(" = ...");
        case SMOperationBase::eOperation::TimerStart:
            return QStringLiteral("start ") + op.getName();
        case SMOperationBase::eOperation::TimerStop:
            return QStringLiteral("stop ") + op.getName();
        case SMOperationBase::eOperation::EventSend:
            return QStringLiteral("send ") + op.getName();
        case SMOperationBase::eOperation::InlineCode:
        default:
            return QStringLiteral("{ ... }");
        }
    }

    //!< Drops the leading verb (`send `, `start `, `stop `) the shared one-line summary prepends.
    //!< Each body row already carries its own kind mark, so the word is redundant beside it.
    QString withoutRowVerb(const SMOperationBase& op, QString text)
    {
        const char* verb = nullptr;
        switch (op.getOperationType())
        {
        case SMOperationBase::eOperation::EventSend:
            verb = "send ";
            break;
        case SMOperationBase::eOperation::TimerStart:
            verb = "start ";
            break;
        case SMOperationBase::eOperation::TimerStop:
            verb = "stop ";
            break;
        default:
            break;
        }

        if (verb != nullptr)
        {
            const QString prefix = QString::fromLatin1(verb);
            if (text.startsWith(prefix))
            {
                text = text.mid(prefix.length());
            }
        }

        return text;
    }

}

SMStateItem::SMStateItem(uint32_t stateId, QGraphicsItem* parent /*= nullptr*/)
    : SMCanvasItem      (stateId, parent)
    , mSize             (NESMDesign::StateDefaultWidth, NESMDesign::StateDefaultHeight)
    , mName             ( )
    , mKind             (SMStateEntry::eStateKind::Normal)
    , mHistory          (SMStateEntry::eHistory::None)
    , mComposite        (false)
    , mImported         (false)
    , mSubmachine       ( )
    , mExpanded         (true)
    , mActionSeverity   (-1)
    , mColorName        ( )
    , mHeaderColorName  ( )
    , mRows             ( )
    , mHasNote          (false)
    , mResizeHandle     (eHandle::None)
    , mResizeStart      ( )
    , mRenameProxy      (nullptr)
    , mClosingRename    (false)
    , mHoverRow         (-1)
    , mLinkClick        (false)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setAcceptHoverEvents(true);
}

SMStateItem::~SMStateItem()
{
}

QRectF SMStateItem::getBoxGeometry() const
{
    return QRectF(pos(), mSize);
}

QRectF SMStateItem::getVisibleGeometry() const
{
    return QRectF(pos(), QSizeF(mSize.width(), visibleHeight()));
}

bool SMStateItem::isBorderDragZone(const QPointF& scenePos) const
{
    if (isRenameActive())
    {
        return false;
    }

    const QPointF p = mapFromScene(scenePos);
    if (hitHandle(p) != eHandle::None)
    {
        return false;
    }

    if ((hasBodyContent() || (mExpanded == false)) && (isMarker() == false) && chevronRect().contains(p))
    {
        return false;
    }

    const QRectF box{ 0.0, 0.0, mSize.width(), visibleHeight() };
    if (isMarker())
    {
        // The pill border band: within the drag margin of the drawn rounded border.
        const QPointF nearest = NESMDesign::nearestBorderPoint(box, boxCornerRadius(), p);
        return (std::hypot(p.x() - nearest.x(), p.y() - nearest.y()) <= NESMDesign::EdgeBorderDragMargin);
    }

    const double m = NESMDesign::EdgeBorderDragMargin;
    const QRectF outer = box.adjusted(-m, -m, m, m);
    const QRectF inner = box.adjusted(m, m, -m, -m);
    return outer.contains(p) && (inner.contains(p) == false);
}

double SMStateItem::boxCornerRadius() const
{
    return (isMarker() ? std::min(visibleHeight(), mSize.width()) / 2.0 : NESMDesign::StateCornerRadius);
}

void SMStateItem::notifyEdgesOfGeometry()
{
    SMScene* canvas = getCanvas();
    if (canvas != nullptr)
    {
        canvas->updateEdgesForState(getElementId());
    }
}

double SMStateItem::visibleHeight() const
{
    return (mExpanded ? mSize.height() : NESMDesign::StateHeaderHeight);
}

QRectF SMStateItem::boundingRect() const
{
    const double margin = NESMDesign::HandleSize;
    return QRectF(-margin, -margin, mSize.width() + 2.0 * margin, visibleHeight() + 2.0 * margin);
}

QPainterPath SMStateItem::shape() const
{
    QPainterPath path;
    if (isSelected())
    {
        // Cover the whole resize-handle band with a plain rectangle, so the corner handles stay
        // hit-testable. A rounded shape put those of a compact marker pill out of reach.
        const double m = NESMDesign::HandleSize;
        path.addRect(QRectF(-m / 2.0, -m / 2.0, mSize.width() + m, visibleHeight() + m));
        return path;
    }

    const double radius = boxCornerRadius();
    path.addRoundedRect(QRectF(0.0, 0.0, mSize.width(), visibleHeight()), radius, radius);
    return path;
}

void SMStateItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* widget)
{
    const QPalette palette{ (widget != nullptr) ? widget->palette() : QPalette() };
    const QRectF   box{ 0.0, 0.0, mSize.width(), visibleHeight() };

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (isMarker())
    {
        paintMarker(painter, box, palette);
        if (mHasNote)
        {
            QColor fill{ mColorName };
            if (fill.isValid() == false)
            {
                fill = (mKind == SMStateEntry::eStateKind::Start)
                        ? NESMDesign::startStateColor(palette) : NESMDesign::finalStateColor(palette);
            }

            paintNoteBadge(painter, NESMDesign::contrastTextColor(fill));
        }
    }
    else
    {
        const double radius  = NESMDesign::StateCornerRadius;
        const double headerH = NESMDesign::StateHeaderHeight;

        QColor bodyColor{ mColorName };
        if (bodyColor.isValid() == false)
        {
            bodyColor = NESMDesign::stateBodyColor(palette);
        }

        QColor headerColor{ mHeaderColorName };
        if (headerColor.isValid() == false)
        {
            headerColor = NESMDesign::deriveHeaderShade(bodyColor);
        }

        QPainterPath path;
        path.addRoundedRect(box, radius, radius);
        painter->fillPath(path, bodyColor);

        painter->save();
        painter->setClipPath(path);
        painter->fillRect(QRectF(0.0, 0.0, box.width(), headerH), headerColor);
        painter->restore();

        const QColor borderColor = NESMDesign::stateBorderColor(palette);
        painter->setPen(QPen(borderColor, 1.2));
        painter->setBrush(Qt::NoBrush);
        if (mExpanded && (box.height() > headerH))
        {
            painter->drawLine(QPointF(0.0, headerH), QPointF(box.width(), headerH));
        }

        painter->drawPath(path);
        paintHeaderContent(painter, box, headerColor);
        if (mHasNote)
        {
            paintNoteBadge(painter, NESMDesign::contrastTextColor(headerColor));
        }

        if (mExpanded)
        {
            paintBodyRows(painter, box, bodyColor);
            paintMiniature(painter, box, bodyColor);
        }
    }

    if (isSelected())
    {
        const QRectF frame = box.adjusted(-2.0, -2.0, 2.0, 2.0);
        NESMDesign::paintSelectionFrame(painter, frame, palette, hasFocus());
        paintHandles(painter, palette);
    }
}

void SMStateItem::paintMarker(QPainter* painter, const QRectF& box, const QPalette& palette)
{
    const bool start = (mKind == SMStateEntry::eStateKind::Start);

    QColor fill{ mColorName };
    if (fill.isValid() == false)
    {
        fill = (start ? NESMDesign::startStateColor(palette) : NESMDesign::finalStateColor(palette));
    }

    const double radius = boxCornerRadius();
    QPainterPath path;
    path.addRoundedRect(box, radius, radius);
    painter->fillPath(path, fill);
    painter->setPen(QPen(NESMDesign::stateBorderColor(palette), 1.2));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);

    const QColor textColor = NESMDesign::contrastTextColor(fill);
    if (start == false)
    {
        // Final: the classic double border (an inner ring inside the pill).
        QColor ring{ textColor };
        ring.setAlphaF(0.85);
        painter->setPen(QPen(ring, 1.4));
        const QRectF inner = box.adjusted(3.5, 3.5, -3.5, -3.5);
        const double ir = std::max(radius - 3.5, 2.0);
        painter->drawRoundedRect(inner, ir, ir);
    }

    // The glyph and the name, centered together: Start = play triangle, Final = bullseye.
    // The marker pill is compact, so its label uses a slightly smaller font than the header.
    QFont nameFont = painter->font();
    nameFont.setBold(true);
    nameFont.setPointSizeF(std::max(nameFont.pointSizeF() * 0.85, 6.5));
    const QFontMetricsF metrics{ nameFont };

    const double glyphW  = 9.0;
    const double gap     = 4.0;
    // The marker pill is only four grid cells wide, so a tight padding keeps "Start" and "Final"
    // un-elided. The centered text never reaches the rounded corners.
    const double padding = 4.0 + radius * 0.25;
    const double availW  = std::max(box.width() - 2.0 * padding - glyphW - gap, 10.0);
    const QString elided = metrics.elidedText(mName, Qt::ElideRight, availW);
    const double textW   = metrics.horizontalAdvance(elided);
    const double left    = box.center().x() - (glyphW + gap + textW) / 2.0;
    const double midY    = box.center().y();

    if (start)
    {
        const double gh = glyphW * 0.6;
        QPainterPath glyph;
        glyph.moveTo(left, midY - gh);
        glyph.lineTo(left + glyphW, midY);
        glyph.lineTo(left, midY + gh);
        glyph.closeSubpath();
        painter->setPen(Qt::NoPen);
        painter->setBrush(textColor);
        painter->drawPath(glyph);
    }
    else
    {
        const QPointF center{ left + glyphW / 2.0, midY };
        painter->setPen(QPen(textColor, 1.4));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(center, glyphW / 2.0, glyphW / 2.0);
        painter->setPen(Qt::NoPen);
        painter->setBrush(textColor);
        painter->drawEllipse(center, glyphW / 4.0, glyphW / 4.0);
    }

    painter->setFont(nameFont);
    painter->setPen(textColor);
    const QRectF nameRect{ left + glyphW + gap, box.top(), textW + 2.0, box.height() };
    painter->drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft, elided);
}

void SMStateItem::paintHeaderContent(QPainter* painter, const QRectF& box, const QColor& headerColor)
{
    const double  headerH   = NESMDesign::StateHeaderHeight;
    const double  padding   = NESMDesign::StatePadding;
    const QColor  textColor = NESMDesign::contrastTextColor(headerColor);
    double        left      = padding;
    double        right     = box.width() - padding;

    // Right-to-left: chevron, then the badges.
    if (hasBodyContent() || (mExpanded == false))
    {
        const QRectF chevron = chevronRect();
        QPen pen{ textColor, 1.4 };
        pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
        const QPointF c = chevron.center();
        if (mExpanded)
        {
            painter->drawLine(c + QPointF(-3.5, -1.5), c + QPointF(0.0, 2.0));
            painter->drawLine(c + QPointF(3.5, -1.5), c + QPointF(0.0, 2.0));
        }
        else
        {
            painter->drawLine(c + QPointF(-1.5, -3.5), c + QPointF(2.0, 0.0));
            painter->drawLine(c + QPointF(-1.5, 3.5), c + QPointF(2.0, 0.0));
        }

        right = chevron.left() - 4.0;
    }

    QFont badgeFont = painter->font();
    badgeFont.setPointSizeF(badgeFont.pointSizeF() * 0.75);
    badgeFont.setBold(true);

    if (mHistory != SMStateEntry::eHistory::None)
    {
        // `H*` is wider than `H`, so the ring grows with its mark instead of clipping the star.
        const QString mark = QString::fromLatin1(mHistory == SMStateEntry::eHistory::Deep ? "H*" : "H");
        const double  width = std::max(QFontMetricsF{ badgeFont }.horizontalAdvance(mark) + 7.0, 14.0);
        const QRectF  badge{ right - width, (headerH - 14.0) / 2.0, width, 14.0 };
        painter->setPen(QPen(textColor, 1.0));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(badge);
        painter->setFont(badgeFont);
        painter->drawText(badge, Qt::AlignCenter, mark);
        right = badge.left() - 4.0;
    }

    if (mComposite || mImported)
    {
        // An imported machine is named on the box: which machine a state runs is the first thing a
        // reader wants. The alias is elided, so a long name never pushes out the state's own.
        painter->setFont(badgeFont);
        const double aliasW = (mImported && (mSubmachine.isEmpty() == false))
                            ? qMin(QFontMetricsF{ badgeFont }.horizontalAdvance(mSubmachine) + 4.0
                                  , qMax((right - padding) * 0.4, 0.0))
                            : 0.0;

        const QRectF badge{ right - 14.0 - aliasW, (headerH - 12.0) / 2.0, 12.0, 12.0 };
        painter->setPen(QPen(textColor, 1.1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(badge.adjusted(0.0, 0.0, -3.0, -3.0));
        if (mComposite)
        {
            painter->drawRect(badge.adjusted(3.0, 3.0, 0.0, 0.0));
        }
        else
        {
            painter->drawLine(badge.bottomRight(), badge.center());
            painter->drawLine(badge.center(), badge.center() + QPointF(4.0, 0.0));
            painter->drawLine(badge.center(), badge.center() + QPointF(0.0, 4.0));
        }

        if (aliasW > 0.0)
        {
            const QRectF aliasRect{ badge.right() + 2.0, 0.0, aliasW, headerH };
            painter->setPen(textColor);
            painter->drawText(aliasRect, Qt::AlignVCenter | Qt::AlignLeft
                             , QFontMetricsF{ badgeFont }.elidedText(mSubmachine, Qt::ElideRight, aliasW));
        }

        right = badge.left() - 4.0;
    }

    // An incomplete entry/exit action mapping warns with a tinted `(!)` badge, matching the edge
    // glyph so both canvas surfaces read the same.
    if (mActionSeverity >= 0)
    {
        const QColor warn = NEGuardStyle::severityColor(static_cast<NEGuardStyle::eSeverity>(mActionSeverity));
        const QRectF badge{ right - 14.0, (headerH - 14.0) / 2.0, 14.0, 14.0 };
        painter->setPen(QPen(warn, 1.2));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(badge);
        painter->setFont(badgeFont);
        painter->drawText(badge, Qt::AlignCenter, QStringLiteral("!"));
        right = badge.left() - 4.0;
    }

    QFont nameFont = painter->font();
    nameFont.setBold(true);
    painter->setFont(nameFont);
    painter->setPen(textColor);
    const QRectF nameRect{ left, 0.0, std::max(right - left, 10.0), headerH };
    const QString elided = QFontMetrics(nameFont).elidedText(mName, Qt::ElideRight, static_cast<int>(nameRect.width()));
    painter->drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft, elided);
}

QList<SMStateItem::RowSlot> SMStateItem::bodyRowLayout() const
{
    QList<RowSlot> layout;      // note: 'slots' is a Qt keyword macro, so this must not be named that.
    if ((mExpanded == false) || (hasBodyContent() == false))
    {
        return layout;
    }

    // The body reads as three bands in execution order: entry at the top, in-state in the middle,
    // exit anchored to the bottom. paintBodyRows draws this packing and bodyRowAt hit-tests it.
    const QRectF box{ 0.0, 0.0, mSize.width(), visibleHeight() };
    const double rowH   = NESMDesign::StateRowHeight;
    const double top    = NESMDesign::StateHeaderHeight + 2.0;
    const double bottom = box.height() - 2.0;
    const int    rowSlots = static_cast<int>((bottom - top) / rowH);
    if (rowSlots <= 0)
    {
        return layout;
    }

    int counts[3] { 0, 0, 0 };
    for (const BodyRow& row : mRows)
    {
        ++counts[static_cast<int>(row.zone)];
    }

    // When the box is too short for every row, the middle band gives way first, then the exit band,
    // and the entry band last: the two edges are the rows the zoning exists to keep apart.
    int shown[3] { counts[0], counts[1], counts[2] };
    if ((counts[0] + counts[1] + counts[2]) > rowSlots)
    {
        shown[1] = std::max(0, rowSlots - counts[0] - counts[2]);
        shown[2] = std::min(counts[2], std::max(0, rowSlots - counts[0]));
        shown[0] = std::min(counts[0], rowSlots);
    }

    // Where each band starts: entry at the top, exit against the bottom edge, and the middle band
    // centered in whatever room is left between the two.
    const double midTop    = top + (shown[0] * rowH);
    const double midBottom = bottom - (shown[2] * rowH);
    const double bandY[3]
    {
          top
        , midTop + std::max(0.0, ((midBottom - midTop) - (shown[1] * rowH)) / 2.0)
        , midBottom
    };

    int seen[3] { 0, 0, 0 };
    for (int i = 0; i < mRows.size(); ++i)
    {
        const int zone = static_cast<int>(mRows.at(i).zone);
        if (seen[zone] >= shown[zone])
        {
            continue;       // this band is full; its remaining rows are covered by the `...` marker
        }

        // The last slot of a truncated band says so, rather than dropping rows silently.
        const bool truncated = ((seen[zone] + 1) == shown[zone]) && (shown[zone] < counts[zone]);
        layout.append(RowSlot{ i, bandY[zone] + (seen[zone] * rowH), truncated });
        ++seen[zone];
    }

    return layout;
}

int SMStateItem::bodyRowAt(const QPointF& pos) const
{
    const double rowH = NESMDesign::StateRowHeight;
    for (const RowSlot& slot : bodyRowLayout())
    {
        if (slot.truncated)
        {
            continue;       // the "..." overflow marker is not a link.
        }

        const QRectF rowRect{ 0.0, slot.y, mSize.width(), rowH };
        if (rowRect.contains(pos))
        {
            // A row is actionable when it references a declaration or when it is a transition. The
            // header row of an internal transition is the second case.
            const BodyRow& row = mRows.at(slot.index);
            return ((row.refs.isEmpty() == false) || (row.transitionId != 0u)) ? slot.index : -1;
        }
    }

    return -1;
}

const SMStateItem::BodyRow* SMStateItem::bodyRowAtPos(const QPointF& pos) const
{
    const int index = bodyRowAt(pos);
    return (index >= 0) ? &mRows.at(index) : nullptr;
}

void SMStateItem::paintBodyRows(QPainter* painter, const QRectF& box, const QColor& bodyColor)
{
    const QList<RowSlot> layout = bodyRowLayout();
    if (layout.isEmpty())
    {
        return;
    }

    const double rowH    = NESMDesign::StateRowHeight;
    const double padding = NESMDesign::StatePadding;
    const QColor color   = NESMDesign::contrastTextColor(bodyColor);

    QFont rowFont = painter->font();
    rowFont.setPointSizeF(rowFont.pointSizeF() * 0.85);
    const QFontMetrics metrics{ rowFont };

    const auto drawRow = [&](const BodyRow& row, double rowY, bool ellipsis, bool continues, bool linked)
    {
        painter->setFont(rowFont);
        painter->setPen(color);
        if (ellipsis)
        {
            painter->drawText(QRectF(padding, rowY, box.width() - 2.0 * padding, rowH)
                             , Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("..."));
            return;
        }

        // A ` \` continuation cue at the right edge says "the next row belongs to this same
        // Enter/Do/Exit group"; reserve its width so it never overlaps the row text.
        const double cueW = (continues ? (metrics.horizontalAdvance(QStringLiteral("\\")) + 4.0) : 0.0);
        // A second mark takes a second gutter slot, so that row's text starts one mark further in.
        // Only the `on <stimulus>` header of an internal transition has one.
        const bool   twoMarks = SMKindGlyph::isDrawn(row.kindIcon);
        const double gutter   = padding + 16.0 + (twoMarks ? SMKindGlyph::GlyphSize : 0.0);
        const QRectF textRect{ gutter, rowY, box.width() - padding - gutter - cueW, rowH };

        SMKindGlyph::paint(*painter, QRectF(padding, rowY + 2.0, SMKindGlyph::GlyphSize, rowH - 4.0), row.icon, color);
        if (twoMarks)
        {
            SMKindGlyph::paint(*painter, QRectF(padding + SMKindGlyph::GlyphSize, rowY + 2.0
                                              , SMKindGlyph::GlyphSize, rowH - 4.0), row.kindIcon, color);
        }

        painter->setFont(rowFont);
        painter->setPen(color);
        const QString elided = metrics.elidedText(row.text, Qt::ElideRight, static_cast<int>(textRect.width()));
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elided);
        if (continues)
        {
            painter->drawText(QRectF(box.width() - padding - cueW, rowY, cueW, rowH)
                             , Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("\\"));
        }

        // Ctrl+Shift link feedback: underline the hovered row's text so the user sees the link.
        if (linked)
        {
            const double textW   = std::min(static_cast<double>(metrics.horizontalAdvance(elided)), textRect.width());
            const double baseline = rowY + (rowH / 2.0) + ((metrics.ascent() - metrics.descent()) / 2.0) + 1.0;
            painter->drawLine(QPointF(textRect.left(), baseline), QPointF(textRect.left() + textW, baseline));
        }
    };

    for (const RowSlot& slot : layout)
    {
        const BodyRow& row = mRows.at(slot.index);
        const bool linked = (slot.truncated == false) && (slot.index == mHoverRow)
                            && ((row.refs.isEmpty() == false) || (row.transitionId != 0u));
        drawRow(row, slot.y, slot.truncated, row.continues && (slot.truncated == false), linked);
    }
}

QRectF SMStateItem::miniatureRect() const
{
    const double maxW = NESMDesign::MiniatureMaxWidth;
    const double maxH = NESMDesign::MiniatureMaxHeight;
    const double pad  = NESMDesign::MiniaturePadding;
    const double height = visibleHeight();
    if ((mComposite == false) || (mExpanded == false)
        || (height < NESMDesign::StateHeaderHeight + maxH + (2.0 * pad)))
    {
        return QRectF();    // no room, or nothing to hint at
    }

    return QRectF(mSize.width() - pad - maxW, height - pad - maxH, maxW, maxH);
}

void SMStateItem::paintMiniature(QPainter* painter, const QRectF& /*box*/, const QColor& bodyColor)
{
    const QRectF avail = miniatureRect();
    if (avail.isNull())
    {
        return;
    }

    // A fixed symbol of one Start marker and two states, never the real substates. It says that
    // there is a machine inside at any size, where the real ones smeared and cost a repaint.
    const double w = avail.width();
    const double h = avail.height();
    const QRectF start{ avail.left(), avail.top() + (h * 0.34), w * 0.30, h * 0.32 };
    const QRectF first{ avail.left() + (w * 0.44), avail.top() + (h * 0.06), w * 0.56, h * 0.36 };
    const QRectF second{ avail.left() + (w * 0.44), avail.top() + (h * 0.58), w * 0.56, h * 0.36 };

    painter->save();
    painter->setOpacity(painter->opacity() * 0.55);
    painter->setPen(QPen(NESMDesign::contrastTextColor(bodyColor), 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(start);                    // the Start marker, drawn as the pill it is
    painter->drawRoundedRect(first, 1.5, 1.5);
    painter->drawRoundedRect(second, 1.5, 1.5);
    // One stub from the marker into the first state: three loose shapes read as three shapes, and a
    // single connector is what makes them read as a machine.
    painter->drawLine(QPointF(start.right(), start.center().y())
                    , QPointF(first.left(), start.center().y()));
    painter->restore();
}

void SMStateItem::paintHandles(QPainter* painter, const QPalette& palette)
{
    painter->setPen(QPen(palette.color(QPalette::Base), 1.0));
    painter->setBrush(NESMDesign::selectionColor(palette));

    constexpr eHandle handles[]
    {
          eHandle::TopLeft, eHandle::Top, eHandle::TopRight, eHandle::Right
        , eHandle::BottomRight, eHandle::Bottom, eHandle::BottomLeft, eHandle::Left
    };

    for (eHandle handle : handles)
    {
        if (isHandleEnabled(handle))
        {
            painter->drawRect(handleRect(handle));
        }
    }
}

bool SMStateItem::isHandleEnabled(eHandle handle) const
{
    if (mExpanded)
    {
        return true;
    }

    // A collapsed box shows only the header; keep its stored height untouched.
    return (handle == eHandle::Left) || (handle == eHandle::Right);
}

QRectF SMStateItem::handleRect(eHandle handle) const
{
    const double size = NESMDesign::HandleSize;
    const double w    = mSize.width();
    const double h    = visibleHeight();

    QPointF center;
    switch (handle)
    {
    case eHandle::TopLeft:      center = QPointF(0.0, 0.0);         break;
    case eHandle::Top:          center = QPointF(w / 2.0, 0.0);     break;
    case eHandle::TopRight:     center = QPointF(w, 0.0);           break;
    case eHandle::Right:        center = QPointF(w, h / 2.0);       break;
    case eHandle::BottomRight:  center = QPointF(w, h);             break;
    case eHandle::Bottom:       center = QPointF(w / 2.0, h);       break;
    case eHandle::BottomLeft:   center = QPointF(0.0, h);           break;
    case eHandle::Left:         center = QPointF(0.0, h / 2.0);     break;
    case eHandle::None:
    default:                    return QRectF();
    }

    return QRectF(center.x() - size / 2.0, center.y() - size / 2.0, size, size);
}

SMStateItem::eHandle SMStateItem::hitHandle(const QPointF& position) const
{
    if (isSelected() == false)
    {
        return eHandle::None;
    }

    constexpr eHandle handles[]
    {
          eHandle::TopLeft, eHandle::TopRight, eHandle::BottomRight, eHandle::BottomLeft
        , eHandle::Top, eHandle::Right, eHandle::Bottom, eHandle::Left
    };

    for (eHandle handle : handles)
    {
        if (isHandleEnabled(handle) && handleRect(handle).adjusted(-1.5, -1.5, 1.5, 1.5).contains(position))
        {
            return handle;
        }
    }

    return eHandle::None;
}

QRectF SMStateItem::chevronRect() const
{
    const double headerH = NESMDesign::StateHeaderHeight;
    // Shift left to make room for the corner note badge when the state carries a note.
    const double shift = (mHasNote ? 17.0 : 0.0);
    return QRectF(mSize.width() - 18.0 - shift, (headerH - 12.0) / 2.0, 12.0, 12.0);
}

QRectF SMStateItem::noteBadgeRect() const
{
    const double headerH = NESMDesign::StateHeaderHeight;
    return QRectF(mSize.width() - 17.0, (headerH - 14.0) / 2.0, 13.0, 14.0);
}

void SMStateItem::paintNoteBadge(QPainter* painter, const QColor& color)
{
    const QRectF badge = noteBadgeRect();

    // A small folded-page glyph: the page outline with a dog-eared top-right corner and
    // two short text lines, signalling "this element has a note".
    const double fold = 4.0;
    QPainterPath page;
    page.moveTo(badge.left(), badge.top());
    page.lineTo(badge.right() - fold, badge.top());
    page.lineTo(badge.right(), badge.top() + fold);
    page.lineTo(badge.right(), badge.bottom());
    page.lineTo(badge.left(), badge.bottom());
    page.closeSubpath();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QColor fill{ color };
    fill.setAlphaF(0.14);
    painter->setBrush(fill);
    QPen pen{ color, 1.0 };
    painter->setPen(pen);
    painter->drawPath(page);

    // The folded corner.
    painter->drawLine(QPointF(badge.right() - fold, badge.top()), QPointF(badge.right() - fold, badge.top() + fold));
    painter->drawLine(QPointF(badge.right() - fold, badge.top() + fold), QPointF(badge.right(), badge.top() + fold));

    // Two text lines.
    const double lx = badge.left() + 2.5;
    const double rx = badge.right() - 2.5;
    painter->drawLine(QPointF(lx, badge.top() + 6.0), QPointF(rx, badge.top() + 6.0));
    painter->drawLine(QPointF(lx, badge.top() + 9.5), QPointF(rx - 2.0, badge.top() + 9.5));
    painter->restore();
}

SMScene* SMStateItem::getCanvas() const
{
    return qobject_cast<SMScene*>(scene());
}

const SMStateEntry* SMStateItem::getState() const
{
    SMScene* canvas = getCanvas();
    return (canvas != nullptr ? canvas->getModel().getData().findStateById(getElementId()) : nullptr);
}

void SMStateItem::updateFromModel()
{
    SMScene* canvas = getCanvas();
    if (canvas == nullptr)
    {
        return;
    }

    StateMachineData& data = canvas->getModel().getData();
    const SMStateEntry* state = data.findStateById(getElementId());
    if (state == nullptr)
    {
        return;
    }

    prepareGeometryChange();
    mName      = state->getName();
    mKind      = state->getKind();
    mHistory   = state->getHistory();
    // A submachine counts as composite only when it owns at least one Normal state. One holding
    // just its Start marker is never persisted, so the box must not advertise it.
    mComposite = state->hasNestedStates() && state->getNestedStates()->hasRealState();
    mImported  = state->isImportedSubmachine();
    mSubmachine = state->getSubmachine();
    mHasNote   = (data.getLayout().findNoteByOwner(getElementId()) != nullptr);

    // An entry/exit action whose arguments are not fully mapped warns in the header, so a method
    // edit that breaks a mapping is visible on the canvas without opening the Properties panel.
    mActionSeverity = -1;
    DocIssue::eSeverity opSeverity = DocIssue::eSeverity::Info;
    if (SMOperationValidation::worstForState(data, getElementId(), opSeverity))
    {
        mActionSeverity = static_cast<int>((opSeverity == DocIssue::eSeverity::Error)
                                           ? NEGuardStyle::eSeverity::Err
                                           : NEGuardStyle::eSeverity::Warn);
    }

    rebuildRows(*state);

    const SMLayoutNode* node = data.getLayout().findNode(getElementId());
    if (node != nullptr)
    {
        // Markers (Start / Final) are compact pills and clamp to a smaller minimum than a
        // normal state box; mKind is already set above, so isMarker() is valid here.
        const double minW = (isMarker() ? NESMDesign::MarkerStateMinWidth : NESMDesign::StateMinWidth);
        const double minH = (isMarker() ? NESMDesign::MarkerStateMinHeight : NESMDesign::StateMinHeight);
        mSize = QSizeF(std::max(node->width, minW), std::max(node->height, minH));
        // Marker boxes have no collapsible body; they always paint at full height.
        mExpanded        = (isMarker() ? true : (node->hasExpanded ? node->expanded : true));
        mColorName       = node->color;
        mHeaderColorName = node->headerColor;

        const QPointF position{ node->x, node->y };
        if (pos() != position)
        {
            setPos(position);
        }
    }

    update();
}

void SMStateItem::rebuildRows(const SMStateEntry& state)
{
    mRows.clear();

    const SMScene* canvas = getCanvas();
    const StateMachineData* data = (canvas != nullptr) ? &canvas->getModel().getData() : nullptr;

    const auto rowText = [data](const SMOperationBase& op) -> QString
    {
        const QString summary = (data != nullptr) ? SMOperationSummary::text(*data, op) : operationText(op);
        // The kind is announced once: by the drawn mark, or -- in the Word style, where no mark is
        // drawn -- by the prefix. Either way the summary's own verb has to go first.
        return SMKindGlyph::prefix(SMKindGlyph::operationGlyph(op)) + withoutRowVerb(op, summary);
    };

    // The band mark of an Enter/Do/Exit group, so the reader sees which activity a row belongs to.
    // An internal transition is not a zone but a construct, and brings its own mark below.
    const auto zoneGlyph = [](eRowZone zone) -> SMKindGlyph::eGlyph
    {
        switch (zone)
        {
        case eRowZone::Enter:   return SMKindGlyph::eGlyph::Entry;
        case eRowZone::Exit:    return SMKindGlyph::exitGlyph();
        default:                return SMKindGlyph::eGlyph::Do;
        }
    };

    // One Enter/Do/Exit group, ordered action, event, then all timers on one row. The first row
    // always carries the band mark, except for an internal transition whose header already has it.
    const auto appendGroup = [&](const SMOperationList& ops, eRowZone zone, bool bandRow = true)
    {
        QList<const SMOperationBase*> actions;
        QList<const SMOperationBase*> events;
        QList<const SMOperationBase*> timers;    // start and stop, in list order
        for (const SMOperationBase* op : ops.getOperations())
        {
            switch (op->getOperationType())
            {
            case SMOperationBase::eOperation::TimerStart:
            case SMOperationBase::eOperation::TimerStop:
                timers.append(op);
                break;
            case SMOperationBase::eOperation::EventSend:
                events.append(op);
                break;
            default:
                actions.append(op);
                break;
            }
        }

        QList<BodyRow> group;
        if (actions.isEmpty())
        {
            if (bandRow && ((events.isEmpty() == false) || (timers.isEmpty() == false)))
            {
                group.append(BodyRow{ zoneGlyph(zone), NoActionText, zone, false, false, { } });
            }
        }
        else
        {
            for (const SMOperationBase* op : std::as_const(actions))
            {
                // A band row takes its band's mark; a group that has no band row of its own (an
                // internal transition's operations) takes the operation's own kind mark, the gear.
                const SMKindGlyph::eGlyph mark = bandRow ? zoneGlyph(zone) : SMKindGlyph::operationGlyph(*op);
                group.append(BodyRow{ mark, rowText(*op), zone, false, false, SMReferences::operationRefs(*op) });
            }
        }

        for (const SMOperationBase* op : std::as_const(events))
        {
            group.append(BodyRow{ SMKindGlyph::eGlyph::Event, rowText(*op), zone, false, false, SMReferences::operationRefs(*op) });
        }
        if (timers.isEmpty() == false)
        {
            // Every timer of the group on one line (`start A | stop B`); the icon follows the first
            // timer so a start-only group shows the play clock and a stop-only group the square clock.
            QStringList parts;
            QList<SMReferences::Ref> timerRefs;    // the row links to every timer it names.
            for (const SMOperationBase* op : std::as_const(timers))
            {
                parts.append(rowText(*op));
                for (const SMReferences::Ref& ref : SMReferences::operationRefs(*op))
                {
                    timerRefs.append(ref);
                }
            }

            group.append(BodyRow{ SMKindGlyph::operationGlyph(*timers.first())
                                , parts.join(QStringLiteral(" | ")), zone, false, false, timerRefs });
        }

        for (int i = 0; i < group.size(); ++i)
        {
            group[i].firstInGroup = (i == 0);
            group[i].continues    = (i < (group.size() - 1));
            mRows.append(group.at(i));
        }
    };

    // Zone by zone -- Enter at the top, everything that runs WHILE in the state in the middle, Exit
    // anchored to the bottom -- so the box reads in the order the state actually executes.
    appendGroup(state.getEntryList(), eRowZone::Enter);
    appendGroup(state.getDoList(), eRowZone::Middle);

    // A transition with a target shows its operations on its edge, one without reads here as its
    // own group whose header separates an internal from an unconnected external transition.
    int internalCount = 0;
    for (const SMTransitionEntry* transition : state.getTransitions().getElements())
    {
        if ((transition != nullptr) && transition->isInternal())
        {
            ++internalCount;
        }
    }

    int internalIndex = 0;
    for (const SMTransitionEntry* transition : state.getTransitions().getElements())
    {
        if (transition->hasTarget() == false)
        {
            const QString stim = (data != nullptr) ? SMOperationSummary::stimulusSignature(*data, *transition) : transition->getStimulus();
            QString head;
            if (transition->isInternal())
            {
                ++internalIndex;

                // The number is the priority, document order deciding which of several transitions
                // on one stimulus runs, and the guard chip is what tells two of them apart.
                if (internalCount > 1)
                {
                    head = QStringLiteral("#") + QString::number(internalIndex) + QLatin1Char(' ');
                }

                head += QStringLiteral("on ") + stim;
                if (data != nullptr)
                {
                    const QString chip = SMGuardRender::chipText(*data, transition->getId(), transition->getGuard()
                                                                , SMGuardRender::ChipStateBox);
                    if (chip.isEmpty() == false)
                    {
                        head += QStringLiteral(" [") + chip + QLatin1Char(']');
                    }
                }
            }
            else
            {
                head = QStringLiteral("on ") + stim + translate(" (not connected)");
            }

            // The row is the transition, so clicking it opens the transition. Its stimulus
            // declaration is the secondary action, offered on the context menu.
            SMReferences::eTarget stimKind = SMReferences::eTarget::Trigger;
            switch (transition->getStimulusKind())
            {
            case SMTransitionEntry::eStimulusKind::Trigger: stimKind = SMReferences::eTarget::Trigger; break;
            case SMTransitionEntry::eStimulusKind::Event:   stimKind = SMReferences::eTarget::Event;   break;
            case SMTransitionEntry::eStimulusKind::Timer:   stimKind = SMReferences::eTarget::Timer;   break;
            }

            QList<SMReferences::Ref> stimRef;
            if (transition->getStimulus().isEmpty() == false)
            {
                stimRef.append({ stimKind, transition->getStimulus() });
            }

            // Two marks: the band mark says what the row is, the kind mark says what fires it, a
            // button, a bolt or a clock. An unfinished external edge gets the kind mark alone.
            BodyRow header{ SMKindGlyph::eGlyph::Internal, head, eRowZone::Middle, false, false, stimRef };
            header.kindIcon    = SMKindGlyph::stimulusGlyph(*transition);
            header.transitionId = transition->getId();
            if (transition->isInternal() == false)
            {
                header.icon     = header.kindIcon;
                header.kindIcon = SMKindGlyph::eGlyph::None;
            }

            mRows.append(header);
            appendGroup(transition->getOperations(), eRowZone::Middle, false);
        }
    }

    appendGroup(state.getExitList(), eRowZone::Exit);
}

void SMStateItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    // Ctrl+Alt over the submachine hint opens the quick view. A different modifier from the link
    // below, since the hint sits inside the body and one chord cannot mean two things there.
    const Qt::KeyboardModifiers mods = event->modifiers();
    const bool peekMode = mods.testFlag(Qt::ControlModifier) && mods.testFlag(Qt::AltModifier);
    const QRectF hint = miniatureRect();
    if (peekMode && (hint.isNull() == false) && hint.contains(event->pos()))
    {
        if (SMScene* canvas = getCanvas())
        {
            canvas->showSubmachinePeek(getElementId(), event->screenPos());
        }

        setCursor(Qt::WhatsThisCursor);
        SMCanvasItem::hoverMoveEvent(event);
        return;
    }

    if (SMScene* canvas = getCanvas())
    {
        canvas->hideSubmachinePeek();
    }

    // Ctrl+Shift held turns each referenced body row into a link: underline the row under the
    // pointer and switch to a link cursor, ahead of the normal resize-handle cursors.
    const bool linkMode = mods.testFlag(Qt::ControlModifier) && mods.testFlag(Qt::ShiftModifier);
    const int linkRow = linkMode ? bodyRowAt(event->pos()) : -1;
    if (linkRow != mHoverRow)
    {
        mHoverRow = linkRow;
        update();
    }

    if (linkRow >= 0)
    {
        setCursor(Qt::PointingHandCursor);
        SMCanvasItem::hoverMoveEvent(event);
        return;
    }

    switch (hitHandle(event->pos()))
    {
    case eHandle::TopLeft:
    case eHandle::BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;

    case eHandle::TopRight:
    case eHandle::BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;

    case eHandle::Left:
    case eHandle::Right:
        setCursor(Qt::SizeHorCursor);
        break;

    case eHandle::Top:
    case eHandle::Bottom:
        setCursor(Qt::SizeVerCursor);
        break;

    case eHandle::None:
    default:
        unsetCursor();
        break;
    }

    SMCanvasItem::hoverMoveEvent(event);
}

void SMStateItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    if (mHoverRow != -1)
    {
        mHoverRow = -1;
        update();
    }

    if (SMScene* canvas = getCanvas())
    {
        canvas->hideSubmachinePeek();
    }

    unsetCursor();
    SMCanvasItem::hoverLeaveEvent(event);
}

void SMStateItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // Ctrl+Shift + primary click on a referenced body row is a link: navigate to what the row's
    // operation references. Plain clicks are untouched, so selecting and moving the box still work.
    const Qt::KeyboardModifiers mods = event->modifiers();
    const bool linkMode = mods.testFlag(Qt::ControlModifier) && mods.testFlag(Qt::ShiftModifier);
    if ((event->button() == Qt::LeftButton) && linkMode)
    {
        const int linkRow = bodyRowAt(event->pos());
        if (linkRow >= 0)
        {
            const BodyRow& row = mRows.at(linkRow);
            if (SMScene* canvas = getCanvas())
            {
                // A row that is a transition opens that transition. Its stimulus declaration stays
                // one right-click away on the state's context menu.
                if (row.transitionId != 0u)
                {
                    canvas->requestInternalEdit(row.transitionId);
                }
                else
                {
                    canvas->requestGotoRefs(row.refs);
                }
            }

            // The matching release is ours too -- see mLinkClick.
            mLinkClick = true;
            event->accept();
            return;
        }
    }

    if (event->button() == Qt::LeftButton)
    {
        if (mHasNote && noteBadgeRect().contains(event->pos()))
        {
            startNoteEdit();
            event->accept();
            return;
        }

        if ((hasBodyContent() || (mExpanded == false)) && chevronRect().contains(event->pos()))
        {
            toggleExpanded();
            event->accept();
            return;
        }

        const eHandle handle = hitHandle(event->pos());
        if (handle != eHandle::None)
        {
            mResizeHandle = handle;
            mResizeStart  = getBoxGeometry();
            event->accept();
            return;
        }
    }

    SMCanvasItem::mousePressEvent(event);
}

void SMStateItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (mResizeHandle != eHandle::None)
    {
        applyResizeDrag(event->scenePos());
        event->accept();
        return;
    }

    SMCanvasItem::mouseMoveEvent(event);
}

void SMStateItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (mLinkClick)
    {
        // The press was consumed as a link, so the base never saw it. Letting it see the release
        // would read the pair as a click on nothing and clear the selection the link just made.
        if (event->button() == Qt::LeftButton)
        {
            mLinkClick = false;
        }

        event->accept();
        return;
    }

    if (mResizeHandle != eHandle::None)
    {
        if (event->button() == Qt::LeftButton)
        {
            commitResize();
            mResizeHandle = eHandle::None;
        }

        event->accept();
        return;
    }

    SMCanvasItem::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton)
    {
        SMScene* canvas = getCanvas();
        if (canvas != nullptr)
        {
            canvas->commitSelectionMove(translate("Move selection"));
        }
    }
}

void SMStateItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Markers have no title and body split, so a double-click always renames. On a normal
        // state the header opens the rename and the body descends into the submachine.
        const bool onTitle = isMarker() || (event->pos().y() <= NESMDesign::StateHeaderHeight);
        if (onTitle)
        {
            if (isRenameActive() == false)
            {
                startInlineRename();
            }
        }
        else if (SMScene* canvas = getCanvas())
        {
            canvas->requestSubstate(getElementId());
        }

        event->accept();
        return;
    }

    SMCanvasItem::mouseDoubleClickEvent(event);
}

QVariant SMStateItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        prepareGeometryChange();
        // A selected box raises above the inactive transition lines so it can cover them, while an
        // inactive one stays below every edge. A selected edge still wins, to keep its handles.
        setZValue(value.toBool() ? 2.0 : 0.0);
        if ((value.toBool() == false) && isRenameActive())
        {
            closeRenameEditor();
        }
    }
    else if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        // Live gluing: connected transition endpoints follow the moving box.
        notifyEdgesOfGeometry();
    }

    return SMCanvasItem::itemChange(change, value);
}

void SMStateItem::applyResizeDrag(const QPointF& scenePos)
{
    SMScene* canvas = getCanvas();
    if (canvas == nullptr)
    {
        return;
    }

    QPointF point{ scenePos };
    if (canvas->isSnapToGrid())
    {
        point = NESMDesign::snapPoint(point, canvas->getGridSize());
    }

    QRectF rect{ mResizeStart };
    const bool left   = (mResizeHandle == eHandle::TopLeft) || (mResizeHandle == eHandle::Left) || (mResizeHandle == eHandle::BottomLeft);
    const bool right  = (mResizeHandle == eHandle::TopRight) || (mResizeHandle == eHandle::Right) || (mResizeHandle == eHandle::BottomRight);
    const bool top    = (mResizeHandle == eHandle::TopLeft) || (mResizeHandle == eHandle::Top) || (mResizeHandle == eHandle::TopRight);
    const bool bottom = (mResizeHandle == eHandle::BottomLeft) || (mResizeHandle == eHandle::Bottom) || (mResizeHandle == eHandle::BottomRight);

    // Markers (Start / Final) are compact pills and may be resized smaller than a normal box.
    const double minW = (isMarker() ? NESMDesign::MarkerStateMinWidth : NESMDesign::StateMinWidth);
    const double minH = (isMarker() ? NESMDesign::MarkerStateMinHeight : NESMDesign::StateMinHeight);

    if (left)
    {
        rect.setLeft(std::min(point.x(), rect.right() - minW));
    }
    else if (right)
    {
        rect.setRight(std::max(point.x(), rect.left() + minW));
    }

    if (top)
    {
        rect.setTop(std::min(point.y(), rect.bottom() - minH));
    }
    else if (bottom)
    {
        rect.setBottom(std::max(point.y(), rect.top() + minH));
    }

    prepareGeometryChange();
    setPos(rect.topLeft());
    mSize = rect.size();
    update();
    notifyEdgesOfGeometry();
}

void SMStateItem::commitResize()
{
    SMScene* canvas = getCanvas();
    const QRectF geometry = getBoxGeometry();
    if ((canvas == nullptr) || (geometry == mResizeStart))
    {
        return;
    }

    StateMachineModel& model = canvas->getModel();
    const QString text = translate("Resize state");
    // A resized border carries the anchors of the transitions that end on it; they are written
    // back in the same undo step (see SMScene::commitSelectionMove).
    const QList<SMEdgeItem*> movedEdges{ canvas->driftedEdgeItems() };
    const uint32_t gesture = SMMoveNodeCommand::takeNextGesture();
    QUndoCommand*  parent  = movedEdges.isEmpty()
                             ? nullptr : new SMCompositeCommand(model.getData(), model.getNotifier(), text);

    QUndoCommand* resize = new SMMoveNodeCommand(  model.getData(), model.getNotifier()
                                                 , getElementId(), gesture
                                                 , geometry.x(), geometry.y(), geometry.width(), geometry.height()
                                                 , text, parent);
    for (SMEdgeItem* edge : movedEdges)
    {
        new SMSetEdgeGeometryCommand(  model.getData(), model.getNotifier()
                                     , edge->getElementId(), gesture, edge->buildGeometry(), text, parent);
    }

    model.getUndoStack().push(parent != nullptr ? parent : resize);
}

void SMStateItem::toggleExpanded()
{
    SMScene* canvas = getCanvas();
    if (canvas != nullptr)
    {
        StateMachineModel& model = canvas->getModel();
        const QString text = (mExpanded ? translate("Collapse state") : translate("Expand state"));
        model.getUndoStack().push(new SMSetNodeExpandedCommand(  model.getData(), model.getNotifier()
                                                               , getElementId(), mExpanded == false, text));
    }
}

QString SMStateItem::validateName(const QString& name) const
{
    if (StateMachineData::isValidIdentifier(name) == false)
    {
        return translate("Not a valid identifier: use letters, digits, and underscores; do not start with a digit.");
    }

    SMScene* canvas = getCanvas();
    if (canvas != nullptr)
    {
        const SMStateEntry* other = canvas->getModel().getData().findState(name);
        if ((other != nullptr) && (other->getId() != getElementId()))
        {
            return translate("A state with this name already exists.");
        }
    }

    return QString();
}

void SMStateItem::startInlineRename()
{
    if (mRenameProxy != nullptr)
    {
        return;
    }

    const SMStateEntry* state = getState();
    if (state == nullptr)
    {
        return;
    }

    // Seed from the displayed name, not state->getName(): the Properties panel mirrors an
    // in-progress rename onto the canvas before it is committed, so the model can still be stale.
    RenameEdit* edit = new RenameEdit(mName);
    edit->mValidate  = [this](const QString& name) { return validateName(name); };
    edit->mCancel    = [this]()
    {
        // Esc restores the committed name everywhere. Restore from the model, not mName, which
        // holds the abandoned typing preview.
        if (SMScene* canvas = getCanvas())
        {
            const SMStateEntry* state = getState();
            canvas->getModel().publishStateNamePreview(getElementId(), state != nullptr ? state->getName() : mName);
        }

        closeRenameEditor();
    };
    edit->setFrame(true);
    edit->setMaxLength(StateMachineData::MAX_IDENTIFIER_LENGTH);
    // State names must be enum-friendly identifiers: reject spaces and other invalid symbols
    // as the user types (a leading digit and an empty field remain intermediate, not typed).
    edit->setValidator(new QRegularExpressionValidator(QRegularExpression(StateMachineData::identifierPattern()), edit));

    mRenameProxy = new QGraphicsProxyWidget(this);
    mRenameProxy->setWidget(edit);
    const double editH = NESMDesign::StateHeaderHeight - 2.0;
    const double editY = (isMarker() ? (visibleHeight() - editH) / 2.0 : 1.0);
    mRenameProxy->setGeometry(QRectF(2.0, editY, mSize.width() - 4.0, editH));
    mRenameProxy->setZValue(1.0);

    QObject::connect(edit, &QLineEdit::textChanged, edit, [this, edit](const QString& text)
        {
            if (SMScene* canvas = getCanvas())
            {
                canvas->getModel().publishStateNamePreview(getElementId(), text);
            }

            const QString reason = validateName(text.trimmed());
            edit->setStyleSheet(reason.isEmpty() ? QString() : QStringLiteral("QLineEdit { border: 1px solid #D04040; }"));
            edit->setToolTip(reason);
        });

    QObject::connect(edit, &QLineEdit::editingFinished, edit, [this]() { finishRename(); });

    edit->selectAll();
    edit->setFocus();
}

void SMStateItem::setNamePreview(const QString& name)
{
    if (mName != name)
    {
        mName = name;
        update();
    }

    // While the in-place editor is open it covers the painted header, so the preview is mirrored
    // into it as well. Skip that when the editor has focus, to keep the caret and break the loop.
    if (mRenameProxy != nullptr)
    {
        QLineEdit* edit = qobject_cast<QLineEdit*>(mRenameProxy->widget());
        if ((edit != nullptr) && (edit->hasFocus() == false) && (edit->text() != name))
        {
            const QSignalBlocker block(edit);
            edit->setText(name);
        }
    }
}

void SMStateItem::startNoteEdit()
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

    // The editor is shown on top of the state box so the note text is edited over its owner
    const QRectF area{ 2.0, 2.0, mSize.width() - 4.0, std::max(visibleHeight() - 4.0, 40.0) };
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

void SMStateItem::commitRename(const QString& name)
{
    SMScene* canvas = getCanvas();
    if (canvas == nullptr)
    {
        return;
    }

    // Compare against the model's committed name, not mName: mName already holds the live typing
    // preview by commit time, so guarding on it turned every rename into a no-op.
    const SMStateEntry* state = getState();
    const QString committed = (state != nullptr ? state->getName() : QString());
    if (name == committed)
    {
        return;
    }

    StateMachineModel& model = canvas->getModel();
    model.getUndoStack().push(new SMRenameStateCommand(  model.getData(), model.getNotifier()
                                                       , getElementId(), name
                                                       , translate("Rename state")));
}

void SMStateItem::finishRename(bool immediate /*= false*/)
{
    if (mRenameProxy == nullptr)
    {
        return;
    }

    const QLineEdit* edit = qobject_cast<const QLineEdit*>(mRenameProxy->widget());
    const QString name = (edit != nullptr ? edit->text().trimmed() : QString());
    const bool valid = (edit != nullptr) && validateName(name).isEmpty();
    closeRenameEditor(immediate);
    if (valid)
    {
        commitRename(name);
    }
    else
    {
        // Reject: restore the committed name (from the model, not mName which holds the
        // rejected typing preview) on both the canvas box and the Properties panel.
        if (SMScene* canvas = getCanvas())
        {
            const SMStateEntry* state = getState();
            canvas->getModel().publishStateNamePreview(getElementId(), state != nullptr ? state->getName() : mName);
        }
    }
}

void SMStateItem::finishInlineEdit()
{
    // Destroy the proxies now rather than deferring: this runs from a tool switch, and a proxy that
    // outlives it keeps its I-beam on the viewport past the point the tool sets its cursor.
    finishRename(true);
    mNoteEditor.commit();
}

void SMStateItem::closeRenameEditor(bool immediate /*= false*/)
{
    if ((mRenameProxy == nullptr) || mClosingRename)
    {
        return;
    }

    mClosingRename = true;
    QGraphicsProxyWidget* proxy = mRenameProxy;
    mRenameProxy = nullptr;
    if (immediate)
    {
        delete proxy;
    }
    else
    {
        proxy->deleteLater();
    }

    mClosingRename = false;
    setFocus();
}
