#include "ProjectSettings.hpp"
#include "ui/ui_ProjectSettings.h"
#include "lusan/app/LusanApplication.hpp"

#include <QAbstractItemView>
#include <QtAssert>
#include <QAbstractButton>


ProjectSettings::ProjectSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProjectSettingsDlg)
{
    ui->setupUi(this);
    setupDialog();
    connectSignals();
}

ProjectSettings::~ProjectSettings()
{
    delete ui;
}

void ProjectSettings::setupDialog()
{
    addSettings();

    ui->horizontalLayout->setStretch(0, 1);
    ui->horizontalLayout->addWidget(settingsStackedWidget, 4);

    settingsStackedWidget->addWidget(mDirSettings);

    ui->settingsList->setModel(&model);

    selectSetting(0);

    setFixedSize(size());
}

void ProjectSettings::connectSignals() const
{
    connect(ui->settingsList, &QAbstractItemView::clicked, this, &ProjectSettings::settingsListSelectionChanged);

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &ProjectSettings::buttonClicked);
}

void ProjectSettings::settingsListSelectionChanged(QModelIndex const& index)
{
    selectSetting(index.row());
}

void ProjectSettings::selectSetting(int const index) const
{
    Q_ASSERT(index < settingsStackedWidget->count());

    settingsStackedWidget->setCurrentIndex(index);
}

void ProjectSettings::addSettings()
{
    QStringList settingsList;
    settingsList.append(tr("Directories"));
    model.setStringList(settingsList);
}

void ProjectSettings::buttonClicked(QAbstractButton* button) const
{
    QDialogButtonBox::ButtonRole const role = ui->buttonBox->buttonRole(button);

    if ((QDialogButtonBox::ButtonRole::AcceptRole != role) &&
        (QDialogButtonBox::ButtonRole::ApplyRole != role))
    {
        return;
    }

    WorkspaceEntry currentWorkspace{ LusanApplication::getActiveWorkspace() };

    currentWorkspace.setWorkspaceRoot(mDirSettings->getRootDirectory());
    currentWorkspace.setDirSources(mDirSettings->getSourceDirectory());
    currentWorkspace.setDirIncludes(mDirSettings->getIncludeDirectory());
    currentWorkspace.setDirDelivery(mDirSettings->getDeliveryDirectory());
    currentWorkspace.setDirLogs(mDirSettings->getLogDirectory());
    currentWorkspace.setWorkspaceDescription(mDirSettings->getWorkspaceDescription());


    OptionsManager& optionsManager = LusanApplication::getOptions();

    optionsManager.removeWorkspace(currentWorkspace.getKey());
    optionsManager.addWorkspace(currentWorkspace);
    optionsManager.writeOptions();
}
