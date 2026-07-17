#ifndef LUSAN_VIEW_COMMON_CONSTANTLISTVIEW_HPP
#define LUSAN_VIEW_COMMON_CONSTANTLISTVIEW_HPP
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
 *  \file        lusan/view/common/ConstantListView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "constant list" panel used by the Service
 *               Interface, State Machine and (future) Data Type Constants pages.
 *
 ************************************************************************/

#include <QKeySequence>
#include <QString>
#include <QWidget>

class QToolButton;
class QTreeWidget;

/**
 * \brief   The shared constant list panel: a "Constants List:" group holding the
 *          add/insert/delete + move up/down toolbar above a 3-column flat list
 *          (Name/Data Type/Value), built on a QTreeWidget.
 *
 *          The view is controller-agnostic: it only builds the widgets and exposes ctrl*()
 *          accessors. The tree is intentionally a flat, non-decorated list so that direct
 *          (inline) cell editing can be attached later via item delegates without changing
 *          this view. The page controller owns all row population and selection logic.
 **/
class ConstantListView : public QWidget
{
    Q_OBJECT

public:
    explicit ConstantListView(QWidget* parent = nullptr);

    QTreeWidget* ctrlTableList() const;

    QToolButton* ctrlButtonAdd() const;
    QToolButton* ctrlButtonInsert() const;
    QToolButton* ctrlButtonRemove() const;
    QToolButton* ctrlButtonMoveUp() const;
    QToolButton* ctrlButtonMoveDown() const;

private:
    void buildUi();

private:
    QTreeWidget*    mTable;
    QToolButton*    mButtonAdd;
    QToolButton*    mButtonInsert;
    QToolButton*    mButtonRemove;
    QToolButton*    mButtonMoveUp;
    QToolButton*    mButtonMoveDown;
};

#endif  // LUSAN_VIEW_COMMON_CONSTANTLISTVIEW_HPP
