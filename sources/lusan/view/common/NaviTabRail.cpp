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
 *  \file        lusan/view/common/NaviTabRail.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The vertical selector strip of the navigation panel.
 *
 ************************************************************************/

#include "lusan/view/common/NaviTabRail.hpp"

#include <QAction>
#include <QContextMenuEvent>
#include <QFontMetrics>
#include <QHash>
#include <QIcon>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QResizeEvent>
#include <QStringList>

namespace
{
    constexpr int   RAIL_WIDTH_COMPACT  { 40 };     //!< Rail width while only icons are drawn.
    constexpr int   RAIL_WIDTH_MIN      { 56 };     //!< Narrowest labelled rail.
    constexpr int   RAIL_WIDTH_MAX      { 72 };     //!< Widest labelled rail.
    constexpr int   CAPTION_LINES       { 2 };      //!< Most lines one caption may take.
    constexpr int   ICON_EXTENT         { 20 };     //!< Edge of the icon square, as on the main toolbar.
    constexpr int   PADDING_TOP         { 6 };      //!< Free space above the first item.
    constexpr int   ITEM_SPACING        { 2 };      //!< Free space between two items.
    constexpr int   MARKER_WIDTH        { 3 };      //!< Width of the current item marker.
    constexpr int   MARKER_INSET        { 7 };      //!< Free space above and below the marker.
    constexpr int   BADGE_RADIUS        { 4 };      //!< Radius of the badge circle.
    constexpr int   CONTENT_ROOM        { 140 };    //!< Room the content needs before captions appear.
    constexpr int   OVERFLOW_MARK       { -2 };     //!< The value indexAt returns over the overflow button.
    constexpr int   CHEVRON_MARK        { -3 };     //!< The value indexAt returns over the collapse button.
    constexpr int   CHEVRON_HEIGHT      { 18 };     //!< Height of the collapse button at the rail foot.
    constexpr int   CHEVRON_BOTTOM      { 3 };      //!< Free space under the collapse button.

    //!< Breaks a caption on its spaces into no more than CAPTION_LINES lines.
    QStringList splitCaption(const QString& label)
    {
        QStringList words = label.split(QLatin1Char(' '), Qt::SplitBehaviorFlags::SkipEmptyParts);
        if (words.size() <= CAPTION_LINES)
            return words;

        QStringList result;
        while (result.size() < (CAPTION_LINES - 1))
        {
            result.append(words.takeFirst());
        }

        result.append(words.join(QLatin1Char(' ')));
        return result;
    }

    //!< Renders a monochrome icon and fills its shape with one color.
    QPixmap tintedPixmap(const QString& path, int extent, const QColor& color, qreal ratio)
    {
        static QHash<QString, QPixmap> _cache;

        const QString key = path + QLatin1Char('|') + QString::number(extent) + QLatin1Char('|')
                          + color.name(QColor::HexArgb) + QLatin1Char('|') + QString::number(ratio, 'f', 2);
        const auto found = _cache.constFind(key);
        if (found != _cache.constEnd())
            return found.value();

        QPixmap pixmap = QIcon(path).pixmap(QSize(extent, extent), ratio);
        if (pixmap.isNull() == false)
        {
            QPainter painter(&pixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pixmap.rect(), color);
        }

        _cache.insert(key, pixmap);
        return pixmap;
    }

    //!< Returns the color with the alpha replaced.
    QColor withAlpha(const QColor& color, int alpha)
    {
        QColor result(color);
        result.setAlpha(alpha);
        return result;
    }

    //!< Returns the strip background: the window color pulled a few percent toward the text color.
    QColor railBackground(const QPalette& colors)
    {
        const QColor window = colors.color(QPalette::ColorRole::Window);
        const QColor text   = colors.color(QPalette::ColorRole::WindowText);
        constexpr qreal mix { 0.06 };
        return QColor::fromRgbF(  (window.redF()   * (1.0 - mix)) + (text.redF()   * mix)
                                , (window.greenF() * (1.0 - mix)) + (text.greenF() * mix)
                                , (window.blueF()  * (1.0 - mix)) + (text.blueF()  * mix));
    }
}

