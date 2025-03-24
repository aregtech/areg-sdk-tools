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

#include "areg/appbase/NEApplication.hpp"
#include "areg/base/DateTime.hpp"

Component * LogObserverComp::CreateComponent(const NERegistry::ComponentEntry & entry, ComponentThread & owner)
{
    return DEBUG_NEW LogObserverComp( entry, owner, entry.getComponentData());
}

void LogObserverComp::DeleteComponent(Component & compObject, const NERegistry::ComponentEntry & /* entry */)
{
    delete (&compObject);
}

QString LogObserverComp::getConnectedAddress(void) const
{
    return mLogObserver.getConectionAddress();
}

uint32_t LogObserverComp::getConnectedPort(void) const
{
    return mLogObserver.getConnectionPort();
}

bool LogObserverComp::isObserverConnected(void) const
{
    return mLogObserver.isLoggingConnected();
}

LogObserver& LogObserverComp::getLogObserver(void)
{
    return mLogObserver;
}

const sLogMessage* LogObserverComp::getLogMessage(uint32_t pos) const
{
    if (pos < mLogObserver.mLogMessages.getSize())
    {
        return reinterpret_cast<const sLogMessage*>(mLogObserver.mLogMessages[pos].getBuffer());
    }
    else
    {
        return nullptr;
    }
}

LogObserverComp::LogObserverComp(const NERegistry::ComponentEntry & entry, ComponentThread & ownerThread, NEMemory::uAlign OPT /* data */)
    : Component ( entry, ownerThread )
    , StubBase  ( self(), NEService::getEmptyInterface() )
    , IELogObserverEventConsumer()
    , mLogObserver()
    , mConfigFile (NEApplication::DEFAULT_CONFIG_FILE)
{
}

LogObserverComp::~LogObserverComp(void)
{

}

void LogObserverComp::startupServiceInterface(Component & holder)
{
    StubBase::startupServiceInterface(holder);
    mLogObserver.loggingStop();
    mLogObserver.loggingClear();
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
    case LogObserverEventData::eLogObserverEvent::CMD_Connect:
        mLogObserver.loggingStart(mConfigFile);
        break;

    case LogObserverEventData::eLogObserverEvent::CMD_Disconnect:
        mLogObserver.loggingStop();
        break;

    case LogObserverEventData::eLogObserverEvent::CMD_Pause:
        mLogObserver.loggingPause();
        break;

    case LogObserverEventData::eLogObserverEvent::CMD_Resume:
        mLogObserver.loggingResume();
        break;

    case LogObserverEventData::eLogObserverEvent::CMD_QueryInstances:
        mLogObserver.loggingRequestScopes(NEService::COOKIE_ANY);
        break;

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

    case LogObserverEventData::eLogObserverEvent::CMD_ScopesRegistered:
    {
        ITEM_ID inst{};
        uint32_t count{};
        data >> inst >> count;
        const sLogScope* scopes = reinterpret_cast<const sLogScope*>(data.getBuffer().getBufferAtCurrentPosition());
        logScopesRegistered(inst, scopes, count);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_ScopesUpdated:
    {
        ITEM_ID inst{};
        uint32_t count{};
        data >> inst >> count;
        const sLogScope* scopes = reinterpret_cast<const sLogScope*>(data.getBuffer().getBufferAtCurrentPosition());
        logScopesUpdated(inst, scopes, count);
    }
    break;

    case LogObserverEventData::eLogObserverEvent::CMD_LogMessageEx:
        logMessageEx(const_cast<SharedBuffer &>(data.getBuffer()));
        break;

    case LogObserverEventData::eLogObserverEvent::CMD_LogPiroirity:
    {
        ITEM_ID inst{NEService::COOKIE_ANY};
        uint32_t count{};
        data >> inst >> count;
        const sLogScope* scopes = reinterpret_cast<const sLogScope*>(data.getBuffer().getBufferAtCurrentPosition());
        mLogObserver.loggingRequestChangeScopePrio(inst, scopes, count);
    }
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
            SharedBuffer buf(sizeof(NELogging::sLogMessage), NEMemory::BLOCK_SIZE);
            NELogging::sLogMessage* log = reinterpret_cast<NELogging::sLogMessage *>(buf.getBuffer());
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
            log->logMessageLen = static_cast<uint32_t>(String::formatString(log->logMessage, NELogging::LOG_MESSAGE_IZE, "CONNECTED the x%u instance of %s", inst.liBitness, inst.liName, inst.liCookie));
            log->logThreadLen = 0;
            log->logThread[0] = String::EmptyChar;
            log->logModuleId = 0;
            log->logModuleLen = static_cast<uint32_t>(NEString::copyString(log->logModule, NELogging::LOG_NAMES_SIZE, inst.liName));

            mLogObserver.mLogSources.add(inst);
            mLogObserver.mLogMessages.add(buf);

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
                SharedBuffer buf(sizeof(NELogging::sLogMessage), NEMemory::BLOCK_SIZE);
                NELogging::sLogMessage* log = reinterpret_cast<NELogging::sLogMessage *>(buf.getBuffer());
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
                log->logMessageLen = static_cast<uint32_t>(String::formatString(log->logMessage, NELogging::LOG_MESSAGE_IZE, "DISCONNECTED the x%u instance %s", inst.liBitness, inst.liName, inst.liCookie));
                log->logThreadLen = 0;
                log->logThread[0] = String::EmptyChar;
                log->logModuleId = 0;
                log->logModuleLen = static_cast<uint32_t>(NEString::copyString(log->logModule, NELogging::LOG_NAMES_SIZE, inst.liName));

                mLogObserver.mLogSources.removeAt(j, 1);
                mLogObserver.mLogScopes.removeAt(cookie);

                mLogObserver.mLogMessages.add(buf);
                break;
            }
        }
    }
}

