#ifndef LUSAN_VIEW_COMMON_OFFLINELOGEXPLORER_HPP
#define LUSAN_VIEW_COMMON_OFFLINELOGEXPLORER_HPP
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
 *  \file        lusan/view/common/OfflineLogExplorer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The offline log explorer for navigating log scopes from database files.
 *
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
class LogViewer;
class MdiMainWindow;
class MdiChild;
class QToolButton;
class QTreeView;
class QAction;
class LogSqliteDatabase;

namespace Ui {
    class OfflineLogExplorer;
}

//////////////////////////////////////////////////////////////////////////
// OfflineLogExplorer class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The OfflineLogExplorer class is a view of the offline logging sources and logging scopes.
 *          It provides navigation for log scopes from offline log database files.
 **/
class OfflineLogExplorer : public NavigationWindow
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor.
     * \param   wndMain     The main window.
     * \param   parent      The parent widget.
     **/
    OfflineLogExplorer(MdiMainWindow* wndMain, QWidget* parent = nullptr);

    /**
     * \brief   Destructor.
     **/
    virtual ~OfflineLogExplorer(void);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Opens an offline log database file and loads its scope data.
     * \param   dbFilePath  The path to the offline log database file.
     * \return  Returns true if the database was successfully opened and scopes loaded.
     **/
    bool openLogDatabase(const QString& dbFilePath);

    /**
     * \brief   Closes the currently active log database.
     **/
    void closeLogDatabase(void);

    /**
     * \brief   Updates the offline scope navigation to reflect the currently active offline log viewer.
     * \param   activeViewer    The currently active offline log viewer.
     **/
    void updateForActiveViewer(LogViewer* activeViewer);

    /**
     * \brief   Returns true if a database is currently loaded.
     **/
    inline bool isDatabaseLoaded(void) const;

    /**
     * \brief   Returns the path of the currently loaded database.
     **/
    inline const QString& getDatabasePath(void) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Called when the options dialog is opened.
     **/
    virtual void optionOpenning(void) override;

    /**
     * \brief   Called when the apply button in options dialog is pressed.
     **/
    virtual void optionApplied(void) override;

    /**
     * \brief   Called when the options dialog is closed.
     * \param   OKpressed   True if OK button was pressed, false if Cancel button was pressed.
     **/
    virtual void optionClosed(bool OKpressed) override;

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns pointer to the tree view.
     **/
    inline QTreeView* ctrlTreeView(void) const;

    /**
     * \brief   Returns pointer to the open database tool button.
     **/
    inline QToolButton* ctrlOpenDatabase(void) const;

    /**
     * \brief   Returns pointer to the close database tool button.
     **/
    inline QToolButton* ctrlCloseDatabase(void) const;

    /**
     * \brief   Returns pointer to the collapse/expand tool button.
     **/
    inline QToolButton* ctrlCollapse(void) const;

    /**
     * \brief   Returns pointer to the reset filters tool button.
     **/
    inline QToolButton* ctrlResetFilters(void) const;

    /**
     * \brief   Returns pointer to the move to top tool button.
     **/
    inline QToolButton* ctrlMoveTop(void) const;

    /**
     * \brief   Returns pointer to the move to bottom tool button.
     **/
    inline QToolButton* ctrlMoveBottom(void) const;

    /**
     * \brief   Returns pointer to the priority error tool button.
     **/
    inline QToolButton* ctrlPrioError(void) const;

    /**
     * \brief   Returns pointer to the priority warning tool button.
     **/
    inline QToolButton* ctrlPrioWarning(void) const;

    /**
     * \brief   Returns pointer to the priority info tool button.
     **/
    inline QToolButton* ctrlPrioInfo(void) const;

    /**
     * \brief   Returns pointer to the priority debug tool button.
     **/
    inline QToolButton* ctrlPrioDebug(void) const;

    /**
     * \brief   Returns pointer to the priority scopes tool button.
     **/
    inline QToolButton* ctrlPrioScopes(void) const;

