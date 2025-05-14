#ifndef LUSAN_VIEW_COMMON_LOGEXPLORER_HPP
#define LUSAN_VIEW_COMMON_LOGEXPLORER_HPP
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
 *  \file        lusan/view/common/LogExplorer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the log explorer.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QList>
#include <QString>
#include <QWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LogScopesModel;
class MdiMainWindow;
class QToolButton;
class QTreeView;

namespace Ui {
    class LogExplorer;
}

//////////////////////////////////////////////////////////////////////////
// LogExplorer class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The LogExplorer class is a view of the logging sources and logging scopes.
 **/
class LogExplorer : public    QWidget
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The constructor of the LogExplorer class.
     * \param   mainFrame   The main frame of the application.
     * \param   parent      The parent widget.
     **/
    LogExplorer(MdiMainWindow* mainFrame, QWidget* parent = nullptr);

    /**
     * \brief   Returns the IP-address of the log collector to connect.
     **/
    const QString & getLogCollectorAddress(void) const;

    /**
     * \brief   Sets the IP-address of the log collector to connect.
     * \param   address     The IP-address of the log collector service to connect.
     **/
    void setLogCollectorAddress(const QString & address);

    /**
     * \brief   Returns the TCP/IP port number of the log collector to connect.
     **/
    uint16_t getLogCollectorPort(void) const;

    /**
     * \brief   Sets the TCP/IP port number of the log collector to connect.
     * \param   port    The port number of the log collector service to connect.
     **/
    void setLogCollectorPort(uint16_t port);

    /**
     * \brief   Sets the IP-address and the TCP port number of the log collector service to connect.
     * @param   address     The IP-address of the log collector service to connect.
     * @param   port        The TCP port number of the log collector service to connect.
     **/
    void setLogCollectorConnection(const QString& address, uint16_t port);

    /**
     * \brief   Connects or disconnects log observer related signals and slots.
     * \param   setup   The flag, indicating whether the signals and slots are connector or not.
     *                  If `true`, the signals and slots are connected.
     *                  If `false`, the signals and slots are disconnected.
     **/
    void setupLogSignals(bool setup);
    
//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:

    //!< Returns the control object to expand or collapse entries of scopes.
    QToolButton* ctrlCollapse(void);

    //!< Returns the control object to connect to log observer service.
    QToolButton* ctrlConnect(void);

    //!< Returns the control object to open settings.
    QToolButton* ctrlSettings(void);

    //!< Returns the control object to save current settings.
    QToolButton* ctrlSaveSettings(void);

    //!< Returns the control object to find a string.
    QToolButton* ctrlFind(void);

    //!< Returns the control object to set error level of the logs
    QToolButton* ctrlLogError(void);

    //!< Returns the control object to set warning level of the logs
    QToolButton* ctrlLogWarning(void);

    //!< Returns the control object to set information level of the logs
    QToolButton* ctrlLogInfo(void);

    //!< Returns the control object to set debug level of the logs
    QToolButton* ctrlLogDebug(void);

    //!< Returns the control object to enable log scopes of the logs
    QToolButton* ctrlLogScopes(void);

    //!< Returns the control object of the log messages
    QTreeView* ctrlTable(void);

    /**
     * \brief   Updates the data of the file system.
     **/
    void updateData(void);
    
    /**
     * \brief   Initializes the widgets.
     **/
    void setupWidgets(void);
    
    /**
     * \brief   Initializes the signals.
     **/
    void setupSignals(void);
    
    /**
     * \brief   Blocks the basic signals.
     * \param   block   If true, blocks the signals. Otherwise, unblocks the signals.
     **/
    void blockBasicSignals(bool block);
    
private slots:
    /**
     * \brief   The slot is triggered when initializing and configuring the observer.
     * \param   isEnabled       The flag, indicating whether the logging service is enabled or not.
     * \param   address         The IP address of the log collector service set in the configuration file.
     * \param   port            The IP port number of the log collector service set in the configuration file.
     **/
    void onLogObserverConfigured(bool isEnabled, const QString& address, uint16_t port);

    /**
     * \brief   The slot is triggered when initializing and configuring the observer.
     * \param   isEnabled       The flag, indicating whether the logging in the database is enabler or not.
     * \param   dbName          The name of the  supported database.
     * \param   dbLocation      The relative or absolute path the database. The path may contain a mask.
     * \param   dbUser          The database user to use when log in. If null or empty, the database may not require the user name.
     **/
    void onLogDbConfigured(bool isEnabled, const QString& dbName, const QString& dbLocation, const QString& dbUser);

    /**
     * \brief   The slot is triggered when the observer connects or disconnects from the log collector service.
     * \param   isConnected     Flag, indicating whether observer is connected or disconnected.
     * \param   address         The IP address of the log collector service to connect or disconnect.
     * \param   port            The IP port number of the log collector service to connect or disconnect.
     **/
    void onLogServiceConnected(bool isConnected, const QString& address, uint16_t port);

    /**
     * \brief   The slot is trigger when starting or pausing the log observer.
     * \param   isStarted       The flag indicating whether the lob observer is started or paused.
     **/
    void onLogObserverStarted(bool isStarted);

    /**
     * \brief   The slot is triggered when the logging database is created.
     * \param   dbLocation      The relative or absolute path to the logging database.
     **/
    void onLogDbCreated(const QString& dbLocation);

    /**
     * \brief   The slot is triggered when fails to send or receive message.
     **/
    void onConnectClicked(bool checked);
    
private:
    MdiMainWindow*          mMainFrame;     //!< The main frame of the application.
    Ui::LogExplorer*        ui;             //!< The user interface object.
    QString                 mAddress;       //!< The IP-address of the log collector.
    uint16_t                mPort;          //!< The TCP port of the log collector.
    QString                 mInitLogFile;   //!< The initialized log file.
    QString                 mActiveLogFile; //!< The active log file.
    QString                 mLogLocation;   //!< The location of log files.
    bool                    mShouldConnect; //!< Flag, indicating to connect to log collector.
    LogScopesModel*         mModel;         //!< The model of the log scopes.
};

#endif  // LUSAN_VIEW_COMMON_LOGEXPLORER_HPP
