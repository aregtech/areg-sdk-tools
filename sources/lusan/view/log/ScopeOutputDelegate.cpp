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
 *  \file        lusan/view/log/ScopeOutputDelegate.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the call structure drawn beside the scope output rows.
 *
 ************************************************************************/

#include "lusan/view/log/ScopeOutputDelegate.hpp"

#include "lusan/common/NELogPalette.hpp"
#include "lusan/model/log/ScopeLogViewerFilter.hpp"

#include <QAbstractItemModel>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

ScopeOutputDelegate::ScopeOutputDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QRect ScopeOutputDelegate::handleRect(const QRect& rect)
{
    const int side{ qMin(ScopeOutputDelegate::HandleSide, rect.height() - 2) };
    if (side <= 0)
        return QRect();

    return QRect( rect.left() + ScopeOutputDelegate::GutterWidth - side - 3
                , rect.top() + ((rect.height() - side) / 2)
                , side
                , side);
}

void ScopeOutputDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem cell{ option };
    if (index.column() == 0)
    {
        cell.rect.setLeft(cell.rect.left() + ScopeOutputDelegate::GutterWidth);
    }

    QStyledItemDelegate::paint(painter, cell, index);

    if ((index.column() == 0) && (painter != nullptr))
    {
        _paintStructure(*painter, option, index);
    }
}

bool ScopeOutputDelegate::editorEvent( QEvent* event
                                     , QAbstractItemModel* model
                                     , const QStyleOptionViewItem& option
                                     , const QModelIndex& index)
{
    if ((event == nullptr) || (event->type() != QEvent::Type::MouseButtonRelease) || (index.column() != 0))
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    const QMouseEvent* mouse{ static_cast<const QMouseEvent *>(event) };
    if (mouse->button() != Qt::MouseButton::LeftButton)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    const int fold{ index.data(ScopeLogViewerFilter::RoleCallFold).toInt() };
    if (fold == static_cast<int>(ScopeLogViewerFilter::eCallFold::FoldNone))
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    const QRect handle{ ScopeOutputDelegate::handleRect(option.rect) };
    if (handle.isEmpty() || (handle.contains(mouse->pos()) == false))
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    ScopeLogViewerFilter* filter{ qobject_cast<ScopeLogViewerFilter *>(model) };
    return (filter != nullptr) && filter->toggleFold(index);
}

void ScopeOutputDelegate::_paintStructure(QPainter& painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int bracket{ index.data(ScopeLogViewerFilter::RoleCallBracket).toInt() };
    const int fold{ index.data(ScopeLogViewerFilter::RoleCallFold).toInt() };
    const int depth{ qMin(index.data(ScopeLogViewerFilter::RoleCallDepth).toInt(), ScopeOutputDelegate::LevelMax) };
    if ((bracket == static_cast<int>(ScopeLogViewerFilter::eCallBracket::BracketNone))
        && (fold == static_cast<int>(ScopeLogViewerFilter::eCallFold::FoldNone)))
    {
        return;
    }

    const QRect& rect{ option.rect };
    const QColor ink{ NELogPalette::railColor(NELogPalette::eLogColorRole::RoleScope) };
    const QColor ghost{ NELogPalette::withOpacity(ink, NELogPalette::eLogOpacity::OpacityGhost) };
    const int mid{ rect.top() + (rect.height() / 2) };
    const int own{ rect.left() + 4 + (depth * ScopeOutputDelegate::LevelStep) };

    painter.save();
    painter.setRenderHint(QPainter::RenderHint::Antialiasing, false);

    for (int level = 0; level < depth; ++level)
    {
        const int x{ rect.left() + 4 + (level * ScopeOutputDelegate::LevelStep) };
        painter.fillRect(QRect(x, rect.top(), 1, rect.height()), ghost);
    }

    switch (static_cast<ScopeLogViewerFilter::eCallBracket>(bracket))
    {
    case ScopeLogViewerFilter::eCallBracket::BracketOpen:
        painter.fillRect(QRect(own, mid, 2, rect.bottom() - mid + 1), ink);
        painter.fillRect(QRect(own, mid, ScopeOutputDelegate::LevelStep + 1, 2), ink);
        break;

    case ScopeLogViewerFilter::eCallBracket::BracketClose:
        painter.fillRect(QRect(own, rect.top(), 2, mid - rect.top()), ink);
        painter.fillRect(QRect(own, mid - 2, ScopeOutputDelegate::LevelStep + 1, 2), ink);
        break;

    case ScopeLogViewerFilter::eCallBracket::BracketInside:
        painter.fillRect(QRect(own, rect.top(), 1, rect.height()), ghost);
        break;

    default:
        break;
    }

    if (fold != static_cast<int>(ScopeLogViewerFilter::eCallFold::FoldNone))
    {
        const QRect handle{ ScopeOutputDelegate::handleRect(rect) };
        if (handle.isEmpty() == false)
        {
            const bool closed{ fold == static_cast<int>(ScopeLogViewerFilter::eCallFold::FoldClosed) };
            const qreal cx{ handle.center().x() + 0.5 };
            const qreal cy{ handle.center().y() + 0.5 };
            const qreal arm{ handle.width() / 2.6 };

            QPainterPath tip;
            if (closed)
            {
                tip.moveTo(cx - (arm * 0.7), cy - arm);
                tip.lineTo(cx + (arm * 0.9), cy);
                tip.lineTo(cx - (arm * 0.7), cy + arm);
            }
            else
            {
                tip.moveTo(cx - arm, cy - (arm * 0.7));
                tip.lineTo(cx + arm, cy - (arm * 0.7));
                tip.lineTo(cx, cy + (arm * 0.9));
            }

            tip.closeSubpath();
            painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);
            painter.fillPath(tip, ink);
        }
    }

    painter.restore();
}
