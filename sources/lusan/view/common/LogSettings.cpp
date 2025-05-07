#include "lusan/view/common/LogSettings.hpp"
#include "lusan/common/LogCollectorClient.hpp"
#include "ui/ui_LogSettings.h"
#include "lusan/app/LusanApplication.hpp"
#include "lusan/view/common/WorkspaceManager.hpp"
#include "lusan/data/log/LogObserver.hpp"
#include "areg/component/ComponentThread.hpp"

#include <QAbstractItemView>
#include <QtAssert>
#include <QAbstractButton>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <string>

namespace
{
    char const * const logCollectorConfigFilePath = "./config/lusan.init";
}


LogSettings::LogSettings(QWidget *parent)
    : QWidget{parent}
    , ui{new Ui::LogSettingsForm}
    , mIpValidator{std::make_unique<QRegularExpressionValidator>(QRegularExpression{R"([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})"}, this)}
    , mPortValidator{std::make_unique<QRegularExpressionValidator>(QRegularExpression{"[0-9]{2,5}"}, this)}
    , mIsEndpointWorking{false}
{
    ui->setupUi(this);
    setupDialog();
    connectSignals();
}

LogSettings::~LogSettings()
{
    delete ui;
}

void LogSettings::setupDialog()
{
    ui->ipAddressEdit->setValidator(mIpValidator.get());
    ui->portNumberEdit->setValidator(mPortValidator.get());

    loadData();
    setFixedSize(size());
}

void LogSettings::connectSignals() const
{
    connect(ui->logDirBrowseBtn, &QAbstractButton::clicked, this, &LogSettings::logDirBrowseButtonClicked);
    connect(ui->testEndpointBtn, &QAbstractButton::clicked, this, &LogSettings::testEndpointButtonClicked);
    connect(ui->ipAddressEdit,   &QLineEdit::textChanged,   this, &LogSettings::endpointChanged);
    connect(ui->portNumberEdit,  &QLineEdit::textChanged,   this, &LogSettings::endpointChanged);
}

void LogSettings::logDirBrowseButtonClicked()
{
    ui->logDirEdit->setText(
        QFileDialog::getExistingDirectory(this, tr("Open Log Directory"), "", QFileDialog::ShowDirsOnly));
}

void LogSettings::applyChanges()
{
    if (!mIsEndpointWorking)
    {
        QMessageBox::critical(
            this, tr("Error"), tr("The endpoint must be tested and must be working before saving the changes!"));
        return;
    }

    saveData();
}

void LogSettings::loadData()
{
    {   // Load logging directory path
        WorkspaceEntry currentWorkspace{ LusanApplication::getActiveWorkspace() };

        ui->logDirEdit->setText(currentWorkspace.getDirLogs());
    }

    {   // Load logfile name
        // TODO: Implement code once known how to.
    }

    {   // Load endpoint data
        LogCollectorClient& lgClient = LogCollectorClient::getInstance();
        lgClient.initialize(logCollectorConfigFilePath);

        ui->ipAddressEdit->setText(QString::fromStdString(lgClient.getConfigLoggerAddress()));
        ui->portNumberEdit->setText(QString::fromStdString(std::to_string(lgClient.getLoggerPort())));
    }
}

void LogSettings::saveData() const
{
    {   // Save logging directory path
        WorkspaceEntry currentWorkspace{ LusanApplication::getActiveWorkspace() };
        currentWorkspace.setDirLogs(ui->logDirEdit->text());

        OptionsManager& optionsManager = LusanApplication::getOptions();
        optionsManager.updateWorkspace(currentWorkspace);
        optionsManager.writeOptions();
    }

    {   // Save logfile name
        // TODO: Implement code once known how to.
    }

    {   // Save endpoint data
        LogCollectorClient& lgClient = LogCollectorClient::getInstance();
        lgClient.initialize(logCollectorConfigFilePath);

        lgClient.setConfigLoggerAddress(ui->ipAddressEdit->text().toStdString());
        lgClient.setConfigLoggerPort(ui->portNumberEdit->text().toUInt());
        lgClient.saveLoggerConfig();
    }
}

void LogSettings::testEndpointButtonClicked()
{
    ComponentThread ct{"Test"};
    LogObserver::CreateComponent(NERegistry::ComponentEntry{}, ct);

    mIsEndpointWorking = LogObserver::connect(
        ui->ipAddressEdit->text(),
        ui->portNumberEdit->text().toUInt(),
        LogObserver::getInitDatabase());

    LogObserver::disconnect();
}

void LogSettings::endpointChanged()
{
    mIsEndpointWorking = false;
}
