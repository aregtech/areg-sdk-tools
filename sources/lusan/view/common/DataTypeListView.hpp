#ifndef LUSAN_VIEW_COMMON_DATATYPELISTVIEW_HPP
#define LUSAN_VIEW_COMMON_DATATYPELISTVIEW_HPP
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
 *  \file        lusan/view/common/DataTypeListView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared Data Types page -- data type list panel.
 *
 ************************************************************************/

#include "lusan/view/common/ElementListView.hpp"

class QAction;

/**
 * \brief   The data type list panel shared by the Service Interface (.siml), State Machine
 *          (.fsml) and Data Type (.dtml) documents: the add/insert/delete toolbar of
 *          \ref ElementListView with its field trio and its move buttons, above a 3-column tree
 *          (Name/Data Type/Default Value).
 **/
class DataTypeListView : public ElementListView
{
    Q_OBJECT

public:
    explicit DataTypeListView(QWidget* parent = nullptr);

    //!< The Add drop-down entries, one per data type category (default click = Structure).
    QAction* actionNewStruct() const;
    QAction* actionNewEnum() const;
    QAction* actionNewImport() const;
    QAction* actionNewContainer() const;

private:
    QAction*        mActNewStruct;      //!< Adds a structure.
    QAction*        mActNewEnum;        //!< Adds an enumeration.
    QAction*        mActNewImport;      //!< Adds an imported type.
    QAction*        mActNewContainer;   //!< Adds a container.
};

#endif  // LUSAN_VIEW_COMMON_DATATYPELISTVIEW_HPP
