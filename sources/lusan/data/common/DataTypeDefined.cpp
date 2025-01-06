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
 *  \file        lusan/data/common/DataTypeDefined.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Defined Data Type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypeDefined.hpp"
#include "lusan/common/XmlSI.hpp"

DataTypeDefined::DataTypeDefined(ElementBase* parent /*= nullptr*/)
    : DataTypeCustom(eCategory::CustomDefined, parent)
    , mContainer    ( )
    , mBaseTypeValue( )
    , mBaseTypeKey  ( )
{
}

DataTypeDefined::DataTypeDefined(const QString& name, ElementBase* parent /*= nullptr*/)
    : DataTypeCustom(eCategory::CustomDefined, 0, name, parent)
    , mContainer    ( )
    , mBaseTypeValue( )
    , mBaseTypeKey  ( )
{
}

DataTypeDefined::DataTypeDefined(const DataTypeDefined& src)
    : DataTypeCustom(src)
    , mContainer    (src.mContainer)
    , mBaseTypeValue(src.mBaseTypeValue)
    , mBaseTypeKey  (src.mBaseTypeKey)
{
}

DataTypeDefined::DataTypeDefined(DataTypeDefined&& src) noexcept
    : DataTypeCustom(std::move(src))
    , mContainer    (std::move(src.mContainer))
    , mBaseTypeValue(std::move(src.mBaseTypeValue))
    , mBaseTypeKey  (std::move(src.mBaseTypeKey))
{
}

DataTypeDefined& DataTypeDefined::operator = (const DataTypeDefined& other)
{
    if (this != &other)
    {
        DataTypeBase::operator = (other);
        mContainer      = other.mContainer;
        mBaseTypeValue  = other.mBaseTypeValue;
        mBaseTypeKey    = other.mBaseTypeKey;
    }

    return *this;
}

DataTypeDefined& DataTypeDefined::operator = (DataTypeDefined&& other) noexcept
{
    if (this != &other)
    {
        DataTypeBase::operator = (std::move(other));
        mContainer      = std::move(other.mContainer);
        mBaseTypeValue  = std::move(other.mBaseTypeValue);
        mBaseTypeKey    = std::move(other.mBaseTypeKey);
    }
    
    return *this;
}

bool DataTypeDefined::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementDataType)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    setName(attributes.value(XmlSI::xmlSIAttributeName).toString());

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementDataType))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSI::xmlSIElementDescription)
            {
                setDescription(xml.readElementText());
            }
            else if (xml.name() == XmlSI::xmlSIElementContainer)
            {
                mContainer = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementBaseTypeValue)
            {
                mBaseTypeValue = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementBaseTypeKey)
            {
                mBaseTypeKey = xml.readElementText();
            }
        }

        xml.readNext();
    }

    return true;
}

void DataTypeDefined::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementDataType);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    xml.writeAttribute(XmlSI::xmlSIAttributeType, getType());

    xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
    xml.writeTextElement(XmlSI::xmlSIElementContainer, mContainer);
    xml.writeTextElement(XmlSI::xmlSIElementBaseTypeValue, mBaseTypeValue);
    if (!mBaseTypeKey.isEmpty())
    {
        xml.writeTextElement(XmlSI::xmlSIElementBaseTypeKey, mBaseTypeKey);
    }

    xml.writeEndElement(); // DataType
}

bool DataTypeDefined::isKeyValueContainer() const
{
    return !mBaseTypeKey.isEmpty();
}
