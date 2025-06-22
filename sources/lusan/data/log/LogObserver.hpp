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
 *  \brief       Lusan application, Log observer component with empty service provider.
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include "areg/base/GEGlobal.h"

#include "lusan/data/log/LogObserverEvent.hpp"
#include "areg/base/NESocket.hpp"
#include "areg/component/Component.hpp"
#include "areg/component/StubBase.hpp"

#include <QObject>
#include <QString>
#include <QMap>

/************************************************************************
 * Dependencies.
 ************************************************************************/
class LogCollectorClient;
struct sLogScope;

//!< The callback to notify that the service component started.
typedef void (*FuncLogObserverStarted)();

/**
 * \brief   The log observer component which runs in the multithreading environment.
 *          It is used to support multithreading environment when receives and sends log data from the log collector.
 **/
class LogObserver   : public    QObject
                    , public    Component
                    , protected StubBase
                    , protected IELogObserverEventConsumer
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////
public:

    //!< The name of the component thread.
    static constexpr const char* const LogobserverThread    { "LusanLogObserverThread"   };
    //!< The name of the model.
    static constexpr const char* const LogobserverModel     { "LusanLogObserverModel"    };
    //!< The name of the component.
    static constexpr const char* const LogObserverComponent { "LusanLogObserverComponent"};

//////////////////////////////////////////////////////////////////////////
// Public static methods
//////////////////////////////////////////////////////////////////////////
public:
    
    /**
     * \brief   Creates and starts the log observer component.
     * \return  Returns true if the component is started successfully. Otherwise, returns false.
     **/
    static bool createLogObserver(FuncLogObserverStarted callbackStarted);

    /**
     * \brief   Stops the log observer component.
     *          The function stops the log observer component and unloads the model.
     **/
    static void releaseLogObserver(void);

    /**
     * \brief   Returns the `SocketAddress` object of the connected log collector service.
     **/
    static const NESocket::SocketAddress& getLogServiceAddress(void);

    /**
     * \brief   Returns the IP-address of connected log collector.
     **/
    static QString getConnectedAddress(void);

    /**
     * \brief   Returns the host name of connected log collector service.
     *          If the host name is not set or was not able to resolve,
     *          returns IP-address of the log collector service.
     **/
    static QString getConnectedHostName(void);

    /**
     * \brief   Returns the port number of connected log collector.
     **/
    static uint16_t getConnectedPort(void);

    /**
     *\brief    The function returns the path to the database, which is used to log messages.
     **/
    static QString getActiveDatabase(void);

    /**
     * \brief   Returns the path to the database, which is set in the initialization file.
     *          The path may contain a mask.
     **/
    static QString getInitDatabase(void);

    /**
     * \brief   Returns the name of the database, which is set in the initialization file.
     *          The name may contain a mask.
     **/
    static QString getConfigDatabaseName(void);

    /**
     * \brief   Returns the location of the database, which is set in the initialization file.
     *          The location may contain a mask.
     **/
    static QString getConfigDatabaseLocation(void);

    /**
     * \brief   Sets the name of the database to set in the initialization file.
     *          The name may contain a mask.
     * \param   dbName  The name of the database to set in the initialization file.
     * \return  Returns true if operation succeeds. Otherwise, returns false.
     **/
    static bool setConfigDatabaseName(const QString& dbName);

    /**
     * \brief   Sets the location of the database to set in the initialization file.
     *          The location may contain a mask.
     * \param   dbLocation  The relative or absolute path to the database. The path may contain a mask.
     * \return  Returns true if operation succeeds. Otherwise, returns false.
     **/
    static bool setConfigDatabaseLocation(const QString& dbLocation);

    /**
     * \brief   Returns true if log observer is connected to the log collector.
     **/
    static bool isConnected(void);

    /**
     * \brief   Connects to the log collector service.
     * \param   address     The IP address of the log collector service.
     * \param   port        The IP port number of the log collector service.
     * \param   dbLocation  The relative or absolute path to the database. The path may contain a mask.
     * \return  Returns true if connected successfully. Otherwise, returns false.
     **/
    static bool connect(const QString& address, uint16_t port, const QString& dbLocation);

    /**
     * \brief   Disconnects from the log collector service. The log observer component remains connected.
     **/
    static void disconnect(void);

    /**
     * \brief   Pauses running log observer component. It stops logging messages, but keeps database active.
     *          On resume it will continue log messages in the active database.
     * \return  Returns true if operation succeeds. Otherwise, returns false.
     **/
    static bool pause(void);

    /**
     * \brief   Resumes the log observer component. It starts logging messages in the active database.
     * \return  Returns true if operation succeeds. Otherwise, returns false.
     **/
    static bool resume(void);

    /**
     * \brief   Stops the log observer component. It stops logging messages and closes the database.
     *          On next start it will create new database.
     **/
    static void stop(void);

    /**
     * \brief   Restarts stopped log observer component. On start it creates new logging database.
     * \param   dbLocation  The relative or absolute path to the database. The path may contain a mask.
     * \return  Returns true if operation succeeds. Otherwise, returns false.
     **/
    static bool restart(const QString & dbLocation = QString());
    
    /**
     * \brief   Requests the list of connected instances that make logs.
     * \return  Returns true if processed with success. Otherwise, returns false.
     **/
    static bool requestInstances(void);
    
    /**
     * \brief   Requests the list of registered scopes of the specified connected instance.
     * \param   target  The cookie ID of the target instance to receive the list of registered scopes.
     *                  If the target is NEService::TARGET_ALL (or 0), it receives the list of scopes of all connected instances.
     *                  Otherwise, should be indicated the valid cookie ID of the connected log instance.
     * \return  Returns true if processed with success. Otherwise, returns false.
     **/
    static bool requestScopes(ITEM_ID target = NEService::TARGET_ALL);
    
    /**
     * \brief   Requests to update the priority of the logging message to receive.
     *          The indicated scopes can be scope group.
     * \param   target  The valid cookie ID of the target to update the log message priority.
     *                  This value cannot be NEService::TARGET_ALL (or 0).
     * \param   scopes  The list of scopes of scope group to update the priority.
     *                  The scope group should  end with '*'. For example 'areg_base_*'.
     *                  In this case the ID of the scope can be 0.
     * \param   count   The number of scope entries in the list.
     * \return  Returns true if processed with success. Otherwise, returns false.
     **/
    static bool requestChangeScopePrio(ITEM_ID target, const sLogScope* scopes, uint32_t count);
    
    /**
     * \brief   Requests to save current configuration of the specified target. This is normally called when update the log priority of the instance,
     *          so that on next start the application logs message of the scopes and priorities currently set.
     * \param   target  The cookie ID of the target instance to save the configuration.
     *                  If the target is NEService::TARGET_ALL (or 0), the request is sent to all connected instances.
     *                  Otherwise, should be indicated the valid cookie ID of the connected log instance.
     * \return  Returns true if processed with success. Otherwise, returns false.
     **/
    static bool requestSaveConfig(ITEM_ID target = NEService::TARGET_ALL);
    
    /**
     * \brief   Saves the configuration of the log observer in the configuration file.
     **/
    static void saveLoggerConfig(void);
    
    /**
     * \brief   Called by system to instantiate the component.
     * \param   entry   The entry of registry, which describes the component.
     * \param   owner   The component owning thread.
     * \return  Returns instantiated component to run in the system
     **/
    static Component * CreateComponent( const NERegistry::ComponentEntry & entry, ComponentThread & owner );
    
    /**
     * \brief   Called by system to delete component and free resources.
     * \param   compObject  The instance of component previously created by CreateComponent method.
     * \param   entry   The entry of registry, which describes the component.
     **/
    static void DeleteComponent( Component & compObject, const NERegistry::ComponentEntry & entry );
    
    /**
     * \brief   Returns the pointer of log observer component if loaded. Otherwise, returns null.
     **/
    static LogObserver* getComponent(void);
    
