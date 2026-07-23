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
 *  \file        lusan/view/sm/SMFlowLayout.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM structure lens flow layout.
 *
 ************************************************************************/

#include "lusan/view/sm/SMFlowLayout.hpp"

#include <QWidget>

SMFlowLayout::SMFlowLayout(QWidget* parent /*= nullptr*/, int margin /*= 0*/, int hSpacing /*= 4*/, int vSpacing /*= 4*/)
    : QLayout   (parent)
    , mItems    ( )
    , mHSpace   (hSpacing)
    , mVSpace   (vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

SMFlowLayout::~SMFlowLayout()
{
    clearItems();
}

void SMFlowLayout::addItem(QLayoutItem* item)
{
    mItems.append(item);
}

int SMFlowLayout::count() const
{
    return static_cast<int>(mItems.size());
}

QLayoutItem* SMFlowLayout::itemAt(int index) const
{
    return ((index >= 0) && (index < mItems.size())) ? mItems.at(index) : nullptr;
}

QLayoutItem* SMFlowLayout::takeAt(int index)
{
    return ((index >= 0) && (index < mItems.size())) ? mItems.takeAt(index) : nullptr;
}

Qt::Orientations SMFlowLayout::expandingDirections() const
{
    return {};
}

bool SMFlowLayout::hasHeightForWidth() const
{
    return true;
}

int SMFlowLayout::heightForWidth(int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize SMFlowLayout::minimumSize() const
{
    QSize size;
    for (const QLayoutItem* item : mItems)
    {
        size = size.expandedTo(item->minimumSize());
    }

    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

QSize SMFlowLayout::sizeHint() const
{
    return minimumSize();
}

void SMFlowLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

void SMFlowLayout::clearItems()
{
    while (mItems.isEmpty() == false)
    {
        QLayoutItem* item = mItems.takeLast();
        delete item->widget();
        delete item;
    }
}

int SMFlowLayout::doLayout(const QRect& rect, bool testOnly) const
{
    const QMargins margins = contentsMargins();
    const QRect effective = rect.adjusted(margins.left(), margins.top(), -margins.right(), -margins.bottom());

    int x = effective.x();
    int y = effective.y();
    int lineHeight = 0;

    for (QLayoutItem* item : mItems)
    {
        const QSize hint = item->sizeHint();
        int nextX = x + hint.width() + mHSpace;
        if (((nextX - mHSpace) > effective.right()) && (lineHeight > 0))
        {
            x = effective.x();
            y = y + lineHeight + mVSpace;
            nextX = x + hint.width() + mHSpace;
            lineHeight = 0;
        }

        if (testOnly == false)
        {
            item->setGeometry(QRect(QPoint(x, y), hint));
        }

        x = nextX;
        lineHeight = qMax(lineHeight, hint.height());
    }

    return (y + lineHeight - rect.y()) + margins.bottom();
}
