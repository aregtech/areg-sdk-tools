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
 *  \file        lusan/data/log/LogObserverComp.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer component with empty service provider.
 *
 ************************************************************************/
 /************************************************************************
  * Include files.
  ************************************************************************/
#include "lusan/data/log/LogObserverComp.hpp"


Component * LogObserverComp::CreateComponent(const NERegistry::ComponentEntry & entry, ComponentThread & owner)
{
    return DEBUG_NEW LogObserverComp( entry, owner, entry.getComponentData());
}

void LogObserverComp::DeleteComponent(Component & compObject, const NERegistry::ComponentEntry & /* entry */)
{
    delete (&compObject);
}

LogObserverComp::LogObserverComp(const NERegistry::ComponentEntry & entry, ComponentThread & ownerThread, NEMemory::uAlign OPT /* data */)
    : Component ( entry, ownerThread )
    , StubBase  ( self(), NEService::getEmptyInterface() )
    , IELogObserverEventConsumer()
    , mLogObserver()

{
}

LogObserverComp::~LogObserverComp(void)
{

}

void LogObserverComp::startupServiceInterface(Component & holder)
{
    StubBase::startupServiceInterface(holder);
}

void LogObserverComp::shutdownServiceIntrface(Component & holder)
{
    mLogObserver.loggingStop();
    mLogObserver.loggingClear();
    StubBase::shutdownServiceIntrface(holder);
}

void LogObserverComp::processEvent(const LogObserverEventData& data)
{
    switch (data.getEvent())
    {
    case LogObserverEventData::eLogObserverEvent::CMD_Connected:
    {
        String address;
        uint16_t port;
        data >> address >> port;
        mLogObserver.setLogCollector(address, port);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_Clear:
        mLogObserver.loggingClear();
        break;

    case LogObserverEventData::eLogObserverEvent::CMD_ConnecedInst:
    {
        const SharedBuffer& buffer = data.getBuffer();
        uint32_t count = buffer.getSizeUsed() / sizeof(sLogInstance);
        const sLogInstance* instances = reinterpret_cast<const sLogInstance*>(buffer.getBuffer());
        connectedInstances(instances, count);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_DisconnecedInst:
    {
        const SharedBuffer& buffer = data.getBuffer();
        uint32_t count = buffer.getSizeUsed() / sizeof(ITEM_ID);
        const ITEM_ID* instances = reinterpret_cast<const ITEM_ID*>(buffer.getBuffer());
        disconnectedInstances(instances, count);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_LogScopes:
    {
        ITEM_ID inst{};
        uint32_t size{};
        data >> inst >> size;
        const sLogScope* scopes = reinterpret_cast<const sLogScope*>(data.getBuffer().getBufferAtCurrentPosition());
        logScopes(inst, scopes, size);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_LogMessageEx:
        logMessageEx(data.getBuffer());
        break;

    default:
        break;
    }
}

void LogObserverComp::connectedInstances(const sLogInstance* instances, uint32_t count)
{
    if (count == 0)
    {
        mLogObserver.loggingClear();
        return;
    }

    for (uint32_t i = 0; i < count; ++i)
    {
        const sLogInstance& inst{ instances[i] };
        bool contains{ false };
        for (uint32_t j = 0; j < mLogObserver.mLogSources.getSize(); ++j)
        {
            if (mLogObserver.mLogSources[j].liCookie == inst.liCookie)
            {
                contains = true;
                break;
            }
        }

        if (contains == false)
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
            log->logTimestamp = inst.liTimestamp;
            log->logScopeId = 0u;
            log->logMessageLen = static_cast<uint32_t>(String::formatString(log->logMessage, NELogging::LOG_MESSAGE_IZE, "CONNECTED the x%u instance %s with cookie %llu", inst.liBitness, inst.liName, inst.liCookie));
            log->logThreadLen = 0;
            log->logThread[0] = String::EmptyChar;
            log->logModuleId = 0;
            log->logModuleLen = static_cast<uint32_t>(NEString::copyString(log->logModule, NELogging::LOG_NAMES_SIZE, inst.liName));

            mLogObserver.mLogSources.add(inst);
            mLogObserver.mLogMessages.pushLast(log);

            ASSERT(mLogObserver.mLogScopes.contains(inst.liCookie) == false);
            ::logObserverRequestScopes(inst.liCookie);
        }
    }
}

void LogObserverComp::disconnectedInstances(const ITEM_ID* instances, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i)
    {
        const ITEM_ID& cookie = instances[i];
        for (uint32_t j = 0; j < mLogObserver.mLogSources.getSize(); ++j)
        {
            const sLogInstance& inst{ mLogObserver.mLogSources[j] };
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

                mLogObserver.mLogSources.removeAt(j, 1);
                mLogObserver.mLogScopes.removeAt(cookie);

                mLogObserver.mLogMessages.pushLast(log);
                break;
            }
        }
    }
}

void LogObserverComp::logScopes(ITEM_ID cookie, const sLogScope* scopes, uint32_t count)
{
    for (uint32_t i = 0; i < mLogObserver.mLogSources.getSize(); ++i)
    {
        const sLogInstance& inst{ mLogObserver.mLogSources[i] };
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
            if (mLogObserver.mLogScopes.contains(cookie) == false)
            {
                mLogObserver.mLogScopes.setAt(cookie, ListScopes());
            }

            ListScopes& scopeList{ mLogObserver.mLogScopes.getAt(cookie) };
            scopeList.resize(count);
            for (uint32_t j = 0; j < count; ++j)
            {
                scopeList[j] = scopes[j];
            }

            mLogObserver.mLogMessages.pushLast(log);
            break;
        }
    }
}

void LogObserverComp::logMessageEx(SharedBuffer& message)
{
    if (message.isValid())
    {
        mLogObserver.mLogMessages.pushLast(message);
    }
}
