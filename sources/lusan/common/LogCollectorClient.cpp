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
 *  \file        lusan/common/LogCollectorClient.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer client object.
 *
 ************************************************************************/
#include "lusan/common/LogCollectorClient.hpp"
#include "lusan/app/LusanApplication.hpp"
#include "areglogger/client/LogObserverApi.h"
#include "areg/base/SharedBuffer.hpp"

LogCollectorClient& LogCollectorClient::getInstance(void)
{
    static LogCollectorClient _instance;
    return _instance;
}

LogCollectorClient::LogCollectorClient(void)
    : QObject ( )
    , areg::logger::LogObserverBase   ( )
{
}

void LogCollectorClient::on_log_observer_configured(bool isEnabled, const std::string& address, uint16_t port)
{
    emit signalLogObserverConfigured(isEnabled, address, port);
}

void LogCollectorClient::on_log_db_configured(bool isEnabled, const std::string& dbName, const std::string& dbLocation, const std::string& dbUser)
{
    std::string location{ LusanApplication::getWorkspaceLogs().toStdString() };
    if (location.empty() == false)
    {
        set_config_logger_database_location(location);
        emit signalLogDbConfigured(isEnabled, dbName, location, dbUser);
    }
    else
    {
        emit signalLogDbConfigured(isEnabled, dbName, dbLocation, dbUser);
    }
}

void LogCollectorClient::on_log_service_connected(bool isConnected, const std::string& address, uint16_t port)
{
    emit signalLogServiceConnected(isConnected, address, port);
}

void LogCollectorClient::on_log_observer_started(bool isStarted)
{
    emit signalLogObserverStarted(isStarted);
}

void LogCollectorClient::on_log_db_created(const std::string& dbLocation)
{
    emit signalLogDbCreated(dbLocation);
}

void LogCollectorClient::on_log_messaging_failed(void)
{
    emit signalLogMessagingFailed();
}

void LogCollectorClient::on_log_instances_connect(const std::vector<areg::ConnectedInstance>& instances)
{
    emit signalLogInstancesConnect(instances);
}

void LogCollectorClient::on_log_instances_disconnect(const std::vector<areg::ConnectedInstance>& instances)
{
    emit signalLogInstancesDisconnect(instances);
}

void LogCollectorClient::on_log_service_disconnected(void)
{
    emit signalLogServiceDisconnected();
}

void LogCollectorClient::on_log_register_scopes(ITEM_ID cookie, const ScopeInfo* scopes, int count)
{
    emit signalLogRegisterScopes(cookie, scopes, count);
}

void LogCollectorClient::on_log_update_scopes(ITEM_ID cookie, const ScopeInfo* scopes, int count)
{
    emit signalLogUpdateScopes(cookie, scopes, count);
}

void LogCollectorClient::on_log_message(const areg::SharedBuffer& logMessage)
{
    emit signalLogMessage(logMessage);
}
