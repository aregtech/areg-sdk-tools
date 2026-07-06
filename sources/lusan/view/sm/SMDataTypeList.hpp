#ifndef LUSAN_VIEW_SM_SMDATATYPELIST_HPP
#define LUSAN_VIEW_SM_SMDATATYPELIST_HPP
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
 *  \file        lusan/view/sm/SMDataTypeList.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Data Types page — data type list panel.
 *
 ************************************************************************/

#include <QKeySequence>
#include <QString>
#include <QWidget>

class QToolButton;
class QTreeWidget;

/**
 * \brief   The data type list panel, code-built to mirror SIDataTypeList: a
 *          "Data Types List:" group holding the add/insert/delete + field add/insert/delete
 *          + move up/down toolbar above a 3-column tree (Name/Data Type/Default Value).
 **/
class SMDataTypeList : public QWidget
{
    Q_OBJECT

public:
    explicit SMDataTypeList(QWidget* parent = nullptr);

    QTreeWidget* ctrlTableList(void) const;

    QToolButton* ctrlButtonAdd(void) const;
    QToolButton* ctrlButtonInsert(void) const;
    QToolButton* ctrlButtonRemove(void) const;
    QToolButton* ctrlButtonMoveUp(void) const;
    QToolButton* ctrlButtonMoveDown(void) const;
    QToolButton* ctrlButtonAddField(void) const;
    QToolButton* ctrlButtonInsertField(void) const;
    QToolButton* ctrlButtonRemoveField(void) const;

private:
    void buildUi(void);
    static QToolButton* createToolButton(QWidget* parent, const QString& iconName, const QString& toolTip, const QKeySequence& shortcut);

private:
    QTreeWidget*    mTable;
    QToolButton*    mButtonAdd;
    QToolButton*    mButtonInsert;
    QToolButton*    mButtonRemove;
    QToolButton*    mButtonAddField;
    QToolButton*    mButtonInsertField;
    QToolButton*    mButtonRemoveField;
    QToolButton*    mButtonMoveUp;
    QToolButton*    mButtonMoveDown;
};

#endif  // LUSAN_VIEW_SM_SMDATATYPELIST_HPP
