/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/common/DataTypeImported.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Imported Data Type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/common/XmlSI.hpp"

DataTypeImported::DataTypeImported(ElementBase* parent /*= nullptr*/)
    : DataTypeCustom(eCategory::Imported, parent)
    , mNamespace    ( )
    , mObject       ( )
    , mLocation     ( )
{
}

DataTypeImported::DataTypeImported(const QString& name, ElementBase* parent /*= nullptr*/)
    : DataTypeCustom(eCategory::Imported, 0, name, parent)
    , mNamespace    ( )
    , mObject       ( )
    , mLocation     ( )
{
}

DataTypeImported::DataTypeImported(const DataTypeImported& src)
    : DataTypeCustom(src)
    , mNamespace    (src.mNamespace)
    , mObject       (src.mObject)
    , mLocation     (src.mLocation)
{
}

DataTypeImported::DataTypeImported(DataTypeImported&& src) noexcept
    : DataTypeCustom(std::move(src))
    , mNamespace    (std::move(src.mNamespace))
    , mObject       (std::move(src.mObject))
    , mLocation     (std::move(src.mLocation))
{
}

DataTypeImported& DataTypeImported::operator = (const DataTypeImported& other)
{
    if (this != &other)
    {
        DataTypeBase::operator = (other);
        mNamespace  = other.mNamespace;
        mObject     = other.mObject;
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
        mObject     = std::move(other.mObject);
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

    QString depValue = attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() : "";
    setIsDeprecated(depValue.compare(XmlSI::xmlSIValueTrue, Qt::CaseSensitivity::CaseInsensitive) == 0);

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementDataType))
    {
        if (xml.tokenType() != QXmlStreamReader::StartElement)
        {
            xml.readNext();
            continue;
        }

        QStringView xmlName{ xml.name() };
        if (xmlName == XmlSI::xmlSIElementDescription)
        {
            setDescription(xml.readElementText());
        }
        else if (xmlName == XmlSI::xmlSIElementNamespace)
        {
            mNamespace = xml.readElementText();
        }
        else if (xmlName == XmlSI::xmlSIElementLocation)
        {
            mLocation = xml.readElementText();
        }
        else if (xmlName == XmlSI::xmlSIElementImportedObject)
        {
            mObject = xml.readElementText();
        }
        else if (xmlName == XmlSI::xmlSIElementDeprecateHint)
        {
            setDeprecateHint(xml.readElementText());
        }

        xml.readNext();
    }

    if  (mObject.isEmpty())
        mObject = getName();

    return true;
}

void DataTypeImported::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementDataType);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    xml.writeAttribute(XmlSI::xmlSIAttributeType, getType());
    if (getIsDeprecated())
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
        writeTextElem(xml, XmlSI::xmlSIElementDeprecateHint, getDeprecateHint(), true);
    }

    writeTextElem(xml, XmlSI::xmlSIElementLocation, mLocation, false);
    writeTextElem(xml, XmlSI::xmlSIElementNamespace, mNamespace, false);
    writeTextElem(xml, XmlSI::xmlSIElementImportedObject, mObject, false);
    writeTextElem(xml, XmlSI::xmlSIElementDescription, mDescription, false);

    xml.writeEndElement(); // DataType
}

QString DataTypeImported::toTypeString(void) const
{
    return (mNamespace.isEmpty() ? mObject : mNamespace + "::" + mObject);
}

QIcon DataTypeImported::getIcon(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return NELusanCommon::iconImported(NELusanCommon::SizeSmall);
    case ElementBase::eDisplay::DisplayType:
        return (isValid() ? QIcon() : NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
    default:
        return QIcon();
    }
}

QString DataTypeImported::getString(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return getName();
    case ElementBase::eDisplay::DisplayType:
        return toTypeString();
    default:
        return QString();
    }
}
