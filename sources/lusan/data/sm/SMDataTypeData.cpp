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
 *  \file        lusan/data/sm/SMDataTypeData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM data types registry. Reuses DataTypeCustom.
 *
 ************************************************************************/

#include "lusan/data/sm/SMDataTypeData.hpp"

#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

SMDataTypeData::SMDataTypeData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<DataTypeCustom*, DocumentElem>(parent)
{
}

SMDataTypeData::~SMDataTypeData()
{
    removeAll();
}

bool SMDataTypeData::isValid() const
{
    return true;
}

bool SMDataTypeData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementDataTypeList)
        return false;

    // `.fsml` reuses the `.siml` DataType vocabulary verbatim, so the shared DataType
    // factory and per-type readers produce an identical model (and identical round-trip).
    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementDataTypeList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementDataType)
        {
            const QString type = xml.attributes().value(XmlSI::xmlSIAttributeType).toString();
            DataTypeCustom* dataType = DataTypeFactory::createCustomDataType(type);
            if (dataType != nullptr)
            {
                dataType->setParent(this);
                if (dataType->readFromXml(xml))
                {
                    addElement(dataType, true);
                }
                else
                {
                    delete dataType;
                }
            }
            else
            {
                xml.skipCurrentElement();
            }
        }

        xml.readNext();
    }

    return true;
}

void SMDataTypeData::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementDataTypeList);
    for (const DataTypeCustom* dataType : getElements())
    {
        dataType->writeToXml(xml);
    }

    xml.writeEndElement();
}

const QList<DataTypeCustom*>& SMDataTypeData::getCustomDataTypes() const
{
    return getElements();
}

DataTypeCustom* SMDataTypeData::addCustomDataType(const QString& name, DataTypeBase::eCategory category)
{
    if (findCustomDataType(name) != nullptr)
    {
        return nullptr;
    }

    DataTypeCustom* dataType = DataTypeFactory::createCustomDataType(category);
    if (dataType == nullptr)
    {
        return nullptr;
    }

    dataType->setParent(this);
    dataType->setId(getNextId());
    dataType->setName(name);
    addElement(dataType, true);
    return dataType;
}

DataTypeCustom* SMDataTypeData::addEnum(const QString& name)
{
    return addCustomDataType(name, DataTypeBase::eCategory::Enumeration);
}

DataTypeCustom* SMDataTypeData::addStructure(const QString& name)
{
    return addCustomDataType(name, DataTypeBase::eCategory::Structure);
}

DataTypeCustom* SMDataTypeData::addImported(const QString& name)
{
    return addCustomDataType(name, DataTypeBase::eCategory::Imported);
}

DataTypeCustom* SMDataTypeData::findCustomDataType(const QString& name) const
{
    DataTypeCustom* const* found = findElement(name);
    return (found != nullptr) ? *found : nullptr;
}

DataTypeCustom* SMDataTypeData::findCustomDataType(uint32_t id) const
{
    DataTypeCustom* const* found = findElement(id);
    return (found != nullptr) ? *found : nullptr;
}

void SMDataTypeData::removeAll()
{
    for (DataTypeCustom* dataType : getElements())
    {
        delete dataType;
    }

    removeAllElements();
}

void SMDataTypeData::validate(const SMDataTypeData& dataTypes)
{
    for (DataTypeCustom* dataType : getElements())
    {
        dataTypes.normalizeType(dataType);
    }
}

void SMDataTypeData::normalizeType(DataTypeCustom* dataType) const
{
    const QList<DataTypeCustom*>& customTypes{ getCustomDataTypes() };
    if (dataType->isStructure())
    {
        static_cast<DataTypeStructure*>(dataType)->validate(customTypes);
    }
    else if (dataType->isContainer())
    {
        static_cast<DataTypeContainer*>(dataType)->validate(customTypes);
    }
}
