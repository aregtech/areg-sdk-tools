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
    : TEDataContainer< AttributeEntry, ElementBase >(parent)
{
}

SIAttributeData::SIAttributeData(const QList<AttributeEntry>& entries, ElementBase* parent /*= nullptr*/)
    : TEDataContainer< AttributeEntry, ElementBase >(parent)
{
    setElements(entries);
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
    xml.writeStartElement(XmlSI::xmlSIElementAttributeList);

    const QList<AttributeEntry>& elements = getElements();
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
