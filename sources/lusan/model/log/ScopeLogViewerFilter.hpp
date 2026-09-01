#ifndef LUSAN_MODEL_LOG_SCOPELOGVIEWERFILTER_HPP
#define LUSAN_MODEL_LOG_SCOPELOGVIEWERFILTER_HPP
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
 *  \file        lusan/model/log/ScopeLogViewerFilter.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope Output Viewer Filter Proxy Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/log/LogViewerFilter.hpp"

#include <QHash>
#include <QSet>
#include <QVector>

/**
 * \brief   The scope logs filter proxy model to filter logging messages by scope ID, session IDs and log priority.
 *          The log messages are displayed in the Log Viewer ot output window for further analyzes.
 **/
class ScopeLogViewerFilter  : public LogViewerFilter
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
private:

    //!< Structure to hold data for filtering
    template <typename T>
    struct sTData
    {
        T           data    { 0u };     //!< The data to filter
        bool        isSet   { false };  //!< True if the data is set, false otherwise

        inline sTData() : data(0u), isSet(false) {}
        inline sTData(const T& value) : data(value), isSet(true) {}
        inline operator const T& ( ) const  { return data; }
        inline operator bool ( ) const      { return isSet; }
        inline void     clear()         { data = 0u; isSet = false; }
        inline bool     valid() const       { return isSet; }
        inline const T& value() const   { return data; }
    };

    using   SessionData = sTData<uint32_t>;
    using   ScopeData   = sTData<uint32_t>;
    using   ThreadData  = sTData<ITEM_ID>;
    using   InstanceData= sTData<ITEM_ID>;
    using   PriorityData= sTData<uint32_t>;

public:

    /**
     * \brief   Filtering data type.
     **/
    enum eDataFilter
    {
          NoFilter          = -1    //!< No filter should apply
        , FilterSession     = 0     //!< Filter logs by session, default filter
        , FilterSublogs     = 1     //!< Filter session logs and sublogs of the thread
        , FilterScope       = 2     //!< Filter logs by scope
        , FilterThread      = 3     //!< Filter logs by thread
        , FilterProcess     = 4     //!< Filter logs by process
    };

    /**
     * \brief   Where a row sits in the calls of its own thread. The nesting is read from the
     *          scope enter and exit entries of one thread of one process, so rows of other
     *          threads standing between an enter and its exit do not disturb it.
     **/
    struct sCallRow
    {
        int32_t     depth   { 0 };      //!< How many calls of the thread enclose the row
        int32_t     opener  { -1 };     //!< The row of the scope enter that encloses this row
        int32_t     closer  { -1 };     //!< On a scope enter, the row of its scope exit
        uint32_t    elapsed { 0 };      //!< On a scope enter, how long the call took, in microseconds
        bool        problem { false };  //!< On a scope enter, true when the call carries a warning or worse
    };

    //!< What the gutter draws beside a row.
    enum eCallBracket : int
    {
          BracketNone   = 0 //!< The row is not inside a call
        , BracketOpen       //!< The row opens a call
        , BracketInside     //!< The row sits inside a call
        , BracketClose      //!< The row closes a call
    };

    //!< How a call is drawn: it cannot be folded, it is open, or it is folded.
    enum eCallFold : int
    {
          FoldNone      = 0 //!< The row does not open a closed call
        , FoldOpen          //!< The call is open
        , FoldClosed        //!< The call is folded and its rows are out of the view
    };

    //!< The role that carries how deep a row sits in the calls of its thread.
    static constexpr int    RoleCallDepth   { Qt::ItemDataRole::UserRole + 20 };

    //!< The role that carries whether the row opens a call, and whether that call is folded.
    static constexpr int    RoleCallFold    { Qt::ItemDataRole::UserRole + 21 };

    //!< The role that carries what the gutter draws beside the row.
    static constexpr int    RoleCallBracket { Qt::ItemDataRole::UserRole + 22 };

    //!< The role that carries how long the call of a scope enter row took, in microseconds.
    static constexpr int    RoleCallElapsed { Qt::ItemDataRole::UserRole + 23 };

