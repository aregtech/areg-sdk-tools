#ifndef LUSAN_COMMON_LOGCOLLECTORCLIENT_HPP
#define LUSAN_COMMON_LOGCOLLECTORCLIENT_HPP
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
 *  \file        lusan/common/LogCollectorClient.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer client object.
 *
 ************************************************************************/

#include "areglogger/client/LogObserverBase.hpp"
#include "areg/base/SharedBuffer.hpp"

#include <QObject>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

/**
 * \brief   The log collector client singleton class.
 **/
class LogCollectorClient  final : public QObject
                                , public areg::logger::LogObserverBase
{
    Q_OBJECT
    
//////////////////////////////////////////////////////////////////////////
// Hidden constructors and destructor.
//////////////////////////////////////////////////////////////////////////
private:
    LogCollectorClient();
    virtual ~LogCollectorClient() = default;

public:

    //!< The most received log messages that wait for the live window, twice the rows it holds.
    //!< Above it the oldest waiting messages are dropped from the window, never from the database.
    static constexpr uint32_t   LIVE_PENDING_LIMIT  { 200'000u };

    static LogCollectorClient& getInstance();

    /**
     * \brief   Moves the log messages of every batch up to the given one into the lists.
     *          Called by the thread of the live window when signalLogBatchReady() reaches it.
     * \param   sequence    The sequence number of the newest batch to take.
     * \param[out] logs     The log messages are appended, oldest first.
     * \param[out] rowIds   The database id of every message is appended, 0 if not in the database.
     * \return  The number of messages dropped from the taken batches because the window did not
     *          take them in time.
     **/
    uint32_t takeLogBatches(uint64_t sequence, std::vector<areg::SharedBuffer>& logs, std::vector<uint32_t>& rowIds);

    /**
     * \brief   Drops every log message that waits for the live window.
     **/
    void clearLogBatches();

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   The signal triggered when initializing and configuring the observer.
     * \param   isEnabled       The flag, indicating whether the logging service is enabled or not.
     * \param   address         The IP address of the log collector service set in the configuration file.
     * \param   port            The IP port number of the log collector service set in the configuration file.
     **/
    void signalLogObserverConfigured(bool isEnabled, const std::string& address, uint16_t port);

    /**
     * \brief   The signal event triggered when initializing and configuring the observer.
     * \param   isEnabled       The flag, indicating whether the logging in the database is enabler or not.
     * \param   dbName          The name of the  supported database.
     * \param   dbLocation      The relative or absolute path the database. The path may contain a mask.
     * \param   dbUser          The database user to use when log in. If null or empty, the database may not require the user name.
     **/
    void signalLogDbConfigured(bool isEnabled, const std::string& dbName, const std::string& dbLocation, const std::string& dbUser);

    /**
     * \brief   The signal triggered when the observer connects or disconnects from the log collector service.
     * \param   isConnected     Flag, indicating whether observer is connected or disconnected.
     * \param   address         The IP address of the log collector service to connect or disconnect.
     * \param   port            The IP port number of the log collector service to connect or disconnect.
     **/
    void signalLogServiceConnected(bool isConnected, const std::string& address, uint16_t port);

    /**
     * \brief   The signal trigger when starting or pausing the log observer.
     * \param   isStarted       The flag indicating whether the lob observer is started or paused.
     **/
    void signalLogObserverStarted(bool isStarted);

    /**
     * \brief   The signal triggered when the logging database is created.
     * \param   dbLocation      The relative or absolute path to the logging database.
     **/
    void signalLogDbCreated(const std::string& dbLocation);

    /**
     * \brief   The signal triggered when fails to send or receive message.
     **/
    void signalLogMessagingFailed();

    /**
     * \brief   The signal triggered when receive the list of connected instances that make logs.
     * \param   instances   The list of the connected instances.
     **/
    void signalLogInstancesConnect(const std::vector< areg::ConnectedInstance >& instances);

    /**
     * \brief   The signal triggered when receive the list of disconnected instances that make logs.
     * \param   instances   The list of IDs of the disconnected instances.
     * \param   count       The number of entries in the list.
     **/
    void signalLogInstancesDisconnect(const std::vector< areg::ConnectedInstance >& instances);

    /**
     * \brief   The signal triggered when connection with the log collector service is lost.
     **/
    void signalLogServiceDisconnected();

    /**
     * \brief   The signal triggered when receive the list of the scopes registered in an application.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of the scopes registered in the application. Each entry contains the ID of the scope, message priority and the full name.
     * \param   count   The number of scope entries in the list.
     **/
    void signalLogRegisterScopes(ITEM_ID cookie, const ScopeInfo* scopes, int count);

    /**
     * \brief   The signal triggered when receive the list of previously registered scopes with new priorities.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of previously registered scopes. Each entry contains the ID of the scope, message priority and the full name.
     * \param   count   The number of scope entries in the list.
     **/
    void signalLogUpdateScopes(ITEM_ID cookie, const ScopeInfo* scopes, int count);

    /**
     * \brief   The signal triggered when the first log message of a new batch is received.
     *          The batch takes the next messages until another callback arrives or
     *          takeLogBatches() takes it.
     * \param   sequence    The sequence number of the batch.
     **/
    void signalLogBatchReady(uint64_t sequence);

    /**
     * \brief   The signal triggered when a log source started or stopped sending the logs it produces.
     * \param   cookie      The ID of the log source.
     * \param   state       The state of the source: it sends its logs, or it drops them.
     * \param   byObserver  The ID of the observer that asked for it, zero when the collector did.
     **/
    void signalLogSourceState(ITEM_ID cookie, uint8_t state, ITEM_ID byObserver);

    /**
     * \brief   The signal triggered when a log source read its configuration file again.
     * \param   cookie      The ID of the log source.
     **/
    void signalLogConfigRestored(ITEM_ID cookie);

//////////////////////////////////////////////////////////////////////////
// Protected Overrides / Callbacks
//////////////////////////////////////////////////////////////////////////
protected:
/************************************************************************
 * LogObserverBase overrides
 ************************************************************************/

    /**
     * \brief   Callback triggered when initializing and configuring the observer. Indicates the IP
     *          address and port number of the log collector service set in the configuration file.
     *
     * \param   is_enabled      The flag indicating whether the logging service is enabled or not.
     * \param   address         The null-terminated string of the IP address of the log collector
     *                          service set in the configuration file.
     * \param   port            The IP port number of the log collector service set in the
     *                          configuration file.
     **/
    void on_log_observer_configured(bool is_enabled, const std::string & address, uint16_t port) final;

    /**
     * \brief   Callback triggered when initializing and configuring the observer. Indicates the
     *          supported database, the database location or URI, and the database user name.
     *
     * \param   is_enabled      The flag indicating whether logging in the database is enabled or not.
     * \param   dbName          The name of the supported database.
     * \param   dbLocation      The relative or absolute path to the database. The path may contain a mask.
     * \param   dbUser          The database user to use when logging in. If null or empty, the
     *                          database may not require the user name.
     **/
    void on_log_db_configured(bool is_enabled, const std::string & dbName, const std::string & dbLocation, const std::string & dbUser) final;

    /**
     * \brief   Callback triggered when the observer connects or disconnects from the log collector
     *          service.
     *
     * \param   is_connected    Flag indicating whether observer is connected or disconnected.
     * \param   address         The IP address of the log collector service to connect or disconnect from.
     * \param   port            The IP port number of the log collector service to connect or disconnect from.
     **/
    void on_log_service_connected(bool is_connected, const std::string & address, uint16_t port) final;

    /**
     * \brief   Callback triggered when starting or pausing the log observer. If paused, on resume
     *          logs continue in the same file. If stopped, on restart a new file is created.
     *
     * \param   isStarted       The flag indicating whether the log observer is started or paused.
     **/
    void on_log_observer_started(bool isStarted) final;

    /**
     * \brief   Callback triggered when the logging database is created.
     *
     * \param   dbLocation      The relative or absolute path to the logging database.
     **/
    void on_log_db_created(const std::string & dbLocation) final;

    /**
     * \brief   Callback triggered when fails to send or receive a message.
     **/
    void on_log_messaging_failed() final;

    /**
     * \brief   Callback triggered when receiving the list of connected instances that make logs.
     *
     * \param   instances       The list of the connected instances.
     **/
    void on_log_instances_connect(const std::vector< areg::ConnectedInstance > & instances) final;

    /**
     * \brief   Callback triggered when receiving the list of disconnected instances that make logs.
     *
     * \param   instances       The list of the disconnected instances.
     **/
    void on_log_instances_disconnect(const std::vector< areg::ConnectedInstance > & instances) final;

    /**
     * \brief   Callback triggered when the connection with the log collector service is lost.
     **/
    void on_log_service_disconnected() final;

    /**
     * \brief   Callback triggered when receiving the list of scopes registered in an application.
     *
     * \param   cookie      The cookie ID of the connected instance/application. Same as
     *                      `LogInstance::liCookie`.
     * \param   scopes      The list of scopes registered in the application. Each entry contains
     *                      the ID of the scope, message priority, and the full name.
     * \param   count       The number of scope entries in the list.
     **/
    void on_log_register_scopes(ITEM_ID cookie, const ScopeInfo* scopes, int32_t count) final;

    /**
     * \brief   Callback triggered when receiving the list of previously registered scopes with new
     *          priorities.
     *
     * \param   cookie      The cookie ID of the connected instance/application. Same as
     *                      `LogInstance::liCookie`.
     * \param   scopes      The list of previously registered scopes. Each entry contains the ID of
     *                      the scope, message priority, and the full name.
     * \param   count       The number of scope entries in the list.
     **/
    void on_log_update_scopes(ITEM_ID cookie, const ScopeInfo* scopes, int32_t count) final;

    /**
     * \brief   Callback triggered when receiving a message to log.
     *
     * \param   logMessage  The structure of the message to log.
     **/
    void on_log_message(const areg::MessageEnvelope& logMessage) final;

    /**
     * \brief   Callback triggered when a log source started or stopped sending its logs.
     * \param   cookie      The ID of the log source.
     * \param   state       The state of the source: it sends its logs, or it drops them.
     * \param   byObserver  The ID of the observer that asked for it, zero when the collector did.
     **/
    void on_log_source_state(ITEM_ID cookie, areg::LogSourceState state, ITEM_ID byObserver) final;

    /**
     * \brief   Callback triggered when a log source read its configuration file again.
     * \param   cookie      The ID of the log source.
     **/
    void on_log_configuration_restored(ITEM_ID cookie) final;

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   The log messages received in a row, with no other callback between them.
     **/
    struct LogBatch
    {
        std::vector<areg::SharedBuffer> logs;       //!< The log messages, oldest first.
        std::vector<uint32_t>           rowIds;     //!< The database id of every message.
        uint64_t                        sequence{0};//!< The sequence number of the batch.
        uint32_t                        skipped {0};//!< The messages dropped from the batch.
    };

    /**
     * \brief   Ends the newest batch, so that the next log message starts a new one.
     **/
    void _closeLogBatch();

    /**
     * \brief   Drops the oldest waiting log messages. Called with the batch lock held.
     **/
    void _dropOldestLogs();

private:
    std::mutex              mBatchLock;     //!< Protects the batches.
    std::deque<LogBatch>    mBatches;       //!< The batches waiting for the live window, oldest first.
    uint32_t                mBatchRows;     //!< The log messages in all batches.
    uint64_t                mBatchSequence; //!< The sequence number of the newest batch.
    bool                    mBatchOpen;     //!< True if the newest batch takes the next log message.
};

#endif  // LUSAN_COMMON_LOGCOLLECTORCLIENT_HPP
