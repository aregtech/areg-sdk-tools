#ifndef LUSAN_DATA_SI_SIDATATYPEDATA_HPP
#define LUSAN_DATA_SI_SIDATATYPEDATA_HPP
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
 *  \file        lusan/data/si/SIDataTypeData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Data Type Data.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/TEDataContainer.hpp"
#include "lusan/common/ElementBase.hpp"
#include <QObject>

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
  * \class   SIDataTypeData
  * \brief   Manages data type data for service interfaces.
  **/
class SIDataTypeData    : public QObject
                        , public TEDataContainer<DataTypeCustom*, ElementBase>
{
    Q_OBJECT
    
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     * \param   parent  The parent element.
     **/
    SIDataTypeData(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   entries     The list of data types.
     **/
    SIDataTypeData(QList<DataTypeCustom *>&& entries, ElementBase* parent = nullptr) noexcept;

    virtual ~SIDataTypeData(void);
    
signals:
        
    void signalDataTypeCreated(DataTypeCustom* dataType);
    
    void signalDataTypeRemoved(DataTypeCustom* dataType);
    
    void signalDataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType);
    
//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Adds a data type to the list.
     * \param   entry   The data type to add.
     **/
    void addCustomDataType(DataTypeCustom* entry);

    /**
     * \brief   Adds a data type to the list.
     * \param   name        The name of the data type.
     * \param   category    The category of the data type.
     * \return  The created data type object.
     **/
    DataTypeCustom* addCustomDataType(const QString& name, DataTypeBase::eCategory category);
    
    /**
     * \brief   Removes a data type from the list.
     * \param   entry   The data type to remove.
     * \return  True if the data type was removed, false otherwise.
     **/
    bool removeCustomDataType(const DataTypeCustom& entry);

    /**
     * \brief   Removes a data type from the list.
     * \param   id      The ID of the data type to remove.
     * \return  True if the data type was removed, false otherwise.
     **/
    bool removeCustomDataType(uint32_t id);

    /**
     * \brief   Replaces a data type in the list.
     * \param   oldEntry    The data type to replace.
     * \param   newEntry    The new data type.
     * \return  True if the data type was replaced, false otherwise.
     **/
    bool replaceCustomDataType(DataTypeCustom* oldEntry, DataTypeCustom * newEntry);

    /**
     * \brief   Reads data type data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the data type data was successfully read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes data type data to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

    /**
     * \brief   remove all entries and frees resources.
     **/
    void removeAll(void);

    /**
     * \brief   Returns the list of primitive data types objects.
     **/
    const QList<DataTypePrimitive*>& getPrimitiveDataTypes(void) const;

    /**
     * \brief   Returns the list of basic data types objects.
     **/
    const QList<DataTypeBasicObject*>& getBasicDataTypes(void) const;

    /**
     * \brief   Returns the list of basic container data types objects.
     **/
    const QList<DataTypeBasicContainer*>& getContainerDatTypes(void) const;

    /**
     * \brief   Gets the list of data types.
     * \return  The list of data types.
     **/
    const QList<DataTypeCustom*>& getCustomDataTypes(void) const;

    /**
     * \brief   Sets the list of data types.
     * \param   entries     The list of data types.
     **/
    void setCustomDataTypes(QList<DataTypeCustom *>&& entries);

    /**
     * \brief   Gets the list of data types.
     * \param   out_dataTypes   On output, this contains the list of data types.
     * \param   excludes        The list of data types to exclude. User for filtering.
     * \param   makeSorting     If true, the list of data types is sorted.
     **/
    void getDataType(QList<DataTypeBase*>& out_dataTypes, const QList<DataTypeBase *>& excludes = QList<DataTypeBase *>(), bool makeSorting = false) const;

    /**
     * \brief   Gets the list of data types.
     * \param   out_dataTypes   On output, this contains the list of data types.
     * \param   includes        The list of data type categories to include.
     * \param   makeSorting     If true, the list of data types is sorted.
     **/
    int getDataTypes(QList<DataTypeBase*>& out_dataTypes, const QList<DataTypeBase::eCategory>& includes, bool makeSorting = false) const;

    /**
     * \brief   Searches for a data type by name in the list of primitive data type objects.
     * \param   dataType    The list of primitive data types to search.
     * \param   searchName  The name of the data type to search.
     * \return  Returns true if the name is found in the list of primitives. Otherwise, returns false.
     **/
    bool existsPrimitive(const QList<DataTypePrimitive*> dataTypes, const QString& searchName) const;

    /**
     * \brief   Searches for a data type by ID in the list of basic data type objects.
     * \param   dataType    The list of basic data types to search.
     * \param   id          The ID of the data type to search.
     * \return  Returns true if the name is found in the list of basic data types. Otherwise, returns false.
     **/
    bool existsPrimitive(const QList<DataTypePrimitive*> dataTypes, uint32_t id) const;
    
    /**
     * \brief   Searches for a data type by name in the list of basic data type objects.
     * \param   dataType    The list of basic data types to search.
     * \param   searchName  The name of the data type to search.
     * \return  Returns true if the name is found in the list of basic data types. Otherwise, returns false.
     **/
    bool existsBasic(const QList<DataTypeBasicObject*> dataTypes, const QString& searchName) const;

    /**
     * \brief   Searches for a data type by ID in the list of basic data type objects.
     * \param   dataType    The list of basic data types to search.
     * \param   id          The ID of the data type to search.
     * \return  Returns true if the name is found in the list of basic data types. Otherwise, returns false.
     **/
    bool existsBasic(const QList<DataTypeBasicObject*> dataTypes, uint32_t id) const;
    
    /**
     * \brief   Searches for a data type by name in the list of basic container data type objects.
     * \param   dataType    The list of basic container data types to search.
     * \param   searchName  The name of the data type to search.
     * \return  Returns true if the name is found in the list of basic container data types. Otherwise, returns false.
     **/
    bool existsContainer(const QList<DataTypeBasicContainer*> dataTypes, const QString& searchName) const;

    /**
     * \brief   Searches for a data type by ID in the list of basic container data type objects.
     * \param   dataType    The list of basic container data types to search.
     * \param   id          The ID of the data type to search.
     * \return  Returns true if the name is found in the list of basic container data types. Otherwise, returns false.
     **/
    bool existsContainer(const QList<DataTypeBasicContainer*> dataTypes, uint32_t id) const;
    
    /**
     * \brief   Searches for a data type by name in the list of custom data type objects.
     * \param   dataType    The list of custom data types to search.
     * \param   searchName  The name of the data type to search.
     * \return  Returns true if the name is found in the list of custom data types. Otherwise, returns false.
     **/
    bool existsCustom(const QList<DataTypeCustom*> dataTypes, const QString& searchName) const;

    /**
     * \brief   Searches for a data type by ID in the list of custom data type objects.
     * \param   dataType    The list of custom data types to search.
     * \param   id          The ID of the data type to search.
     * \return  Returns true if the name is found in the list of custom data types. Otherwise, returns false.
     **/
    bool existsCustom(const QList<DataTypeCustom*> dataTypes, uint32_t id) const;
    
    /**
     * \brief   Searches for a data type by name in the list of data type objects.
     * \param   typeName    The name of the data type to search.
     * \return  Returns true if the name is found in the list of data types. Otherwise, returns false.
     **/
    template<class DataType>
    inline bool exists(const QList<DataType*> & dataTypes, const QString& typeName) const;

    /**
     * \brief   Searches for a data type by ID in the list of data type objects.
     * \param   typeName    The name of the data type to search.
     * \return  Returns true if the name is found in the list of data types. Otherwise, returns false.
     **/
    template<class DataType>
    inline bool exists(const QList<DataType*> & dataTypes, uint32_t id) const;
    
    /**
     * \brief   Searches for a data type by name in the list of all data type objects.
     * \param   typeName    The name of the data type to search.
     * \return  Returns true if the name is found in the list of data types. Otherwise, returns false.
     **/
    bool exists(const QString& typeName) const;

    /**
     * \brief   Searches for a data type by ID in the list of all data type objects.
     * \param   id  The ID of the data type to search.
     * \return  Returns true if the name is found in the list of data types. Otherwise, returns false.
     **/
    bool exists(uint32_t id) const;

    template<class DataType>
    inline DataType* findDataType(const QList<DataType*>& dataTypes, const QString& typeName) const;

    template<class DataType>
    inline DataType* findDataType(const QList<DataType*>& dataTypes, uint32_t id) const;

    /**
     * \brief   Searches for a data type by name in the list of all data type objects.
     * \param   typeName    The name of the data type to search.
     * \return  Returns the data type object if found. Otherwise, returns nullptr.
     **/
    DataTypeBase* findDataType(const QString& typeName) const;

    /**
     * \brief   Searches for a data type by unique ID in the list of all data type objects.
     * \param   id  The ID of the data type to search.
     * \return  Returns the data type object if found. Otherwise, returns nullptr.
     **/
    DataTypeBase* findDataType(uint32_t id) const;

    /**
     * \brief   Adds a Structure custom data type to the list.
     * \param   name    The name of the primitive data type.
     * \return  The created Structure custom data type object.
     **/
    DataTypeStructure* addStructure(const QString& name);

    /**
     * \brief   Adds an Enumeration custom data type to the list.
     * \param   name    The name of the primitive data type.
     * \return  The created Enumeration custom data type object.
     **/
    DataTypeEnum* addEnum(const QString& name);

    /**
     * \brief   Adds a Container custom data type to the list.
     * \param   name    The name of the primitive data type.
     * \return  The created Container custom data type object.
     **/
    DataTypeContainer* addContainer(const QString& name);

    /**
     * \brief   Adds an Imported custom data type to the list.
     * \param   name    The name of the primitive data type.
     * \return  The created Imported custom data type object.
     **/
    DataTypeImported* addImported(const QString& name);

    /**
     * \brief   Converts a specified custom data type to a new custom data type of specified category.
     *          It keeps the name and ID of the specified custom data type.
     * \param   name    The name of the data type to convert.
     * \return  Returns the data type object of a new category. Otherwise, returns nullptr.
     **/
    DataTypeCustom* convertDataType(DataTypeCustom* dataType, DataTypeBase::eCategory category);

    /**
     * \brief   Sorts custom data types ascending or descending.
     * \param   ascending   Flag, indicating whether the list of custom data types
     *                      should be sorted ascending or descending.
     *                      If true, the sorting is ascending.
     **/
    void sortByName(bool ascending);
    
    void sortById(bool ascending);

private:

    /**
     * \brief   Creates a custom data type object based on the given name, ID and category.
     * \param   name        The name of the data type.
     * \param   parent      The parent object.
     * \param   id          The unique ID of the data type.
     * \param   category    The category of the data type.
     * \return  Returns the created custom data type object.
     **/
    DataTypeCustom* _createType(const QString& name, ElementBase * parent, uint32_t id, DataTypeBase::eCategory category);

//////////////////////////////////////////////////////////////////////////
// Forbidden calls.
//////////////////////////////////////////////////////////////////////////
private:
    SIDataTypeData(const SIDataTypeData& /*src*/) = delete;
    SIDataTypeData(SIDataTypeData&& /*src*/) noexcept = delete;
    SIDataTypeData& operator = (const SIDataTypeData& /*src*/) = delete;
    SIDataTypeData& operator = (SIDataTypeData&& /*src*/) noexcept = delete;
};

//////////////////////////////////////////////////////////////////////////
// SIDataTypeData class inline methods.
//////////////////////////////////////////////////////////////////////////

template<class DataType>
inline bool SIDataTypeData::exists(const QList<DataType*>& dataTypes, const QString& typeName) const
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
inline bool SIDataTypeData::exists(const QList<DataType*>& dataTypes, uint32_t id) const
{
    for (const DataType* dataType : dataTypes)
    {
        Q_ASSERT(dataType != nullptr);
        if (dataType->getId() == id)
        {
            return true;
        }
    }
    
    return false;
}

template<class DataType>
inline DataType* SIDataTypeData::findDataType(const QList<DataType*>& dataTypes, const QString& typeName) const
{
    for (DataType* dataType : dataTypes)
    {
        if (dataType->getName() == typeName)
        {
            return dataType;
        }
    }

    return nullptr;
}

template<class DataType>
inline DataType* SIDataTypeData::findDataType(const QList<DataType*>& dataTypes, uint32_t id) const
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

#endif  // LUSAN_DATA_SI_SIDATATYPEDATA_HPP
