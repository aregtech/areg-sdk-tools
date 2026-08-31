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
 *  \file        lusan/data/common/AttributeDataSection.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the `AttributeList` section shared by every document editor.
 *
 ************************************************************************/

#include "lusan/data/common/AttributeDataSection.hpp"

#include "lusan/common/XmlSI.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeDataSection.hpp"

AttributeDataSection::AttributeDataSection(const AttributeConfig& config, ElementBase* parent /*= nullptr*/)
    : TEDataContainer<AttributeEntry, DocumentElem>(parent)
    , mConfig(config)
{
}

AttributeDataSection::AttributeDataSection(const AttributeConfig& config, const QList<AttributeEntry>& entries, ElementBase* parent /*= nullptr*/)
    : TEDataContainer<AttributeEntry, DocumentElem>(parent)
    , mConfig(config)
{
    setElements(entries);
    for (AttributeEntry& entry : getElements())
    {
        entry.setConfig(mConfig);
    }
}

bool AttributeDataSection::isValid() const
{
    return true;
}

bool AttributeDataSection::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementAttributeList))
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementAttributeList))
    {
        if ((xml.tokenType() == QXmlStreamReader::StartElement) && (xml.name() == XmlSI::xmlSIElementAttribute))
        {
            AttributeEntry entry(this);
            entry.setConfig(mConfig);
            if (entry.readFromXml(xml))
            {
                addElement(std::move(entry), true);
            }
        }

        xml.readNext();
    }

    return true;
}

void AttributeDataSection::writeToXml(QXmlStreamWriter& xml) const
{
    const QList<AttributeEntry>& elements = getElements();
    if (elements.isEmpty())
        return;

    xml.writeStartElement(XmlSI::xmlSIElementAttributeList);
    for (const AttributeEntry& entry : elements)
    {
        // The section's shape wins over the entry's own: an entry that reached the list through a
        // generic add command was never stamped, and the file must still read back as this
        // document.
        entry.writeToXml(xml, mConfig);
    }

    xml.writeEndElement(); // AttributeList
}

AttributeEntry* AttributeDataSection::createAttribute(const QString& name)
{
    if (findElement(name) != nullptr)
        return nullptr;

    AttributeEntry entry(getNextId(), name, AttributeEntry::eNotification::NotifyOnChange, this);
    entry.setConfig(mConfig);
    return addElement(std::move(entry), true) ? findElement(name) : nullptr;
}

AttributeEntry* AttributeDataSection::insertAttribute(int position, const QString& name)
{
    if (findElement(name) != nullptr)
        return nullptr;

    AttributeEntry entry(getNextId(), name, AttributeEntry::eNotification::NotifyOnChange, this);
    entry.setConfig(mConfig);
    return insertElement(position, std::move(entry), true) ? findElement(name) : nullptr;
}

void AttributeDataSection::validate(const DataTypeDataSection& dataTypes)
{
    const QList<DataTypeCustom*>& customTypes = dataTypes.getResolutionTypes();
    for (AttributeEntry& entry : getElements())
    {
        entry.validate(customTypes);
    }
}

QList<uint32_t> AttributeDataSection::replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType)
{
    QList<uint32_t> result;
    for (AttributeEntry& entry : getElements())
    {
        if (entry.getParamType() == oldDataType)
        {
            entry.setParamType(newDataType);
            result.push_back(entry.getId());
        }
    }

    return result;
}
