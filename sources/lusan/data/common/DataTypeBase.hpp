#ifndef LUSAN_DATA_COMMON_DATATYPEBASE_HPP
#define LUSAN_DATA_COMMON_DATATYPEBASE_HPP
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
 *  \file        lusan/data/common/DataTypeBase.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type Base.
 *
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

 /**
  * \class   DataTypeBase
  * \brief   Base class for data types in the Lusan application.
  **/
class DataTypeBase  : public DocumentElem
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
        , Primitive     = 0x0008    //!< bits: 0000 0000 0000 1000, Primitive type, as well boolean type
        , PrimitiveSint = 0x0009    //!< bits: 0000 0000 0000 1001, Primitive integer type
        , PrimitiveUint = 0x000B    //!< bits: 0000 0000 0000 1011, Primitive unsigned integer type
        , PrimitiveFloat= 0x000C    //!< bits: 0000 0000 0000 1100, Primitive digit with floating point
        , BasicObject   = 0x0010    //!< bits: 0000 0000 0001 0000, Basic object
        , BasicContainer= 0x0020    //!< bits: 0000 0000 0010 0001, Basic container
        , CustomDefined = 0x0100    //!< bits: 0000 0001 0000 0000, Custom type basic
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
     * \param   parent  The parent element.
     **/
    DataTypeBase(ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   category    The category of the data type.
     * \param   name        The name of the data type.
     * \param   id          The ID of the data type.
     * \param   parent      The parent element.
     **/
    DataTypeBase(eCategory category, const QString& name = QString(), uint32_t id = 0, ElementBase* parent = nullptr);

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
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief Returns the icon to display for specific display type.
     * \param display   The classification to display.
     */
    virtual QIcon getIcon(ElementBase::eDisplay display) const override;
    
    /**
     * \brief Returns the string to display for specific display type.
     * \param display   The classification to display.
     */
    virtual QString getString(ElementBase::eDisplay display) const override;
    
    /**
     * \brief   Checks if the data type is valid.
     * \return  True if the data type is valid, false otherwise.
     **/
    virtual bool isValid() const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the name of the data type.
     * \return  The name of the data type.
     **/
    const QString& getName() const;

    /**
     * \brief   Sets the name of the data type.
     * \param   name    The name of the data type.
     **/
    void setName(const QString& name);

    /**
     * \brief   Gets the category of the data type.
     * \return  The category of the data type.
     **/
    eCategory getCategory() const;

    /**
     * \brief   Checks if the data type is a primitive type.
     * \return  True if the data type is a primitive type, false otherwise.
     **/
    bool isPrimitive() const;

    /**
     * \brief   Checks if the data type is a custom defined type.
     * \return  True if the data type is a custom defined type, false otherwise.
     **/
    bool isCustomDefined() const;

    /**
     * \brief   Checks if the data type is a predefined type.
     * \return  True if the data type is a predefined type, false otherwise.
     **/
    bool isPredefined() const;

    /**
     * \brief   Checks if the data type is a boolean type.
     * \return  True if the data type is a boolean type, false otherwise.
     **/
    bool isPrimitiveBool() const;

    /**
     * \brief   Checks if the data type is a primitive signed integer type.
     * \return  True if the data type is a primitive integer type, false otherwise.
     **/
    bool isPrimitiveInt() const;

    /**
     * \brief   Checks if the data type is a primitive signed integer type.
     * \return  True if the data type is a primitive integer type, false otherwise.
     **/
    bool isPrimitiveSint() const;

    /**
     * \brief   Checks if the data type is a primitive unsigned integer type.
     * \return  True if the data type is a primitive unsigned integer type, false otherwise.
     **/
    bool isPrimitiveUint() const;

    /**
     * \brief   Checks if the data type is a primitive float type.
     * \return  True if the data type is a primitive float type, false otherwise.
     **/
    bool isPrimitiveFloat() const;

    /**
     * \brief   Checks if the data type is a basic object.
     * \return  True if the data type is a basic object, false otherwise.
     **/
    bool isBasicObject() const;

    /**
     * \brief   Checks if the data type is a basic container.
     * \return  True if the data type is a basic container, false otherwise.
     **/
    bool isBasicContainer() const;

    /**
     * \brief   Checks if the data type is an enumeration.
     * \return  True if the data type is an enumeration, false otherwise.
     **/
    bool isEnumeration() const;

    /**
     * \brief   Checks if the data type is a structure.
     * \return  True if the data type is a structure, false otherwise.
     **/
    bool isStructure() const;

    /**
     * \brief   Checks if the data type is imported.
     * \return  True if the data type is imported, false otherwise.
     **/
    bool isImported() const;

    /**
     * \brief   Checks if the data type is a container.
     * \return  True if the data type is a container, false otherwise.
     **/
    bool isContainer() const;

    /**
     * \brief   Checks if the data type matches the specified type.
     * \param   theType The type to check against.
     * \return  True if the data type matches the specified type, false otherwise.
     **/
    bool isTypeOf(const QString& theType) const;
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    eCategory   mCategory;      //!< The category of the data type.
    QString     mName;          //!< The name of the data type.
};

//////////////////////////////////////////////////////////////////////////
// DataTypeBase class inline methods
//////////////////////////////////////////////////////////////////////////

#endif  // LUSAN_DATA_COMMON_DATATYPEBASE_HPP
