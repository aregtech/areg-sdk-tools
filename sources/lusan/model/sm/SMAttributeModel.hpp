#ifndef LUSAN_MODEL_SM_SMATTRIBUTEMODEL_HPP
#define LUSAN_MODEL_SM_SMATTRIBUTEMODEL_HPP
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
 *  \file        lusan/model/sm/SMAttributeModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Attributes page model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMAttributeData.hpp"

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
 * \class   SMAttributeModel
 * \brief   The Attributes page model. Reads the live `AttributeList` section through the
 *          facade and routes every edit through an undo command, so the page never mutates
 *          `SMAttributeEntry` directly. `SMAttributeEntry` is stored by value in its section,
 *          so mutators identify an entry by ID and re-resolve it inside the command's
 *          getter/setter rather than capturing a pointer that a sibling insert/remove could
 *          invalidate.
 **/
class SMAttributeModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMAttributeModel(StateMachineModel& facade);

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const QList<SMAttributeEntry>& getAttributes() const;
    int getAttributeCount() const;

    SMAttributeEntry* findAttribute(const QString& name) const;
    SMAttributeEntry* findAttribute(uint32_t id) const;
    int findIndex(uint32_t id) const;

    //!< The data types page model, so the value editor can offer declared types and
    //!< resolve an enumeration's enumerators.
    SMDataTypeModel& getDataTypeModel() const;
    DocModelNotifier& getNotifier() const;

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////
public:
    SMAttributeEntry* createAttribute(const QString& name);

    SMAttributeEntry* insertAttribute(int position, const QString& name);
    
    void deleteAttribute(uint32_t id);
    
    void swapAttributes(uint32_t firstId, uint32_t secondId);

    void renameAttribute(uint32_t id, const QString& newName);
    
    //!< Sets the attribute's declared type by name — see SMDataTypeModel::setFieldType for
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
    const SMAttributeData& attributes() const;
    SMAttributeData& attributes();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;
};

#endif  // LUSAN_MODEL_SM_SMATTRIBUTEMODEL_HPP
