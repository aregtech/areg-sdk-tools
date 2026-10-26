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
 *  \file        lusan/common/LogCollectorClient.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer client object.
 *
 ************************************************************************/
#include "lusan/common/LogCollectorClient.hpp"
#include "lusan/app/LusanApplication.hpp"
#include "areglogger/client/LogObserverApi.h"
#include "areg/base/SharedBuffer.hpp"

LogCollectorClient& LogCollectorClient::getInstance()
{
    static LogCollectorClient _instance;
    return _instance;
}

LogCollectorClient::LogCollectorClient()
    : QObject ( )
    , areg::logger::LogObserverBase   ( )
    , mBatchLock    ( )
    , mBatches      ( )
    , mBatchRows    ( 0u )
    , mBatchSequence( 0u )
    , mBatchOpen    ( false )
{
}

uint32_t LogCollectorClient::takeLogBatches(uint64_t sequence, std::vector<areg::SharedBuffer>& logs, std::vector<uint32_t>& rowIds)
{
    uint32_t skipped{ 0u };
    std::lock_guard<std::mutex> lock(mBatchLock);
    while ((mBatches.empty() == false) && (mBatches.front().sequence <= sequence))
    {
        LogBatch& batch{ mBatches.front() };
        skipped += batch.skipped;
        mBatchRows -= static_cast<uint32_t>(batch.logs.size());
        if (logs.empty())
        {
            logs.swap(batch.logs);
            rowIds.swap(batch.rowIds);
        }
        else
        {
            logs.insert(logs.end(), std::make_move_iterator(batch.logs.begin()), std::make_move_iterator(batch.logs.end()));
            rowIds.insert(rowIds.end(), batch.rowIds.begin(), batch.rowIds.end());
        }

        if (mBatches.size() == 1u)
        {
            mBatchOpen = false;
        }

        mBatches.pop_front();
    }

    return skipped;
}

void LogCollectorClient::clearLogBatches()
{
    std::lock_guard<std::mutex> lock(mBatchLock);
    mBatches.clear();
    mBatchRows = 0u;
    mBatchOpen = false;
}

void LogCollectorClient::_closeLogBatch()
{
    std::lock_guard<std::mutex> lock(mBatchLock);
    mBatchOpen = false;
}

void LogCollectorClient::_dropOldestLogs()
{
    for (size_t i = 0; i < mBatches.size(); ++i)
    {
        LogBatch& batch{ mBatches[i] };
        if (batch.logs.empty())
            continue;

        // An older batch is dropped whole, the newest one keeps its later half.
        const size_t count{ (i + 1u) < mBatches.size() ? batch.logs.size() : (batch.logs.size() + 1u) / 2u };
        batch.logs.erase(batch.logs.begin(), batch.logs.begin() + static_cast<std::ptrdiff_t>(count));
        batch.rowIds.erase(batch.rowIds.begin(), batch.rowIds.begin() + static_cast<std::ptrdiff_t>(count));
        batch.skipped += static_cast<uint32_t>(count);
        mBatchRows -= static_cast<uint32_t>(count);
        break;
    }
}

void LogCollectorClient::on_log_observer_configured(bool isEnabled, const std::string& address, uint16_t port)
{
    _closeLogBatch();
    emit signalLogObserverConfigured(isEnabled, address, port);
}

void LogCollectorClient::on_log_db_configured(bool isEnabled, const std::string& dbName, const std::string& dbLocation, const std::string& dbUser)
{
    _closeLogBatch();
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
    _closeLogBatch();
    emit signalLogServiceConnected(isConnected, address, port);
}

void LogCollectorClient::on_log_observer_started(bool isStarted)
{
    _closeLogBatch();
    emit signalLogObserverStarted(isStarted);
}

void LogCollectorClient::on_log_db_created(const std::string& dbLocation)
{
    _closeLogBatch();
    emit signalLogDbCreated(dbLocation);
}

void LogCollectorClient::on_log_messaging_failed()
{
    _closeLogBatch();
    emit signalLogMessagingFailed();
}

void LogCollectorClient::on_log_instances_connect(const std::vector<areg::ConnectedInstance>& instances)
{
    _closeLogBatch();
    emit signalLogInstancesConnect(instances);
}

void LogCollectorClient::on_log_instances_disconnect(const std::vector<areg::ConnectedInstance>& instances)
{
    _closeLogBatch();
    emit signalLogInstancesDisconnect(instances);
}

void LogCollectorClient::on_log_service_disconnected()
{
    _closeLogBatch();
    emit signalLogServiceDisconnected();
}

void LogCollectorClient::on_log_register_scopes(ITEM_ID cookie, const ScopeInfo* scopes, int count)
{
    _closeLogBatch();
    emit signalLogRegisterScopes(cookie, scopes, count);
}

void LogCollectorClient::on_log_update_scopes(ITEM_ID cookie, const ScopeInfo* scopes, int count)
{
    _closeLogBatch();
    emit signalLogUpdateScopes(cookie, scopes, count);
}

void LogCollectorClient::on_log_message(const areg::MessageEnvelope& logMessage)
{
    areg::SharedBuffer log(logMessage);
    const uint32_t rowId{ delivered_log_id() };
    uint64_t newBatch{ 0u };
    do
    {
        std::lock_guard<std::mutex> lock(mBatchLock);
        if (mBatchOpen == false)
        {
            mBatches.emplace_back();
            mBatches.back().sequence = ++mBatchSequence;
            mBatchOpen = true;
            newBatch = mBatchSequence;
        }

        LogBatch& batch{ mBatches.back() };
        batch.logs.push_back(std::move(log));
        batch.rowIds.push_back(rowId);
        if (++mBatchRows > LogCollectorClient::LIVE_PENDING_LIMIT)
        {
            _dropOldestLogs();
        }
    } while (false);

    if (newBatch != 0u)
    {
        emit signalLogBatchReady(newBatch);
    }
}

void LogCollectorClient::on_log_source_state(ITEM_ID cookie, areg::LogSourceState state, ITEM_ID byObserver)
{
    _closeLogBatch();
    emit signalLogSourceState(cookie, static_cast<uint8_t>(state), byObserver);
}

void LogCollectorClient::on_log_configuration_restored(ITEM_ID cookie)
{
    _closeLogBatch();
    emit signalLogConfigRestored(cookie);
}
