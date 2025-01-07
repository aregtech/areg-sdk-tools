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
 *  \file        lusan/data/common/DataTypeEnum.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Enum Data Type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/common/XmlSI.hpp"

DataTypeEnum::DataTypeEnum(ElementBase* parent /*= nullptr*/)
    : TEDataTypeContainer<EnumEntry>(eCategory::Enumeration, parent)
    , mDerived  ( "" )
{
}

DataTypeEnum::DataTypeEnum(const QString& name, ElementBase* parent /*= nullptr*/)
    : TEDataTypeContainer<EnumEntry>(eCategory::Enumeration, name, 0, parent)
    , mDerived("")
{
}

DataTypeEnum::DataTypeEnum(const DataTypeEnum& src)
    : TEDataTypeContainer<EnumEntry>(src)
    , mDerived(src.mDerived)
{
}

DataTypeEnum::DataTypeEnum(DataTypeEnum&& src) noexcept
    : TEDataTypeContainer<EnumEntry>(std::move(src))
    , mDerived(std::move(src.mDerived))
{
}

DataTypeEnum& DataTypeEnum::operator = (const DataTypeEnum& other)
{
    if (this != &other)
    {
        DataTypeBase::operator = (other);
        mDerived = other.mDerived;
    }

    return *this;
}

DataTypeEnum& DataTypeEnum::operator = (DataTypeEnum&& other) noexcept
{
    if (this != &other)
    {
        DataTypeBase::operator = (std::move(other));
        mDerived = std::move(other.mDerived);
    }

    return *this;
}

bool DataTypeEnum::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementDataType)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
    QString type = attributes.hasAttribute(XmlSI::xmlSIAttributeValues) ? attributes.value(XmlSI::xmlSIAttributeName).toString() : "";
    mDerived = type == DEFAULT_VALUES ? "" : type;

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
                if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementEnumEntry)
                {
                    EnumEntry entry(this);
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

void DataTypeEnum::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementDataType);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    xml.writeAttribute(XmlSI::xmlSIAttributeType, getType());
    xml.writeAttribute(XmlSI::xmlSIAttributeValues, mDerived.isEmpty() ? DEFAULT_VALUES : mDerived);

    xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);

    xml.writeStartElement(XmlSI::xmlSIElementFieldList);
    const QList<EnumEntry> & elements = getElements();
    for (const EnumEntry& entry : elements)
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement(); // FieldList
    xml.writeEndElement(); // DataType
}
