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
 *  \file        lusan/view/log/LogTextHighlight.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Styling class to highlight search elements.
 *
 ************************************************************************/

#include "lusan/view/log/LogTextHighlight.hpp"

#include "lusan/common/NELogPalette.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"

#include "areg/logging/LoggingDefs.hpp"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QFontMetrics>
#include <QTextLayout>
#include <QTextLine>
#include <QTextOption>
#include <QList>

LogTextHighlight::LogTextHighlight(const LogSearchModel::sFoundPos& foundPos, QObject* parent /*= nullptr*/)
    : QStyledItemDelegate(parent)
    , mFoundPos(foundPos)
    , mMarkEnter(NELusanCommon::iconScopeEnter(QSize(LogTextHighlight::MarkSide, LogTextHighlight::MarkSide)))
    , mMarkExit (NELusanCommon::iconScopeExit (QSize(LogTextHighlight::MarkSide, LogTextHighlight::MarkSide)))
    , mElideLeft(0)
    , mWordWrap (false)
    , mMaxLines (4)
{
}

int LogTextHighlight::wrappedHeight(const QString& text, const QFontMetrics& metrics, int width, int maxLines)
{
    const int line{ metrics.lineSpacing() };
    const int air { 4 };
    if (text.isEmpty() || (width <= 0))
        return line + air;

    const QRect box{ metrics.boundingRect(QRect(0, 0, width, 0), Qt::TextFlag::TextWordWrap, text) };
    const int lines{ qBound(1, (box.height() + (line / 2)) / qMax(1, line), qMax(1, maxLines)) };
    return (lines * line) + air;
}

void LogTextHighlight::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    const int column{ index.column() };
    if ((column >= 0) && (column < 32) && (((mElideLeft >> column) & 1u) != 0u))
    {
        option->textElideMode = Qt::TextElideMode::ElideLeft;
    }

    // A wrapped cell reads from its top edge. Centring it would move the first line down as
    // soon as the row grows, and the eye would lose the column it was following.
    if (mWordWrap && ((option->features & QStyleOptionViewItem::ViewItemFeature::WrapText) != 0))
    {
        option->displayAlignment = (option->displayAlignment & ~Qt::AlignmentFlag::AlignVertical_Mask)
                                 | Qt::AlignmentFlag::AlignTop;
    }
}

void LogTextHighlight::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    _paintAnalyzed(painter, option, index);
    _paintRevealed(painter, option, index);
    _paintPriorityRail(painter, option, index);
    _paintScopeMark(painter, option, index);
    _paintDayChange(painter, option, index);

    const QStyleOptionViewItem& cell{ option };
    const int foundColumn = static_cast<int>(mFoundPos.colFound);
    if ((static_cast<int>(mFoundPos.rowFound) != index.row()) ||
        ((foundColumn >= 0) && (foundColumn != index.column())) ||
        (mFoundPos.posStart < 0) || (mFoundPos.posEnd <= mFoundPos.posStart))
    {
        QStyledItemDelegate::paint(painter, cell, index);
        return;
    }
    
    const int start{ mFoundPos.posStart };
    const int end{ mFoundPos.posEnd };
    const QString cellText{ index.data(Qt::DisplayRole).toString() };
    painter->save();

    QTextLayout layout(cellText, option.font);
    QTextOption shape;
    shape.setWrapMode(mWordWrap ? QTextOption::WrapMode::WrapAtWordBoundaryOrAnywhere
                                : QTextOption::WrapMode::NoWrap);
    layout.setTextOption(shape);

    QTextLayout::FormatRange highlightRange;
    highlightRange.start = start;
    highlightRange.length = end - start;
    highlightRange.format.setBackground(NELogPalette::markColor(NELogPalette::eLogMarkRole::MarkSearchHit));
    highlightRange.format.setForeground(NELogPalette::markColor(NELogPalette::eLogMarkRole::MarkSearchHitText));
    layout.setFormats(QList<QTextLayout::FormatRange>{ highlightRange });

    const int room{ cell.rect.width() - 4 };
    qreal used{ 0.0 };
    layout.beginLayout();
    for (QTextLine line = layout.createLine(); line.isValid(); line = layout.createLine())
    {
        line.setLineWidth(room);
        line.setPosition(QPointF(0.0, used));
        used += line.height();
        if ((mWordWrap == false) || (used + line.height() > cell.rect.height()))
            break;
    }

    layout.endLayout();
    painter->setFont(cell.font);

    // One line sits in the middle of the row, a wrapped block starts at the top of it.
    const qreal top{ mWordWrap ? static_cast<qreal>(cell.rect.top() + 1)
                               : cell.rect.top() + ((cell.rect.height() - used) / 2.0) + 1.0 };
    layout.draw(painter, QPointF(cell.rect.left() + 2, top));
    painter->restore();
}

