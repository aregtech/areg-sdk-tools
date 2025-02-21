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
#include "lusan/view/common/TableCell.hpp"

#include <QMessageBox>
#include <QTreeView>
#include <QToolButton>

NaviFileSystem::NaviFileSystem(MdiMainWindow* mainFrame, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , IETableHelper ()
    
    , mMainFrame    (mainFrame)
    , mNaviModel    (new FileSystemModel())
    , ui            (new Ui::NaviFileSystem)
    , mRootPaths    ( )
    , mTableCell    (nullptr)
{
    ui->setupUi(this);
    this->setBaseSize(MIN_WIDTH, MIN_HEIGHT);
    this->setMinimumSize(MIN_WIDTH, MIN_HEIGHT);
    this->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    updateData();
    setupWidgets();    
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

int NaviFileSystem::getColumnCount(void) const
{
    return 1;
}

QString NaviFileSystem::getCellText(const QModelIndex& cell) const
{
    return mNaviModel->getFileInfo(cell).fileName();
}

void NaviFileSystem::onToolRefreshClicked(bool checked)
{
    QTreeView * table = ui->treeView;
    table->collapseAll();
    table->clearSelection();
    this->mNaviModel->refresh();
    QModelIndex idxRoot = mNaviModel->getRootIndex();
    table->setRootIndex(idxRoot);
}

void NaviFileSystem::onToolShowAllToggled(bool checked)
{
    if (checked)
    {
        QTreeView * table = ui->treeView;
        table->collapseAll();
        table->clearSelection();
        mNaviModel->setFileFilter(QStringList());
        mNaviModel->refresh();
        QModelIndex idxRoot = mNaviModel->getRootIndex();
        table->setRootIndex(idxRoot);
        ctrlToolShowAll()->setChecked(true);
    }
    else
    {
        QTreeView * table = ui->treeView;
        table->collapseAll();
        table->clearSelection();
        QStringList filters{ LusanApplication::InternalExts };
        filters.append(LusanApplication::ExternalExts);
        mNaviModel->setFileFilter(filters);
        mNaviModel->refresh();
        QModelIndex idxRoot = mNaviModel->getRootIndex();
        table->setRootIndex(idxRoot);
        ctrlToolShowAll()->setChecked(false);
    }
}

void NaviFileSystem::onToolCollapseAllClicked(bool checked)
{
    ui->treeView->collapseAll();
}

void NaviFileSystem::onToolNewFolderClicked(bool checked)
{
    QTreeView* table = ui->treeView;
    QModelIndex index = table->selectionModel()->currentIndex();
    QFileInfo fi = mNaviModel->getFileInfo(index);
    QString folderPath = fi.isDir() ? fi.filePath() : fi.dir().path();
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
    if (filePath.isEmpty() == false)
    {
        mMainFrame->openFile(filePath);
    }
}

void NaviFileSystem::onToolEditSelectedClicked(bool checked)
{
    QTreeView * table = ui->treeView;
    QModelIndex index = table->selectionModel()->currentIndex();
    if (index.isValid())
    {
        table->edit(index);
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

void NaviFileSystem::onToolNaviRootClicked(bool checked)
{
    
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

void NaviFileSystem::onTreeViewActivated(const QModelIndex &index)
{
    
}

void NaviFileSystem::onEditorDataChanged(const QModelIndex& index, const QString& newValue)
{
    if (index.isValid() == false)
        return;

    QTreeView * table = ui->treeView;
    // cellChanged(index.row(), index.column(), newValue);
}

void NaviFileSystem::updateData(void)
{
    QString root = LusanApplication::getWorkspaceRoot();
    QString sources = LusanApplication::getWorkspaceSources();
    QString includes = LusanApplication::getWorkspaceIncludes();
    QString delivery = LusanApplication::getWOrkspaceDelivery();

    Q_ASSERT(root.isEmpty() == false);
    mRootPaths.insert(root, "[Project: " + root + "]");
    if (sources.isEmpty() == false)
    {
        mRootPaths.insert(sources, "[Sources: " + sources + "]");
    }

    if (includes.isEmpty() == false)
    {
        mRootPaths.insert(includes, "[Includes: " + includes + "]");
    }

    if (delivery.isEmpty() == false)
    {
        mRootPaths.insert(delivery, "[Delivery: " + delivery + "]");
    }

    QStringList filters{ LusanApplication::InternalExts };
    filters.append(LusanApplication::ExternalExts);

    mNaviModel->setFileFilter(filters);
}

void NaviFileSystem::setupWidgets(void)
{
    ctrlFileSystem()->setModel(mNaviModel);
    QModelIndex idxRoot = mNaviModel->setRootPaths(mRootPaths);
    ctrlFileSystem()->setRootIndex(idxRoot);
    ctrlFileSystem()->expand(idxRoot);
    ctrlFileSystem()->setSortingEnabled(true);
    ctrlToolShowAll()->setCheckable(true);

    mTableCell = new TableCell(ctrlFileSystem(), this);
    ctrlFileSystem()->setItemDelegateForColumn(0, mTableCell);
}

void NaviFileSystem::setupSignals(void)
{
    connect(ui->toolRefresh,        &QToolButton::clicked,      this, &NaviFileSystem::onToolRefreshClicked);
    connect(ui->toolShowAll,        &QToolButton::toggled,      this, &NaviFileSystem::onToolShowAllToggled);
    connect(ui->toolCollapseAll,    &QToolButton::clicked,      this, &NaviFileSystem::onToolCollapseAllClicked);
    connect(ui->toolNewFolder,      &QToolButton::clicked,      this, &NaviFileSystem::onToolNewFolderClicked);
    connect(ui->toolNewFile,        &QToolButton::clicked,      this, &NaviFileSystem::onToolNewFileClicked);
    connect(ui->toolEditSelected,   &QToolButton::clicked,      this, &NaviFileSystem::onToolEditSelectedClicked);
    connect(ui->toolOpenSelected,   &QToolButton::clicked,      this, &NaviFileSystem::onToolOpenSelectedClicked);
    connect(ui->toolDeleteSelected, &QToolButton::clicked,      this, &NaviFileSystem::onToolDeleteSelectedClicked);
    connect(ui->toolNaviRoot,       &QToolButton::clicked,      this, &NaviFileSystem::onToolNaviRootClicked);
    connect(ui->treeView,           &QTreeView::collapsed,      this, &NaviFileSystem::onTreeViewCollapsed);
    connect(ui->treeView,           &QTreeView::expanded,       this, &NaviFileSystem::onTreeViewExpanded);
    connect(ui->treeView,           &QTreeView::doubleClicked,  this, &NaviFileSystem::onTreeViewDoubleClicked);
    connect(ui->treeView,           &QTreeView::activated,      this, &NaviFileSystem::onTreeViewActivated);

    connect(mTableCell, &TableCell::editorDataChanged, this, &NaviFileSystem::onEditorDataChanged);
}

