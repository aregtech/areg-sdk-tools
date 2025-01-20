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

#include <algorithm>

SIDataTypeData::SIDataTypeData(ElementBase* parent /*= nullptr*/)
    : ElementBase       (parent)
    , QObject           ( )
    , mCustomDataTypes  ( )
    , mDataTypes        ( )
{
}

SIDataTypeData::SIDataTypeData(QList<DataTypeCustom *>&& entries, ElementBase* parent /*= nullptr*/) noexcept
    : ElementBase       (parent)
    , QObject           ( )
    , mCustomDataTypes  (std::move(entries))
{
    for (DataTypeCustom* entry : mCustomDataTypes)
    {
        Q_ASSERT(entry != nullptr);
        if (entry->getParent() != this)
        {
            entry->setParent(this);
            entry->setId(getNextId());
        }
    }
}

SIDataTypeData::~SIDataTypeData(void)
{
    qDeleteAll(mCustomDataTypes);
    mCustomDataTypes.clear();
}

int SIDataTypeData::findCustomDataType(const DataTypeCustom& entry) const
{
    return mCustomDataTypes.indexOf(&entry);
}

int SIDataTypeData::findCustomDataType(uint32_t id) const
{
    for (int i = 0; i < static_cast<int>(mCustomDataTypes.size()); ++i)
    {
        if (mCustomDataTypes[i]->getId() == id)
        {
            return i;
        }
    }

    return -1;
}

int SIDataTypeData::findCustomDataType(const QString& name) const
{
    for (int i = 0; i < static_cast<int>(mCustomDataTypes.size()); ++i)
    {
        if (mCustomDataTypes[i]->getName() == name)
        {
            return i;
        }
    }

    return -1;
}

void SIDataTypeData::addCustomDataType(DataTypeCustom * entry)
{
    if (entry != nullptr)
    {
        if (entry->getParent() != this)
        {
            entry->setParent(this);
            entry->setId(getNextId());
        }

        mCustomDataTypes.append(entry);
        emit signalDataTypeCreated(entry);
    }
}

bool SIDataTypeData::removeCustomDataType(const DataTypeCustom& entry)
{
    int index = findCustomDataType(entry);
    if (index != -1)
    {
        DataTypeCustom* dataType = mCustomDataTypes[index];
        mCustomDataTypes.removeAt(index);
        emit signalDataTypeRemoved(dataType);
        delete dataType;
        return true;
    }

    return false;
}

bool SIDataTypeData::removeCustomDataType(uint32_t id)
{
    for (int i = 0; i < static_cast<int>(mCustomDataTypes.size()); ++ i)
    {
        if (mCustomDataTypes[i]->getId() == id)
        {
            DataTypeCustom* dataType = mCustomDataTypes[i];
            mCustomDataTypes.removeAt(i);
            emit signalDataTypeRemoved(dataType);
            delete dataType;
            return true;
        }
    }
    
    return false;
}

bool SIDataTypeData::replaceCustomDataType(const DataTypeCustom& oldEntry, DataTypeCustom * newEntry)
{
    if (&oldEntry != newEntry)
    {
        return true;
    }

    int index = findCustomDataType(oldEntry);
    if (index != -1)
    {
        DataTypeCustom* dataType = mCustomDataTypes[index];
        mCustomDataTypes[index] = newEntry;
        if (newEntry->getParent() != this)
        {
            newEntry->setParent(this);
            newEntry->setId(getNextId());
        }
        
        emit signalDataTypeConverted(dataType, newEntry);        
        delete dataType;
        return true;
    }

    return false;
}

bool SIDataTypeData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.isStartElement() && (xml.name() == XmlSI::xmlSIElementDataTypeList))
    {
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
                        mCustomDataTypes.append(dataType);
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

    return false;
}

void SIDataTypeData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementDataTypeList);
    for (const DataTypeCustom* dataType : mCustomDataTypes)
    {
        dataType->writeToXml(xml);
    }

    xml.writeEndElement();
}

void SIDataTypeData::removeAll(void)
{
    qDeleteAll(mCustomDataTypes);
    mCustomDataTypes.clear();
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
    return mCustomDataTypes;
}

void SIDataTypeData::setCustomDataTypes(QList<DataTypeCustom*>&& entries)
{
    mCustomDataTypes = std::move(entries);
    for (DataTypeCustom* entry : mCustomDataTypes)
    {
        Q_ASSERT(entry != nullptr);
        if (entry->getParent() != this)
        {
            entry->setParent(this);
            entry->setId(getNextId());
        }
    }
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

    for (DataTypeCustom* dataType : mCustomDataTypes)
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
            for (DataTypeCustom* dataType : mCustomDataTypes)
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
    if (existsCustom(mCustomDataTypes, typeName))
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
    if (existsCustom(mCustomDataTypes, id))
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
    for (DataTypeCustom* dataType : mCustomDataTypes)
    {
        if (dataType->getName() == typeName)
        {
            return static_cast<DataTypeBase *>(dataType);
        }
    }

    const QList<DataTypePrimitive*>& primitives = getPrimitiveDataTypes();
    for (DataTypePrimitive* dataType : primitives)
    {
        if (dataType->getName() == typeName)
        {
            return static_cast<DataTypeBase *>(dataType);
        }
    }

    const QList<DataTypeBasicObject*>& basics = getBasicDataTypes();
    for (DataTypeBasicObject* dataType : basics)
    {
        if (dataType->getName() == typeName)
        {
            return static_cast<DataTypeBase *>(dataType);
        }
    }

    const QList<DataTypeBasicContainer*>& containers = getContainerDatTypes();
    for (DataTypeBasicContainer* dataType : containers)
    {
        if (dataType->getName() == typeName)
        {
            return static_cast<DataTypeBase *>(dataType);
        }
    }

    return nullptr;
}

DataTypeBase* SIDataTypeData::findDataType(uint32_t id) const
{
    for (DataTypeCustom* dataType : mCustomDataTypes)
    {
        if (dataType->getId() == id)
        {
            return static_cast<DataTypeBase*>(dataType);
        }
    }

    const QList<DataTypePrimitive*>& primitives = getPrimitiveDataTypes();
    for (DataTypePrimitive* dataType : primitives)
    {
        if (dataType->getId() == id)
        {
            return static_cast<DataTypeBase*>(dataType);
        }
    }

    const QList<DataTypeBasicObject*>& basics = getBasicDataTypes();
    for (DataTypeBasicObject* dataType : basics)
    {
        if (dataType->getId() == id)
        {
            return static_cast<DataTypeBase*>(dataType);
        }
    }

    const QList<DataTypeBasicContainer*>& containers = getContainerDatTypes();
    for (DataTypeBasicContainer* dataType : containers)
    {
        if (dataType->getId() == id)
        {
            return static_cast<DataTypeBase*>(dataType);
        }
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
    mCustomDataTypes.append(dataType);
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
    int i = mCustomDataTypes.indexOf(dataType);
    if (i != -1)
    {
        mCustomDataTypes[i] = newType;
    }
    else
    {
        mCustomDataTypes.append(newType);
    }
    
    emit signalDataTypeConverted(dataType, newType);
    
    return newType;
}

void SIDataTypeData::sortByName(bool ascending)
{
    NELusanCommon::sortByName(mCustomDataTypes, true);
}

void SIDataTypeData::sortById(bool ascending)
{
    NELusanCommon::sortById(mCustomDataTypes, true);
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
