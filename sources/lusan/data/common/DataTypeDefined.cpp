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
#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"

#include "lusan/common/XmlSI.hpp"

DataTypeDefined::DataTypeDefined(ElementBase* parent /*= nullptr*/)
    : DataTypeCustom(eCategory::Container, parent)
    , mContainer    ( )
    , mValue( )
    , mKey  ( )
{
}

DataTypeDefined::DataTypeDefined(const QString& name, ElementBase* parent /*= nullptr*/)
    : DataTypeCustom(eCategory::Container, 0, name, parent)
    , mContainer    ( )
    , mValue( )
    , mKey  ( )
{
}

DataTypeDefined::DataTypeDefined(const DataTypeDefined& src)
    : DataTypeCustom(src)
    , mContainer    (src.mContainer)
    , mValue(src.mValue)
    , mKey  (src.mKey)
{
}

DataTypeDefined::DataTypeDefined(DataTypeDefined&& src) noexcept
    : DataTypeCustom(std::move(src))
    , mContainer(std::move(src.mContainer))
    , mValue    (std::move(src.mValue))
    , mKey      (std::move(src.mKey))
{
}

DataTypeDefined& DataTypeDefined::operator = (const DataTypeDefined& other)
{
    if (this != &other)
    {
        DataTypeBase::operator = (other);
        mContainer  = other.mContainer;
        mValue      = other.mValue;
        mKey        = other.mKey;
    }

    return *this;
}

DataTypeDefined& DataTypeDefined::operator = (DataTypeDefined&& other) noexcept
{
    if (this != &other)
    {
        DataTypeBase::operator = (std::move(other));
        mContainer  = std::move(other.mContainer);
        mValue      = std::move(other.mValue);
        mKey        = std::move(other.mKey);
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
                mValue = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementBaseTypeKey)
            {
                mKey = xml.readElementText();
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
    xml.writeTextElement(XmlSI::xmlSIElementBaseTypeValue, mValue);
    if (canHaveKey())
    {
        xml.writeTextElement(XmlSI::xmlSIElementBaseTypeKey, mKey);
    }

    xml.writeEndElement(); // DataType
}

bool DataTypeDefined::canHaveKey(void) const
{
    bool result{ false };
    const QList<DataTypeBasicContainer*> containerTypes = DataTypeFactory::getContainerTypes();
    for (const DataTypeBasicContainer* container : containerTypes)
    {
        if (container->getName() == mContainer)
        {
            result = container->hasKey();
            break;
        }
    }

    return result;
}
