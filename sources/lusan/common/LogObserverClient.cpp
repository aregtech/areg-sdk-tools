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

void LogObserverClient::onLogObserverConfigured(bool isEnabled, const std::string& address, uint16_t port)
{
    emit signalLogObserverConfigured(isEnabled, address, port);
}

void LogObserverClient::onLogDbConfigured(bool isEnabled, const std::string& dbName, const std::string& dbLocation, const std::string& dbUser)
{
    emit signalLogDbConfigured(isEnabled, dbName, dbLocation, dbUser);
}

void LogObserverClient::onLogServiceConnected(bool isConnected, const std::string& address, uint16_t port)
{
    emit signalLogServiceConnected(isConnected, address, port);
}

void LogObserverClient::onLogObserverStarted(bool isStarted)
{
    emit signalLogObserverStarted(isStarted);
}

void LogObserverClient::onLogDbCreated(const std::string& dbLocation)
{
    emit signalLogDbCreated(dbLocation);
}

void LogObserverClient::onLogMessagingFailed(void)
{
    emit signalLogMessagingFailed();
}

void LogObserverClient::onLogInstancesConnect(const std::vector<NEService::sServiceConnectedInstance>& instances)
{
    emit signalLogInstancesConnect(instances);
}

void LogObserverClient::onLogInstancesDisconnect(const std::vector<NEService::sServiceConnectedInstance>& instances)
{
    emit signalLogInstancesDisconnect(instances);
}

void LogObserverClient::onLogServiceDisconnected(const std::map<ITEM_ID, NEService::sServiceConnectedInstance> & instances)
{
    emit signalLogServiceDisconnected(instances);
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
    emit signalLogMessage(logMessage);
}
