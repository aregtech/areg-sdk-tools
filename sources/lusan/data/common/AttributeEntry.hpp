#ifndef LUSAN_DATA_COMMON_ATTRIBUTEENTRY_HPP
#define LUSAN_DATA_COMMON_ATTRIBUTEENTRY_HPP
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
 *  \file        lusan/data/common/AttributeEntry.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Attribute Entry.
 *
 ************************************************************************/

#include "lusan/data/common/ParamBase.hpp"

/**
 * \class   AttributeEntry
 * \brief   Represents an attribute in the Lusan application.
 **/
class AttributeEntry : public ParamBase
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eNotification
     * \brief   Represents the notification type of the attribute.
     **/
    typedef enum class E_Notification
    {
          NotifyOnChange    //!< NotifyOnChange, notify only when value has been changed.
        , NotifyAlways      //!< NotifyAlways, notify always when value has been set.
    } eNotification;

    /**
     * \brief   Converts notification type to string value.
     * \param   value   The notification type to convert.
     * \return  The string value of the notification type.
     **/
    static const QString toString(eNotification value);

    /**
     * \brief   Converts string value to notification type.
     * \param   value   The string value to convert.
     * \return  The notification type converted from string value.
     **/
    static eNotification fromString(const QString& value);

private:
    static constexpr eNotification DEFAULT_NOTIFICATION { eNotification::NotifyOnChange };  //!< The default notification type of the attribute.
    static constexpr const char* const NOTIFY_ONCHANGE  { "OnChange" }; //!< The string value of the notification type.
    static constexpr const char* const NOTIFY_ALWAYS    { "Always" };   //!< The string value of the notification type.

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    AttributeEntry(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the attribute.
     * \param   name            The name of the attribute.
     * \param   type            The data type name of the attribute.
     * \param   notification    The notification type of the attribute.
     * \param   isDeprecated    The deprecated flag of the attribute.
     * \param   description     The description of the attribute.
     * \param   deprecateHint   The deprecation hint of the attribute.
     **/
    AttributeEntry(   uint32_t id
                    , const QString& name
                    , const QString& type           = "bool"
                    , eNotification notification    = DEFAULT_NOTIFICATION
                    , bool isDeprecated             = false
                    , const QString& description    = ""
                    , const QString& deprecateHint  = ""
                    , ElementBase* parent           = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    AttributeEntry(const AttributeEntry& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    AttributeEntry(AttributeEntry&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    AttributeEntry& operator=(const AttributeEntry& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    AttributeEntry& operator=(AttributeEntry&& other) noexcept;

    /**
     * \brief   Equality operator.
     * \param   other   The other object to compare with.
     * \return  True if the attributes are equal, false otherwise.
     **/
    bool operator==(const AttributeEntry& other) const;

    /**
     * \brief   Inequality operator.
     * \param   other   The other object to compare with.
     * \return  True if the attributes are not equal, false otherwise.
     **/
    bool operator!=(const AttributeEntry& other) const;

    /**
     * \brief   Greater operator.
     * \param   other   The other object to compare with.
     * \return  True if the attribute is greater than the other, false otherwise.
     **/
    bool operator > (const AttributeEntry& other) const;

    /**
     * \brief   Less than operator.
     * \param   other   The other object to compare with.
     * \return  True if the attribute is less than the other, false otherwise.
     **/
    bool operator < (const AttributeEntry& other) const;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the notification type of the attribute.
     * \return  The notification type of the attribute.
     **/
    eNotification getNotification() const;

    /**
     * \brief   Sets the notification type of the attribute.
     * \param   notification    The notification type of the attribute.
     **/
    void setNotification(eNotification notification);

    /**
     * \brief   Sets the notification type of the attribute.
     * \param   notification    The notification type of the attribute as a string value.
     **/
    void setNotification(const QString& notification);

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
    eNotification mNotification; //!< The notification type of the attribute.
};

#endif // LUSAN_DATA_COMMON_ATTRIBUTEENTRY_HPP

