#ifndef LUSAN_VIEW_COMMON_METHODLISTVIEW_HPP
#define LUSAN_VIEW_COMMON_METHODLISTVIEW_HPP
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
 *  \file        lusan/view/common/MethodListView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "method list" panel used by the Service Interface
 *               and State Machine Methods pages.
 *
 ************************************************************************/

#include <QList>
#include <QString>
#include <QStringList>
#include <QWidget>

class QAction;
class QToolButton;
class QTreeWidget;

/**
 * \brief   The section-3 differences between the Service Interface and State Machine Methods
 *          list panels, parameterized so a single shared view serves both editors:
 *          - `groupTitle`          : the group box caption.
 *          - `typeColumnLabel`     : the header text of the type column ("Data Type:" vs "Type:").
 *          - `typeMenuLabels`      : the Add split-button drop-down entries, one per method kind;
 *                                    a plain Add click creates the first kind (the default).
 *          - `hasResponseColumn`   : append a fourth "Response:" column (Service Interface).
 *          - `hasParamInsertRemove`: show the separate delete/insert-parameter buttons (Service
 *                                    Interface); the State Machine editor has only "add parameter".
 **/
struct MethodListConfig
{
    QString     groupTitle;
    QString     typeColumnLabel;
    QStringList typeMenuLabels;
    bool        hasResponseColumn;
    bool        hasParamInsertRemove;
};

/**
 * \brief   The shared methods list panel: an add/insert/delete + add-parameter (+ optional
 *          delete/insert-parameter) + move up/down toolbar above a tree whose top-level rows are
 *          methods and whose child rows are the method parameters. The Add split button always
 *          adds a method; its drop-down offers the method kinds explicitly.
 *
 *          The view is controller-agnostic: it only builds the widgets and exposes ctrl*()
 *          accessors. The page controller owns all row population and selection logic. Columns
 *          are always Name / Type / Value, with an optional fourth Response column.
 **/
class MethodListView : public QWidget
{
    Q_OBJECT

public:
    /**
     * \brief   Column indexes of the method list.
     **/
    enum eColumn
    {
          ColName     = 0 //!< The method / parameter name.
        , ColType     = 1 //!< The method kind / parameter data type.
        , ColValue    = 2 //!< The parameter default value (empty for methods).
        , ColResponse = 3 //!< The connected response (Service Interface only).
    };

    explicit MethodListView(const MethodListConfig& config, QWidget* parent = nullptr);

    QTreeWidget* ctrlTableList() const;

    //!< The split Add button; the drop-down actions are typeAction(index).
    QToolButton* ctrlButtonAdd() const;
    QToolButton* ctrlButtonInsert() const;
    QToolButton* ctrlButtonRemove() const;
    //!< The "add parameter" button (enabled only when a method/parameter is selected).
    QToolButton* ctrlButtonParamAdd() const;
    //!< The "delete parameter" button, or nullptr when the config omits it.
    QToolButton* ctrlButtonParamRemove() const;
    //!< The "insert parameter" button, or nullptr when the config omits it.
    QToolButton* ctrlButtonParamInsert() const;
    QToolButton* ctrlButtonMoveUp() const;
    QToolButton* ctrlButtonMoveDown() const;

    //!< The Add drop-down action for the given method kind, or nullptr if out of range.
    QAction* typeAction(int index) const;

private:
    void buildUi();

private:
    const MethodListConfig  mConfig;
    QTreeWidget*            mTable;
    QToolButton*            mButtonAdd;
    QToolButton*            mButtonInsert;
    QToolButton*            mButtonRemove;
    QToolButton*            mButtonParamAdd;
    QToolButton*            mButtonParamRemove;
    QToolButton*            mButtonParamInsert;
    QToolButton*            mButtonMoveUp;
    QToolButton*            mButtonMoveDown;
    QList<QAction*>         mTypeActions;
};

#endif  // LUSAN_VIEW_COMMON_METHODLISTVIEW_HPP
