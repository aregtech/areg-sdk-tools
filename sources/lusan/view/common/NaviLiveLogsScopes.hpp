#ifndef LUSAN_VIEW_COMMON_NAVILIVELOGSSCOPES_HPP
#define LUSAN_VIEW_COMMON_NAVILIVELOGSSCOPES_HPP
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
 *  \file        lusan/view/common/NaviLiveLogsScopes.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the log explorer.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/view/common/NaviLogScopeBase.hpp"
#include "areg/logging/areg_log.h"

#include <QItemSelection>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LiveScopesModel;
class LiveLogsModel;
class LiveLogViewer;
class MdiMainWindow;
class MdiChild;
class QAction;
class QLabel;
class QTimer;
class QToolButton;
class QTreeView;

//////////////////////////////////////////////////////////////////////////
// NaviLiveLogsScopes class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The NaviLiveLogsScopes class is a view of the logging sources and logging scopes.
 **/
class NaviLiveLogsScopes : public NaviLogScopeBase
{
    Q_OBJECT

private:

    //!< The logging states.
    enum eLoggingStates
    {
          LoggingUndefined      = 0 //!< Undefined logging state
        , LoggingConfigured         //!< Logging is initialized, but not connected
        , LoggingConnecting         //!< Logging is waiting for the log collector service to answer
        , LoggingConnected          //!< Logging is connected to the log collector service
        , LoggingStopped            //!< Logging is stopped, but can be restarted
        , LoggingPaused             //!< Logging is paused, but can be resumed
        , LoggingRunning            //!< Logging is running and collecting logs
        , LoggingDisconnected       //!< Logging is disconnected from the log collector service
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The constructor of the NaviLiveLogsScopes class.
     * \param   wndMain     The main frame of the application.
     * \param   parent      The parent widget.
     **/
    NaviLiveLogsScopes(MdiMainWindow* wndMain, QWidget* parent = nullptr);

    virtual ~NaviLiveLogsScopes();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the IP-address of the log collector to connect.
     **/
    const QString & getLogCollectorAddress() const;

    /**
     * \brief   Sets the IP-address of the log collector to connect.
     * \param   address     The IP-address of the log collector service to connect.
     **/
    void setLogCollectorAddress(const QString & address);

    /**
     * \brief   Returns the TCP/IP port number of the log collector to connect.
     **/
    uint16_t getLogCollectorPort() const;

    /**
     * \brief   Sets the TCP/IP port number of the log collector to connect.
     * \param   port    The port number of the log collector service to connect.
     **/
    void setLogCollectorPort(uint16_t port);

    /**
     * \brief   Sets the IP-address and the TCP port number of the log collector service to connect.
     * \param   address     The IP-address of the log collector service to connect.
     * \param   port        The TCP port number of the log collector service to connect.
     **/
    void setLogCollectorConnection(const QString& address, uint16_t port);

    /**
     * \brief   Disconnects the log collector service and releases the log observer, if any is
     *          active. Does nothing when the logging was never started.
     **/
    void disconnectLogging(void);


    //!< Returns true if the logging is configured.
    inline bool isConfigured() const;

    //!< Returns true if disconnected from log collector service.
    inline bool isDisconnected() const;

    //!< Returns true if connected to log observer service.
    inline bool isConnected() const;

    //!< Returns true while waiting for the log collector service to answer.
    inline bool isConnecting() const;

    //!< Returns true if connected to log observer service and receives messages.
    inline bool isRunning() const;

    //!< Returns true if connection is paused (disconnected) and can be restored.
    inline bool isPaused() const;

    //!< Returns true if connection is stopped (paused) and can be restored only when new data is applied.
    inline bool isStopped() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   This method is called when the options dialog is opened.
     **/
    void optionOpenning() override;

    /**
     * \brief   This method is called when the apply button in options dialog is pressed.
     *          It can be used to apply changes made in the options dialog.
     **/
    void optionApplied() override;

    /**
     * \brief   This method is called when the options dialog is closed.
     * \param   OKpressed   True if OK button was pressed, false if Cancel button was pressed.
     **/
    void optionClosed(bool OKpressed) override;

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Adds the connect, settings and save settings tool buttons.
     **/
    QToolButton* addSourceTool(void) override;

    void addExtraTools(void) override;

    /**
     * \brief   Returns true, the live explorer can save priorities on the logging targets.
     **/
    bool hasSavePrioMenu(void) const override;

    /**
     * \brief   Returns true if the log collector service is connected.
     **/
    bool canSavePrio(void) const override;

    /**
     * \brief   Draws the target sending button for the state the given tree entry reports.
     * \param   selection   The current tree entry, invalid when there is none.
     **/
    void refreshTargetControls(const QModelIndex& selection) override;

private:

