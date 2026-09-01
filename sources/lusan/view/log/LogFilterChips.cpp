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
 *  \file        lusan/view/log/LogFilterChips.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the row of chips naming the filters a log window has on.
 *
 ************************************************************************/

#include "lusan/view/log/LogFilterChips.hpp"

#include "lusan/common/NELogPalette.hpp"

#include <QAction>
#include <QCursor>
#include <QFontMetrics>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPalette>
#include <QToolButton>

namespace
{
    //! The air the row keeps around the chips.
    constexpr int   _rowMargin  { 3 };

    //! The gap between two chips.
    constexpr int   _rowGap     { 4 };

    //! The widest a chip label is drawn before it is cut short.
    constexpr int   _labelMax   { 240 };

    //! The edge of the button that drops a chip.
    constexpr int   _dropExtent { 12 };
}

LogFilterChips::LogFilterChips(QWidget* parent /*= nullptr*/)
    : QWidget   (parent)

    , mLayout   (nullptr)
    , mClearAll (nullptr)
    , mChips    ( )
{
    setObjectName(QStringLiteral("logFilterChips"));

    mLayout = new QHBoxLayout(this);
    mLayout->setContentsMargins(_rowMargin, _rowMargin, _rowMargin, _rowMargin);
    mLayout->setSpacing(_rowGap);

    mClearAll = new QToolButton(this);
    mClearAll->setText(tr("Clear filters"));
    mClearAll->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
    mClearAll->setAutoRaise(true);
    mClearAll->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
    mClearAll->setToolTip(tr("Drops every filter this window has on, and brings every row back"));
    connect(mClearAll, &QToolButton::clicked, this, [this]() { emit signalClearAll(); });

    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    setVisible(false);
}

void LogFilterChips::setChips(const LogFilterChips::ListChips& chips)
{
    mChips = chips;
    _clearRow();

    for (const LogFilterChips::sChip& chip : mChips)
    {
        _addChip(chip);
    }

    mLayout->addStretch(1);
    mLayout->addWidget(mClearAll);
    mClearAll->setVisible(mChips.size() > 1);
    setVisible(mChips.isEmpty() == false);
}

void LogFilterChips::_clearRow(void)
{
    // The clear button outlives the chips, so it leaves the layout instead of being deleted.
    mLayout->removeWidget(mClearAll);
    mClearAll->setParent(this);

    while (QLayoutItem* item = mLayout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            // A chip is dropped from inside its own button, so it cannot be deleted here.
            // Hiding it keeps it off the row until the deferred delete takes it.
            widget->hide();
            widget->deleteLater();
        }

        delete item;
    }
}

QColor LogFilterChips::_chipColor(LogFilterChips::eChipKind kind, const QPalette& palette)
{
    return (kind == LogFilterChips::eChipKind::ChipScopes)
           ? NELogPalette::textColor(NELogPalette::eLogColorRole::RoleScope)
           : palette.color(QPalette::ColorRole::Highlight);
}

void LogFilterChips::_addChip(const LogFilterChips::sChip& chip)
{
    const QColor tone { LogFilterChips::_chipColor(chip.kind, palette()) };
    const QColor fill { NELogPalette::withOpacity(tone, NELogPalette::eLogOpacity::OpacityTint) };
    const QColor edge { NELogPalette::withOpacity(tone, NELogPalette::eLogOpacity::OpacityHover) };

    QFrame* frame = new QFrame(this);
    frame->setObjectName(QStringLiteral("logFilterChip"));
    frame->setToolTip(chip.hint);
    frame->setStyleSheet(QString("QFrame#logFilterChip { border: 1px solid rgba(%1,%2,%3,%4);"
                                 " border-radius: 3px; background-color: rgba(%1,%2,%3,%5); }")
                         .arg(tone.red()).arg(tone.green()).arg(tone.blue())
                         .arg(edge.alpha()).arg(fill.alpha()));

    QHBoxLayout* row = new QHBoxLayout(frame);
    row->setContentsMargins(6, 1, 2, 1);
    row->setSpacing(4);

    QLabel* text = new QLabel(frame);
    text->setText(QFontMetrics(text->font()).elidedText(chip.label, Qt::TextElideMode::ElideMiddle, _labelMax));
    row->addWidget(text);

    QToolButton* drop = new QToolButton(frame);
    drop->setIcon(NELusanCommon::iconClose(NELusanCommon::SizeBig));
    drop->setIconSize(QSize(_dropExtent, _dropExtent));
    drop->setFixedSize(_dropExtent + 4, _dropExtent + 4);
    drop->setAutoRaise(true);
    drop->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
    drop->setToolTip(tr("Drop this filter"));
    row->addWidget(drop);

    connect(drop, &QToolButton::clicked, this, [this, chip]() { emit signalChipDropped(chip); });

    frame->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(frame, &QWidget::customContextMenuRequested, this, [this, chip, frame](const QPoint& pos) {
            QMenu menu(frame);
            if (chip.phrase.text.isEmpty() == false)
            {
                QAction* search = menu.addAction(tr("Search this instead"));
                search->setStatusTip(tr("Drops the filter and looks for the same phrase, leaving every row in place"));
                connect(search, &QAction::triggered, this, [this, chip]() { emit signalSearchInstead(chip); });
                menu.addSeparator();
            }

            QAction* drop = menu.addAction(tr("Drop this filter"));
            connect(drop, &QAction::triggered, this, [this, chip]() { emit signalChipDropped(chip); });

            QAction* all = menu.addAction(tr("Drop every filter"));
            connect(all, &QAction::triggered, this, [this]() { emit signalClearAll(); });

            menu.exec(frame->mapToGlobal(pos));
        });

    mLayout->addWidget(frame);
}
