#ifndef LUSAN_DATA_COMMON_FIELDENTRY_HPP
#define LUSAN_DATA_COMMON_FIELDENTRY_HPP
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
 *  \file        lusan/data/common/FieldEntry.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FieldEntry.
 *
 ************************************************************************/

#include "lusan/data/common/ParamBase.hpp"

/**
 * \class   FieldEntry
 * \brief   Represents a field in the Lusan application.
 **/
class FieldEntry : public ParamBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    FieldEntry(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the field.
     * \param   name            The name of the field.
     * \param   type            The data type name of the field.
     * \param   value           The value of the field.
     * \param   isDeprecated    The deprecated flag of the field.
     * \param   description     The description of the field.
     * \param   deprecateHint   The deprecation hint of the field.
     **/
    FieldEntry(uint32_t id, const QString& name, const QString& type, const QString& value, bool isDeprecated, const QString& description, const QString& deprecateHint, ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    FieldEntry(const FieldEntry& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    FieldEntry(FieldEntry&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    FieldEntry& operator = (const FieldEntry& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    FieldEntry& operator = (FieldEntry&& other) noexcept;

    /**
     * \brief   Equality operator.
     * \param   other   The other object to compare with.
     * \return  True if the fields are equal, false otherwise.
     **/
    bool operator == (const FieldEntry& other) const;

    /**
     * \brief   Inequality operator.
     * \param   other   The other object to compare with.
     * \return  True if the fields are not equal, false otherwise.
     **/
    bool operator != (const FieldEntry& other) const;

    /**
     * \brief   Less than operator for sorting.
     * \param   other   The other object to compare with.
     * \return  True if this field is less than the other, false otherwise.
     **/
    bool operator < (const FieldEntry& other) const;

    /**
     * \brief   Greater than operator for sorting.
     * \param   other   The other object to compare with.
     * \return  True if this field is greater than the other, false otherwise.
     **/
    bool operator > (const FieldEntry& other) const;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the value of the field.
     * \return  The value of the field.
     **/
    QString getValue() const;

    /**
     * \brief   Sets the value of the field.
     * \param   value   The value of the field.
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
    QString mValue; //!< The value of the field.
};

#endif // LUSAN_DATA_COMMON_FIELDENTRY_HPP
