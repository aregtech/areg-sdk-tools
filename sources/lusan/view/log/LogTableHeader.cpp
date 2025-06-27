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
#include "lusan/model/log/LogViewerModel.hpp"

#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QTableView>
#include <QVBoxLayout>

LogTableHeader::LogTableHeader(LogViewer* viewer, QTableView* parent, LogViewerModel* model, Qt::Orientation orientation /*= Qt::Horizontal*/)
    : QHeaderView   (orientation, parent)
    , mModel        (model)
    , mViewer       (viewer)
    , mHeaders      ( )
{
    setSectionsMovable(true);
    setSectionsClickable(true);
    setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setHighlightSections(true);

    initializeHeaderTypes();
    // updateButtonGeometry();

    connect(this, &QHeaderView::sectionResized, this, &LogTableHeader::updateButtonGeometry);
    connect(this, &QHeaderView::sectionMoved, this, &LogTableHeader::updateButtonGeometry);
}

#if 0
void LogTableHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    if (!rect.isValid())
        return;
    
    LogViewerModel::eColumn col = mModel->fromIndexToColum(logicalIndex);
    Q_ASSERT(col != LogViewerModel::eColumn::LogColumnInvalid);
    mHeaders[static_cast<int>(col)]->setGeometry(rect);
    mHeaders[static_cast<int>(col)]->show();
}
#endif

void LogTableHeader::resizeEvent(QResizeEvent* event)
{
    QHeaderView::resizeEvent(event);
    // updateButtonGeometry();
}

void LogTableHeader::updateButtonGeometry()
{
#if 0
    int count = mModel->columnCount();
    for (int i = 0; i < count; ++i)
    {
        LogViewerModel::eColumn col = mModel->fromIndexToColum(i);
        if (col != LogViewerModel::eColumn::LogColumnInvalid)
        {
            LogHeaderItem* item = mHeaders[static_cast<int>(col)];
            QRect rect = sectionRect(i);
            mHeaders[static_cast<int>(col)]->setGeometry(rect);
            mHeaders[static_cast<int>(col)]->show();
        }
    }
#endif
}

void LogTableHeader::hideAll(void)
{
#if 0
    for (auto* entry : mHeaders)
    {
        entry->hide();
    }
#endif
}

void LogTableHeader::initializeHeaderTypes(void)
{
    const QStringList& names = LogViewerModel::getHeaderList();
    int count = mModel->getMaxColumCount();
    for (int i = 0; i < count; ++i)
    {
        mHeaders.push_back(new LogHeaderObject(*this, i, names[i]));
    }
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
    
    LogViewerModel::eColumn col = mModel->fromIndexToColum(logicalIndex);
    if (col != LogViewerModel::eColumn::LogColumnInvalid)
    {
        constexpr int textMargin { 4 };
        // Button rectangle (left side)
        QRect rcButton(rect.left() + 2, rect.top() + 2, 18, rect.height() - 2);
        
        // Text rectangle (right of button, with a small gap)
        QRect rcText(rcButton.right() + textMargin + 2, rect.top(), rect.right() - rcButton.right() - textMargin, rect.height());
        
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
    QRect buttonRect = QRect(rect.left() + 2, rect.top() + 2, 18, rect.height() - 2);

    if (buttonRect.contains(event->pos()))
    {
        LogViewerModel::eColumn col = mModel->fromIndexToColum(logical);
        if ((col != LogViewerModel::eColumn::LogColumnInvalid) && mHeaders[static_cast<int>(col)]->canPopupFilter())
        {
            // Show filter popup for this column
            mHeaders[static_cast<int>(col)]->showFilters();
            return;
        }
    }

    // Otherwise, allow normal header behavior (resize/move)
    QHeaderView::mousePressEvent(event);
}

