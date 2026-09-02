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
#include <QObject>
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
class QFileSystemWatcher;

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
 *
 *          The model also keeps the section coherent: a structure field and a container
 *          key/value each store a resolved type object beside the type name they were given,
 *          and the model re-resolves all of them whenever the set of types changes -- an edit,
 *          an undo, a redo or a reload alike. It listens to the notifier for that, so the work
 *          happens once per change and before the pages read the section.
 **/
class DataTypeModel : public QObject
{
    Q_OBJECT

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
// Imported data type documents
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   What the included data type documents currently contribute, in include order.
     **/
    const QList<DataTypeDataSection::ImportedTypes>& getImports() const;

    /**
     * \brief   Rebuilds the imported groups from the document's include list and, when that
     *          changed anything, re-resolves the declared types, re-arms the file watch and
     *          announces it. Called on every include edit and whenever an included file changes
     *          on disk, so a data type document saved in one window reaches the others at once.
     **/
    void refreshImports();

signals:
    /**
     * \brief   The imported groups are not what they were: a document was included, dropped,
     *          repointed, or changed on disk. Not routed through the document notifier, which
     *          carries what a command did to the document itself.
     **/
    void importsChanged();

//////////////////////////////////////////////////////////////////////////
// Mutations -- data type level
//////////////////////////////////////////////////////////////////////////
public:
    DataTypeCustom* createDataType(const QString& name, DataTypeBase::eCategory category);
    DataTypeCustom* insertDataType(int position, const QString& name, DataTypeBase::eCategory category);
    void deleteDataType(DataTypeCustom* dataType);
    DataTypeCustom* convertDataType(DataTypeCustom* dataType, DataTypeBase::eCategory category);
    /**
     * rief   Moves the data type one position up or down and answers the ID it carries
     *          afterwards. A reorder leaves the moved element with the ID of the element it
     *          passed, so a caller keeps the selection on what moved by taking this answer.
     * \param   id      The ID of the data type to move.
     * \param   delta   -1 to move one position up, +1 to move one position down.
     * eturn  The ID the moved data type carries afterwards, or 0 when nothing moved.
     **/
    uint32_t moveDataType(uint32_t id, int delta);

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
    /**
     * \brief   Moves a field of the given data type one position up or down and answers the ID it
     *          carries afterwards.
     * \param   dataType    The structure or enumeration owning the field.
     * \param   fieldId     The ID of the field to move.
     * \param   delta       -1 to move one position up, +1 to move one position down.
     * \return  The ID the moved field carries afterwards, or 0 when nothing moved.
     **/
    uint32_t moveField(DataTypeCustom* dataType, uint32_t fieldId, int delta);

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

    //!< Points the watch at the files the imports currently resolve to. A watch is dropped by
    //!< the system when the file is replaced rather than written in place, which is how most
    //!< editors save, so the whole set is re-armed on every rebuild.
    void rearmImportWatch();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    IEDocumentModel&    mDocument;
    QFileSystemWatcher* mImportWatch;   //!< Watches the included data type documents; owned by this.
};

//////////////////////////////////////////////////////////////////////////
// DataTypeModel inline methods
//////////////////////////////////////////////////////////////////////////

inline DataTypeDataSection& DataTypeModel::types() const
{
    return mDocument.getDataTypeSection();
}

#endif  // LUSAN_MODEL_COMMON_DATATYPEMODEL_HPP
