#ifndef LUSAN_VIEW_LOG_LOGVIEWER_HPP
#define LUSAN_VIEW_LOG_LOGVIEWER_HPP
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
 *  \file        lusan/view/log/LogViewer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view widget.
 *
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/MdiChild.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class QHeaderView;
class QTableView;
class QToolbutton;
class QLabel;
class QWidget;
class LogViewerModel;
class MdiMainWindow;

namespace Ui {
    class LogViewer;
}

/**
 * \brief   Log viewer MDI window
 **/
class LogViewer : public MdiChild
{
    Q_OBJECT

public:
    /**
     * \brief   Returns the file extension of the service interface document.
     **/
    static const QString& fileExtension(void);

    /**
     * \brief   Returns the file name of the log viewer.
     *          The file name is generated based on the current date and time.
     **/
    static QString generateFileName(void);

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Constructor.
     * \param   wndMain    The main window of the application.
     * \param   parent     The parent widget.
     **/
    explicit LogViewer(MdiMainWindow *wndMain, QWidget *parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Called when lusan application is connected to the logging service.
     * \param   isConnected     Flag, indicating that whether the service is connected or disconnected.
     * \param   address         The address of the connected logging service
     * \param   port            The TCP port number of the connected logging service.
     * \param   dbPath          The path to the logging database.
     **/
    void logServiceConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath);

    /**
     * \brief   Called when the log database is created.
     **/
    void logDatabaseCreated(const QString& dbPath);

    /**
     * \brief   Returns true if application is connected to the logging service.
     **/
    bool isServiceConnected(void) const;

    /**
     * \brief   Called to move to the bottom of the logs.
     * \param   lastSelect    If true, the last row of logs will be selected after moving to the bottom.
     **/
    void moveToBottom(bool lastSelect);

    /**
     * \brief   Called to check whether log viewer has log entries.
     **/
    bool isEmpty(void) const;

    /**
     * \brief   Called to detach the log viewer and stop receiving messages.
     **/
    void detachLiveLog(void);

//////////////////////////////////////////////////////////////////////////
// Slots.
//////////////////////////////////////////////////////////////////////////
private slots:

    /**
     * \brief   Triggered when new entry is inserted in the table
     **/
    void onRowsInserted(const QModelIndex &parent, int first, int last);

    /**
     * \brief   Slot, triggered when make mouse right click on header.
     **/
    void onHeaderContextMenu(const QPoint& pos);

    /**
     * \brief   Slot, triggered when make mouse right click on table view
     **/
    void onTableContextMenu(const QPoint& pos);

    /**
     * \brief   Slot, triggered when Pause toolbutton is clicked.
     **/
    void onPauseClicked(void);

    /**
     * \brief   Slot, triggered when Resume toolbutton is clicked.
     *          It resumes the logging process.
     **/
    void onResumeClicked(void);
    
    /**
     * \brief   Slot, triggered when Stop toolbutton is clicked.
     **/
    void onStopClicked(void);
    
    /**
     * \brief   Slot, triggered when Restart toolbutton is clicked.
     *          It restarts the logging process in a new database.
     **/
    void onRestartClicked(void);
    
private:
    //!< Returns the pointer to the log table object.
    QTableView* ctrlTable(void);

    //!< Returns the pointer to the header object.
    QHeaderView* ctrlHeader(void);

    //!< Returns Pause toolbutton
    QToolButton* ctrlPause(void);

    //!< Returns Resume toolbutton
    QToolButton* ctrlResume(void);

    //!< Returns Stop toolbutton
    QToolButton* ctrlStop(void);

    //!< Returns Restart toolbutton.
    QToolButton* ctrlRestart(void);

    //!< Returns Logging File name label widget.
    QLabel* ctrlFile(void);

    /**
     * \brief   Populates menu and sets the action handlers.
     **/
    void populateColumnsMenu(QMenu* menu, int curRow);
    
    /**
     * \brief   Resets the order of the columns.
     **/
    void resetColumnOrder();
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::LogViewer*      ui;         //!< User interface object, generated by Qt Designer.
    LogViewerModel*     mLogModel;  //!< Model for the log viewer, handling the data and its representation.
    QWidget*            mMdiWindow; //!< MDI window widget, used for displaying the log viewer in a multi-document interface.
};

#endif // LUSAN_VIEW_LOG_LOGVIEWER_HPP
