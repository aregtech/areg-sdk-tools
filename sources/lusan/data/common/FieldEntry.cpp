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
 *  \file        lusan/data/common/FieldEntry.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FieldEntry.
 *
 ************************************************************************/
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/common/XmlSI.hpp"

FieldEntry::FieldEntry(ElementBase* parent /*= nullptr*/)
    : ParamBase(parent)
    , mValue()
{
}

FieldEntry::FieldEntry(   uint32_t id
                        , const QString& name
                        , const QString& type
                        , const QString& value
                        , bool isDeprecated
                        , const QString& description
                        , const QString& deprecateHint
                        , ElementBase* parent /*= nullptr*/)
    : ParamBase(id, name, type, isDeprecated, description, deprecateHint, parent)
    , mValue(value)
{
}

FieldEntry::FieldEntry(const FieldEntry& src)
    : ParamBase(src)
    , mValue(src.mValue)
{
}

FieldEntry::FieldEntry(FieldEntry&& src) noexcept
    : ParamBase(std::move(src))
    , mValue(std::move(src.mValue))
{
}

FieldEntry& FieldEntry::operator = (const FieldEntry& other)
{
    if (this != &other)
    {
        ParamBase::operator = (other);
        mValue = other.mValue;
    }

    return *this;
}

FieldEntry& FieldEntry::operator = (FieldEntry&& other) noexcept
{
    if (this != &other)
    {
        ParamBase::operator = (std::move(other));
        mValue = std::move(other.mValue);
    }

    return *this;
}

bool FieldEntry::operator == (const FieldEntry& other) const
{
    return (mName == other.mName);
}

bool FieldEntry::operator != (const FieldEntry& other) const
{
    return (mName != other.mName);
}

bool FieldEntry::operator < (const FieldEntry& other) const
{
    return getName() < other.getName();
}

bool FieldEntry::operator > (const FieldEntry& other) const
{
    return getName() > other.getName();
}

QString FieldEntry::getValue() const
{
    return mValue;
}

void FieldEntry::setValue(const QString& value)
{
    mValue = value;
}

bool FieldEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementField)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    setName(attributes.value(XmlSI::xmlSIAttributeName).toString());
    setType(attributes.value(XmlSI::xmlSIAttributeDataType).toString());
    setDeprecated(attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() == XmlSI::xmlSIValueTrue : false);

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementField))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSI::xmlSIElementValue)
            {
                mValue = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementDescription)
            {
                setDescription(xml.readElementText());
            }
            else if (xml.name() == XmlSI::xmlSIElementDeprecateHint)
            {
                setDeprecateHint(xml.readElementText());
            }
        }

        xml.readNext();
    }

    return true;
}

void FieldEntry::writeToXml(QXmlStreamWriter& xml) const
{
    if (isValid())
    {
        xml.writeStartElement(XmlSI::xmlSIElementField);
        xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
        xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
        xml.writeAttribute(XmlSI::xmlSIAttributeDataType, mType);
        if (mIsDeprecated)
        {
            xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
        }

        xml.writeTextElement(XmlSI::xmlSIElementValue, mValue);
        xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
        if (mIsDeprecated && (mDeprecateHint.isEmpty() == false))
        {
            xml.writeTextElement(XmlSI::xmlSIElementDeprecateHint, mDeprecateHint);
        }

        xml.writeEndElement();
    }
}
