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
    : ElementBase   (parent)
    , mLocation     ( )
    , mDescription  ( )
    , mDeprecated   (false)
    , mDeprecateHint( )
{
}

IncludeEntry::IncludeEntry(uint32_t id, const QString & location, ElementBase * parent /*= nullptr*/)
    : ElementBase   (id, parent)
    , mLocation     ( location )
    , mDescription  ( )
    , mDeprecated   (false)
    , mDeprecateHint( )
{
    
}    

IncludeEntry::IncludeEntry(const QString& path, uint32_t id, const QString& description, bool deprecated, const QString& deprecationHint, ElementBase* parent /*= nullptr*/)
    : ElementBase   (id, parent)
    , mLocation     (path)
    , mDescription  (description)
    , mDeprecated   (deprecated)
    , mDeprecateHint(deprecationHint)
{
}

IncludeEntry::IncludeEntry(const IncludeEntry& other)
    : ElementBase   (other)
    , mLocation     (other.mLocation)
    , mDescription  (other.mDescription)
    , mDeprecated   (other.mDeprecated)
    , mDeprecateHint(other.mDeprecateHint)
{
}

IncludeEntry::IncludeEntry(IncludeEntry&& other) noexcept
    : ElementBase(std::move(other))
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
        ElementBase::operator = (other);
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
        ElementBase::operator = (std::move(other));
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

bool IncludeEntry::isValid(void) const
{
    return !mLocation.isEmpty();
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

bool IncludeEntry::isDeprecated(void) const
{
    return mDeprecated;
}

void IncludeEntry::setIsDeprecated(bool deprecated)
{
    mDeprecated = deprecated;
}

const QString& IncludeEntry::getDeprecationHint(void) const
{
    return mDeprecateHint;
}

void IncludeEntry::setDeprecationHint(const QString& deprecationHint)
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

    setId(xml.attributes().value(XmlSI::xmlSIAttributeID).toUInt());
    mLocation = xml.attributes().value(XmlSI::xmlSIAttributeName).toString();
    mDeprecated = xml.attributes().value(XmlSI::xmlSIAttributeIsDeprecated).toString() == "true";

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
                mDeprecateHint = xml.readElementText();
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
    if (mDeprecated)
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, "true");
    }

    xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
    if (mDeprecated && (mDeprecateHint.isEmpty() == false))
    {
        xml.writeTextElement(XmlSI::xmlSIElementDeprecateHint, mDeprecateHint);
    }

    xml.writeEndElement();
}
