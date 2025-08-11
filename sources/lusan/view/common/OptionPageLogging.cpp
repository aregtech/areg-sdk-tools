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
 *  \file        lusan/view/common/OptionPageLogging.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag, Artak Avetyan
 *  \brief       Lusan application, implementation of Log Settings page of options dialog.
 *
 ************************************************************************/

#include "lusan/view/common/OptionPageLogging.hpp"
#include "ui/ui_OptionPageLogging.h"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/LogCollectorClient.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/log/LogObserver.hpp"
#include "lusan/view/common/OptionPageWorkspace.hpp"

#include "areg/base/NESocket.hpp"

#include <QAbstractItemView>
#include <QtAssert>
#include <QAbstractButton>
#include <QDialog>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <string>

const QString   OptionPageLogging::_textNoChanges         { tr("No data changed yet ...") };
const QString   OptionPageLogging::_textDataChanged       { tr("WARNING: Test the Log Collector Service connection before saving changes ...") };
const QString   OptionPageLogging::_textTestInProgress    { tr("WAITING: Test connection is in progress, make sure the Log Collector Service is configured and runs ...") };
const QString   OptionPageLogging::_textTestInterrupted   { tr("WARNING: The Log Collector Service connection data is updated, interrupting ongoing connection ...") };
const QString   OptionPageLogging::_textServiceConnected  { tr("RESULT: Connected to the Log Collector Service at %1:%2, waiting for messaging ...") };
const QString   OptionPageLogging::_textTestSucceeded     { tr("SUCCESS: Succeeded the Log Collector Service connection test, currently there are %1 connected log sources instances.") };
const QString   OptionPageLogging::_textConnectionFailed  { tr("ERROR: Failed to trigger connection to the Log Collector Service, check network connection and retry.") };
const QString   OptionPageLogging::_textTestFailed        { tr("FAILURE: Failed to connect to the Log Collector Service. Check connection data and try again.") };
const QString   OptionPageLogging::_textTestCanceled      { tr("WARNING: Connection to the Log Collector Service was interrupted") };

OptionPageLogging::OptionPageLogging(QDialog* parent)
    : OptionPageBase{parent}
    , ui            {std::make_unique<Ui::OptionPageLoggingForm>()}
    , mPortValidator{QRegularExpression("[0-9]{2,5}"), this}
    , mTestTriggered{false}
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

OptionPageLogging::OptionPageLogging(QDialog *parent, const QString& address, uint16_t port, const QString &logFile, const QString &logLocation)
    : OptionPageBase{parent}
    , ui            {std::make_unique<Ui::OptionPageLoggingForm>()}
    , mPortValidator{QRegularExpression("[0-9]{2,5}"), this}
    , mTestTriggered{false}
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

OptionPageLogging::~OptionPageLogging()
{
    if (mTestTriggered)
    {
        LogObserver::disconnect();
        LogObserver::releaseLogObserver();
    }
}

void OptionPageLogging::setupDialog()
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
        logLocation = NELusanCommon::fixPath(logLocation);
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

void OptionPageLogging::connectSignals() const
{
    connect(buttonBrowseDirs()      , &QAbstractButton::clicked , this, &OptionPageLogging::onBrowseButtonClicked);
    connect(buttonTestConnection()  , &QAbstractButton::clicked , this, &OptionPageLogging::onTestButtonClicked);
    connect(textIpAddress()         , &QLineEdit::textChanged   , this, &OptionPageLogging::onDataChanged);
    connect(textPortNumber()        , &QLineEdit::textChanged   , this, &OptionPageLogging::onDataChanged);
    connect(textLogFileName()       , &QLineEdit::textChanged   , this, &OptionPageLogging::onLogFileNameChanged);
    
    connect(textLogLocation()       , &QLineEdit::textChanged   , [this]() {
        QString logLocation = textLogLocation()->text();
        emit signalWorkspaceLocationsChanged(  sWorkspaceDir{}
                                             , sWorkspaceDir{}
                                             , sWorkspaceDir{}
                                             , sWorkspaceDir{true, textLogLocation()->text()});
    });
}

void OptionPageLogging::onBrowseButtonClicked()
{
    QString oldPath = NELusanCommon::fixPath(textLogLocation()->text());
    QString newPath = NELusanCommon::fixPath(QFileDialog::getExistingDirectory(this, tr("Open Log Directory"), oldPath, QFileDialog::ShowDirsOnly));
    if ((newPath.isEmpty() == false) && (newPath != oldPath))
    {
        textLogLocation()->setText(newPath);
        setDataModified(true);
    }
}

void OptionPageLogging::applyChanges()
{
    if (isDataModified() && (canSave() == false))
    {
        warnMessage();
    }
    else
    {
        saveData();
        OptionPageBase::applyChanges();
    }
}