//////////////////////////////////////////////////////////////////////////
// Constructor / destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Initializes filter
     * \param   scopeId     The ID of the scope to filter, pass 0 if there is no scope ID is set.
     * \param   model       The pointer to the data model.
     **/
    ScopeLogViewerFilter(uint32_t scopeId = 0u, LoggingModelBase* model = nullptr);

    virtual ~ScopeLogViewerFilter();

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public: 

    /**
     * \brief   Sets the scope filter data for the model.
     * \param   model       The pointer to the logging source model to filter.
     * \param   scopeId     The ID of the scope to filter, pass 0 if there is no scope ID is set.
     * \param   sessionId   The session ID to filter.
     * \param   threadId    The thread ID to filter.
     * \param   instanceId  The instance ID to filter.
     **/
    void setScopeFilter(LoggingModelBase * model, uint32_t scopeId, uint32_t sessionId, ITEM_ID threadId, ITEM_ID instanceId);

    /**
     * \brief   Sets the scope filter data for the model.
     * \param   model       The pointer to the logging source model to filter.
     * \param   index       The index in the source model to filter.
     **/
    void setScopeFilter(LoggingModelBase *model, const QModelIndex& index);

    /**
     * \brief   Keeps exactly the given rows of the log, in the order the log holds them.
     *          The scope, session, thread and process filters are dropped.
     * \param   model       The log to read.
     * \param   sourceRows  The rows of the source model to keep.
     * \note    The rows are named in the coordinates of the source model.
     **/
    void setRowFilter(LoggingModelBase* model, const QList<int>& sourceRows);

    //!< Returns true when the view holds the rows the reader picked, not a scope call.
    inline bool isRowFilter(void) const;

    /**
     * \brief   Takes the given rows out of the view. Rows already out of it stay out.
     * \param   sourceRows  The rows of the source model to hide.
     **/
    void hideRows(const QList<int>& sourceRows);

    //!< Brings back every row that was taken out of the view.
    void showHiddenRows(void);

    //!< Returns true when at least one row is taken out of the view.
    inline bool hasHiddenRows(void) const;

    /**
     * \brief   Sets or resets the filters by data.
     * \param   dataFilter  The data to filter.
     **/
    void filterData(ScopeLogViewerFilter::eDataFilter dataFilter);

    /**
     * \brief   Folds or unfolds the call the given row opens.
     * \param   index   The row of a scope enter, in the coordinates of this filter.
     * \return  True if the row opens a call and its state changed.
     **/
    bool toggleFold(const QModelIndex& index);

    /**
     * \brief   Folds every call that carries nothing above information, and unfolds the rest.
     * \param   enable  True to fold the quiet calls, false to open every call again.
     **/
    void setAutoFold(bool enable);

    //!< Returns true if the quiet calls are folded.
    inline bool isAutoFold(void) const;

    /**
     * \brief   Keeps only the rows worth reading: the entries of warning priority or worse, and
     *          the calls that carry one or that ran longer than the given time.
     * \param   enable  True to narrow the view to those rows.
     * \param   slowUs  How long a call must run to count as slow, in microseconds. Zero drops
     *                  the duration from the question and leaves only the priorities.
     **/
    void setInterestingOnly(bool enable, uint32_t slowUs);

    //!< Returns true if the view is narrowed to the rows worth reading.
    inline bool isInterestingOnly(void) const;

    //!< Returns how long a call must run to count as slow, in microseconds.
    inline uint32_t slowCallUs(void) const;

    /**
     * \brief   Switches the timestamp column between the time of day and the time since the
     *          call the row belongs to was entered.
     * \param   enable  True to show the time since the call was entered.
     **/
    void setRelativeTime(bool enable);

    //!< Returns true if the timestamp column counts from the entry of the call.
    inline bool isRelativeTime(void) const;

    /**
     * \brief   Returns the starting index of the logs of the selected session.
     * \param   asSource    If true, the returned index is based on the source model.
     *                      Otherwise, based on filter index.
     **/
    inline QModelIndex getIndexStart(bool asSource) const;

    /**
     * \brief   Returns the last index of the logs of the selected session.
     * \param   asSource    If true, the returned index is based on the source model.
     *                      Otherwise, based on filter index.
     **/
    inline QModelIndex getIndexEnd(bool asSource) const;

    /**
     * \brief   Returns the next index of the scope log in the output window.
     *          If the scope message is not available, it tracks the scope ID change
     *          and returns the index of the next log message in output window.
     * \param   startAt The index to start searching next message.
     *                  If the index is invalid and the output window has entries,
     *                  it returns the index of the first entry.
     *                  If the index is the last scope message entry, the returned value is invalid.
     * \param   asSource    If true, the returned index is based on the source model.
     *                      Otherwise, based on filter index.
     * \return  Returns the index of the next scope message in output window.
     **/
    QModelIndex getIndexNextScope(const QModelIndex& startAt, bool asSource) const;

    /**
     * \brief   Returns the previous index of the scope log in the output window.
     *          If the scope message is not available, it tracks the scope ID change
     *          and returns the index of the previous log message in output window
     * \param   startAt The index to start searching previous message.
     *                  If the index is invalid and the output window has entries,
     *                  it returns the index of the last entry.
     *                  If the index is the first scope message entry, the returned value is invalid.
     * \param   asSource    If true, the returned index is based on the source model.
     *                      Otherwise, based on filter index.
     * \return  Returns the index of the previous scope message in output window.
     **/
    QModelIndex getIndexPrevScope(const QModelIndex& startAt, bool asSource) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Sets the source model to filter.
     *          The method clears all filters and sets the source model.
     * \param   sourceModel The pointer to the source model to filter.
     **/
    void setSourceModel(QAbstractItemModel *sourceModel) override;

    /**
     * \brief   Clears all filters.
     **/
    void clearFilters() override;

    /**
     * \brief   Returns true if the given source row has exact match of the filters.
     *          The method returns false if source model is not set or there are no filters.
     *          The method returns true if filters passed and at least one hat exact match.
     * \param   row      The row index in the source model.
     * \param   parent   The parent index in the source model.
     * \return  True if the row has exact match of the filter.
     **/
    bool filterExactMatch(const QModelIndex & index) const override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   Signal emitted when the filter selects the range of filtered session log.
     * \param   indexStart  The index of the start of the filter range.
     * \param   indexEnd    The index of the end of the filter range.
     **/
    void signalFilterSelected(const QModelIndex& indexStart, const QModelIndex& indexEnd);

