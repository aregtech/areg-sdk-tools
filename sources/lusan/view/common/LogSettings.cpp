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
    textConnectionStatus()->setText(tr("No changes yet..."));
    
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
        return;
    }

    saveData();
}

void LogSettings::setData(const QString& address, uint16_t port, const QString& logFile, const QString& logLocation)
{
    textLogLocation()->setText(logLocation);
    textLogFileName()->setText(logFile);
    textIpAddress()->setText(address);
    textPortNumber()->setText(QString::number(port));

    update();
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
        LogCollectorClient& client = LogCollectorClient::getInstance();
        static_cast<LogObserverBase &>(client).disconnect();
        LogObserver::releaseLogObserver();
        disconnect(&client, &LogCollectorClient::signalLogServiceConnected, this, &LogSettings::onLogServiceConnected);
        disconnect(&client, &LogCollectorClient::signalLogInstancesConnect, this, &LogSettings::onLogInstancesConnected);
        
        textConnectionStatus()->setText(tr("Warning: Connection to Log Collector Service was interrupted."));
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
        QMessageBox::critical(this, tr("Error"), tr("Invalid Log Collector Service configuration, fileds cannot be invalid!"));
        return;
    }
    
    if (LogObserver::isConnected())
        LogObserver::disconnect();
    
    LogCollectorClient& client = LogCollectorClient::getInstance();
    if (client.isConnected())
    {
        static_cast<LogObserverBase &>(client).disconnect();
        LogObserver::releaseLogObserver();
    }
    
    connect(&client, &LogCollectorClient::signalLogServiceConnected, this, &LogSettings::onLogServiceConnected);
    connect(&client, &LogCollectorClient::signalLogInstancesConnect, this, &LogSettings::onLogInstancesConnected);
    
    std::filesystem::path path(logLocation.toStdString());
    if (static_cast<LogObserverBase &>(client).connect(ipAddress.toStdString(), portNumber, path.string()) == false)
    {
        disconnect(&client, &LogCollectorClient::signalLogServiceConnected, this, &LogSettings::onLogServiceConnected);
        disconnect(&client, &LogCollectorClient::signalLogInstancesConnect, this, &LogSettings::onLogInstancesConnected);
        textConnectionStatus()->setText(tr("Error: Failed to trigger Log Collector Service connection. Check network connection!"));
    }
    else
    {
        textConnectionStatus()->setText(tr("Waiting: The Log Collector Service connection is in progress..."));
        buttonTestConnection()->setText(tr("Stop &Test"));
    }
}

void LogSettings::onDataChanged()
{
    if (mTestTriggered)
    {
        LogCollectorClient& client = LogCollectorClient::getInstance();
        static_cast<LogObserverBase &>(client).disconnect();
        LogObserver::releaseLogObserver();
        disconnect(&client, &LogCollectorClient::signalLogServiceConnected, this, &LogSettings::onLogServiceConnected);
        disconnect(&client, &LogCollectorClient::signalLogInstancesConnect, this, &LogSettings::onLogInstancesConnected);
        textConnectionStatus()->setText(tr("Warning: The connection data is updated, the Log Observer Service connection test is canceled..."));
    }
    else
    {
        textConnectionStatus()->setText(tr("Warning: Test Log Observer Service connection before save configuration"));
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
        textConnectionStatus()->setText(tr("Result: Connected to Log Collector Service at %1:%2, waiting for messages...").arg(address.c_str()).arg(port));
        LogCollectorClient::getInstance().requestInstances();
    }
    else
    {
        LogCollectorClient& client = LogCollectorClient::getInstance();
        static_cast<LogObserverBase &>(client).disconnect();
        LogObserver::releaseLogObserver();
        disconnect(&client, &LogCollectorClient::signalLogServiceConnected, this, &LogSettings::onLogServiceConnected);
        disconnect(&client, &LogCollectorClient::signalLogInstancesConnect, this, &LogSettings::onLogInstancesConnected);
        
        mCanSave = false;
        textConnectionStatus()->setText(tr("Error: Failed to connect to the Log Collector Service. Check data and network connection."));
    }
}

void LogSettings::onLogInstancesConnected(const std::vector< NEService::sServiceConnectedInstance >& instances)
{
    if (mTestTriggered == false)
        return;
    
    LogCollectorClient& client = LogCollectorClient::getInstance();
    static_cast<LogObserverBase &>(client).disconnect();
    LogObserver::releaseLogObserver();
    disconnect(&client, &LogCollectorClient::signalLogServiceConnected, this, &LogSettings::onLogServiceConnected);
    disconnect(&client, &LogCollectorClient::signalLogInstancesConnect, this, &LogSettings::onLogInstancesConnected);
    
    textConnectionStatus()->setText(tr("Success: Succeeded Log Collector Service connection test, currently there are %1 connected log sources instances.").arg(instances.size()));
    buttonTestConnection()->setText(tr("&Test"));
    mCanSave = true;
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

inline QLineEdit* LogSettings::textConnectionStatus(void) const
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
