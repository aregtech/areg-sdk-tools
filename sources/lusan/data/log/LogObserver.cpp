/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/log/LogObserver.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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

#include "areg/appbase/AppDefs.hpp"
#include "areg/base/DateTime.hpp"
#include "areg/component/ComponentLoader.hpp"
#include "areg/component/ComponentThread.hpp"
#include "areglogger/client/LogObserverApi.h"

#include <atomic>

#include <QMetaType>

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
                                   , areg::InvalidElement
                                , ([](const areg::ComponentEntry& e, areg::ComponentThread& t) -> areg::Component * {
                                    return LogObserver::CreateComponent(e, t);
                                })
                                , ([](areg::Component & c, const areg::ComponentEntry & e) -> void {
                                    LogObserver::DeleteComponent(c, e);
                                }))
            REGISTER_IMPLEMENT_SERVICE( areg::EmptyServiceName, areg::EmptyServiceVersion )
        END_REGISTER_COMPONENT(LogObserver::LogObserverComponent)
    END_REGISTER_THREAD(LogObserver::LogobserverThread)

END_MODEL(LogObserver::LogobserverModel)

bool LogObserver::createLogObserver(FuncLogObserverStarted callbackStarted)
{
    bool result{ true };
    if (_modelInitialized.exchange(true) == false)
    {
        Q_ASSERT(areg::ComponentLoader::is_model_loaded(LogObserver::LogobserverModel) == false);

        _observerStart = false;
        _logObserverStarted = callbackStarted;
        result = areg::ComponentLoader::load_component_model(LogObserver::LogobserverModel);
    }
    
    return result;
}

void LogObserver::releaseLogObserver()
{
    _modelInitialized = false;
    _observerStart = false;
    areg::ComponentLoader::unload_component_model(true, LogObserver::LogobserverModel);
}

void LogObserver::disconnect()
{
    static_cast<areg::logger::LogObserverBase &>(LogObserver::getClient()).disconnect();
}

bool LogObserver::pause()
{
    return LogObserver::getClient().pause();
}

bool LogObserver::resume()
{
    return LogObserver::getClient().resume();
}

void LogObserver::stop()
{
    LogObserver::getClient().stop();
}

bool LogObserver::restart(const QString& dbLocation)
{
    return LogObserver::getClient().restart(dbLocation.toStdString());
}

bool LogObserver::requestInstances()
{
    return LogObserver::getClient().request_instances();
}

bool LogObserver::requestScopes(ITEM_ID target /*= areg::TARGET_ALL*/)
{
    return LogObserver::getClient().request_scopes(target);
}

bool LogObserver::requestChangeScopePrio(ITEM_ID target, const ScopeInfo* scopes, uint32_t count)
{
    return LogObserver::getClient().request_change_scope_prio(target, scopes, count);
}

bool LogObserver::requestSaveConfig(ITEM_ID target /*= areg::TARGET_ALL*/)
{
    return LogObserver::getClient().request_save_config(target);
}

bool LogObserver::requestRestoreConfig(ITEM_ID target /*= areg::TARGET_ALL*/)
{
    return LogObserver::getClient().request_restore_config(target);
}

bool LogObserver::requestSourceState(ITEM_ID target, areg::LogSourceState state)
{
    return LogObserver::getClient().request_source_state(target, state);
}

void LogObserver::saveLoggerConfig()
{
    LogObserver::getClient().save_logger_config();
}

const areg::SocketAddress& LogObserver::getLogServiceAddress()
{
    return LogObserver::getClient().logger_address();
}

QString LogObserver::getConnectedAddress()
{
    return QString(LogObserver::getClient().logger_ip_address().c_str());
}

QString LogObserver::getConnectedHostName()
{
    return QString(LogObserver::getClient().logger_host_name().c_str());
}

uint16_t LogObserver::getConnectedPort()
{
    return LogObserver::getClient().logger_port();
}

QString LogObserver::getActiveDatabase()
{
    return QString(LogObserver::getClient().active_database_path().c_str());
}

QString LogObserver::getInitDatabase()
{
    return QString(LogObserver::getClient().init_database_path().c_str());
}

QString LogObserver::getConfigDatabaseName()
{
    return QString(LogObserver::getClient().config_logger_database_name().c_str());
}

QString LogObserver::getConfigDatabaseLocation()
{
    return QString(LogObserver::getClient().config_logger_database_location().c_str());
}

bool LogObserver::setConfigDatabaseName(const QString& dbName)
{
    if (LogObserver::getComponent() != nullptr)
    {
        LogObserver::getClient().set_config_logger_database_name(dbName.toStdString());
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
        LogObserver::getClient().set_config_logger_database_location(dbLocation.toStdString());
        return true;
    }
    else
    {
        return false;
    }
}

