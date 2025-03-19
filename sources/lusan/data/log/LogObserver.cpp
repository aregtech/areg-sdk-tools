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

#include "lusan/data/log/LogObserverEvent.hpp"
#include "areg/base/DateTime.hpp"
#include "areg/component/DispatcherThread.hpp"

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
    LogObserverEventData data(LogObserverEventData::eLogObserverEvent::CMD_Connected);
    if (isConnected)
    {
        data << address << port;
    }
    else
    {
        data << "" << NESocket::InvalidPort;
    }

    LogObserverEvent::sendEvent(data, LogObserver::_logObserverThread());
}

void LogObserver::callbackObserverStarted(bool /* isStarted */)
{
    LogObserverEventData data(LogObserverEventData::eLogObserverEvent::CMD_Clear);
    LogObserverEvent::sendEvent(data, LogObserver::_logObserverThread());
}

void LogObserver::callbackMessagingFailed(void)
{
}

void LogObserver::callbackConnectedInstances(const sLogInstance* instances, uint32_t count)
{
    LogObserverEventData data(LogObserverEventData::eLogObserverEvent::CMD_ConnecedInst);
    if (count != 0)
    {
        data.getBuffer().writeData(reinterpret_cast<const unsigned char*>(instances), count * sizeof(sLogInstance));
    }

    LogObserverEvent::sendEvent(data, LogObserver::_logObserverThread());
}

void LogObserver::callbackDisconnecteInstances(const ITEM_ID* instances, uint32_t count)
{
    LogObserverEventData data(LogObserverEventData::eLogObserverEvent::CMD_DisconnecedInst);
    if (count != 0)
    {
        data.getBuffer().writeData(reinterpret_cast<const unsigned char*>(instances), count * sizeof(ITEM_ID));
    }

    LogObserverEvent::sendEvent(data, LogObserver::_logObserverThread());
}

void LogObserver::callbackLogScopes(ITEM_ID cookie, const sLogScope* scopes, uint32_t count)
{
    LogObserverEventData data(LogObserverEventData::eLogObserverEvent::CMD_LogScopes);
    if (count != 0)
    {
        data.getBuffer().write64Bits(cookie);
        data.getBuffer().write32Bits(count);
        data.getBuffer().writeData(reinterpret_cast<const unsigned char*>(scopes), count * sizeof(sLogScope));
    }

    LogObserverEvent::sendEvent(data, LogObserver::_logObserverThread());


    LogObserver & _log = LogObserver::getInstance();

}

void LogObserver::callbackLogMessageEx(const unsigned char* logBuffer, uint32_t size)
{
    LogObserverEventData data(LogObserverEventData::eLogObserverEvent::CMD_LogMessageEx);
    if ((size != 0) && (logBuffer != nullptr))
    {
        data.getBuffer().writeData(logBuffer, size);
    }

    LogObserverEvent::sendEvent(data, LogObserver::_logObserverThread());
}

DispatcherThread& LogObserver::_logObserverThread(void)
{
    return DispatcherThread::getDispatcherThread(NELusanCommon::LogobserverThread);
}

void LogObserver::_clear(void)
{
    mLogConnect.lcAddress.clear();
    mLogConnect.lcPort = NESocket::InvalidPort;
    mLogSources.clear();
    mLogScopes.clear();
    mLogMessages.clear();
}
