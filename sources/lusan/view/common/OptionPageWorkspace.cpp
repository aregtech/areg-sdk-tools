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
 *  \file        lusan/view/common/OptionPageWorkspace.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, workspace manager widget.
 *
 ************************************************************************/

#include "lusan/view/common/OptionPageWorkspace.hpp"
#include "ui/ui_OptionPageWorkspace.h"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/OptionsManager.hpp"

#include <QDialog>
#include <algorithm>

OptionPageWorkspace::OptionPageWorkspace(QDialog* parent)
    : OptionPageBase(parent)
    , mUi(std::make_unique<Ui::OptionPageWorkspace>())
    , mModifiedWorkspaces()
{
    mUi->setupUi(this);
    setupUi();
    connectSignalHandlers();
}

OptionPageWorkspace::~OptionPageWorkspace()
{
}

void OptionPageWorkspace::connectSignalHandlers() const
{
    connect(mUi->deleteButton    , &QPushButton::clicked             , this, &OptionPageWorkspace::onDeleteButtonClicked);
    connect(mUi->listOfWorkspaces, &QListWidget::itemSelectionChanged, this, &OptionPageWorkspace::onWorkspaceSelectionChanged);
    connect(mUi->workspaceEdit   , &QPlainTextEdit::textChanged      , this, &OptionPageWorkspace::onWorkspaceDescChanged);
}

void OptionPageWorkspace::initializePathsWithSelectedWorkspaceData(uint32_t const workspaceId) const
{
    std::optional<WorkspaceEntry> const workspace{ getWorkspace(workspaceId) };
    if (!workspace)
        return;

    mUi->rootDirEdit->setText(workspace->getWorkspaceRoot());
    mUi->sourceDirEdit->setText(workspace->getDirSources());
    mUi->includeDirEdit->setText(workspace->getDirIncludes());
    mUi->deliveryDirEdit->setText(workspace->getDirDelivery());
    mUi->logDirEdit->setText(workspace->getDirLogs());
    mUi->workspaceEdit->setPlainText(workspace->getWorkspaceDescription());
}

void OptionPageWorkspace::populateListOfWorkspaces() const
{
    WorkspaceEntry const currentWorkspace{ LusanApplication::getActiveWorkspace() };
    std::vector<WorkspaceEntry> const& workspaces { LusanApplication::getOptions().getWorkspaceList() };
    
    QListWidget* list = mUi->listOfWorkspaces;
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

    mUi->listOfWorkspaces->sortItems();
}

void OptionPageWorkspace::onDeleteButtonClicked()
{
    QListWidgetItem* selectedItem = mUi->listOfWorkspaces->currentItem();
    if (nullptr == selectedItem)
        return;

    uint32_t const selectedWorkspaceId{ selectedItem->data(Qt::ItemDataRole::UserRole).toUInt() };
    if (selectedWorkspaceId != LusanApplication::getActiveWorkspace().getId())
    {
        mModifiedWorkspaces[selectedWorkspaceId] = WorkspaceChangeData{ true, {} };
        deleteSelectedWorkspaceItem();
    }
}

void OptionPageWorkspace::deleteSelectedWorkspaceItem() const
{
    disconnect(mUi->workspaceEdit, &QPlainTextEdit::textChanged, this, &OptionPageWorkspace::onWorkspaceDescChanged);

    delete mUi->listOfWorkspaces->takeItem(mUi->listOfWorkspaces->currentRow());

    connect(mUi->workspaceEdit, &QPlainTextEdit::textChanged, this, &OptionPageWorkspace::onWorkspaceDescChanged);
}

void OptionPageWorkspace::onWorkspaceSelectionChanged() const
{
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return;

    mUi->deleteButton->setDisabled(LusanApplication::getActiveWorkspace().getId() == *selectedItemId);
    initializePathsWithSelectedWorkspaceData(*selectedItemId);
}

void OptionPageWorkspace::setupUi() const
{
    populateListOfWorkspaces();
    selectWorkspace(0);
}

void OptionPageWorkspace::selectWorkspace(int const index) const
{
    if (index < mUi->listOfWorkspaces->count())
    {
        mUi->listOfWorkspaces->setCurrentItem(mUi->listOfWorkspaces->item(index));
        onWorkspaceSelectionChanged();
    }
}

void OptionPageWorkspace::applyChanges()
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
    
    OptionPageBase::applyChanges();
}

std::optional<WorkspaceEntry> OptionPageWorkspace::getWorkspace(uint32_t const workspaceId)
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

void OptionPageWorkspace::onWorkspaceDescChanged()
{
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return;

    mModifiedWorkspaces[*selectedItemId] = WorkspaceChangeData{false, mUi->workspaceEdit->toPlainText()};
}

std::optional<uint32_t> OptionPageWorkspace::getSelectedWorkspaceId() const
{
    QListWidgetItem* selectedItem = mUi->listOfWorkspaces->currentItem();
    if (nullptr != selectedItem)
    {
        return selectedItem->data(Qt::ItemDataRole::UserRole).toUInt();
    }
    else
    {
        return std::nullopt;
    }
}
