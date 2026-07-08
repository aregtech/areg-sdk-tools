#ifndef LUSAN_VIEW_SM_SMINCLUDELIST_HPP
#define LUSAN_VIEW_SM_SMINCLUDELIST_HPP
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
 *  \file        lusan/view/sm/SMIncludeList.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page — include list panel.
 *
 ************************************************************************/

#include <QKeySequence>
#include <QString>
#include <QWidget>

class QToolButton;
class QTreeWidget;

/**
 * \brief   The include list panel, code-built to mirror the Service Interface Includes list:
 *          an "Include Files:" group holding the add/insert/delete + move up/down toolbar
 *          above a single-column flat list of include locations.
 **/
class SMIncludeList : public QWidget
{
    Q_OBJECT

public:
    explicit SMIncludeList(QWidget* parent = nullptr);

    QTreeWidget* ctrlTableList() const;

    QToolButton* ctrlButtonAdd() const;
    QToolButton* ctrlButtonInsert() const;
    QToolButton* ctrlButtonRemove() const;
    QToolButton* ctrlButtonMoveUp() const;
    QToolButton* ctrlButtonMoveDown() const;
    QToolButton* ctrlButtonUpdate() const;

private:
    void buildUi();

private:
    QTreeWidget*    mTable;
    QToolButton*    mButtonAdd;
    QToolButton*    mButtonInsert;
    QToolButton*    mButtonRemove;
    QToolButton*    mButtonMoveUp;
    QToolButton*    mButtonMoveDown;
    QToolButton*    mButtonUpdate;
};

#endif  // LUSAN_VIEW_SM_SMINCLUDELIST_HPP
