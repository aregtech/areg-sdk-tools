#ifndef LUSAN_MODEL_COMMON_DATATYPEMODEL_HPP
#define LUSAN_MODEL_COMMON_DATATYPEMODEL_HPP
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
 *  \file        lusan/model/common/DataTypeModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Data Types page model shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeDataSection.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeCustom;
class DataTypeContainer;
class DataTypeEnum;
class DataTypeImported;
class DataTypeStructure;
class ElementBase;
class EnumEntry;
class FieldEntry;

/**
 * \class   DataTypeModel
 * \brief   The Data Types page model. Reads the document's `DataTypeList` section and routes
 *          every edit through an undo command, so a page never mutates a `DataTypeCustom`,
 *          `FieldEntry` or `EnumEntry` directly. Enumeration, Structure, Imported and
 *          Container categories are all supported.
 *
 *          The model knows only the document interface, so the same class serves the service
 *          interface, the state machine and a standalone data type document. The section is
 *          asked for on every access: a document that opens or reloads a file swaps the data
 *          object underneath, and a section reference kept from construction would dangle.
 *
 *          Field-level commands re-resolve the field by owner pointer + ID inside the
 *          command's getter and setter, never by a captured `FieldEntry*` / `EnumEntry*`:
 *          those are stored by value in the owning type's list, so a sibling add or remove
 *          can reallocate and move them. The owning `DataTypeCustom*` is stable, because the
 *          section stores it by pointer, so capturing it directly is safe.
 **/
class DataTypeModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit DataTypeModel(IEDocumentModel& document);

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const DataTypeDataSection& getDataTypeData() const;
    DataTypeDataSection& getDataTypeData();

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
    IEDocumentModel& getDocument() const;

//////////////////////////////////////////////////////////////////////////
// Mutations -- data type level
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

    //!< Sets namespace and object together from the "<namespace::>object" form the tree shows,
    //!< as one undo step -- the user typed one text, not two.
    void setImportQualifiedName(DataTypeImported* dataType, const QString& qualified);

    //!< Sets the basic-container object (Array/LinkedList/HashMap/Map/Pair/custom) by name.
    //!< Re-derives the resulting key (defaulted, kept, or cleared) the same way
    //!< DataTypeContainer::setContainer() does, so undo restores the exact prior key.
    void setContainerObject(DataTypeContainer* dataType, const QString& basicName);
    //!< Sets the key element type by name -- see setFieldType for why name, not DataTypeBase*.
    void setContainerKey(DataTypeContainer* dataType, const QString& typeName);
    //!< Sets the value element type by name -- see setFieldType for why name, not DataTypeBase*.
    void setContainerValue(DataTypeContainer* dataType, const QString& typeName);

//////////////////////////////////////////////////////////////////////////
// Mutations -- field level (structure fields / enumeration entries)
//////////////////////////////////////////////////////////////////////////
public:
    ElementBase* createField(DataTypeCustom* dataType, const QString& name);
    ElementBase* insertField(DataTypeCustom* dataType, int position, const QString& name);
    void deleteField(DataTypeCustom* dataType, uint32_t fieldId);
    void swapFields(DataTypeCustom* dataType, uint32_t firstId, uint32_t secondId);

    void setFieldName(DataTypeCustom* dataType, uint32_t fieldId, const QString& name);

    //!< Sets the field's type by name -- the actual persisted state (`Field DataType="..."`).
    //!< Not by DataTypeBase*: the type wrapper only resolves a pointer on demand and its
    //!< setType() cannot restore an unresolved original name on undo, so the name string is
    //!< the only value this command may round-trip.
    void setFieldType(DataTypeStructure* dataType, uint32_t fieldId, const QString& typeName);
    void setFieldValue(DataTypeCustom* dataType, uint32_t fieldId, const QString& value);
    void setFieldDescription(DataTypeCustom* dataType, uint32_t fieldId, const QString& text);
    void setFieldDeprecated(DataTypeCustom* dataType, uint32_t fieldId, bool deprecated);
    void setFieldDeprecateHint(DataTypeCustom* dataType, uint32_t fieldId, const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    inline DataTypeDataSection& types() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    IEDocumentModel&    mDocument;
};

//////////////////////////////////////////////////////////////////////////
// DataTypeModel inline methods
//////////////////////////////////////////////////////////////////////////

inline DataTypeDataSection& DataTypeModel::types() const
{
    return mDocument.getDataTypeSection();
}

#endif  // LUSAN_MODEL_COMMON_DATATYPEMODEL_HPP
