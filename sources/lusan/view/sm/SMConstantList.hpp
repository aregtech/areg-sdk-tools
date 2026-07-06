#ifndef LUSAN_VIEW_SM_SMCONSTANTLIST_HPP
#define LUSAN_VIEW_SM_SMCONSTANTLIST_HPP
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
 *  \file        lusan/view/sm/SMConstantList.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Constants page — constant list panel.
 *
 ************************************************************************/

#include <QKeySequence>
#include <QString>
#include <QWidget>

class QToolButton;
class QTreeWidget;

/**
 * \brief   The constant list panel, code-built to mirror SIConstantList: a
 *          "Constants List:" group holding the add/insert/delete + move up/down toolbar
 *          above a 3-column flat list (Name/Data Type/Value).
 **/
class SMConstantList : public QWidget
{
    Q_OBJECT

public:
    explicit SMConstantList(QWidget* parent = nullptr);

    QTreeWidget* ctrlTableList() const;

    QToolButton* ctrlButtonAdd() const;
    QToolButton* ctrlButtonInsert() const;
    QToolButton* ctrlButtonRemove() const;
    QToolButton* ctrlButtonMoveUp() const;
    QToolButton* ctrlButtonMoveDown() const;

private:
    void buildUi();
    static QToolButton* createToolButton(QWidget* parent, const QString& iconName, const QString& toolTip, const QKeySequence& shortcut);

private:
    QTreeWidget*    mTable;
    QToolButton*    mButtonAdd;
    QToolButton*    mButtonInsert;
    QToolButton*    mButtonRemove;
    QToolButton*    mButtonMoveUp;
    QToolButton*    mButtonMoveDown;
};

#endif  // LUSAN_VIEW_SM_SMCONSTANTLIST_HPP
