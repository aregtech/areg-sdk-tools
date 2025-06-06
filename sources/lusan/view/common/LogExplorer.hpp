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

#include "lusan/view/common/NavigationWindow.hpp"
#include "areg/logging/NELogging.hpp"

#include <QItemSelection>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LogScopesModel;
class MdiMainWindow;
class MdiChild;
class QToolButton;
class QTreeView;
class QAction;

namespace Ui {
    class LogExplorer;
}

//////////////////////////////////////////////////////////////////////////
// LogExplorer class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The LogExplorer class is a view of the logging sources and logging scopes.
 **/
class LogExplorer : public NavigationWindow
{
private:

    //!< The priority indexes for the menu entries.
    enum eLogActions
    {
          PrioNotset    = 0
        , PrioDebug
        , PrioInfo
        , PrioWarn
        , PrioError
        , PrioFatal
        , PrioScope
        , SavePrioTarget
        , SavePrioAll

        , PrioCount
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The constructor of the LogExplorer class.
     * \param   wndMain     The main frame of the application.
     * \param   parent      The parent widget.
     **/
    LogExplorer(MdiMainWindow* wndMain, QWidget* parent = nullptr);

    virtual ~LogExplorer(void);

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

    //!< Returns the control object to move to the bottom of log window.
    QToolButton* ctrlMoveBottom(void);

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

    /**
     * \brief   Enables or disables lot priority tool buttons based on selection index.
     *          It also changes the colors of the buttons depending on the priority.
     **/
    void enableButtons(const QModelIndex& selection);

    /**
     * \brief   Updates the colors of the log priority tool buttons.
     * \param   errSelected    If true, the error button is checked and the colored.
     * \param   warnSelected   If true, the warning button is checked and the colored.
     * \param   infoSelected   If true, the info button is checked and the colored.
     * \param   dbgSelected    If true, the debug button is checked and the colored.
     * \param   scopeSelected  If true, the scopes button is checked and the colored.
     **/
    void updateColors(bool errSelected, bool warnSelected, bool infoSelected, bool dbgSelected, bool scopeSelected);

    /**
     * \brief   Updates the expanded of the log scopes model based on the current index.
     * \param   current    The current index to update expanded.
     **/
    void updateExpanded(const QModelIndex& current);

    /**
     * \brief   Updates the priority of the log scope at the given index.
     * \param   node       The index of the log scope to update priority.
     * \param   addPrio    If true, adds the priority. Otherwise, removes the priority.
     * \param   prio       The log priority to set or remove.
     * \return  Returns true if succeeded the request to update the priority. Otherwise, returns false.
     **/
    bool updatePriority(const QModelIndex& node, bool addPrio, NELogging::eLogPriority prio);

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

    /**
     * \brief   The slot is triggered when the move to bottom tool button is clicked.
     **/
    void onMoveBottomClicked();

    // Slot for error log priority tool button
    void onPrioErrorClicked(bool checked);

    // Slot for warning log priority tool button
    void onPrioWarningClicked(bool checked);

    // Slot for information log priority tool button
    void onPrioInfoClicked(bool checked);

    // Slot for debug log priority tool button
    void onPrioDebugClicked(bool checked);

    // Slot for log scope priority tool button
    void onPrioScopesClicked(bool checked);

    // Slot for saving log priority changes on the target configuration.
    void onSaveSettingsClicked(bool checked);

    // Slot. which triggered when the selection in the log scopes navigation is changed.
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

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

    /**
     * \brief   Slot triggered when the user makes right click on the scope navigation window.
     * \param   pos     The mouse right click cursor position on scope navigation window.
     **/
    void onTreeViewContextMenuRequested(const QPoint& pos);

    /**
     * \brief   The slot is triggered when the application is about to exit.
     * \param   mdiChild    The MDI child window that is about to be closed.
     **/
    void onWindowCreated(MdiChild* mdiChild);

    /**
     * \brief   The slot is triggered when the MDI child window is activated.
     * \param   mdiChild    The MDI child window that is activated.
     **/
    void onWindowActivated(MdiChild* mdiChild);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::LogExplorer*        ui;             //!< The user interface object.
    QString                 mAddress;       //!< The IP-address of the log collector.
    uint16_t                mPort;          //!< The TCP port of the log collector.
    QString                 mInitLogFile;   //!< The initialized log file.
    QString                 mActiveLogFile; //!< The active log file.
    QString                 mLogLocation;   //!< The location of log files.
    LogScopesModel*         mModel;         //!< The model of the log scopes.
    QItemSelectionModel*    mSelModel;      //!< The item selection model to catch selection events.
    bool                    mSignalsActive; //!< The flag, indicating whether the log observer signals are active or not.
    QAction*                mMenuActions[static_cast<int>(eLogActions::PrioCount)];   //!< The list of menu actions
};

#endif  // LUSAN_VIEW_COMMON_LOGEXPLORER_HPP
