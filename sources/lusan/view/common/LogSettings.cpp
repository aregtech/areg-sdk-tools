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
 *  \file        lusan/view/common/ProjectSettings.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag, Artak Avetyan
 *  \brief       Lusan application, implementation of Log Settings page of options dialog.
 *
 ************************************************************************/

#include "lusan/view/common/LogSettings.hpp"
#include "ui/ui_LogSettings.h"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/LogCollectorClient.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/log/LogObserver.hpp"
#include "lusan/view/common/WorkspaceManager.hpp"
#include "lusan/view/common/ProjectSettings.hpp"

#include "areg/base/NESocket.hpp"

#include <QAbstractItemView>
#include <QtAssert>
#include <QAbstractButton>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <string>

const QString   LogSettings::_textNoChanges         { tr("No data changed yet ...") };
const QString   LogSettings::_textDataChanged       { tr("WARNING: Test the Log Collector Service connection before saving changes ...") };
const QString   LogSettings::_textTestInProgress    { tr("WAITING: Test connection is in progress, make sure the Log Collector Service is configured and runs ...") };
const QString   LogSettings::_textTestInterrupted   { tr("WARNING: The Log Collector Service connection data is updated, interrupting ongoing connection ...") };
const QString   LogSettings::_textServiceConnected  { tr("RESULT: Connected to the Log Collector Service at %1:%2, waiting for messaging ...") };
const QString   LogSettings::_textTestSucceeded     { tr("SUCCESS: Succeeded the Log Collector Service connection test, currently there are %1 connected log sources instances.") };
const QString   LogSettings::_textConnectionFailed  { tr("ERROR: Failed to trigger connection to the Log Collector Service, check network connection and retry.") };
const QString   LogSettings::_textTestFailed        { tr("FAILURE: Failed to connect to the Log Collector Service. Check connection data and try again.") };
const QString   LogSettings::_textTestCanceled      { tr("WARNING: Connection to the Log Collector Service was interrupted") };

LogSettings::LogSettings(ProjectSettings* parent)
    : QWidget       {parent}
    , ui            {std::make_unique<Ui::LogSettingsForm>()}
    , mPortValidator{QRegularExpression("[0-9]{2,5}"), this}
    , mTestTriggered{false}
    , mCanSave      {false}
    , mAddress      {}
    , mPort         {NESocket::InvalidPort}
    , mLogFileName  {}
    , mLogLocation  {}
    , mTestConnect  {}
    , mTestMessage  {}
{
    ui->setupUi(this);
    setupDialog();
    connectSignals();
    
    setWindowTitle(tr("Log Settings"));
}

LogSettings::LogSettings(ProjectSettings *parent, const QString& address, uint16_t port, const QString &logFile, const QString &logLocation)
    : QWidget       {parent}
    , ui            {std::make_unique<Ui::LogSettingsForm>()}
    , mPortValidator{QRegularExpression("[0-9]{2,5}"), this}
    , mTestTriggered{false}
    , mCanSave      {false}
    , mAddress      {address}
    , mPort         {port}
    , mLogFileName  {logFile}
    , mLogLocation  {logLocation}
{
    ui->setupUi(this);
    setupDialog();
    connectSignals();
    
    setWindowTitle(tr("Log Settings"));
}

LogSettings::~LogSettings()
{
    if (mTestTriggered)
    {
        LogObserver::disconnect();
        LogObserver::releaseLogObserver();
    }
}

void LogSettings::setupDialog()
{
    LogCollectorClient& client = LogCollectorClient::getInstance();
    if (client.isInitialized() == false)
    {
        client.initialize(NELusanCommon::INIT_FILE.toStdString());
    }

    // Load logging directory path
    WorkspaceEntry currentWorkspace{ LusanApplication::getActiveWorkspace() };
    QString logLocation { currentWorkspace.getDirLogs() };
    if (logLocation.isEmpty())
    {
        logLocation = mLogLocation.isEmpty() ? client.getConfigLoggerDatabaseLocation().c_str() : mLogLocation;
    }
    
    QString logFile = mLogFileName.isEmpty() ? client.getConfigLoggerDatabaseName().c_str() : mLogFileName;
    QString address = mAddress.isEmpty() ? client.getConfigLoggerAddress().c_str() : mAddress;
    uint16_t port   = mPort == NESocket::InvalidPort ? client.getConfigLoggerPort() : mPort;
    
    textPortNumber()->setValidator(&mPortValidator);
    textLogLocation()->setText(logLocation);
    textLogFileName()->setText(logFile);
    textIpAddress()->setText(address);
    textPortNumber()->setText(QString::number(port));
    textConnectionStatus()->setTextColor(QColor(Qt::gray));
    textConnectionStatus()->setText(_textNoChanges);
    
    setFixedSize(size());
}

