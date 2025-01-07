#ifndef LUSAN_DATA_COMMON_DATATYPEBASIC_HPP
#define LUSAN_DATA_COMMON_DATATYPEBASIC_HPP
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
 *  \file        lusan/data/common/DataTypeBasic.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Basic Data Type.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeBase.hpp"

//////////////////////////////////////////////////////////////////////////
// DataTypeBasicObject class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   Basic object data type. Such as string or byte array.
 **/
class DataTypeBasicObject : public DataTypeBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    DataTypeBasicObject(void);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypeBasicObject(const DataTypeBasicObject& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypeBasicObject(DataTypeBasicObject&& src) noexcept;

    /**
     * \brief   Constructor with name initialization.
     * \param   name    The name of the data type.
     **/
    DataTypeBasicObject(const QString& name);

//////////////////////////////////////////////////////////////////////////
// operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypeBasicObject& operator = (const DataTypeBasicObject& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypeBasicObject& operator = (DataTypeBasicObject&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes, operations and overrides
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
};

//////////////////////////////////////////////////////////////////////////
// DataTypeBasicContainer class declaration
//////////////////////////////////////////////////////////////////////////

class DataTypeBasicContainer : public DataTypeBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    DataTypeBasicContainer(void);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypeBasicContainer(const DataTypeBasicContainer& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypeBasicContainer(DataTypeBasicContainer&& src) noexcept;

    /**
     * \brief   Constructor with name initialization.
     * \param   name    The name of the data type.
     **/
    DataTypeBasicContainer(const QString& name);

//////////////////////////////////////////////////////////////////////////
// operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypeBasicContainer& operator=(const DataTypeBasicContainer& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypeBasicContainer& operator=(DataTypeBasicContainer&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes, operations and overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Sets the data container flag.
     * \param   hasKey  If true, the data type is a container with key-value pair.
     **/
    void setKey(bool hasKey);

    /**
     * \brief   Checks if the data type has a key.
     * \return  True if the data type has a key, false otherwise.
     **/
    bool hasKey(void) const;

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

private:
    bool    mHasKey;    //!< The flag, indicating whether the data type has a key.
};

#endif // LUSAN_DATA_COMMON_DATATYPEBASIC_HPP
