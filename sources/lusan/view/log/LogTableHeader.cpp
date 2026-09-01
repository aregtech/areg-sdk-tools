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
 *  \file        lusan/view/log/LogTableHeader.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header.
 *
 ************************************************************************/

#include "lusan/view/log/LogTableHeader.hpp"

#include "lusan/common/NELogPalette.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"

#include <QApplication>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTableView>

namespace
{
    //! The air a section keeps at each end.
    constexpr int   _sectionPad     { 10 };

    //! The square the filter funnel is drawn in, at the right end of a section.
    constexpr int   _funnelZone     { 18 };

    //! The edge of the funnel itself inside that square.
    constexpr int   _funnelExtent   { 13 };

    //! The rounding of the plate the funnel is drawn on.
    constexpr qreal _funnelRound    { 4.0 };

    //! The thickness of the line that marks a column the reader has narrowed.
    constexpr int   _markThickness  { 2 };

    //! The air the header keeps above and below the title.
    constexpr int   _titleAir       { 5 };

    //! The funnel of the icon family, on the grid the drawn set uses.
    constexpr qreal _iconGrid       { 24.0 };

    //!< Mixes @p over into @p base, where @p amount is how much of @p over is taken.
    QColor blend(const QColor& base, const QColor& over, qreal amount)
    {
        const qreal keep{ 1.0 - amount };
        return QColor( qRound((base.red()   * keep) + (over.red()   * amount))
                     , qRound((base.green() * keep) + (over.green() * amount))
                     , qRound((base.blue()  * keep) + (over.blue()  * amount)));
    }

    //!< The ground of the header row, a shade apart from the rows below it.
    QColor headerGround(const QPalette& palette)
    {
        const QColor window{ palette.color(QPalette::ColorRole::Window) };
        return NELogPalette::isDarkTheme() ? blend(window, QColor(Qt::GlobalColor::white), 0.05)
                                           : blend(window, QColor(Qt::GlobalColor::white), 0.45);
    }

    //!< The ground of the section the pointer stands on.
    QColor headerHover(const QPalette& palette)
    {
        const QColor ground{ headerGround(palette) };
        return NELogPalette::isDarkTheme() ? blend(ground, QColor(Qt::GlobalColor::white), 0.08)
                                           : blend(ground, QColor(Qt::GlobalColor::black), 0.05);
    }

    //!< The hairline that separates the sections and closes the header at the bottom.
    QColor headerLine(const QPalette& palette)
    {
        const QColor ground{ headerGround(palette) };
        return NELogPalette::isDarkTheme() ? blend(ground, QColor(Qt::GlobalColor::white), 0.18)
                                           : blend(ground, QColor(Qt::GlobalColor::black), 0.16);
    }
}

LogTableHeader::LogTableHeader(QTableView* parent, LoggingModelBase* model, Qt::Orientation orientation /*= Qt::Horizontal*/)
    : QHeaderView   (orientation, parent)
    , mModel        (model)
    , mHeaders      ( )
    , mHovered      (-1)
    , mOnFunnel     (false)
{
    setSectionsMovable(true);
    setSectionsClickable(true);
    setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setHighlightSections(false);
    setMouseTracking(true);

    // The table draws its rows in a fixed width face. The titles are read, not compared
    // column by column, so the header keeps the face of the application.
    setFont(QApplication::font());

    const int count{ mModel != nullptr ? mModel->getMaxColumCount() : 0 };
    for (int i = 0; i < count; ++i)
    {
        mHeaders.push_back(new LogHeaderItem(*this, i));
    }
}

void LogTableHeader::resetFilters()
{
    for (LogHeaderItem* item : mHeaders)
    {
        item->resetFilter();
    }
}

int LogTableHeader::getColumnIndex(LoggingModelBase::eColumn column) const
{
    return (mModel != nullptr ? mModel->fromColumnToIndex(column) : -1);
}

LoggingModelBase::eColumn LogTableHeader::getColumn(int logicalIndex) const
{
    return (mModel != nullptr ? mModel->fromIndexToColumn(logicalIndex) : LoggingModelBase::eColumn::LogColumnInvalid);
}

LogHeaderItem* LogTableHeader::getHeaderItem(LoggingModelBase::eColumn column) const
{
    const int pos{ static_cast<int>(column) };
    return ((pos >= 0) && (pos < static_cast<int>(mHeaders.size())) ? mHeaders[pos] : nullptr);
}

