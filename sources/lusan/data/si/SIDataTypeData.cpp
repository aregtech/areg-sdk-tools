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
 *  \file        lusan/data/si/SIConstantData.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Constant Data.
 *
 ************************************************************************/

#include "lusan/data/si/SIDataTypeData.hpp"
#include "lusan/common/XmlSI.hpp"
#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypePrimitive.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

SIDataTypeData::SIDataTypeData(ElementBase* parent /*= nullptr*/)
    : QObject           ( )
    , TEDataContainer<DataTypeCustom*, ElementBase>(parent)
{
}

SIDataTypeData::SIDataTypeData(QList<DataTypeCustom *>&& entries, ElementBase* parent /*= nullptr*/) noexcept
    : QObject           ( )
    , TEDataContainer<DataTypeCustom*, ElementBase>(std::move(entries), parent)
{
}

SIDataTypeData::~SIDataTypeData(void)
{
    QList<DataTypeCustom*>& list = getElements();;
    qDeleteAll(list);
}

void SIDataTypeData::addCustomDataType(DataTypeCustom * entry)
{
    if (addElement(entry, true))
    {
        emit signalDataTypeCreated(entry);
    }
}

bool SIDataTypeData::removeCustomDataType(const DataTypeCustom& entry)
{
    return removeCustomDataType(entry.getId());
}

bool SIDataTypeData::removeCustomDataType(uint32_t id)
{
    DataTypeCustom* dataType = nullptr;
    if (removeElement(id, &dataType))
    {
        Q_ASSERT(dataType != nullptr);
        emit signalDataTypeRemoved(dataType);
        delete dataType;
        return true;
    }
    
    return false;
}

bool SIDataTypeData::replaceCustomDataType(DataTypeCustom* oldEntry, DataTypeCustom * newEntry)
{
    return replaceElement(oldEntry, newEntry, true);
}

bool SIDataTypeData::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementDataTypeList))
        return false;

    while (xml.readNextStartElement())
    {
        if (xml.name() == XmlSI::xmlSIElementDataType)
        {
            QString type = xml.attributes().value(XmlSI::xmlSIAttributeType).toString();
            DataTypeCustom* dataType = DataTypeFactory::createCustomDataType(type);
            if (dataType != nullptr)
            {
                dataType->setParent(this);
                if (dataType->readFromXml(xml))
                {
                    addElement(dataType);
                }
                else
                {
                    xml.skipCurrentElement();
                }
            }
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    return true;
}

void SIDataTypeData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementDataTypeList);
    const QList<DataTypeCustom*>& customDataTypes {getElements()};
    for (const DataTypeCustom* dataType : customDataTypes)
    {
        dataType->writeToXml(xml);
    }

    xml.writeEndElement();
}

void SIDataTypeData::removeAll(void)
{
    QList<DataTypeCustom*>& customDataTypes {getElements()};
    qDeleteAll(customDataTypes);
    customDataTypes.clear();
}

const QList<DataTypePrimitive*>& SIDataTypeData::getPrimitiveDataTypes(void) const
{
    return DataTypeFactory::getPrimitiveTypes();
}

const QList<DataTypeBasicObject*>& SIDataTypeData::getBasicDataTypes(void) const
{
    return DataTypeFactory::getBasicTypes();
}

const QList<DataTypeBasicContainer*>& SIDataTypeData::getContainerDatTypes(void) const
{
    return DataTypeFactory::getContainerTypes();
}

const QList<DataTypeCustom*>& SIDataTypeData::getCustomDataTypes(void) const
{
    return getElements();
}

void SIDataTypeData::setCustomDataTypes(QList<DataTypeCustom*>&& entries)
{
    setElements(std::move(entries));
}