protected:
    /**
     * \brief   Returns true if the given source row should be included in the model.
     * \param   row      The row index in the source model.
     * \param   parent   The parent index in the source model.
     * \return  True if the row should be included, false otherwise.
     **/
    bool filterAcceptsRow(int row, const QModelIndex& parent) const override;

    /**
     * \brief   Adds the call structure of a row to what the base filter reports.
     * \param   index   The row to read.
     * \param   role    The role to read.
     * \return  The data of the role.
     **/
    QVariant data(const QModelIndex& index, int role) const override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Checks if the given index matches the scope filter.
     * \param   index   The index in the source model to check.
     * \return  Returns the match type of the index against the scope filter.
     **/
    NELusanCommon::eMatchType matchesScopeFilter(const QModelIndex& index) const;

    /**
     * \brief   Clears all filter data.
     *          The method resets scope ID, session IDs, instance IDs, and priority bits.
     **/
    inline void _clearData();

    /**
     * \brief   Reads the calls of the source rows that are not walked yet. The walk is picked
     *          up where it stopped, so a stream of arriving rows costs one step each.
     **/
    void _readCalls(void);

    //!< Drops the call structure so the next read starts from the first source row.
    void _resetCalls(void);

    //!< Returns true if a call that encloses the given source row is folded.
    bool _isHiddenByFold(int srcRow) const;

    //!< Returns true if the given source row is worth reading while the view is narrowed.
    bool _isInteresting(int srcRow) const;

    //!< Returns the call structure of the given source row, an empty record when there is none.
    inline const ScopeLogViewerFilter::sCallRow& _callOf(int srcRow) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    ScopeData       mSelScopeData;      //<! The selected scope data to filter
    ScopeData       mScopeData;         //<! The scope data to filter
    SessionData     mSelSessionData;    //<! The selected session data to filter
    SessionData     mSessionData;       //<! The session data to filter
    ThreadData      mSelThreadData;     //<! The selected thread data to filter
    ThreadData      mThreadData;        //<! The thread data to filter
    InstanceData    mSelInstanceData;   //!< The selected instance data to filter
    InstanceData    mInstanceData;      //!< The instance data to filter
    PriorityData    mSelPriorityData;   //!< The selected priority data to filter
    PriorityData    mPriorityData;      //!< The priority data to filter
    eDataFilter     mActiveFilter;      //!< Active filter type
    mutable QModelIndex mIndexStart;    //!< The first selected index of filtered data, index is based on the source model
    mutable QModelIndex mIndexEnd;      //<! The last selected index of filtered data, index is based on the source model
    QVector<sCallRow>   mCalls;         //!< Where every source row sits in the calls of its thread
    QHash<quint64, QVector<int32_t>> mStacks;   //!< The calls still open on each thread of each process
    QSet<int32_t>       mFolded;        //!< The scope enter rows whose calls are out of the view
    QSet<int32_t>       mPicked;        //!< The source rows the reader picked, when the view holds a picked set
    QSet<int32_t>       mHidden;        //!< The source rows the reader took out of the view
    bool                mRowsPicked;    //!< True while the view holds the picked rows instead of a scope call
    int                 mCallsRead;     //!< How many source rows the call walk has already read
    bool                mAutoFold;      //!< True while the calls that carry nothing above information are folded
    bool                mInteresting;   //!< True while only the rows worth reading are shown
    uint32_t            mSlowUs;        //!< How long a call must run to count as slow, in microseconds
    bool                mRelativeTime;  //!< True while the timestamp column counts from the entry of the call

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    AREG_NOCOPY_NOMOVE(ScopeLogViewerFilter);
};