NaviTabRail::NaviTabRail(QWidget* parent)
    : QWidget           (parent)

    , mItems            ( )
    , mCurrent          (-1)
    , mHovered          (-1)
    , mFocused          (-1)
    , mSide             (NaviTabRail::eSide::West)
    , mLabelsPreferred  (false)
    , mLabels           (false)
    , mCaptionLines     (1)
    , mIconExtent       (ICON_EXTENT)
    , mItemHeight       (RAIL_WIDTH_COMPACT)
    , mRailWidth        (RAIL_WIDTH_COMPACT)
    , mLabelledWidth    (RAIL_WIDTH_MIN)
    , mOverflowRect     ( )
    , mOverflowHovered  (false)
    , mChevronRect      ( )
    , mChevronHovered   (false)
    , mCollapsed        (false)
{
    setObjectName(QStringLiteral("naviTabRail"));
    setMouseTracking(true);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
    setAttribute(Qt::WidgetAttribute::WA_OpaquePaintEvent, false);
    setAccessibleName(tr("Navigator selector"));
    recalcMetrics();
}

void NaviTabRail::addItem(int id, const QString& iconPath, const QString& label, const QString& hint, const QString& shortcut)
{
    const int index = indexOf(id);
    if (index >= 0)
    {
        sRailItem& item = mItems[index];
        item.iconPath   = iconPath;
        item.label      = label;
        item.hint       = hint;
        item.shortcut   = shortcut;
        item.visible    = true;
    }
    else
    {
        sRailItem item{ id, iconPath, label, hint, shortcut, eBadge::None, true, QRect() };
        int place = 0;
        while ((place < mItems.size()) && (mItems.at(place).id < id))
        {
            ++place;
        }

        mItems.insert(place, item);
        if ((mCurrent >= place) && (mCurrent >= 0))
            ++mCurrent;
        if ((mFocused >= place) && (mFocused >= 0))
            ++mFocused;
    }

    mHovered = -1;
    recalcMetrics();
    relayout();
    update();
}

void NaviTabRail::removeItem(int id)
{
    const int index = indexOf(id);
    if (index < 0)
        return;

    mItems.removeAt(index);
    if (mCurrent == index)
        mCurrent = -1;
    else if (mCurrent > index)
        --mCurrent;

    if (mFocused > index)
        --mFocused;
    if (mFocused >= mItems.size())
        mFocused = mItems.size() - 1;

    mHovered = -1;
    recalcMetrics();
    relayout();
    update();
}

bool NaviTabRail::hasItem(int id) const
{
    return (indexOf(id) >= 0);
}

void NaviTabRail::setItemVisible(int id, bool visible)
{
    const int index = indexOf(id);
    if ((index < 0) || (mItems.at(index).visible == visible))
        return;

    mItems[index].visible = visible;
    if ((visible == false) && (mCurrent == index))
        mCurrent = -1;

    mHovered = -1;
    relayout();
    update();
}

bool NaviTabRail::isItemVisible(int id) const
{
    const int index = indexOf(id);
    return ((index >= 0) && mItems.at(index).visible);
}

void NaviTabRail::setItemBadge(int id, NaviTabRail::eBadge badge)
{
    const int index = indexOf(id);
    if ((index < 0) || (mItems.at(index).badge == badge))
        return;

    mItems[index].badge = badge;
    update();
}

void NaviTabRail::setCurrentItem(int id)
{
    const int index = indexOf(id);
    if ((index < 0) || (mItems.at(index).visible == false) || (index == mCurrent))
        return;

    mCurrent = index;
    mFocused = index;
    relayout();
    update();
    emit signalItemActivated(id);
}

int NaviTabRail::firstVisibleItem(void) const
{
    for (const sRailItem& item : mItems)
    {
        if (item.visible)
            return item.id;
    }

    return -1;
}

int NaviTabRail::visibleCount(void) const
{
    int count{ 0 };
    for (const sRailItem& item : mItems)
    {
        if (item.visible)
            ++count;
    }

    return count;
}

void NaviTabRail::setSide(NaviTabRail::eSide side)
{
    if (mSide == side)
        return;

    mSide = side;
    update();
}

