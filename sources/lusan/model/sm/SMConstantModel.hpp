#ifndef LUSAN_MODEL_SM_SMCONSTANTMODEL_HPP
#define LUSAN_MODEL_SM_SMCONSTANTMODEL_HPP
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
 *  \file        lusan/model/sm/SMConstantModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Constants page model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMConstantData.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;
class SMDataTypeModel;
class DocModelNotifier;

/**
 * \class   SMConstantModel
 * \brief   The Constants page model. Reads the live `ConstantList` section through the
 *          facade and routes every edit through an undo command, so the page never mutates
 *          `ConstantEntry` directly. `ConstantEntry` is stored by value in its section, so
 *          mutators identify an entry by ID and re-resolve it inside the command's
 *          getter/setter rather than capturing a pointer that a sibling insert/remove could
 *          invalidate.
 **/
class SMConstantModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMConstantModel(StateMachineModel& facade);

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const QList<ConstantEntry>& getConstants() const;
    int getConstantCount() const;

    ConstantEntry* findConstant(const QString& name) const;
    ConstantEntry* findConstant(uint32_t id) const;
    int findIndex(uint32_t id) const;

    //!< The data types page model, so the value editor can offer declared types and
    //!< resolve an enumeration's enumerators.
    SMDataTypeModel& getDataTypeModel() const;
    DocModelNotifier& getNotifier() const;

    //!< The document facade (the page's guard where-used check on delete).
    StateMachineModel& getFacade() const;

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////
public:
    ConstantEntry* createConstant(const QString& name);
    ConstantEntry* insertConstant(int position, const QString& name);
    void deleteConstant(uint32_t id);
    void swapConstants(uint32_t firstId, uint32_t secondId);

    void renameConstant(uint32_t id, const QString& newName);
    //!< Sets the constant's declared type by name — see SMDataTypeModel::setFieldType for
    //!< why name, not the resolved DataTypeBase*.
    void setType(uint32_t id, const QString& typeName);
    void setValue(uint32_t id, const QString& value);
    void setDescription(uint32_t id, const QString& text);
    void setDeprecated(uint32_t id, bool deprecated);
    void setDeprecateHint(uint32_t id, const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    const SMConstantData& constants() const;
    SMConstantData& constants();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;
};

#endif  // LUSAN_MODEL_SM_SMCONSTANTMODEL_HPP