void LogTextHighlight::_paintRevealed(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data(LogViewerFilter::RevealedRole).toBool() == false)
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->fillRect(option.rect, NELogPalette::markColor(NELogPalette::eLogMarkRole::MarkRevealedRow));

    // The gutter mark rides on the leading column, beside the priority rail, so the row is
    // told apart even where the message is scrolled out of sight.
    if (index.column() == 0)
    {
        const int edge{ 2 };
        painter->fillRect(QRect(option.rect.left(), option.rect.top(), edge, option.rect.height())
                         , NELogPalette::markColor(NELogPalette::eLogMarkRole::MarkRevealedEdge));
    }

    painter->restore();
}

void LogTextHighlight::_paintAnalyzed(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data(LoggingModelBase::AnalyzedRole).toBool() == false)
        return;

    const QColor accent{ QApplication::palette().color(QPalette::ColorRole::Highlight) };

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->fillRect(option.rect, NELogPalette::withOpacity(accent, NELogPalette::eLogOpacity::OpacityTint));

    // The edge rides on the leading column only, so the whole call reads as one block
    // with a single line down its side rather than a box around every row.
    if (index.column() == 0)
    {
        painter->fillRect( QRect(option.rect.left(), option.rect.top(), LogTextHighlight::AnalyzedEdge, option.rect.height())
                         , accent);
    }

    painter->restore();
}

void LogTextHighlight::_paintDayChange(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data(LoggingModelBase::DayChangeRole).toBool() == false)
        return;

    const QColor ink{ QApplication::palette().color(QPalette::ColorRole::WindowText) };

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->fillRect( QRect(option.rect.left(), option.rect.top(), option.rect.width(), 1)
                     , NELogPalette::withOpacity(ink, NELogPalette::eLogOpacity::OpacityHover));
    painter->restore();
}

void LogTextHighlight::_paintScopeMark(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.column() != 0)
        return;

    const QVariant entryData{ index.data(Qt::UserRole) };
    const areg::LogEntry* entry{ entryData.value<const areg::LogEntry*>() };
    if (entry == nullptr)
        return;

    const bool enters{ entry->logMsgType == areg::LogMessageType::ScopeEnter };
    if ((enters == false) && (entry->logMsgType != areg::LogMessageType::ScopeExit))
        return;

    const int side{ qMin(LogTextHighlight::MarkSide, option.rect.height() - 2) };
    if (side <= 0)
        return;

    const QRect box( option.rect.left() + LogTextHighlight::MarkLeft
                   , option.rect.top() + ((option.rect.height() - side) / 2)
                   , side
                   , side);

    painter->save();
    (enters ? mMarkEnter : mMarkExit).paint(painter, box);
    painter->restore();
}

void LogTextHighlight::_paintPriorityRail(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // The rail rides on the leading column, so it survives every column being hidden
    // except the message, and it stays visible when the row is selected.
    if (index.column() != 0)
        return;

    const QVariant entryData{ index.data(Qt::UserRole) };
    const areg::LogEntry* entry{ entryData.value<const areg::LogEntry*>() };
    if (entry == nullptr)
        return;

    const QColor rail{ NELogPalette::railColor(NELogPalette::roleOf(*entry)) };
    if (rail.alpha() == 0)
        return;

    // Error is drawn heavier than Warning, so the two are told apart with the colour removed.
    const bool heavy{ (entry->logMessagePrio == areg::LogPriority::PrioError) ||
                      (entry->logMessagePrio == areg::LogPriority::PrioFatal) };
    const int width { heavy ? 4 : 3 };
    const int inset { 2 };

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->fillRect(QRect(option.rect.left() + inset, option.rect.top() + 1, width, option.rect.height() - 2), rail);
    painter->restore();
}