void NaviTabRail::setLabelsPreferred(bool labels)
{
    if (mLabelsPreferred == labels)
        return;

    mLabelsPreferred = labels;
    applyPanelWidth(parentWidget() != nullptr ? parentWidget()->width() : width() + CONTENT_ROOM);
}

void NaviTabRail::applyPanelWidth(int panelWidth)
{
    const bool labels = mLabelsPreferred && (panelWidth >= (mLabelledWidth + CONTENT_ROOM));
    if (labels == mLabels)
        return;

    mLabels = labels;
    recalcMetrics();
    relayout();
    update();
}

int NaviTabRail::indexOf(int id) const
{
    for (int i = 0; i < mItems.size(); ++i)
    {
        if (mItems.at(i).id == id)
            return i;
    }

    return -1;
}

int NaviTabRail::indexAt(const QPoint& pos) const
{
    if (mChevronRect.isNull() == false && mChevronRect.contains(pos))
        return CHEVRON_MARK;

    if (mOverflowRect.isNull() == false && mOverflowRect.contains(pos))
        return OVERFLOW_MARK;

    for (int i = 0; i < mItems.size(); ++i)
    {
        const sRailItem& item = mItems.at(i);
        if (item.visible && (item.rect.isNull() == false) && item.rect.contains(pos))
            return i;
    }

    return -1;
}

QFont NaviTabRail::captionFont(void) const
{
    QFont result(font());
    const qreal points = result.pointSizeF();
    if (points > 0.0)
    {
        result.setPointSizeF(qMax(6.5, points * 0.86));
    }
    else
    {
        result.setPixelSize(qMax(9, static_cast<int>(result.pixelSize() * 0.86)));
    }

    return result;
}

void NaviTabRail::recalcMetrics(void)
{
    mIconExtent = ICON_EXTENT;

    // The rail is sized by the widest caption line, never by a whole caption, so a two-word
    // name like "Offline Logs" wraps instead of pushing every item wider.
    const QFontMetrics metrics(captionFont());
    int captionWidth{ 0 };
    mCaptionLines = 1;
    for (const sRailItem& item : mItems)
    {
        const QStringList lines = splitCaption(item.label);
        mCaptionLines = qMax(mCaptionLines, static_cast<int>(lines.size()));
        for (const QString& line : lines)
        {
            captionWidth = qMax(captionWidth, metrics.horizontalAdvance(line));
        }
    }

    mLabelledWidth = qBound(RAIL_WIDTH_MIN, captionWidth + 12, RAIL_WIDTH_MAX);
    mRailWidth = mLabels ? mLabelledWidth : RAIL_WIDTH_COMPACT;
    mItemHeight = mLabels ? (mIconExtent + (mCaptionLines * metrics.height()) + 15) : (mIconExtent + 16);

    setFixedWidth(mRailWidth);
    updateGeometry();
}

void NaviTabRail::relayout(void)
{
    for (sRailItem& item : mItems)
    {
        item.rect = QRect();
    }

    mOverflowRect = QRect();
    mOverflowHovered = false;

    // The collapse button is pinned to the foot of the rail; the navigators fill what is left.
    mChevronRect = QRect(0, qMax(0, height() - CHEVRON_BOTTOM - CHEVRON_HEIGHT), mRailWidth, CHEVRON_HEIGHT);
    if (mChevronRect.top() < PADDING_TOP)
        mChevronRect = QRect();

    QList<int> visible;
    for (int i = 0; i < mItems.size(); ++i)
    {
        if (mItems.at(i).visible)
            visible.append(i);
    }

    if (visible.isEmpty())
        return;

    const int slot = mItemHeight + ITEM_SPACING;
    const int foot = mChevronRect.isNull() ? 0 : (CHEVRON_HEIGHT + ITEM_SPACING);
    const int room = qMax(0, height() - (2 * PADDING_TOP) - foot + ITEM_SPACING);
    int fits = qMax(0, room / slot);
    if (fits >= visible.size())
    {
        fits = visible.size();
    }
    else if (fits > 0)
    {
        // The last slot carries the overflow button, so one more item moves into the menu.
        --fits;
    }

    QList<int> shown = visible.mid(0, fits);
    if ((mCurrent >= 0) && (shown.contains(mCurrent) == false) && (shown.isEmpty() == false))
    {
        shown[shown.size() - 1] = mCurrent;
    }

    int top = PADDING_TOP;
    for (int index : shown)
    {
        mItems[index].rect = QRect(0, top, mRailWidth, mItemHeight);
        top += slot;
    }

    if (shown.size() < visible.size())
    {
        const int footTop = mChevronRect.isNull() ? height() : mChevronRect.top() - ITEM_SPACING;
        mOverflowRect = QRect(0, top, mRailWidth, qMin(mItemHeight, qMax(0, footTop - top - PADDING_TOP)));
        if (mOverflowRect.height() < 12)
            mOverflowRect = QRect();
    }
}

