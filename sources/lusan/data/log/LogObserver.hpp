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
#include "areg/persist/IEConfigurationListener.hpp"
#include "areg/base/NESocket.hpp"

class LogObserver
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    using ListInstances = TEArrayList<sLogInstance>;
    using ListScopes    = TEArrayList<sLogScope>;
    using MapScopes     = TEHashMap<ITEM_ID, ListScopes>;

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
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
    static void callbackLogScopes(ITEM_ID cookie, const sLogScope* scopes, uint32_t count);

    /**
     * \brief   The callback of the event triggered when receive remote message to log.
     *          The buffer indicates to the NELogging::sLogMessage structure.
     * \param   logBuffer   The pointer to the NELogging::sLogMessage structure to log messages.
     * \param   size        The size of the buffer with log message.
     **/
    static void callbackLogMessageEx(const unsigned char* logBuffer, uint32_t size);

private:
    String      mAddress;       //!< The IP address of the remote log collector service.
    uint16_t    mPort;          //!< The IP port number of the remote log collector service. 
};

#endif  // LUSAN_DATA_LOG_LOGOBSERVER_HPP
