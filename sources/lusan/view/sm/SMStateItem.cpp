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
#include "lusan/model/sm/SMOperationSummary.hpp"
#include "lusan/model/sm/SMOperationValidation.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
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
#include <QPolygonF>
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

    /**
     * \brief   Which of the two exit glyphs a generic exit operation draws. Both are implemented
     *          (\c Exit is `<-|`, \c ExitAlt is `|<-`) so the pair can be compared on a real
     *          diagram; change this one line to adopt the other. `<-|` is the current choice: it
     *          mirrors the entry glyph `->|` exactly -- the same bar, the arrow reversed -- so the
     *          two read as a pair rather than as two unrelated marks.
     **/
    constexpr SMStateItem::eRowIcon ExitRowIcon { SMStateItem::eRowIcon::Exit };

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

    //!< Drops the leading verb the shared one-line summary prepends for an event or a timer
    //!< (`send `, `start `, `stop `). On a state body every such row already carries its own kind
    //!< glyph -- the lightning bolt for an event, the play/stop clock for a timer -- so the word is
    //!< redundant beside the icon. The edge summary keeps the verbs (it has no per-op icons), so the
    //!< strip lives here, in the body renderer, not in SMOperationSummary.
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

    //!< Draws one behavior-row glyph centered in the given rectangle.
    void drawRowIcon(QPainter* painter, const QRectF& rect, SMStateItem::eRowIcon icon, const QColor& color)
    {
        QPen pen{ color, 1.2 };
        pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        const double midY = rect.center().y();
        switch (icon)
        {
        case SMStateItem::eRowIcon::Entry:
        {
            // `->|` -- an arrow running rightwards INTO the bar that stands for the state: control
            // arrives here. The bar sits on the right so the arrow reads left-to-right, the same
            // direction the eye travels along the row text beside it.
            const double barX = rect.right() - 1.0;
            const double tipX = barX - 2.0;
            painter->drawLine(QPointF(rect.left() + 1.0, midY), QPointF(tipX, midY));
            painter->drawLine(QPointF(tipX - 3.5, midY - 3.0), QPointF(tipX, midY));
            painter->drawLine(QPointF(tipX - 3.5, midY + 3.0), QPointF(tipX, midY));
            painter->drawLine(QPointF(barX, midY - 4.5), QPointF(barX, midY + 4.5));
            break;
        }

        case SMStateItem::eRowIcon::Exit:
        {
            // `<-|` -- the mirror of the entry glyph: the same bar on the right, but the arrow runs
            // AWAY from it, leftwards. Entry and exit therefore differ only in arrow direction,
            // which is what makes the pair readable at a glance in a small box.
            const double barX = rect.right() - 1.0;
            const double tipX = rect.left() + 1.0;
            painter->drawLine(QPointF(barX - 2.0, midY), QPointF(tipX, midY));
            painter->drawLine(QPointF(tipX + 3.5, midY - 3.0), QPointF(tipX, midY));
            painter->drawLine(QPointF(tipX + 3.5, midY + 3.0), QPointF(tipX, midY));
            painter->drawLine(QPointF(barX, midY - 4.5), QPointF(barX, midY + 4.5));
            break;
        }

        case SMStateItem::eRowIcon::ExitAlt:
        {
            // `|<-` -- the alternative exit glyph: the bar on the LEFT with the arrow pointing back
            // into it. Offered beside `<-|` so the two can be compared on a real diagram; flip
            // ExitRowIcon (below) to adopt it.
            const double barX = rect.left() + 1.0;
            const double tipX = barX + 2.0;
            painter->drawLine(QPointF(barX, midY - 4.5), QPointF(barX, midY + 4.5));
            painter->drawLine(QPointF(tipX, midY), QPointF(rect.right() - 1.0, midY));
            painter->drawLine(QPointF(tipX + 3.5, midY - 3.0), QPointF(tipX, midY));
            painter->drawLine(QPointF(tipX + 3.5, midY + 3.0), QPointF(tipX, midY));
            break;
        }

        case SMStateItem::eRowIcon::TimerStart:
        {
            // Clock face with a play triangle (start).
            const QRectF face{ rect.center().x() - 4.5, midY - 4.5, 9.0, 9.0 };
            painter->drawEllipse(face);
            painter->setBrush(color);
            painter->drawPolygon(QPolygonF({ QPointF(face.center().x() - 1.5, midY - 2.2)
                                           , QPointF(face.center().x() + 2.5, midY)
                                           , QPointF(face.center().x() - 1.5, midY + 2.2) }));
            painter->setBrush(Qt::NoBrush);
            break;
        }

        case SMStateItem::eRowIcon::TimerStop:
        {
            // Clock face with a stop square.
            const QRectF face{ rect.center().x() - 4.5, midY - 4.5, 9.0, 9.0 };
            painter->drawEllipse(face);
            painter->setBrush(color);
            painter->drawRect(QRectF(face.center().x() - 1.8, midY - 1.8, 3.6, 3.6));
            painter->setBrush(Qt::NoBrush);
            break;
        }

        case SMStateItem::eRowIcon::Event:
        {
            // A filled lightning bolt -- the same glyph family as the Events page and the toolbar so
            // "event" reads the same way on every surface. Normalized coordinates (0..1 in the icon
            // rect) trace the bolt, then map onto the row-icon rect.
            const double x = rect.left();
            const double y = rect.top();
            const double w = rect.width();
            const double h = rect.height();
            QPainterPath bolt;
            bolt.moveTo(x + 0.62 * w, y + 0.05 * h);
            bolt.lineTo(x + 0.25 * w, y + 0.55 * h);
            bolt.lineTo(x + 0.48 * w, y + 0.55 * h);
            bolt.lineTo(x + 0.40 * w, y + 0.95 * h);
            bolt.lineTo(x + 0.78 * w, y + 0.42 * h);
            bolt.lineTo(x + 0.53 * w, y + 0.42 * h);
            bolt.closeSubpath();
            painter->setBrush(color);
            painter->drawPath(bolt);
            painter->setBrush(Qt::NoBrush);
            break;
        }

        case SMStateItem::eRowIcon::Internal:
        default:
        {
            // Self-loop: an open circle with an arrowhead at the gap.
            const QRectF loop{ rect.center().x() - 4.0, midY - 4.0, 8.0, 8.0 };
            painter->drawArc(loop, 30 * 16, 300 * 16);
            const QPointF tip{ loop.right(), midY + 2.0 };
            painter->drawLine(tip, tip + QPointF(-3.5, 1.0));
            painter->drawLine(tip, tip + QPointF(-0.5, -3.5));
            break;
        }
        }
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
        // Cover the full resize-handle band with a plain rectangle so every handle -- the
        // corner ones too -- is hit-testable. A rounded shape leaves the corner handles of a
        // compact marker pill in the rounded-away region, which made the Start / Final state
        // impossible to grab and resize.
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
    // The default marker pill is only 4 grid cells wide (issue #514); a tight padding keeps
    // "Start" / "Final" un-elided. The vertically centered text never reaches the rounded
    // corners, so the inset can be much smaller than the corner radius.
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
        const QRectF badge{ right - 14.0, (headerH - 14.0) / 2.0, 14.0, 14.0 };
        painter->setPen(QPen(textColor, 1.0));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(badge);
        painter->setFont(badgeFont);
        const char* mark = (mHistory == SMStateEntry::eHistory::Deep ? "H*" : "H");
        painter->drawText(badge, Qt::AlignCenter, QString::fromLatin1(mark));
        right = badge.left() - 4.0;
    }

    if (mComposite || mImported)
    {
        // Composite: nested boxes; imported: a box with an inbound arrow.
        const QRectF badge{ right - 14.0, (headerH - 12.0) / 2.0, 12.0, 12.0 };
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

void SMStateItem::paintBodyRows(QPainter* painter, const QRectF& box, const QColor& bodyColor)
{
    if (hasBodyContent() == false)
    {
        return;
    }

    const double rowH    = NESMDesign::StateRowHeight;
    const double padding = NESMDesign::StatePadding;
    const QColor color   = NESMDesign::contrastTextColor(bodyColor);

    QFont rowFont = painter->font();
    rowFont.setPointSizeF(rowFont.pointSizeF() * 0.85);
    const QFontMetrics metrics{ rowFont };

    // The body is read as three bands, in execution order: what runs on the way IN sits at the top,
    // what runs while in the state sits in the middle, and what runs on the way OUT is anchored to
    // the bottom edge. Stacking all three from the top (as this did) put the exit actions directly
    // under the entry ones, which reads as one undifferentiated list.
    const double top    = NESMDesign::StateHeaderHeight + 2.0;
    const double bottom = box.height() - 2.0;
    const int    rowSlots = static_cast<int>((bottom - top) / rowH);
    if (rowSlots <= 0)
    {
        return;
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

    const auto drawRow = [&](const QString& text, eRowIcon icon, double rowY, bool ellipsis, bool continues)
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
        const QRectF textRect{ padding + 16.0, rowY, box.width() - padding - (padding + 16.0) - cueW, rowH };

        drawRowIcon(painter, QRectF(padding, rowY + 2.0, 12.0, rowH - 4.0), icon, color);
        painter->setFont(rowFont);
        painter->setPen(color);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter
                         , metrics.elidedText(text, Qt::ElideRight, static_cast<int>(textRect.width())));
        if (continues)
        {
            painter->drawText(QRectF(box.width() - padding - cueW, rowY, cueW, rowH)
                             , Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("\\"));
        }
    };

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
    for (const BodyRow& row : mRows)
    {
        const int zone = static_cast<int>(row.zone);
        if (seen[zone] >= shown[zone])
        {
            continue;       // this band is full; its remaining rows are covered by the `...` below
        }

        // The last slot of a truncated band says so, rather than dropping rows silently.
        const bool truncated = ((seen[zone] + 1) == shown[zone]) && (shown[zone] < counts[zone]);
        drawRow(row.text, row.icon, bandY[zone] + (seen[zone] * rowH), truncated, row.continues && (truncated == false));
        ++seen[zone];
    }
}