void NaviTabRail::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);
    painter.setRenderHint(QPainter::RenderHint::SmoothPixmapTransform, true);

    const QPalette& colors = palette();
    const QColor background = railBackground(colors);
    const QColor foreground = colors.color(QPalette::ColorRole::WindowText);
    const QColor accent     = colors.color(QPalette::ColorRole::Highlight);
    const qreal  ratio      = devicePixelRatioF();

    painter.fillRect(rect(), background);

    // A hairline seam on the inner edge separates the rail from the navigator body.
    painter.setPen(withAlpha(foreground, 38));
    const int seam = (mSide == eSide::West) ? (width() - 1) : 0;
    painter.drawLine(seam, 0, seam, height());

    const QFont caption = captionFont();
    const QFontMetrics metrics(caption);

    for (int i = 0; i < mItems.size(); ++i)
    {
        const sRailItem& item = mItems.at(i);
        if ((item.visible == false) || item.rect.isNull())
            continue;

        const bool selected = (i == mCurrent);
        const bool hovered  = (i == mHovered);
        const QRect body    = item.rect.adjusted(3, 1, -3, -1);

        painter.setPen(Qt::PenStyle::NoPen);
        if (selected)
        {
            painter.setBrush(withAlpha(accent, 42));
            painter.drawRoundedRect(body, 6.0, 6.0);
        }
        else if (hovered)
        {
            painter.setBrush(withAlpha(foreground, 26));
            painter.drawRoundedRect(body, 6.0, 6.0);
        }

        if (selected)
        {
            const qreal markerX = (mSide == eSide::West) ? 0.0 : static_cast<qreal>(width() - MARKER_WIDTH);
            const QRectF marker(markerX, item.rect.top() + MARKER_INSET, MARKER_WIDTH, item.rect.height() - (2 * MARKER_INSET));
            painter.setBrush(accent);
            painter.drawRoundedRect(marker, MARKER_WIDTH / 2.0, MARKER_WIDTH / 2.0);
        }

        QColor iconColor = withAlpha(foreground, 160);
        if (selected)
            iconColor = accent;
        else if (hovered)
            iconColor = foreground;

        const int iconTop = mLabels ? (item.rect.top() + 7) : (item.rect.top() + ((item.rect.height() - mIconExtent) / 2));
        const QRect iconRect((mRailWidth - mIconExtent) / 2, iconTop, mIconExtent, mIconExtent);
        painter.drawPixmap(iconRect, tintedPixmap(item.iconPath, mIconExtent, iconColor, ratio));

        if (item.badge != eBadge::None)
        {
            const QColor badgeColor = (item.badge == eBadge::Active) ? accent : QColor(0xE0, 0x8A, 0x1E);
            const QPointF center(iconRect.right() - 1.0, iconRect.top() + 1.0);
            painter.setBrush(background);
            painter.setPen(Qt::PenStyle::NoPen);
            painter.drawEllipse(center, BADGE_RADIUS + 1.5, BADGE_RADIUS + 1.5);
            painter.setBrush(badgeColor);
            painter.drawEllipse(center, BADGE_RADIUS, BADGE_RADIUS);
        }

        if (mLabels)
        {
            const QStringList lines = splitCaption(item.label);
            const int lineHeight    = metrics.height();
            const int textWidth     = item.rect.width() - 4;
            int lineTop = iconRect.bottom() + 3 + (((mCaptionLines - lines.size()) * lineHeight) / 2);

            painter.setFont(caption);
            painter.setPen(selected ? foreground : withAlpha(foreground, 170));
            for (const QString& line : lines)
            {
                const QRect lineRect(item.rect.left() + 2, lineTop, textWidth, lineHeight);
                painter.drawText(lineRect, Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignVCenter
                               , metrics.elidedText(line, Qt::TextElideMode::ElideRight, textWidth));
                lineTop += lineHeight;
            }
        }

        if (hasFocus() && (i == mFocused))
        {
            QPen focus(withAlpha(accent, 200), 1.0, Qt::PenStyle::DashLine);
            painter.setPen(focus);
            painter.setBrush(Qt::BrushStyle::NoBrush);
            painter.drawRoundedRect(body.adjusted(1, 1, -1, -1), 5.0, 5.0);
        }
    }

    if (mOverflowRect.isNull() == false)
    {
        painter.setPen(Qt::PenStyle::NoPen);
        if (mOverflowHovered)
        {
            painter.setBrush(withAlpha(foreground, 26));
            painter.drawRoundedRect(mOverflowRect.adjusted(3, 1, -3, -1), 6.0, 6.0);
        }

        painter.setBrush(mOverflowHovered ? foreground : withAlpha(foreground, 160));
        const qreal dotX = mRailWidth / 2.0;
        const qreal dotY = mOverflowRect.center().y() + 0.5;
        for (int step = -1; step <= 1; ++step)
        {
            painter.drawEllipse(QPointF(dotX, dotY + (step * 5.0)), 1.7, 1.7);
        }
    }

    if (mChevronRect.isNull() == false)
    {
        painter.setPen(withAlpha(foreground, 30));
        painter.drawLine(mChevronRect.left() + 6, mChevronRect.top() - 1, mChevronRect.right() - 6, mChevronRect.top() - 1);

        painter.setPen(Qt::PenStyle::NoPen);
        if (mChevronHovered)
        {
            painter.setBrush(withAlpha(foreground, 26));
            painter.drawRoundedRect(mChevronRect.adjusted(3, 1, -3, -1), 6.0, 6.0);
        }

        // The arrow points the way the content travels, so it flips with the rail side too.
        const bool toLeft = (mSide == eSide::West) ? (mCollapsed == false) : mCollapsed;
        const qreal midX  = mRailWidth / 2.0;
        const qreal midY  = mChevronRect.center().y() + 0.5;
        const qreal step  = toLeft ? -2.6 : 2.6;
        QPen arrow(mChevronHovered ? foreground : withAlpha(foreground, 150), 1.4);
        arrow.setCapStyle(Qt::PenCapStyle::RoundCap);
        arrow.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
        painter.setPen(arrow);
        painter.setBrush(Qt::BrushStyle::NoBrush);
        for (int wing = 0; wing < 2; ++wing)
        {
            const qreal x = midX + (step * (wing == 0 ? -0.4 : 1.4));
            QPainterPath path;
            path.moveTo(x - (step * 0.5), midY - 3.0);
            path.lineTo(x + (step * 0.5), midY);
            path.lineTo(x - (step * 0.5), midY + 3.0);
            painter.drawPath(path);
        }
    }
}

