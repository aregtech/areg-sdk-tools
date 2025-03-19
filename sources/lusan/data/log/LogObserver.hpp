#ifndef LUSAN_DATA_LOG_LOGOBSERVER_HPP
#define LUSAN_DATA_LOG_LOGOBSERVER_HPP
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

#include "lusan/common/NELusanCommon.hpp"

#include <QString>

#include "areg/persist/IEConfigurationListener.hpp"
#include "areg/base/NESocket.hpp"
#include "areg/base/TEArrayList.hpp"
#include "areg/base/TEHashMap.hpp"
#include "areg/base/TELinkedList.hpp"
#include "areg/logging/NELogging.hpp"
#include "areglogger/client/LogObserverApi.h"

class DispatcherThread;

class LogObserver
{
    friend class LogObserverComp;

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    // The list of log instances / sources
    using ListInstances = TEArrayList<sLogInstance>;
    // The list of log scopes
    using ListScopes    = TEArrayList<sLogScope>;
    // The map of scopes, where the key is the ID of log instance
    using MapScopes     = TEHashMap<ITEM_ID, ListScopes>;
    // The list of log messages
    using ListLogs      = TELinkedList<SharedBuffer>;

    /**
     * \brief   The structure to store the IP address and port number of the remote log collector service.
     **/
    struct sLoggerConnect
    {
        // The IP address of the remote log collector service
        String      lcAddress;
        // The IP port number of the remote log collector service
        uint16_t    lcPort{ NESocket::InvalidPort };
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    LogObserver(void);
    ~LogObserver(void);

//////////////////////////////////////////////////////////////////////////
// Attributes and operation
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Initializes and starts the logging observer.
     **/
    void logingStart(void);

    /**
     * \brief   Initializes and starts the logging observer.
     * \param   dbPath      The path of the database to log messages.
     * \param   address     The IP address of the remote log collector service.
     * \param   port        The IP port number of the remote log collector service.
     **/
    void loggingStart(const QString & dbPath, const QString& address, unsigned short port);

    /**
     * \brief   Initializes and starts the logging observer.
     * \param   configPath  The path of the configuration file.
     **/
    void loggingStart(const String& configPath);
    void loggingStart(const QString& configPath);

    /**
     * \brief   Stops the logging observer.
     **/
    void loggingStop(void);

    /**
     * \brief   Pauses the logging observer.
     **/
    void loggingPause(void);

    /**
     * \brief   Resumes the logging observer.
     **/
    void loggingResume(void);

    /**
     * \brief   Clears the log messages.
     **/
    void loggingClear(void);

    /**
     * \brief   Requests the list of connected instances.
     **/
    void loggingRequestInstances(void);

    /**
     * \brief   Requests the list of scopes of the specified connected instance.
     * \param   target  The cookie ID of the target instance to receive the list of registered scopes.
     *                  If the target is ID_IGNORE (or 0), it receives the list of scopes of all connected instances.
     *                  Otherwise, should be indicated the valid cookie ID of the connected log instance.
     **/
    void loggingRequestScopes(ITEM_ID target);

    /**
     * \brief   Requests to change the priority of the logging message to receive.
     * \param   target  The cookie ID of the target instance to receive the list of registered scopes.
     * \param   scopes  The list of the scopes to change the priority.
     * \param   count   The number of entries in the list.
     **/
    void loggingRequestChangeScopePrio(ITEM_ID target, const sLogScope* scopes, uint32_t count);

    /**
     * \brief   Saves the configuration of the logging.
     * \param   target  The cookie ID of the target instance to save the configuration.
     **/
    void loggingSaveConfiguration(ITEM_ID target);

    /**
     * \brief   Returns true if the logging observer is initialized.
     **/
    bool isLoggingInitialized(void) const;

    /**
     * \brief   Returns true if the logging observer is connected to the remote log collector service.
     **/
    bool isLoggingConnected(void) const;

    /**
     * \brief   Returns true if the logging observer is started.
     **/
    bool isLoggingStarted(void) const;

    /**
     * \brief   Returns true if the logging observer is paused.
     **/
    bool isLoggingEnabled(void) const;

    /**
     * \brief   Returns the IP address of the remote log collector service.
     **/
    bool isLoggingPaused(void) const;

    /**
     * \brief   Returns the IP address of the remote log collector service.
     **/
    QString getConectionAddress(void) const;

    /**
     * \brief   Returns the IP port of the remote log collector service.
     **/
    unsigned short getConnectionPort(void) const;

    /**
     * \brief   Returns the list of connected instances.
     **/
    inline const LogObserver::sLoggerConnect& getLogConnect(void) const;

    /**
     * \brief   Returns the list of connected instances.
     **/
    inline const LogObserver::ListInstances& getLogSources(void) const;

    /**
     * \brief   Returns the list of connected instances.
     **/
    inline const LogObserver::MapScopes& getLogScopes(void) const;

    /**
     * \brief   Returns the list of connected instances.
     **/
    inline const LogObserver::ListLogs& getLogMessages(void) const;

