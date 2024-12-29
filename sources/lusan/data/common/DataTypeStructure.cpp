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

DataTypeStructure::DataTypeStructure(void)
    : TEDataTypeContainer< FieldEntry>(eCategory::Structure)
{
}

DataTypeStructure::DataTypeStructure(const QString& name)
    : TEDataTypeContainer< FieldEntry>(eCategory::Structure, name, 0)
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

DataTypeStructure& DataTypeStructure::operator=(const DataTypeStructure& other)
{
    DataTypeBase::operator=(other);
    return *this;
}

DataTypeStructure& DataTypeStructure::operator=(DataTypeStructure&& other) noexcept
{
    DataTypeBase::operator=(std::move(other));
    return *this;
}

bool DataTypeStructure::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementDataType)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.hasAttribute(XmlSI::xmlSIAttributeID) ? attributes.value(XmlSI::xmlSIAttributeID).toUInt() : 0);
    setName(attributes.hasAttribute(XmlSI::xmlSIAttributeName) ? attributes.value(XmlSI::xmlSIAttributeName).toString() : "");
    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementDataType))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementDescription)
        {
            mDescription = xml.readElementText();
        }
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementFieldList)
        {
            while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementFieldList))
            {
                if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementField)
                {
                    FieldEntry entry;
                    if (entry.readFromXml(xml))
                    {
                        mFieldList.append(entry);
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
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(mId));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    xml.writeAttribute(XmlSI::xmlSIAttributeType, "Structure");

    xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);

    xml.writeStartElement(XmlSI::xmlSIElementFieldList);
    for (const FieldEntry& entry : mFieldList)
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement(); // FieldList
    xml.writeEndElement(); // DataType
}
