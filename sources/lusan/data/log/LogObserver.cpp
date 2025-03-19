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
 *  \file        lusan/data/log/LogObserver.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer data object to get logs on real-time mode.
 *
 ************************************************************************/

#include "lusan/data/log/LogObserver.hpp"

#include "areg/base/DateTime.hpp"

LogObserver& LogObserver::getInstance(void)
{
    static LogObserver  _log;
    return _log;
}

void LogObserver::logingStart(void)
{
    if (::logObserverInitialize(&mEvents, nullptr))
    {
        ::logObserverConnectLogger(nullptr, nullptr, 0);
    }
}

void LogObserver::loggingStart(const QString& dbPath, const QString& address, unsigned short port)
{
    if (::logObserverInitialize(&mEvents, nullptr))
    {
        ::logObserverConnectLogger(dbPath.isEmpty() ? nullptr : dbPath.toStdString().c_str(), address.isEmpty() ? nullptr : address.toStdString().c_str(), port);
    }
}

void LogObserver::loggingStart(const QString& configPath)
{
    if (::logObserverInitialize(&mEvents, configPath.isEmpty() ? nullptr : configPath.toStdString().c_str()))
    {
        ::logObserverConnectLogger(nullptr, nullptr, 0);
    }
}

void LogObserver::loggingStop(void)
{
    ::logObserverRelease();
    ::logObserverDisconnectLogger();
}

void LogObserver::loggingPause(void)
{
    ::logObserverPauseLogging(true);
}

void LogObserver::loggingResume(void)
{
    ::logObserverPauseLogging(false);
}

void LogObserver::loggingClear(void)
{
    _clear();
}

void LogObserver::loggingRequestInstances(void)
{
    ::logObserverRequestInstances();
}

void LogObserver::loggingRequestScopes(ITEM_ID target)
{
    ::logObserverRequestScopes(target);
}

void LogObserver::loggingRequestChangeScopePrio(ITEM_ID target, const sLogScope* scopes, uint32_t count)
{
    ::logObserverRequestChangeScopePrio(target, scopes, count);
}

void LogObserver::loggingSaveConfiguration(ITEM_ID target)
{
    ::logObserverRequestSaveConfig(target);
}

bool LogObserver::isLoggingInitialized(void) const
{
    return ::logObserverIsInitialized();
}

bool LogObserver::isLoggingConnected(void) const
{
    return ::logObserverIsConnected();
}

bool LogObserver::isLoggingStarted(void) const
{
    return ::logObserverIsStarted();
}

bool LogObserver::isLoggingEnabled(void) const
{
    return ::logObserverConfigLoggerEnabled();
}

bool LogObserver::isLoggingPaused(void) const
{
    return (::logObserverCurrentState() == eObserverStates::ObserverPaused);
}

QString LogObserver::getConectionAddress(void) const
{
    return QString(::logObserverLoggerAddress());
}

unsigned short LogObserver::getConnectionPort(void) const
{
    return ::logObserverLoggerPort();
}

LogObserver::LogObserver(void)
    : mLogConnect   ()
    , mLogSources   ()
    , mLogScopes    ()
    , mLogMessages  ()
    , mEvents       ()
{
    mEvents.evtObserverConfigured   = &LogObserver::callbackObserverConfigured;
    mEvents.evtLogDbConfigured      = &LogObserver::callbackDatabaseConfigured;
    mEvents.evtServiceConnected     = &LogObserver::callbackServiceConnected;
    mEvents.evtLoggingStarted       = &LogObserver::callbackObserverStarted;
    mEvents.evtMessagingFailed      = &LogObserver::callbackMessagingFailed;
    mEvents.evtInstConnected        = &LogObserver::callbackConnectedInstances;
    mEvents.evtInstDisconnected     = &LogObserver::callbackDisconnecteInstances;
    mEvents.evtLogRegisterScopes    = &LogObserver::callbackLogScopes;
    mEvents.evtLogUpdatedScopes     = &LogObserver::callbackLogScopes;
    mEvents.evtLogMessage           = nullptr;
    mEvents.evtLogMessageEx         = &LogObserver::callbackLogMessageEx;
}

