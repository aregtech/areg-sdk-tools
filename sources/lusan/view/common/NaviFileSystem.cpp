﻿/************************************************************************
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
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/model/common/FileSystemFilter.hpp"
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
    , mGenModel     (nullptr)
    , mFileFilter   (nullptr)
    , ui            (new Ui::NaviFileSystem)
    , mRootPaths    ( )
    , mTableCell    (nullptr)
{
    ui->setupUi(this);
    this->setBaseSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
    this->setMinimumSize(NELusanCommon::MIN_NAVO_WIDTH, NELusanCommon::MIN_NAVI_HEIGHT);
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

QToolButton* NaviFileSystem::ctrlToolEdit(void) const
{
    return ui->toolEditSelected;
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
    return (mNaviModel != nullptr ? mNaviModel->getFileInfo(cell).fileName() : QString());
}

void NaviFileSystem::onToolRefreshClicked(bool checked)
{
    QTreeView * table = ctrlFileSystem();
    if (mNaviModel != nullptr)
    {
        table->collapseAll();
        table->clearSelection();
        mNaviModel->refresh();
        QModelIndex idxRoot = mNaviModel->getRootIndex();
        table->setRootIndex(idxRoot);
    }
    else if (mGenModel != nullptr)
    {
        table->collapseAll();
        table->clearSelection();
        table->reset();
    }
}

void NaviFileSystem::onToolShowAllToggled(bool checked)
{
    if (mNaviModel == nullptr)
        return;

    QTreeView * table = ctrlFileSystem();
    table->collapseAll();
    table->clearSelection();
    QStringList filters{ LusanApplication::InternalExts };
    filters.append(LusanApplication::ExternalExts);
    mNaviModel->setFileFilter(checked ? QStringList() : filters);
    mNaviModel->refresh();
    QModelIndex idxRoot = mNaviModel->getRootIndex();
    table->setRootIndex(idxRoot);
    ctrlToolShowAll()->setChecked(checked);
}

void NaviFileSystem::onToolCollapseAllClicked(bool checked)
{
    ctrlFileSystem()->collapseAll();
}

void NaviFileSystem::onToolNewFolderClicked(bool checked)
{
    static QString _defName("NewFolder");
    if (mNaviModel == nullptr)
        return;

    QTreeView* table = ctrlFileSystem();
    QModelIndex index = table->selectionModel()->currentIndex();
    if (mNaviModel->isFile(index))
        index = mNaviModel->parent(index);

    uint32_t count{ 1 };
    QString name;
    do
    {
        name = _defName + QString::number(count ++);
    } while(mNaviModel->existsDirectory(index, name));
    
    QModelIndex newIndex = mNaviModel->insertDirectory(name, index);
    if (newIndex.isValid())
    {
        table->setCurrentIndex(newIndex);
        table->edit(newIndex);
    }
}

void NaviFileSystem::onToolNewFileClicked(bool checked)
{
    static QString _defName("NewService");
    static QString _defExt(".siml");

    if (mNaviModel == nullptr)
        return;

    QTreeView* table = ctrlFileSystem();
    QModelIndex index = table->currentIndex();
    if (mNaviModel->isFile(index))
        index = mNaviModel->parent(index);

    uint32_t count{ 1 };
    QString name;
    do
    {
        name = _defName + QString::number(count ++) + _defExt;
    } while(mNaviModel->existsDirectory(index, name));
    
    QModelIndex newIndex = mNaviModel->insertFile(name, index);
    if (newIndex.isValid())
    {
        table->setCurrentIndex(newIndex);
        table->edit(newIndex);
    }
}

void NaviFileSystem::onToolOpenSelectedClicked(bool checked)
{
    QTreeView * table = ctrlFileSystem();
    QModelIndex index = table->selectionModel()->currentIndex();
    QFileInfo fi (getFileInfo(index));

    QString filePath = fi.isFile() ? fi.filePath() : "";
    if (filePath.isEmpty() == false)
    {
        mMainFrame->openFile(filePath);
    }
}

void NaviFileSystem::onToolEditSelectedClicked(bool checked)
{
    QTreeView * table = ctrlFileSystem();
    QModelIndex index = table->selectionModel()->currentIndex();
    if (index.isValid())
    {
        table->edit(index);
    }
}

void NaviFileSystem::onToolDeleteSelectedClicked(bool checked)
{
    QTreeView * table = ctrlFileSystem();
    QModelIndex index = table->selectionModel()->currentIndex();
    QFileInfo fi = mNaviModel->getFileInfo(index);
    QString filePath = fi.filePath();
    if (filePath.isEmpty() == false)
    {
        QModelIndex parent = index.parent();
        int result = QMessageBox::question(   mMainFrame
                                            , tr("Delete File") + " - Lusan"
                                            , tr("Are you sure you want to delete ") + (fi.isDir() ? tr("directory") : tr("file")) + "\n" + filePath
                                            , QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel
                                            , QMessageBox::StandardButton::Cancel);
        
        if ((result == QMessageBox::StandardButton::Ok) && mNaviModel->deleteEntry(index))
        {
            Q_ASSERT(parent.isValid());
            int rowCount = mNaviModel->rowCount(parent);
            if (rowCount == 0)
            {
                index = parent;
            }
            else if (index.row() >= rowCount)
            {
                index = mNaviModel->index(rowCount - 1, 0, parent);
            }
            else
            {
                index  = mNaviModel->index(index.row(), 0, parent);
            }
            
            table->setCurrentIndex(index);
        }
    }
}

void NaviFileSystem::onToolNaviRootClicked(bool checked)
{
    if ((checked == true) && (mGenModel == nullptr))
    {
        disconnect(ctrlFileSystem()->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &NaviFileSystem::onTreeSelectinoRowChanged);
        mGenModel = new GeneralFileSystemModel();
        mFileFilter = new FileSystemFilter(mGenModel);
        mGenModel->setReadOnly(true);
        ctrlFileSystem()->setModel(mFileFilter);
        ctrlFileSystem()->setSortingEnabled(true);
        ctrlFileSystem()->reset();
        // ctrlFileSystem()->update();
        delete mNaviModel;
        mNaviModel = nullptr;
        // QString rootPath = QDir::rootPath();
        QString rootPath = mGenModel->myComputer().toString();
        QModelIndex idxRoot = mGenModel->setRootPath(rootPath);
        ctrlFileSystem()->setRootIndex(mFileFilter->mapFromSource(idxRoot));

        ctrlToolDelete()->setEnabled(false);
        ctrlToolNewFile()->setEnabled(false);
        ctrlToolNewFolder()->setEnabled(false);
        ctrlToolOpen()->setEnabled(true);
        ctrlToolEdit()->setEnabled(false);
    }
    else if ((checked == false) && (mNaviModel == nullptr))
    {
        mNaviModel = new FileSystemModel();
        this->updateData();
        this->setupWidgets();

        mFileFilter->setSourceModel(nullptr);
        delete mFileFilter;
        delete mGenModel;

        mFileFilter = nullptr;
        mGenModel = nullptr;

        connect(ctrlFileSystem()->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &NaviFileSystem::onTreeSelectinoRowChanged);
    }
}

void NaviFileSystem::onTreeViewDoubleClicked(const QModelIndex &index)
{
    if (index.isValid() == false)
        return;

    QFileInfo fi (getFileInfo(index));
    QString filePath = fi.isDir() ? "" : fi.filePath();
    if (filePath.isEmpty() == false)
    {
        mMainFrame->openFile(filePath);
    }
}

void NaviFileSystem::onTreeViewActivated(const QModelIndex &index)
{
    bool enable = (mNaviModel != nullptr) && (mNaviModel->isRoot(index) == false) && index.isValid();
    ctrlToolDelete()->setEnabled(enable && (mNaviModel->isWorkspaceEntry(index) == false));
    ctrlToolNewFile()->setEnabled(enable);
    ctrlToolNewFolder()->setEnabled(enable);
    ctrlToolOpen()->setEnabled(enable && mNaviModel->isFile(index));
    ctrlToolEdit()->setEnabled(enable && (mNaviModel->isWorkspaceEntry(index) == false));
}

void NaviFileSystem::onTreeSelectinoRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    onTreeViewActivated(current);
}

void NaviFileSystem::onEditorDataChanged(const QModelIndex& index, const QString& newValue)
{
    if ((index.isValid() == false) || (mNaviModel == nullptr))
        return;

    QTreeView * table = ctrlFileSystem();
    QModelIndex newIndex = mNaviModel->renameEntry(newValue, index);
    if (newIndex.isValid())
    {
        table->setCurrentIndex(newIndex);
    }
}

void NaviFileSystem::updateData(void)
{
    QString root = LusanApplication::getWorkspaceRoot();
    QString sources = LusanApplication::getWorkspaceSources();
    QString includes = LusanApplication::getWorkspaceIncludes();
    QString delivery = LusanApplication::getWorkspaceDelivery();

    mRootPaths.clear();
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
    
    mTableCell = new TableCell(ctrlFileSystem(), this, true);
    ctrlFileSystem()->setItemDelegateForColumn(0, mTableCell);
    
    ctrlToolDelete()->setEnabled(false);
    ctrlToolNewFile()->setEnabled(false);
    ctrlToolNewFolder()->setEnabled(false);
    ctrlToolOpen()->setEnabled(false);
    ctrlToolEdit()->setEnabled(false);
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
    connect(ctrlFileSystem(),       &QTreeView::doubleClicked,  this, &NaviFileSystem::onTreeViewDoubleClicked);
    connect(ctrlFileSystem(),       &QTreeView::entered,        this, &NaviFileSystem::onTreeViewActivated);
    connect(ctrlFileSystem()->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &NaviFileSystem::onTreeSelectinoRowChanged);

    connect(mTableCell, &TableCell::editorDataChanged, this, &NaviFileSystem::onEditorDataChanged);
}

void NaviFileSystem::blockBasicSignals(bool block)
{
    ctrlFileSystem()->blockSignals(block);
}

QFileInfo NaviFileSystem::getFileInfo(const QModelIndex & index) const
{
    if (mNaviModel != nullptr)
    {
        return mNaviModel->getFileInfo(index);
    }
    else
    {
        Q_ASSERT(mGenModel != nullptr);
        Q_ASSERT(mFileFilter != nullptr);
        QModelIndex src = mFileFilter->mapToSource(index);
        return mGenModel->fileInfo(src);
    }
}