void LogSettings::connectSignals() const
{
    connect(buttonBrowseDirs()      , &QAbstractButton::clicked , this, &LogSettings::onBrowseButtonClicked);
    connect(buttonTestConnection()  , &QAbstractButton::clicked , this, &LogSettings::onTestButtonClicked);
    connect(textIpAddress()         , &QLineEdit::textChanged   , this, &LogSettings::onDataChanged);
    connect(textPortNumber()        , &QLineEdit::textChanged   , this, &LogSettings::onDataChanged);
}

void LogSettings::onBrowseButtonClicked()
{
    textLogLocation()->setText(QFileDialog::getExistingDirectory(this, tr("Open Log Directory"), textLogLocation()->text(), QFileDialog::ShowDirsOnly));
}

void LogSettings::applyChanges()
{
    if (mCanSave == false)
    {
        QMessageBox::critical(this, tr("Error"), tr("The endpoint must be tested and must be working before saving the changes!"));
    }
    else
    {
        saveData();
    }
}

void LogSettings::setData(const QString& address, uint16_t port, const QString& logFile, const QString& logLocation)
{
    textLogLocation()->setText(logLocation);
    textLogFileName()->setText(logFile);
    textIpAddress()->setText(address);
    textPortNumber()->setText(QString::number(port));

    update();
}

void LogSettings::closingSettings(void)
{
    LogObserver::disconnect();
    LogObserver::releaseLogObserver();
}

void LogSettings::saveData() const
{
    QString logLocation{ textLogLocation()->text() };
    QString logFileName{ textLogFileName()->text() };
    QString ipAddress{ textIpAddress()->text() };
    uint16_t portNumber{ static_cast<uint16_t>(textPortNumber()->text().toUInt()) };

    if (logLocation.isEmpty() || logFileName.isEmpty() || ipAddress.isEmpty() || (portNumber == NESocket::InvalidPort))
    {
        return;
    }

    // Save logging directory path
    WorkspaceEntry currentWorkspace{ LusanApplication::getActiveWorkspace() };
    currentWorkspace.setDirLogs(logLocation);

    OptionsManager& optionsManager = LusanApplication::getOptions();
    optionsManager.updateWorkspace(currentWorkspace);
    optionsManager.writeOptions();
    
    LogCollectorClient& lgClient = LogCollectorClient::getInstance();

    // Save logging configuration
    lgClient.setConfigLoggerDatabaseLocation(logLocation.toStdString());
    lgClient.setConfigLoggerDatabaseName(logFileName.toStdString());
    lgClient.setConfigLoggerAddress(ipAddress.toStdString());
    lgClient.setConfigLoggerPort(portNumber);
    lgClient.saveLoggerConfig();
}

