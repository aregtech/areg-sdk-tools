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
 *  \file        lusan/view/common/DataTypeListView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared Data Types page -- data type list panel implementation.
 *
 ************************************************************************/

#include "lusan/view/common/DataTypeListView.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QAction>
#include <QMenu>
#include <QStringList>
#include <QToolButton>

DataTypeListView::DataTypeListView(QWidget* parent /*= nullptr*/)
    : ElementListView( ElementListConfig{ tr("Data Types List:")
                                        , QStringList{ tr("Name:"), tr("Data Type:"), tr("Default Value:") }
                                        , tr("data type")
                                        , tr("field")
                                        , false }
                     , parent)
    , mActNewStruct    (nullptr)
    , mActNewEnum      (nullptr)
    , mActNewImport    (nullptr)
    , mActNewContainer (nullptr)
{
    ctrlButtonAdd()->setToolTip(tr("Create and add new data type entry (a structure by default)"));

    // The Add split button: a plain click creates the default (a structure), the drop-down
    // offers the four data type categories explicitly.
    mActNewStruct    = new QAction(NELusanCommon::loadIcon(QStringLiteral(":/icons/data type structure"), NELusanCommon::SizeSmall), tr("Structure")  , this);
    mActNewEnum      = new QAction(NELusanCommon::loadIcon(QStringLiteral(":/icons/data type enum"), NELusanCommon::SizeSmall)     , tr("Enumeration"), this);
    mActNewImport    = new QAction(NELusanCommon::loadIcon(QStringLiteral(":/icons/data type import"), NELusanCommon::SizeSmall)   , tr("Imported")   , this);
    mActNewContainer = new QAction(NELusanCommon::loadIcon(QStringLiteral(":/icons/data type container"), NELusanCommon::SizeSmall), tr("Container")  , this);

    QMenu* addMenu = new QMenu(ctrlButtonAdd());
    addMenu->addAction(mActNewStruct);
    addMenu->addAction(mActNewEnum);
    addMenu->addAction(mActNewImport);
    addMenu->addAction(mActNewContainer);
    NELusanCommon::decorateToolButton(ctrlButtonAdd(), addMenu);
}

QAction* DataTypeListView::actionNewStruct() const
{
    return mActNewStruct;
}

QAction* DataTypeListView::actionNewEnum() const
{
    return mActNewEnum;
}

QAction* DataTypeListView::actionNewImport() const
{
    return mActNewImport;
}

QAction* DataTypeListView::actionNewContainer() const
{
    return mActNewContainer;
}
