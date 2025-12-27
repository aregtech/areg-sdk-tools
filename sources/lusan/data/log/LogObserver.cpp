/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/log/LogObserver.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer component with empty service provider.
 *
 ************************************************************************/
 /************************************************************************
  * Include files.
  ************************************************************************/
#include "lusan/data/log/LogObserver.hpp"

#include "lusan/common/LogCollectorClient.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include "areg/appbase/NEApplication.hpp"
#include "areg/base/DateTime.hpp"
#include "areg/component/ComponentLoader.hpp"
#include "areg/component/ComponentThread.hpp"
#include "areglogger/client/LogObserverApi.h"

#include <atomic>

namespace
{
    std::atomic_bool _modelInitialized{ false };
    std::atomic_bool _observerStart{ false };
    std::atomic<LogObserver*> _component(nullptr);
    std::atomic< FuncLogObserverStarted> _logObserverStarted(nullptr);
}

BEGIN_MODEL(LogObserver::LogobserverModel)

    BEGIN_REGISTER_THREAD(LogObserver::LogobserverThread)
        BEGIN_REGISTER_COMPONENT_EX( LogObserver::LogObserverComponent
                                   , NEMemory::InvalidElement
                                , ([](const NERegistry::ComponentEntry& e, ComponentThread& t) -> Component * {
                                    return LogObserver::CreateComponent(e, t);
                                })
                                , ([](Component & c, const NERegistry::ComponentEntry & e) -> void {
                                    LogObserver::DeleteComponent(c, e);
                                }))
            REGISTER_IMPLEMENT_SERVICE( NEService::EmptyServiceName, NEService::EmptyServiceVersion )
        END_REGISTER_COMPONENT(LogObserver::LogObserverComponent)
    END_REGISTER_THREAD(LogObserver::LogobserverThread)

END_MODEL(LogObserver::LogobserverModel)

bool LogObserver::createLogObserver(FuncLogObserverStarted callbackStarted)
{
    bool result{ true };
    if (_modelInitialized.exchange(true) == false)
    {
        Q_ASSERT(Thread::findThreadByName(LogObserver::LogobserverThread) == nullptr);
        Q_ASSERT(ComponentLoader::isModelLoaded(LogObserver::LogobserverModel) == false);

        _observerStart = false;
        _logObserverStarted = callbackStarted;
        result = ComponentLoader::loadComponentModel(LogObserver::LogobserverModel);
    }
    
    return result;
}

void LogObserver::releaseLogObserver(void)
{
    _modelInitialized = false;
    _observerStart = false;
    ComponentLoader::unloadComponentModel(true, LogObserver::LogobserverModel);
}

void LogObserver::disconnect(void)
{
    static_cast<LogObserverBase &>(LogObserver::getClient()).disconnect();
}

bool LogObserver::pause(void)
{
    return LogObserver::getClient().pause();
}

bool LogObserver::resume(void)
{
    return LogObserver::getClient().resume();
}

void LogObserver::stop(void)
{
    LogObserver::getClient().stop();
}

bool LogObserver::restart(const QString& dbLocation)
{
    return LogObserver::getClient().restart(dbLocation.toStdString());
}

bool LogObserver::requestInstances(void)
{
    return LogObserver::getClient().requestInstances();
}

bool LogObserver::requestScopes(ITEM_ID target /*= NEService::TARGET_ALL*/)
{
    return LogObserver::getClient().requestScopes(target);
}

bool LogObserver::requestChangeScopePrio(ITEM_ID target, const sLogScope* scopes, uint32_t count)
{
    return LogObserver::getClient().requestChangeScopePrio(target, scopes, count);
}

bool LogObserver::requestSaveConfig(ITEM_ID target /*= NEService::TARGET_ALL*/)
{
    return LogObserver::getClient().requestSaveConfig(target);
}

void LogObserver::saveLoggerConfig(void)
{
    LogObserver::getClient().saveLoggerConfig();
}

