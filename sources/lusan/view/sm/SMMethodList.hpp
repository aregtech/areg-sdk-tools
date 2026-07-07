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
 *          single toolbar. The Add button is a split button: its plain click creates the kind
 *          matching the current selection (a method, or a parameter when a method/parameter is
 *          selected), while its drop-down always offers New Method / New Parameter explicitly.
 *          The panel is presentation only; the SMMethod page wires the buttons and fills the
 *          tree.
 **/
class SMMethodList : public QWidget
{
    Q_OBJECT

public:
    //!< The entry kind a plain click on the Add button creates; drives its icon and tooltip.
    enum class eAddTarget : int
    {
          AddMethod //!< Plain click adds a new method.
        , AddParam  //!< Plain click adds a new parameter to the selected method.
    };

public:
    explicit SMMethodList(QWidget* parent = nullptr);

    QTreeWidget* ctrlTableList() const;

    //!< The split Add button; the drop-down actions are actionNewMethod/Param.
    QToolButton* ctrlButtonAdd() const;
    QToolButton* ctrlButtonInsert() const;
    QToolButton* ctrlButtonRemove() const;
    QToolButton* ctrlButtonMoveUp() const;
    QToolButton* ctrlButtonMoveDown() const;

    QAction* actionNewMethod() const;
    QAction* actionNewParam() const;

    //!< Switches what a plain click on the Add button creates (icon + tooltip follow).
    void setAddTarget(eAddTarget target);

private:
    void buildUi();
    static QToolButton* createToolButton(QWidget* parent, const QString& iconName, const QString& toolTip, const QKeySequence& shortcut);

private:
    QTreeWidget*        mTable;         //!< The methods/parameters tree.
    QToolButton*        mButtonAdd;     //!< Split button: context-aware create + drop-down.
    QToolButton*        mButtonInsert;  //!< Inserts a sibling above the selection.
    QToolButton*        mButtonRemove;  //!< Deletes the selected entry.
    QToolButton*        mButtonMoveUp;  //!< Moves the selection up within its list.
    QToolButton*        mButtonMoveDown;//!< Moves the selection down within its list.
    QAction*            mActNewMethod;  //!< Drop-down: create a new method.
    QAction*            mActNewParam;   //!< Drop-down: create a new parameter.
};

#endif  // LUSAN_VIEW_SM_SMMETHODLIST_HPP
