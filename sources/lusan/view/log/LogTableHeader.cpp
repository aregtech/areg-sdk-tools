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
 *  \file        lusan/view/log/LogTableHeader.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header.
 *
 ************************************************************************/

#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"

#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QTableView>
#include <QVBoxLayout>

LogTableHeader::LogTableHeader(QTableView* parent, LoggingModelBase* model, Qt::Orientation orientation /*= Qt::Horizontal*/)
    : QHeaderView   (orientation, parent)
    , mModel        (model)
    , mHeaders      ( )
{
    setSectionsMovable(true);
    setSectionsClickable(true);
    setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setHighlightSections(true);

    int count = mModel->getMaxColumCount();
    for (int i = 0; i < count; ++i)
    {
        mHeaders.push_back(new LogHeaderItem(*this, i));
    }
}

void LogTableHeader::resetFilters(void)
{
    for (LogHeaderItem* item : mHeaders)
    {
        item->resetFilter();
    }
}

inline void LogTableHeader::drawingRects(const QRect& rect, QRect& rcButton, QRect& rcText) const
{
    constexpr int marginText    { 4 };
    constexpr int marginButton  { 2 };
    constexpr int sizeButton    { 18 };

    // Button rectangle (left side)
    rcButton = QRect(rect.left() + marginButton, rect.top() + marginButton, sizeButton, rect.height() - marginButton);

    // Text rectangle (right of button, with a small gap)
    rcText = QRect(rcButton.right() + marginText + marginButton, rect.top() + marginButton, rect.right() - rcButton.right() - marginText, rect.height() - marginButton);
}

QRect LogTableHeader::sectionRect(int logicalIndex) const
{
    // int pos = sectionPosition(logicalIndex);
    int pos = sectionViewportPosition(logicalIndex);
    int size = sectionSize(logicalIndex);
    if (orientation() == Qt::Horizontal)
        return QRect(pos, 0, size, height());
    else
        return QRect(0, pos, width(), size);
}

void LogTableHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    // Draw the background (mimic QHeaderView default)
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    opt.rect = rect;
    opt.section = logicalIndex;
    opt.state |= QStyle::State_Raised;
    if (isSortIndicatorShown() && (sortIndicatorSection() == logicalIndex))
    {
        opt.sortIndicator = (sortIndicatorOrder() == Qt::AscendingOrder) ? QStyleOptionHeader::SortUp : QStyleOptionHeader::SortDown;
    }
    
    style()->drawControl(QStyle::CE_Header, &opt, painter, this);
    
    LiveLogsModel::eColumn col = mModel->fromIndexToColumn(logicalIndex);
    if (col != LiveLogsModel::eColumn::LogColumnInvalid)
    {
        QRect rcButton; // Button rectangle (left side)
        QRect rcText;   // Text rectangle (right of button, with a small gap)
        drawingRects(rect, rcButton, rcText);
        
        // Draw button and symbol
        if (mHeaders[static_cast<int>(col)]->canPopupFilter())
        {
            // painter->drawRect(rcButton);
            painter->drawText(rcButton, Qt::AlignCenter, QChar(0x25BC)); // ▼
        }
        
        // Draw header text
        painter->drawText(rcText, Qt::AlignVCenter | Qt::AlignLeft, mModel->getHeaderName(logicalIndex));
    }
}

void LogTableHeader::mousePressEvent(QMouseEvent* event)
{
    int logical = logicalIndexAt(event->pos());
    QRect rect = sectionRect(logical);
    QRect rcButton; // Button rectangle (left side)
    QRect rcText;   // Text rectangle (right of button, with a small gap)
    drawingRects(rect, rcButton, rcText);

    if (rcButton.contains(event->pos()))
    {
        LiveLogsModel::eColumn col = mModel->fromIndexToColumn(logical);
        if ((col != LiveLogsModel::eColumn::LogColumnInvalid) && mHeaders[static_cast<int>(col)]->canPopupFilter())
        {
            switch (col)
            {
            case LiveLogsModel::eColumn::LogColumnPriority:
            {
                std::vector<String> names;
                mModel->getPriorityNames(names);
                mHeaders[static_cast<int>(col)]->setFilterData(names);
            }
            break;
                
            case LiveLogsModel::eColumn::LogColumnSource:
            {
                std::vector<String> names;
                mModel->getLogInstanceNames(names);
                mHeaders[static_cast<int>(col)]->setFilterData(names);
            }
            break;
                
            case LiveLogsModel::eColumn::LogColumnSourceId:
            {
                std::vector<ITEM_ID> ids;
                mModel->getLogInstanceIds(ids);
                mHeaders[static_cast<int>(col)]->setFilterData(ids);
            }
            break;
                
            case LiveLogsModel::eColumn::LogColumnThread:
            {
                std::vector<String> names;
                mModel->getLogThreadNames(names);
                mHeaders[static_cast<int>(col)]->setFilterData(names);
            }
            break;
                
            case LiveLogsModel::eColumn::LogColumnThreadId:
            {
                std::vector<ITEM_ID> ids;
                mModel->getLogThreads(ids);
                mHeaders[static_cast<int>(col)]->setFilterData(ids);
            }
            break;
                
            case LiveLogsModel::eColumn::LogColumnScopeId:
            case LiveLogsModel::eColumn::LogColumnMessage:
            default:
                break;
            }

            // Show filter popup for this column
            mHeaders[static_cast<int>(col)]->showFilters();
            return;
        }
    }

    // Otherwise, allow normal header behavior (resize/move)
    QHeaderView::mousePressEvent(event);
}
