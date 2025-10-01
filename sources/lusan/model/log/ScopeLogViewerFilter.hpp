#ifndef LUSAN_MODEL_LOG_SCOPELOGVIEWERFILTER_HPP
#define LUSAN_MODEL_LOG_SCOPELOGVIEWERFILTER_HPP
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
 *  \file        lusan/model/log/ScopeLogViewerFilter.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope Output Viewer Filter Proxy Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/log/LogViewerFilter.hpp"

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

        inline sTData(void) : data(0u), isSet(false) {}
        inline sTData(const T& value) : data(value), isSet(true) {}
        inline operator const T& ( ) const  { return data; }
        inline operator bool ( ) const      { return isSet; }
        inline void     clear(void)         { data = 0u; isSet = false; }
        inline bool     valid() const       { return isSet; }
        inline const T& value(void) const   { return data; }
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

    virtual ~ScopeLogViewerFilter(void);

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
     * \brief   Sets or resets the filters by data.
     * \param   dataFilter  The data to filter.
     **/
    void filterData(ScopeLogViewerFilter::eDataFilter dataFilter);

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
    virtual void setSourceModel(QAbstractItemModel *sourceModel) override;

    /**
     * \brief   Clears all filters.
     **/
    virtual void clearFilters(void) override;

    /**
     * \brief   Returns true if the given source row has exact match of the filters.
     *          The method returns false if source model is not set or there are no filters.
     *          The method returns true if filters passed and at least one hat exact match.
     * \param   row      The row index in the source model.
     * \param   parent   The parent index in the source model.
     * \return  True if the row has exact match of the filter.
     **/
    virtual bool filterExactMatch(const QModelIndex & index) const override;

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
    virtual bool filterAcceptsRow(int row, const QModelIndex& parent) const override;

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
    inline void _clearData(void);

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

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DECLARE_NOCOPY_NOMOVE(ScopeLogViewerFilter);
};

//////////////////////////////////////////////////////////////////////////
// ScopeLogViewerFilter inline methods
//////////////////////////////////////////////////////////////////////////

inline QModelIndex ScopeLogViewerFilter::getIndexStart(bool asSource) const
{
    return (asSource || (mIndexStart.isValid() == false) ? mIndexStart : mapFromSource(mIndexStart));
}

inline QModelIndex ScopeLogViewerFilter::getIndexEnd(bool asSource) const
{
    return (asSource || (mIndexEnd.isValid() == false) ? mIndexEnd : mapFromSource(mIndexEnd));
}

#endif  // LUSAN_MODEL_LOG_SCOPELOGVIEWERFILTER_HPP