    //!< Returns the control object to connect to log observer service.
    inline QToolButton* ctrlConnect(void) const;

    //!< Returns the control object to open settings.
    inline QToolButton* ctrlSettings(void) const;

    //!< Returns the control object to save current settings.
    inline QToolButton* ctrlSaveSettings(void) const;

    /**
     * \brief   Initializes the widgets.
     **/
    void setupWidgets();

    /**
     * \brief   Initializes the signals.
     **/
    void setupSignals();

    /**
     * \brief   Builds the row that says the panel is waiting for the log collector service.
     *          The row stays hidden until a connection is asked for.
     **/
    void setupConnectStatus();

    /**
     * \brief   Enters the waiting state: the panel keeps asking for the log collector service
     *          until it answers, and the row says so.
     **/
    void beginConnecting();

    /**
     * \brief   Leaves the waiting state and hides the row.
     **/
    void stopConnecting();

    /**
     * \brief   Counts one refused attempt and writes it into the row.
     **/
    void countAttempt();

    /**
     * \brief   Writes the current attempt into the row and shows it.
     **/
    void updateConnectStatus();

    /**
     * \brief   Asks for the log collector service again while the panel is waiting.
     **/
    void retryConnect();

    /**
     * \brief   Returns the absolute path of the database the live session writes into.
     **/
    QString databasePath() const;

    /**
     * \brief   Blocks the basic signals.
     * \param   block   If true, blocks the signals. Otherwise, unblocks the signals.
     **/
    void blockBasicSignals(bool block);

    /**
     * \brief   Connects or disconnects log observer related signals and slots.
     * \param   setup   The flag, indicating whether the signals and slots are connector or not.
     *                  If `true`, the signals and slots are connected.
     *                  If `false`, the signals and slots are disconnected.
     **/
    void setupLogSignals(bool setup);

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
     * \brief   The slot is triggered when the log observer instance is activated or shutdown.
     * \param   isStarted       The flag indicating whether the log observer instance is started or stopped.
     * \param   address         The IP address of the log observer instance.
     * \param   port            The TCP port number of the log observer instance.
     * \param   filePath        The file path of the log file, if any. If empty, no file is used.
     **/
    void onLogObserverInstance(bool isStarted, const QString& address, uint16_t port, const QString& filePath);

    /**
     * \brief   The slot is triggered when fails to send or receive message.
     **/
    void onConnectClicked(bool checked);

    // Slot for saving log priority changes on the target configuration.
    void onSaveSettingsClicked(bool checked);

    // Slot for opening the options dialog.
    void onOptionsClicked(bool checked);

    /**
     * \brief   The signal triggered when receive the list of connected instances that make logs.
     * \param   instances   The list of the connected instances.
     **/
    void onRootUpdated(const QModelIndex & root);

    /**
     * \brief   Slot triggered when the scopes of an instance are inserted.
     * \param   parent  The index of the parent instance item where scopes are inserted.
     **/
    void onScopesInserted(const QModelIndex & parent);

    /**
     * \brief   Slot triggered when the scopes of an instance are updated.
     * \param   parent  The index of the parent instance item that is updated.
     **/
    void onScopesUpdated(const QModelIndex & parent);

    /**
     * \brief   Slot triggered when the data of scopes are updated.
     * \param   topLeft     The top-left index of the node, which data is updated.
     * \param   bottomRight The bottom-right index of the node, which data is updated.
     * \param   roles       The list of roles, which data is updated. If empty, all roles are updated.
     **/
    void onScopesDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles = QList<int>());

//////////////////////////////////////////////////////////////////////////
// Static methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Callback to get notified that log observer service client has been started.
    static void _logObserverStarted();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QToolButton*            mToolConnect;   //!< The tool button to connect or disconnect the log collector.
    QToolButton*            mToolSettings;  //!< The tool button to open the logging options.
    QToolButton*            mToolSave;      //!< The tool button to save the log settings.
    QToolButton*            mToolTargetStop;   //!< The tool button that stops the selected target producing any log.
    QToolButton*            mToolTargetPause;  //!< The tool button that holds what the selected target sends.
    QToolButton*            mToolTargetResume; //!< The tool button that lets the selected target log and send again.
    QToolButton*            mToolTargetRestore;//!< The tool button that applies the priorities the target has saved.
    QString                 mAddress;       //!< The IP-address of the log collector.
    uint16_t                mPort;          //!< The TCP port of the log collector.
    QString                 mInitLogFile;   //!< The initialized log file.
    QString                 mActiveLogFile; //!< The active log file.
    QString                 mLogLocation;   //!< The location of log files.
    bool                    mSignalsActive; //!< The flag, indicating whether the log observer signals are active or not.
    eLoggingStates          mState;         //!< The variable to store live logging state.
    QWidget*                mConnectBar;    //!< The row that says the panel is waiting for the log collector, hidden when it is not.
    QLabel*                 mConnectText;   //!< What the panel is waiting for and how many attempts it took.
    QTimer*                 mRetryTimer;    //!< Asks for the log collector service again while the panel is waiting.
    int                     mAttempts;      //!< How many times the log collector service refused the connection.
};

