#ifndef LUSAN_VIEW_COMMON_NAVIOFFLINELOGSSCOPES_HPP
#define LUSAN_VIEW_COMMON_NAVIOFFLINELOGSSCOPES_HPP
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
 *  \file        lusan/view/common/NaviOfflineLogsScopes.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
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

    virtual ~NaviOfflineLogsScopes(void);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the currently opened database file path.
     **/
    QString getOpenedDatabasePath(void) const;

    /**
     * \brief   Opens a log database file for offline analysis.
     * \param   filePath    The path to the log database file to open.
     * \return  True if the database was opened successfully, false otherwise.
     **/
    bool openDatabase(const QString& filePath);

    /**
     * \brief   Closes the currently opened database.
     **/
    void closeDatabase(void);

    /**
     * \brief   Returns true if a database is currently open.
     **/
    bool isDatabaseOpen(void) const;

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
    virtual void optionOpenning(void) override;

    /**
     * \brief   This method is called when the apply button in options dialog is pressed.
     *          It can be used to apply changes made in the options dialog.
     **/
    virtual void optionApplied(void) override;

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
    QToolButton* ctrlCollapse(void) const;

    //!< Returns the control object to open database files.
    QToolButton* ctrlOpenDatabase(void) const;

    //!< Returns the control object to close the current database.
    QToolButton* ctrlCloseDatabase(void) const;

    //!< Returns the control object to refresh the current database.
    QToolButton* ctrlRefreshDatabase(void) const;

    //!< Returns the control object to find a string.
    QToolButton* ctrlFind(void) const;

    //!< Returns the control object to set error level of the logs
    QToolButton* ctrlLogError(void) const;

    //!< Returns the control object to set warning level of the logs
    QToolButton* ctrlLogWarning(void) const;

    //!< Returns the control object to set information level of the logs
    QToolButton* ctrlLogInfo(void) const;

    //!< Returns the control object to set debug level of the logs
    QToolButton* ctrlLogDebug(void) const;

    //!< Returns the control object to enable log scopes of the logs
    QToolButton* ctrlLogScopes(void) const;

    //!< Returns the control object to move to the top of log window.
    QToolButton* ctrlMoveTop(void) const;

    //!< Returns the control object to move to the bottom of log window.
    QToolButton* ctrlMoveBottom(void) const;

    //!< Returns the control object of the log messages
    QTreeView* ctrlTable(void) const;

    /**
     * \brief   Initializes the widgets.
     **/
    void setupWidgets(void);

    /**
     * \brief   Initializes the signals.
     **/
    void setupSignals(void);

    /**
     * \brief   Updates the UI controls based on database state.
     **/
    void updateControls(void);

    /**
     * \brief   Shows database information and available log data.
     **/
    void showDatabaseInfo(void);

    /**
     * \brief   Updates the data of the scope tree, restores the view from the data like expanded and
     *          selected nodes set in the logging model.
     **/
    void restoreView(void);

    //!< Returns the accumulated selected priorities.
    uint32_t getSelectedPrios(void) const;

private slots:
    /**
     * \brief   The slot is triggered when the open database tool button is clicked.
     **/
    void onOpenDatabaseClicked(void);

    /**
     * \brief   The slot is triggered when the close database tool button is clicked.
     **/
    void onCloseDatabaseClicked(void);

    /**
     * \brief   The slot is triggered when the refresh database tool button is clicked.
     **/
    void onRefreshDatabaseClicked(void);

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