void NaviTabRail::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::MouseButton::LeftButton)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    const int index = indexAt(event->position().toPoint());
    if (index == CHEVRON_MARK)
    {
        emit signalToggleCollapseRequested();
    }
    else if (index == OVERFLOW_MARK)
    {
        showOverflowMenu();
    }
    else if (index >= 0)
    {
        setFocus(Qt::FocusReason::MouseFocusReason);
        mFocused = index;
        if (index == mCurrent)
        {
            update();
            emit signalCurrentItemClicked();
        }
        else
        {
            activateIndex(index);
        }
    }
    else
    {
        QWidget::mousePressEvent(event);
    }
}

void NaviTabRail::mouseDoubleClickEvent(QMouseEvent* event)
{
    // Only the item that is already current collapses the panel, so the gesture cannot fire while
    // the first click of the pair is still switching navigators.
    if ((event->button() == Qt::MouseButton::LeftButton) && (indexAt(event->position().toPoint()) == mCurrent) && (mCurrent >= 0))
    {
        emit signalToggleCollapseRequested();
        event->accept();
    }
    else
    {
        QWidget::mouseDoubleClickEvent(event);
    }
}

void NaviTabRail::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint pos = event->position().toPoint();
    const int index = indexAt(pos);
    const int hovered = (index >= 0) ? index : -1;
    const bool overflow = (index == OVERFLOW_MARK);
    const bool chevron  = (index == CHEVRON_MARK);

    if ((hovered != mHovered) || (overflow != mOverflowHovered) || (chevron != mChevronHovered))
    {
        mHovered = hovered;
        mOverflowHovered = overflow;
        mChevronHovered = chevron;
        QString tip;
        if (hovered >= 0)
        {
            tip = toolTipOf(hovered);
        }
        else if (overflow)
        {
            tip = tr("More navigators");
        }
        else if (chevron)
        {
            tip = QStringLiteral("<b>%1</b><br>%2")
                    .arg((mCollapsed ? tr("Expand the panel") : tr("Collapse the panel")).toHtmlEscaped()
                       , QStringLiteral("Ctrl+B"));
        }

        setToolTip(tip);
        update();
    }

    QWidget::mouseMoveEvent(event);
}

