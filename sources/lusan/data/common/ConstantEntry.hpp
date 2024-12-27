#ifndef LUSAN_DATA_COMMON_CONSTANTENTRY_HPP
#define LUSAN_DATA_COMMON_CONSTANTENTRY_HPP
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
 *  \file        lusan/data/common/ConstantEntry.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, ConstantEntry.
 *
 ************************************************************************/

#include "lusan/data/common/ParamBase.hpp"

/**
 * \class   ConstantEntry
 * \brief   Represents a constant in the Lusan application.
 **/
class ConstantEntry : public ParamBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    ConstantEntry(void);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the constant.
     * \param   name            The name of the constant.
     * \param   type            The data type name of the constant.
     * \param   value           The value of the constant.
     * \param   isDeprecated    The deprecated flag of the constant.
     * \param   description     The description of the constant.
     * \param   deprecateHint   The deprecation hint of the constant.
     **/
    ConstantEntry(uint32_t id, const QString& name, const QString& type, const QString& value, bool isDeprecated, const QString& description, const QString& deprecateHint);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    ConstantEntry(const ConstantEntry& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    ConstantEntry(ConstantEntry&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    ConstantEntry& operator = (const ConstantEntry& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    ConstantEntry& operator = (ConstantEntry&& other) noexcept;

    /**
     * \brief   Equality operator.
     * \param   other   The other object to compare with.
     * \return  True if the constants are equal, false otherwise.
     **/
    bool operator == (const ConstantEntry& other) const;

    /**
     * \brief   Inequality operator.
     * \param   other   The other object to compare with.
     * \return  True if the constants are not equal, false otherwise.
     **/
    bool operator != (const ConstantEntry& other) const;

    /**
     * \brief   Less than operator for sorting.
     * \param   other   The other object to compare with.
     * \return  True if this constant is less than the other, false otherwise.
     **/
    bool operator < (const ConstantEntry& other) const;

    /**
     * \brief   Less than operator for sorting.
     * \param   other   The other object to compare with.
     * \return  True if this constant is less than the other, false otherwise.
     **/
    bool operator > (const ConstantEntry& other) const;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the value of the constant.
     * \return  The value of the constant.
     **/
    QString getValue() const;

    /**
     * \brief   Sets the value of the constant.
     * \param   value   The value of the constant.
     **/
    void setValue(const QString& value);

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
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString mValue; //!< The value of the constant.
};

#endif // LUSAN_DATA_COMMON_CONSTANTENTRY_HPP
