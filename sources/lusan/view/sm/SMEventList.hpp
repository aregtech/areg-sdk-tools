#ifndef LUSAN_VIEW_SM_SMEVENTLIST_HPP
#define LUSAN_VIEW_SM_SMEVENTLIST_HPP
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
 *  \file        lusan/view/sm/SMEventList.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Events page — grouped events and timers list panel.
 *
 ************************************************************************/

#include "lusan/view/common/ElementListView.hpp"

class QAction;
class QTreeWidgetItem;

/**
 * \brief   The list half of the Events page: one 3-column tree (Name/Type/Value) with two
 *          permanent, non-editable top-level group nodes — "Events" (children: events, whose
 *          children are their payload parameters) and "Timers" (children: timers) — under a
 *          single toolbar. The Add button always opens a menu offering New Event / New Timer
 *          (an event and a timer are unrelated stimuli, so there is no default kind). Adding a
 *          payload parameter is a separate toolbar button, enabled only while an event or
 *          parameter is selected. The panel is presentation only; the SMEvent page wires the
 *          buttons and fills the tree.
 **/
class SMEventList : public ElementListView
{
    Q_OBJECT

public:
    explicit SMEventList(QWidget* parent = nullptr);

    //!< The permanent "Events" group node (never deleted, never editable).
    QTreeWidgetItem* ctrlGroupEvents() const;
    //!< The permanent "Timers" group node (never deleted, never editable).
    QTreeWidgetItem* ctrlGroupTimers() const;

    //!< The Add drop-down menu entries.
    QAction* actionNewEvent() const;
    QAction* actionNewTimer() const;

    //!< Refreshes the group labels with live counts and re-applies the group row styling
    //!< (bold, full-row span, accent tint) — call whenever the tree is rebuilt.
    void updateGroups(int eventCount, int timerCount);

protected:
    //!< Re-applies the palette-derived group tint when the application theme changes.
    virtual void changeEvent(QEvent* event) override;

private:
    //!< Applies the section-header look (bold, span, tint) to one group node.
    void decorateGroup(QTreeWidgetItem* group);

private:
    QTreeWidgetItem*    mGroupEvents;   //!< Permanent "Events" group node.
    QTreeWidgetItem*    mGroupTimers;   //!< Permanent "Timers" group node.
    QAction*            mActNewEvent;   //!< Drop-down: create a new event.
    QAction*            mActNewTimer;   //!< Drop-down: create a new timer.
};

#endif  // LUSAN_VIEW_SM_SMEVENTLIST_HPP