void NaviTabRail::leaveEvent(QEvent* event)
{
    if ((mHovered >= 0) || mOverflowHovered || mChevronHovered)
    {
        mHovered = -1;
        mOverflowHovered = false;
        mChevronHovered = false;
        setToolTip(QString());
        update();
    }

    QWidget::leaveEvent(event);
}

void NaviTabRail::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key::Key_Up:
    case Qt::Key::Key_Left:
        stepFocus(-1);
        break;

    case Qt::Key::Key_Down:
    case Qt::Key::Key_Right:
        stepFocus(1);
        break;

    case Qt::Key::Key_Home:
        stepFocus(-mItems.size());
        break;

    case Qt::Key::Key_End:
        stepFocus(mItems.size());
        break;

    case Qt::Key::Key_Space:
    case Qt::Key::Key_Return:
    case Qt::Key::Key_Enter:
        if ((mFocused >= 0) && (mFocused < mItems.size()) && mItems.at(mFocused).visible)
        {
            activateIndex(mFocused);
        }
        break;

    default:
        QWidget::keyPressEvent(event);
        return;
    }

    event->accept();
}

void NaviTabRail::focusInEvent(QFocusEvent* event)
{
    if ((mFocused < 0) || (mFocused >= mItems.size()) || (mItems.at(mFocused).visible == false))
    {
        mFocused = (mCurrent >= 0) ? mCurrent : indexOf(firstVisibleItem());
    }

    update();
    QWidget::focusInEvent(event);
}

void NaviTabRail::focusOutEvent(QFocusEvent* event)
{
    update();
    QWidget::focusOutEvent(event);
}

void NaviTabRail::resizeEvent(QResizeEvent* event)
{
    relayout();
    QWidget::resizeEvent(event);
}

void NaviTabRail::changeEvent(QEvent* event)
{
    const QEvent::Type type = event->type();
    if ((type == QEvent::Type::FontChange) || (type == QEvent::Type::StyleChange))
    {
        recalcMetrics();
        relayout();
    }

    if ((type == QEvent::Type::PaletteChange) || (type == QEvent::Type::FontChange) || (type == QEvent::Type::StyleChange))
    {
        update();
    }

    QWidget::changeEvent(event);
}

