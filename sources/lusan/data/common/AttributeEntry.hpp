#ifndef LUSAN_DATA_COMMON_ATTRIBUTEENTRY_HPP
#define LUSAN_DATA_COMMON_ATTRIBUTEENTRY_HPP
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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/common/AttributeEntry.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Attribute Entry.
 *
 ************************************************************************/

#include "lusan/data/common/ParamBase.hpp"

/**
 * \brief   What an attribute carries beyond a name, a type and a description, decided by the
 *          document that owns the section:
 *          - `hasValue`        : the attribute carries a default value. State machine attributes
 *                                have one, service interface attributes do not.
 *          - `hasNotification` : the attribute carries an update notification kind. Service
 *                                interface attributes have one, state machine attributes do not.
 *
 *          Service interface: `{ hasValue = false, hasNotification = true }`.
 *          State machine    : `{ hasValue = true , hasNotification = false }`.
 *
 *          The entry keeps a copy of its section's setting, so it writes the same shape wherever
 *          it is serialized -- the document, the clipboard, or a paste fragment.
 **/
struct AttributeConfig
{
    bool hasValue;          //!< The attribute carries a default value.
    bool hasNotification;   //!< The attribute carries an update notification kind.
};

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
    //!< What an entry carries until a section stamps it: everything it holds, so a stray entry
    //!< never silently drops a field on the way to a file.
    static constexpr AttributeConfig DEFAULT_CONFIG { true, true };
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
     * \param   parent          The parent object.
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
     * \brief   Constructor with initialization.
     * \param   id              The ID of the attribute.
     * \param   name            The name of the attribute.
     * \param   notification    The notification type of the attribute.
     * \param   parent          The parent object.
     **/
    AttributeEntry(   uint32_t id
                    , const QString& name
                    , eNotification notification    = DEFAULT_NOTIFICATION
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
     * \brief   Returns what this attribute carries besides name, type and description.
     **/
    const AttributeConfig& getConfig() const;

    /**
     * \brief   Sets what this attribute carries. The section stamps every entry it creates or
     *          reads, so an entry written anywhere keeps its document's shape.
     **/
    void setConfig(const AttributeConfig& config);

    /**
     * \brief   Gets the default value of the attribute. Empty in a document whose attributes
     *          carry no value.
     **/
    const QString& getValue() const;

    /**
     * \brief   Sets the default value of the attribute.
     **/
    void setValue(const QString& value);

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
    bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes data to an XML stream, in the shape this entry was stamped with.
     * \param   xml     The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Writes data to an XML stream in the shape the caller asks for. The section that
     *          holds the entry uses this, so what reaches the file is the document's shape even
     *          if the entry itself was never stamped.
     * \param   xml     The XML stream writer.
     * \param   config  What to write besides name, type, description and deprecation.
     **/
    void writeToXml(QXmlStreamWriter& xml, const AttributeConfig& config) const;

    /**
     * \brief Returns the icon to display for specific display type.
     * \param display   The classification to display.
     */
    QIcon getIcon(ElementBase::eDisplay display) const override;

    /**
     * \brief Returns the string to display for specific display type.
     * \param display   The classification to display.
     */
    QString getString(ElementBase::eDisplay display) const override;

    /**
     * \brief   Checks if the attribute is valid.
     **/
    bool isValid() const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    eNotification   mNotification;  //!< The notification type of the attribute.
    QString         mValue;         //!< The default value of the attribute.
    AttributeConfig mConfig;        //!< What this attribute carries, from the section it lives in.
};

#endif // LUSAN_DATA_COMMON_ATTRIBUTEENTRY_HPP

