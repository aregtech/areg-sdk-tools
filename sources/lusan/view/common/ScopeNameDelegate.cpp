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
 *  \file        lusan/view/common/ScopeNameDelegate.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The delegate that marks the matched part of a scope name.
 *
 ************************************************************************/

#include "lusan/view/common/ScopeNameDelegate.hpp"

#include "lusan/common/NELogPalette.hpp"
#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/model/log/LoggingScopesModelBase.hpp"

#include <QApplication>
#include <QFontMetrics>
#include <QModelIndex>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>

namespace
{
    //! The wash of a mark drawn over an unselected row and over a selected one.
    constexpr qreal _markPlain      { 0.35 };
    constexpr qreal _markSelected   { 0.45 };

    //! The corner of a mark, and how far it grows past the glyphs.
    constexpr qreal _markRadius     { 2.0 };
    constexpr int   _markGrow       { 1 };

    //! The target state dot: its edge and the gap it keeps from the end of the cell.
    constexpr qreal _dotEdge        { 7.0 };
    constexpr qreal _dotInset       { 5.0 };
}

ScopeNameDelegate::ScopeNameDelegate(QObject* parent)
    : QStyledItemDelegate   (parent)

    , mNeedle               ( )
    , mSensitivity          (Qt::CaseSensitivity::CaseInsensitive)
{
}

bool ScopeNameDelegate::setNeedle(const QString& needle, Qt::CaseSensitivity sensitivity)
{
    if ((mNeedle == needle) && (mSensitivity == sensitivity))
        return false;

    mNeedle = needle;
    mSensitivity = sensitivity;
    return true;
}

void ScopeNameDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    if (painter == nullptr)
        return;

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    const QWidget* widget{ opt.widget };
    const QStyle* uiStyle{ widget != nullptr ? widget->style() : QApplication::style() };
    Q_ASSERT(uiStyle != nullptr);

    const int textMargin{ uiStyle->pixelMetric(QStyle::PixelMetric::PM_FocusFrameHMargin, nullptr, widget) + 1 };
    const QRect box{ uiStyle->subElementRect(QStyle::SubElement::SE_ItemViewItemText, &opt, widget).adjusted(textMargin, 0, -textMargin, 0) };
    if (box.isEmpty())
        return;

    painter->save();
    painter->setRenderHint(QPainter::RenderHint::Antialiasing, true);
    painter->setPen(Qt::PenStyle::NoPen);

    if ((mNeedle.isEmpty() == false) && opt.text.contains(mNeedle, mSensitivity))
    {
        paintMatches(*painter, opt, box);
    }

    paintTargetState(*painter, opt, index);
    painter->restore();
}

void ScopeNameDelegate::paintMatches(QPainter& painter, const QStyleOptionViewItem& option, const QRect& box) const
{
    const QString& text{ option.text };
    const QFontMetrics metrics{ option.font };
    const int height{ metrics.height() };
    const int top{ box.y() + ((box.height() - height) / 2) };

    const bool selected{ (option.state & QStyle::StateFlag::State_Selected) != 0 };
    const QPalette::ColorGroup group{ (option.state & QStyle::StateFlag::State_Enabled) != 0 ? QPalette::ColorGroup::Normal : QPalette::ColorGroup::Disabled };
    // The mark takes the colour that contrasts with what it sits on: the selection ink on a
    // plain row, the row ink on a selected one.
    QColor mark{ option.palette.color(group, selected ? QPalette::ColorRole::Base : QPalette::ColorRole::Highlight) };
    mark.setAlphaF(selected ? _markSelected : _markPlain);

    painter.save();
    painter.setClipRect(box);
    painter.setBrush(mark);

    for (int at = text.indexOf(mNeedle, 0, mSensitivity); at >= 0; at = text.indexOf(mNeedle, at + mNeedle.length(), mSensitivity))
    {
        const int left{ box.x() + metrics.horizontalAdvance(text, at) };
        const int right{ box.x() + metrics.horizontalAdvance(text, at + mNeedle.length()) };
        painter.drawRoundedRect(QRectF(left - _markGrow, top, (right - left) + (_markGrow * 2), height), _markRadius, _markRadius);
    }

    painter.restore();
}

void ScopeNameDelegate::paintTargetState(QPainter& painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QVariant held{ index.data(LoggingScopesModelBase::RoleTargetState) };
    if (held.isValid() == false)
        return;

    NELogPalette::eLogStateRole role{ NELogPalette::eLogStateRole::StateCount };
    switch (static_cast<ScopeRoot::eTargetState>(held.toInt()))
    {
    case ScopeRoot::eTargetState::TargetPending:
        role = NELogPalette::eLogStateRole::StatePending;
        break;

    case ScopeRoot::eTargetState::TargetSent:
        role = NELogPalette::eLogStateRole::StateSent;
        break;

    case ScopeRoot::eTargetState::TargetSaved:
        role = NELogPalette::eLogStateRole::StateSaved;
        break;

    default:
        return;
    }

    QColor ink{ NELogPalette::stateColor(role) };
    const qreal fade{ index.data(LoggingScopesModelBase::RoleTargetFade).toReal() };
    if (fade <= 0.0)
        return;

    ink.setAlphaF(qMin(fade, 1.0));

    const QRect& cell{ option.rect };
    const qreal left{ cell.right() - _dotEdge - _dotInset };
    const qreal top{ cell.y() + ((cell.height() - _dotEdge) / 2.0) };

    painter.save();
    painter.setBrush(ink);
    painter.drawEllipse(QRectF(left, top, _dotEdge, _dotEdge));
    painter.restore();
}
