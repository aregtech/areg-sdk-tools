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

namespace
{
    ElementListConfig listConfigOf(void)
    {
        return ElementListConfig{ QObject::tr("Events and Timers:")
                                , QStringList{ QObject::tr("Name:"), QObject::tr("Type:"), QObject::tr("Value:") }
                                , QObject::tr("entry")
                                , QObject::tr("parameter")
                                , false };
    }
}

SMEventList::SMEventList(QWidget* parent /*= nullptr*/)
    : ElementListView (listConfigOf(), parent)
    , mGroupEvents    (nullptr)
    , mGroupTimers    (nullptr)
    , mActNewEvent    (nullptr)
    , mActNewTimer    (nullptr)
{
    ctrlButtonAdd()->setToolTip(tr("Create a new event or timer"));

    // The Add button always opens a menu (no default kind): an event and a timer are unrelated
    // stimuli, so the user always chooses.
    mActNewEvent = new QAction(NELusanCommon::loadIcon(QStringLiteral(":/icons/sm-event"), NELusanCommon::SizeSmall), tr("New Event"), this);
    mActNewTimer = new QAction(NELusanCommon::loadIcon(QStringLiteral(":/icons/sm-timer"), NELusanCommon::SizeSmall), tr("New Timer"), this);
    QMenu* addMenu = new QMenu(ctrlButtonAdd());
    addMenu->addAction(mActNewEvent);
    addMenu->addAction(mActNewTimer);
    // The same split-button decoration as the Data Types and Methods pages. The main area adds by
    // the current row, the drop-down zone opens the New Event / New Timer menu.
    NELusanCommon::decorateToolButton(ctrlButtonAdd(), addMenu);

    QTreeWidget* table = ctrlTableList();
    mGroupEvents = new QTreeWidgetItem(table);
    mGroupEvents->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    mGroupEvents->setIcon(0, NELusanCommon::loadIcon(QStringLiteral(":/icons/sm-event"), NELusanCommon::SizeSmall));
    // Always show the expand/collapse indicator on the group rows, even while empty, so they
    // read as expandable containers rather than leaf rows.
    mGroupEvents->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    mGroupTimers = new QTreeWidgetItem(table);
    mGroupTimers->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    mGroupTimers->setIcon(0, NELusanCommon::loadIcon(QStringLiteral(":/icons/sm-timer"), NELusanCommon::SizeSmall));
    mGroupTimers->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    updateGroups(0, 0);
    mGroupEvents->setExpanded(true);
    mGroupTimers->setExpanded(true);
}

void SMEventList::decorateGroup(QTreeWidgetItem* group)
{
    QFont font{ ctrlTableList()->font() };
    font.setBold(true);
    group->setFont(0, font);
    group->setFirstColumnSpanned(true);

    // Palette-derived tint follows the active theme without hardcoding a color.
    QColor tint{ ctrlTableList()->palette().color(QPalette::Highlight) };
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

    ElementListView::changeEvent(event);
}

QTreeWidgetItem* SMEventList::ctrlGroupEvents() const
{
    return mGroupEvents;
}

QTreeWidgetItem* SMEventList::ctrlGroupTimers() const
{
    return mGroupTimers;
}

QAction* SMEventList::actionNewEvent() const
{
    return mActNewEvent;
}

QAction* SMEventList::actionNewTimer() const
{
    return mActNewTimer;
}