void OptionPageLogging::closingOptions(bool OKpressed)
{
    LogObserver::disconnect();
    LogObserver::releaseLogObserver();
    OptionPageBase::closingOptions(OKpressed);
}

void OptionPageLogging::warnMessage(void)
{
    QMessageBox::critical(static_cast<QWidget *>(this), tr("Error"), tr("The endpoint must be tested and must be working before saving the changes!"));
}

void OptionPageLogging::setData(const QString& address, const QString& hostName, uint16_t port, const QString& logFile, const QString& logLocation)
{
    QString oldLocation { getLogLocation() };
    QString oldFileName { getLogFileName() };
    QString oldAddress  { getServiceAddress() };
    uint16_t oldPort    { getServicePort() };
    
    if (oldLocation != logLocation)
    {
        textLogLocation()->setText(logLocation);
    }

    if (oldFileName != logFile)
    {
        textLogFileName()->setText(logFile);
    }
    
    if (oldPort != port)
    {
        textPortNumber()->setText(QString::number(port));
    }
    
    if (NESocket::isIpAddress(oldAddress.toStdString()))
    {
        if (oldAddress != address)
        {
            textIpAddress()->setText(address);
        }
    }
    else if (oldAddress != hostName)
    {
        textIpAddress()->setText(hostName);
    }
}

void OptionPageLogging::updateWorkspaceDirectories( const sWorkspaceDir& sources
                                                  , const sWorkspaceDir& includes
                                                  , const sWorkspaceDir& delivery
                                                  , const sWorkspaceDir& logs)
{
    if (logs.isValid && (textLogLocation()->text() != logs.location))
    {
        textLogLocation()->blockSignals(true);
        textLogLocation()->setText(logs.location);
        textLogLocation()->blockSignals(false);
    }
}

void OptionPageLogging::saveData() const
{
    QString logLocation { getLogLocation() };
    QString logFileName { getLogFileName() };
    QString ipAddress   { getServiceAddress() };
    uint16_t portNumber { getServicePort() };

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

inline QString OptionPageLogging::getLogLocation(void) const
{
    return textLogLocation()->text();
}

inline QString OptionPageLogging::getLogFileName(void) const
{
    return textLogFileName()->text();
}

inline QString OptionPageLogging::getServiceAddress(void) const
{
    return textIpAddress()->text();
}

inline uint16_t OptionPageLogging::getServicePort(void) const
{
    return static_cast<uint16_t>(textPortNumber()->text().toUInt());
}

void OptionPageLogging::onTestButtonClicked(bool checked)
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
    setCanSave(false);

    if (logLocation.isEmpty() || logFileName.isEmpty() || ipAddress.isEmpty() || (portNumber == NESocket::InvalidPort))
    {
        QMessageBox::critical(this, tr("Error"), tr("Invalid Log Collector Service configuration, fields cannot be invalid!"));
        return;
    }
    
    LogObserver::disconnect();
    LogObserver::releaseLogObserver();

    LogCollectorClient& client = LogCollectorClient::getInstance();
    mTestConnect = connect(&client, &LogCollectorClient::signalLogServiceConnected, this, &OptionPageLogging::onLogServiceConnected);
    mTestMessage = connect(&client, &LogCollectorClient::signalLogInstancesConnect, this, &OptionPageLogging::onLogInstancesConnected);
    
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

void OptionPageLogging::onDataChanged()
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
    setCanSave(false);
    setDataModified(true);
}

void OptionPageLogging::onLogLocationChanged(void)
{
}

void OptionPageLogging::onLogFileNameChanged(void)
{
}

void OptionPageLogging::onLogServiceConnected(bool isConnected, const std::string& address, uint16_t port)
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
        
        if (canSave() == false)
        {
            textConnectionStatus()->setTextColor(QColor(Qt::darkRed));
            textConnectionStatus()->setText(_textTestFailed);
        }
    }
}

void OptionPageLogging::onLogInstancesConnected(const std::vector< NEService::sServiceConnectedInstance >& instances)
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
    setCanSave(true);
    mTestTriggered = false;
}

inline QLineEdit* OptionPageLogging::textLogLocation(void) const
{
    return ui->editLogLocation;
}

inline QLineEdit* OptionPageLogging::textLogFileName(void) const
{
    return ui->editLogFileName;
}

inline QLineEdit* OptionPageLogging::textIpAddress(void) const
{
    return ui->editLogAddres;
}

inline QLineEdit* OptionPageLogging::textPortNumber(void) const
{
    return ui->editLogPort;
}

inline QTextEdit* OptionPageLogging::textConnectionStatus(void) const
{
    return ui->textConnectStatus;
}

inline QPushButton* OptionPageLogging::buttonBrowseDirs(void) const
{
    return ui->buttonBrowseDirs;
}

inline QPushButton* OptionPageLogging::buttonTestConnection(void) const
{
    return ui->buttonTestConnect;
}
