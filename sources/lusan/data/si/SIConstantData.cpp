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
 *  \file        lusan/data/si/SIConstantData.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Constant Data.
 *
 ************************************************************************/
#include "lusan/data/si/SIConstantData.hpp"
#include "lusan/data/si/SIDataTypeData.hpp"
#include "lusan/common/XmlSI.hpp"

SIConstantData::SIConstantData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer< ConstantEntry, DocumentElem>(parent)
{
}

SIConstantData::SIConstantData(const QList<ConstantEntry>& entries, ElementBase* parent /*= nullptr*/)
    : TEDataContainer<ConstantEntry, DocumentElem>(parent)
{
    setElements(entries);
}

bool SIConstantData::isValid() const
{
    return true;
}

bool SIConstantData::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementConstantList))
        return false;

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementConstantList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementConstant)
        {
            ConstantEntry entry(this);
            if (entry.readFromXml(xml))
            {
                addElement(std::move(entry), true);
            }
        }

        xml.readNext();
    }

    return true;
}

void SIConstantData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementConstantList);
    const QList<ConstantEntry>& elements = getElements();
    for (const ConstantEntry& entry : elements)
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement(); // ConstantList
}

void SIConstantData::validate(const SIDataTypeData& dataTypes)
{
    const QList<DataTypeCustom *>& customTypes = dataTypes.getCustomDataTypes();
    QList< ConstantEntry>& list = getElements();
    for (ConstantEntry& entry : list)
    {
        entry.validate(customTypes);
    }
}

ConstantEntry* SIConstantData::createConstant(const QString& name)
{
    ConstantEntry entry(getNextId(), name, this);
    return addElement(std::move(entry), false) ? static_cast<ConstantEntry *>(findElement(entry.getId())) : nullptr;
}
    
