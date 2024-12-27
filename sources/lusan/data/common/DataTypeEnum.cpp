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

DataTypeEnum::DataTypeEnum(void)
    : DataTypeCustom(eCategory::Enumeration)
{
}

DataTypeEnum::DataTypeEnum(const QString& name)
    : DataTypeCustom(eCategory::Enumeration, 0, name)
{
}

DataTypeEnum::DataTypeEnum(const DataTypeEnum& src)
    : DataTypeCustom(src)
    , mFieldList(src.mFieldList)
{
}

DataTypeEnum::DataTypeEnum(DataTypeEnum&& src) noexcept
    : DataTypeCustom(std::move(src))
    , mFieldList(std::move(src.mFieldList))
{
}

DataTypeEnum& DataTypeEnum::operator=(const DataTypeEnum& other)
{
    if (this != &other)
    {
        DataTypeBase::operator=(other);
        mFieldList = other.mFieldList;
    }
    return *this;
}

DataTypeEnum& DataTypeEnum::operator=(DataTypeEnum&& other) noexcept
{
    if (this != &other)
    {
        DataTypeBase::operator=(std::move(other));
        mFieldList = std::move(other.mFieldList);
    }
    return *this;
}

bool DataTypeEnum::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementDataType)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    if (attributes.hasAttribute(XmlSI::xmlSIAttributeID))
    {
        mId = attributes.value(XmlSI::xmlSIAttributeID).toInt();
    }
    else
    {
        mId = 0;
    }

    if (attributes.hasAttribute(XmlSI::xmlSIAttributeName))
    {
        mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
    }
    else
    {
        mName = "";
    }

    if (attributes.hasAttribute(XmlSI::xmlSIAttributeValues))
    {
        QString type = attributes.value(XmlSI::xmlSIAttributeName).toString();
        mDerived.setName(type == DEFAULT_VALUES ? "" : type);
    }
    else
    {
        mDerived.setName("");
    }

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementDataType))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementFieldList)
        {
            while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementFieldList))
            {
                if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementEnumEntry)
                {
                    EnumEntry entry;
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

void DataTypeEnum::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementDataType);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(mId));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    xml.writeAttribute(XmlSI::xmlSIAttributeType, getType());
    xml.writeAttribute(XmlSI::xmlSIAttributeValues, mDerived.getName().isEmpty() ? DEFAULT_VALUES : mDerived.getName());

    xml.writeStartElement(XmlSI::xmlSIElementFieldList);
        for (const EnumEntry& entry : mFieldList)
        {
            entry.writeToXml(xml);
        }

    xml.writeEndElement(); // FieldList
    xml.writeEndElement(); // DataType
}

const QList<EnumEntry>& DataTypeEnum::getFieldList() const
{
    return mFieldList;
}

void DataTypeEnum::setFieldList(const QList<EnumEntry>& fieldList)
{
    mFieldList = fieldList;
}
