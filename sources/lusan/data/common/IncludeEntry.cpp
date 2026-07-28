/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/common/IncludeEntry.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Include Entry.
 *
 ************************************************************************/

#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/common/XmlSI.hpp"
#include "IncludeEntry.hpp"

#include <QFileInfo>

eIncludeKind includeKindOf(const QString& location, const QString& docExtension)
{
    const QString suffix = QFileInfo(location).suffix().toLower();
    if (suffix == docExtension.toLower())
    {
        return eIncludeKind::Document;
    }
    else if (suffix == QStringLiteral("dtml"))
    {
        return eIncludeKind::DataType;
    }
    else
    {
        return eIncludeKind::Source;
    }
}

IncludeEntry::IncludeEntry(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mLocation     ( )
    , mDescription  ( )
    , mDeprecated   (false)
    , mDeprecateHint( )
    , mAlias        ( )
    , mVersion      ( )
{
}

IncludeEntry::IncludeEntry(uint32_t id, const QString & location, ElementBase * parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mLocation     ( location )
    , mDescription  ( )
    , mDeprecated   (false)
    , mDeprecateHint( )
    , mAlias        ( )
    , mVersion      ( )
{
    
}    

IncludeEntry::IncludeEntry(const QString& path, uint32_t id, const QString& description, bool deprecated, const QString& deprecationHint, ElementBase* parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mLocation     (path)
    , mDescription  (description)
    , mDeprecated   (deprecated)
    , mDeprecateHint(deprecationHint)
    , mAlias        ( )
    , mVersion      ( )
{
}

IncludeEntry::IncludeEntry(const IncludeEntry& other)
    : DocumentElem  (other)
    , mLocation     (other.mLocation)
    , mDescription  (other.mDescription)
    , mDeprecated   (other.mDeprecated)
    , mDeprecateHint(other.mDeprecateHint)
    , mAlias        (other.mAlias)
    , mVersion      (other.mVersion)
{
}

IncludeEntry::IncludeEntry(IncludeEntry&& other) noexcept
    : DocumentElem  (std::move(other))
    , mLocation     (std::move(other.mLocation))
    , mDescription  (std::move(other.mDescription))
    , mDeprecated   (other.mDeprecated)
    , mDeprecateHint(std::move(other.mDeprecateHint))
    , mAlias        (std::move(other.mAlias))
    , mVersion      (other.mVersion)
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
        mAlias          = other.mAlias;
        mVersion        = other.mVersion;
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
        mAlias          = std::move(other.mAlias);
        mVersion        = other.mVersion;

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

const QString& IncludeEntry::getLocation() const
{
    return mLocation;
}

void IncludeEntry::setLocation(const QString& path)
{
    mLocation = path;
}

const QString& IncludeEntry::getAlias() const
{
    return mAlias;
}

void IncludeEntry::setAlias(const QString& alias)
{
    mAlias = alias;
}

const VersionNumber& IncludeEntry::getVersion() const
{
    return mVersion;
}

void IncludeEntry::setVersion(const VersionNumber& version)
{
    mVersion = version;
}

const QString& IncludeEntry::getDescription() const
{
    return mDescription;
}

void IncludeEntry::setDescription(const QString& description)
{
    mDescription = description;
}

bool IncludeEntry::getIsDeprecated() const
{
    return mDeprecated;
}

void IncludeEntry::setIsDeprecated(bool deprecated)
{
    mDeprecated = deprecated;
}

const QString& IncludeEntry::getDeprecateHint() const
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
    mAlias    = attributes.value(XmlSI::xmlSIAttributeAlias).toString();
    mVersion  = VersionNumber(attributes.value(XmlSI::xmlSIAttributeVersion).toString());

    QString depValue = attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() : "";
    setIsDeprecated(depValue.compare(XmlSI::xmlSIValueTrue, Qt::CaseSensitivity::CaseInsensitive) == 0);

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementLocation))
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
    // Only an imported state machine carries these two, so a .siml file never grows them. The
    // alias is what marks the entry as an import: without one there is nothing to pin a version to.
    if (mAlias.isEmpty() == false)
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeAlias, mAlias);
        if (mVersion.isValid())
        {
            xml.writeAttribute(XmlSI::xmlSIAttributeVersion, mVersion.toString());
        }
    }

    if (getIsDeprecated())
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
        writeTextElem(xml, XmlSI::xmlSIElementDeprecateHint, getDeprecateHint(), true);
    }

    writeTextElem(xml, XmlSI::xmlSIElementDescription, mDescription, false);
    xml.writeEndElement();
}

bool IncludeEntry::isValid() const
{
    return !mLocation.isEmpty();
}

QIcon IncludeEntry::getIcon(ElementBase::eDisplay display) const
{
    return (display == ElementBase::eDisplay::DisplayName ? NELusanCommon::iconInclude(NELusanCommon::SizeSmall) : QIcon());
}

QString IncludeEntry::getString(ElementBase::eDisplay display) const
{
    return (display == ElementBase::eDisplay::DisplayName ? mLocation : QString());
}
