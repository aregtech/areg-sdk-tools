/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/common/EnumEntry.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Enum Entry.
 *
 ************************************************************************/
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/common/XmlSI.hpp"

EnumEntry::EnumEntry(ElementBase* parent /*= nullptr*/)
    : DocumentElem(parent)
    , mName         ( )
    , mValue        ( )
    , mDescription  ( )
    , mDeprecateHint( )
    , mIsDeprecated (false)
{
}

EnumEntry::EnumEntry(uint32_t id, const QString& name, const QString& value, ElementBase* parent /*= nullptr*/)
    : DocumentElem   (id, parent)
    , mName         (name)
    , mValue        (value)
    , mDescription  ( )
    , mDeprecateHint( )
    , mIsDeprecated (false)
{
}

EnumEntry::EnumEntry(const EnumEntry& src)
    : DocumentElem   (src)
    , mName         (src.mName)
    , mValue        (src.mValue)
    , mDescription  (src.mDescription)
    , mDeprecateHint(src.mDeprecateHint)
    , mIsDeprecated (src.mIsDeprecated)
{
}

EnumEntry::EnumEntry(EnumEntry&& src) noexcept
    : DocumentElem   (std::move(src))
    , mName         (std::move(src.mName))
    , mValue        (std::move(src.mValue))
    , mDescription  (std::move(src.mDescription))
    , mDeprecateHint(std::move(src.mDeprecateHint))
    , mIsDeprecated (src.mIsDeprecated)
{
}

EnumEntry& EnumEntry::operator = (const EnumEntry& other)
{
    if (this != &other)
    {
        DocumentElem::operator = (other);
        mName           = other.mName;
        mValue          = other.mValue;
        mDescription    = other.mDescription;
        mDeprecateHint  = other.mDeprecateHint;
        mIsDeprecated   = other.mIsDeprecated;

    }

    return *this;
}

EnumEntry& EnumEntry::operator=(EnumEntry&& other) noexcept
{
    if (this != &other)
    {
        DocumentElem::operator = (std::move(other));
        mName           = std::move(other.mName);
        mValue          = std::move(other.mValue);
        mDescription    = std::move(other.mDescription);
        mDeprecateHint  = std::move(other.mDeprecateHint);
        mIsDeprecated   = std::move(other.mIsDeprecated);
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

    QString depValue = attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() : "";
    setIsDeprecated(depValue.compare(XmlSI::xmlSIValueTrue, Qt::CaseSensitivity::CaseInsensitive) == 0);

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementEnumEntry))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSI::xmlSIElementValue)
            {
                mValue = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementDescription)
            {
                mDescription = xml.readElementText();
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

void EnumEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementEnumEntry);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    if (getIsDeprecated())
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
        writeTextElem(xml, XmlSI::xmlSIElementDeprecateHint, getDeprecateHint(), true);
    }

    writeTextElem(xml, XmlSI::xmlSIElementValue, mValue, true);
    writeTextElem(xml, XmlSI::xmlSIElementDescription, mDescription, false);

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

const QString EnumEntry::getDescription(void) const
{
    return mDescription;
}

void EnumEntry::setDescription(const QString& describe)
{
    mDescription = describe;
}

bool EnumEntry::isValid() const
{
    return getName().isEmpty() == false;
}

QIcon EnumEntry::getIcon(ElementBase::eDisplay display) const
{
    if (display == ElementBase::eDisplay::DisplayName)
    {
        return NELusanCommon::iconEnumField(NELusanCommon::SizeSmall);
    }
    else
    {
        return QIcon();
    }
}

QString EnumEntry::getString(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return getName();
    case ElementBase::eDisplay::DisplayValue:
        return mValue;
    default:
        return QString();
    }
}