const NESocket::SocketAddress& LogObserver::getLogServiceAddress(void)
{
    return LogObserver::getClient().getLoggerAddress();
}

QString LogObserver::getConnectedAddress(void)
{
    return QString(LogObserver::getClient().getLoggerIpAddress().c_str());
}

QString LogObserver::getConnectedHostName(void)
{
    return QString(LogObserver::getClient().getLoggerHostName().c_str());
}

uint16_t LogObserver::getConnectedPort(void)
{
    return LogObserver::getClient().getLoggerPort();
}

QString LogObserver::getActiveDatabase(void)
{
    return QString(LogObserver::getClient().getActiveDatabasePath().c_str());
}

QString LogObserver::getInitDatabase(void)
{
    return QString(LogObserver::getClient().getInitDatabasePath().c_str());
}

QString LogObserver::getConfigDatabaseName(void)
{
    return QString(LogObserver::getClient().getConfigLoggerDatabaseName().c_str());
}

QString LogObserver::getConfigDatabaseLocation(void)
{
    return QString(LogObserver::getClient().getConfigLoggerDatabaseLocation().c_str());
}

bool LogObserver::setConfigDatabaseName(const QString& dbName)
{
    if (LogObserver::getComponent() != nullptr)
    {
        LogObserver::getClient().setConfigLoggerDatabaseName(dbName.toStdString());
        return true;
    }
    else
    {
        return false;
    }
}

bool LogObserver::setConfigDatabaseLocation(const QString& dbLocation)
{
    if (LogObserver::getComponent() != nullptr)
    {
        LogObserver::getClient().setConfigLoggerDatabaseLocation(dbLocation.toStdString());
        return true;
    }
    else
    {
        return false;
    }
}

bool LogObserver::isConnected(void)
{
    return LogObserver::getClient().isConnected();
}

bool LogObserver::connect(const QString& address, uint16_t port, const QString& dbLocation)
{
    return static_cast<LogObserverBase &>(LogObserver::getClient()).connect(address.toStdString(), port, dbLocation.toStdString());
}

Component * LogObserver::CreateComponent(const NERegistry::ComponentEntry & entry, ComponentThread & owner)
{
    Q_ASSERT(_component.load() == nullptr);
    _component.store(DEBUG_NEW LogObserver( entry, owner));
    return static_cast<Component *>(_component.load());
}

void LogObserver::DeleteComponent(Component & compObject, const NERegistry::ComponentEntry & /* entry */)
{
    _component.store(nullptr);
    delete (&compObject);
}

LogObserver* LogObserver::getComponent(void)
{
    return _component.load();
}

void LogObserver::queryLogInstanceNames(std::vector<String>& names)
{
    LogObserver::getClient().getLogInstanceNames(names);
}

void LogObserver::queryLogInstances(std::vector<ITEM_ID>& ids)
{
    LogObserver::getClient().getLogInstances(ids);
}

void LogObserver::queryLogThreadNames(std::vector<String>& names)
{
    LogObserver::getClient().getLogThreadNames(names);
}

void LogObserver::queryLogThreads(std::vector<ITEM_ID>& ids)
{
    LogObserver::getClient().getLogThreads(ids);
}

void LogObserver::queryPriorityNames(std::vector<String>& names)
{
    LogObserver::getClient().getPriorityNames(names);
}

void LogObserver::queryLogInstanceInfos(std::vector<NEService::sServiceConnectedInstance>& infos)
{
    LogObserver::getClient().getLogInstanceInfos(infos);
}

void LogObserver::queryLogInstScopes(std::vector<NELogging::sScopeInfo>& scopes, ITEM_ID instId)
{
    LogObserver::getClient().getLogInstScopes(scopes, instId);
}

void LogObserver::queryLogMessages(std::vector<SharedBuffer>& messages)
{
    LogObserver::getClient().getLogMessages(messages);
}

void LogObserver::queryLogInstMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId)
{
    LogObserver::getClient().getLogInstMessages(messages, instId);
}

