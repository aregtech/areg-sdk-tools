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
#include "lusan/data/common/DataTypeDefined.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypePrimitive.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <algorithm>
#include <memory>

SIDataTypeData::SIDataTypeData(ElementBase* parent /*= nullptr*/)
    : ElementBase       (parent)
    , mCustomDataTypes  ( )
{
}

SIDataTypeData::SIDataTypeData(QList<DataTypeCustom *>&& entries, ElementBase* parent /*= nullptr*/) noexcept
    : ElementBase       (parent)
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
    }
}

bool SIDataTypeData::removeCustomDataType(const DataTypeCustom& entry)
{
    int index = findCustomDataType(entry);
    if (index != -1)
    {
        delete mCustomDataTypes[index];
        mCustomDataTypes.removeAt(index);
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
            delete mCustomDataTypes[i];
            mCustomDataTypes.removeAt(i);
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
        delete mCustomDataTypes[index];
        mCustomDataTypes[index] = newEntry;
        if (newEntry->getParent() != this)
        {
            newEntry->setParent(this);
            newEntry->setId(getNextId());
        }

        return true;
    }

    return false;
}

bool SIDataTypeData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.readNextStartElement() && xml.name() == XmlSI::xmlSIElementDataTypeList)
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

void SIDataTypeData::getDataType(QList<DataTypeBase*>& out_dataTypes, const QList<DataTypeBase*>& excludes /*= QList<DataTypeCustom *>*/, bool makeSorting /*= true*/) const
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

    for (DataTypeCustom* dataType : mCustomDataTypes)
    {
        if (exists<DataTypeBase>(excludes, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }

    if (makeSorting)
    {
        std::sort(out_dataTypes.begin(), out_dataTypes.end(), [](const DataTypeBase* lhs, const DataTypeBase* rhs) -> bool
            {
                Q_ASSERT(lhs->getName() != rhs->getName());
                return lhs->getName() < rhs->getName();
            });
    }
}

bool SIDataTypeData::existsPrimitive(const QList<DataTypePrimitive*> dataTypes, const QString& searchName) const
{
    return exists<DataTypePrimitive>(dataTypes, searchName);
}

bool SIDataTypeData::existsBasic(const QList<DataTypeBasicObject*> dataTypes, const QString& searchName) const
{
    return exists<DataTypeBasicObject>(dataTypes, searchName);
}

bool SIDataTypeData::existsContainer(const QList<DataTypeBasicContainer*> dataTypes, const QString& searchName) const
{
    return exists<DataTypeBasicContainer>(dataTypes, searchName);
}

bool SIDataTypeData::existsCustom(const QList<DataTypeCustom*> dataTypes, const QString& searchName) const
{
    return exists<DataTypeCustom>(dataTypes, searchName);
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

DataTypeDefined* SIDataTypeData::addDefined(const QString& name)
{
    return static_cast<DataTypeDefined*>(addCustomDataType(name, DataTypeBase::eCategory::Container));
}

DataTypeImported* SIDataTypeData::addImported(const QString& name)
{
    return static_cast<DataTypeImported*>(addCustomDataType(name, DataTypeBase::eCategory::Imported));
}

DataTypeCustom* SIDataTypeData::addCustomDataType(const QString& name, DataTypeBase::eCategory category)
{
    DataTypeCustom* dataType = _createType(name, this, getNextId(), category);
    mCustomDataTypes.append(dataType);
    return dataType;
}

DataTypeCustom* SIDataTypeData::convertDataType(DataTypeCustom* dataType, DataTypeBase::eCategory category)
{
    DataTypeCustom* newType = _createType(dataType->getName(), dataType->getParent(), dataType->getId(), category);
    int i = mCustomDataTypes.indexOf(dataType);
    if (i != -1)
    {
        mCustomDataTypes[i] = newType;
        delete dataType;
    }
    else
    {
        mCustomDataTypes.append(newType);
    }
    
    return newType;
}

DataTypeCustom* SIDataTypeData::_createType(const QString& name, ElementBase* parent, uint32_t id, DataTypeBase::eCategory category)
{
    Q_ASSERT(findDataType(id) == nullptr);

    DataTypeCustom* result = DataTypeFactory::createCustomDataType(category);
    Q_ASSERT(result != nullptr);
    result->setParent(parent);
    result->setId(id);
    result->setName(name);
    return result;
}
