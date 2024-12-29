#ifndef LUSAN_DATA_COMMON_DATATYPEBASE_HPP
#define LUSAN_DATA_COMMON_DATATYPEBASE_HPP
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
 *  \file        lusan/data/common/DataTypeBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type Base.
 *
 ************************************************************************/

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

 /**
  * \class   DataTypeBase
  * \brief   Base class for data types in the Lusan application.
  **/
class DataTypeBase
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    DataTypeBase::eCategory
     * \brief   Represents the category of the data type.
     **/
    typedef enum class E_Category : uint16_t
    {
          Undefined     = 0x0000    //!< bits: 0000 0000 0000 0000, Undefined type
        , Primitive     = 0x0008    //!< bits: 0000 0000 0000 1000, Primitive type
        , PrimitiveInt  = 0x0009    //!< bits: 0000 0000 0000 1001, Primitive integer type
        , PrimitiveUint = 0x000B    //!< bits: 0000 0000 0000 1011, Primitive unsigned integer type
        , PrimitiveFloat= 0x000C    //!< bits: 0000 0000 0000 1100, Primitive unsigned integer type
        , BasicObject   = 0x0010    //!< bits: 0000 0000 0001 0000, Basic object
        , CustomDefined = 0x0100    //!< bits: 0000 0001 0000 0000, Custom type enumeration
        , Enumeration   = 0x0300    //!< bits: 0000 0011 0000 0000, Custom type enumeration
        , Structure     = 0x0500    //!< bits: 0000 0101 0000 0000, Custom type structure
        , Imported      = 0x0900    //!< bits: 0000 1001 0000 0000, Custom type imported
        , Container     = 0x1100    //!< bits: 0001 0001 0000 0000, Custom type container
    } eCategory;

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Default constructor.
     **/
    DataTypeBase(void);

    /**
     * \brief   Constructor with initialization.
     * \param   name        The name of the data type.
     * \param   category    The category of the data type.
     **/
    DataTypeBase(eCategory category, const QString& name = QString());

    /**
     * \brief   Copy constructor.
     * \param   other   The other DataTypeBase to copy from.
     **/
    DataTypeBase(const DataTypeBase& other);

    /**
     * \brief   Move constructor.
     * \param   other   The other DataTypeBase to move from.
     **/
    DataTypeBase(DataTypeBase&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other DataTypeBase to copy from.
     * \return  Reference to the assigned DataTypeBase.
     **/
    DataTypeBase& operator = (const DataTypeBase& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other DataTypeBase to move from.
     * \return  Reference to the assigned DataTypeBase.
     **/
    DataTypeBase& operator = (DataTypeBase&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Equality operator.
     * \param   other   The other DataTypeBase to compare with.
     * \return  True if the name and category are equal, false otherwise.
     **/
    bool operator == (const DataTypeBase& other) const;

    /**
     * \brief   Inequality operator.
     * \param   other   The other DataTypeBase to compare with.
     * \return  True if the name or category are not equal, false otherwise.
     **/
    bool operator != (const DataTypeBase& other) const;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the name of the data type.
     * \return  The name of the data type.
     **/
    const QString& getName(void) const;

    /**
     * \brief   Sets the name of the data type.
     * \param   name    The name of the data type.
     **/
    void setName(const QString& name);

    /**
     * \brief   Gets the category of the data type.
     * \return  The category of the data type.
     **/
    eCategory getCategory(void) const;

    /**
     * \brief   Checks if the data type is a primitive type.
     * \return  True if the data type is a primitive type, false otherwise.
     **/
    bool isPrimitive(void) const;

    bool isCustomDefined(void) const;

    bool isPredefined(void) const;

    /**
     * \brief   Checks if the data type is a primitive integer type.
     * \return  True if the data type is a primitive integer type, false otherwise.
     **/
    bool isPrimitiveInt(void) const;

    /**
     * \brief   Checks if the data type is a primitive unsigned integer type.
     * \return  True if the data type is a primitive unsigned integer type, false otherwise.
     **/
    bool isPrimitiveUint(void) const;

    /**
     * \brief   Checks if the data type is a primitive float type.
     * \return  True if the data type is a primitive float type, false otherwise.
     **/
    bool isPrimitiveFloat(void) const;

    /**
     * \brief   Checks if the data type is a basic object.
     * \return  True if the data type is a basic object, false otherwise.
     **/
    bool isBasicObject(void) const;

    /**
     * \brief   Checks if the data type is an enumeration.
     * \return  True if the data type is an enumeration, false otherwise.
     **/
    bool isEnumeration(void) const;

    /**
     * \brief   Checks if the data type is a structure.
     * \return  True if the data type is a structure, false otherwise.
     **/
    bool isStructure(void) const;

    /**
     * \brief   Checks if the data type is imported.
     * \return  True if the data type is imported, false otherwise.
     **/
    bool isImported(void) const;

    /**
     * \brief   Checks if the data type is a container.
     * \return  True if the data type is a container, false otherwise.
     **/
    bool isContainer(void) const;

    /**
     * \brief   Checks if the data type matches the specified type.
     * \param   theType The type to check against.
     * \return  True if the data type matches the specified type, false otherwise.
     **/
    bool isTypeOf(const QString& theType) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Checks if the data type is valid.
     * \return  True if the data type is valid, false otherwise.
     **/
    virtual bool isValid(void) const;

    /**
     * \brief   Reads data from an XML stream.
     * \param   xml     The XML stream reader.
     * \return  True if the data was successfully read, false otherwise.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) = 0;

    /**
     * \brief   Writes data to an XML stream.
     * \param   xml     The XML stream writer.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const = 0;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    eCategory   mCategory;      //!< The category of the data type.
    QString     mName;          //!< The name of the data type.
};

#endif  // LUSAN_DATA_COMMON_DATATYPEBASE_HPP