QSize LogTableHeader::sizeHint() const
{
    QSize hint{ QHeaderView::sizeHint() };
    hint.setHeight(qMax(fontMetrics().height() + (_titleAir * 2), _funnelZone + (_titleAir * 2) - 4));
    return hint;
}

inline QRect LogTableHeader::filterRect(const QRect& rect) const
{
    const int extent{ qMin(_funnelZone, rect.height()) };
    return QRect(rect.right() - extent - (_sectionPad / 2), rect.top() + ((rect.height() - extent) / 2), extent, extent);
}

inline QRect LogTableHeader::sectionRect(int logicalIndex) const
{
    if ((logicalIndex < 0) || (logicalIndex >= count()))
        return QRect();

    const int pos { sectionViewportPosition(logicalIndex) };
    const int size{ sectionSize(logicalIndex) };
    return (orientation() == Qt::Horizontal ? QRect(pos, 0, size, height()) : QRect(0, pos, width(), size));
}

bool LogTableHeader::isOnFunnel(const QPoint& pos, int logicalIndex) const
{
    const LogHeaderItem* item{ getHeaderItem(getColumn(logicalIndex)) };
    if ((item == nullptr) || (item->canPopupFilter() == false))
        return false;

    const QRect section{ sectionRect(logicalIndex) };
    return (section.isEmpty() == false) && filterRect(section).contains(pos);
}

void LogTableHeader::drawFunnel(QPainter& painter, const QRect& rect, bool isOn, bool isHot) const
{
    const qreal edge { static_cast<qreal>(qMin(_funnelExtent, qMin(rect.width(), rect.height()))) };
    const qreal scale{ edge / _iconGrid };
    const qreal left { rect.left() + ((rect.width()  - edge) / 2.0) };
    const qreal top  { rect.top()  + ((rect.height() - edge) / 2.0) };

    const auto at = [left, top, scale](qreal x, qreal y) { return QPointF(left + (x * scale), top + (y * scale)); };

    QPainterPath funnel;
    funnel.moveTo(at(4.6, 5.0));
    funnel.lineTo(at(19.4, 5.0));
    funnel.lineTo(at(13.4, 12.4));
    funnel.lineTo(at(13.4, 19.4));
    funnel.lineTo(at(10.6, 17.6));
    funnel.lineTo(at(10.6, 12.4));
    funnel.closeSubpath();

    const QPalette& pal{ palette() };
    const QColor accent{ pal.color(QPalette::ColorRole::Highlight) };
    const QColor ink   { pal.color(QPalette::ColorRole::WindowText) };

    painter.save();
    painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);

    // The funnel of a narrowed column sits on a plate, and so does the one under the pointer.
    if (isOn || isHot)
    {
        QColor plate{ isOn ? accent : ink };
        plate.setAlphaF(isOn ? 0.18 : 0.10);
        painter.setPen(Qt::PenStyle::NoPen);
        painter.setBrush(plate);
        painter.drawRoundedRect(QRectF(rect), _funnelRound, _funnelRound);
    }

    // The funnel keeps its outline in both states. Filling it at this size closes the stem
    // and the shape reads as a plain triangle.
    QPen pen(isOn ? accent : ink);
    pen.setWidthF(isOn ? 1.6 : 1.2);
    pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
    pen.setCapStyle(Qt::PenCapStyle::RoundCap);

    painter.setOpacity(isOn ? 1.0 : (isHot ? 0.85 : 0.4));
    painter.setPen(pen);
    painter.setBrush(Qt::BrushStyle::NoBrush);
    painter.drawPath(funnel);
    painter.restore();
}

void LogTableHeader::refreshColumn(LoggingModelBase::eColumn column)
{
    const int logical{ getColumnIndex(column) };
    if (logical >= 0)
    {
        updateSection(logical);
    }
}

void LogTableHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    const LoggingModelBase::eColumn col{ getColumn(logicalIndex) };
    if (col == LoggingModelBase::eColumn::LogColumnInvalid)
        return;

    const LogHeaderItem* item{ getHeaderItem(col) };
    const bool canFilter    { (item != nullptr) && item->canPopupFilter() };
    const bool isFiltered   { canFilter && item->isFiltered() };
    const bool isHovered    { logicalIndex == mHovered };

    const QPalette& pal{ palette() };
    const QColor accent { pal.color(QPalette::ColorRole::Highlight) };
    const QColor line   { headerLine(pal) };

    painter->save();
    painter->setRenderHint(QPainter::RenderHint::Antialiasing, false);
    painter->fillRect(rect, isHovered ? headerHover(pal) : headerGround(pal));

    // One hairline down the right edge of every section and one along the bottom of the row.
    painter->setPen(line);
    painter->drawLine(rect.right(), rect.top() + _titleAir, rect.right(), rect.bottom() - _titleAir);
    painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());

    QRect title{ rect.adjusted(_sectionPad, 0, -_sectionPad, 0) };
    if (canFilter)
    {
        const QRect zone{ filterRect(rect) };
        title.setRight(zone.left() - (_sectionPad / 2));
        // A column that is narrowed says so at all times. The rest show the funnel when the
        // pointer is on their section, so the header stays quiet while it is read.
        if (isFiltered || isHovered)
        {
            drawFunnel(*painter, zone, isFiltered, isHovered && mOnFunnel);
        }
    }

    if (title.width() > 0)
    {
        QFont face{ painter->font() };
        face.setWeight(isFiltered ? QFont::Weight::DemiBold : QFont::Weight::Medium);
        painter->setFont(face);

        QColor ink{ pal.color(QPalette::ColorRole::WindowText) };
        if (isFiltered == false)
        {
            ink = blend(ink, headerGround(pal), 0.25);
        }

        const int flags{ mModel->getAlignmentData(col) };
        const QString name{ mModel->getHeaderName(logicalIndex) };
        painter->setPen(ink);
        painter->drawText(title, flags, painter->fontMetrics().elidedText(name, Qt::TextElideMode::ElideRight, title.width()));
    }

    // The bar under a narrowed column, so the table says at a glance which ones are filtered.
    if (isFiltered)
    {
        painter->fillRect(QRect(rect.left(), rect.bottom() - _markThickness + 1, rect.width(), _markThickness), accent);
    }

    painter->restore();
}

void LogTableHeader::fillFilterData(LoggingModelBase::eColumn column)
{
    LogHeaderItem* item{ getHeaderItem(column) };
    if ((item == nullptr) || (mModel == nullptr))
        return;

    std::vector<areg::String> names;
    NELusanCommon::AnyList values;

    switch (column)
    {
    case LoggingModelBase::eColumn::LogColumnPriority:
        mModel->getPriorityValues(names, values);
        item->setFilterData(names, values);
        break;

    case LoggingModelBase::eColumn::LogColumnSource:
        mModel->getLogInstances(names, values);
        item->setFilterData(names, values);
        break;

    case LoggingModelBase::eColumn::LogColumnThread:
        mModel->getLogThreadValues(names, values);
        item->setFilterData(names, values);
        break;

    case LoggingModelBase::eColumn::LogColumnSourceId:
        mModel->getLogInstances(names, values);
        item->setFilterData(names, values);
        break;

    case LoggingModelBase::eColumn::LogColumnThreadId:
        mModel->getLogThreadValues(names, values);
        item->setFilterData(names, values);
        break;

    default:
        break;
    }
}

void LogTableHeader::mousePressEvent(QMouseEvent* event)
{
    const int logical{ logicalIndexAt(event->pos()) };
    if ((event->button() == Qt::MouseButton::LeftButton) && isOnFunnel(event->pos(), logical))
    {
        const LoggingModelBase::eColumn col{ getColumn(logical) };
        fillFilterData(col);
        mHeaders[static_cast<int>(col)]->showFilters();
        event->accept();
        return;
    }

    // Otherwise, allow normal header behavior (resize/move)
    QHeaderView::mousePressEvent(event);
}

void LogTableHeader::mouseMoveEvent(QMouseEvent* event)
{
    const int logical{ logicalIndexAt(event->pos()) };
    const bool onFunnel{ isOnFunnel(event->pos(), logical) };
    if ((logical != mHovered) || (onFunnel != mOnFunnel))
    {
        const int left{ mHovered };
        mHovered  = logical;
        mOnFunnel = onFunnel;
        if ((left >= 0) && (left != logical))
        {
            updateSection(left);
        }

        if (logical >= 0)
        {
            updateSection(logical);
        }
    }

    if (onFunnel)
    {
        setCursor(Qt::CursorShape::PointingHandCursor);
    }
    else if (cursor().shape() == Qt::CursorShape::PointingHandCursor)
    {
        // Only the funnel cursor is taken back. The split cursor of a section edge is the
        // header's own, and it manages it itself.
        unsetCursor();
    }

    QHeaderView::mouseMoveEvent(event);
}

void LogTableHeader::leaveEvent(QEvent* event)
{
    if (mHovered >= 0)
    {
        const int left{ mHovered };
        mHovered  = -1;
        mOnFunnel = false;
        updateSection(left);
    }

    QHeaderView::leaveEvent(event);
}