bool LogObserver::isConnected()
{
    return LogObserver::getClient().is_connected();
}

bool LogObserver::connect(const QString& address, uint16_t port, const QString& dbLocation)
{
    return static_cast<areg::logger::LogObserverBase &>(LogObserver::getClient()).connect(address.toStdString(), port, dbLocation.toStdString());
}

areg::Component * LogObserver::CreateComponent(const areg::ComponentEntry & entry, areg::ComponentThread & owner)
{
    Q_ASSERT(_component.load() == nullptr);
    _component.store(DEBUG_NEW LogObserver( entry, owner));
    return static_cast<areg::Component *>(_component.load());
}

void LogObserver::DeleteComponent(areg::Component & compObject, const areg::ComponentEntry & /* entry */)
{
    _component.store(nullptr);
    delete (&compObject);
}

LogObserver* LogObserver::getComponent()
{
    return _component.load();
}

void LogObserver::queryLogInstanceNames(std::vector<areg::String>& names)
{
    LogObserver::getClient().log_instance_names(names);
}

void LogObserver::queryLogInstances(std::vector<ITEM_ID>& ids)
{
    LogObserver::getClient().log_instances(ids);
}

void LogObserver::queryLogThreadNames(std::vector<areg::String>& names)
{
    LogObserver::getClient().log_thread_names(names);
}

void LogObserver::queryLogThreads(std::vector<ITEM_ID>& ids)
{
    LogObserver::getClient().log_threads(ids);
}

void LogObserver::queryPriorityNames(std::vector<areg::String>& names)
{
    LogObserver::getClient().log_priority_names(names);
}

void LogObserver::queryLogInstanceInfos(std::vector<areg::ConnectedInstance>& infos)
{
    LogObserver::getClient().log_instance_infos(infos);
}

void LogObserver::queryLogInstScopes(std::vector<areg::ScopeEntry>& scopes, ITEM_ID instId)
{
    LogObserver::getClient().log_inst_scopes(scopes, instId);
}

void LogObserver::queryLogMessages(std::vector<areg::SharedBuffer>& messages)
{
    LogObserver::getClient().log_messages(messages);
}

void LogObserver::queryLogInstMessages(std::vector<areg::SharedBuffer>& messages, ITEM_ID instId)
{
    LogObserver::getClient().log_inst_messages(messages, instId);
}

void LogObserver::queryLogScopeMessages(std::vector<areg::SharedBuffer>& messages, uint32_t scopeId)
{
    LogObserver::getClient().log_scope_messages(messages, scopeId);
}

void LogObserver::queryLogMessages(std::vector<areg::SharedBuffer>& messages, ITEM_ID instId, uint32_t scopeId)
{
    LogObserver::getClient().log_messages(messages, instId, scopeId);
}

LogCollectorClient& LogObserver::getClient()
{
    return LogCollectorClient::getInstance();
}

LogObserver::LogObserver(const areg::ComponentEntry & entry, areg::ComponentThread & ownerThread)
    : areg::Component ( entry, ownerThread )
    , areg::StubBase  ( self(), areg::empty_interface() )
    , LogObserverEventConsumer()

    , mLogClient    (LogCollectorClient::getInstance())
    , mConfigFile   (NELusanCommon::INIT_FILE.toStdString())
    , mIsRunning    (false)
{
}

LogObserver::~LogObserver()
{
}

void LogObserver::startup_service_interface(areg::Component & holder)
{
    areg::StubBase::startup_service_interface(holder);
    static_cast<areg::logger::LogObserverBase &>(mLogClient).stop();
    static_cast<areg::logger::LogObserverBase &>(mLogClient).disconnect();

    FuncLogObserverStarted callback = _logObserverStarted.load();
    if (callback != nullptr)
    {
        callback();
    }
    
    LogObserverEvent::add_listener(static_cast<LogObserverEventConsumer &>(self()), master_thread());

    // The dispatcher is listening, so the callbacks connected below may reach it.
    mIsRunning.store(true, std::memory_order_release);

    qRegisterMetaType<areg::SharedBuffer>("areg::SharedBuffer");

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
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogSourceState          , this, &LogObserver::slotLogSourceState           , Qt::DirectConnection);
    QObject::connect(&mLogClient, &LogCollectorClient::signalLogConfigRestored       , this, &LogObserver::slotLogConfigRestored        , Qt::DirectConnection);
    
    if (mLogClient.is_initialized() == false)
    {
        mLogClient.initialize(mConfigFile.data());
    }
    
    QString address { mLogClient.logger_ip_address().c_str() };
    uint16_t port   { mLogClient.logger_port() };
    QString logFile { mLogClient.active_database_path().c_str() };

    emit signalLogObserverInstance(true, address, port, logFile);
}

