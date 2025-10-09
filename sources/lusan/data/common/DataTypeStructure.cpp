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
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/FieldEntry.hpp"
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

    QString depValue = attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() : "";
    setIsDeprecated(depValue.compare(XmlSI::xmlSIValueTrue, Qt::CaseSensitivity::CaseInsensitive) == 0);

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
        writeTextElem(xml, XmlSI::xmlSIElementDeprecateHint, getDeprecateHint(), true);
    }

    writeTextElem(xml, XmlSI::xmlSIElementDescription, mDescription, false);

    if (getElementCount() > 0)
    {
        xml.writeStartElement(XmlSI::xmlSIElementFieldList);
        for (const FieldEntry& entry : mElementList)
        {
            entry.writeToXml(xml);
        }

        xml.writeEndElement(); // FieldList
    }

    xml.writeEndElement(); // DataType
}

FieldEntry* DataTypeStructure::addField(const QString& name)
{
    FieldEntry* result{ nullptr };
    FieldEntry entry(getNextId(), name, this);
    if (addElement(std::move(entry), true))
    {
        Q_ASSERT(mElementList.size() > 0);
        result = &mElementList[mElementList.size() - 1];
    }

    return result;
}

FieldEntry* DataTypeStructure::insertField(int position, const QString& name)
{
    FieldEntry* result{ nullptr };
    FieldEntry entry(getNextId(), name, this);
    if (insertElement(position, std::move(entry), true))
    {
        Q_ASSERT(mElementList.size() > position);
        result = &mElementList[position];
    }

    return result;
}

void DataTypeStructure::removeField(const QString& name)
{
    removeElement(name);
}

void DataTypeStructure::removeField(uint32_t id)
{
    removeElement(id);
}

DataTypeBase* DataTypeStructure::getFieldType(const QString& name) const
{
    FieldEntry * field = findElement(name);
    return (field != nullptr ? field->getParamType() : nullptr);
}

DataTypeBase* DataTypeStructure::getFieldType(uint32_t id) const
{
    FieldEntry* field = findElement(id);
    return (field != nullptr ? field->getParamType() : nullptr);
}

bool DataTypeStructure::validate(const QList<DataTypeCustom*>& customTypes)
{
    bool result{ true };
    for (FieldEntry& entry : mElementList)
    {
        result &= entry.validate(customTypes);
    }

    return result;
}

void DataTypeStructure::invalidate(void)
{
    for (FieldEntry& entry : mElementList)
    {
        entry.invalidate();
    }
}

QIcon DataTypeStructure::getIcon(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return NELusanCommon::iconStructure(NELusanCommon::SizeSmall);
    case ElementBase::eDisplay::DisplayType:
        return (isValid() ? QIcon() : NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
    default:
        return QIcon();
    }
}

QString DataTypeStructure::getString(ElementBase::eDisplay display) const
{
    if (display == ElementBase::eDisplay::DisplayName)
    {
        return getName();
    }
    else
    {
        return QString();
    }
}
