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
 *  \file        lusan/view/sm/SMEventList.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Events page — grouped events and timers list panel.
 *
 ************************************************************************/

#include "lusan/view/sm/SMEventList.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QAction>
#include <QEvent>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

SMEventList::SMEventList(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mTable            (nullptr)
    , mGroupEvents      (nullptr)
    , mGroupTimers      (nullptr)
    , mButtonAdd        (nullptr)
    , mButtonInsert     (nullptr)
    , mButtonRemove     (nullptr)
    , mButtonAddParam   (nullptr)
    , mButtonMoveUp     (nullptr)
    , mButtonMoveDown   (nullptr)
    , mActNewEvent      (nullptr)
    , mActNewTimer      (nullptr)
{
    buildUi();
}

void SMEventList::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* group = new QGroupBox(tr("Events and Timers:"), this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);

    QWidget* toolbar = new QWidget(group);
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setSpacing(5);
    toolbarLayout->setContentsMargins(2, 2, 2, 2);

    mButtonAdd    = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry add")   , tr("Create a new event or timer")      , QKeySequence(Qt::CTRL | Qt::Key_A));
    mButtonRemove = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry delete"), tr("Delete selected entry")            , QKeySequence(Qt::CTRL | Qt::Key_D));
    mButtonInsert = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry insert"), tr("Create and insert new entry above"), QKeySequence(Qt::CTRL | Qt::Key_T));

    // The Add button always opens a menu (no default kind): an event and a timer are unrelated
    // stimuli, so the user always chooses. Adding a parameter is a separate toolbar button.
    mActNewEvent = new QAction(QIcon(QStringLiteral(":/icons/sm-event")), tr("New Event"), this);
    mActNewTimer = new QAction(QIcon(QStringLiteral(":/icons/sm-timer")), tr("New Timer"), this);
    QMenu* addMenu = new QMenu(mButtonAdd);
    addMenu->addAction(mActNewEvent);
    addMenu->addAction(mActNewTimer);
    // Same split-button decoration as the Data Types and Methods pages (MenuButtonPopup, the "+ v"
    // look) so the Add button is visually identical across every list page. The main area does a
    // context-sensitive add (SMEvent::onAddClicked adds an event or a timer by the current row, and
    // falls back to opening this menu when nothing is selected); the drop-down zone always opens the
    // New Event / New Timer menu. Do NOT switch this to InstantPopup -- that drew the native
    // menu-indicator triangle instead of the shared chevron and made this button the odd one out.
    NELusanCommon::decorateToolButton(mButtonAdd, addMenu);

    QFrame* sepParam = new QFrame(toolbar);
    sepParam->setFrameShape(QFrame::VLine);
    sepParam->setMaximumSize(24, 24);

    mButtonAddParam = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/field add"), tr("Create and add new parameter to the selected event"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));

    QFrame* sep = new QFrame(toolbar);
    sep->setFrameShape(QFrame::VLine);
    sep->setMaximumSize(24, 24);

    mButtonMoveUp   = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move up")  , tr("Move selection up.")  , QKeySequence(Qt::CTRL | Qt::Key_Up));
    mButtonMoveDown = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move down"), tr("Move selection down."), QKeySequence(Qt::CTRL | Qt::Key_Down));

    toolbarLayout->addWidget(mButtonAdd);
    toolbarLayout->addWidget(mButtonRemove);
    toolbarLayout->addWidget(mButtonInsert);
    toolbarLayout->addWidget(sepParam);
    toolbarLayout->addWidget(mButtonAddParam);
    toolbarLayout->addWidget(sep);
    toolbarLayout->addWidget(mButtonMoveUp);
    toolbarLayout->addWidget(mButtonMoveDown);
    toolbarLayout->addStretch(1);

    mTable = new QTreeWidget(group);
    mTable->setCursor(Qt::PointingHandCursor);
    mTable->setMouseTracking(true);
    mTable->setDropIndicatorShown(false);
    mTable->setIconSize(QSize(16, 16));
    mTable->setSortingEnabled(false);
    mTable->setAnimated(true);
    mTable->setAllColumnsShowFocus(false);
    mTable->setColumnCount(3);
    mTable->header()->setCascadingSectionResizes(false);
    mTable->header()->setMinimumSectionSize(50);
    mTable->setHeaderLabels(QStringList{ tr("Name:"), tr("Type:"), tr("Value:") });

    // One column policy shared by every list page (Data Types, Attributes, Events, Methods,
    // Constants): the Name column takes all the horizontal slack (Stretch) so names stay readable,
    // and every other column fits its content (ResizeToContents). Kept identical to the Methods
    // table so the Name / Type / Value columns look and behave the same across all pages.
    QHeaderView* header = mTable->header();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    mGroupEvents = new QTreeWidgetItem(mTable);
    mGroupEvents->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    mGroupEvents->setIcon(0, QIcon(QStringLiteral(":/icons/sm-event")));
    // Always show the expand/collapse indicator on the group rows, even while empty, so they
    // read as expandable containers rather than leaf rows.
    mGroupEvents->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    mGroupTimers = new QTreeWidgetItem(mTable);
    mGroupTimers->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    mGroupTimers->setIcon(0, QIcon(QStringLiteral(":/icons/sm-timer")));
    mGroupTimers->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    updateGroups(0, 0);
    mGroupEvents->setExpanded(true);
    mGroupTimers->setExpanded(true);

    groupLayout->addWidget(toolbar);
    groupLayout->addWidget(mTable);
    root->addWidget(group);
}

void SMEventList::decorateGroup(QTreeWidgetItem* group)
{
    QFont font{ mTable->font() };
    font.setBold(true);
    group->setFont(0, font);
    group->setFirstColumnSpanned(true);

    // Palette-derived tint follows the active theme without hardcoding a color.
    QColor tint{ mTable->palette().color(QPalette::Highlight) };
    tint.setAlpha(28);
    group->setBackground(0, tint);
}

void SMEventList::updateGroups(int eventCount, int timerCount)
{
    mGroupEvents->setText(0, tr("Events (%1)").arg(eventCount));
    mGroupTimers->setText(0, tr("Timers (%1)").arg(timerCount));
    decorateGroup(mGroupEvents);
    decorateGroup(mGroupTimers);
}

void SMEventList::changeEvent(QEvent* event)
{
    if ((event->type() == QEvent::PaletteChange) && (mGroupEvents != nullptr))
    {
        decorateGroup(mGroupEvents);
        decorateGroup(mGroupTimers);
    }

    QWidget::changeEvent(event);
}

QTreeWidget* SMEventList::ctrlTableList() const
{
    return mTable;
}

QTreeWidgetItem* SMEventList::ctrlGroupEvents() const
{
    return mGroupEvents;
}

QTreeWidgetItem* SMEventList::ctrlGroupTimers() const
{
    return mGroupTimers;
}

QToolButton* SMEventList::ctrlButtonAdd() const
{
    return mButtonAdd;
}

QToolButton* SMEventList::ctrlButtonInsert() const
{
    return mButtonInsert;
}

QToolButton* SMEventList::ctrlButtonRemove() const
{
    return mButtonRemove;
}

QToolButton* SMEventList::ctrlButtonAddParam() const
{
    return mButtonAddParam;
}

QToolButton* SMEventList::ctrlButtonMoveUp() const
{
    return mButtonMoveUp;
}

QToolButton* SMEventList::ctrlButtonMoveDown() const
{
    return mButtonMoveDown;
}

QAction* SMEventList::actionNewEvent() const
{
    return mActNewEvent;
}

QAction* SMEventList::actionNewTimer() const
{
    return mActNewTimer;
}
