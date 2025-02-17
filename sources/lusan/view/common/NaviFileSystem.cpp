/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/NaviFileSystem.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the workspace related file system.
 *
 ************************************************************************/

#include "lusan/view/common/NaviFileSystem.hpp"
#include "ui/ui_NaviFileSystem.h"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/model/common/FileSystemModel.hpp"

#include <QTreeView>
#include <QToolButton>

NaviFileSystem::NaviFileSystem(QWidget* parent /*= nullptr*/)
    : QWidget       (parent)

    , mNaviModel    (new FileSystemModel())
    // , mNaviFilter   (new FileSystemFilter(mNaviModel))
    , mListFilters  ()
    , ui            (new Ui::NaviFileSystem)
{
    ui->setupUi(this);
    
    QMap<QString, QString> rootPaths;
    QString root = LusanApplication::getWorkspaceRoot();
    QString sources = LusanApplication::getWorkspaceSources();
    QString includes = LusanApplication::getWorkspaceIncludes();
    QString delivery = LusanApplication::getWOrkspaceDelivery();

    Q_ASSERT(root.isEmpty() == false);
    rootPaths.insert(root, "[Project: " + root + "]");
    if (sources.isEmpty() == false)
    {
        rootPaths.insert(sources, "[Sources: " + sources + "]");
    }

    if (includes.isEmpty() == false)
    {
        rootPaths.insert(includes, "[Includes: " + includes + "]");
    }

    if (delivery.isEmpty() == false)
    {
        rootPaths.insert(delivery, "[Delivery: " + delivery + "]");
    }
    
    ctrlFileSystem()->setModel(mNaviModel);
    QModelIndex idxRoot = mNaviModel->setRootPaths(rootPaths);
    ctrlFileSystem()->setRootIndex(idxRoot);
    ctrlFileSystem()->expand(idxRoot);
        
    ctrlFileSystem()->setSortingEnabled(true);
    this->setBaseSize(MIN_WIDTH, MIN_HEIGHT);
    this->setMinimumSize(MIN_WIDTH, MIN_HEIGHT);
    this->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
}

QTreeView* NaviFileSystem::ctrlFileSystem(void) const
{
    return ui->treeView;
}

QToolButton* NaviFileSystem::ctrlToolRefresh(void) const
{
    return ui->toolRefresh;
}

QToolButton* NaviFileSystem::ctrlToolShowAll(void) const
{
    return ui->toolShowAll;
}

QToolButton* NaviFileSystem::ctrlToolCollapse(void) const
{
    return ui->toolCollapseAll;
}

QToolButton* NaviFileSystem::ctrlToolExpand(void) const
{
    return ui->toolExpandAll;
}

QToolButton* NaviFileSystem::ctrlToolNewFolder(void) const
{
    return ui->toolNewFolder;
}

QToolButton* NaviFileSystem::ctrlToolNewFile(void) const
{
    return ui->toolNewFile;
}

QToolButton* NaviFileSystem::ctrlToolOpen(void) const
{
    return ui->toolOpenSelected;
}

QToolButton* NaviFileSystem::ctrlToolDelete(void) const
{
    return ui->toolDeleteSelected;
}