LogObserver::~LogObserver(void)
{
    _clear();
}

void LogObserver::callbackObserverConfigured(bool /* isEnabled */, const char* /* address */, uint16_t /* port */)
{
}

void LogObserver::callbackDatabaseConfigured(bool /* isEnabled */, const char* /* dbName */, const char* /* dbLocation */, const char* /* user */)
{
}

void LogObserver::callbackServiceConnected(bool isConnected, const char* address, uint16_t port)
{
    LogObserver & _log = LogObserver::getInstance();
    if (isConnected)
    {
        _log.mLogConnect.lcAddress = address;
        _log.mLogConnect.lcPort = port;
    }
    else
    {
        _log.mLogConnect.lcAddress.clear();
        _log.mLogConnect.lcPort = NESocket::InvalidPort;
    }
}

void LogObserver::callbackObserverStarted(bool /* isStarted */)
{
    LogObserver & _log = LogObserver::getInstance();
    _log._clear();

}

void LogObserver::callbackMessagingFailed(void)
{
}

void LogObserver::callbackConnectedInstances(const sLogInstance* instances, uint32_t count)
{
    LogObserver & _log = LogObserver::getInstance();

    if (count == 0)
    {
        _log.mLogSources.clear();
        _log.mLogScopes.clear();
        return;
    }

    for (uint32_t i = 0; i < count; ++i)
    {
        const sLogInstance& inst{ instances[i] };
        bool contains{ false };
        for (uint32_t j = 0; j < _log.mLogSources.getSize(); ++j)
        {
            if (_log.mLogSources[j].liCookie == inst.liCookie)
            {
                contains = true;
                break;
            }
        }

        if (contains == false)
        {
            NELogging::sLogMessage * log = DEBUG_NEW NELogging::sLogMessage;
            log->logDataType = NELogging::eLogDataType::LogDataLocal;
            log->logMsgType = NELogging::eLogMessageType::LogMessageText;
            log->logMessagePrio = NELogging::eLogPriority::PrioAny;
            log->logSource = inst.liSource;
            log->logTarget = NEService::COOKIE_LOCAL;
            log->logCookie = inst.liCookie;
            log->logModuleId = 0u;
            log->logThreadId = 0u;
            log->logTimestamp = inst.liTimestamp;
            log->logScopeId = 0u;
            log->logMessageLen = static_cast<uint32_t>(String::formatString(log->logMessage, NELogging::LOG_MESSAGE_IZE, "CONNECTED the x%u instance %s with cookie %llu", inst.liBitness, inst.liName, inst.liCookie));
            log->logThreadLen = 0;
            log->logThread[0] = String::EmptyChar;
            log->logModuleId = 0;
            log->logModuleLen = static_cast<uint32_t>(NEString::copyString(log->logModule, NELogging::LOG_NAMES_SIZE, inst.liName));

            _log.mLogSources.add(inst);
            _log.mLogMessages.pushLast(log);

            ASSERT(_log.mLogScopes.contains(inst.liCookie) == false);
            ::logObserverRequestScopes(inst.liCookie);
        }
    }
}

