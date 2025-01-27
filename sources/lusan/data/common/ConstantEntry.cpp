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
 *  \file        lusan/data/common/ConstantEntry.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, ConstantEntry.
 *
 ************************************************************************/
#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/common/XmlSI.hpp"

ConstantEntry::ConstantEntry(ElementBase* parent /*= nullptr*/)
    : ParamBase (parent)
    , mValue    ()
{
}

ConstantEntry::ConstantEntry(uint32_t id, const QString & name, ElementBase* parent /*= nullptr*/)
    : ParamBase(id, name, "bool", false, "", "", parent)
    , mValue   (XmlSI::xmlSIValueTrue)
{
}


ConstantEntry::ConstantEntry( uint32_t id
                            , const QString& name
                            , const QString& type           /*= "bool"*/
                            , const QString& value          /*= "true"*/
                            , bool isDeprecated             /*= false*/
                            , const QString& description    /*= ""*/
                            , const QString& deprecateHint  /*= ""*/
                            , ElementBase* parent           /*= nullptr*/)
    : ParamBase(id, name, type, isDeprecated, description, deprecateHint, parent)
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
    return ParamBase::operator == (other);
}

bool ConstantEntry::operator != (const ConstantEntry& other) const
{
    return ParamBase::operator != (other);
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
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    setName(attributes.value(XmlSI::xmlSIAttributeName).toString());
    setType(attributes.value(XmlSI::xmlSIAttributeDataType).toString());
    setIsDeprecated(attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() == XmlSI::xmlSIValueTrue : false);

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementConstant))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            QStringView xmlName{ xml.name() };
            if (xmlName == XmlSI::xmlSIElementValue)
            {
                mValue = xml.readElementText();
            }
            else if (xmlName == XmlSI::xmlSIElementDescription)
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

void ConstantEntry::writeToXml(QXmlStreamWriter& xml) const
{
    if (isValid())
    {
        xml.writeStartElement(XmlSI::xmlSIElementConstant);
        xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
        xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
        xml.writeAttribute(XmlSI::xmlSIAttributeDataType, mParamType.getName());
        if (getIsDeprecated())
        {
            xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
        }

        xml.writeTextElement(XmlSI::xmlSIElementValue, mValue);
        xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
        if (getIsDeprecated())
        {
            xml.writeTextElement(XmlSI::xmlSIElementDeprecateHint, getDeprecateHint());
        }

        xml.writeEndElement();
    }
}

QIcon ConstantEntry::getIcon(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return QIcon::fromTheme(QIcon::ThemeIcon::InputGaming);
    case ElementBase::eDisplay::DisplayType:
        return (mParamType.isValid() ? QIcon() : QIcon::fromTheme(QIcon::ThemeIcon::DialogWarning));
    default:
        return QIcon();
    }
}

QString ConstantEntry::getString(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return getName();
    case ElementBase::eDisplay::DisplayType:
        return getType();
    case ElementBase::eDisplay::DisplayValue:
        return mValue;
    default:
        return QString();
    }
}
