﻿/************************************************************************
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
 *  \file        lusan/data/si/SIAttributeData.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Attribute Data.
 *
 ************************************************************************/
#include "lusan/data/si/SIAttributeData.hpp"
#include "lusan/data/si/SIDataTypeData.hpp"
#include "lusan/common/XmlSI.hpp"

SIAttributeData::SIAttributeData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer< AttributeEntry, DocumentElem >(parent)
{
}

SIAttributeData::SIAttributeData(const QList<AttributeEntry>& entries, ElementBase* parent /*= nullptr*/)
    : TEDataContainer< AttributeEntry, DocumentElem >(parent)
{
    setElements(entries);
}

bool SIAttributeData::isValid() const
{
    return true;
}

bool SIAttributeData::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementAttributeList))
        return false;

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementAttributeList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementAttribute)
        {
            AttributeEntry entry(this);
            if (entry.readFromXml(xml))
            {
                addElement(std::move(entry), true);
            }
        }

        xml.readNext();
    }

    return true;
}

void SIAttributeData::writeToXml(QXmlStreamWriter& xml) const
{
    const QList<AttributeEntry>& elements = getElements();
    if (elements.size() == 0)
        return;
    
    xml.writeStartElement(XmlSI::xmlSIElementAttributeList);
    for (const AttributeEntry& entry : elements)
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement(); // AttributeList
}

void SIAttributeData::validate(const SIDataTypeData& dataTypes)
{
    const QList<DataTypeCustom * >& customTypes = dataTypes.getCustomDataTypes();
    QList< AttributeEntry>& list = getElements();
    for (AttributeEntry& entry : list)
    {
        entry.validate(customTypes);
    }
}

AttributeEntry* SIAttributeData::createAttribute(const QString& name, AttributeEntry::eNotification notification /*= AttributeEntry::eNotification::NotifyOnChange*/)
{
    AttributeEntry* result{ nullptr };
    AttributeEntry entry(getNextId(), name, notification, this);
    if (addElement(std::move(entry), false))
    {
        Q_ASSERT(mElementList.size() > 0);
        result = &mElementList[mElementList.size() - 1];
    }

    return result;
}

QList<uint32_t> SIAttributeData::replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType)
{
    QList<uint32_t> result;
    QList< AttributeEntry>& list = getElements();
    for (AttributeEntry& entry : list)
    {
        if (entry.getParamType() == oldDataType)
        {
            entry.setParamType(newDataType);
            result.push_back(entry.getId());
        }
    }

    return result;
}

AttributeEntry* SIAttributeData::insertAttribute(int position, const QString& name, AttributeEntry::eNotification notification /*= AttributeEntry::eNotification::NotifyOnChange*/)
{
    AttributeEntry* result{ nullptr };
    AttributeEntry entry(getNextId(), name, notification, this);
    if (insertElement(position, std::move(entry), false))
    {
        result = &mElementList[position];
    }

    return result;
}