void LogObserver::shutdown_service_interface(Component & holder) noexcept
{
    // The socket threads of the client keep calling back while it stops. Close that door
    // first: drop the callbacks still in flight, then take the connections away, and only
    // then dismantle the client and the dispatcher they were delivering to.
    mIsRunning.store(false, std::memory_order_release);

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
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogSourceState           , this, &LogObserver::slotLogSourceState);
    QObject::disconnect(&mLogClient, &LogCollectorClient::signalLogConfigRestored        , this, &LogObserver::slotLogConfigRestored);

    QString address { mLogClient.logger_ip_address().c_str() };
    uint16_t port   { mLogClient.logger_port() };
    QString logFile { mLogClient.active_database_path().c_str() };

    emit signalLogObserverInstance(false, address, port, logFile);

    static_cast<areg::logger::LogObserverBase &>(mLogClient).stop();
    static_cast<areg::logger::LogObserverBase &>(mLogClient).disconnect();
    StubBase::shutdown_service_interface(holder);

    LogObserverEvent::remove_listener(static_cast<LogObserverEventConsumer &>(self()), master_thread());
}

void LogObserver::postObserverEvent(LogObserverEventData && data)
{
    if (mIsRunning.load(std::memory_order_acquire))
    {
        LogObserverEvent::send_event(std::move(data), master_thread());
    }
}

void LogObserver::send_notification(unsigned int  /*msgId*/)
{
}

void LogObserver::error_request(unsigned int /*msgId*/, bool /*msgCancel*/)
{
}

void LogObserver::process_request_event(areg::ServiceRequestEvent& /*eventElem*/)
{
}

void LogObserver::process_attribute_event(areg::ServiceRequestEvent& /*eventElem*/)
{
}

void LogObserver::process_event(const LogObserverEventData& data)
{
    const areg::SharedBuffer& stream{ data.getBuffer() };
    stream.move_to_begin();

    switch (data.getEvent())
    {
    case LogObserverEventData::LogObserverCommand::CMD_Configured:
    {
        bool isEnabled{ false };
        uint16_t port{ areg::InvalidPort };
        std::string address;
        stream >> isEnabled >> address >> port;
        emit signalLogObserverConfigured(isEnabled, QString::fromStdString(address), port);
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_DbConfigured:
    {
        bool isEnabled{ false };
        std::string dbName;
        std::string dbLocation;
        std::string dbUser;
        stream >> isEnabled >> dbName >> dbLocation >> dbUser;
        emit signalLogDbConfigured(isEnabled, QString::fromStdString(dbName), QString::fromStdString(dbLocation), QString::fromStdString(dbUser));

    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_Connected:
    {
        bool isConnected{ false };
        std::string address;
        uint16_t port{ areg::InvalidPort };
        stream >> isConnected >> address >> port;
        emit signalLogServiceConnected(isConnected, QString::fromStdString(address), port);
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_Started:
    {
        bool isStarted{ false };
        stream >> isStarted;
        emit signalLogObserverStarted(isStarted);
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_DbCreated:
    {
        std::string dbLocation;
        stream >> dbLocation;
        emit signalLogDbCreated(QString::fromStdString(dbLocation));
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_MessageFailed:
    {
        emit signalLogMessagingFailed();
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_InstConnected:
    {
        uint32_t count{ 0 };
        stream >> count;

        std::vector< areg::ConnectedInstance> instances(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            areg::ConnectedInstance& instance = instances[i];
            stream >> instance;
        }

        emit signalLogInstancesConnect(instances);
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_InstDisconnected:
    {
        uint32_t count{ 0 };
        stream >> count;

        std::vector< areg::ConnectedInstance> instances(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            areg::ConnectedInstance& instance = instances[i];
            stream >> instance;
        }

        emit signalLogInstancesDisconnect(instances);
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_ServiceDisconnect:
    {
        emit signalLogServiceDisconnected();
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_ScopesRegistered:
    {
        ITEM_ID inst{ areg::COOKIE_ANY };
        uint32_t count{ 0 };
        stream >> inst >> count;

        std::vector<areg::ScopeEntry> scopes(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            ScopeInfo data{};
            areg::ScopeEntry & scope = scopes[i];
            stream.read(reinterpret_cast<unsigned char *>(&data), sizeof(ScopeInfo));
            scope.scopeId   = data.lsId;
            scope.scopePrio = data.lsPrio;
            scope.scopeName = data.lsName;
        }

        emit signalLogRegisterScopes(inst, scopes);
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_ScopesUpdated:
    {
        ITEM_ID inst{ areg::COOKIE_ANY };
        uint32_t count{ 0 };
        stream >> inst >> count;

        std::vector<areg::ScopeEntry> scopes(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            ScopeInfo data{};
            areg::ScopeEntry & scope = scopes[i];
            stream.read(reinterpret_cast<unsigned char *>(&data), sizeof(ScopeInfo));
            scope.scopeId   = data.lsId;
            scope.scopePrio = data.lsPrio;
            scope.scopeName = data.lsName;
        }

        emit signalLogUpdateScopes(inst, scopes);
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_LogMessage:
    {
        emit signalLogMessage(stream);
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_SourceState:
    {
        ITEM_ID cookie{ areg::COOKIE_ANY };
        uint8_t state{ 0 };
        ITEM_ID byObserver{ areg::COOKIE_ANY };
        stream >> cookie >> state >> byObserver;

        emit signalLogSourceState(cookie, state, byObserver);
    }
    break;

    case LogObserverEventData::LogObserverCommand::CMD_ConfigRestored:
    {
        ITEM_ID cookie{ areg::COOKIE_ANY };
        stream >> cookie;

        emit signalLogConfigRestored(cookie);
    }
    break;

    default:
        break;
    }
}

void LogObserver::slotLogObserverConfigured(bool isEnabled, const std::string& address, uint16_t port)
{
    areg::SharedBuffer stream;
    stream << isEnabled << address << port;
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_Configured, stream));
}

void LogObserver::slotLogDbConfigured(bool isEnabled, const std::string& dbName, const std::string& dbLocation, const std::string& dbUser)
{
    areg::SharedBuffer stream;
    stream << isEnabled << dbName << dbLocation << dbUser;
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_DbConfigured, stream));
}

void LogObserver::slotLogServiceConnected(bool isConnected, const std::string& address, uint16_t port)
{
    areg::SharedBuffer stream;
    stream << isConnected << address << port;
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_Connected, stream));
}

void LogObserver::slotLogObserverStarted(bool isStarted)
{
    areg::SharedBuffer stream;
    stream << isStarted;
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_Started, stream));
}

void LogObserver::slotLogDbCreated(const std::string& dbLocation)
{
    areg::SharedBuffer stream;
    stream << dbLocation;
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_DbCreated, stream));
}

void LogObserver::slotLogMessagingFailed()
{
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_MessageFailed));
}