void SMStateItem::paintMiniature(QPainter* painter, const QRectF& box, const QColor& bodyColor)
{
    const double maxW = NESMDesign::MiniatureMaxWidth;
    const double maxH = NESMDesign::MiniatureMaxHeight;
    const double pad  = NESMDesign::MiniaturePadding;
    if (mMiniature.isEmpty() || (box.height() < NESMDesign::StateHeaderHeight + maxH + 2.0 * pad))
    {
        return;
    }

    QRectF bounds = mMiniature.first();
    for (const QRectF& rect : mMiniature)
    {
        bounds = bounds.united(rect);
    }

    if ((bounds.width() <= 0.0) || (bounds.height() <= 0.0))
    {
        return;
    }

    const double scale = std::min(maxW / bounds.width(), maxH / bounds.height());
    const QRectF avail{ box.width() - pad - maxW, box.height() - pad - maxH, maxW, maxH };
    const QPointF origin{ avail.center().x() - bounds.width() * scale / 2.0
                        , avail.center().y() - bounds.height() * scale / 2.0 };

    painter->save();
    painter->setOpacity(painter->opacity() * 0.55);
    painter->setPen(QPen(NESMDesign::contrastTextColor(bodyColor), 1.0));
    painter->setBrush(Qt::NoBrush);
    for (const QRectF& rect : mMiniature)
    {
        const QRectF scaled{ origin.x() + (rect.x() - bounds.x()) * scale
                           , origin.y() + (rect.y() - bounds.y()) * scale
                           , std::max(rect.width() * scale, 2.0)
                           , std::max(rect.height() * scale, 2.0) };
        painter->drawRoundedRect(scaled, 1.5, 1.5);
    }

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
    // A submachine counts as composite only when it is "real" -- it owns at least one Normal state.
    // A just-entered-then-left submachine that still holds only its Start marker is not real (it is
    // never persisted), so the box must not advertise a substate it does not really have.
    mComposite = state->hasNestedStates() && state->getNestedStates()->hasRealState();
    mImported  = state->isImportedSubmachine();
    mHasNote   = (data.getLayout().findNoteByOwner(getElementId()) != nullptr);

    // An entry/exit action whose arguments are not fully mapped warns in the header, so a method
    // edit that breaks a mapping is visible on the canvas without opening the Properties panel.
    mActionSeverity = -1;
    const SMOperationValidation::eSeverity opSeverity = SMOperationValidation::stateSeverity(data, getElementId());
    if (opSeverity != SMOperationValidation::eSeverity::Ok)
    {
        mActionSeverity = static_cast<int>((opSeverity == SMOperationValidation::eSeverity::Error)
                                           ? NEGuardStyle::eSeverity::Err
                                           : NEGuardStyle::eSeverity::Warn);
    }

    rebuildRows(*state);

    mMiniature.clear();
    if (mComposite)
    {
        for (const SMStateEntry* child : state->getNestedStates()->getElements())
        {
            const SMLayoutNode* childNode = data.getLayout().findNode(child->getId());
            if (childNode != nullptr)
            {
                mMiniature.append(QRectF(childNode->x, childNode->y, childNode->width, childNode->height));
            }
        }
    }

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
        return withoutRowVerb(op, summary);
    };

    // The zone glyph the FIRST row of an Enter/Do/Exit group carries, so every group opens with the
    // same mark (`->|` enter, `<-|` exit, a self-loop for Do), and the reader sees at a glance which
    // activity band a row belongs to.
    const auto zoneGlyph = [](eRowZone zone) -> eRowIcon
    {
        switch (zone)
        {
        case eRowZone::Enter:   return eRowIcon::Entry;
        case eRowZone::Exit:    return ExitRowIcon;
        default:                return eRowIcon::Internal;
        }
    };

    // One Enter/Do/Exit group, ordered action -> event -> timer(s): the actions each on a row, the
    // events each on a row, then ALL timers combined on one row (start and stop together). The first
    // emitted row carries the zone glyph; every row but the last flags a ` \` continuation cue so the
    // group reads as one block. An absent kind emits nothing (no empty row, no dangling backslash).
    const auto appendGroup = [&](const SMOperationList& ops, eRowZone zone)
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
        for (const SMOperationBase* op : std::as_const(actions))
        {
            group.append(BodyRow{ zoneGlyph(zone), rowText(*op), zone, false, false });
        }
        for (const SMOperationBase* op : std::as_const(events))
        {
            group.append(BodyRow{ eRowIcon::Event, rowText(*op), zone, false, false });
        }
        if (timers.isEmpty() == false)
        {
            // Every timer of the group on one line (`start A | stop B`); the icon follows the first
            // timer so a start-only group shows the play clock and a stop-only group the square clock.
            QStringList parts;
            for (const SMOperationBase* op : std::as_const(timers))
            {
                parts.append(rowText(*op));
            }

            const eRowIcon tIcon = (timers.first()->getOperationType() == SMOperationBase::eOperation::TimerStop)
                                    ? eRowIcon::TimerStop : eRowIcon::TimerStart;
            group.append(BodyRow{ tIcon, parts.join(QStringLiteral(" | ")), zone, false, false });
        }

        for (int i = 0; i < group.size(); ++i)
        {
            if (i == 0)
            {
                group[i].icon = zoneGlyph(zone);    // the group opens with `->|` / `<-|` / loop
                group[i].firstInGroup = true;
            }

            group[i].continues = (i < (group.size() - 1));
            mRows.append(group.at(i));
        }
    };

    // Zone by zone -- Enter at the top, everything that runs WHILE in the state in the middle, Exit
    // anchored to the bottom -- so the box reads in the order the state actually executes.
    appendGroup(state.getEntryList(), eRowZone::Enter);
    appendGroup(state.getDoList(), eRowZone::Middle);

    // External transitions show their operations on the edge (below the line); internal ones have no
    // edge, so each reads here as its own group under an "on <stimulus>" header row.
    for (const SMTransitionEntry* transition : state.getTransitions().getElements())
    {
        if (transition->isExternal() == false)
        {
            const QString stim = (data != nullptr) ? SMOperationSummary::stimulusSignature(*data, *transition) : transition->getStimulus();
            mRows.append(BodyRow{ eRowIcon::Internal, QStringLiteral("on ") + stim, eRowZone::Middle, false, false });
            appendGroup(transition->getOperations(), eRowZone::Middle);
        }
    }

    appendGroup(state.getExitList(), eRowZone::Exit);
}

void SMStateItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
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
    unsetCursor();
    SMCanvasItem::hoverLeaveEvent(event);
}

void SMStateItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
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
        // Start / Final markers have no title/body split and no submachine: double-click always
        // edits the name. A normal state splits on the header band: the title (header) opens the
        // in-place rename; the body descends into the state's submachine, which the owning page
        // creates on the fly for a plain normal state (the same path as the Enter Submachine
        // action). This is why a double-click on the body must NOT fall back to rename.
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
        // A selected (active) box raises above the inactive transition lines (z = 1) so it can
        // cover them, while an inactive box stays at z = 0, below every edge; a selected edge
        // still wins at z = 3 so its handles stay grabbable (see SMEdgeItem).
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
    model.getUndoStack().push(new SMMoveNodeCommand(  model.getData(), model.getNotifier()
                                                    , getElementId(), SMMoveNodeCommand::takeNextGesture()
                                                    , geometry.x(), geometry.y(), geometry.width(), geometry.height()
                                                    , translate("Resize state")));
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

    // Seed from the currently displayed name (mName), not state->getName(): the Properties panel
    // mirrors an in-progress rename onto the canvas via a name preview that is not committed to
    // the model until its field loses focus, so state->getName() can still hold the old value.
    RenameEdit* edit = new RenameEdit(mName);
    edit->mValidate  = [this](const QString& name) { return validateName(name); };
    edit->mCancel    = [this]()
    {
        // Esc discards the in-progress name and restores the committed one everywhere (canvas box
        // and the Properties panel field) -- restore from the model, not mName, which holds the
        // abandoned typing preview (Bug 3, Esc case).
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

    QObject::connect(edit, &QLineEdit::editingFinished, edit, [this, edit]()
        {
            if (mRenameProxy == nullptr)
            {
                return;
            }

            const QString name = edit->text().trimmed();
            const bool valid = validateName(name).isEmpty();
            closeRenameEditor();
            if (valid == false)
            {
                // Reject: restore the committed name (from the model, not mName which holds the
                // rejected typing preview) on both the canvas box and the Properties panel.
                if (SMScene* canvas = getCanvas())
                {
                    const SMStateEntry* state = getState();
                    canvas->getModel().publishStateNamePreview(getElementId(), state != nullptr ? state->getName() : mName);
                }
            }

            if (valid)
            {
                commitRename(name);
            }
        });

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

    // When the state is in edit mode, the in-place editor widget covers the painted header, so
    // updating mName alone is invisible: the on-canvas title is whatever the RenameEdit shows.
    // Mirror the preview into the editor too, so typing the name in the Properties panel updates
    // the on-canvas editor in real time (Bug 1). Skip when the editor is the source of the change
    // (it has focus) to avoid clobbering the caret and to break the preview feedback loop.
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
    // (spec: "display the note on top of the State"); focus-out commits and collapses to the
    // corner badge again.
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

    // Compare against the model's committed name, NOT mName: mName tracks the live typing preview
    // (see setNamePreview), so it already equals the typed text by commit time. Guarding on mName
    // made every rename a no-op, so the new name was never written and later reappeared as the old
    // one (Bug 3). getState() is the single source of truth for what is actually stored.
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

void SMStateItem::closeRenameEditor()
{
    if ((mRenameProxy == nullptr) || mClosingRename)
    {
        return;
    }

    mClosingRename = true;
    QGraphicsProxyWidget* proxy = mRenameProxy;
    mRenameProxy = nullptr;
    proxy->deleteLater();
    mClosingRename = false;
    setFocus();
}
