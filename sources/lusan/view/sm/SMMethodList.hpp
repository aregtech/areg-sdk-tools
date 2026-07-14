#ifndef LUSAN_VIEW_SM_SMMETHODLIST_HPP
#define LUSAN_VIEW_SM_SMMETHODLIST_HPP
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
 *  \file        lusan/view/sm/SMMethodList.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Methods page — methods and parameters list panel.
 *
 ************************************************************************/

#include <QKeySequence>
#include <QString>
#include <QWidget>

class QAction;
class QToolButton;
class QTreeWidget;

/**
 * \brief   The list half of the Methods page: one 3-column tree (Name/Type/Value) whose
 *          top-level rows are methods and whose child rows are the method parameters, under a
 *          single toolbar. The Add split button always adds a method: a plain click creates a
 *          trigger (the default), and its drop-down offers Trigger / Action / Condition
 *          explicitly. Adding a parameter is a separate toolbar button, enabled only while a
 *          method or parameter is selected. The panel is presentation only; the SMMethod page
 *          wires the buttons and fills the tree.
 **/
class SMMethodList : public QWidget
{
    Q_OBJECT

public:
    explicit SMMethodList(QWidget* parent = nullptr);

    QTreeWidget* ctrlTableList() const;

    //!< The split Add button; the drop-down actions are actionNewTrigger/Action/Condition.
    QToolButton* ctrlButtonAdd() const;
    QToolButton* ctrlButtonInsert() const;
    QToolButton* ctrlButtonRemove() const;
    //!< The separate "add parameter" button (enabled only when a method/parameter is selected).
    QToolButton* ctrlButtonAddParam() const;
    QToolButton* ctrlButtonMoveUp() const;
    QToolButton* ctrlButtonMoveDown() const;

    //!< The Add drop-down entries, one per method kind (default click = Trigger).
    QAction* actionNewTrigger() const;
    QAction* actionNewAction() const;
    QAction* actionNewCondition() const;

private:
    void buildUi();

private:
    QTreeWidget*        mTable;         //!< The methods/parameters tree.
    QToolButton*        mButtonAdd;     //!< Split button: add a method + method-kind drop-down.
    QToolButton*        mButtonInsert;  //!< Inserts a sibling above the selection.
    QToolButton*        mButtonRemove;  //!< Deletes the selected entry.
    QToolButton*        mButtonAddParam;//!< Adds a parameter to the selected method.
    QToolButton*        mButtonMoveUp;  //!< Moves the selection up within its list.
    QToolButton*        mButtonMoveDown;//!< Moves the selection down within its list.
    QAction*            mActNewTrigger; //!< Drop-down: create a new trigger method.
    QAction*            mActNewAction;  //!< Drop-down: create a new action method.
    QAction*            mActNewCondition;//!< Drop-down: create a new condition method.
};

#endif  // LUSAN_VIEW_SM_SMMETHODLIST_HPP