//////////////////////////////////////////////////////////////////////////
// NaviLiveLogsScopes inline methods
//////////////////////////////////////////////////////////////////////////

inline QToolButton* NaviLiveLogsScopes::ctrlConnect(void) const
{
    return mToolConnect;
}

inline QToolButton* NaviLiveLogsScopes::ctrlSettings(void) const
{
    return mToolSettings;
}

inline QToolButton* NaviLiveLogsScopes::ctrlSaveSettings(void) const
{
    return mToolSave;
}

inline bool NaviLiveLogsScopes::isConfigured() const
{
    switch (mState)
    {
    case eLoggingStates::LoggingConfigured:
    case eLoggingStates::LoggingConnecting:
    case eLoggingStates::LoggingConnected:
    case eLoggingStates::LoggingPaused:
    case eLoggingStates::LoggingRunning:
    case eLoggingStates::LoggingDisconnected:
        return true;

    case eLoggingStates::LoggingUndefined:
    case eLoggingStates::LoggingStopped:
        return false;

    default:
        Q_ASSERT(false);
        return false;
    }
}

inline bool NaviLiveLogsScopes::isDisconnected() const
{
    switch (mState)
    {
    case eLoggingStates::LoggingUndefined:
    case eLoggingStates::LoggingConfigured:
    case eLoggingStates::LoggingConnecting:
    case eLoggingStates::LoggingDisconnected:
    case eLoggingStates::LoggingStopped:
    case eLoggingStates::LoggingPaused:
        return true;

    case eLoggingStates::LoggingConnected:
    case eLoggingStates::LoggingRunning:
        return false;

    default:
        Q_ASSERT(false);
        return false;
    }
}

inline bool NaviLiveLogsScopes::isConnected() const
{
    switch (mState)
    {
    case eLoggingStates::LoggingConnected:
    case eLoggingStates::LoggingRunning:
        return true;

    case eLoggingStates::LoggingUndefined:
    case eLoggingStates::LoggingConfigured:
    case eLoggingStates::LoggingConnecting:
    case eLoggingStates::LoggingDisconnected:
    case eLoggingStates::LoggingStopped:
    case eLoggingStates::LoggingPaused:
        return false;

    default:
        Q_ASSERT(false);
        return false;
    }
}

inline bool NaviLiveLogsScopes::isConnecting() const
{
    return mState == eLoggingStates::LoggingConnecting;
}

inline bool NaviLiveLogsScopes::isRunning() const
{
    switch (mState)
    {
    case eLoggingStates::LoggingRunning:
        return true;

    case eLoggingStates::LoggingUndefined:
    case eLoggingStates::LoggingConfigured:
    case eLoggingStates::LoggingConnecting:
    case eLoggingStates::LoggingDisconnected:
    case eLoggingStates::LoggingStopped:
    case eLoggingStates::LoggingPaused:
    case eLoggingStates::LoggingConnected:
        return false;

    default:
        Q_ASSERT(false);
        return false;
    }
}

inline bool NaviLiveLogsScopes::isPaused() const
{
    switch (mState)
    {
    case eLoggingStates::LoggingPaused:
        return true;

    case eLoggingStates::LoggingUndefined:
    case eLoggingStates::LoggingConfigured:
    case eLoggingStates::LoggingConnecting:
    case eLoggingStates::LoggingDisconnected:
    case eLoggingStates::LoggingStopped:
    case eLoggingStates::LoggingConnected:
    case eLoggingStates::LoggingRunning:
        return false;

    default:
        Q_ASSERT(false);
        return false;
    }
}

inline bool NaviLiveLogsScopes::isStopped() const
{
    switch (mState)
    {
    case eLoggingStates::LoggingStopped:
        return true;

    case eLoggingStates::LoggingUndefined:
    case eLoggingStates::LoggingConfigured:
    case eLoggingStates::LoggingConnecting:
    case eLoggingStates::LoggingDisconnected:
    case eLoggingStates::LoggingConnected:
    case eLoggingStates::LoggingRunning:
    case eLoggingStates::LoggingPaused:
        return false;

    default:
        Q_ASSERT(false);
        return false;
    }
}

#endif  // LUSAN_VIEW_COMMON_NAVILIVELOGSSCOPES_HPP
