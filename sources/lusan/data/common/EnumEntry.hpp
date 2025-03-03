﻿#ifndef LUSAN_DATA_COMMON_ENUMENTRY_HPP
#define LUSAN_DATA_COMMON_ENUMENTRY_HPP
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
 *  \file        lusan/data/common/EnumEntry.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Enum Entry.
 *
 ************************************************************************/

/************************************************************************
 * Include files
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/**
 * \brief   A single entry of the enumeration data type.
 **/
class EnumEntry : public DocumentElem
{
//////////////////////////////////////////////////////////////////////////
// constructors / destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    EnumEntry(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with parameters.
     * \param   id      The ID of the enum entry.
     * \param   name    The name of the enum entry.
     * \param   value   The value of the enum entry.
     * \param   parent  The parent element.
     **/
    EnumEntry(uint32_t id, const QString& name, const QString & value, ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    EnumEntry(const EnumEntry& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    EnumEntry(EnumEntry&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    EnumEntry& operator = (const EnumEntry& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    EnumEntry& operator = (EnumEntry&& other) noexcept;

    /**
     * \brief   Equality operator.
     * \param   other   The other object to compare with.
     * \return  True if the enum entries are equal, false otherwise.
     **/
    bool operator == (const EnumEntry& other) const;

    /**
     * \brief   Inequality operator.
     * \param   other   The other object to compare with.
     * \return  True if the enum entries are not equal, false otherwise.
     **/
    bool operator != (const EnumEntry& other) const;

    /**
     * \brief   Less than operator for sorting.
     * \param   other   The other object to compare with.
     * \return  True if this enum entry is less than the other, false otherwise.
     **/
    bool operator < (const EnumEntry& other) const;

    /**
     * \brief   Greater than operator for sorting.
     * \param   other   The other object to compare with.
     * \return  True if this enum entry is greater than the other, false otherwise.
     **/
    bool operator > (const EnumEntry& other) const;

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
    
    /**
     * \brief   Checks if the parameter is valid.
     * \return  True if the parameter is valid, false otherwise.
     **/
    virtual bool isValid() const override;
    
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

//////////////////////////////////////////////////////////////////////////
// Operations and attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the name of the enum entry.
     * \return  The name of the enum entry.
     **/
    const QString& getName() const;

    /**
     * \brief   Sets the name of the enum entry.
     * \param   name    The new name of the enum entry.
     **/
    void setName(const QString& name);

    /**
     * \brief   Gets the value of the enum entry.
     * \return  The value of the enum entry.
     **/
    const QString & getValue() const;

    /**
     * \brief   Sets the value of the enum entry.
     * \param   value   The new value of the enum entry.
     **/
    void setValue(const QString & value);

    /**
     * \brief   Returns the description of the enum entry.
     **/
    const QString getDescription(void) const;

    /**
     * \brief   Sets the description of the enum entry.
     * \param   describe    The description of the enum entry.
     **/
    void setDescription(const QString& describe);

    /**
     * \brief   Sets the flag, indicating if the enum entry is deprecated.
     * \param   isDeprecated    If true, the enum entry is deprecated.
     **/
    inline void setIsDeprecated(bool isDeprecated);

    /**
     * \brief   Returns true if the enum entry is deprecated.
     **/
    inline bool getIsDeprecated(void) const;
  
    /**
     * \brief   Sets the deprecation hint of the enum entry.
     * \param   hint    The deprecation hint of the enum entry.
     **/
    inline void setDeprecateHint(const QString & hint);

    /**
     * \brief   Returns the deprecation hint of the enum entry.
     **/
    inline const QString& getDeprecateHint(void) const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables.
//////////////////////////////////////////////////////////////////////////
private:
    QString     mName;          //!< The name of the enum entry.
    QString     mValue;         //!< The value of the enum entry.
    QString     mDescription;   //!< The description of the enum entry
    QString     mDeprecateHint; //!< The deprecation hint of the enum entry.
    bool        mIsDeprecated;  //!< Flag, indicating if the enum entry is deprecated.
};

//////////////////////////////////////////////////////////////////////////
// EnumEntry class inline methods.
//////////////////////////////////////////////////////////////////////////

inline void EnumEntry::setIsDeprecated(bool isDeprecated)
{
    mIsDeprecated = isDeprecated;
}

inline bool EnumEntry::getIsDeprecated(void) const
{
    return mIsDeprecated;
}

inline void EnumEntry::setDeprecateHint(const QString & hint)
{
    mDeprecateHint = mIsDeprecated ? hint : QString();
}

inline const QString& EnumEntry::getDeprecateHint(void) const
{
    return (mIsDeprecated ? mDeprecateHint : EmptyString);
}

#endif // LUSAN_DATA_COMMON_ENUMENTRY_HPP
