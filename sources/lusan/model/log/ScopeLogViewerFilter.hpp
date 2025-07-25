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
    void setScopeFilter(LoggingModelBase * model, uint32_t scopeId, const std::vector<uint32_t>& sessionids, const std::vector<ITEM_ID>& instances, uint32_t priorities);
    
    void setScopeFilter(LoggingModelBase *model, const QModelIndex& index);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Clears all filters.
     **/
    virtual void clearFilters(void) override;
    
protected:
    /**
     * \brief   Returns true if the given source row should be included in the model.
     * \param   source_row      The row index in the source model.
     * \param   source_parent   The parent index in the source model.
     * \return  True if the row should be included, false otherwise.
     **/
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    
    bool matchesScopeFilter(int source_row, const QModelIndex& source_parent) const;
    
    inline void _clearData(void);

private:
    uint32_t    mScopeId;       //!< The scope ID for this filter
    Priorities  mPriorities;    //!< Priority bits to filter log messages
    Sessions    mSessionIds;    //!< List of session IDs to filter
    Instances   mInstanceIds;   //!< List of instance IDs to filter
};

#endif  // LUSAN_MODEL_LOG_SCOPELOGVIEWERFILTER_HPP
