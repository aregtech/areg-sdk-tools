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
 *  \copyright   � 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/common/ConstantEntry.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, ConstantEntry.
 *
 ************************************************************************/
#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/common/XmlSI.hpp"

ConstantEntry::ConstantEntry(void)
    : ParamBase()
    , mValue()
{
}

ConstantEntry::ConstantEntry(uint32_t id, const QString& name, const QString& type, const QString& value, bool isDeprecated, const QString& description, const QString& deprecateHint)
    : ParamBase(id, name, type, isDeprecated, description, deprecateHint)
    , mValue(value)
{
}

ConstantEntry::ConstantEntry(const ConstantEntry& src)
    : ParamBase(src)
    , mValue(src.mValue)
{
}

ConstantEntry::ConstantEntry(ConstantEntry&& src) noexcept
    : ParamBase(std::move(src))
    , mValue(std::move(src.mValue))
{
}

ConstantEntry& ConstantEntry::operator = (const ConstantEntry& other)
{
    if (this != &other)
    {
        ParamBase::operator = (other);
        mValue = other.mValue;
    }

    return *this;
}

ConstantEntry& ConstantEntry::operator = (ConstantEntry&& other) noexcept
{
    if (this != &other)
    {
        ParamBase::operator = (std::move(other));
        mValue = std::move(other.mValue);
    }

    return *this;
}

bool ConstantEntry::operator == (const ConstantEntry& other) const
{
    return (this != &other ? (mName == other.mName) && (mType == other.mType) : true);
}

bool ConstantEntry::operator != (const ConstantEntry& other) const
{
    return (this != &other ? (mName != other.mName) || (mType != other.mType) : false);
}

bool ConstantEntry::operator < (const ConstantEntry& other) const
{
    return getName() < other.getName();
}

bool ConstantEntry::operator > (const ConstantEntry& other) const
{
    return getName() > other.getName();
}

QString ConstantEntry::getValue() const
{
    return mValue;
}

void ConstantEntry::setValue(const QString& value)
{
    mValue = value;
}

bool ConstantEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementConstant)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.hasAttribute(XmlSI::xmlSIAttributeID)          ? attributes.value(XmlSI::xmlSIAttributeID).toUInt()        : 0);
    setName(attributes.hasAttribute(XmlSI::xmlSIAttributeName)      ? attributes.value(XmlSI::xmlSIAttributeName).toString()    : "");
    setType(attributes.hasAttribute(XmlSI::xmlSIAttributeDataType)  ? attributes.value(XmlSI::xmlSIAttributeDataType).toString(): "");
    setDeprecated(attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() == "true" : false);

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementConstant))
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

void ConstantEntry::writeToXml(QXmlStreamWriter& xml) const
{
    if (isValid())
    {
        xml.writeStartElement(XmlSI::xmlSIElementConstant);
        xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(mId));
        xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
        xml.writeAttribute(XmlSI::xmlSIAttributeDataType, mType);
        if (mIsDeprecated)
        {
            xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, "true");
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