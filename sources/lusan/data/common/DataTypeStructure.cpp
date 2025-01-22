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
 *  \file        lusan/data/common/DataTypeStructure.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Structure Data Type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/common/XmlSI.hpp"

DataTypeStructure::DataTypeStructure(ElementBase* parent /*= nullptr*/)
    : TEDataTypeContainer< FieldEntry>(eCategory::Structure, parent)
{
}

DataTypeStructure::DataTypeStructure(const QString& name, ElementBase* parent /*= nullptr*/)
    : TEDataTypeContainer< FieldEntry>(eCategory::Structure, name, 0, parent)
{
}

DataTypeStructure::DataTypeStructure(const DataTypeStructure& src)
    : TEDataTypeContainer< FieldEntry>(src)
{
}

DataTypeStructure::DataTypeStructure(DataTypeStructure&& src) noexcept
    : TEDataTypeContainer< FieldEntry>(std::move(src))
{
}

DataTypeStructure& DataTypeStructure::operator = (const DataTypeStructure& other)
{
    DataTypeBase::operator = (other);
    return *this;
}

DataTypeStructure& DataTypeStructure::operator = (DataTypeStructure&& other) noexcept
{
    DataTypeBase::operator = (std::move(other));
    return *this;
}

bool DataTypeStructure::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementDataType)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    setName(attributes.value(XmlSI::xmlSIAttributeName).toString());
    setIsDeprecated(attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() == XmlSI::xmlSIValueTrue : false);

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementDataType))
    {
        if (xml.tokenType() != QXmlStreamReader::StartElement)
        {
            xml.readNext();
            continue;
        }

        if (xml.name() == XmlSI::xmlSIElementDescription)
        {
            mDescription = xml.readElementText();
        }
        else if (xml.name() == XmlSI::xmlSIElementDeprecateHint)
        {
            setDeprecateHint(xml.readElementText());
        }
        else if (xml.name() == XmlSI::xmlSIElementFieldList)
        {
            while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementFieldList))
            {
                if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementField)
                {
                    FieldEntry entry(this);
                    if (entry.readFromXml(xml))
                    {
                        addElement(std::move(entry), true);
                    }
                }

                xml.readNext();
            }
        }

        xml.readNext();
    }

    return true;
}

void DataTypeStructure::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementDataType);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    xml.writeAttribute(XmlSI::xmlSIAttributeType, getType());
    if (getIsDeprecated())
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
    }

    xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
    if (getIsDeprecated())
    {
        xml.writeTextElement(XmlSI::xmlSIElementDeprecateHint, getDeprecateHint());
    }

    xml.writeStartElement(XmlSI::xmlSIElementFieldList);
    for (const FieldEntry& entry : mElementList)
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement(); // FieldList
    xml.writeEndElement(); // DataType
}

FieldEntry* DataTypeStructure::addField(const QString& name)
{
    uint32_t id{ getNextId() };
    addElement(std::move(FieldEntry(id, name, this)), true);
    return findElement(id);
}
