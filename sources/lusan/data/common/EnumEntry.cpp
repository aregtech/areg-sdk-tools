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
 *  \file        lusan/data/common/EnumEntry.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Enum Entry.
 *
 ************************************************************************/
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/common/XmlSI.hpp"

EnumEntry::EnumEntry(ElementBase* parent /*= nullptr*/)
    : ElementBase   (parent)
    , mName         ("")
    , mValue        ("")
{
}

EnumEntry::EnumEntry(uint32_t id, const QString& name, const QString& value, ElementBase* parent /*= nullptr*/)
    : ElementBase   (id, parent)
    , mName         (name)
    , mValue        (value)
{
}

EnumEntry::EnumEntry(const EnumEntry& src)
    : ElementBase(src)
    , mName(src.mName)
    , mValue(src.mValue)
{
}

EnumEntry::EnumEntry(EnumEntry&& src) noexcept
    : ElementBase(std::move(src))
    , mName(std::move(src.mName))
    , mValue(std::move(src.mValue))
{
}

EnumEntry& EnumEntry::operator = (const EnumEntry& other)
{
    if (this != &other)
    {
        ElementBase::operator = (other);
        mName = other.mName;
        mValue = other.mValue;
    }

    return *this;
}

EnumEntry& EnumEntry::operator=(EnumEntry&& other) noexcept
{
    if (this != &other)
    {
        ElementBase::operator = (std::move(other));
        mName = std::move(other.mName);
        mValue = std::move(other.mValue);
    }

    return *this;
}

bool EnumEntry::operator == (const EnumEntry& other) const
{
    return (mName == other.mName);
}

bool EnumEntry::operator!=(const EnumEntry& other) const
{
    return (mName != other.mName);
}

bool EnumEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementEnumEntry)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    mName = attributes.value(XmlSI::xmlSIAttributeName).toString();

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementEnumEntry))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementValue)
        {
            mValue = xml.readElementText();
        }

        xml.readNext();
    }

    return true;
}

void EnumEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementEnumEntry);
        xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
        xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
        xml.writeTextElement(XmlSI::xmlSIElementValue, mValue);
    xml.writeEndElement();
}

const QString & EnumEntry::getName() const
{
    return mName;
}

void EnumEntry::setName(const QString& name)
{
    mName = name;
}

const QString& EnumEntry::getValue() const
{
    return mValue;
}

void EnumEntry::setValue(const QString& value)
{
    mValue = value;
}