void LogObserver::queryLogScopeMessages(std::vector<SharedBuffer>& messages, uint32_t scopeId)
{
    LogObserver::getClient().getLogScopeMessages(messages, scopeId);
}

void LogObserver::queryLogMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId, uint32_t scopeId)
{
    LogObserver::getClient().getLogMessages(messages, instId, scopeId);
}

LogCollectorClient& LogObserver::getClient(void)
{
    return LogCollectorClient::getInstance();
}

LogObserver::LogObserver(const NERegistry::ComponentEntry & entry, ComponentThread & ownerThread)
    : Component ( entry, ownerThread )
    , StubBase  ( self(), NEService::getEmptyInterface() )
    , IELogObserverEventConsumer()

    , mLogClient    (LogCollectorClient::getInstance())
    , mConfigFile   (NELusanCommon::INIT_FILE.toStdString())
{
}

LogObserver::~LogObserver(void)
{
}

void LogObserver::startupServiceInterface(Component & holder)
{
    StubBase::startupServiceInterface(holder);
    static_cast<LogObserverBase &>(mLogClient).stop();
    static_cast<LogObserverBase &>(mLogClient).disconnect();

    FuncLogObserverStarted callback = _logObserverStarted.load();
    if (callback != nullptr)
    {
        callback();
    }
    
    LogObserverEvent::addListener(static_cast<IELogObserverEventConsumer &>(self()), getComponentThread());
    
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogObserverConfigured   , this, &LogObserver::slotLogObserverConfigured    , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogDbConfigured         , this, &LogObserver::slotLogDbConfigured          , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogServiceConnected     , this, &LogObserver::slotLogServiceConnected      , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogObserverStarted      , this, &LogObserver::slotLogObserverStarted       , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogDbCreated            , this, &LogObserver::slotLogDbCreated             , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogMessagingFailed      , this, &LogObserver::slotLogMessagingFailed       , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogInstancesConnect     , this, &LogObserver::slotLogInstancesConnect      , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogInstancesDisconnect  , this, &LogObserver::slotLogInstancesDisconnect   , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogServiceDisconnected  , this, &LogObserver::slotLogServiceDisconnected   , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogRegisterScopes       , this, &LogObserver::slotLogRegisterScopes        , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogUpdateScopes         , this, &LogObserver::slotLogUpdateScopes          , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogMessage              , this, &LogObserver::slotLogMessage               , Qt::DirectConnection);
    
    if (mLogClient.isInitialized() == false)
    {
        mLogClient.initialize(mConfigFile.getData());
    }
    
    QString address { mLogClient.getLoggerIpAddress().c_str() };
    uint16_t port   { mLogClient.getLoggerPort() };
    QString logFile { mLogClient.getActiveDatabasePath().c_str() };

    emit signalLogObserverInstance(true, address, port, logFile);
}

void LogObserver::shutdownServiceInterface(Component & holder)
{
    QString address { mLogClient.getLoggerIpAddress().c_str() };
    uint16_t port   { mLogClient.getLoggerPort() };
    QString logFile { mLogClient.getActiveDatabasePath().c_str() };

    emit signalLogObserverInstance(false, address, port, logFile);

    static_cast<LogObserverBase &>(mLogClient).stop();
    static_cast<LogObserverBase &>(mLogClient).disconnect();
    StubBase::shutdownServiceInterface(holder);
    
    LogObserverEvent::removeListener(static_cast<IELogObserverEventConsumer &>(self()), getComponentThread());
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogObserverConfigured    , this, &LogObserver::slotLogObserverConfigured);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogDbConfigured          , this, &LogObserver::slotLogDbConfigured);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogServiceConnected      , this, &LogObserver::slotLogServiceConnected);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogObserverStarted       , this, &LogObserver::slotLogObserverStarted);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogDbCreated             , this, &LogObserver::slotLogDbCreated);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogMessagingFailed       , this, &LogObserver::slotLogMessagingFailed);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogInstancesConnect      , this, &LogObserver::slotLogInstancesConnect);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogInstancesDisconnect   , this, &LogObserver::slotLogInstancesDisconnect);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogServiceDisconnected   , this, &LogObserver::slotLogServiceDisconnected);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogRegisterScopes        , this, &LogObserver::slotLogRegisterScopes);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogUpdateScopes          , this, &LogObserver::slotLogUpdateScopes);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogMessage               , this, &LogObserver::slotLogMessage);
}

