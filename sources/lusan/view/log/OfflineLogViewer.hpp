#ifndef LUSAN_VIEW_LOG_OFFLINELOGVIEWER_HPP
#define LUSAN_VIEW_LOG_OFFLINELOGVIEWER_HPP
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
 *  \file        lusan/view/log/OfflineLogViewer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, offline log viewer widget.
 *
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/MdiChild.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class QHeaderView;
class QTableView;
class QLabel;
class QWidget;
class OfflineLogsModel;
class LiveLogViewer;
class LogViewerFilterProxy;
class LogTableHeader;
class MdiMainWindow;
class SearchLineEdit;

namespace Ui {
    class OfflineLogViewer;
}

/**
 * \brief   Offline log viewer MDI window for viewing log files (.sqlog)
 **/
class OfflineLogViewer : public MdiChild
{
    Q_OBJECT

public:
    /**
     * \brief   Returns the file extension of the offline log files.
     **/
    static const QString& fileExtension(void);

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Constructor.
     * \param   wndMain    The main window of the application.
     * \param   parent     The parent widget.
     **/
    explicit OfflineLogViewer(MdiMainWindow *wndMain, QWidget *parent = nullptr);

    /**
     * \brief   Constructor to use to make a copy of data from the live log viewer.
     * \param   wndMain    The main window of the application.
     * \param   liveLogs   The live log viewer object to copy data from.
     * \param   parent     The parent widget.
     **/
    explicit OfflineLogViewer(MdiMainWindow* wndMain, LiveLogViewer & liveLogs, QWidget* parent = nullptr);

    virtual ~OfflineLogViewer(void);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns true if the offline log database is successfully opened.
     **/
    bool isDatabaseOpen(void) const;

    /**
     * \brief   Returns the pointer to the logging model used by this viewer.
     **/
    inline OfflineLogsModel* getLoggingModel(void);

    /**
     * \brief   Opens the offline log database file.
     * \param   logPath     The path to the log database file (.sqlog).
     * \return  Returns true if the database is successfully opened, false otherwise.
     **/
    bool openDatabase(const QString & logPath);

/************************************************************************
 * MdiChild overrides
 ************************************************************************/
protected:

    /**
     * \brief   Called when the MDI child window is closed.
     * \param   isActive    Indicates whether the window is active or not.
     **/
    virtual void onWindowClosing(bool isActive) override;

    /**
     * \brief   Called when the MDI child window is activated.
     *          This method can be overridden to handle window activation events.
     **/
    virtual void onWindowActivated(void) override;

//////////////////////////////////////////////////////////////////////////
// Slots.
//////////////////////////////////////////////////////////////////////////
private slots:

    /**
     * \brief   Slot, triggered when make mouse right click on header.
     **/
    void onHeaderContextMenu(const QPoint& pos);

    /**
     * \brief   Slot, triggered when make mouse right click on table view
     **/
    void onTableContextMenu(const QPoint& pos);

    /**
     * \brief   Slot, triggered when database is successfully opened.
     **/
    void onDatabaseOpened(const QString& dbPath);

    /**
     * \brief   Slot, triggered when database is closed.
     **/
    void onDatabaseClosed(const QString& dbPath);

private:
    //!< Returns the pointer to the log table object.
    QTableView* ctrlTable(void);

    //!< Returns the pointer to the header object.
    QHeaderView* ctrlHeader(void);

    //!< Returns Logging File name label widget.
    QLabel* ctrlFile(void);

    //!< Returns the pointer to the search line edit control.
    SearchLineEdit* ctrlSearchText(void);

    //!< Returns the pointer to the search next button of the search line edit control.
    QToolButton* ctrlButtonSearch(void);

    //!< Returns the pointer to the search case sensitive button of the search line edit control.
    QToolButton* ctrlButtonCaseSensitive(void);

    //!< Returns the pointer to the search match word button of the search line edit control.
    QToolButton* ctrlButtonWholeWords(void);

    //!< Returns the pointer to the search wild card button of the search line edit control.
    QToolButton* ctrlSearchWildcard(void);

    //!< Returns the pointer to the search backward button of the search line edit control.
    QToolButton* ctrlSearchBackward(void);

    /**
     * \brief   Populates menu and sets the action handlers.
     **/
    void populateColumnsMenu(QMenu* menu, int curRow);
    
    /**
     * \brief   Resets the order of the columns.
     **/
    void resetColumnOrder();

    /**
     * \brief   Sets up or clears the offline log viewer signals.
     * \param   doSetup     If true, sets up the signals. Otherwise, clears the signal.
     **/
    void setupSignals(bool doSetup);

    /**
     * \brief   Sets up the widgets of the offline log viewer.
     *          This method initializes the user interface components.
     **/
    void setupWidgets(void);

    /**
     * \brief   Cleans up resources used by the offline log viewer.
     *          This method is called when the viewer is closed or no longer needed.
     **/
    void cleanResources(void);
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::OfflineLogViewer*   ui;         //!< User interface object, generated by Qt Designer.
    OfflineLogsModel*       mLogModel;  //!< Model for the offline log viewer, handling the data and its representation.
    LogViewerFilterProxy*   mFilter;    //!< The filter object
    QWidget*                mMdiWindow; //!< MDI window widget, used for displaying the log viewer in a multi-document interface.
    LogTableHeader*         mHeader;    //<!< Log table header object, used for managing the header of the log table.
};

//////////////////////////////////////////////////////////////////////////
// OfflineLogViewer inline methods implementation
//////////////////////////////////////////////////////////////////////////

inline OfflineLogsModel* OfflineLogViewer::getLoggingModel(void)
{
    return mLogModel;
}

#endif // LUSAN_VIEW_LOG_OFFLINELOGVIEWER_HPP
