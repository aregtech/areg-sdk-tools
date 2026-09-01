#ifndef LUSAN_VIEW_LOG_LOGVIEWERBASE_HPP
#define LUSAN_VIEW_LOG_LOGVIEWERBASE_HPP
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
 *  \file        lusan/view/log/LogViewerBase.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log viewer base widget.
 *
 ************************************************************************/
#include "lusan/view/common/MdiChild.hpp"
#include "lusan/model/log/LogClockSkew.hpp"
#include "lusan/view/log/LogEmptyState.hpp"
#include "lusan/view/log/LogFilterChips.hpp"
#include "lusan/model/log/LogSearchModel.hpp"
#include "lusan/view/log/LogTextHighlight.hpp"
#include "lusan/data/common/WorkspaceEntry.hpp"
#include "areg/base/areg_global.h"

/************************************************************************
 * Dependencies
 ************************************************************************/
class LoggingModelBase;
class LogEmptyState;
class LogHitMap;
class NaviLogScopeBase;
class LogSessionBar;
class LogTableHeader;
class LogViewerFilter;
class SearchLineEdit;


class QHeaderView;
class QModelIndex;
class QPoint;
class QString;
class QTableView;
class QTimer;
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
    //!< The rows keep arriving faster than a counter can be read. The changes are
    //!< collected for this many milliseconds and drawn once.
    static constexpr int    COUNTER_DELAY_MS    { 200 };

    //!< The most matches the hit list draws before it says how many are left.
    static constexpr int    HitListMax          { 300 };

    //!< The longest a message stays in a hit list entry.
    static constexpr int    HitListChars        { 110 };

    //!< Set on every log table, so a change of the row height finds them all again.
    static constexpr const char* PropertyLogTable { "lusanLogTable" };

    /**
     * \brief   Returns the file extension of the offline log files.
     **/
    static const QString& fileExtension();

    /**
     * \brief   Gives the rows of the given log table the height chosen in the options and
     *          marks the table, so a later change of the setting finds it again.
     * \param   table   The table to apply the height to.
     **/
    static void applyRowHeight(QTableView* table);

    /**
     * \brief   Gives every marked log table the height chosen in the options. Called when
     *          the setting changes.
     **/
    static void refreshRowHeights(void);

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

    virtual ~LogViewerBase();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the logging model used by the log viewer.
     **/
    inline LoggingModelBase* getLoggingModel() const;

    /**
     * \brief   Returns the logging table object.
     **/
    inline QTableView* getLoggingTable() const;

    /**
     * \brief   Returns the session bar of this log window.
     **/
    inline LogSessionBar* getSessionBar() const;

    /**
     * \brief   Returns true if the offline log database is successfully opened.
     **/
    bool isDatabaseOpen() const;

    /**
     * \brief   Returns true if the window has a source that can produce rows: an open
     *          archive, or a connected log collector.
     **/
    virtual bool isSourceReady() const;

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

    /**
     * \brief   Selects the entry with the sources index in the log viewer.
     * \param   index   The entry to select. The index should be valid and must be mapped to the source model.
     */
    void selectSourceElement(const QModelIndex & index);

/************************************************************************
 * Overrides
 ************************************************************************/
protected:

    /**
      * \brief   Override keyPressEvent to handle search shortcuts.
      * \param   event   The key press event.
      **/
    void keyPressEvent(QKeyEvent* event) override;
    
    /**
     * \brief   Sets up the widgets of the log viewer.
     *          This method initializes the user interface components.
     **/
    virtual void setupWidgets();
    
    /**
     * \brief   Called when the MDI child window is closing.
     *          This method can be overridden to handle window closing events.
     * \param   isActive    Indicates whether the window is active or not.
     **/
    void onWindowClosing(bool isActive) override;

    /**
     * \brief   Returns the default file filter.
     **/
    const QString& fileFilter() const override;

    /**
     * \brief   Reads the document from the file.
     * \param   filePath    The path of the file to read.
     * \return  True if the document was successfully read, false otherwise.
     **/
    bool writeToFile(const QString& filePath) override;

    /**
     * \brief   Saves the file with the specified name.
     * \param   fileName    The name of the file to save.
     * \return  True if the file was successfully saved, false otherwise.
     **/
    bool saveFile(const QString& fileName) override;

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
     * \brief   Copies the message text of the selected rows to the clipboard, one row per line.
     **/
    void onCopyMessage();

    /**
     * \brief   Copies the selected rows to the clipboard in the layout that the target writes
     *          into its own log file, one row per line.
     **/
    void onCopyRow();

    /**
     * \brief   Slot, triggered when mouse button is clicked on the log table.
     * \param   index   The index of the cell that was clicked.
     **/
    virtual void onMouseButtonClicked(const QModelIndex& index);

    /**
     * \brief   Slot, triggered when mouse button is double clicked on the log table.
     * \param   index   The index of the cell that was double clicked.
     **/
    virtual void onMouseDoubleClicked(const QModelIndex& index);
    
    /**
     * \brief   Slot. which triggered when the selection in the log scopes navigation is changed.
     **/
    virtual void onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

    /**
     * \brief   Keeps the empty-state panel over the table viewport.
     * \param   watched The object the event came from.
     * \param   event   The event.
     **/
    bool eventFilter(QObject* watched, QEvent* event) override;

    /**
     * \brief   Slot, triggered when rows are appended to the logging model. It measures the
     *          clocks of the sources and raises the notice when two of them disagree.
     * \param   parent  The parent index of the inserted rows.
     * \param   first   The first inserted row.
     * \param   last    The last inserted row.
     **/
    void onSourceRowsInserted(const QModelIndex& parent, int first, int last);
    
