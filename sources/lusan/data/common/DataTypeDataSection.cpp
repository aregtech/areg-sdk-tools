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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/common/DataTypeDataSection.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the data types section of a document.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeDataSection.hpp"

#include "lusan/common/XmlSI.hpp"
#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypePrimitive.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

DataTypeDataSection::DataTypeDataSection(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<DataTypeCustom*, DocumentElem>(parent)
{
}

DataTypeDataSection::~DataTypeDataSection()
{
    removeAll();
}

bool DataTypeDataSection::isValid() const
{
    return true;
}

bool DataTypeDataSection::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementDataTypeList))
        return false;

    while (xml.readNextStartElement())
    {
        if (xml.name() != XmlSI::xmlSIElementDataType)
        {
            xml.skipCurrentElement();
            continue;
        }

        const QString type = xml.attributes().value(XmlSI::xmlSIAttributeType).toString();
        DataTypeCustom* dataType = DataTypeFactory::createCustomDataType(type);
        if (dataType == nullptr)
        {
            xml.skipCurrentElement();
            continue;
        }

        dataType->setParent(this);
        if (dataType->readFromXml(xml))
        {
            addElement(dataType);
        }
        else
        {
            delete dataType;
            xml.skipCurrentElement();
        }
    }

    return true;
}

void DataTypeDataSection::writeToXml(QXmlStreamWriter& xml) const
{
    const QList<DataTypeCustom*>& elements {getElements()};
    if (elements.isEmpty())
        return;

    xml.writeStartElement(XmlSI::xmlSIElementDataTypeList);
    for (const DataTypeCustom* dataType : elements)
    {
        dataType->writeToXml(xml);
    }

    xml.writeEndElement();
}

const QList<DataTypeCustom*>& DataTypeDataSection::getCustomDataTypes() const
{
    return getElements();
}

const QList<DataTypePrimitive*>& DataTypeDataSection::getPrimitiveDataTypes() const
{
    return DataTypeFactory::getPrimitiveTypes();
}

const QList<DataTypeBasicObject*>& DataTypeDataSection::getBasicDataTypes() const
{
    return DataTypeFactory::getBasicTypes();
}

const QList<DataTypeBasicContainer*>& DataTypeDataSection::getContainerDatTypes() const
{
    return DataTypeFactory::getContainerTypes();
}

void DataTypeDataSection::getDataType(QList<DataTypeBase*>& out_dataTypes, const QList<DataTypeBase*>& excludes /*= QList<DataTypeBase *>*/, bool makeSorting /*= false*/) const
{
    out_dataTypes.clear();
    for (DataTypePrimitive* dataType : getPrimitiveDataTypes())
    {
        if (exists<DataTypeBase>(excludes, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }

    for (DataTypeBasicObject* dataType : getBasicDataTypes())
    {
        if (exists<DataTypeBase>(excludes, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }

    for (DataTypeBasicContainer* dataType : getContainerDatTypes())
    {
        if (exists<DataTypeBase>(excludes, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }

    if (makeSorting)
    {
        NELusanCommon::sortById(out_dataTypes, true);
    }

    const int begin = static_cast<int>(out_dataTypes.size());
    for (DataTypeCustom* dataType : getElements())
    {
        if (exists<DataTypeBase>(excludes, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }

    if (makeSorting && (begin < static_cast<int>(out_dataTypes.size())))
    {
        NELusanCommon::sortById<const DataTypeBase *>(out_dataTypes.begin() + begin, out_dataTypes.end(), true);
    }
}

DataTypeBase* DataTypeDataSection::findDataType(const QString& typeName) const
{
    if (typeName.isEmpty())
        return nullptr;

    if (DataTypeCustom* dataType = findTypeByName(getElements(), typeName); dataType != nullptr)
        return static_cast<DataTypeBase*>(dataType);

    if (DataTypePrimitive* primitive = findTypeByName(getPrimitiveDataTypes(), typeName); primitive != nullptr)
        return static_cast<DataTypeBase*>(primitive);

    if (DataTypeBasicObject* basic = findTypeByName(getBasicDataTypes(), typeName); basic != nullptr)
        return static_cast<DataTypeBase*>(basic);

    if (DataTypeBasicContainer* container = findTypeByName(getContainerDatTypes(), typeName); container != nullptr)
        return static_cast<DataTypeBase*>(container);

    return nullptr;
}

DataTypeBase* DataTypeDataSection::findDataType(uint32_t id) const
{
    if (id == 0)
        return nullptr;

    if (DataTypeCustom* dataType = findById(getElements(), id); dataType != nullptr)
        return static_cast<DataTypeBase*>(dataType);

    if (DataTypePrimitive* primitive = findById(getPrimitiveDataTypes(), id); primitive != nullptr)
        return static_cast<DataTypeBase*>(primitive);

    if (DataTypeBasicObject* basic = findById(getBasicDataTypes(), id); basic != nullptr)
        return static_cast<DataTypeBase*>(basic);

    if (DataTypeBasicContainer* container = findById(getContainerDatTypes(), id); container != nullptr)
        return static_cast<DataTypeBase*>(container);

    return nullptr;
}

DataTypeCustom* DataTypeDataSection::findCustomDataType(const QString& typeName) const
{
    return (typeName.isEmpty() == false ? findTypeByName(getElements(), typeName) : nullptr);
}

DataTypeCustom* DataTypeDataSection::findCustomDataType(uint32_t typeId) const
{
    return (typeId != 0 ? findById(getElements(), typeId) : nullptr);
}

DataTypeCustom* DataTypeDataSection::addCustomDataType(const QString& name, DataTypeBase::eCategory category)
{
    DataTypeCustom* dataType = DataTypeFactory::createCustomDataType(category);
    if (dataType == nullptr)
        return nullptr;

    dataType->setParent(this);
    dataType->setId(getNextId());
    dataType->setName(name);
    if (addElement(dataType, true))
        return dataType;

    delete dataType;
    return nullptr;
}

DataTypeStructure* DataTypeDataSection::addStructure(const QString& name)
{
    return static_cast<DataTypeStructure*>(addCustomDataType(name, DataTypeBase::eCategory::Structure));
}

DataTypeEnum* DataTypeDataSection::addEnum(const QString& name)
{
    return static_cast<DataTypeEnum*>(addCustomDataType(name, DataTypeBase::eCategory::Enumeration));
}

DataTypeContainer* DataTypeDataSection::addContainer(const QString& name)
{
    return static_cast<DataTypeContainer*>(addCustomDataType(name, DataTypeBase::eCategory::Container));
}

DataTypeImported* DataTypeDataSection::addImported(const QString& name)
{
    return static_cast<DataTypeImported*>(addCustomDataType(name, DataTypeBase::eCategory::Imported));
}

void DataTypeDataSection::removeAll()
{
    QList<DataTypeCustom*>& customDataTypes {getElements()};
    qDeleteAll(customDataTypes);
    customDataTypes.clear();
}

void DataTypeDataSection::validate(const DataTypeDataSection& dataTypes)
{
    for (DataTypeCustom* dataType : getElements())
    {
        dataTypes.normalizeType(dataType);
    }
}

void DataTypeDataSection::normalizeType(DataTypeCustom* dataType) const
{
    const QList<DataTypeCustom *>& customTypes{ getCustomDataTypes() };
    if (dataType->isStructure())
    {
        static_cast<DataTypeStructure *>(dataType)->validate(customTypes);
    }
    else if (dataType->isContainer())
    {
        static_cast<DataTypeContainer*>(dataType)->validate(customTypes);
    }
}
