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
 *  \file        lusan/data/common/OverviewDataSection.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Overview section shared by every document.
 *
 ************************************************************************/

#include "lusan/data/common/OverviewDataSection.hpp"
#include "lusan/common/XmlSI.hpp"

OverviewDataSection::OverviewDataSection(bool omitEmptyDescription, ElementBase* parent)
    : DocumentElem          (parent)
    , mName                 ( )
    , mVersion              (0, 0, 1)
    , mDescription          ( )
    , mIsDeprecated         (false)
    , mDeprecateHint        ( )
    , mOmitEmptyDescription (omitEmptyDescription)
{
}

OverviewDataSection::OverviewDataSection(uint32_t id, const QString& name, bool omitEmptyDescription, ElementBase* parent)
    : DocumentElem          (id, parent)
    , mName                 (name)
    , mVersion              (0, 0, 1)
    , mDescription          ( )
    , mIsDeprecated         (false)
    , mDeprecateHint        ( )
    , mOmitEmptyDescription (omitEmptyDescription)
{
}

bool OverviewDataSection::isValid(void) const
{
    return (mName.isEmpty() == false);
}

bool OverviewDataSection::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementOverview))
        return false;

    const QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
    mVersion = VersionNumber(attributes.value(XmlSI::xmlSIAttributeVersion).toString());
    mIsDeprecated = attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated)
                    && (attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString().compare(XmlSI::xmlSIValueTrue, Qt::CaseInsensitive) == 0);
    mDescription.clear();
    mDeprecateHint.clear();

    readOwnAttributes(attributes);

    while (xml.readNextStartElement())
    {
        if (xml.name() == XmlSI::xmlSIElementDescription)
        {
            mDescription = xml.readElementText();
        }
        else if (xml.name() == XmlSI::xmlSIElementDeprecateHint)
        {
            mDeprecateHint = xml.readElementText();
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    return true;
}

void OverviewDataSection::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementOverview);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    xml.writeAttribute(XmlSI::xmlSIAttributeVersion, mVersion.toString());
    writeOwnAttributes(xml);
    if (mIsDeprecated)
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
        writeTextElem(xml, XmlSI::xmlSIElementDeprecateHint, mDeprecateHint, true);
    }

    writeTextElem(xml, XmlSI::xmlSIElementDescription, mDescription, mOmitEmptyDescription);
    xml.writeEndElement();
}

void OverviewDataSection::readOwnAttributes(const QXmlStreamAttributes& /*attributes*/)
{
}

void OverviewDataSection::writeOwnAttributes(QXmlStreamWriter& /*xml*/) const
{
}