//////////////////////////////////////////////////////////////////////////
// Hidden operations
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Initializes the widget and sets up the user interface.
     **/
    void setupWidgets(void);

    /**
     * \brief   Sets up the signal connections for UI controls.
     **/
    void setupSignals(void);

    /**
     * \brief   Updates data from the database and refreshes the model.
     **/
    void updateData(void);

    /**
     * \brief   Loads scope data from the database into the model.
     **/
    void loadScopeData(void);

    /**
     * \brief   Enables or disables UI buttons based on the current state.
     * \param   index   The current model index.
     **/
    void enableButtons(const QModelIndex& index = QModelIndex());

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    /**
     * \brief   Slot for opening a log database file.
     **/
    void onOpenDatabaseClicked(void);

    /**
     * \brief   Slot for closing the current log database.
     **/
    void onCloseDatabaseClicked(void);

    /**
     * \brief   Slot for collapsing and expanding nodes.
     **/
    void onCollapseClicked(bool checked);

    /**
     * \brief   Slot for resetting all filters.
     **/
    void onResetFiltersClicked(void);

    /**
     * \brief   Slot for moving to top.
     **/
    void onMoveTopClicked(void);

    /**
     * \brief   Slot for moving to bottom.
     **/
    void onMoveBottomClicked(void);

    /**
     * \brief   Slot for log priority error tool button.
     **/
    void onPrioErrorClicked(bool checked);

    /**
     * \brief   Slot for log priority warning tool button.
     **/
    void onPrioWarningClicked(bool checked);

    /**
     * \brief   Slot for log priority info tool button.
     **/
    void onPrioInfoClicked(bool checked);

    /**
     * \brief   Slot for log priority debug tool button.
     **/
    void onPrioDebugClicked(bool checked);

    /**
     * \brief   Slot for log scope priority tool button.
     **/
    void onPrioScopesClicked(bool checked);

    /**
     * \brief   Slot triggered when the selection in the log scopes navigation is changed.
     **/
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    /**
     * \brief   The signal triggered when receive the list of connected instances that make logs.
     * \param   root   The root model index.
     **/
    void onRootUpdated(const QModelIndex & root);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::OfflineLogExplorer* ui;                 //!< The user interface.
    QString                 mDatabasePath;      //!< The path to the currently loaded database.
    LogSqliteDatabase*      mDatabase;          //!< The database connection.
    LogScopesModel*         mModel;             //!< The model of the log scopes.
    QItemSelectionModel*    mSelModel;          //!< The item selection model to catch selection events.
    bool                    mSignalsActive;     //!< Flag indicating whether signals are active.
    LogViewer*              mActiveViewer;      //!< The currently active offline log viewer.
    QList<QAction*>         mMenuActions;       //!< The list of menu actions.
};

//////////////////////////////////////////////////////////////////////////
// OfflineLogExplorer inline methods
//////////////////////////////////////////////////////////////////////////

inline bool OfflineLogExplorer::isDatabaseLoaded(void) const
{
    return (mDatabase != nullptr) && mDatabase->isOperable();
}

inline const QString& OfflineLogExplorer::getDatabasePath(void) const
{
    return mDatabasePath;
}

inline QTreeView* OfflineLogExplorer::ctrlTreeView(void) const
{
    return ui->treeView;
}

inline QToolButton* OfflineLogExplorer::ctrlOpenDatabase(void) const
{
    return ui->toolOpenDatabase;
}

inline QToolButton* OfflineLogExplorer::ctrlCloseDatabase(void) const
{
    return ui->toolCloseDatabase;
}

inline QToolButton* OfflineLogExplorer::ctrlCollapse(void) const
{
    return ui->toolCollapse;
}

inline QToolButton* OfflineLogExplorer::ctrlResetFilters(void) const
{
    return ui->toolResetFilters;
}

inline QToolButton* OfflineLogExplorer::ctrlMoveTop(void) const
{
    return ui->toolMoveTop;
}

inline QToolButton* OfflineLogExplorer::ctrlMoveBottom(void) const
{
    return ui->toolMoveBottom;
}

inline QToolButton* OfflineLogExplorer::ctrlPrioError(void) const
{
    return ui->toolError;
}

inline QToolButton* OfflineLogExplorer::ctrlPrioWarning(void) const
{
    return ui->toolWarning;
}

inline QToolButton* OfflineLogExplorer::ctrlPrioInfo(void) const
{
    return ui->toolInformation;
}

inline QToolButton* OfflineLogExplorer::ctrlPrioDebug(void) const
{
    return ui->toolDebug;
}

inline QToolButton* OfflineLogExplorer::ctrlPrioScopes(void) const
{
    return ui->toolScopes;
}

#endif  // LUSAN_VIEW_COMMON_OFFLINELOGEXPLORER_HPP