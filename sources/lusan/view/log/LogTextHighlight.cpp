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
#include "lusan/model/log/LogViewerFilter.hpp"

#include "areg/logging/LoggingDefs.hpp"

#include <QPainter>
#include <QTextLayout>
#include <QTextLine>
#include <QList>

LogTextHighlight::LogTextHighlight(const LogSearchModel::sFoundPos& foundPos, QObject* parent /*= nullptr*/)
    : QStyledItemDelegate(parent)
    , mFoundPos(foundPos)
    , mMarkEnter(NELusanCommon::iconScopeEnter(QSize(LogTextHighlight::MarkSide, LogTextHighlight::MarkSide)))
    , mMarkExit (NELusanCommon::iconScopeExit (QSize(LogTextHighlight::MarkSide, LogTextHighlight::MarkSide)))
{
}

void LogTextHighlight::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    _paintRevealed(painter, option, index);
    _paintPriorityRail(painter, option, index);
    _paintScopeMark(painter, option, index);

    // The gutter belongs to the delegate, so the text of the leading column starts after it.
    QStyleOptionViewItem cell{ option };
    if (index.column() == 0)
    {
        cell.rect.setLeft(cell.rect.left() + LogTextHighlight::GutterWidth);
    }

    const int foundColumn = static_cast<int>(mFoundPos.colFound);
    if ((static_cast<int>(mFoundPos.rowFound) != index.row()) ||
        ((foundColumn >= 0) && (foundColumn != index.column())) ||
        (mFoundPos.posStart < 0) || (mFoundPos.posEnd <= mFoundPos.posStart))
    {
        QStyledItemDelegate::paint(painter, cell, index);
        return;
    }
    
    int start{ mFoundPos.posStart };
    int end{ mFoundPos.posEnd };
    QString cellText = index.data(Qt::DisplayRole).toString();
    painter->save();

    // Prepare text layout
    QTextLayout layout(cellText, option.font);
    QList<QTextLayout::FormatRange> formats;

    QTextLayout::FormatRange highlightRange;
    highlightRange.start = start;
    highlightRange.length = end - start;
    highlightRange.format.setBackground(NELogPalette::markColor(NELogPalette::eLogMarkRole::MarkSearchHit));
    highlightRange.format.setForeground(NELogPalette::markColor(NELogPalette::eLogMarkRole::MarkSearchHitText));

    formats.append(highlightRange);
    layout.setFormats(formats);

    layout.beginLayout();
    QTextLine line = layout.createLine();

    // Set the line width to match the highlighted text only
    line.setLineWidth(cell.rect.width());
    layout.endLayout();

    painter->setFont(cell.font);

    // Calculate vertical alignment (centered)
    float textHeight = static_cast<int>(line.height());
    float yOffset = static_cast<float>(cell.rect.top()) + static_cast<float>(cell.rect.height() - textHeight) / 2;

    QPointF textPos(cell.rect.left() + 2, yOffset + 1);

    layout.draw(painter, textPos);
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