//////////////////////////////////////////////////////////////////////////
// Static methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Returns instance of the client. The log observer component should be already loaded.
     **/
    static LogCollectorClient& getClient(void);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   The signal is triggered when initializing and configuring the observer.
     * \param   isEnabled       The flag, indicating whether the logging service is enabled or not.
     * \param   address         The IP address of the log collector service set in the configuration file.
     * \param   port            The IP port number of the log collector service set in the configuration file.
     **/
    void signalLogObserverConfigured(bool isEnabled, const QString& address, uint16_t port);

    /**
     * \brief   The signal event triggered when initializing and configuring the observer.
     * \param   isEnabled       The flag, indicating whether the logging in the database is enabler or not.
     * \param   dbName          The name of the  supported database.
     * \param   dbLocation      The relative or absolute path the database. The path may contain a mask.
     * \param   dbUser          The database user to use when log in. If null or empty, the database may not require the user name.
     **/
    void signalLogDbConfigured(bool isEnabled, const QString& dbName, const QString& dbLocation, const QString& dbUser);

    /**
     * \brief   The signal is triggered when the observer connects or disconnects from the log collector service.
     * \param   isConnected     Flag, indicating whether observer is connected or disconnected.
     * \param   address         The IP address of the log collector service to connect or disconnect.
     * \param   port            The IP port number of the log collector service to connect or disconnect.
     **/
    void signalLogServiceConnected(bool isConnected, const QString& address, uint16_t port);

    /**
     * \brief   The signal trigger when starting or pausing the log observer.
     * \param   isStarted       The flag indicating whether the lob observer is started or paused.
     **/
    void signalLogObserverStarted(bool isStarted);

    /**
     * \brief   The signal is triggered when the logging database is created.
     * \param   dbLocation      The relative or absolute path to the logging database.
     **/
    void signalLogDbCreated(const QString& dbLocation);

    /**
     * \brief   The signal is triggered when fails to send or receive message.
     **/
    void signalLogMessagingFailed(void);

    /**
     * \brief   The signal is triggered when receive the list of connected instances that make logs.
     * \param   instances   The list of the connected instances.
     **/
    void signalLogInstancesConnect(const QList< NEService::sServiceConnectedInstance >& instances);

    /**
     * \brief   The signal is triggered when receive the list of disconnected instances that make logs.
     * \param   instances   The list of IDs of the disconnected instances.
     * \param   count       The number of entries in the list.
     **/
    void signalLogInstancesDisconnect(const QList< NEService::sServiceConnectedInstance >& instances);

    /**
     * \brief   The signal is triggered when connection with the log collector service is lost.
     * \param   instances   The list of disconnected instances.
     **/
    void signalLogServiceDisconnected(const QMap<ITEM_ID, NEService::sServiceConnectedInstance>& instances);

    /**
     * \brief   The signal is triggered when receive the list of the scopes registered in an application.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of the scopes registered in the application. Each entry contains the ID of the scope, message priority and the full name.
     **/
    void signalLogRegisterScopes(ITEM_ID cookie, const QList<sLogScope *>& scopes);

    /**
     * \brief   The signal is triggered when receive the list of previously registered scopes with new priorities.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of previously registered scopes. Each entry contains the ID of the scope, message priority and the full name.
     **/
    void signalLogUpdateScopes(ITEM_ID cookie, const QList<sLogScope *>& scopes);

    /**
     * \brief   The signal is triggered when receive message to log.
     * \param   logMessage  The buffer with structure of the message to log.
     **/
    void signalLogMessage(const SharedBuffer & logMessage);

    /**
     * \brief   The signal is triggered when the log observer instance is activated or shutdown.
     * \param   isStarted       The flag indicating whether the log observer instance is started or stopped.
     * \param   address         The IP address of the log observer instance.
     * \param   port            The TCP port number of the log observer instance.
     * \param   filePath        The file path of the log file, if any. If empty, no file is used.
     **/
    void signalLogObserverInstance(bool isStarted, const QString& address, uint16_t port, const QString& filePath);

