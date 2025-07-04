#ifndef LUSAN_MODEL_LOG_LOGVIEWERMODEL_HPP
#define LUSAN_MODEL_LOG_LOGVIEWERMODEL_HPP
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
 *  \file        lusan/model/log/LogViewerModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/log/LoggingModelBase.hpp"
#include "areg/component/NEService.hpp"
#include "areglogger/client/LogObserverApi.h"

#include <QList>
#include <QMap>

/**
 * \brief   The model for the log viewer window.
 **/
class LogViewerModel : public LoggingModelBase
{
    Q_OBJECT
    
//////////////////////////////////////////////////////////////////////////
// Static methods
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Generates and returns file name of the log database base on the information set in the initialization file.
     *          Normalizes the file name if it contains a mask, such as "log_%time%.sqlog".
     **/
    static QString generateFileName(void);
    
    /**
     * \brief   Generates and returns the path to the log database new file.
     *          The log database location and file name are set in the initialization file.
     *          Normalizes the file name if it contains a mask, such as "log_%time%.sqlog".
     **/
    static QString newFileName(void);
    
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogViewerModel(QObject *parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations and attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Call to start connection to the log collector service.
     * \param   hostName    The host name of the log collector service.
     * \param   portNr      The port number of the log collector service.
     * \return  True if the connection process started, false otherwise.
     **/
    bool connectService(const QString& hostName = "", unsigned short portNr = 0u);

    /**
     * \brief   Call to start disconnection from the log collector service.
     **/
    void disconnectService(void);

    /**
     * \brief   Triggered when service is connected.
     * \param   isConnected  True if the service is connected, false otherwise.
     * \param   address      The address of the service.
     * \param   port         The port of the service.
     * \param   dbPath       The path to the database.
     **/
    void serviceConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath);

    /**
     * \brief   Returns true if application is connected to the log collector service.
     **/
    inline bool isConnected(void) const;

    /**
     * \brief   Returns the address of the log collector service.
     **/
    inline QString getLofServiceAddress(void) const;

    /**
     * \brief   Returns the port of the log collector service.
     **/
    inline uint16_t getLogServicePort(void) const;

    /**
     * \brief   Call to pause logging. When logging is paused,
     *          on resume it continues writing logs in the same database.
     **/
    void pauseLogging(void);

    /**
     * \brief   Call to resume logging. When logging is resumed,
     *          it continues writing logs in the same database.
     *          Only paused logging can be resumed.
     **/
    void resumeLogging(void);

    /**
     * \brief   Call to stop logging. When logging is stopped,
     *          it closes the database and stops writing logs.
     *          On restart, it creates new database.
     **/
    void stopLogging(void);

    /**
     * \brief   Call to restart logging. When logging is restarted,
     *          it creates new database.
     * \param   dbName  The database name to restart logging.
     *                  If empty string, uses the name set in the configuration file.
     *                  The name may have a mask, such as "log_%time%.sqlog",
     **/
    void restartLogging(const QString & dbName = QString());

//////////////////////////////////////////////////////////////////////////
// Signals.
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   Signal emitted when receive the list of connected instances that make logs.
     * \param   instances   The list of the connected instances.
     **/
    void signalLogInstancesConnect(const QList< NEService::sServiceConnectedInstance >& instances);

    /**
     * \brief   Signal emitted when receive the list of disconnected instances that make logs.
     * \param   instances   The list of IDs of the disconnected instances.
     **/
    void signalLogInstancesDisconnect(const QList< NEService::sServiceConnectedInstance >& instances);

    /**
     * \brief   Signal emitted when connection with the log collector service is lost.
     * \param   instances   The list of disconnected instances.
     **/
    void signalLogServiceDisconnected(const QMap<ITEM_ID, NEService::sServiceConnectedInstance>& instances);

    /**
     * \brief   Signal emitted when receive the list of the scopes registered in an application.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of the scopes registered in the application. Each entry contains the ID of the scope, message priority and the full name.
     **/
    void signalLogRegisterScopes(ITEM_ID cookie, const QList<sLogScope *>& scopes);