    inline void setLogCollector(const String& address, uint16_t port);

//////////////////////////////////////////////////////////////////////////
// Callbacks
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   The callback of the event triggered when initializing and configuring the observer.
     *          The callback indicates the IP address and port number of the remote log collector
     *          service set in the configuration file.
     * \param   isEnabled       Flag, indicating whether the logging service is enabled or not.
     * \param   address         IP address of the remote log collector service set in the configuration file.
     * \param   port            IP port number of the remote log collector service set in the configuration file.
     **/
    static void callbackObserverConfigured(bool isEnabled, const char* address, uint16_t port);

    /**
     * \brief   The callback of the event triggered when initializing and configuring the observer.
     *          The callback indicates the supported database, the database location or URI and
     *          the database user name.
     * \param   isEnabled       The flag, indicating whether the logging in the database is enabler or not.
     * \param   dbName          The name of the  supported database.
     * \param   dbLocation      The relative or absolute path the database. The path may contain a mask.
     * \param   user            The database user to use when log in. If null or empty, the database may not require the user name.
     **/
    static void callbackDatabaseConfigured(bool isEnabled, const char* dbName, const char* dbLocation, const char* user);

    /**
     * \brief   The callback of the event triggered when the observer connects or disconnects
     *          from the remote log collector service.
     * \param   isConnected     Flag, indicating whether observer is connected or disconnected.
     * \param   address         IP address of the remote log collector service to connect or disconnect.
     * \param   port            IP port number of the remote log collector service to connect or disconnect.
     **/
    static void callbackServiceConnected(bool isConnected, const char* address, uint16_t port);

    /**
     * \brief   The callback of the event trigger when starting or pausing the log observer.
     *          If the log observer is paused, on start it continues to write logs in the same file.
     *          If the log observer is stopped (disconnected is called), on start it creates new file.
     * \param   isStarted       The flag indicating whether the lob observer is started or paused.
     **/
    static void callbackObserverStarted(bool isStarted);

    /**
     * \brief   The callback of the event triggered when fails to send or receive message.
     **/
    static void callbackMessagingFailed(void);

    /**
     * \brief   The callback of the event triggered when receive the list of connected instances that make logs.
     * \param   instances   The pointer to the list of the connected instances.
     * \param   count       The number of entries in the list.
     **/
    static void callbackConnectedInstances(const sLogInstance* instances, uint32_t count);

    /**
     * \brief   The callback of the event triggered when receive the list of disconnected instances that make logs.
     * \param   instances   The pointer to the list of IDs of the disconnected instances.
     * \param   count       The number of entries in the list.
     **/
    static void callbackDisconnecteInstances(const ITEM_ID* instances, uint32_t count);

    /**
     * \brief   The callback of the event triggered when receive the list of the scopes registered in an application.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of the scopes registered in the application. Each entry contains the ID of the scope, message priority and the full name.
     * \param   count   The number of scope entries in the list.
     **/
    static void callbackLogScopesRegistered(ITEM_ID cookie, const sLogScope* scopes, uint32_t count);

    /**
     * \brief   The callback of the event triggered when receive the list of previously registered scopes with new priorities.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of previously registered scopes. Each entry contains the ID of the scope, message priority and the full name.
     * \param   count   The number of scope entries in the list.
     **/
    static void callbackLogScopesUpdated(ITEM_ID cookie, const sLogScope* scopes, uint32_t count);

    /**
     * \brief   The callback of the event triggered when receive remote message to log.
     *          The buffer indicates to the NELogging::sLogMessage structure.
     * \param   logBuffer   The pointer to the NELogging::sLogMessage structure to log messages.
     * \param   size        The size of the buffer with log message.
     **/
    static void callbackLogMessageEx(const unsigned char* logBuffer, uint32_t size);

private:
    void _clear(void);

    static DispatcherThread& _logObserverThread(void);

private:
    sLoggerConnect  mLogConnect;    //!< The IP address and the port number of the remote log collector service.
    ListInstances   mLogSources;    //!< The list of the connected instances that make logs.
    MapScopes       mLogScopes;     //!< The map of the scopes registered in the application.
    ListLogs        mLogMessages;   //!< The list of the log messages to write in the log file.
    sObserverEvents mEvents;        //!< The structure of the callbacks to set when send or receive messages.

private:
    LogObserver(const LogObserver& /*src*/) = delete;
    LogObserver(LogObserver&& /*src*/) noexcept = delete;
    LogObserver& operator = (const LogObserver& /*src*/) = delete;
    LogObserver& operator = (LogObserver&& /*src*/) noexcept = delete;
};

//////////////////////////////////////////////////////////////////////////
// LogObserver class inline function implementation
//////////////////////////////////////////////////////////////////////////

inline const LogObserver::sLoggerConnect& LogObserver::getLogConnect(void) const
{
    return mLogConnect;
}

inline const LogObserver::ListInstances& LogObserver::getLogSources(void) const
{
    return mLogSources;
}

inline const LogObserver::MapScopes& LogObserver::getLogScopes(void) const
{
    return mLogScopes;
}

inline const LogObserver::ListLogs& LogObserver::getLogMessages(void) const
{
    return mLogMessages;
}

inline void LogObserver::setLogCollector(const String& address, uint16_t port)
{
    mLogConnect.lcAddress = address;
    mLogConnect.lcPort = port;
}

#endif  // LUSAN_DATA_LOG_LOGOBSERVER_HPP
