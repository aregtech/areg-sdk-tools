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

void LogObserver::callbackObserverConfigured(bool /* isEnabled */, const char* /* address */, uint16_t /* port */)
{
}

void LogObserver::callbackDatabaseConfigured(bool /* isEnabled */, const char* /* dbName */, const char* /* dbLocation */, const char* /* user */)
{
}

void LogObserver::callbackServiceConnected(bool isConnected, const char* address, uint16_t port)
{
    if (isConnected)
    {
        _logConnect.lcAddress = address;
        _logConnect.lcPort = port;
    }
    else
    {
        _listInstances.clear();
        _logConnect.lcAddress.clear();
        _logConnect.lcPort = NESocket::InvalidPort;
    }
}

void LogObserver::callbackObserverStarted(bool /* isStarted */)
{
}

void LogObserver::callbackMessagingFailed(void)
{
}

void LogObserver::callbackConnectedInstances(const sLogInstance* instances, uint32_t count)
{
    if (count == 0)
    {
        _listInstances.clear();
        _mapScopes.clear();
        return;
    }

    for (uint32_t i = 0; i < count; ++i)
    {
        const sLogInstance& inst{ instances[i] };
        bool contains{ false };
        for (uint32_t j = 0; j < _listInstances.getSize(); ++j)
        {
            if (_listInstances[j].liCookie == inst.liCookie)
            {
                contains = true;
                break;
            }
        }

        if (contains == false)
        {
            NELogging::sLogMessage log{ };
            log.logDataType = NELogging::eLogDataType::LogDataLocal;
            log.logMsgType = NELogging::eLogMessageType::LogMessageText;
            log.logMessagePrio = NELogging::eLogPriority::PrioAny;
            log.logSource = inst.liSource;
            log.logTarget = NEService::COOKIE_LOCAL;
            log.logCookie = inst.liCookie;
            log.logModuleId = 0u;
            log.logThreadId = 0u;
            log.logTimestamp = inst.liTimestamp;
            log.logScopeId = 0u;
            log.logMessageLen = static_cast<uint32_t>(String::formatString(log.logMessage, NELogging::LOG_MESSAGE_IZE, "CONNECTED the x%u instance %s with cookie %llu", inst.liBitness, inst.liName, inst.liCookie));
            log.logThreadLen = 0;
            log.logThread[0] = String::EmptyChar;
            log.logModuleId = 0;
            log.logModuleLen = static_cast<uint32_t>(NEString::copyString(log.logModule, NELogging::LOG_NAMES_SIZE, inst.liName));

            _listInstances.add(inst);
            NELogging::logAnyMessage(log);

            ASSERT(_mapScopes.contains(inst.liCookie) == false);
            ::logObserverRequestScopes(inst.liCookie);
        }
    }
}

void LogObserver::callbackDisconnecteInstances(const ITEM_ID* instances, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i)
    {
        const ITEM_ID& cookie = instances[i];
        for (uint32_t j = 0; j < _listInstances.getSize(); ++j)
        {
            const sLogInstance& inst{ _listInstances[j] };
            if (inst.liCookie == cookie)
            {
                NELogging::sLogMessage log{ };
                log.logDataType = NELogging::eLogDataType::LogDataLocal;
                log.logMsgType = NELogging::eLogMessageType::LogMessageText;
                log.logMessagePrio = NELogging::eLogPriority::PrioAny;
                log.logSource = inst.liSource;
                log.logTarget = NEService::COOKIE_LOCAL;
                log.logCookie = inst.liCookie;
                log.logModuleId = 0u;
                log.logThreadId = 0u;
                log.logTimestamp = static_cast<TIME64>(DateTime::getNow());
                log.logScopeId = 0u;
                log.logMessageLen = static_cast<uint32_t>(String::formatString(log.logMessage, NELogging::LOG_MESSAGE_IZE, "DISCONNECTED the x%u instance %s with cookie %llu", inst.liBitness, inst.liName, inst.liCookie));
                log.logThreadLen = 0;
                log.logThread[0] = String::EmptyChar;
                log.logModuleId = 0;
                log.logModuleLen = static_cast<uint32_t>(NEString::copyString(log.logModule, NELogging::LOG_NAMES_SIZE, inst.liName));

                _listInstances.removeAt(j, 1);
                _mapScopes.removeAt(cookie);

                NELogging::logAnyMessage(log);
                break;
            }
        }
    }
}

void LogObserver::callbackLogScopes(ITEM_ID cookie, const sLogScope* scopes, uint32_t count)
{
    for (uint32_t i = 0; i < _listInstances.getSize(); ++i)
    {
        const sLogInstance& inst{ _listInstances[i] };
        if (cookie == inst.liCookie)
        {
            NELogging::sLogMessage log{ };
            log.logDataType = NELogging::eLogDataType::LogDataLocal;
            log.logMsgType = NELogging::eLogMessageType::LogMessageText;
            log.logMessagePrio = NELogging::eLogPriority::PrioAny;
            log.logSource = inst.liSource;
            log.logTarget = NEService::COOKIE_LOCAL;
            log.logCookie = inst.liCookie;
            log.logModuleId = 0u;
            log.logThreadId = 0u;
            log.logTimestamp = static_cast<TIME64>(DateTime::getNow());
            log.logScopeId = 0u;
            log.logMessageLen = static_cast<uint32_t>(String::formatString(log.logMessage, NELogging::LOG_MESSAGE_IZE, "Registered %u scopes for instance %s with cookie %llu", count, inst.liName, inst.liCookie));
            log.logThreadLen = 0;
            log.logThread[0] = String::EmptyChar;
            log.logModuleId = 0;
            log.logModuleLen = static_cast<uint32_t>(NEString::copyString(log.logModule, NELogging::LOG_NAMES_SIZE, inst.liName));

            _mapScopes.setAt(cookie, ListScopes());
            ListScopes& scopeList{ _mapScopes.getAt(cookie) };
            scopeList.resize(count);
            for (uint32_t j = 0; j < count; ++j)
            {
                scopeList[j] = scopes[j];
            }

            NELogging::logAnyMessage(log);
            break;
        }
    }
}

void LogObserver::callbackLogMessageEx(const unsigned char* logBuffer, uint32_t size)
{
    if (logBuffer != nullptr)
    {
        ASSERT(size >= sizeof(NELogging::sLogMessage));
        const NELogging::sLogMessage& log{ reinterpret_cast<const NELogging::sLogMessage&>(*logBuffer) };
        NELogging::logAnyMessage(log);
    }
}
