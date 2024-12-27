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
 *  \file        lusan/data/si/SIIncludeEntry.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Include Entry.
 *
 ************************************************************************/

#include "lusan/data/si/SIIncludeEntry.hpp"
#include "lusan/common/XmlSI.hpp"

SIIncludeEntry::SIIncludeEntry(void)
    : mLocation     ( )
    , mEntryId      (0)
    , mDescription  ( )
    , mDeprecated   (false)
    , mDeprecateHint( )
{
}

SIIncludeEntry::SIIncludeEntry(const QString& path, uint32_t id, const QString& description, bool deprecated, const QString& deprecationHint)
    : mLocation     (path)
    , mEntryId      (id)
    , mDescription  (description)
    , mDeprecated   (deprecated)
    , mDeprecateHint(deprecationHint)
{
}

SIIncludeEntry::SIIncludeEntry(const SIIncludeEntry& other)
    : mLocation     (other.mLocation)
    , mEntryId      (other.mEntryId)
    , mDescription  (other.mDescription)
    , mDeprecated   (other.mDeprecated)
    , mDeprecateHint(other.mDeprecateHint)
{
}

SIIncludeEntry::SIIncludeEntry(SIIncludeEntry&& other) noexcept
    : mLocation     (std::move(other.mLocation))
    , mEntryId      (other.mEntryId)
    , mDescription  (std::move(other.mDescription))
    , mDeprecated   (other.mDeprecated)
    , mDeprecateHint(std::move(other.mDeprecateHint))
{
    other.mEntryId      = 0;
    other.mDeprecated   = false;
}

SIIncludeEntry& SIIncludeEntry::operator=(const SIIncludeEntry& other)
{
    if (this != &other)
    {
        mLocation       = other.mLocation;
        mEntryId        = other.mEntryId;
        mDescription    = other.mDescription;
        mDeprecated     = other.mDeprecated;
        mDeprecateHint  = other.mDeprecateHint;
    }

    return *this;
}

SIIncludeEntry& SIIncludeEntry::operator=(SIIncludeEntry&& other) noexcept
{
    if (this != &other)
    {
        mLocation       = std::move(other.mLocation);
        mEntryId        = other.mEntryId;
        mDescription    = std::move(other.mDescription);
        mDeprecated     = other.mDeprecated;
        mDeprecateHint  = std::move(other.mDeprecateHint);

        other.mEntryId      = 0;
        other.mDeprecated   = false;
    }

    return *this;
}

bool SIIncludeEntry::operator==(const SIIncludeEntry& other) const
{
    return (mLocation == other.mLocation);
}

bool SIIncludeEntry::operator!=(const SIIncludeEntry& other) const
{
    return (mLocation != other.mLocation);
}

bool SIIncludeEntry::isValid(void) const
{
    return !mLocation.isEmpty();
}

const QString& SIIncludeEntry::getLocation(void) const
{
    return mLocation;
}

void SIIncludeEntry::setLocation(const QString& path)
{
    mLocation = path;
}

uint32_t SIIncludeEntry::getId(void) const
{
    return mEntryId;
}

void SIIncludeEntry::setId(uint32_t id)
{
    mEntryId = id;
}

const QString& SIIncludeEntry::getDescription(void) const
{
    return mDescription;
}

void SIIncludeEntry::setDescription(const QString& description)
{
    mDescription = description;
}

bool SIIncludeEntry::isDeprecated(void) const
{
    return mDeprecated;
}

void SIIncludeEntry::setDeprecated(bool deprecated)
{
    mDeprecated = deprecated;
}

const QString& SIIncludeEntry::getDeprecationHint(void) const
{
    return mDeprecateHint;
}

void SIIncludeEntry::setDeprecationHint(const QString& deprecationHint)
{
    mDeprecateHint = deprecationHint;
}

void SIIncludeEntry::deprecateEntry(const QString& deprecationHint)
{
    mDeprecated     = true;
    mDeprecateHint  = deprecationHint;
}

bool SIIncludeEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSI::xmlSIElementLocation)
    {
        return false;
    }

    mEntryId = xml.attributes().value(XmlSI::xmlSIAttributeID).toUInt();
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

void SIIncludeEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementLocation);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(mEntryId));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mLocation);
    xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, mDeprecated ? "true" : "false");

    xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
    xml.writeTextElement(XmlSI::xmlSIElementDeprecateHint, mDeprecateHint);

    xml.writeEndElement();
}
