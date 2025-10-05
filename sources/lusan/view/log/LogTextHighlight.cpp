/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/log/LogTextHighlight.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Styling class to highlight search elements.
 *
 ************************************************************************/

#include "lusan/view/log/LogTextHighlight.hpp"

#include <QPainter>
#include <QTextLayout>
#include <QTextLine>
#include <QList>

LogTextHighlight::LogTextHighlight(const LogSearchModel::sFoundPos& foundPos, QObject* parent /*= nullptr*/)
    : QStyledItemDelegate(parent)
    , mFoundPos(foundPos)
{
}

void LogTextHighlight::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if ((static_cast<int>(mFoundPos.rowFound) != index.row()) || (mFoundPos.posStart < 0) || (mFoundPos.posEnd <= mFoundPos.posStart))
    {
        QStyledItemDelegate::paint(painter, option, index);
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
    highlightRange.format.setBackground(Qt::yellow);
    highlightRange.format.setForeground(Qt::red);

    formats.append(highlightRange);
    layout.setFormats(formats);

    layout.beginLayout();
    QTextLine line = layout.createLine();

    // Set the line width to match the highlighted text only
    line.setLineWidth(option.rect.width());
    layout.endLayout();

    painter->setFont(option.font);

    // Calculate vertical alignment (centered)
    float textHeight = static_cast<int>(line.height());
    float yOffset = static_cast<float>(option.rect.top()) + static_cast<float>(option.rect.height() - textHeight) / 2;

    QPointF textPos(option.rect.left() + 2, yOffset + 1);

    layout.draw(painter, textPos);
    painter->restore();
}