void LogObserver::sendNotification(unsigned int  /*msgId*/)
{
}

void LogObserver::errorRequest(unsigned int /*msgId*/, bool /*msgCancel*/)
{
}

void LogObserver::processRequestEvent(ServiceRequestEvent& /*eventElem*/)
{
}

void LogObserver::processAttributeEvent(ServiceRequestEvent& /*eventElem*/)
{
}

void LogObserver::processEvent(const LogObserverEventData& data)
{
    const SharedBuffer& stream{ data.getBuffer() };
    stream.moveToBegin();

    switch (data.getEvent())
    {
    case LogObserverEventData::eLogObserverEvent::CMD_Configured:
    {
        bool isEnabled{ false };
        uint16_t port{ NESocket::InvalidPort };
        std::string address;
        stream >> isEnabled >> address >> port;
        emit signalLogObserverConfigured(isEnabled, QString::fromStdString(address), port);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_DbConfigured:
    {
        bool isEnabled{ false };
        std::string dbName;
        std::string dbLocation;
        std::string dbUser;
        stream >> isEnabled >> dbName >> dbLocation >> dbUser;
        emit signalLogDbConfigured(isEnabled, QString::fromStdString(dbName), QString::fromStdString(dbLocation), QString::fromStdString(dbUser));

    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_Connected:
    {
        bool isConnected{ false };
        std::string address;
        uint16_t port{ NESocket::InvalidPort };
        stream >> isConnected >> address >> port;
        emit signalLogServiceConnected(isConnected, QString::fromStdString(address), port);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_Started:
    {
        bool isStarted{ false };
        stream >> isStarted;
        emit signalLogObserverStarted(isStarted);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_DbCreated:
    {
        std::string dbLocation;
        stream >> dbLocation;
        emit signalLogDbCreated(QString::fromStdString(dbLocation));
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_MessageFailed:
    {
        emit signalLogMessagingFailed();
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_InstConnected:
    {
        uint32_t count{ 0 };
        stream >> count;

        std::vector< NEService::sServiceConnectedInstance> instances(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            NEService::sServiceConnectedInstance& instance = instances[i];
            stream >> instance;
        }

        emit signalLogInstancesConnect(instances);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_InstDisconnected:
    {
        uint32_t count{ 0 };
        stream >> count;

        std::vector< NEService::sServiceConnectedInstance> instances(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            NEService::sServiceConnectedInstance& instance = instances[i];
            stream >> instance;
        }

        emit signalLogInstancesDisconnect(instances);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_ServiceDisconnect:
    {
        emit signalLogServiceDisconnected();
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_ScopesRegistered:
    {
        ITEM_ID inst{ NEService::COOKIE_ANY };
        uint32_t count{ 0 };
        stream >> inst >> count;

        std::vector<NELogging::sScopeInfo> scopes(inst);
        for (uint32_t i = 0; i < count; ++i)
        {
            sLogScope data{};
            NELogging::sScopeInfo & scope = scopes[i];
            stream.read(reinterpret_cast<unsigned char *>(&data), sizeof(sLogScope));
            scope.scopeId   = data.lsId;
            scope.scopePrio = data.lsPrio;
            scope.scopeName = data.lsName;
        }

        emit signalLogRegisterScopes(inst, scopes);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_ScopesUpdated:
    {
        ITEM_ID inst{ NEService::COOKIE_ANY };
        uint32_t count{ 0 };
        stream >> inst >> count;

        std::vector<NELogging::sScopeInfo> scopes(inst);
        for (uint32_t i = 0; i < count; ++i)
        {
            sLogScope data{};
            NELogging::sScopeInfo & scope = scopes[i];
            stream.read(reinterpret_cast<unsigned char *>(&data), sizeof(sLogScope));
            scope.scopeId   = data.lsId;
            scope.scopePrio = data.lsPrio;
            scope.scopeName = data.lsName;
        }

        emit signalLogUpdateScopes(inst, scopes);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_LogMessage:
    {
        emit signalLogMessage(stream);
    }
    break;

    default:
        break;
    }
}

void LogObserver::slotLogObserverConfigured(bool isEnabled, const std::string& address, uint16_t port)
{
    SharedBuffer stream;
    stream << isEnabled << address << port;
    LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_Configured, stream), getComponentThread());
}

void LogObserver::slotLogDbConfigured(bool isEnabled, const std::string& dbName, const std::string& dbLocation, const std::string& dbUser)
{
    SharedBuffer stream;
    stream << isEnabled << dbName << dbLocation << dbUser;
    LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_DbConfigured, stream), getComponentThread());
}

void LogObserver::slotLogServiceConnected(bool isConnected, const std::string& address, uint16_t port)
{
    SharedBuffer stream;
    stream << isConnected << address << port;
    LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_Connected, stream), getComponentThread());
}

void LogObserver::slotLogObserverStarted(bool isStarted)
{
    if (isStarted )
    {
        SharedBuffer stream;
        stream << isStarted;
        LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_Started, stream), getComponentThread());
    }
    else
    {
        emit signalLogObserverStarted(false);
    }
}

void LogObserver::slotLogDbCreated(const std::string& dbLocation)
{
    SharedBuffer stream;
    stream << dbLocation;
    LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_DbCreated, stream), getComponentThread());
}

void LogObserver::slotLogMessagingFailed(void)
{
    LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_MessageFailed), getComponentThread());
}