void NaviTabRail::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);

    QAction* labels = menu.addAction(tr("Show &Labels"));
    labels->setCheckable(true);
    labels->setChecked(mLabelsPreferred);
    connect(labels, &QAction::toggled, this, [this](bool on) { emit signalLabelsToggled(on); });

    if (mItems.isEmpty() == false)
    {
        menu.addSeparator();
        for (const sRailItem& item : mItems)
        {
            QAction* entry = menu.addAction(item.label);
            entry->setCheckable(true);
            entry->setChecked(item.visible);
            entry->setIcon(QIcon(tintedPixmap(item.iconPath, 16, palette().color(QPalette::ColorRole::WindowText), devicePixelRatioF())));
            const int id = item.id;
            connect(entry, &QAction::toggled, this, [this, id](bool on) { emit signalItemVisibilityToggled(id, on); });
        }
    }

    menu.addSeparator();
    QAction* hide = menu.addAction(tr("&Hide Navigation Panel"));
    connect(hide, &QAction::triggered, this, [this]() { emit signalHidePanelRequested(); });

    menu.exec(event->globalPos());
    event->accept();
}

QSize NaviTabRail::sizeHint(void) const
{
    const int count = qMax(1, visibleCount());
    return QSize(mRailWidth, (count * (mItemHeight + ITEM_SPACING)) + (2 * PADDING_TOP));
}

QSize NaviTabRail::minimumSizeHint(void) const
{
    return QSize(mRailWidth, mItemHeight + (2 * PADDING_TOP));
}

void NaviTabRail::stepFocus(int delta)
{
    if (mItems.isEmpty() || (delta == 0))
        return;

    const int step = (delta > 0) ? 1 : -1;
    int index = mFocused;
    if ((index < 0) || (index >= mItems.size()))
    {
        index = (step > 0) ? -1 : mItems.size();
    }

    for (int left = qAbs(delta); left > 0; --left)
    {
        int probe = index + step;
        while ((probe >= 0) && (probe < mItems.size()) && (mItems.at(probe).visible == false))
        {
            probe += step;
        }

        if ((probe < 0) || (probe >= mItems.size()))
            break;

        index = probe;
    }

    if ((index >= 0) && (index < mItems.size()) && (index != mFocused))
    {
        mFocused = index;
        update();
    }
}

void NaviTabRail::activateIndex(int index)
{
    if ((index < 0) || (index >= mItems.size()) || (mItems.at(index).visible == false) || (index == mCurrent))
        return;

    mCurrent = index;
    mFocused = index;
    relayout();
    update();
    emit signalItemActivated(mItems.at(index).id);
}

void NaviTabRail::showOverflowMenu(void)
{
    QMenu menu(this);
    const QColor foreground = palette().color(QPalette::ColorRole::WindowText);
    for (int i = 0; i < mItems.size(); ++i)
    {
        const sRailItem& item = mItems.at(i);
        if ((item.visible == false) || (item.rect.isNull() == false))
            continue;

        QAction* entry = menu.addAction(QIcon(tintedPixmap(item.iconPath, 16, foreground, devicePixelRatioF())), item.label);
        entry->setCheckable(true);
        entry->setChecked(i == mCurrent);
        const int index = i;
        connect(entry, &QAction::triggered, this, [this, index]() { activateIndex(index); });
    }

    if (menu.isEmpty())
        return;

    const QPoint origin = (mSide == eSide::West) ? mOverflowRect.topRight() : mOverflowRect.topLeft();
    menu.exec(mapToGlobal(origin));
}

QString NaviTabRail::toolTipOf(int index) const
{
    if ((index < 0) || (index >= mItems.size()))
        return QString();

    const sRailItem& item = mItems.at(index);
    QString result = QStringLiteral("<b>%1</b>").arg(item.label.toHtmlEscaped());
    if (item.hint.isEmpty() == false)
    {
        result += QStringLiteral("<br>%1").arg(item.hint.toHtmlEscaped());
    }

    if (item.shortcut.isEmpty() == false)
    {
        result += QStringLiteral("<br>%1").arg(item.shortcut.toHtmlEscaped());
    }

    if (mCollapsed)
    {
        result += QStringLiteral("<br>%1").arg(tr("Click to open").toHtmlEscaped());
    }
    else if (index == mCurrent)
    {
        result += QStringLiteral("<br>%1").arg(tr("Double-click to collapse the panel").toHtmlEscaped());
    }

    return result;
}

void NaviTabRail::setCollapsed(bool collapsed)
{
    if (mCollapsed == collapsed)
        return;

    mCollapsed = collapsed;
    update();
}
