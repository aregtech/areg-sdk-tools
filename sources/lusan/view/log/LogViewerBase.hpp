#ifndef LUSAN_VIEW_LOG_LOGVIEWERBASE_HPP
#define LUSAN_VIEW_LOG_LOGVIEWERBASE_HPP
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
 *  \file        lusan/view/log/LogViewerBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log viewer base widget.
 *
 ************************************************************************/
#include "lusan/view/common/MdiChild.hpp"
#include "lusan/model/log/LogSearchModel.hpp"
#include "areg/base/GEGlobal.h"

/************************************************************************
 * Dependencies
 ************************************************************************/
class LoggingModelBase;
class LogTableHeader;
class LogViewerFilter;
class SearchLineEdit;
class LogTextHighlight;

class QHeaderView;
class QModelIndex;
class QPoint;
class QString;
class QTableView;
class QToolButton;
class QWidget;

//////////////////////////////////////////////////////////////////////////
// LogViewerBase class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   Base class of the Log Viewer widget.
 **/
class LogViewerBase : public MdiChild
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
protected:

    /**
     * \brief   Constructor.
     * \param   windowType The type of the MDI window.
     * \param   logModel   The logging model to use for displaying logs.
     * \param   wndMain    The main window of the application.
     * \param   parent     The parent widget.
     **/
    explicit LogViewerBase(MdiChild::eMdiWindow windowType, LoggingModelBase* logModel, MdiMainWindow *wndMain, QWidget *parent = nullptr);

    virtual ~LogViewerBase(void);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the logging model used by the log viewer.
     **/
    inline LoggingModelBase* getLoggingModel(void) const;
    
    inline QTableView* getLoggingTable(void) const;

    /**
     * \brief   Returns true if the offline log database is successfully opened.
     **/
    bool isDatabaseOpen(void) const;

    /**
     * \brief   Opens the offline log database file.
     * \param   logPath     The path to the log database file (.sqlog).
     * \return  Returns true if the database is successfully opened, false otherwise.
     **/
    bool openDatabase(const QString & logPath);
    
    /**
     * \brief   Called to move to the top of the logs.
     * \param   select  If true, the first row of logs will be selected after moving to the bottom.
     **/
    void moveToBottom(bool select);
    
    /**
     * \brief   Called to move to the bottom of the logs.
     * \param   select  If true, the last row of logs will be selected after moving to the top.
     **/
    void moveToTop(bool select);
    
    /**
     * \brief   Called to move to the specified row of the logs.
     * \param   row     The row to move the log viewer table.
     * \param   select  If true, the specified row of logs will be selected after moving to the specified row.
     **/
    void moveToRow(int row, bool select);

/************************************************************************
 * Overrides
 ************************************************************************/
protected:

    /**
      * \brief   Override keyPressEvent to handle search shortcuts.
      * \param   event   The key press event.
      **/
    virtual void keyPressEvent(QKeyEvent* event) override;
    
    /**
     * \brief   Sets up the widgets of the log viewer.
     *          This method initializes the user interface components.
     **/
    virtual void setupWidgets(void);
    
    /**
     * \brief   Called when the MDI child window is closing.
     *          This method can be overridden to handle window closing events.
     * \param   isActive    Indicates whether the window is active or not.
     **/
    virtual void onWindowClosing(bool isActive) override;

//////////////////////////////////////////////////////////////////////////
// Slots.
//////////////////////////////////////////////////////////////////////////
protected:
    
    /**
     * \brief   Slot, triggered when make mouse right click on header.
     **/
    virtual void onHeaderContextMenu(const QPoint& pos);
    
    /**
     * \brief   Slot, triggered when make mouse right click on table view
     **/
    virtual void onTableContextMenu(const QPoint& pos);

    /**
     * \brief   Slot, triggered when the search tool-button is clicked.
     * \param   newSearch   It indicates a new search; otherwise, it continues the previous search.
     **/
    virtual void onSearchClicked(bool newSearch);

    /**
     * \brief   Slot, triggered when mouse button is clicked on the log table.
     * \param   index   The index of the cell that was clicked.
     **/
    virtual void onMouseButtonClicked(const QModelIndex& index);
    
    virtual void onMouseDoubleClicked(const QModelIndex& index);
    
    /**
     * \brief   Slot. which triggered when the selection in the log scopes navigation is changed.
     **/
    virtual void onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);
    
//////////////////////////////////////////////////////////////////////////
// attributes
//////////////////////////////////////////////////////////////////////////
protected:
    
    //!< Returns the pointer to the log table object.
    QTableView* ctrlTable(void);

    //!< Returns the pointer to the header object.
    LogTableHeader* ctrlHeader(void);

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

    //!< Resets the order of the columns in the log viewer.
    void resetColumnOrder(void);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    //!< Clears the resources used by the log viewer.
    void _clearResources(void);

    /**
     * \brief   Populates menu and sets the action handlers.
     **/
    void _populateColumnsMenu(QMenu* menu, int curRow);

    /**
     * \brief   Resets the search result in the log viewer.
     **/
    void _resetSearchResult(void);

//////////////////////////////////////////////////////////////////////////
// Member variables.
//////////////////////////////////////////////////////////////////////////
protected:
    LoggingModelBase*           mLogModel;  //!< The logging model used by the log viewer, which provides the data for the log table.
    LogViewerFilter*            mFilter;    //!< The filter object
    QTableView*                 mLogTable;  //!< The table view widget that displays the logs in the log viewer.
    SearchLineEdit*             mLogSearch; //!< The search line edit control, used for searching logs in the log viewer.
    QWidget*                    mMdiWindow; //!< MDI window widget, used for displaying the log viewer in a multi-document interface.
    LogTableHeader*             mHeader;    //!< Log table header object, used for managing the header of the log table.
    LogSearchModel              mSearch;    //!< The search model, used for searching logs in the log viewer.
    LogSearchModel::sFoundPos   mFoundPos;  //!< The found position of the search in the log viewer.
    LogTextHighlight*           mHighlight; //!< The text highlight object, used for highlighting the search results in the log viewer.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls.
//////////////////////////////////////////////////////////////////////////
private:
    DECLARE_NOCOPY_NOMOVE(LogViewerBase);
};

//////////////////////////////////////////////////////////////////////////
// LogViewerBase inline methods implementation
//////////////////////////////////////////////////////////////////////////

inline LoggingModelBase* LogViewerBase::getLoggingModel(void) const
{
    return mLogModel;
}

inline QTableView* LogViewerBase::getLoggingTable(void) const
{
    return mLogTable;
}

#endif  // LUSAN_VIEW_LOG_LOGVIEWERBASE_HPP
