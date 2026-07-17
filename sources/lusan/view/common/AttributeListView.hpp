#ifndef LUSAN_VIEW_COMMON_ATTRIBUTELISTVIEW_HPP
#define LUSAN_VIEW_COMMON_ATTRIBUTELISTVIEW_HPP
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
 *  \file        lusan/view/common/AttributeListView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "attribute list" panel used by the Service
 *               Interface, State Machine and (future) Data Type Attributes pages.
 *
 ************************************************************************/

#include <QKeySequence>
#include <QString>
#include <QWidget>

#include "lusan/view/common/AttributeDetailsView.hpp"

class QToolButton;
class QTreeWidget;

/**
 * \brief   The shared attribute list panel: an "Attributes List:" group holding the
 *          add/insert/delete + move up/down toolbar above a flat list built on a QTreeWidget.
 *
 *          The columns are config-driven: Name and Type are always present; a Value column is
 *          appended when `AttributeViewConfig::hasValue` is set (State Machine) and a
 *          Notification column when `AttributeViewConfig::hasNotification` is set (Service
 *          Interface). For the two current editors this yields exactly three columns with the
 *          differing column at index 2, so the page controllers can address it as a fixed index.
 *
 *          The view is controller-agnostic: it only builds the widgets and exposes ctrl*()
 *          accessors. The page controller owns all row population and selection logic.
 **/
class AttributeListView : public QWidget
{
    Q_OBJECT

public:
    /**
     * \brief   Column indexes of the attribute list. ColName / ColType are always valid;
     *          ColExtra is the config-dependent third column (Value or Notification).
     **/
    enum eColumn
    {
          ColName   = 0 //!< The attribute name (editable inline).
        , ColType   = 1 //!< The attribute data type.
        , ColExtra  = 2 //!< The Value (State Machine) or Notification (Service Interface) column.
    };

    explicit AttributeListView(const AttributeViewConfig& config, QWidget* parent = nullptr);

    QTreeWidget* ctrlTableList() const;

    QToolButton* ctrlButtonAdd() const;
    QToolButton* ctrlButtonInsert() const;
    QToolButton* ctrlButtonRemove() const;
    QToolButton* ctrlButtonMoveUp() const;
    QToolButton* ctrlButtonMoveDown() const;

private:
    void buildUi();

private:
    const AttributeViewConfig mConfig;
    QTreeWidget*    mTable;
    QToolButton*    mButtonAdd;
    QToolButton*    mButtonInsert;
    QToolButton*    mButtonRemove;
    QToolButton*    mButtonMoveUp;
    QToolButton*    mButtonMoveDown;
};

#endif  // LUSAN_VIEW_COMMON_ATTRIBUTELISTVIEW_HPP
