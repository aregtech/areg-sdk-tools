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
 *  \file        lusan/data/sm/SMAttributeData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM machine attributes registry.
 *
 ************************************************************************/

#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

//////////////////////////////////////////////////////////////////////////
// SMAttributeEntry implementation
//////////////////////////////////////////////////////////////////////////

SMAttributeEntry::SMAttributeEntry(ElementBase* parent /*= nullptr*/)
    : ParamBase (parent)
    , mValue    ( )
{
}

SMAttributeEntry::SMAttributeEntry(  uint32_t id
                                   , const QString& name
                                   , const QString& type    /*= "bool"*/
                                   , const QString& value   /*= QString()*/
                                   , ElementBase* parent     /*= nullptr*/)
    : ParamBase (id, name, type, parent)
    , mValue    (value)
{
}

SMAttributeEntry::SMAttributeEntry(const SMAttributeEntry& src)
    : ParamBase (src)
    , mValue    (src.mValue)
{
}

SMAttributeEntry::SMAttributeEntry(SMAttributeEntry&& src) noexcept
    : ParamBase (std::move(src))
    , mValue    (std::move(src.mValue))
{
}

SMAttributeEntry& SMAttributeEntry::operator = (const SMAttributeEntry& other)
{
    if (this != &other)
    {
        ParamBase::operator = (other);
        mValue = other.mValue;
    }

    return *this;
}

SMAttributeEntry& SMAttributeEntry::operator = (SMAttributeEntry&& other) noexcept
{
    if (this != &other)
    {
        ParamBase::operator = (std::move(other));
        mValue = std::move(other.mValue);
    }

    return *this;
}

bool SMAttributeEntry::isValid(void) const
{
    return (getName().isEmpty() == false) && (getType().isEmpty() == false);
}

bool SMAttributeEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementAttribute)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    setName(attributes.value(XmlSM::xmlSMAttributeName).toString());
    setType(attributes.value(XmlSM::xmlSMAttributeDataType).toString());
    mValue = attributes.value(XmlSM::xmlSMAttributeValue).toString();

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementAttribute))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementDescription)
        {
            setDescription(xml.readElementText());
        }

        xml.readNext();
    }

    return true;
}

void SMAttributeEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementAttribute);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSM::xmlSMAttributeName, getName());
    xml.writeAttribute(XmlSM::xmlSMAttributeDataType, getType());
    xml.writeAttribute(XmlSM::xmlSMAttributeValue, mValue);
    writeTextElem(xml, XmlSM::xmlSMElementDescription, getDescription(), true);
    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMAttributeData implementation
//////////////////////////////////////////////////////////////////////////

SMAttributeData::SMAttributeData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMAttributeEntry, DocumentElem>(parent)
{
}

SMAttributeData::SMAttributeData(const QList<SMAttributeEntry>& entries, ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMAttributeEntry, DocumentElem>(entries, parent)
{
}

bool SMAttributeData::isValid(void) const
{
    return true;
}

bool SMAttributeData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementAttributeList)
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementAttributeList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementAttribute)
        {
            SMAttributeEntry entry(this);
            if (entry.readFromXml(xml))
            {
                addElement(std::move(entry), true);
            }
        }

        xml.readNext();
    }

    return true;
}

void SMAttributeData::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementAttributeList);
    for (const SMAttributeEntry& entry : getElements())
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement();
}

SMAttributeEntry* SMAttributeData::createAttribute(const QString& name)
{
    if (findElement(name) != nullptr)
    {
        return nullptr;
    }

    SMAttributeEntry entry(getNextId(), name, "bool", QString(), this);
    addElement(std::move(entry), true);
    return findElement(name);
}
