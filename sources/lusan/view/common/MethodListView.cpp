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
 *  \file        lusan/view/common/MethodListView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "method list" panel implementation.
 *
 ************************************************************************/

#include "lusan/view/common/MethodListView.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QAction>
#include <QMenu>
#include <QToolButton>
#include <QTreeWidget>

namespace
{
    ElementListConfig listConfigOf(const MethodListConfig& config)
    {
        QStringList headers{ QObject::tr("Name:"), QObject::tr("Method Type:"), QObject::tr("Value:") };
        if (config.hasReplyColumn)
        {
            headers.append(QObject::tr("Reply:"));
        }

        return ElementListConfig{ config.groupTitle, headers, QObject::tr("method"), QObject::tr("parameter"), false };
    }
}

MethodListView::MethodListView(const MethodListConfig& config, QWidget* parent /*= nullptr*/)
    : ElementListView (listConfigOf(config), parent)
    , mTypeActions    ( )
{
    // The Add split button: a plain click creates the default (the first kind), the drop-down
    // offers the method kinds explicitly.
    QMenu* addMenu = new QMenu(ctrlButtonAdd());
    for (const QString& label : config.typeMenuLabels)
    {
        QAction* action = new QAction(label, this);
        addMenu->addAction(action);
        mTypeActions.append(action);
    }

    NELusanCommon::decorateToolButton(ctrlButtonAdd(), addMenu);
}

QAction* MethodListView::typeAction(int index) const
{
    return ((index >= 0) && (index < mTypeActions.size())) ? mTypeActions.at(index) : nullptr;
}
