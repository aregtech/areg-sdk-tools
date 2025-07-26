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

class ScopeLogViewerFilter  : public LogViewerFilter
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    using   Sessions    = std::vector<uint32_t>;
    using   Instances   = std::vector<ITEM_ID>;
    using   Priorities  = uint32_t;

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
     * \param   sessionIds  The list of session IDs to filter.
     * \param   instances   The list of instance IDs to filter.
     * \param   priorities  The priority bits to filter log messages.
     **/
    void setScopeFilter(LoggingModelBase * model, uint32_t scopeId, const std::vector<uint32_t>& sessionIds, const std::vector<ITEM_ID>& instances, uint32_t priorities);

    /**
     * \brief   Sets the scope filter data for the model.
     * \param   model       The pointer to the logging source model to filter.
     * \param   index       The index in the source model to filter.
     **/
    void setScopeFilter(LoggingModelBase *model, const QModelIndex& index);

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
    LogViewerFilter::eMatchType matchesScopeFilter(const QModelIndex& index) const;

    /**
     * \brief   Clears all filter data.
     *          The method resets scope ID, session IDs, instance IDs, and priority bits.
     **/
    inline void _clearData(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    uint32_t    mScopeId;       //!< The scope ID for this filter
    Priorities  mPriorities;    //!< Priority bits to filter log messages
    Sessions    mSessionIds;    //!< List of session IDs to filter
    Instances   mInstanceIds;   //!< List of instance IDs to filter

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DECLARE_NOCOPY_NOMOVE(ScopeLogViewerFilter);
};

#endif  // LUSAN_MODEL_LOG_SCOPELOGVIEWERFILTER_HPP