void LogObserverComp::logScopesRegistered(ITEM_ID target, const sLogScope* scopes, uint32_t count)
{
    for (uint32_t i = 0; i < mLogObserver.mLogSources.getSize(); ++i)
    {
        const sLogInstance& inst{ mLogObserver.mLogSources[i] };
        if (target == inst.liCookie)
        {
            SharedBuffer buf(sizeof(NELogging::sLogMessage), NEMemory::BLOCK_SIZE);
            NELogging::sLogMessage* log = reinterpret_cast<NELogging::sLogMessage *>(buf.getBuffer());
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
            log->logMessageLen = static_cast<uint32_t>(String::formatString(log->logMessage, NELogging::LOG_MESSAGE_IZE, "Registered %u scopes of instance %s", count, inst.liName, inst.liCookie));
            log->logThreadLen = 0;
            log->logThread[0] = String::EmptyChar;
            log->logModuleId = 0;
            log->logModuleLen = static_cast<uint32_t>(NEString::copyString(log->logModule, NELogging::LOG_NAMES_SIZE, inst.liName));
            if (mLogObserver.mLogScopes.contains(target) == false)
            {
                mLogObserver.mLogScopes.setAt(target, LogObserver::ListScopes());
            }

            LogObserver::ListScopes& scopeList{ mLogObserver.mLogScopes.getAt(target) };
            uint32_t shift = scopeList.getSize();
            scopeList.resize(shift + count);
            for (uint32_t j = 0; j < count; ++j)
            {
                scopeList[j + shift] = scopes[j];
            }

            mLogObserver.mLogMessages.add(buf);
            break;
        }
    }
}

void LogObserverComp::logScopesUpdated(ITEM_ID target, const sLogScope* scopes, uint32_t count)
{
    if (mLogObserver.mLogScopes.contains(target))
    {
        LogObserver::ListScopes& scopeList{ mLogObserver.mLogScopes.getAt(target) };
        for (uint32_t i = 0; i < count; ++i)
        {
            const sLogScope& scope{ scopes[i] };
            for (uint32_t j = 0; j < scopeList.getSize(); ++j)
            {
                if (scopeList[j].lsId == scope.lsId)
                {
                    scopeList[j].lsPrio = scope.lsPrio;
                    break;
                }
            }
        }
    }
}


void LogObserverComp::logMessageEx(SharedBuffer& message)
{
    if (message.isValid())
    {
        mLogObserver.mLogMessages.add(message);
    }
}

void LogObserverComp::requestChangeScopePrio(ITEM_ID target, const sLogScope* scopes, uint32_t count)
{
    mLogObserver.loggingRequestChangeScopePrio(target, scopes, count);
}
