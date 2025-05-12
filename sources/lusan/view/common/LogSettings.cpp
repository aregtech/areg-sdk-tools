#include "lusan/view/common/LogSettings.hpp"
#include "ui/ui_LogSettings.h"
#include "lusan/common/LogCollectorClient.hpp"
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


LogSettings::LogSettings(QWidget *parent)
    : QWidget{parent}
    , mUi{std::make_unique<Ui::LogSettingsForm>()}
    , mIpValidator{std::make_unique<QRegularExpressionValidator>(QRegularExpression{R"([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})"}, this)}
    , mPortValidator{std::make_unique<QRegularExpressionValidator>(QRegularExpression{"[0-9]{2,5}"}, this)}
    , mIsEndpointWorking{false}
{
    mUi->setupUi(this);
    setupDialog();
    connectSignals();
}

LogSettings::~LogSettings()
{
}

void LogSettings::setupDialog()
{
    mUi->ipAddressEdit->setValidator(mIpValidator.get());
    mUi->portNumberEdit->setValidator(mPortValidator.get());

    loadData();
    setFixedSize(size());
}

void LogSettings::connectSignals() const
{
    connect(mUi->logDirBrowseBtn, &QAbstractButton::clicked, this, &LogSettings::logDirBrowseButtonClicked);
    connect(mUi->testEndpointBtn, &QAbstractButton::clicked, this, &LogSettings::testEndpointButtonClicked);
    connect(mUi->ipAddressEdit,   &QLineEdit::textChanged,   this, &LogSettings::endpointChanged);
    connect(mUi->portNumberEdit,  &QLineEdit::textChanged,   this, &LogSettings::endpointChanged);
}

void LogSettings::logDirBrowseButtonClicked()
{
    mUi->logDirEdit->setText(
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

        mUi->logDirEdit->setText(currentWorkspace.getDirLogs());
    }

    {
        LogCollectorClient& lgClient = LogCollectorClient::getInstance();
        lgClient.initialize();

        // Load logfile name
        mUi->logfileNameEdit->setText(QString::fromStdString(lgClient.getConfigLoggerDatabaseName()));

        // Load endpoint data
        mUi->ipAddressEdit->setText(QString::fromStdString(lgClient.getConfigLoggerAddress()));
        mUi->portNumberEdit->setText(QString::fromStdString(std::to_string(lgClient.getConfigLoggerPort())));
    }
}

void LogSettings::saveData() const
{
    {   // Save logging directory path
        WorkspaceEntry currentWorkspace{ LusanApplication::getActiveWorkspace() };
        currentWorkspace.setDirLogs(mUi->logDirEdit->text());

        OptionsManager& optionsManager = LusanApplication::getOptions();
        optionsManager.updateWorkspace(currentWorkspace);
        optionsManager.writeOptions();
    }

    {
        LogCollectorClient& lgClient = LogCollectorClient::getInstance();

        // Save logfile name
        lgClient.setConfigLoggerDatabaseName(mUi->logfileNameEdit->text().toStdString());

        // Save endpoint data
        lgClient.setConfigLoggerAddress(mUi->ipAddressEdit->text().toStdString());
        lgClient.setConfigLoggerPort(mUi->portNumberEdit->text().toUInt());
        lgClient.saveLoggerConfig();
    }
}

void LogSettings::testEndpointButtonClicked()
{
    ComponentThread testComponentThread{"Test ComponentThread"};
    LogObserver::CreateComponent(NERegistry::ComponentEntry{}, testComponentThread);

    LogObserver::connect(
        mUi->ipAddressEdit->text(),
        mUi->portNumberEdit->text().toUInt(),
        LogObserver::getInitDatabase());

    mIsEndpointWorking = LogObserver::isConnected();

    LogObserver::disconnect();
    testComponentThread.shutdownThread();
    testComponentThread.completionWait();
}

void LogSettings::endpointChanged()
{
    mIsEndpointWorking = false;
}