protected:

    /**
     * \brief   Instantiates the component object.
     * \param   entry   The entry of registry, which describes the component.
     * \param   ownerThread The instance of component owner thread.
     * \param   data        The optional component data set in system. Can be empty / no data.
     **/
    LogObserver(const NERegistry::ComponentEntry & entry, ComponentThread & ownerThread, NEMemory::uAlign OPT data);

    virtual ~LogObserver(void);

/************************************************************************/
// IELogObserverEventConsumer overrides
/************************************************************************/

    virtual void processEvent(const LogObserverEventData & data) override;

/************************************************************************/
// StubBase overrides. Triggered by Component on startup.
/************************************************************************/

    /**
     * \brief   This function is triggered by Component when starts up.
     * \param   holder  The holder component of service interface of Stub.
     **/
    virtual void startupServiceInterface( Component & holder ) override;

    /**
     * \brief   This function is triggered by Component when shuts down.
     * \param   holder  The holder component of service interface of Stub.
     **/
    virtual void shutdownServiceIntrface ( Component & holder ) override;

//////////////////////////////////////////////////////////////////////////
// These methods must exist, but can have empty body
//////////////////////////////////////////////////////////////////////////
protected:
/************************************************************************/
// StubBase overrides. Public pure virtual methods
/************************************************************************/

    /**
     * \brief   Sends update notification message to all clients.
     **/
    virtual void sendNotification(unsigned int /*msgId*/) override;

    /**
     * \brief   Sends error message to clients.
     **/
    virtual void errorRequest(unsigned int /*msgId*/, bool /*msgCancel*/) override;

/************************************************************************/
// IEStubEventConsumer interface overrides.
/************************************************************************/

    /**
     * \brief   Triggered to process service request event.
     **/
    virtual void processRequestEvent(ServiceRequestEvent& /*eventElem*/) override;

    /**
     * \brief   Triggered to process attribute update notification event.
     **/
    virtual void processAttributeEvent(ServiceRequestEvent& /*eventElem*/) override;

