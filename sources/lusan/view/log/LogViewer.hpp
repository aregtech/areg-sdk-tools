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
class QLineEdit;
class QPushButton;
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

private:

    static const QString    _tooltipPauseLogging;   //!< Tooltip for the Pause logging button.
    static const QString    _tooltipResumeLogging;  //!< Tooltip for the Resume logging button.
    static const QString    _tooltipStopLogging;    //!< Tooltip for the Stop logging button.
    static const QString    _tooltipRestartLogging; //!< Tooltip for the Restart logging button.

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
     * \brief   Triggered when new table column is inserted.
     **/
    void onColumnsInserted(const QModelIndex &parent, int first, int last);

    /**
     * \brief   Triggered when a table column is removed.
     **/
    void onColumnsRemoved(const QModelIndex &parent, int first, int last);

    /**
     * \brief   Triggered when table columns are moved inserted from position to another.
     **/
    void onColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationColumn);

    /**
     * \brief   Slot, triggered when make mouse right click on header.
     **/
    void onHeaderContextMenu(const QPoint& pos);

    /**
     * \brief   Slot, triggered when make mouse right click on table view
     **/
    void onTableContextMenu(const QPoint& pos);

    /**
     * \brief   Slot, triggered when Pause / Resume toolbutton is clicked.
     * \param   checked     If true, requested to pause. If false, requested to resume.
     **/
    void onPauseClicked(bool checked);

    /**
     * \brief   Slot, triggered when Stop / Restart toolbutton is clicked.
     *          When stops, closes log database.
     *          When restarts, creates new database.
     * \param   checked     If true, requested to stop. If false, requested to restart logging.
     **/
    void onStopClicked(bool checked);

    /**
     * \brief   Slot, triggered when Clear logging window buggon is clicked.
     * @return
     **/
    void onClearClicked(void);

    /**
     * \brief   Slot, triggered when Search button is clicked.
     **/
    void onSearchClicked(void);

    /**
     * \brief   Slot, triggered when text in search field changes.
     * \param   text    The new text in the search field.
     **/
    void onSearchTextChanged(const QString& text);

    /**
     * \brief   Slot triggered when user right-clicks on search field.
     * \param   pos     The position of the context menu.
     **/
    void onSearchTextContextMenu(const QPoint& pos);

protected:
    /**
     * \brief   Override keyPressEvent to handle search shortcuts.
     * \param   event   The key press event.
     **/
    virtual void keyPressEvent(QKeyEvent* event) override;
    
private:
    //!< Returns the pointer to the log table object.
    QTableView* ctrlTable(void);

    //!< Returns the pointer to the header object.
    QHeaderView* ctrlHeader(void);

    //!< Returns Pause / Resume toolbutton
    QToolButton* ctrlPause(void);

    //!< Returns Stop / Restart toolbutton
    QToolButton* ctrlStop(void);

    //!< Returns Clear logs toolbutton
    QToolButton* ctrlClear(void);

    //!< Returns Logging File name label widget.
    QLabel* ctrlFile(void);

    //!< Returns Search text input field.
    QLineEdit* ctrlSearchText(void);

    //!< Returns Search button.
    QPushButton* ctrlSearchButton(void);

    /**
     * \brief   Populates menu and sets the action handlers.
     **/
    void populateColumnsMenu(QMenu* menu, int curRow);
    
    /**
     * \brief   Resets the order of the columns.
     **/
    void resetColumnOrder();

    /**
     * \brief   Updates the toolbuttons based on the current state of logging.
     * \param   isPaused    If true, logging is paused. Show resume button.
     * \param   isStopped   If true, logging is stopped. Show restart button.
     **/
    void updateToolbuttons(bool isPaused, bool isStopped);

    /**
     * \brief   Performs search in log messages.
     * \param   searchText      The text to search for.
     * \param   startFromRow    The row to start searching from.
     * \param   caseSensitive   Whether to perform case-sensitive search.
     * \param   searchColumn    The specific column to search in, or -1 for all columns.
     * \return  The row index of the found match, or -1 if not found.
     **/
    int performSearch(const QString& searchText, int startFromRow = 0, bool caseSensitive = false, int searchColumn = -1);
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::LogViewer*      ui;             //!< User interface object, generated by Qt Designer.
    LogViewerModel*     mLogModel;      //!< Model for the log viewer, handling the data and its representation.
    QWidget*            mMdiWindow;     //!< MDI window widget, used for displaying the log viewer in a multi-document interface.
    QString             mLastSearchText;//!< Last search text used for searching.
    int                 mLastFoundRow;  //!< Last found row for search navigation.
    bool                mCaseSensitiveSearch; //!< Flag indicating if search should be case-sensitive.
    int                 mSearchColumn;  //!< Column to search in (-1 for all columns).
};

#endif // LUSAN_VIEW_LOG_LOGVIEWER_HPP
