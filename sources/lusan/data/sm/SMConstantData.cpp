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
 *  \file        lusan/data/sm/SMConstantData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM constants registry. Reuses ConstantEntry.
 *
 ************************************************************************/

#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/common/XmlSM.hpp"
#include "lusan/data/sm/SMDataTypeData.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

SMConstantData::SMConstantData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<ConstantEntry, DocumentElem>(parent)
{
}

bool SMConstantData::isValid() const
{
    return true;
}

bool SMConstantData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementConstantList)
        return false;

    // `.fsml` carries the constant value as an attribute (not a `<Value>` child as `.siml`
    // does), so the entry is read here rather than delegated to ConstantEntry::readFromXml.
    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementConstantList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementConstant)
        {
            ConstantEntry entry(this);
            QXmlStreamAttributes attributes = xml.attributes();
            entry.setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
            entry.setName(attributes.value(XmlSM::xmlSMAttributeName).toString());
            entry.setType(attributes.value(XmlSM::xmlSMAttributeDataType).toString());
            entry.setValue(attributes.value(XmlSM::xmlSMAttributeValue).toString());

            while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementConstant))
            {
                if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementDescription)
                {
                    entry.setDescription(xml.readElementText());
                }

                xml.readNext();
            }

            addElement(std::move(entry), true);
        }

        xml.readNext();
    }

    return true;
}

void SMConstantData::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementConstantList);
    for (const ConstantEntry& entry : getElements())
    {
        xml.writeStartElement(XmlSM::xmlSMElementConstant);
        xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(entry.getId()));
        xml.writeAttribute(XmlSM::xmlSMAttributeName, entry.getName());
        xml.writeAttribute(XmlSM::xmlSMAttributeDataType, entry.getType());
        xml.writeAttribute(XmlSM::xmlSMAttributeValue, entry.getValue());
        writeTextElem(xml, XmlSM::xmlSMElementDescription, entry.getDescription(), true);
        xml.writeEndElement();
    }

    xml.writeEndElement();
}

ConstantEntry* SMConstantData::createConstant(const QString& name)
{
    if (findElement(name) != nullptr)
    {
        return nullptr;
    }

    ConstantEntry entry(getNextId(), name, this);
    addElement(std::move(entry), true);
    return findElement(name);
}

void SMConstantData::validate(const SMDataTypeData& dataTypes)
{
    const QList<DataTypeCustom*>& customTypes = dataTypes.getCustomDataTypes();
    for (ConstantEntry& entry : getElements())
    {
        entry.validate(customTypes);
    }
}
