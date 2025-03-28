#include "WorkspaceManager.hpp"
#include "ui/ui_WorkspaceManager.h"
#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/OptionsManager.hpp"
#include <algorithm>

WorkspaceManager::WorkspaceManager(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::workspaceManager)
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
    connect(ui->deleteButton, &QPushButton::clicked, this, &WorkspaceManager::handleDeleteButtonClicked);
    connect(ui->listOfWorkspaces, &QListWidget::itemSelectionChanged, this, &WorkspaceManager::handleWorkspaceSelectionChanged);
    connect(ui->workspaceEdit, &QPlainTextEdit::textChanged, this, &WorkspaceManager::handleWorkspaceDescChanged);
}

void WorkspaceManager::initialisePathsWithSelectedWorkspaceData(uint32_t const workspaceId) const
{
    std::optional<WorkspaceEntry> const workspace{ GetWorkspace(workspaceId) };
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

    ui->listOfWorkspaces->clear();

    for (WorkspaceEntry const& workspace : workspaces)
    {
        ui->listOfWorkspaces->addItem(QString::number(workspace.getId()));

        if (currentWorkspace.getId() == workspace.getId())
        {
            QBrush const grayBrush{ QColor(Qt::gray) };

            ui->listOfWorkspaces->item(ui->listOfWorkspaces->count() - 1)->setForeground(grayBrush);
        }
    }

    ui->listOfWorkspaces->sortItems();
}

void WorkspaceManager::handleDeleteButtonClicked()
{
    QListWidgetItem* selectedItem = ui->listOfWorkspaces->currentItem();
    if (!selectedItem)
        return;


    uint32_t const selectedWorkspaceId{ selectedItem->text().toUInt() };

    if (selectedWorkspaceId == LusanApplication::getActiveWorkspace().getId())
        return;

    mModifiedWorkspaces[selectedWorkspaceId] = WorkspaceChangeData{true, {}};

    deleteSelectedWorkspaceItem();
}

void WorkspaceManager::deleteSelectedWorkspaceItem() const
{
    disconnect(ui->workspaceEdit, &QPlainTextEdit::textChanged, this, &WorkspaceManager::handleWorkspaceDescChanged);

    delete ui->listOfWorkspaces->takeItem(ui->listOfWorkspaces->currentRow());

    connect(ui->workspaceEdit, &QPlainTextEdit::textChanged, this, &WorkspaceManager::handleWorkspaceDescChanged);
}

void WorkspaceManager::handleWorkspaceSelectionChanged() const
{
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return;

    ui->deleteButton->setDisabled(LusanApplication::getActiveWorkspace().getId() == *selectedItemId);

    initialisePathsWithSelectedWorkspaceData(*selectedItemId);
}

void WorkspaceManager::setupUi() const
{
    populateListOfWorkspaces();

    selectWorkspace(0);
}

void WorkspaceManager::selectWorkspace(int const index) const
{
    if (index >= ui->listOfWorkspaces->count())
        return;

    ui->listOfWorkspaces->setCurrentItem(ui->listOfWorkspaces->item(index));

    handleWorkspaceSelectionChanged();
}

void WorkspaceManager::applyChanges()
{
    if (mModifiedWorkspaces.empty())
        return;

    for (auto const& [id, data] : mModifiedWorkspaces)
    {
        std::optional<WorkspaceEntry> workspace{ GetWorkspace(id) };
        if (!workspace)
        {
            assert(false);
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

std::optional<WorkspaceEntry> WorkspaceManager::GetWorkspace(uint32_t const workspaceId) const
{
    std::vector<WorkspaceEntry> const& workspaces { LusanApplication::getOptions().getWorkspaceList() };

    auto workspacesIter{std::find_if(std::begin(workspaces), std::end(workspaces),
            [workspaceId](WorkspaceEntry const& we){ return we.getId() == workspaceId; }) };

    if (std::end(workspaces) == workspacesIter)
    {
        assert(false);
        return {};
    }

    return *workspacesIter;
}

void WorkspaceManager::handleWorkspaceDescChanged()
{
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return;

    mModifiedWorkspaces[*selectedItemId] = WorkspaceChangeData{false, ui->workspaceEdit->toPlainText()};
}

std::optional<uint32_t> WorkspaceManager::getSelectedWorkspaceId() const
{
    QListWidgetItem* selectedItem = ui->listOfWorkspaces->currentItem();
    if (!selectedItem)
        return {};

    return selectedItem->text().toUInt();
}
