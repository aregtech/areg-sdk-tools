#ifndef LUSAN_VIEW_COMMON_NAVIOFFLINELOGSSCOPES_HPP
#define LUSAN_VIEW_COMMON_NAVIOFFLINELOGSSCOPES_HPP
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
 *  \file        lusan/view/common/NaviOfflineLogsScopes.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the offline log explorer.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/view/common/NaviLogScopeBase.hpp"

#include <QList>
#include <QString>
#include <QWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LoggingModelBase;
class OfflineLogsModel;
class OfflineScopesModel;
class MdiMainWindow;
class ScopeNodeBase;
class QAction;
class QToolButton;
class QTreeView;
class QVBoxLayout;
class QFileDialog;

namespace Ui {
    class NaviOfflineLogsScopes;
}

//////////////////////////////////////////////////////////////////////////
// NaviOfflineLogsScopes class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The NaviOfflineLogsScopes class is a view for offline log navigation.
 *          It provides functionality to load and browse log database files.
 **/
class NaviOfflineLogsScopes : public NaviLogScopeBase
{
    Q_OBJECT

    //!< The priority indexes for the context menu entries.
    enum eLogActions
    {
          PrioNotset    = 0 //!< Reset priorities
        , PrioAllset        //!< Set all priorities
        , PrioDebug         //!< Set debug priority
        , PrioInfo          //!< Set info priority
        , PrioWarn          //!< Set warning priority
        , PrioError         //!< Set error priority
        , PrioFatal         //!< Set fatal priority
        , PrioScope         //!< Set scope priority
        , ExpandSelected    //!< Expands selected node
        , CollapseSelected  //!< Collapse selected node
        , ExpandAll         //!< Expand all nodes
        , CollapseAll       //!< Collapse all nodes

        , PrioCount         //!< The number of entries in the menu
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The constructor of the NaviOfflineLogsScopes class.
     * \param   wndMain     The main frame of the application.
     * \param   parent      The parent widget.
     **/
    NaviOfflineLogsScopes(MdiMainWindow* wndMain, QWidget* parent = nullptr);

    virtual ~NaviOfflineLogsScopes();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the currently opened database file path.
     **/
    QString getOpenedDatabasePath() const;

    /**
     * \brief   Opens a log database file for offline analysis.
     * \param   filePath    The path to the log database file to open.
     * \return  True if the database was opened successfully, false otherwise.
     **/
    bool openDatabase(const QString& filePath);

    /**
     * \brief   Closes the currently opened database.
     **/
    void closeDatabase();

    /**
     * \brief   Returns true if a database is currently open.
     **/
    bool isDatabaseOpen() const;

    /**
     * \brief   Sets the currently active logging model object.
     *          If model is valid and the logs are read from database, it will automatically update scope data.
     *          Otherwise, the scope explorer is reset and no scopes are displayed.
     * \param   model   The offline logging data model to read log data.
     *                  If null or database is not opened, it resets the scope explorer.
     **/
    virtual void setLoggingModel(LoggingModelBase * model) override;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   This method is called when the options dialog is opened.
     **/
    virtual void optionOpenning() override;

    /**
     * \brief   This method is called when the apply button in options dialog is pressed.
     *          It can be used to apply changes made in the options dialog.
     **/
    virtual void optionApplied() override;

    /**
     * \brief   This method is called when the options dialog is closed.
     * \param   OKpressed   True if OK button was pressed, false if Cancel button was pressed.
     **/
    virtual void optionClosed(bool OKpressed) override;

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:

    //!< Returns the control object to expand or collapse entries of scopes.
    QToolButton* ctrlCollapse() const;

    //!< Returns the control object to open database files.
    QToolButton* ctrlOpenDatabase() const;

    //!< Returns the control object to close the current database.
    QToolButton* ctrlCloseDatabase() const;

    //!< Returns the control object to refresh the current database.
    QToolButton* ctrlRefreshDatabase() const;

    //!< Returns the control object to find a string.
    QToolButton* ctrlFind() const;

    //!< Returns the control object to set error level of the logs
    QToolButton* ctrlLogError() const;

    //!< Returns the control object to set warning level of the logs
    QToolButton* ctrlLogWarning() const;

    //!< Returns the control object to set information level of the logs
    QToolButton* ctrlLogInfo() const;

    //!< Returns the control object to set debug level of the logs
    QToolButton* ctrlLogDebug() const;

    //!< Returns the control object to enable log scopes of the logs
    QToolButton* ctrlLogScopes() const;

    //!< Returns the control object to move to the top of log window.
    QToolButton* ctrlMoveTop() const;

    //!< Returns the control object to move to the bottom of log window.
    QToolButton* ctrlMoveBottom() const;

    //!< Returns the control object of the log messages
    QTreeView* ctrlTable() const;

    /**
     * \brief   Initializes the widgets.
     **/
    void setupWidgets();

    /**
     * \brief   Initializes the signals.
     **/
    void setupSignals();

    /**
     * \brief   Updates the UI controls based on database state.
     **/
    void updateControls();

    /**
     * \brief   Shows database information and available log data.
     **/
    void showDatabaseInfo();

    /**
     * \brief   Updates the data of the scope tree, restores the view from the data like expanded and
     *          selected nodes set in the logging model.
     **/
    void restoreView();

    //!< Returns the accumulated selected priorities.
    uint32_t getSelectedPrios() const;

private slots:
    /**
     * \brief   The slot is triggered when the open database tool button is clicked.
     **/
    void onOpenDatabaseClicked();

    /**
     * \brief   The slot is triggered when the close database tool button is clicked.
     **/
    void onCloseDatabaseClicked();

    /**
     * \brief   The slot is triggered when the refresh database tool button is clicked.
     **/
    void onRefreshDatabaseClicked();

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
     * \brief   Slot triggered when the user makes right click on the scope navigation window.
     * \param   pos     The mouse right click cursor position on scope navigation window.
     **/
    void onTreeViewContextMenuRequested(const QPoint& pos);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::NaviOfflineLogsScopes*  ui;             //!< The user interface object.
    QList<QAction*>             mMenuActions;   //!< The list of menu actions
};

#endif  // LUSAN_VIEW_COMMON_NAVIOFFLINELOGSSCOPES_HPP
