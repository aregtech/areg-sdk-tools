#include "ProjectSettings.hpp"
#include "ui/ui_ProjectSettings.h"
#include "ProjectDirSettings.hpp"
#include <QAbstractItemView>
#include <QtAssert>


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
    ui->horizontalLayout->addWidget(&settingsStackedWidget, 4);

    settingsStackedWidget.addWidget(new ProjectDirSettings(this));
    ui->settingsList->setModel(&model);

    selectSetting(0);

    setFixedSize(size());
}

void ProjectSettings::connectSignals() const
{
    connect(ui->settingsList, &QAbstractItemView::clicked, this, &ProjectSettings::settingsListSelectionChanged);
}

void ProjectSettings::settingsListSelectionChanged(QModelIndex const& index)
{
    selectSetting(index.row());
}

void ProjectSettings::selectSetting(int const index)
{
    Q_ASSERT(index < settingsStackedWidget.count());

    settingsStackedWidget.setCurrentIndex(index);
}

void ProjectSettings::addSettings()
{
    QStringList settingsList;
    settingsList.append(tr("Directories"));
    model.setStringList(settingsList);
}
