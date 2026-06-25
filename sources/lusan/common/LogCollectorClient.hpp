#ifndef LUSAN_COMMON_LOGCOLLECTORCLIENT_HPP
#define LUSAN_COMMON_LOGCOLLECTORCLIENT_HPP
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
 *  \file        lusan/common/LogCollectorClient.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer client object.
 *
 ************************************************************************/

#include "areglogger/client/LogObserverBase.hpp"

#include <QObject>
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
    LogCollectorClient(void);
    virtual ~LogCollectorClient(void) = default;

public:

    static LogCollectorClient& getInstance(void);
    
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
    void signalLogMessagingFailed(void);

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
    void signalLogServiceDisconnected(void);

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
     * \brief   The signal triggered when receive message to log.
     * \param   logMessage  The buffer with structure of the message to log.
     **/
    void signalLogMessage(const areg::MessageEnvelope & logMessage);

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

};

#endif  // LUSAN_COMMON_LOGCOLLECTORCLIENT_HPP
