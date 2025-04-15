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
 *  \file        lusan/common/LogObserverClient.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer client object.
 *
 ************************************************************************/
#include "lusan/common/LogObserverClient.hpp"
#include "areglogger/client/LogObserverApi.h"
#include "areg/base/SharedBuffer.hpp"
#include "areg/base/String.hpp"
#include "areg/base/TEArrayList.hpp"
#include "areg/base/TEMap.hpp"

LogObserverClient& LogObserverClient::getInstance(void)
{
    static LogObserverClient _instance;
    return _instance;
}

LogObserverClient::LogObserverClient(void)
    : QObject           ( )
    , LogObserverBase   ( )
{
}

void LogObserverClient::onLogObserverConfigured(bool isEnabled, const String& address, uint16_t port)
{
    emit signalLogObserverConfigured(isEnabled, address.getData(), port);
}

void LogObserverClient::onLogDbConfigured(bool isEnabled, const String& dbName, const String& dbLocation, const String& dbUser)
{
    emit signalLogDbConfigured(isEnabled, dbName.getData(), dbLocation.getData(), dbUser.getData());
}

void LogObserverClient::onLogServiceConnected(bool isConnected, const String& address, uint16_t port)
{
    emit signalLogServiceConnected(isConnected, address.getData(), port);
}

void LogObserverClient::onLogObserverStarted(bool isStarted)
{
    emit signalLogObserverStarted(isStarted);
}

void LogObserverClient::onLogDbCreated(const String& dbLocation)
{
    emit signalLogDbCreated(dbLocation.getData());
}

void LogObserverClient::onLogMessagingFailed(void)
{
    emit signalLogMessagingFailed();
}

void LogObserverClient::onLogInstancesConnect(const TEArrayList<NEService::sServiceConnectedInstance>& instances)
{
    emit signalLogInstancesConnect(instances.getData());
}

void LogObserverClient::onLogInstancesDisconnect(const TEArrayList<NEService::sServiceConnectedInstance>& instances)
{
    emit signalLogInstancesDisconnect(instances.getData());
}

void LogObserverClient::onLogServiceDisconnected(const NEService::MapInstances& instances)
{
    emit signalLogServiceDisconnected(instances.getData());
}

void LogObserverClient::onLogRegisterScopes(ITEM_ID cookie, const sLogScope* scopes, int count)
{
    emit signalLogRegisterScopes(cookie, scopes, count);
}

void LogObserverClient::onLogUpdateScopes(ITEM_ID cookie, const sLogScope* scopes, int count)
{
    emit signalLogUpdateScopes(cookie, scopes, count);
}

void LogObserverClient::onLogMessage(const SharedBuffer& logMessage)
{
    emit signalLogMessage(reinterpret_cast<const sLogMessage *>(logMessage.getBuffer()));
}
