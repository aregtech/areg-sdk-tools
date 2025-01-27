#ifndef LUSAN_DATA_COMMON_DATATYPEPRIMITIVE_HPP
#define LUSAN_DATA_COMMON_DATATYPEPRIMITIVE_HPP
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
 *  \file        lusan/data/common/DataTypePrimitive.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Primitive Data Type.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeBase.hpp"

//////////////////////////////////////////////////////////////////////////
// DataTypePrimitive class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   Primitive data type such as integers, characters, floats, etc.
 **/
class DataTypePrimitive : public DataTypeBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Constructor with category initialization.
     * \param   category    The category of the data type.
     **/
    DataTypePrimitive(DataTypeBase::eCategory category);

    /**
     * \brief   Constructor with name initialization.
     * \param   name    The name of the data type.
     **/
    DataTypePrimitive(const QString& name);

    /**
     * \brief   Constructor with category and name initialization.
     * \param   category    The category of the data type.
     * \param   name        The name of the data type.
     **/
    DataTypePrimitive(DataTypeBase::eCategory category, const QString& name);

public:
    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypePrimitive(const DataTypePrimitive& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypePrimitive(DataTypePrimitive&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypePrimitive& operator=(const DataTypePrimitive& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypePrimitive& operator=(DataTypePrimitive&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Reads data from an XML stream.
     * \param   xml     The XML stream reader.
     * \return  True if the data was successfully read, false otherwise.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes data to an XML stream.
     * \param   xml     The XML stream writer.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Default constructor.
     **/
    DataTypePrimitive(void) = delete;
};

//////////////////////////////////////////////////////////////////////////
// DataTypePrimitiveInt class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   Primitive data type representing an integer.
 **/
class DataTypePrimitiveInt : public DataTypePrimitive
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Default constructor.
     **/
    DataTypePrimitiveInt(void);
    /**
     * \brief   Constructor with name initialization.
     * \param   name    The name of the data type.
     **/
    DataTypePrimitiveInt(const QString& name);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypePrimitiveInt(const DataTypePrimitiveInt& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypePrimitiveInt(DataTypePrimitiveInt&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypePrimitiveInt& operator=(const DataTypePrimitiveInt& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypePrimitiveInt& operator=(DataTypePrimitiveInt&& other) noexcept;
};

//////////////////////////////////////////////////////////////////////////
// DataTypePrimitiveUint class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   Primitive data type representing an unsigned integer.
 **/
class DataTypePrimitiveUint : public DataTypePrimitive
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Default constructor.
     **/
    DataTypePrimitiveUint(void);
    /**
     * \brief   Constructor with name initialization.
     * \param   name    The name of the data type.
     **/
    DataTypePrimitiveUint(const QString& name);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypePrimitiveUint(const DataTypePrimitiveUint& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypePrimitiveUint(DataTypePrimitiveUint&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypePrimitiveUint& operator=(const DataTypePrimitiveUint& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypePrimitiveUint& operator=(DataTypePrimitiveUint&& other) noexcept;
};

//////////////////////////////////////////////////////////////////////////
// DataTypePrimitiveFloat class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   Primitive data type representing a floating-point number.
 **/
class DataTypePrimitiveFloat : public DataTypePrimitive
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Default constructor.
     **/
    DataTypePrimitiveFloat(void);
    /**
     * \brief   Constructor with name initialization.
     * \param   name    The name of the data type.
     **/
    DataTypePrimitiveFloat(const QString& name);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypePrimitiveFloat(const DataTypePrimitiveFloat& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypePrimitiveFloat(DataTypePrimitiveFloat&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypePrimitiveFloat& operator=(const DataTypePrimitiveFloat& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypePrimitiveFloat& operator=(DataTypePrimitiveFloat&& other) noexcept;
};

#endif // LUSAN_DATA_COMMON_DATATYPEPRIMITIVE_HPP