void LogObserver::slotLogInstancesConnect(const std::vector<areg::ConnectedInstance>& instances)
{
    areg::SharedBuffer stream;
    stream << instances;
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_InstConnected, stream));
}

void LogObserver::slotLogInstancesDisconnect(const std::vector<areg::ConnectedInstance>& instances)
{
    areg::SharedBuffer stream;
    stream << instances;
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_InstDisconnected, stream));
}

void LogObserver::slotLogServiceDisconnected()
{
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_ServiceDisconnect));
}

void LogObserver::slotLogRegisterScopes(ITEM_ID cookie, const ScopeInfo* scopes, int count)
{
    constexpr uint32_t size{ sizeof(ScopeInfo) };
    areg::SharedBuffer stream;
    stream << cookie << count;
    for (int i = 0; i < count; ++i)
    {
        const ScopeInfo& scope{ scopes[i] };
        stream.write(reinterpret_cast<const unsigned char*>(&scope), size);
    }

    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_ScopesRegistered, stream));
}

void LogObserver::slotLogUpdateScopes(ITEM_ID cookie, const ScopeInfo* scopes, int count)
{
    constexpr uint32_t size{ sizeof(ScopeInfo) };
    areg::SharedBuffer stream;
    stream << cookie << count;
    for (int i = 0; i < count; ++i)
    {
        const ScopeInfo& scope{ scopes[i] };
        stream.write(reinterpret_cast<const unsigned char*>(&scope), size);
    }

    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_ScopesUpdated, stream));
}

void LogObserver::slotLogMessage(const areg::MessageEnvelope& logMessage)
{
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_LogMessage, areg::SharedBuffer(logMessage)));
}

void LogObserver::slotLogSourceState(ITEM_ID cookie, uint8_t state, ITEM_ID byObserver)
{
    areg::SharedBuffer stream;
    stream << cookie << state << byObserver;
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_SourceState, stream));
}

void LogObserver::slotLogConfigRestored(ITEM_ID cookie)
{
    areg::SharedBuffer stream;
    stream << cookie;
    postObserverEvent(LogObserverEventData(LogObserverEventData::LogObserverCommand::CMD_ConfigRestored, stream));
}
