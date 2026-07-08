/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMAttributeData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM machine attributes registry.
 *
 ************************************************************************/

#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/common/XmlSM.hpp"
#include "lusan/data/sm/SMDataTypeData.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

//////////////////////////////////////////////////////////////////////////
// SMAttributeEntry implementation
//////////////////////////////////////////////////////////////////////////

SMAttributeEntry::SMAttributeEntry(ElementBase* parent /*= nullptr*/)
    : ParamBase (parent)
    , mValue    ( )
{
}

SMAttributeEntry::SMAttributeEntry(  uint32_t id
                                   , const QString& name
                                   , const QString& type    /*= "bool"*/
                                   , const QString& value   /*= QString()*/
                                   , ElementBase* parent     /*= nullptr*/)
    : ParamBase (id, name, type, parent)
    , mValue    (value)
{
}

SMAttributeEntry::SMAttributeEntry(const SMAttributeEntry& src)
    : ParamBase (src)
    , mValue    (src.mValue)
{
}

SMAttributeEntry::SMAttributeEntry(SMAttributeEntry&& src) noexcept
    : ParamBase (std::move(src))
    , mValue    (std::move(src.mValue))
{
}

SMAttributeEntry& SMAttributeEntry::operator = (const SMAttributeEntry& other)
{
    if (this != &other)
    {
        ParamBase::operator = (other);
        mValue = other.mValue;
    }

    return *this;
}

SMAttributeEntry& SMAttributeEntry::operator = (SMAttributeEntry&& other) noexcept
{
    if (this != &other)
    {
        ParamBase::operator = (std::move(other));
        mValue = std::move(other.mValue);
    }

    return *this;
}

bool SMAttributeEntry::isValid() const
{
    return (getName().isEmpty() == false) && (getType().isEmpty() == false);
}

QIcon SMAttributeEntry::getIcon(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return NELusanCommon::iconAttribute(NELusanCommon::SizeSmall);
    case ElementBase::eDisplay::DisplayType:
        return (mParamType.isValid() ? QIcon() : NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
    default:
        return QIcon();
    }
}

QString SMAttributeEntry::getString(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return getName();
    case ElementBase::eDisplay::DisplayType:
        return getType();
    case ElementBase::eDisplay::DisplayValue:
        return mValue;
    default:
        return QString();
    }
}

bool SMAttributeEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementAttribute)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    setName(attributes.value(XmlSM::xmlSMAttributeName).toString());
    setType(attributes.value(XmlSM::xmlSMAttributeDataType).toString());
    mValue = attributes.value(XmlSM::xmlSMAttributeValue).toString();
    setIsDeprecated(attributes.hasAttribute(XmlSM::xmlSMAttributeIsDeprecated)
        && (attributes.value(XmlSM::xmlSMAttributeIsDeprecated).toString().compare(XmlSM::xmlSMValueTrue, Qt::CaseInsensitive) == 0));
    setDeprecateHint(QString());

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementAttribute))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSM::xmlSMElementDeprecateHint)
            {
                setDeprecateHint(xml.readElementText());
            }
            else if (xml.name() == XmlSM::xmlSMElementDescription)
            {
                setDescription(xml.readElementText());
            }
        }

        xml.readNext();
    }

    return true;
}

void SMAttributeEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementAttribute);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSM::xmlSMAttributeName, getName());
    xml.writeAttribute(XmlSM::xmlSMAttributeDataType, getType());
    xml.writeAttribute(XmlSM::xmlSMAttributeValue, mValue);
    if (getIsDeprecated())
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeIsDeprecated, XmlSM::xmlSMValueTrue);
        writeTextElem(xml, XmlSM::xmlSMElementDeprecateHint, getDeprecateHint(), true);
    }
    writeTextElem(xml, XmlSM::xmlSMElementDescription, getDescription(), true);
    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMAttributeData implementation
//////////////////////////////////////////////////////////////////////////

SMAttributeData::SMAttributeData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMAttributeEntry, DocumentElem>(parent)
{
}

SMAttributeData::SMAttributeData(const QList<SMAttributeEntry>& entries, ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMAttributeEntry, DocumentElem>(entries, parent)
{
}

bool SMAttributeData::isValid() const
{
    return true;
}

bool SMAttributeData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementAttributeList)
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementAttributeList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementAttribute)
        {
            SMAttributeEntry entry(this);
            if (entry.readFromXml(xml))
            {
                addElement(std::move(entry), true);
            }
        }

        xml.readNext();
    }

    return true;
}

void SMAttributeData::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementAttributeList);
    for (const SMAttributeEntry& entry : getElements())
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement();
}

SMAttributeEntry* SMAttributeData::createAttribute(const QString& name)
{
    if (findElement(name) != nullptr)
    {
        return nullptr;
    }

    SMAttributeEntry entry(getNextId(), name, "bool", QString(), this);
    addElement(std::move(entry), true);
    return findElement(name);
}

void SMAttributeData::validate(const SMDataTypeData& dataTypes)
{
    const QList<DataTypeCustom*>& customTypes = dataTypes.getCustomDataTypes();
    for (SMAttributeEntry& entry : getElements())
    {
        entry.validate(customTypes);
    }
}
