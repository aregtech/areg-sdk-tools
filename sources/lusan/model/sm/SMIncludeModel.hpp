#ifndef LUSAN_MODEL_SM_SMINCLUDEMODEL_HPP
#define LUSAN_MODEL_SM_SMINCLUDEMODEL_HPP
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
 *  \file        lusan/model/sm/SMIncludeModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMIncludeData.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;
class DocModelNotifier;

/**
 * \class   SMIncludeModel
 * \brief   The Includes page model. Reads the live `IncludeList` section through the facade
 *          and routes every edit through an undo command, so the page never mutates an
 *          `IncludeEntry` directly. An include is identified by its location, which is also
 *          its unique name; mutators resolve the entry by ID inside the command's
 *          getter/setter rather than capturing a pointer a sibling insert/remove could
 *          invalidate.
 **/
class SMIncludeModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMIncludeModel(StateMachineModel& facade);

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const QList<IncludeEntry>& getIncludes() const;
    int getIncludeCount() const;

    IncludeEntry* findInclude(const QString& location) const;
    IncludeEntry* findInclude(uint32_t id) const;
    int findIndex(uint32_t id) const;

    DocModelNotifier& getNotifier() const;

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////
public:
    IncludeEntry* createInclude(const QString& location);
    IncludeEntry* insertInclude(int position, const QString& location);
    void deleteInclude(uint32_t id);
    void swapIncludes(uint32_t firstId, uint32_t secondId);

    //!< The location is the include's unique name, so this is both the rename and the path edit.
    void setLocation(uint32_t id, const QString& location);
    void setDescription(uint32_t id, const QString& text);
    void setDeprecated(uint32_t id, bool deprecated);
    void setDeprecateHint(uint32_t id, const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    const SMIncludeData& includes() const;
    SMIncludeData& includes();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;
};

#endif  // LUSAN_MODEL_SM_SMINCLUDEMODEL_HPP
