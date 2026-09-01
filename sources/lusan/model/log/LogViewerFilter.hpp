#ifndef LUSAN_MODEL_LOG_LOGVIEWERFILTER_HPP
#define LUSAN_MODEL_LOG_LOGVIEWERFILTER_HPP
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
 *  \file        lusan/model/log/LogViewerFilter.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Filter Proxy Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include <QSortFilterProxyModel>

#include "lusan/common/NELusanCommon.hpp"
#include "areg/logging/areg_log.h"
#include <QMap>
#include <QSet>
#include <QString>
#include <QRegularExpression>


class LoggingModelBase;

/**
 * \brief   Filter proxy model for the log viewer to enable filtering of log messages.
 *          This proxy model filters the LiveLogsModel based on user-selected criteria
 *          from the header filters (combo boxes and text filters).
 **/
class LogViewerFilter : public QSortFilterProxyModel
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   One filter the window has on, in the form a chip draws it.
     **/
    struct sActiveFilter
    {
        int                         column  { -1 };     //!< The column it acts on, as a LoggingModelBase::eColumn value.
        bool                        isText  { false };  //!< True when a phrase was typed, false when entries were picked.
        QString                     text    { };        //!< The phrase, or the picked entries joined.
        NELusanCommon::FilterString phrase  { };        //!< The phrase and its match options. Text filters only.
    };

    using ListActiveFilters = QList<LogViewerFilter::sActiveFilter>;

    /**
     * \brief   What a row was picked out by, when the reader isolated one.
     **/
    enum class eIsolation : int
    {
          IsolationNone = 0 //!< Nothing is isolated.
        , IsolationCall     //!< One run of one scope: one process, one scope, one session.
        , IsolationThread   //!< One thread of one process.
        , IsolationProcess  //!< One process.
        , IsolationScope    //!< One scope of one process, every run of it.
    };

    /**
     * \brief   The row the reader isolated, in the fields it is recognised by.
     **/
    struct sIsolation
    {
        LogViewerFilter::eIsolation kind        { LogViewerFilter::eIsolation::IsolationNone };
        ITEM_ID                     cookie      { 0 };  //!< The process the row came from.
        ITEM_ID                     thread      { 0 };  //!< The thread the row came from.
        uint32_t                    scopeId     { 0 };  //!< The scope that wrote the row.
        uint32_t                    sessionId   { 0 };  //!< The run of that scope.
    };

    //!< The role that answers whether the row is only in the table because the search
    //!< asked for it. A delegate reads it to mark the row apart.
    static constexpr int    RevealedRole    { Qt::ItemDataRole::UserRole + 1 };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor with parent object.
     * \param   model   The logging data model object.
     **/
    explicit LogViewerFilter(LoggingModelBase* model = nullptr);

    virtual ~LogViewerFilter();

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
public slots:
    /**
     * \brief   Sets combo box filter for a specific column.
     * \param   logicalColumn   The logical column index to filter.
     * \param   filters         The list of selected items to filter by.
     **/
    void setComboFilter(int logicalColumn, const NELusanCommon::FilterList& filters);

    /**
     * \brief   Sets text filter for a specific column.
     * \param   logicalColumn   The logical column index to filter.
     * \param   text           The text to filter by.
     **/
    void setTextFilter(int logicalColumn, const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard);
    void setTextFilter(int logicalColumn, const NELusanCommon::FilterString& filter);

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns true if a column filter is set, in a combo box or in a text box.
     *          It does not answer for the scopes the tree refuses, which the model holds.
     **/
    bool hasColumnFilters(void) const;

    /**
     * \brief   Returns every column filter that is on, one entry per column. The scopes the
     *          tree refuses are not among them, they belong to the model.
     **/
    LogViewerFilter::ListActiveFilters activeFilters(void) const;

    /**
     * \brief   Returns true if any row is in the table only because the search asked for it.
     **/
    inline bool hasRevealedRows(void) const;

    /**
     * \brief   Returns true if any filter this window owns is on: a column filter, an
     *          isolated row or a priority the bar narrowed to.
     **/
    bool hasWindowFilters(void) const;

    //!< Returns the row the reader isolated, if any.
    inline const LogViewerFilter::sIsolation& isolation(void) const;

    //!< Returns true if the table is narrowed to one process, thread, scope or scope call.
    inline bool hasIsolation(void) const;

    //!< Returns the priorities the table draws, or zero when it draws every priority.
    inline uint16_t viewPriority(void) const;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Lets one row of the source model through the filters that hide it, so a
     *          search hit is never reported on a row the table does not draw.
     * \param   sourceRow   The row of the source model to let through.
     **/
    void revealRow(int sourceRow);

    /**
     * \brief   Takes back every row the search let through.
     **/
    void clearRevealedRows(void);

    /**
     * \brief   Keeps only the rows that belong to the given process, thread, scope or
     *          scope call.
     * \param   isolation   What to keep. A kind of IsolationNone lets every row through.
     **/
    void setIsolation(const LogViewerFilter::sIsolation& isolation);

    /**
     * \brief   Lets the rows of every process, thread and scope back in.
     **/
    void clearIsolation(void);

    /**
     * \brief   Keeps only the rows whose priority is in the given set.
     * \param   mask    The priorities to draw, as a bit mask of areg::LogPriority values.
     *                  Zero lets every priority through.
     **/
    void setViewPriority(uint16_t mask);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Re-applies the row filter. Call it right after changing the filter
     *          parameters so that filterAcceptsRow() runs again on the mapped rows.
     *          Qt 6.10 replaced invalidateFilter() with endFilterChange().
     **/
    inline void invalidateRowFilter(void)
    {
        // The mapping is dropped and built again in one pass. Asking the proxy to work out
        // the difference instead walks the kept rows run by run, and a log filter keeps rows
        // that are spread over the whole table, which makes that path quadratic.
        invalidate();
    }

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Clears all filters.
     **/
    virtual void clearFilters();

    /**
     * \brief   Returns true if the given source row has exact match of the filters.
     *          The method returns false if source model is not set or there are no filters.
     *          The method returns true if filters passed and at least one hat exact match.
     * \param   row      The row index in the source model.
     * \param   parent   The parent index in the source model.
     * \return  True if the row has exact match of the filter.
     **/
    virtual bool filterExactMatch(const QModelIndex& index) const;

    /**
     * \brief   Returns true if the given source row should be included in the model.
     * \param   row      The row index in the source model.
     * \param   parent   The parent index in the source model.
     * \return  True if the row should be included, false otherwise.
     **/
    bool filterAcceptsRow(int row, const QModelIndex& parent) const override;

    /**
     * \brief   Answers RevealedRole from the set of rows the search let through, and hands
     *          every other role to the source model.
     * \param   index   The index in this proxy.
     * \param   role    The role asked for.
     **/
    QVariant data(const QModelIndex& index, int role) const override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Helper method to check if a row matches the combo filters.
     * \param   model  The logging model to use for filtering.
     * \param   msg    The log message to check against the filters.
     * \return  True if the row matches all combo filters.
     **/
    NELusanCommon::eMatchType matchesComboFilters(LoggingModelBase* model, const areg::LogEntry* msg) const;

    /**
     * \brief   Helper method to check if a row matches the text filters.
     * \param   model  The logging model to use for filtering.
     * \param   msg    The log message to check against the filters.
     * \return  True if the row matches all text filters.
     **/
    NELusanCommon::eMatchType matchesTextFilters(LoggingModelBase* model, const areg::LogEntry* msg) const;

    /**
     * \brief   Helper method to perform wildcard matching.
     * \param   text            The text to match against the wildcard pattern.
     * \param   wildcardPattern The wildcard pattern to match against.
     * \param   isCaseSensitive Flag indicating if the match is case-sensitive.
     * \param   isWholeWord     Flag indicating if the match is for whole words only.
     * \return  True if the text matches the wildcard pattern, false otherwise.
     **/
    bool wildcardMatch(const QString& text, const QString& wildcardPattern, bool isCaseSensitive, bool isWholeWord) const;

    /**
     * \brief   Checks if the log message matches the priority filters.
     * \param   msg     The log message to check.
     * \param   filters The list of priority filters to match against.
     * \return  True if the log message matches any of the priority filters, false otherwise.
     **/
    inline bool matchPrio(const areg::LogEntry* msg, const NELusanCommon::FilterList& filters) const;

    /**
     * \brief   Checks if the log message matches the source filters.
     * \param   msg     The log message to check.
     * \param   filters The list of source filters to match against.
     * \return  True if the log message matches any of the source filters, false otherwise.
     **/
    inline bool matchSources(const areg::LogEntry* msg, const NELusanCommon::FilterList& filters) const;

    /**
     * \brief   Checks if the log message matches the thread filters.
     * \param   msg     The log message to check.
     * \param   filters The list of thread filters to match against.
     * \return  True if the log message matches any of the thread filters, false otherwise.
     **/
    inline bool matchThreads(const areg::LogEntry* msg, const NELusanCommon::FilterList& filters) const;

    /**
     * \brief   Checks if the log message matches the duration filters.
     * \param   msg     The log message to check.
     * \param   filters The list of duration filters to match against.
     * \return  True if the log message matches any of the duration filters, false otherwise.
     **/
    inline bool matchDuration(const areg::LogEntry* msg, const NELusanCommon::FilterList& filters) const;

    /**
     * \brief   Checks if the log message matches the message text filters.
     * \param   msg     The log message to check.
     * \param   filters The list of message text filters to match against.
     * \return  True if the log message matches any of the message text filters, false otherwise.
     **/
    inline bool matchMessage(const areg::LogEntry* msg, const NELusanCommon::FilterList& filters) const;

    /**
     * \brief   Checks if the log message belongs to the isolated process, thread, scope or
     *          scope call. Every message passes while nothing is isolated.
     * \param   msg     The log message to check.
     * \return  True if the message belongs to what the reader isolated.
     **/
    inline bool matchIsolation(const areg::LogEntry* msg) const;

    /**
     * \brief   Prepares the regular expression for wildcard matching.
     * \param   wildcardPattern The wildcard pattern to convert to a regular expression.
     * \param   isCaseSensitive Flag indicating if the match is case-sensitive.
     * \param   isWholeWord     Flag indicating if the match is for whole words only.
     * \param   isWildCard      Flag indicating if the pattern is a wildcard.
     **/
    inline void prepareReExpression(const QString& wildcardPattern, bool isCaseSensitive, bool isWholeWord, bool isWildCard);

    //!< Clear filter data/
    inline void _clearData();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    QMap<int, NELusanCommon::FilterList>    mComboFilters;  //!< Map of column index to selected filter items
    QMap<int, NELusanCommon::FilterList>    mTextFilters;   //!< Map of column index to filter text
    QString                                 mRePattern;     //!< Regular expression pattern for wildcard matching
    QRegularExpression                      mReExpression;  //!< Regular expression for wildcard matching
    QSet<int>                               mRevealed;      //!< Source rows the search let through the filters
    LogViewerFilter::sIsolation             mIsolation;     //!< The process, thread, scope or scope call the table is narrowed to
    uint16_t                                mViewPriority;  //!< The priorities the table draws, zero for every one of them

//////////////////////////////////////////////////////////////////////////
// Forbidden call
//////////////////////////////////////////////////////////////////////////
private:
    LogViewerFilter() = delete;
    AREG_NOCOPY_NOMOVE(LogViewerFilter);
};

//////////////////////////////////////////////////////////////////////////
// LogViewerFilter class inline methods
//////////////////////////////////////////////////////////////////////////

inline bool LogViewerFilter::hasRevealedRows(void) const
{
    return (mRevealed.isEmpty() == false);
}

inline const LogViewerFilter::sIsolation& LogViewerFilter::isolation(void) const
{
    return mIsolation;
}

inline bool LogViewerFilter::hasIsolation(void) const
{
    return (mIsolation.kind != LogViewerFilter::eIsolation::IsolationNone);
}

inline uint16_t LogViewerFilter::viewPriority(void) const
{
    return mViewPriority;
}

#endif // LUSAN_MODEL_LOG_LOGVIEWERFILTER_HPP
