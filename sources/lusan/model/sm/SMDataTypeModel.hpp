#ifndef LUSAN_MODEL_SM_SMDATATYPEMODEL_HPP
#define LUSAN_MODEL_SM_SMDATATYPEMODEL_HPP
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
 *  \file        lusan/model/sm/SMDataTypeModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Data Types page model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DataTypeBase.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;
class DocModelNotifier;
class ElementBase;
class DataTypeCustom;
class DataTypeStructure;
class DataTypeEnum;
class DataTypeImported;
class DataTypeContainer;
class FieldEntry;
class EnumEntry;
class SMDataTypeData;

/**
 * \class   SMDataTypeModel
 * \brief   The Data Types page model. Reads the live `DataTypeList` section through the
 *          facade and routes every edit through an undo command, so the page never mutates
 *          `DataTypeCustom`/`FieldEntry`/`EnumEntry` objects directly. Enumeration, Structure,
 *          Imported and Container categories are supported.
 *
 *          Field-level commands re-resolve the field by owner pointer + ID inside the
 *          command's getter/setter, never by a captured `FieldEntry*`/`EnumEntry*`: those
 *          are stored by value in the owning type's list, so a sibling add/remove can
 *          reallocate and move them. The owning `DataTypeCustom*` itself is stable — it is
 *          stored by pointer in `SMDataTypeData` — so it is safe to capture directly.
 **/
class SMDataTypeModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMDataTypeModel(StateMachineModel& facade);

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const SMDataTypeData& getDataTypeData() const;
    SMDataTypeData& getDataTypeData();

    const QList<DataTypeCustom*>& getCustomDataTypes() const;
    int getDataTypeCount() const;

    DataTypeCustom* findDataType(const QString& name) const;
    DataTypeCustom* findDataType(uint32_t id) const;
    int findIndex(uint32_t id) const;
    int findIndex(const DataTypeCustom* dataType) const;

    const QList<FieldEntry>& getStructChildren(const DataTypeStructure* dataType) const;
    const QList<EnumEntry>& getEnumChildren(const DataTypeEnum* dataType) const;
    ElementBase* findChild(const DataTypeCustom* dataType, uint32_t childId) const;
    int findChildIndex(const DataTypeCustom* dataType, uint32_t childId) const;
    int findChildIndex(const DataTypeCustom* dataType, const QString& childName) const;
    int getChildCount(const DataTypeCustom* dataType) const;

    DocModelNotifier& getNotifier() const;

//////////////////////////////////////////////////////////////////////////
// Mutations — data type level
//////////////////////////////////////////////////////////////////////////
public:
    DataTypeCustom* createDataType(const QString& name, DataTypeBase::eCategory category);
    DataTypeCustom* insertDataType(int position, const QString& name, DataTypeBase::eCategory category);
    void deleteDataType(DataTypeCustom* dataType);
    DataTypeCustom* convertDataType(DataTypeCustom* dataType, DataTypeBase::eCategory category);
    void swapDataTypes(uint32_t firstId, uint32_t secondId);

    void renameDataType(DataTypeCustom* dataType, const QString& newName);
    void setDescription(DataTypeCustom* dataType, const QString& text);
    void setDeprecated(DataTypeCustom* dataType, bool deprecated);
    void setDeprecateHint(DataTypeCustom* dataType, const QString& hint);
    void setEnumDerived(DataTypeEnum* dataType, const QString& derived);
    void setImportLocation(DataTypeImported* dataType, const QString& location);
    void setImportNamespace(DataTypeImported* dataType, const QString& space);
    void setImportObject(DataTypeImported* dataType, const QString& object);

    //!< Sets the basic-container object (Array/LinkedList/HashMap/Map/Pair/custom) by name.
    //!< Re-derives the resulting key (defaulted, kept, or cleared) the same way
    //!< DataTypeContainer::setContainer() does, so undo restores the exact prior key.
    void setContainerObject(DataTypeContainer* dataType, const QString& basicName);
    //!< Sets the key element type by name — see setFieldType for why name, not DataTypeBase*.
    void setContainerKey(DataTypeContainer* dataType, const QString& typeName);
    //!< Sets the value element type by name — see setFieldType for why name, not DataTypeBase*.
    void setContainerValue(DataTypeContainer* dataType, const QString& typeName);

//////////////////////////////////////////////////////////////////////////
// Mutations — field level (structure fields / enumeration entries)
//////////////////////////////////////////////////////////////////////////
public:
    ElementBase* createField(DataTypeCustom* dataType, const QString& name);
    ElementBase* insertField(DataTypeCustom* dataType, int position, const QString& name);
    void deleteField(DataTypeCustom* dataType, uint32_t fieldId);
    void swapFields(DataTypeCustom* dataType, uint32_t firstId, uint32_t secondId);

    void setFieldName(DataTypeCustom* dataType, uint32_t fieldId, const QString& name);

    //!< Sets the field's type by name — the actual persisted state (`Field DataType="..."`).
    //!< Not by DataTypeBase*: ParamType only resolves a pointer on demand and its setType()
    //!< cannot restore an unresolved original name on undo (see fsm-data-pointer-containers /
    //!< that handoff), so the name string is the only value this command may round-trip.
    void setFieldType(DataTypeStructure* dataType, uint32_t fieldId, const QString& typeName);
    void setFieldValue(DataTypeCustom* dataType, uint32_t fieldId, const QString& value);
    void setFieldDescription(DataTypeCustom* dataType, uint32_t fieldId, const QString& text);
    void setFieldDeprecated(DataTypeCustom* dataType, uint32_t fieldId, bool deprecated);
    void setFieldDeprecateHint(DataTypeCustom* dataType, uint32_t fieldId, const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    const SMDataTypeData& types() const;
    SMDataTypeData& types();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;
};

#endif  // LUSAN_MODEL_SM_SMDATATYPEMODEL_HPP