    /**
     * \brief   Signal emitted when receive the list of previously registered scopes with new priorities.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of previously registered scopes. Each entry contains the ID of the scope, message priority and the full name.
     **/
    void signalLogUpdateScopes(ITEM_ID cookie, const QList<sLogScope *>& scopes);

//////////////////////////////////////////////////////////////////////////
// Slots.
//////////////////////////////////////////////////////////////////////////
private slots:
    /**
     * \brief   The slot is triggered when receive message to log.
     * \param   logMessage  The structure of the message to log.
     **/
    void slotLogMessage(const SharedBuffer& logMessage);
    
    /**
     * \brief   The slot is triggered when the observer connects or disconnects from the log collector service.
     * \param   isConnected     Flag, indicating whether observer is connected or disconnected.
     * \param   address         The IP address of the log collector service to connect or disconnect.
     * \param   port            The IP port number of the log collector service to connect or disconnect.
     **/
    void slotLogServiceConnected(bool isConnected, const QString& address, uint16_t port);

    /**
     * \brief   The slot is triggered when receive the list of connected instances that make logs.
     * \param   instances   The list of the connected instances.
     **/
    void slotLogInstancesConnect(const QList< NEService::sServiceConnectedInstance >& instances);

    /**
     * \brief   The slot is triggered when receive the list of disconnected instances that make logs.
     * \param   instances   The list of IDs of the disconnected instances.
     **/
    void slotLogInstancesDisconnect(const QList< NEService::sServiceConnectedInstance >& instances);

    /**
     * \brief   The slot is triggered when connection with the log collector service is lost.
     * \param   instances   The list of disconnected instances.
     **/
    void slotLogServiceDisconnected(const QMap<ITEM_ID, NEService::sServiceConnectedInstance>& instances);

    /**
     * \brief   The slot is triggered when receive the list of the scopes registered in an application.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of the scopes registered in the application. Each entry contains the ID of the scope, message priority and the full name.
     **/
    void slotLogRegisterScopes(ITEM_ID cookie, const QList<sLogScope*>& scopes);

    /**
     * \brief   The slot is triggered when receive the list of previously registered scopes with new priorities.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of previously registered scopes. Each entry contains the ID of the scope, message priority and the full name.
     **/
    void slotLogUpdateScopes(ITEM_ID cookie, const QList<sLogScope*>& scopes);
    
//////////////////////////////////////////////////////////////////////////
// Member variable
//////////////////////////////////////////////////////////////////////////
private:
    bool                    mIsConnected;   //!< Flag to indicate whether application is connected to log collector service
    QString                 mAddress;       //!< The address of the log collector service
    uint16_t                mPort;          //!< The port of the log collector service
    QMetaObject::Connection mConLogger;     //!< The connection signal for log messages
    QMetaObject::Connection mConLogs;       //!< The connection signal for service connection
    QMetaObject::Connection mConInstancesConnect;    //!< The connection signal for instances connect
    QMetaObject::Connection mConInstancesDisconnect; //!< The connection signal for instances disconnect
    QMetaObject::Connection mConServiceDisconnected; //!< The connection signal for service disconnected
    QMetaObject::Connection mConRegisterScopes;      //!< The connection signal for register scopes
    QMetaObject::Connection mConUpdateScopes;        //!< The connection signal for update scopes
};

//////////////////////////////////////////////////////////////////////////
// LogViewerModel class inline methods.
//////////////////////////////////////////////////////////////////////////

inline bool LogViewerModel::isConnected(void) const
{
    return mIsConnected;
}

inline QString LogViewerModel::getLofServiceAddress(void) const
{
    return mAddress;
}

inline uint16_t LogViewerModel::getLogServicePort(void) const
{
    return mPort;
}

#endif // LUSAN_MODEL_LOG_LOGVIEWERMODEL_HPP