void LogObserver::callbackDisconnecteInstances(const ITEM_ID* instances, uint32_t count)
{
    LogObserver & _log = LogObserver::getInstance();

    for (uint32_t i = 0; i < count; ++i)
    {
        const ITEM_ID& cookie = instances[i];
        for (uint32_t j = 0; j < _log.mLogSources.getSize(); ++j)
        {
            const sLogInstance& inst{ _log.mLogSources[j] };
            if (inst.liCookie == cookie)
            {
                NELogging::sLogMessage* log = DEBUG_NEW NELogging::sLogMessage;
                log->logDataType = NELogging::eLogDataType::LogDataLocal;
                log->logMsgType = NELogging::eLogMessageType::LogMessageText;
                log->logMessagePrio = NELogging::eLogPriority::PrioAny;
                log->logSource = inst.liSource;
                log->logTarget = NEService::COOKIE_LOCAL;
                log->logCookie = inst.liCookie;
                log->logModuleId = 0u;
                log->logThreadId = 0u;
                log->logTimestamp = static_cast<TIME64>(DateTime::getNow());
                log->logScopeId = 0u;
                log->logMessageLen = static_cast<uint32_t>(String::formatString(log->logMessage, NELogging::LOG_MESSAGE_IZE, "DISCONNECTED the x%u instance %s with cookie %llu", inst.liBitness, inst.liName, inst.liCookie));
                log->logThreadLen = 0;
                log->logThread[0] = String::EmptyChar;
                log->logModuleId = 0;
                log->logModuleLen = static_cast<uint32_t>(NEString::copyString(log->logModule, NELogging::LOG_NAMES_SIZE, inst.liName));

                _log.mLogSources.removeAt(j, 1);
                _log.mLogScopes.removeAt(cookie);

                _log.mLogMessages.pushLast(log);
                break;
            }
        }
    }
}

void LogObserver::callbackLogScopes(ITEM_ID cookie, const sLogScope* scopes, uint32_t count)
{
    LogObserver & _log = LogObserver::getInstance();

    for (uint32_t i = 0; i < _log.mLogSources.getSize(); ++i)
    {
        const sLogInstance& inst{ _log.mLogSources[i] };
        if (cookie == inst.liCookie)
        {
            NELogging::sLogMessage* log = DEBUG_NEW NELogging::sLogMessage;
            log->logDataType = NELogging::eLogDataType::LogDataLocal;
            log->logMsgType = NELogging::eLogMessageType::LogMessageText;
            log->logMessagePrio = NELogging::eLogPriority::PrioAny;
            log->logSource = inst.liSource;
            log->logTarget = NEService::COOKIE_LOCAL;
            log->logCookie = inst.liCookie;
            log->logModuleId = 0u;
            log->logThreadId = 0u;
            log->logTimestamp = static_cast<TIME64>(DateTime::getNow());
            log->logScopeId = 0u;
            log->logMessageLen = static_cast<uint32_t>(String::formatString(log->logMessage, NELogging::LOG_MESSAGE_IZE, "Registered %u scopes for instance %s with cookie %llu", count, inst.liName, inst.liCookie));
            log->logThreadLen = 0;
            log->logThread[0] = String::EmptyChar;
            log->logModuleId = 0;
            log->logModuleLen = static_cast<uint32_t>(NEString::copyString(log->logModule, NELogging::LOG_NAMES_SIZE, inst.liName));

            if (_log.mLogScopes.contains(cookie) == false)
            {
                _log.mLogScopes.setAt(cookie, ListScopes());
            }

            ListScopes& scopeList{ _log.mLogScopes.getAt(cookie) };
            scopeList.resize(count);
            for (uint32_t j = 0; j < count; ++j)
            {
                scopeList[j] = scopes[j];
            }

            _log.mLogMessages.pushLast(log);
            break;
        }
    }
}

void LogObserver::callbackLogMessageEx(const unsigned char* logBuffer, uint32_t size)
{
    LogObserver & _log = LogObserver::getInstance();

    if (logBuffer != nullptr)
    {
        ASSERT(size >= sizeof(NELogging::sLogMessage));
        NELogging::sLogMessage * log = DEBUG_NEW NELogging::sLogMessage;
        memcpy(log, logBuffer, sizeof(NELogging::sLogMessage));
        _log.mLogMessages.pushLast(log);
    }
}

void LogObserver::_clear(void)
{
    while (mLogMessages.isEmpty() == false)
    {
        NELogging::sLogMessage *msg = mLogMessages.popFirst();
        delete msg;
    }

    mLogConnect.lcAddress.clear();
    mLogConnect.lcPort = NESocket::InvalidPort;
    mLogSources.clear();
    mLogScopes.clear();
    mLogMessages.clear();
}