//////////////////////////////////////////////////////////////////////////
// ScopeLogViewerFilter inline methods
//////////////////////////////////////////////////////////////////////////

inline bool ScopeLogViewerFilter::isAutoFold(void) const
{
    return mAutoFold;
}

inline bool ScopeLogViewerFilter::isInterestingOnly(void) const
{
    return mInteresting;
}

inline uint32_t ScopeLogViewerFilter::slowCallUs(void) const
{
    return mSlowUs;
}

inline bool ScopeLogViewerFilter::isRelativeTime(void) const
{
    return mRelativeTime;
}

inline bool ScopeLogViewerFilter::isRowFilter(void) const
{
    return mRowsPicked;
}

inline bool ScopeLogViewerFilter::hasHiddenRows(void) const
{
    return (mHidden.isEmpty() == false);
}

inline const ScopeLogViewerFilter::sCallRow& ScopeLogViewerFilter::_callOf(int srcRow) const
{
    static const ScopeLogViewerFilter::sCallRow _empty{ };
    return ((srcRow >= 0) && (srcRow < mCalls.size())) ? mCalls.at(srcRow) : _empty;
}

inline QModelIndex ScopeLogViewerFilter::getIndexStart(bool asSource) const
{
    return (asSource || (mIndexStart.isValid() == false) ? mIndexStart : mapFromSource(mIndexStart));
}

inline QModelIndex ScopeLogViewerFilter::getIndexEnd(bool asSource) const
{
    return (asSource || (mIndexEnd.isValid() == false) ? mIndexEnd : mapFromSource(mIndexEnd));
}

#endif  // LUSAN_MODEL_LOG_SCOPELOGVIEWERFILTER_HPP
