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
    using   Sessions    = std::vector<uint32_t>;
    using   Instances   = std::vector<ITEM_ID>;
    using   Priorities  = uint32_t;

public:
    ScopeLogViewerFilter(uint32_t scopeId, LoggingModelBase* model);

    virtual ~ScopeLogViewerFilter(void) = default;

private:
    uint32_t    mScopeId;      //!< The scope ID for this filter
    Sessions    mSessionIds;   //!< List of session IDs to filter
    Instances   mInstanceIds;  //!< List of instance IDs to filter
    Priorities  mPriorities;    //!< Priority bits to filter log messages
};

#endif  // LUSAN_MODEL_LOG_SCOPELOGVIEWERFILTER_HPP
