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
 *  \file        lusan/data/common/IncludeEntry.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Include Entry.
 *
 ************************************************************************/

#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/common/XmlSI.hpp"
#include "IncludeEntry.hpp"

IncludeEntry::IncludeEntry(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mLocation     ( )
    , mDescription  ( )
    , mDeprecated   (false)
    , mDeprecateHint( )
{
}

IncludeEntry::IncludeEntry(uint32_t id, const QString & location, ElementBase * parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mLocation     ( location )
    , mDescription  ( )
    , mDeprecated   (false)
    , mDeprecateHint( )
{
    
}    

IncludeEntry::IncludeEntry(const QString& path, uint32_t id, const QString& description, bool deprecated, const QString& deprecationHint, ElementBase* parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mLocation     (path)
    , mDescription  (description)
    , mDeprecated   (deprecated)
    , mDeprecateHint(deprecationHint)
{
}

IncludeEntry::IncludeEntry(const IncludeEntry& other)
    : DocumentElem  (other)
    , mLocation     (other.mLocation)
    , mDescription  (other.mDescription)
    , mDeprecated   (other.mDeprecated)
    , mDeprecateHint(other.mDeprecateHint)
{
}

IncludeEntry::IncludeEntry(IncludeEntry&& other) noexcept
    : DocumentElem  (std::move(other))
    , mLocation     (std::move(other.mLocation))
    , mDescription  (std::move(other.mDescription))
    , mDeprecated   (other.mDeprecated)
    , mDeprecateHint(std::move(other.mDeprecateHint))
{
    other.mDeprecated   = false;
}

IncludeEntry& IncludeEntry::operator = (const IncludeEntry& other)
{
    if (this != &other)
    {
        DocumentElem::operator = (other);
        mLocation       = other.mLocation;
        mDescription    = other.mDescription;
        mDeprecated     = other.mDeprecated;
        mDeprecateHint  = other.mDeprecateHint;
    }

    return *this;
}

IncludeEntry& IncludeEntry::operator=(IncludeEntry&& other) noexcept
{
    if (this != &other)
    {
        DocumentElem::operator = (std::move(other));
        mLocation       = std::move(other.mLocation);
        mDescription    = std::move(other.mDescription);
        mDeprecated     = other.mDeprecated;
        mDeprecateHint  = std::move(other.mDeprecateHint);

        other.mDeprecated   = false;
    }

    return *this;
}

bool IncludeEntry::operator==(const IncludeEntry& other) const
{
    return (mLocation == other.mLocation);
}

bool IncludeEntry::operator!=(const IncludeEntry& other) const
{
    return (mLocation != other.mLocation);
}

bool IncludeEntry::operator < (const IncludeEntry& other) const
{
    return (mLocation < other.mLocation);
}

bool IncludeEntry::operator > (const IncludeEntry& other) const
{
    return (mLocation > other.mLocation);
}

const QString& IncludeEntry::getLocation(void) const
{
    return mLocation;
}

void IncludeEntry::setLocation(const QString& path)
{
    mLocation = path;
}

const QString& IncludeEntry::getDescription(void) const
{
    return mDescription;
}

void IncludeEntry::setDescription(const QString& description)
{
    mDescription = description;
}

bool IncludeEntry::getIsDeprecated(void) const
{
    return mDeprecated;
}

void IncludeEntry::setIsDeprecated(bool deprecated)
{
    mDeprecated = deprecated;
}

const QString& IncludeEntry::getDeprecateHint(void) const
{
    return mDeprecateHint;
}

void IncludeEntry::setDeprecateHint(const QString& deprecationHint)
{
    mDeprecateHint = deprecationHint;
}

void IncludeEntry::deprecateEntry(const QString& deprecationHint)
{
    mDeprecated     = true;
    mDeprecateHint  = deprecationHint;
}

bool IncludeEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSI::xmlSIElementLocation)
    {
        return false;
    }
    
    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    mLocation = attributes.value(XmlSI::xmlSIAttributeName).toString();
    setIsDeprecated(attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() == XmlSI::xmlSIValueTrue : false);

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementLocation))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSI::xmlSIElementDescription)
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

void IncludeEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementLocation);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mLocation);
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

bool IncludeEntry::isValid(void) const
{
    return !mLocation.isEmpty();
}

QIcon IncludeEntry::getIcon(ElementBase::eDisplay display) const
{
    return (display == ElementBase::eDisplay::DisplayName ? QIcon::fromTheme(QIcon::ThemeIcon::ImageLoading) : QIcon());
}

QString IncludeEntry::getString(ElementBase::eDisplay display) const
{
    return (display == ElementBase::eDisplay::DisplayName ? mLocation : QString());
}
