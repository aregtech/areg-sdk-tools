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
 *  \file        lusan/view/common/WorkspaceManager.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, workspace manager widget.
 *
 ************************************************************************/

#include "lusan/view/common/WorkspaceManager.hpp"
#include "ui/ui_WorkspaceManager.h"
#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/OptionsManager.hpp"
#include <algorithm>

WorkspaceManager::WorkspaceManager(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::workspaceManager)
    , mModifiedWorkspaces()
{
    ui->setupUi(this);
    setupUi();
    connectSignalHandlers();
}

WorkspaceManager::~WorkspaceManager()
{
    delete ui;
}

void WorkspaceManager::connectSignalHandlers() const
{
    connect(ui->deleteButton    , &QPushButton::clicked             , this, &WorkspaceManager::onDeleteButtonClicked);
    connect(ui->listOfWorkspaces, &QListWidget::itemSelectionChanged, this, &WorkspaceManager::onWorkspaceSelectionChanged);
    connect(ui->workspaceEdit   , &QPlainTextEdit::textChanged      , this, &WorkspaceManager::onWorkspaceDescChanged);
}

void WorkspaceManager::initializePathsWithSelectedWorkspaceData(uint32_t const workspaceId) const
{
    std::optional<WorkspaceEntry> const workspace{ getWorkspace(workspaceId) };
    if (!workspace)
        return;

    ui->rootDirEdit->setText(workspace->getWorkspaceRoot());
    ui->sourceDirEdit->setText(workspace->getDirSources());
    ui->includeDirEdit->setText(workspace->getDirIncludes());
    ui->deliveryDirEdit->setText(workspace->getDirDelivery());
    ui->logDirEdit->setText(workspace->getDirLogs());
    ui->workspaceEdit->setPlainText(workspace->getWorkspaceDescription());
}

void WorkspaceManager::populateListOfWorkspaces() const
{
    WorkspaceEntry const currentWorkspace{ LusanApplication::getActiveWorkspace() };
    std::vector<WorkspaceEntry> const& workspaces { LusanApplication::getOptions().getWorkspaceList() };
    
    QListWidget* list = ui->listOfWorkspaces;
    list->clear();

    for (WorkspaceEntry const& workspace : workspaces)
    {
        uint32_t wsId = workspace.getId();
        QString text(QString::number(wsId));
        text += " : " + workspace.getWorkspaceRoot();

        QListWidgetItem* item = new QListWidgetItem(QIcon::fromTheme(QIcon::ThemeIcon::FolderOpen), text, list);
        item->setData(Qt::ItemDataRole::UserRole, wsId);

        if (currentWorkspace.getId() == wsId)
        {
            QBrush const grayBrush{ QColor(Qt::gray) };
            item->setForeground(grayBrush);
        }
        
        list->addItem(item);
    }

    ui->listOfWorkspaces->sortItems();
}

void WorkspaceManager::onDeleteButtonClicked()
{
    QListWidgetItem* selectedItem = ui->listOfWorkspaces->currentItem();
    if (nullptr == selectedItem)
        return;

    uint32_t const selectedWorkspaceId{ selectedItem->data(Qt::ItemDataRole::UserRole).toUInt() };
    if (selectedWorkspaceId != LusanApplication::getActiveWorkspace().getId())
    {
        mModifiedWorkspaces[selectedWorkspaceId] = WorkspaceChangeData{ true, {} };
        deleteSelectedWorkspaceItem();
    }
}

void WorkspaceManager::deleteSelectedWorkspaceItem() const
{
    disconnect(ui->workspaceEdit, &QPlainTextEdit::textChanged, this, &WorkspaceManager::onWorkspaceDescChanged);

    delete ui->listOfWorkspaces->takeItem(ui->listOfWorkspaces->currentRow());

    connect(ui->workspaceEdit, &QPlainTextEdit::textChanged, this, &WorkspaceManager::onWorkspaceDescChanged);
}

void WorkspaceManager::onWorkspaceSelectionChanged() const
{
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return;

    ui->deleteButton->setDisabled(LusanApplication::getActiveWorkspace().getId() == *selectedItemId);
    initializePathsWithSelectedWorkspaceData(*selectedItemId);
}

void WorkspaceManager::setupUi() const
{
    populateListOfWorkspaces();
    selectWorkspace(0);
}

void WorkspaceManager::selectWorkspace(int const index) const
{
    if (index < ui->listOfWorkspaces->count())
    {
        ui->listOfWorkspaces->setCurrentItem(ui->listOfWorkspaces->item(index));
        onWorkspaceSelectionChanged();
    }
}

void WorkspaceManager::applyChanges()
{
    if (mModifiedWorkspaces.empty())
        return;

    for (auto const& [id, data] : mModifiedWorkspaces)
    {
        std::optional<WorkspaceEntry> workspace{ getWorkspace(id) };
        if (!workspace)
        {
            Q_ASSERT(false);
            continue;
        }

        if (data.hasDeleted)
        {
            LusanApplication::getOptions().removeWorkspace(workspace->getKey());
        }
        else if (data.newDescription)
        {
            workspace->setWorkspaceDescription(*data.newDescription);
            LusanApplication::getOptions().updateWorkspace(*workspace);
        }
    }

    mModifiedWorkspaces.clear();
    LusanApplication::getOptions().writeOptions();
}

std::optional<WorkspaceEntry> WorkspaceManager::getWorkspace(uint32_t const workspaceId)
{
    std::vector<WorkspaceEntry> const& workspaces { LusanApplication::getOptions().getWorkspaceList() };

    auto workspacesIter{std::find_if(std::begin(workspaces), std::end(workspaces),
            [workspaceId](WorkspaceEntry const& we){ return we.getId() == workspaceId; }) };

    if (std::end(workspaces) != workspacesIter)
    {
        return *workspacesIter;
    }
    else
    {
        Q_ASSERT(false);
        return std::nullopt;
    }
}

void WorkspaceManager::onWorkspaceDescChanged()
{
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return;

    mModifiedWorkspaces[*selectedItemId] = WorkspaceChangeData{false, ui->workspaceEdit->toPlainText()};
}

std::optional<uint32_t> WorkspaceManager::getSelectedWorkspaceId() const
{
    QListWidgetItem* selectedItem = ui->listOfWorkspaces->currentItem();
    if (nullptr != selectedItem)
    {
        return selectedItem->data(Qt::ItemDataRole::UserRole).toUInt();
    }
    else
    {
        return std::nullopt;
    }
}
