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
 *  \file        lusan/data/common/AttributeEntry.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Attribute Entry.
 *
 ************************************************************************/
#include "lusan/data/common/AttributeEntry.hpp"
#include "lusan/common/XmlSI.hpp"

const QString AttributeEntry::toString(eNotification value)
{
    switch (value)
    {
    case eNotification::NotifyOnChange:
        return NOTIFY_ONCHANGE;
    case eNotification::NotifyAlways:
        return NOTIFY_ALWAYS;
    default:
        return "";
    }
}

AttributeEntry::eNotification AttributeEntry::fromString(const QString& value)
{
    if (value == NOTIFY_ALWAYS)
    {
        return AttributeEntry::eNotification::NotifyAlways;
    }
    else
    {
        return AttributeEntry::eNotification::NotifyOnChange;
    }
}

AttributeEntry::AttributeEntry(ElementBase* parent /*= nullptr*/)
    : ParamBase(parent)
    , mNotification(eNotification::NotifyOnChange)
{
}

AttributeEntry::AttributeEntry(   uint32_t id
                                , const QString& name
                                , const QString& type           /*= "bool"*/
                                , eNotification notification    /*= DEFAULT_NOTIFICATION*/
                                , bool isDeprecated             /*= false*/
                                , const QString& description    /*= ""*/
                                , const QString& deprecateHint  /*= ""*/
                                , ElementBase* parent           /*= nullptr*/)
    : ParamBase(id, name, type, isDeprecated, description, deprecateHint, parent)
    , mNotification(notification)
{
}

AttributeEntry::AttributeEntry(const AttributeEntry& src)
    : ParamBase(src)
    , mNotification(src.mNotification)
{
}

AttributeEntry::AttributeEntry(AttributeEntry&& src) noexcept
    : ParamBase(std::move(src))
    , mNotification(src.mNotification)
{
}

AttributeEntry& AttributeEntry::operator = (const AttributeEntry& other)
{
    ParamBase::operator = (other);
    mNotification = other.mNotification;
    return *this;
}

AttributeEntry& AttributeEntry::operator = (AttributeEntry&& other) noexcept
{
    ParamBase::operator = (std::move(other));
    mNotification = other.mNotification;
    return *this;
}

bool AttributeEntry::operator == (const AttributeEntry& other) const
{
    return ParamBase::operator == (other);
}

bool AttributeEntry::operator!=(const AttributeEntry& other) const
{
    return !(*this == other);
}

bool AttributeEntry::operator > (const AttributeEntry& other) const
{
    return getName() > other.getName();
}

bool AttributeEntry::operator < (const AttributeEntry& other) const
{
    return getName() < other.getName();
}

AttributeEntry::eNotification AttributeEntry::getNotification() const
{
    return mNotification;
}

void AttributeEntry::setNotification(eNotification notification)
{
    mNotification = notification;
}

void AttributeEntry::setNotification(const QString& notification)
{
    if (notification == NOTIFY_ONCHANGE)
    {
        mNotification = eNotification::NotifyOnChange;
    }
    else if (notification == NOTIFY_ALWAYS)
    {
        mNotification = eNotification::NotifyAlways;
    }
}

bool AttributeEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementAttribute)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    setName(attributes.value(XmlSI::xmlSIAttributeName).toString());
    setType(attributes.value(XmlSI::xmlSIAttributeDataType).toString());

    if (attributes.hasAttribute(XmlSI::xmlSIAttributeNotify))
    {
        QString notifyValue = attributes.value(XmlSI::xmlSIAttributeNotify).toString();
        mNotification = fromString(notifyValue);
    }

    setIsDeprecated(attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() == XmlSI::xmlSIValueTrue : false);
    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementAttribute))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            QStringView xmlName = xml.name();
            if (xmlName == XmlSI::xmlSIElementDescription)
            {
                setDescription(xml.readElementText());
            }
            else if (xmlName == XmlSI::xmlSIElementDeprecateHint)
            {
                setDeprecateHint(xml.readElementText());
            }
        }

        xml.readNext();
    }

    return true;
}

void AttributeEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementAttribute);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    xml.writeAttribute(XmlSI::xmlSIAttributeDataType, mParamType.getName());
    xml.writeAttribute(XmlSI::xmlSIAttributeNotify, toString(mNotification));
    if (getIsDeprecated())
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
    }

    xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
    if (getIsDeprecated())
    {
        xml.writeTextElement(XmlSI::xmlSIElementDeprecateHint, getDeprecateHint());
    }

    xml.writeEndElement();
}