//////////////////////////////////////////////////////////////////////////
// slots
//////////////////////////////////////////////////////////////////////////
private slots:
/************************************************************************
 * LogObserver component slots
 ************************************************************************/

    /**
     * \brief   The callback of the event triggered when initializing and configuring the observer.
     *          The callback indicates the IP address and port number of the log collector service set
     *          in the configuration file.
     * \param   isEnabled       The flag, indicating whether the logging service is enabled or not.
     * \param   address         The null-terminated string of the IP address of the log collector service set in the configuration file.
     * \param   port            The IP port number of the log collector service set in the configuration file.
     **/
    void slotLogObserverConfigured(bool isEnabled, const std::string & address, uint16_t port);

    /**
     * \brief   The callback of the event triggered when initializing and configuring the observer.
     *          The callback indicates the supported database, the database location or URI and
     *          the database user name.
     * \param   isEnabled       The flag, indicating whether the logging in the database is enabler or not.
     * \param   dbName          The name of the  supported database.
     * \param   dbLocation      The relative or absolute path the database. The path may contain a mask.
     * \param   dbUser          The database user to use when log in. If null or empty, the database may not require the user name.
     **/
    void slotLogDbConfigured(bool isEnabled, const std::string & dbName, const std::string & dbLocation, const std::string & dbUser);

    /**
     * \brief   The callback of the event triggered when the observer connects or disconnects from the log collector service.
     * \param   isConnected     Flag, indicating whether observer is connected or disconnected.
     * \param   address         The IP address of the log collector service to connect or disconnect.
     * \param   port            The IP port number of the log collector service to connect or disconnect.
     **/
    void slotLogServiceConnected(bool isConnected, const std::string & address, uint16_t port);

    /**
     * \brief   The callback of the event trigger when starting or pausing the log observer.
     *          If the log observer is paused, on start it continues to write logs in the same file.
     *          If the log observer is stopped (disconnected is called), on start it creates new file.
     * \param   isStarted       The flag indicating whether the lob observer is started or paused.
     **/
    void slotLogObserverStarted(bool isStarted);

    /**
     * \brief   The callback of the event triggered when the logging database is created.
     * \param   dbLocation      The relative or absolute path to the logging database.
     **/
    void slotLogDbCreated(const std::string & dbLocation);

    /**
     * \brief   The callback of the event triggered when fails to send or receive message.
     **/
    void slotLogMessagingFailed(void);

    /**
     * \brief   The callback of the event triggered when receive the list of connected instances that make logs.
     * \param   instances   The list of the connected instances.
     **/
    void slotLogInstancesConnect(const std::vector< NEService::sServiceConnectedInstance > & instances);

    /**
     * \brief   The callback of the event triggered when receive the list of disconnected instances that make logs.
     * \param   instances   The list of IDs of the disconnected instances.
     * \param   count       The number of entries in the list.
     **/
    void slotLogInstancesDisconnect(const std::vector< NEService::sServiceConnectedInstance > & instances);

    /**
     * \brief   The callback of the event triggered when connection with the log collector service is lost.
     * \param   instances   The list of disconnected instances.
     **/
    void slotLogServiceDisconnected(const std::map<ITEM_ID, NEService::sServiceConnectedInstance>& instances);

    /**
     * \brief   The callback of the event triggered when receive the list of the scopes registered in an application.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of the scopes registered in the application. Each entry contains the ID of the scope, message priority and the full name.
     * \param   count   The number of scope entries in the list.
     **/
    void slotLogRegisterScopes(ITEM_ID cookie, const sLogScope* scopes, int count);

    /**
     * \brief   The callback of the event triggered when receive the list of previously registered scopes with new priorities.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of previously registered scopes. Each entry contains the ID of the scope, message priority and the full name.
     * \param   count   The number of scope entries in the list.
     **/
    void slotLogUpdateScopes(ITEM_ID cookie, const sLogScope* scopes, int count);

    /**
     * \brief   The callback of the event triggered when receive message to log.
     * \param   logMessage  The structure of the message to log.
     **/
    void slotLogMessage(const SharedBuffer & logMessage);

private:
    inline LogObserver& self(void);

private:
    LogCollectorClient& mLogClient;     //!< Log observer client.
    String              mConfigFile;    //!< The path to config file.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    LogObserver( void ) = delete;
    DECLARE_NOCOPY_NOMOVE( LogObserver );
};

//////////////////////////////////////////////////////////////////////////
// LogObserver class inline methods
//////////////////////////////////////////////////////////////////////////

inline LogObserver& LogObserver::self(void)
{
    return (*this);
}

#endif  // LUSAN_DATA_LOG_LOGOBSERVER_HPP