void LogSettings::onTestButtonClicked(bool checked)
{
    if (mTestTriggered)
    {
        disconnect(mTestConnect);
        disconnect(mTestMessage);
        LogObserver::disconnect();
        LogObserver::releaseLogObserver();
        
        textConnectionStatus()->setTextColor(QColor(Qt::magenta));
        textConnectionStatus()->setText(_textTestCanceled);
        buttonTestConnection()->setText(tr("&Test"));
        mTestTriggered = false;
        
        return;
    }

    QString logLocation{ textLogLocation()->text() };
    QString logFileName{ textLogFileName()->text() };
    QString ipAddress{ textIpAddress()->text() };
    uint16_t portNumber{ static_cast<uint16_t>(textPortNumber()->text().toUInt()) };
    mCanSave = false;

    if (logLocation.isEmpty() || logFileName.isEmpty() || ipAddress.isEmpty() || (portNumber == NESocket::InvalidPort))
    {
        QMessageBox::critical(this, tr("Error"), tr("Invalid Log Collector Service configuration, fields cannot be invalid!"));
        return;
    }
    
    LogObserver::disconnect();
    LogObserver::releaseLogObserver();

    LogCollectorClient& client = LogCollectorClient::getInstance();
    mTestConnect = connect(&client, &LogCollectorClient::signalLogServiceConnected, this, &LogSettings::onLogServiceConnected);
    mTestMessage = connect(&client, &LogCollectorClient::signalLogInstancesConnect, this, &LogSettings::onLogInstancesConnected);
    
    std::filesystem::path path(logLocation.toStdString());
    if (static_cast<LogObserverBase &>(client).connect(ipAddress.toStdString(), portNumber, path.string()) == false)
    {
        disconnect(mTestConnect);
        disconnect(mTestMessage);
        textConnectionStatus()->setTextColor(QColor(Qt::darkRed));
        textConnectionStatus()->setText(_textConnectionFailed);
    }
    else
    {
        mTestTriggered = true;
        textConnectionStatus()->setTextColor(QColor(Qt::darkBlue));
        textConnectionStatus()->setText(_textTestInProgress);
        buttonTestConnection()->setText(tr("Stop &Test"));
    }
}

void LogSettings::onDataChanged()
{
    if (mTestTriggered)
    {
        disconnect(mTestConnect);
        disconnect(mTestMessage);

        LogObserver::disconnect();
        LogObserver::releaseLogObserver();

        textConnectionStatus()->setTextColor(QColor(Qt::magenta));
        textConnectionStatus()->setText(_textTestInterrupted);
    }
    else
    {
        textConnectionStatus()->setTextColor(QColor(Qt::darkBlue));
        textConnectionStatus()->setText(_textDataChanged);
    }
    
    buttonTestConnection()->setText(tr("&Test"));
    mTestTriggered = false;
    mCanSave = false;
    
}

void LogSettings::onLogServiceConnected(bool isConnected, const std::string& address, uint16_t port)
{
    if (mTestTriggered == false)
        return;

    if (isConnected)
    {
        mAddress = address.c_str();
        mPort = port;
        textConnectionStatus()->setTextColor(QColor(Qt::green));
        textConnectionStatus()->setText(_textServiceConnected.arg(address.c_str()).arg(port));
        LogCollectorClient::getInstance().requestInstances();
    }
    else
    {
        disconnect(mTestConnect);
        disconnect(mTestMessage);

        LogObserver::disconnect();
        LogObserver::releaseLogObserver();

        if (mCanSave == false)
        {
            textConnectionStatus()->setTextColor(QColor(Qt::darkRed));
            textConnectionStatus()->setText(_textTestFailed);
        }
    }
}

void LogSettings::onLogInstancesConnected(const std::vector< NEService::sServiceConnectedInstance >& instances)
{
    if (mTestTriggered == false)
        return;
    
    disconnect(mTestConnect);
    disconnect(mTestMessage);

    LogObserver::disconnect();
    LogObserver::releaseLogObserver();

    textConnectionStatus()->setTextColor(QColor(Qt::darkGreen));
    textConnectionStatus()->setText(_textTestSucceeded.arg(instances.size()));
    buttonTestConnection()->setText(tr("&Test"));
    mCanSave = true;
    mTestTriggered = false;
}

inline QLineEdit* LogSettings::textLogLocation(void) const
{
    return ui->editLogLocation;
}

inline QLineEdit* LogSettings::textLogFileName(void) const
{
    return ui->editLogFileName;
}

inline QLineEdit* LogSettings::textIpAddress(void) const
{
    return ui->editLogAddres;
}

inline QLineEdit* LogSettings::textPortNumber(void) const
{
    return ui->editLogPort;
}

inline QTextEdit* LogSettings::textConnectionStatus(void) const
{
    return ui->textConnectStatus;
}

inline QPushButton* LogSettings::buttonBrowseDirs(void) const
{
    return ui->buttonBrowseDirs;
}

inline QPushButton* LogSettings::buttonTestConnection(void) const
{
    return ui->buttonTestConnect;
}
