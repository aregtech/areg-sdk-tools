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

EnumEntry::EnumEntry(void)
    : mId   (0)
    , mName ("")
    , mValue("")
{
}

EnumEntry::EnumEntry(uint32_t id, const QString& name, const QString& value)
    : mId   (id)
    , mName (name)
    , mValue(value)
{
}

bool EnumEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementEnumEntry)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    if (attributes.hasAttribute(XmlSI::xmlSIAttributeID))
    {
        mId = attributes.value(XmlSI::xmlSIAttributeID).toInt();
    }

    if (attributes.hasAttribute(XmlSI::xmlSIAttributeName))
    {
        mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
    }

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
        xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(mId));
        xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
        xml.writeTextElement(XmlSI::xmlSIElementValue, mValue);
    xml.writeEndElement();
}

uint32_t EnumEntry::getId() const
{
    return mId;
}

void EnumEntry::setId(uint32_t id)
{
    mId = id;
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
