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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMImportData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM submachine imports registry
 *
 ************************************************************************/

#include "lusan/data/sm/SMImportData.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

//////////////////////////////////////////////////////////////////////////
// SMImportEntry implementation
//////////////////////////////////////////////////////////////////////////

SMImportEntry::SMImportEntry(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mName         ( )
    , mLocation     ( )
    , mVersion      ( )
    , mDescription  ( )
{
}

SMImportEntry::SMImportEntry(  uint32_t id
                             , const QString& name
                             , const QString& location /*= QString()*/
                             , const VersionNumber& version /*= VersionNumber()*/
                             , ElementBase* parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mName         (name)
    , mLocation     (location)
    , mVersion      (version)
    , mDescription  ( )
{
}

SMImportEntry::SMImportEntry(const SMImportEntry& src)
    : DocumentElem  (src)
    , mName         (src.mName)
    , mLocation     (src.mLocation)
    , mVersion      (src.mVersion)
    , mDescription  (src.mDescription)
{
}

SMImportEntry::SMImportEntry(SMImportEntry&& src) noexcept
    : DocumentElem  (std::move(src))
    , mName         (std::move(src.mName))
    , mLocation     (std::move(src.mLocation))
    , mVersion      (src.mVersion)
    , mDescription  (std::move(src.mDescription))
{
}

SMImportEntry& SMImportEntry::operator = (const SMImportEntry& other)
{
    if (this != &other)
    {
        DocumentElem::operator = (other);
        mName        = other.mName;
        mLocation    = other.mLocation;
        mVersion     = other.mVersion;
        mDescription = other.mDescription;
    }

    return *this;
}

SMImportEntry& SMImportEntry::operator = (SMImportEntry&& other) noexcept
{
    if (this != &other)
    {
        DocumentElem::operator = (std::move(other));
        mName        = std::move(other.mName);
        mLocation    = std::move(other.mLocation);
        mVersion     = other.mVersion;
        mDescription = std::move(other.mDescription);
    }

    return *this;
}

bool SMImportEntry::isValid(void) const
{
    return (mName.isEmpty() == false) && (mLocation.isEmpty() == false);
}

bool SMImportEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementMachineImport)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    mName     = attributes.value(XmlSM::xmlSMAttributeName).toString();
    mLocation = attributes.value(XmlSM::xmlSMAttributeLocation).toString();
    mVersion  = VersionNumber(attributes.value(XmlSM::xmlSMAttributeVersion).toString());
    mDescription.clear();

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementMachineImport))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementDescription)
        {
            mDescription = xml.readElementText();
        }

        xml.readNext();
    }

    return true;
}

void SMImportEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementMachineImport);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSM::xmlSMAttributeName, mName);
    xml.writeAttribute(XmlSM::xmlSMAttributeLocation, mLocation);
    xml.writeAttribute(XmlSM::xmlSMAttributeVersion, mVersion.toString());
    writeTextElem(xml, XmlSM::xmlSMElementDescription, mDescription, true);
    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMImportData implementation
//////////////////////////////////////////////////////////////////////////

SMImportData::SMImportData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMImportEntry, DocumentElem>(parent)
{
}

bool SMImportData::isValid(void) const
{
    return true;
}

bool SMImportData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementImportList)
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementImportList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementMachineImport)
        {
            SMImportEntry entry(this);
            if (entry.readFromXml(xml))
            {
                addElement(std::move(entry), true);
            }
        }

        xml.readNext();
    }

    return true;
}

void SMImportData::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementImportList);
    for (const SMImportEntry& entry : getElements())
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement();
}

SMImportEntry* SMImportData::createImport(const QString& name)
{
    if (findElement(name) != nullptr)
    {
        return nullptr;
    }

    SMImportEntry entry(getNextId(), name, QString(), VersionNumber(), this);
    addElement(std::move(entry), true);
    return findElement(name);
}
