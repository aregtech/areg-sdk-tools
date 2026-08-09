#ifndef LUSAN_DATA_COMMON_DATATYPEDATASECTION_HPP
#define LUSAN_DATA_COMMON_DATATYPEDATASECTION_HPP
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
 *  \file        lusan/data/common/DataTypeDataSection.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the data types section of a document.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

#include "lusan/data/common/DataTypeBase.hpp"

#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QXmlStreamReader;
class QXmlStreamWriter;

class DataTypeCustom;
class DataTypeBasicContainer;
class DataTypeBasicObject;
class DataTypeContainer;
class DataTypeEnum;
class DataTypeImported;
class DataTypePrimitive;
class DataTypeStructure;

 /**
  * \class   DataTypeDataSection
  * \brief   The `DataTypeList` section of a document: the enumerations, structures, imported
  *          types and containers it declares, in document order. A data type carries the same
  *          declaration in every document type, so one section serves the service interface,
  *          the state machine and a standalone data type document alike.
  *
  *          The section owns its entries and deletes the ones it still holds on destruction.
  *          It announces nothing: every edit reaches it through an undo command, and the
  *          command reports the change on the document's notifier.
  **/
class DataTypeDataSection : public TEDataContainer<DataTypeCustom*, DocumentElem>
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     * \param   parent  The parent element.
     **/
    DataTypeDataSection(ElementBase * parent = nullptr);

    virtual ~DataTypeDataSection();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Checks if the section is valid.
     **/
    bool isValid() const override;

    /**
     * \brief   Reads the data types from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the data types were read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes the data types to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the list of custom data types the document declares, in document order.
     **/
    const QList<DataTypeCustom*>& getCustomDataTypes() const;

    /**
     * \brief   Returns the list of predefined primitive data type objects.
     **/
    const QList<DataTypePrimitive*>& getPrimitiveDataTypes() const;

    /**
     * \brief   Returns the list of predefined basic data type objects.
     **/
    const QList<DataTypeBasicObject*>& getBasicDataTypes() const;

    /**
     * \brief   Returns the list of predefined basic container data type objects.
     **/
    const QList<DataTypeBasicContainer*>& getContainerDatTypes() const;

    /**
     * \brief   Collects the predefined types followed by the document's custom types.
     * \param   out_dataTypes   On output, this contains the list of data types.
     * \param   excludes        The data types to leave out. Used for filtering.
     * \param   makeSorting     If true, each of the two groups is sorted.
     **/
    void getDataType(QList<DataTypeBase*>& out_dataTypes, const QList<DataTypeBase *>& excludes = QList<DataTypeBase *>(), bool makeSorting = false) const;

    /**
     * \brief   Searches for a data type by name among the predefined and the custom types.
     * \param   typeName    The name of the data type to search.
     * \return  Returns the data type object if found. Otherwise, returns nullptr.
     **/
    DataTypeBase* findDataType(const QString& typeName) const;

    /**
     * \brief   Searches for a data type by ID among the predefined and the custom types.
     * \param   id  The ID of the data type to search.
     * \return  Returns the data type object if found. Otherwise, returns nullptr.
     **/
    DataTypeBase* findDataType(uint32_t id) const;

    /**
     * \brief   Searches for a custom data type by name. Matches the qualified form of an
     *          imported type as well, which is why it is not the plain container lookup.
     * \param   typeName    The name of the data type to search.
     * \return  Returns the data type object if found. Otherwise, returns nullptr.
     **/
    DataTypeCustom* findCustomDataType(const QString& typeName) const;

    /**
     * \brief   Searches for a custom data type by unique ID.
     * \param   typeId  The ID of the data type to search.
     * \return  Returns the data type object if found. Otherwise, returns nullptr.
     **/
    DataTypeCustom* findCustomDataType(uint32_t typeId) const;

    /**
     * \brief   Creates a custom data type of the given category and appends it.
     * \param   name        The unique name of the new data type.
     * \param   category    The category of the data type.
     * \return  The created data type object, or nullptr if the name is already taken.
     **/
    DataTypeCustom* addCustomDataType(const QString& name, DataTypeBase::eCategory category);

    /**
     * \brief   Appends a Structure custom data type.
     **/
    DataTypeStructure* addStructure(const QString& name);

    /**
     * \brief   Appends an Enumeration custom data type.
     **/
    DataTypeEnum* addEnum(const QString& name);

    /**
     * \brief   Appends a Container custom data type.
     **/
    DataTypeContainer* addContainer(const QString& name);

    /**
     * \brief   Appends an Imported custom data type.
     **/
    DataTypeImported* addImported(const QString& name);

    /**
     * \brief   Deletes and removes every data type.
     **/
    void removeAll();

    /**
     * \brief   Resolves every structure field's and container key/value declared type against
     *          the given registry, so a freshly loaded document reflects the same resolved
     *          state as one edited interactively.
     * \param   dataTypes   The document's data type registry.
     **/
    void validate(const DataTypeDataSection& dataTypes);

    /**
     * \brief   Resolves one custom type's nested type references (structure fields, container
     *          key and value) against this registry. Does nothing for the other categories.
     * \param   dataType    The data type to resolve.
     **/
    void normalizeType(DataTypeCustom* dataType) const;

//////////////////////////////////////////////////////////////////////////
// Hidden calls.
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Returns true if a data type with the given name is in the list.
     **/
    template<class DataType>
    inline bool exists(const QList<DataType*> & dataTypes, const QString& typeName) const;

    /**
     * \brief   Returns the data type with the given ID, or nullptr.
     **/
    template<class DataType>
    inline DataType* findById(const QList<DataType*>& dataTypes, uint32_t id) const;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls.
//////////////////////////////////////////////////////////////////////////
private:
    DataTypeDataSection(const DataTypeDataSection& /*src*/) = delete;
    DataTypeDataSection(DataTypeDataSection&& /*src*/) noexcept = delete;
    DataTypeDataSection& operator = (const DataTypeDataSection& /*src*/) = delete;
    DataTypeDataSection& operator = (DataTypeDataSection&& /*src*/) noexcept = delete;
};

//////////////////////////////////////////////////////////////////////////
// DataTypeDataSection class inline methods.
//////////////////////////////////////////////////////////////////////////

template<class DataType>
inline bool DataTypeDataSection::exists(const QList<DataType*>& dataTypes, const QString& typeName) const
{
    for (const DataType* dataType : dataTypes)
    {
        Q_ASSERT(dataType != nullptr);
        if (dataType->getName() == typeName)
        {
            return true;
        }
    }

    return false;
}

template<class DataType>
inline DataType* DataTypeDataSection::findById(const QList<DataType*>& dataTypes, uint32_t id) const
{
    for (DataType* dataType : dataTypes)
    {
        if (dataType->getId() == id)
        {
            return dataType;
        }
    }

    return nullptr;
}

#endif  // LUSAN_DATA_COMMON_DATATYPEDATASECTION_HPP
