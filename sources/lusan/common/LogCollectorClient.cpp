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
 *  \file        lusan/common/LogCollectorClient.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer client object.
 *
 ************************************************************************/
#include "lusan/common/LogCollectorClient.hpp"
#include "areglogger/client/LogObserverApi.h"
#include "areg/base/SharedBuffer.hpp"
#include "areg/base/String.hpp"
#include "areg/base/TEArrayList.hpp"
#include "areg/base/TEMap.hpp"

LogCollectorClient& LogCollectorClient::getInstance(void)
{
    static LogCollectorClient _instance;
    return _instance;
}

LogCollectorClient::LogCollectorClient(void)
    : QObject           ( )
    , LogObserverBase   ( )
{
}

void LogCollectorClient::onLogObserverConfigured(bool isEnabled, const std::string& address, uint16_t port)
{
    emit signalLogObserverConfigured(isEnabled, address, port);
}

void LogCollectorClient::onLogDbConfigured(bool isEnabled, const std::string& dbName, const std::string& dbLocation, const std::string& dbUser)
{
    emit signalLogDbConfigured(isEnabled, dbName, dbLocation, dbUser);
}

void LogCollectorClient::onLogServiceConnected(bool isConnected, const std::string& address, uint16_t port)
{
    emit signalLogServiceConnected(isConnected, address, port);
}

void LogCollectorClient::onLogObserverStarted(bool isStarted)
{
    emit signalLogObserverStarted(isStarted);
}

void LogCollectorClient::onLogDbCreated(const std::string& dbLocation)
{
    emit signalLogDbCreated(dbLocation);
}

void LogCollectorClient::onLogMessagingFailed(void)
{
    emit signalLogMessagingFailed();
}

void LogCollectorClient::onLogInstancesConnect(const std::vector<NEService::sServiceConnectedInstance>& instances)
{
    emit signalLogInstancesConnect(instances);
}

void LogCollectorClient::onLogInstancesDisconnect(const std::vector<NEService::sServiceConnectedInstance>& instances)
{
    emit signalLogInstancesDisconnect(instances);
}

void LogCollectorClient::onLogServiceDisconnected(void)
{
    emit signalLogServiceDisconnected();
}

void LogCollectorClient::onLogRegisterScopes(ITEM_ID cookie, const sLogScope* scopes, int count)
{
    emit signalLogRegisterScopes(cookie, scopes, count);
}

void LogCollectorClient::onLogUpdateScopes(ITEM_ID cookie, const sLogScope* scopes, int count)
{
    emit signalLogUpdateScopes(cookie, scopes, count);
}

void LogCollectorClient::onLogMessage(const SharedBuffer& logMessage)
{
    emit signalLogMessage(logMessage);
}