void SIDataTypeData::getDataType(QList<DataTypeBase*>& out_dataTypes, const QList<DataTypeBase*>& excludes /*= QList<DataTypeBase *>*/, bool makeSorting /*= false*/) const
{
    out_dataTypes.clear();
    const QList<DataTypePrimitive*>& primitives = getPrimitiveDataTypes();
    for (DataTypePrimitive* dataType : primitives)
    {
        if (exists<DataTypeBase>(excludes, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }


    const QList<DataTypeBasicObject*>& basics = getBasicDataTypes();
    for (DataTypeBasicObject* dataType : basics)
    {
        if (exists<DataTypeBase>(excludes, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }

    const QList<DataTypeBasicContainer*>& containers = getContainerDatTypes();
    for (DataTypeBasicContainer* dataType : containers)
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

    int begin = static_cast<int>(out_dataTypes.size());
    const QList<DataTypeCustom*>& customDataTypes = getElements();
    for (DataTypeCustom* dataType : customDataTypes)
    {
        if (exists<DataTypeBase>(excludes, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }

    if (makeSorting)
    {
        if (begin < static_cast<int>(out_dataTypes.size()))
        {
            NELusanCommon::sortById<const DataTypeBase *>(out_dataTypes.begin() + begin, out_dataTypes.end(), true);
        }
    }
}

int SIDataTypeData::getDataTypes(QList<DataTypeBase*>& out_dataTypes, const QList<DataTypeBase::eCategory>& includes, bool makeSorting /*= false*/) const
{
    out_dataTypes.clear();
    for (auto category : includes)
    {
        switch(category)
        {
        case DataTypeBase::eCategory::Primitive:
        case DataTypeBase::eCategory::PrimitiveSint:
        case DataTypeBase::eCategory::PrimitiveUint:
        case DataTypeBase::eCategory::PrimitiveFloat:
        case DataTypeBase::eCategory::BasicObject:
        case DataTypeBase::eCategory::BasicContainer:
        {
            uint32_t begin {static_cast<uint32_t>(out_dataTypes.size())};
            DataTypeFactory::getPredefinedTypes(out_dataTypes, QList<DataTypeBase::eCategory>{category});
            if (makeSorting)
            {
                NELusanCommon::sortById<const DataTypeBase *>(out_dataTypes.begin() + begin, out_dataTypes.end(), true);
            }
        }
        break;
            
        case DataTypeBase::eCategory::Enumeration:
        case DataTypeBase::eCategory::Structure:
        case DataTypeBase::eCategory::Imported:
        case DataTypeBase::eCategory::Container:
        {
            uint32_t begin {static_cast<uint32_t>(out_dataTypes.size())};
            const QList<DataTypeCustom*>& customDataTypes = getElements();
            for (DataTypeCustom* dataType : customDataTypes)
            {
                if (dataType->getCategory() == category)
                {
                    out_dataTypes.append(dataType);
                }
            }
            
            if (makeSorting)
            {
                NELusanCommon::sortByName<const DataTypeBase*>(out_dataTypes.begin() + begin, out_dataTypes.end(), true);
            }
        }
        break;
        
        default:
            break;
        }
    }
    
    return static_cast<int>(out_dataTypes.size());
}

bool SIDataTypeData::existsPrimitive(const QList<DataTypePrimitive*> dataTypes, const QString& searchName) const
{
    return exists<DataTypePrimitive>(dataTypes, searchName);
}

bool SIDataTypeData::existsPrimitive(const QList<DataTypePrimitive*> dataTypes, uint32_t id) const
{
    return exists<DataTypePrimitive>(dataTypes, id);
}
    
bool SIDataTypeData::existsBasic(const QList<DataTypeBasicObject*> dataTypes, const QString& searchName) const
{
    return exists<DataTypeBasicObject>(dataTypes, searchName);
}

bool SIDataTypeData::existsBasic(const QList<DataTypeBasicObject*> dataTypes, uint32_t id) const
{
    return exists<DataTypeBasicObject>(dataTypes, id);
}

bool SIDataTypeData::existsContainer(const QList<DataTypeBasicContainer*> dataTypes, const QString& searchName) const
{
    return exists<DataTypeBasicContainer>(dataTypes, searchName);
}

bool SIDataTypeData::existsContainer(const QList<DataTypeBasicContainer*> dataTypes, uint32_t id) const
{
    return exists<DataTypeBasicContainer>(dataTypes, id);
}

bool SIDataTypeData::existsCustom(const QList<DataTypeCustom*> dataTypes, const QString& searchName) const
{
    return exists<DataTypeCustom>(dataTypes, searchName);
}

bool SIDataTypeData::existsCustom(const QList<DataTypeCustom*> dataTypes, uint32_t id) const
{
    return exists<DataTypeCustom>(dataTypes, id);
}

bool SIDataTypeData::exists(const QString& typeName) const
{
    const QList<DataTypeCustom*>& customDataTypes = getElements();
    if (existsCustom(customDataTypes, typeName))
        return true;

    const QList<DataTypePrimitive*>& primitives = getPrimitiveDataTypes();
    if (existsPrimitive(primitives, typeName))
        return true;

    const QList<DataTypeBasicObject*>& basics = getBasicDataTypes();
    if (existsBasic(basics, typeName))
        return true;

    const QList<DataTypeBasicContainer*>& containers = getContainerDatTypes();
    if (existsContainer(containers, typeName))
        return true;

    return false;
}

bool SIDataTypeData::exists(uint32_t id) const
{
    const QList<DataTypeCustom*>& customDataTypes = getElements();
    if (existsCustom(customDataTypes, id))
        return true;
    
    const QList<DataTypePrimitive*>& primitives = getPrimitiveDataTypes();
    if (existsPrimitive(primitives, id))
        return true;
    
    const QList<DataTypeBasicObject*>& basics = getBasicDataTypes();
    if (existsBasic(basics, id))
        return true;
    
    const QList<DataTypeBasicContainer*>& containers = getContainerDatTypes();
    if (existsContainer(containers, id))
        return true;
    
    return false;
}

DataTypeBase* SIDataTypeData::findDataType(const QString& typeName) const
{
    DataTypeCustom* dataType = findDataType(getElements(), typeName);
    if (dataType != nullptr)
    {
        return static_cast<DataTypeBase*>(dataType);
    }

    DataTypePrimitive* primitive = findDataType(getPrimitiveDataTypes(), typeName);
    if (primitive != nullptr)
    {
        return static_cast<DataTypeBase*>(primitive);
    }

    DataTypeBasicObject* basic = findDataType(getBasicDataTypes(), typeName);
    if (basic != nullptr)
    {
        return static_cast<DataTypeBase*>(basic);
    }

    DataTypeBasicContainer* container = findDataType(getContainerDatTypes(), typeName);
    if (container != nullptr)
    {
        return static_cast<DataTypeBase*>(container);
    }

    return nullptr;
}

DataTypeBase* SIDataTypeData::findDataType(uint32_t id) const
{
    DataTypeCustom* dataType = findDataType(getElements(), id);
    if (dataType != nullptr)
    {
        return static_cast<DataTypeBase*>(dataType);
    }

    DataTypePrimitive* primitive = findDataType(getPrimitiveDataTypes(), id);
    if (primitive != nullptr)
    {
        return static_cast<DataTypeBase*>(primitive);
    }

    DataTypeBasicObject* basic = findDataType(getBasicDataTypes(), id);
    if (basic != nullptr)
    {
        return static_cast<DataTypeBase*>(basic);
    }

    DataTypeBasicContainer* container = findDataType(getContainerDatTypes(), id);
    if (container != nullptr)
    {
        return static_cast<DataTypeBase*>(container);
    }

    return nullptr;
}

DataTypeStructure* SIDataTypeData::addStructure(const QString& name)
{
    return static_cast<DataTypeStructure*>(addCustomDataType(name, DataTypeBase::eCategory::Structure));
}

DataTypeEnum* SIDataTypeData::addEnum(const QString& name)
{
    return static_cast<DataTypeEnum*>(addCustomDataType(name, DataTypeBase::eCategory::Enumeration));
}

DataTypeContainer* SIDataTypeData::addContainer(const QString& name)
{
    return static_cast<DataTypeContainer*>(addCustomDataType(name, DataTypeBase::eCategory::Container));
}

DataTypeImported* SIDataTypeData::addImported(const QString& name)
{
    return static_cast<DataTypeImported*>(addCustomDataType(name, DataTypeBase::eCategory::Imported));
}

DataTypeCustom* SIDataTypeData::addCustomDataType(const QString& name, DataTypeBase::eCategory category)
{
    DataTypeCustom* dataType = _createType(name, this, getNextId(), category);
    addElement(dataType, false);
    emit signalDataTypeCreated(dataType);
    return dataType;
}

DataTypeCustom* SIDataTypeData::convertDataType(DataTypeCustom* dataType, DataTypeBase::eCategory category)
{
    if (dataType->getCategory() == category)
    {
        return dataType;
    }

    DataTypeCustom* newType = _createType(dataType->getName(), dataType->getParent(), dataType->getId(), category);
    if (replaceElement(dataType, newType, false) == false)
    {
        addElement(newType, false);
    }

    emit signalDataTypeConverted(dataType, newType);
    
    return newType;
}

void SIDataTypeData::sortByName(bool ascending)
{
    sortElementsByName(ascending);
}

void SIDataTypeData::sortById(bool ascending)
{
    sortElementsById(ascending);
}

DataTypeCustom* SIDataTypeData::_createType(const QString& name, ElementBase* parent, uint32_t id, DataTypeBase::eCategory category)
{
    DataTypeCustom* result = DataTypeFactory::createCustomDataType(category);
    Q_ASSERT(result != nullptr);
    result->setParent(parent);
    result->setId(id);
    result->setName(name);
    return result;
}
