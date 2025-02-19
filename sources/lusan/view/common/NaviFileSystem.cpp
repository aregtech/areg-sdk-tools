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
#include "lusan/view/common/MdiMainWindow.hpp"

#include <QMessageBox>
#include <QTreeView>
#include <QToolButton>

NaviFileSystem::NaviFileSystem(MdiMainWindow* mainFrame, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    
    , mMainFrame    (mainFrame)
    , mNaviModel    (new FileSystemModel())
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
    
    QStringList filters{ LusanApplication::InternalExts };
    filters.append(LusanApplication::ExternalExts);

    mNaviModel->setFileFilter(filters);
    ctrlFileSystem()->setModel(mNaviModel);
    QModelIndex idxRoot = mNaviModel->setRootPaths(rootPaths);
    ctrlFileSystem()->setRootIndex(idxRoot);
    ctrlFileSystem()->expand(idxRoot);
        
    ctrlFileSystem()->setSortingEnabled(true);
    this->setBaseSize(MIN_WIDTH, MIN_HEIGHT);
    this->setMinimumSize(MIN_WIDTH, MIN_HEIGHT);
    this->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    
    setupSignals();
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

void NaviFileSystem::onToolRefreshClicked(bool checked)
{
    QTreeView * table = ui->treeView;
    table->collapseAll();
    table->clearSelection();
    this->mNaviModel->refresh(table->rootIndex());
}

void NaviFileSystem::onToolShowAllToggled(bool checked)
{
    if (checked)
    {
        QTreeView * table = ui->treeView;
        table->collapseAll();
        table->clearSelection();
        mNaviModel->setFileFilter(QStringList());
        mNaviModel->refresh(table->rootIndex());
    }
    else
    {
        QTreeView * table = ui->treeView;
        table->collapseAll();
        table->clearSelection();
        QStringList filters{ LusanApplication::InternalExts };
        filters.append(LusanApplication::ExternalExts);
        mNaviModel->setFileFilter(filters);
        mNaviModel->refresh(table->rootIndex());
    }
}

void NaviFileSystem::onToolCollapseAllClicked(bool checked)
{
    ui->treeView->collapseAll();
}


void NaviFileSystem::onToolExpandAllClicked(bool checked)
{
    ui->treeView->expandAll();
}

void NaviFileSystem::onToolNewFolderClicked(bool checked)
{
    
}

void NaviFileSystem::onToolNewFileClicked(bool checked)
{
    
}

void NaviFileSystem::onToolOpenSelectedClicked(bool checked)
{
    QTreeView * table = ui->treeView;
    QModelIndex index = table->selectionModel()->currentIndex();
    QFileInfo fi = mNaviModel->getFileInfo(index);
    QString filePath = fi.isFile() ? fi.filePath() : "";
    if (filePath.isEmpty())
    {
        mMainFrame->openFile(filePath);
    }
}

void NaviFileSystem::onToolDeleteSelectedClicked(bool checked)
{
    QTreeView * table = ui->treeView;
    QModelIndex index = table->selectionModel()->currentIndex();
    QFileInfo fi = mNaviModel->getFileInfo(index);
    QString filePath = fi.filePath();
    if (filePath.isEmpty() == false)
    {
        int result = QMessageBox::question(   mMainFrame
                                            , tr("Delete File") + " - Lusan"
                                            , tr("Are you sure you want to delete ") + (fi.isDir() ? tr("directory") : tr("file")) + "\n" + filePath
                                            , QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel
                                            , QMessageBox::StandardButton::Cancel);
        if (result == QMessageBox::StandardButton::Ok)
        {
            mNaviModel->deleteEntry(index);
        }
    }
}

void NaviFileSystem::onTreeViewCollapsed(const QModelIndex &index)
{
}

void NaviFileSystem::onTreeViewExpanded(const QModelIndex &index)
{
}

void NaviFileSystem::onTreeViewDoubleClicked(const QModelIndex &index)
{
    if (index.isValid() == false)
        return;
    
    QFileInfo fi = mNaviModel->getFileInfo(index);
    QString filePath = fi.isDir() ? "" : fi.filePath();
    if (filePath.isEmpty() == false)
    {
        mMainFrame->openFile(filePath);
    }
}

void NaviFileSystem::onTreeViewCctivated(const QModelIndex &index)
{
    
}

void NaviFileSystem::updateData(void)
{
}

void NaviFileSystem::setupWidgets(void)
{
}

void NaviFileSystem::setupSignals(void)
{
    connect(ui->toolRefresh,        &QToolButton::clicked,      this, &NaviFileSystem::onToolRefreshClicked);
    connect(ui->toolShowAll,        &QToolButton::toggled,      this, &NaviFileSystem::onToolShowAllToggled);
    connect(ui->toolCollapseAll,    &QToolButton::clicked,      this, &NaviFileSystem::onToolCollapseAllClicked);
    connect(ui->toolExpandAll,      &QToolButton::clicked,      this, &NaviFileSystem::onToolExpandAllClicked);
    connect(ui->toolNewFolder,      &QToolButton::clicked,      this, &NaviFileSystem::onToolNewFolderClicked);
    connect(ui->toolNewFile,        &QToolButton::clicked,      this, &NaviFileSystem::onToolNewFileClicked);
    connect(ui->toolOpenSelected,   &QToolButton::clicked,      this, &NaviFileSystem::onToolOpenSelectedClicked);
    connect(ui->toolDeleteSelected, &QToolButton::clicked,      this, &NaviFileSystem::onToolDeleteSelectedClicked);
    connect(ui->treeView,           &QTreeView::collapsed,      this, &NaviFileSystem::onTreeViewCollapsed);
    connect(ui->treeView,           &QTreeView::expanded,       this, &NaviFileSystem::onTreeViewExpanded);
    connect(ui->treeView,           &QTreeView::doubleClicked,  this, &NaviFileSystem::onTreeViewDoubleClicked);
    connect(ui->treeView,           &QTreeView::activated,      this, &NaviFileSystem::onTreeViewCctivated);
}