//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
protected:
    //!< Resets the order of the columns in the log viewer.
    void resetColumnOrder();

    /**
     * \brief   Resets filters.
     **/
    void resetFilters();

    /**
     * \brief   Drops every filter that keeps a row out of this window: the column filters and
     *          the scopes the navigation tree refuses.
     **/
    void clearEveryFilter();

    /**
     * \brief   Sets the message column filter from the given phrase and match options, so
     *          the table keeps only the rows that carry it.
     * \param   phrase  The phrase and the options to match it with. An empty phrase drops
     *                  the filter.
     **/
    void filterToPhrase(const NELusanCommon::FilterString& phrase);

    /**
     * \brief   Moves the table to its last row without turning the follow toggle off.
     *          Every scroll the application makes itself goes through this method; a scroll
     *          the user makes is what switches following off.
     **/
    void scrollFollowing();

//////////////////////////////////////////////////////////////////////////
// attributes
//////////////////////////////////////////////////////////////////////////
protected:
    
    //!< Returns the pointer to the log table object.
    QTableView* ctrlTable();

    //!< Returns the pointer to the header object.
    LogTableHeader* ctrlHeader();

    //!< Returns the pointer to the search line edit control.
    SearchLineEdit* ctrlSearchText();

    //!< Returns the pointer to the search case sensitive button of the search line edit control.
    QToolButton* ctrlButtonCaseSensitive();

    //!< Returns the pointer to the search match word button of the search line edit control.
    QToolButton* ctrlButtonWholeWords();

    //!< Returns the pointer to the search wild card button of the search line edit control.
    QToolButton* ctrlSearchWildcard();

    //!< Returns the pointer to the search backward button of the search line edit control.
    QToolButton* ctrlSearchBackward();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    //!< Clears the resources used by the log viewer.
    inline void _clearResources();

    /**
     * \brief   Populates menu and sets the action handlers.
     **/
    void _populateColumnsMenu(QMenu* menu, int curRow);

    /**
     * \brief   Returns the rows currently selected in the log table, in view order.
     *          Falls back to the current row when there is no selection.
     **/
    QList<int> _selectedRows() const;

    /**
     * \brief   Builds the clipboard text of the given rows.
     * \param   rows        The rows to format.
     * \param   fullLayout  If true, uses the target's own log file layout; otherwise
     *                      returns the message text alone.
     **/
    QString _rowsToText(const QList<int>& rows, bool fullLayout) const;

    /**
     * \brief   Updates the current logical index of the "Message" column.
     **/
    void _updateHighlightColumn();

    /**
     * \brief   Widens every selected range back to the full row. Inserting or removing
     *          a column cuts the selected rows at that column, which would leave the
     *          moved cells out of a selection the user made by row.
     **/
    void _refitRowSelection();

    /**
     * \brief   Listens to the table's selection, so that a row the user picks releases the
     *          follow toggle. Setting a model on a view builds a new selection model, so
     *          this runs again after every such call.
     **/
    void _bindSelection();

    /**
     * \brief   Draws the row counters of the session bar.
     **/
    void _updateCounters();

    /**
     * \brief   Draws, or hides, the panel that says why the table has no row.
     **/
    void _updateEmptyState();

    /**
     * \brief   Draws one chip for every filter the window has on.
     **/
    void _updateChips();

    /**
     * \brief   Returns the name the reader sees for the given column. The column does not
     *          have to be one the table shows.
     * \param   column  The column, as a LoggingModelBase::eColumn value.
     **/
    static QString _columnName(int column);

    /**
     * \brief   Switches off the filter the given chip stands for.
     * \param   chip    The chip the reader dropped.
     **/
    void _dropChip(const LogFilterChips::sChip& chip);

    /**
     * \brief   Brings the hit the search just found into the table: reveals it when a filter
     *          hides it, moves to it and draws what the counter and the notice say about it.
     * \param   allLogs True when the search walks every row the window holds.
     **/
    void _showSearchHit(bool allLogs);

    /**
     * \brief   Draws the counter inside the search field, and the line that names the filters
     *          a revealed row came back from.
     * \param   allLogs True when the search walks every row the window holds.
     **/
    void _drawSearchState(bool allLogs);

    /**
     * \brief   Opens the rows the phrase matches as a list under the search field. Choosing
     *          one moves the table to it.
     **/
    void _showHitList(void);

    /**
     * \brief   Moves the table to the next row of warning priority or worse.
     * \param   forward True to walk down the table, false to walk up.
     **/
    void _stepToProblem(bool forward);

    /**
     * \brief   Returns the scope panel that belongs to this window, live or offline.
     *          Null when the main window is not known.
     **/
    NaviLogScopeBase* _scopePanel(void) const;

    /**
     * \brief   Writes the columns of the table and the open database into the workspace.
     **/
    void _saveLayout(void) const;

    /**
     * \brief   Applies the columns the workspace remembers. Does nothing when it holds none.
     **/
    void _restoreLayout(void);

    //!< Returns which column record this window reads and writes.
    WorkspaceEntry::eLogMode _columnMode(void) const;

    /**
     * \brief   Drops the remembered columns, so the defaults come back on the next run.
     **/
    void _forgetLayout(void) const;

    /**
     * \brief   Returns the filters that are on, named in one line.
     **/
    QString _filterSummary() const;

    /**
     * \brief   Appends the entries that narrow the table to one process, thread, scope or
     *          scope call, taken from the given row.
     * \param   menu    The menu to fill.
     * \param   row     The row of the table the menu was opened on, or -1.
     **/
    void _populateIsolateMenu(QMenu* menu, int row);

    /**
     * \brief   Returns the given number of microseconds as a phrase, in the largest unit
     *          that keeps the value readable.
     **/
    static QString _formatOffset(qint64 offsetUs);

    /**
     * \brief   Resets the search result in the log viewer.
     **/
    inline void _resetSearchResult();

    /**
     * \brief   Selects the source log entry based on the source index.
     * \param   source  The source index of the log entry to select.
     * \return  Returns true if the selection was successful, false otherwise.
     *          If returns false, the log entry with the specified source index is not visible in the log view window.
     **/
    inline bool _selectSourceLog(const QModelIndex& source);

    /**
     * \brief   Selects the target log entry based on the target index.
     * \param   target  The target index of the log entry to select.
     **/
    inline void _selectTargetLog(const QModelIndex& target);

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
    uint32_t                    mFoundRow;  //!< The hit in the rows of the model the search walks.
    QList<uint32_t>             mHits;      //!< Every row the current phrase matches, ascending.
    int                         mHiddenHits;//!< How many of those rows the filters keep out.
    LogTextHighlight*           mHighlight; //!< The text highlight object, used for highlighting the search results in the log viewer.
    int                         mHighlightColumn; //!< The current logical column index where highlight delegate is installed.
    LogSessionBar*              mSessionBar;//!< The bar above the table, carrying the state, the counters and the controls of the window.
    QString                     mIsolationText; //!< What the chip of the isolated row says.
    QTimer*                     mCountTimer;//!< Collects the row changes so the counters are drawn once instead of once per row.
    LogEmptyState*              mEmptyState;//!< What the table says when it has no row to draw.
    LogHitMap*                  mHitMap;    //!< The marks on the scrollbar naming the hits and the rows above a severity.
    LogClockSkew                mSkew;      //!< Watches the sources for a clock that disagrees with the collector.
    bool                        mSkewShown; //!< True once the clock notice was raised, so it is raised once per session.
    bool                        mFollowScroll; //!< True while the application scrolls the table itself.
    bool                        mFollowSelect; //!< True while the application selects a row itself.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls.
//////////////////////////////////////////////////////////////////////////
private:
    AREG_NOCOPY_NOMOVE(LogViewerBase);
};

//////////////////////////////////////////////////////////////////////////
// LogViewerBase inline methods implementation
//////////////////////////////////////////////////////////////////////////

inline LoggingModelBase* LogViewerBase::getLoggingModel() const
{
    return mLogModel;
}

inline QTableView* LogViewerBase::getLoggingTable() const
{
    return mLogTable;
}

inline LogSessionBar* LogViewerBase::getSessionBar() const
{
    return mSessionBar;
}

#endif  // LUSAN_VIEW_LOG_LOGVIEWERBASE_HPP
