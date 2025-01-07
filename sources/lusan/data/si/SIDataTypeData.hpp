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
 *  \copyright   � 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/si/SIDataTypeData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Data Type Data.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/ElementBase.hpp"
#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QXmlStreamReader;
class QXmlStreamWriter;
class DataTypeBase;
class DataTypeCustom;
class DataTypePrimitive;
class DataTypeBasicContainer;
class DataTypeBasicObject;

 /**
  * \class   SIDataTypeData
  * \brief   Manages data type data for service interfaces.
  **/
class SIDataTypeData    : public ElementBase
{
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
     * \brief   Constructor. Copies data from given source.
     **/
    SIDataTypeData(const SIDataTypeData& src) = default;

    /**
     * \brief   Constructor with initialization.
     * \param   entries     The list of data types.
     **/
    SIDataTypeData(QList<DataTypeCustom *>&& entries, ElementBase* parent = nullptr) noexcept;

    virtual ~SIDataTypeData(void);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Searches for a data type in the list.
     * \param   entry   The data type to search for.
     * \return  The index of the data type, or -1 if not found.
     **/
    int findCustomDataType(const DataTypeCustom& entry) const;

    /**
     * \brief   Adds a data type to the list.
     * \param   entry   The data type to add.
     **/
    void addCustomDataType(DataTypeCustom* entry);

    /**
     * \brief   Removes a data type from the list.
     * \param   entry   The data type to remove.
     * \return  True if the data type was removed, false otherwise.
     **/
    bool removeCustomDataType(const DataTypeCustom& entry);

    /**
     * \brief   Replaces a data type in the list.
     * \param   oldEntry    The data type to replace.
     * \param   newEntry    The new data type.
     * \return  True if the data type was replaced, false otherwise.
     **/
    bool replaceCustomDataType(const DataTypeCustom& oldEntry, DataTypeCustom * newEntry);

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
    void getDataType(QList<DataTypeBase*>& out_dataTypes, const QList<DataTypeBase *>& excludes = QList<DataTypeBase *>(), bool makeSorting = true) const;

    /**
     * \brief   Searches for a data type by name in the list of primitive data type objects.
     * \param   dataType    The list of primitive data types to search.
     * \param   searchName  The name of the data type to search.
     * \return  Returns true if the name is found in the list of primitives. Otherwise, returns false.
     **/
    bool existsPrimitive(const QList<DataTypePrimitive*> dataTypes, const QString& searchName) const;

    /**
     * \brief   Searches for a data type by name in the list of basic data type objects.
     * \param   dataType    The list of basic data types to search.
     * \param   searchName  The name of the data type to search.
     * \return  Returns true if the name is found in the list of basic data types. Otherwise, returns false.
     **/
    bool existsBasic(const QList<DataTypeBasicObject*> dataTypes, const QString& searchName) const;

    /**
     * \brief   Searches for a data type by name in the list of basic container data type objects.
     * \param   dataType    The list of basic container data types to search.
     * \param   searchName  The name of the data type to search.
     * \return  Returns true if the name is found in the list of basic container data types. Otherwise, returns false.
     **/
    bool existsContainer(const QList<DataTypeBasicContainer*> dataTypes, const QString& searchName) const;

    /**
     * \brief   Searches for a data type by name in the list of custom data type objects.
     * \param   dataType    The list of custom data types to search.
     * \param   searchName  The name of the data type to search.
     * \return  Returns true if the name is found in the list of custom data types. Otherwise, returns false.
     **/
    bool existsCustom(const QList<DataTypeCustom*> dataTypes, const QString& searchName) const;

    /**
     * \brief   Searches for a data type by name in the list of data type objects.
     * \param   typeName    The name of the data type to search.
     * \return  Returns true if the name is found in the list of data types. Otherwise, returns false.
     **/
    template<class DataType>
    bool exists(const QList<DataType*> & dataTypes, const QString& typeName) const;

    /**
     * \brief   Searches for a data type by name in the list of all data type objects.
     * \param   typeName    The name of the data type to search.
     * \return  Returns true if the name is found in the list of data types. Otherwise, returns false.
     **/
    bool exists(const QString& typeName) const;

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

//////////////////////////////////////////////////////////////////////////
// Hidden member variables.
//////////////////////////////////////////////////////////////////////////
private:
    QList<DataTypeCustom *>     mCustomDataTypes;   //!< The list of data types.
    QList<DataTypeBase *>       mDataTypes;         //!< The list of all data types.
};

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

#endif  // LUSAN_DATA_SI_SIDATATYPEDATA_HPP
