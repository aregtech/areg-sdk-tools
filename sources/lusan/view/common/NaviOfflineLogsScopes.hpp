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
    void setLoggingModel(LoggingModelBase * model) override;

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
     * \brief   Adds the open, close and refresh database tool buttons.
     **/
    void addSpecificTools(void) override;

    /**
     * \brief   Returns true, the offline explorer offers to select all priorities at once.
     **/
    bool hasSelectAllPrioMenu(void) const override;

private:

    //!< Returns the control object to open database files.
    inline QToolButton* ctrlOpenDatabase(void) const;

    //!< Returns the control object to close the current database.
    inline QToolButton* ctrlCloseDatabase(void) const;

    //!< Returns the control object to refresh the current database.
    inline QToolButton* ctrlRefreshDatabase(void) const;

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

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QToolButton*    mToolDbOpen;    //!< The tool button to open a log database file.
    QToolButton*    mToolDbClose;   //!< The tool button to close the opened log database file.
    QToolButton*    mToolRefresh;   //!< The tool button to reload the opened log database file.
};

//////////////////////////////////////////////////////////////////////////
// NaviOfflineLogsScopes class inline methods
//////////////////////////////////////////////////////////////////////////

inline QToolButton* NaviOfflineLogsScopes::ctrlOpenDatabase(void) const
{
    return mToolDbOpen;
}

inline QToolButton* NaviOfflineLogsScopes::ctrlCloseDatabase(void) const
{
    return mToolDbClose;
}

inline QToolButton* NaviOfflineLogsScopes::ctrlRefreshDatabase(void) const
{
    return mToolRefresh;
}

#endif  // LUSAN_VIEW_COMMON_NAVIOFFLINELOGSSCOPES_HPP
