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

#include <QKeySequence>
#include <QString>
#include <QWidget>

class QAction;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * \brief   The list half of the Events page: one 3-column tree (Name/Type/Value) with two
 *          permanent, non-editable top-level group nodes — "Events" (children: events, whose
 *          children are their payload parameters) and "Timers" (children: timers) — under a
 *          single toolbar. The Add button is a split button: its plain click creates the
 *          entry kind matching the current selection, while its drop-down menu always offers
 *          New Event / New Timer / New Parameter explicitly. The panel is presentation only;
 *          the SMEvent page wires the buttons and fills the tree.
 **/
class SMEventList : public QWidget
{
    Q_OBJECT

public:
    //!< The entry kind a plain click on the Add button creates; drives its icon and tooltip.
    enum class eAddTarget : int
    {
          AddEvent  //!< Plain click adds a new event.
        , AddTimer  //!< Plain click adds a new timer.
        , AddParam  //!< Plain click adds a new payload parameter to the selected event.
    };

public:
    explicit SMEventList(QWidget* parent = nullptr);

    //!< The tree with the two permanent group nodes.
    QTreeWidget* ctrlTableList() const;
    //!< The permanent "Events" group node (never deleted, never editable).
    QTreeWidgetItem* ctrlGroupEvents() const;
    //!< The permanent "Timers" group node (never deleted, never editable).
    QTreeWidgetItem* ctrlGroupTimers() const;

    //!< The split Add button; the drop-down actions are actionNewEvent/Timer/Param.
    QToolButton* ctrlButtonAdd() const;
    QToolButton* ctrlButtonInsert() const;
    QToolButton* ctrlButtonRemove() const;
    QToolButton* ctrlButtonMoveUp() const;
    QToolButton* ctrlButtonMoveDown() const;

    //!< The Add drop-down menu entries, always listed regardless of the button's default.
    QAction* actionNewEvent() const;
    QAction* actionNewTimer() const;
    QAction* actionNewParam() const;

    //!< Switches what a plain click on the Add button creates (icon + tooltip follow).
    void setAddTarget(eAddTarget target);

    //!< Refreshes the group labels with live counts and re-applies the group row styling
    //!< (bold, full-row span, accent tint) — call whenever the tree is rebuilt.
    void updateGroups(int eventCount, int timerCount);

protected:
    //!< Re-applies the palette-derived group tint when the application theme changes.
    virtual void changeEvent(QEvent* event) override;

private:
    void buildUi();
    //!< Applies the section-header look (bold, span, tint) to one group node.
    void decorateGroup(QTreeWidgetItem* group);
    static QToolButton* createToolButton(QWidget* parent, const QString& iconName, const QString& toolTip, const QKeySequence& shortcut);

private:
    QTreeWidget*        mTable;         //!< The grouped events/timers tree.
    QTreeWidgetItem*    mGroupEvents;   //!< Permanent "Events" group node.
    QTreeWidgetItem*    mGroupTimers;   //!< Permanent "Timers" group node.
    QToolButton*        mButtonAdd;     //!< Split button: context-aware create + drop-down.
    QToolButton*        mButtonInsert;  //!< Inserts a sibling above the selection.
    QToolButton*        mButtonRemove;  //!< Deletes the selected entry.
    QToolButton*        mButtonMoveUp;  //!< Moves the selection up within its group.
    QToolButton*        mButtonMoveDown;//!< Moves the selection down within its group.
    QAction*            mActNewEvent;   //!< Drop-down: create a new event.
    QAction*            mActNewTimer;   //!< Drop-down: create a new timer.
    QAction*            mActNewParam;   //!< Drop-down: create a new payload parameter.
};

#endif  // LUSAN_VIEW_SM_SMEVENTLIST_HPP