void LogObserver::slotLogInstancesConnect(const std::vector<NEService::sServiceConnectedInstance>& instances)
{
    SharedBuffer stream;
    stream << instances;
    LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_InstConnected, stream), getComponentThread());
}

void LogObserver::slotLogInstancesDisconnect(const std::vector<NEService::sServiceConnectedInstance>& instances)
{
    SharedBuffer stream;
    stream << instances;
    LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_InstDisconnected, stream), getComponentThread());
}

void LogObserver::slotLogServiceDisconnected(void)
{
    LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_ServiceDisconnect), getComponentThread());
}

void LogObserver::slotLogRegisterScopes(ITEM_ID cookie, const sLogScope* scopes, int count)
{
    constexpr uint32_t size{ sizeof(sLogScope) };
    SharedBuffer stream;
    stream << cookie << count;
    for (int i = 0; i < count; ++i)
    {
        const sLogScope& scope{ scopes[i] };
        stream.write(reinterpret_cast<const unsigned char*>(&scope), size);
    }

    LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_ScopesRegistered, stream), getComponentThread());
}

void LogObserver::slotLogUpdateScopes(ITEM_ID cookie, const sLogScope* scopes, int count)
{
    constexpr uint32_t size{ sizeof(sLogScope) };
    SharedBuffer stream;
    stream << cookie << count;
    for (int i = 0; i < count; ++i)
    {
        const sLogScope& scope{ scopes[i] };
        stream.write(reinterpret_cast<const unsigned char*>(&scope), size);
    }

    LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_ScopesUpdated, stream), getComponentThread());
}

void LogObserver::slotLogMessage(const SharedBuffer& logMessage)
{
    LogObserverEvent::sendEvent(LogObserverEventData(LogObserverEventData::eLogObserverEvent::CMD_LogMessage, logMessage), getComponentThread());
}
