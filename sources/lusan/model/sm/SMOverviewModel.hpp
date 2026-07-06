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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/sm/SMOverviewModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview page model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMOverviewData.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;
class DocModelNotifier;

/**
 * \class   SMOverviewModel
 * \brief   The Overview page model. It reads the live Overview section and routes every
 *          edit through an undo command on the document's stack, so the page never mutates
 *          the model directly. It reaches the section through the facade rather than a
 *          captured reference, so a document reload (which replaces the data root and
 *          clears the stack) needs no rebinding.
 **/
class SMOverviewModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMOverviewModel(StateMachineModel& facade);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    const QString& getName() const;
    const VersionNumber& getVersion() const;
    SMOverviewData::eThreading getThreading() const;
    const QString& getDescription() const;

    void setName(const QString& name);
    void setVersion(const VersionNumber& version);
    void setThreading(SMOverviewData::eThreading threading);
    void setDescription(const QString& description);

    DocModelNotifier& getNotifier() const;
    uint32_t getOverviewId() const;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    const SMOverviewData& overview() const;
    SMOverviewData& overview();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;
};

#endif  // LUSAN_MODEL_SM_SMOVERVIEWMODEL_HPP
