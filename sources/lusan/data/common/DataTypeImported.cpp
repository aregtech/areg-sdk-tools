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
 *  \copyright   � 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/common/DataTypeImported.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Imported Data Type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/common/XmlSI.hpp"

DataTypeImported::DataTypeImported(ElementBase* parent /*= nullptr*/)
    : DataTypeCustom(eCategory::Imported, parent)
    , mNamespace    ( )
    , mLocation     ( )
{
}

DataTypeImported::DataTypeImported(const QString& name, ElementBase* parent /*= nullptr*/)
    : DataTypeCustom(eCategory::Imported, 0, name, parent)
    , mNamespace    ( )
    , mLocation     ( )
{
}

DataTypeImported::DataTypeImported(const DataTypeImported& src)
    : DataTypeCustom(src)
    , mNamespace    (src.mNamespace)
    , mLocation     (src.mLocation)
{
}

DataTypeImported::DataTypeImported(DataTypeImported&& src) noexcept
    : DataTypeCustom(std::move(src))
    , mNamespace    (std::move(src.mNamespace))
    , mLocation     (std::move(src.mLocation))
{
}

DataTypeImported& DataTypeImported::operator = (const DataTypeImported& other)
{
    if (this != &other)
    {
        DataTypeBase::operator = (other);
        mNamespace  = other.mNamespace;
        mLocation   = other.mLocation;
    }

    return *this;
}

DataTypeImported& DataTypeImported::operator = (DataTypeImported&& other) noexcept
{
    if (this != &other)
    {
        DataTypeBase::operator = (std::move(other));
        mNamespace  = std::move(other.mNamespace);
        mLocation   = std::move(other.mLocation);
    }

    return *this;
}

bool DataTypeImported::readFromXml(QXmlStreamReader& xml)
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
            else if (xml.name() == XmlSI::xmlSIElementNamespace)
            {
                mNamespace = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementLocation)
            {
                mLocation = xml.readElementText();
            }
        }

        xml.readNext();
    }

    return true;
}

void DataTypeImported::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementDataType);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    xml.writeAttribute(XmlSI::xmlSIAttributeType, getType());

    xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
    xml.writeTextElement(XmlSI::xmlSIElementNamespace, mNamespace);
    xml.writeTextElement(XmlSI::xmlSIElementLocation, mLocation);

    xml.writeEndElement(); // DataType
}
