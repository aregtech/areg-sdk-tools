#ifndef LUSAN_MODEL_SM_SMOVERVIEWMODEL_HPP
#define LUSAN_MODEL_SM_SMOVERVIEWMODEL_HPP
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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/sm/SMOverviewModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview page model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/OverviewModel.hpp"

#include "lusan/data/sm/SMOverviewData.hpp"

/**
 * \class   SMOverviewModel
 * \brief   The shared Overview page model plus the threading mode, which only a state machine
 *          declares. The mode is edited through an undo command like every other row.
 **/
class SMOverviewModel : public OverviewModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMOverviewModel(IEDocumentModel& document);

    virtual ~SMOverviewModel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    SMOverviewData::eThreading getThreading(void) const;
    void setThreading(SMOverviewData::eThreading threading);
};

#endif  // LUSAN_MODEL_SM_SMOVERVIEWMODEL_HPP
