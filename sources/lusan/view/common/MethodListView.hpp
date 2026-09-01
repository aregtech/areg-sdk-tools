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

#include "lusan/view/common/ElementListView.hpp"

#include <QList>
#include <QStringList>

class QAction;

/**
 * \brief   The section-3 differences between the Service Interface and State Machine Methods
 *          list panels, parameterized so a single shared view serves both editors:
 *          - `groupTitle`      : the group box caption.
 *          - `typeMenuLabels`  : the Add split-button drop-down entries, one per method kind;
 *                                a plain Add click creates the first kind (the default).
 *          - `hasReplyColumn`  : append a fourth "Reply:" column (Service Interface).
 **/
struct MethodListConfig
{
    QString     groupTitle;
    QStringList typeMenuLabels;
    bool        hasReplyColumn;
};

/**
 * \brief   The shared methods list panel: the add/insert/delete toolbar of \ref ElementListView
 *          with its parameter trio and its move buttons, above a tree whose top-level rows are
 *          methods and whose child rows are the method parameters. The Add split button always
 *          adds a method; its drop-down offers the method kinds explicitly.
 *
 *          Columns are always Name / Method Type / Value, with an optional fourth Reply column.
 **/
class MethodListView : public ElementListView
{
    Q_OBJECT

public:
    /**
     * \brief   Column indexes of the method list.
     **/
    enum eColumn
    {
          ColName  = 0 //!< The method / parameter name.
        , ColType  = 1 //!< The method kind / parameter data type.
        , ColValue = 2 //!< The parameter default value (empty for methods).
        , ColReply = 3 //!< The method that answers this one (Service Interface only).
    };

    explicit MethodListView(const MethodListConfig& config, QWidget* parent = nullptr);

    //!< The Add drop-down action for the given method kind, or nullptr if out of range.
    QAction* typeAction(int index) const;

private:
    QList<QAction*> mTypeActions;    //!< The Add drop-down entries, one per method kind.
};

#endif  // LUSAN_VIEW_COMMON_METHODLISTVIEW_HPP
