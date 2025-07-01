#ifndef LUSAN_VIEW_COMMON_LOGOFFLINEEXPLORER_HPP
#define LUSAN_VIEW_COMMON_LOGOFFLINEEXPLORER_HPP
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
 *  \file        lusan/view/common/LogOfflineExplorer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the offline log explorer.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/view/common/NavigationWindow.hpp"

#include <QWidget>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LogOfflineModel;
class MdiMainWindow;
class QToolButton;
class QTreeView;
class QVBoxLayout;
class QFileDialog;

namespace Ui {
    class LogOfflineExplorer;
}

//////////////////////////////////////////////////////////////////////////
// LogOfflineExplorer class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The LogOfflineExplorer class is a view for offline log navigation.
 *          It provides functionality to load and browse log database files.
 **/
class LogOfflineExplorer : public NavigationWindow
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The constructor of the LogOfflineExplorer class.
     * \param   wndMain     The main frame of the application.
     * \param   parent      The parent widget.
     **/
    LogOfflineExplorer(MdiMainWindow* wndMain, QWidget* parent = nullptr);

    virtual ~LogOfflineExplorer(void);

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

    //!< Returns the control object to open database files.
    QToolButton* ctrlOpenDatabase(void);

    //!< Returns the control object to close the current database.
    QToolButton* ctrlCloseDatabase(void);

    //!< Returns the control object to refresh the current database.
    QToolButton* ctrlRefreshDatabase(void);

    //!< Returns the control object of the log database information display
    QTreeView* ctrlDatabaseInfo(void);

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

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::LogOfflineExplorer* ui;             //!< The user interface object.
    LogOfflineModel*        mModel;         //!< The offline log model.
    QString                 mDatabasePath;  //!< The path to the currently opened database.
};

#endif  // LUSAN_VIEW_COMMON_LOGOFFLINEEXPLORER_HPP