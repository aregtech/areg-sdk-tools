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
 *  \file        lusan/model/si/SIDataTypeModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Custom Data Type Model.
 *
 ************************************************************************/

#include "lusan/model/si/SIDataTypeModel.hpp"

#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/data/si/SIDataTypeData.hpp"

SIDataTypeModel::SIDataTypeModel(SIDataTypeData& data)
    : mData(data)
{
}

int SIDataTypeModel::getDataTypes(QList<DataTypeBase *> & result, const QList<DataTypeBase::eCategory> & categories, bool makeSorting /*= false*/)
{
    return mData.getDataTypes(result, categories, makeSorting);
}

const QList<DataTypeBasicContainer*>& SIDataTypeModel::getContainerDatTypes(void) const
{
    return mData.getContainerDatTypes();
}

const QList<DataTypeCustom *>& SIDataTypeModel::getCustomDataTypes(void) const
{
    return mData.getCustomDataTypes();
}

DataTypeCustom* SIDataTypeModel::createDataType(const QString& name, DataTypeBase::eCategory category)
{
    return mData.addCustomDataType(name, category);
}

DataTypeCustom* SIDataTypeModel::insertDataType(int position, const QString& name, DataTypeBase::eCategory category)
{
    return mData.insertCustomDataType(position, name, category);
}

bool SIDataTypeModel::deleteDataType(uint32_t id)
{
    return mData.removeCustomDataType(id);
}

bool SIDataTypeModel::deleteDataType(const DataTypeCustom* dataType)
{
    return mData.removeCustomDataType(dataType->getId());
}

DataTypeCustom* SIDataTypeModel::convertDataType(DataTypeCustom* dataType, DataTypeBase::eCategory category)
{
    return mData.convertDataType(dataType, category);
}

DataTypeCustom* SIDataTypeModel::findDataType(const QString& name)
{
    DataTypeCustom** result = mData.findElement(name);
    return (result != nullptr ? *result : nullptr);
}

const DataTypeCustom* SIDataTypeModel::findDataType(const QString& name) const
{
    DataTypeCustom** result = mData.findElement(name);
    return (result != nullptr ? *result : nullptr);
}

DataTypeCustom* SIDataTypeModel::findDataType(uint32_t id)
{
    DataTypeCustom** result = mData.findElement(id);
    return (result != nullptr ? *result : nullptr);
}

const DataTypeCustom* SIDataTypeModel::findDataType(uint32_t id) const
{
    DataTypeCustom** result = mData.findElement(id);
    return (result != nullptr ? *result : nullptr);
}

const DataTypeBase* SIDataTypeModel::findStructFieldType(const DataTypeStructure* dataType, const QString& childName) const
{
    if (dataType == nullptr)
        return nullptr;
    
    FieldEntry* entry = dataType->findElement(childName);
    return (entry != nullptr ? mData.findDataType(entry->getId()) : nullptr);
}

DataTypeBase* SIDataTypeModel::findStructFieldType(const DataTypeStructure* dataType, const QString& childName)
{
    if (dataType == nullptr)
        return nullptr;
    
    FieldEntry* entry = dataType->findElement(childName);
    return (entry != nullptr ? mData.findDataType(entry->getId()) : nullptr);
}

const DataTypeBase* SIDataTypeModel::findStructFieldType(const DataTypeStructure* dataType, uint32_t childId) const
{
    if (dataType == nullptr)
        return nullptr;
    
    FieldEntry* entry = dataType->findElement(childId);
    return (entry != nullptr ? mData.findDataType(entry->getId()) : nullptr);
}

DataTypeBase* SIDataTypeModel::findStructFieldType(const DataTypeStructure* dataType, uint32_t childId)
{
    if (dataType == nullptr)
        return nullptr;
    
    FieldEntry* entry = dataType->findElement(childId);
    return (entry != nullptr ? mData.findDataType(entry->getId()) : nullptr);
}

void SIDataTypeModel::sortByName(bool ascending)
{
    mData.sortByName(ascending);
}

void SIDataTypeModel::sortById(bool ascending)
{
    mData.sortById(ascending);
}

const QList<DataTypeCustom*>& SIDataTypeModel::getDataTypes(void) const
{
    return mData.getCustomDataTypes();
}

int SIDataTypeModel::getDataTypeCount(void) const
{
    return static_cast<int>(mData.getCustomDataTypes().size());
}

ElementBase* SIDataTypeModel::ceateDataTypeChild(DataTypeCustom* dataType, const QString& name)
{
    ElementBase* result{nullptr};
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        result = static_cast<DataTypeStructure*>(dataType)->addField(name);
        if (result != nullptr)
        {
            static_cast<FieldEntry *>(result)->validate(getCustomDataTypes());
        }
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        result = static_cast<DataTypeEnum*>(dataType)->addField(name);
    }

    return result;
}

ElementBase* SIDataTypeModel::insertDataTypeChild(int position, DataTypeCustom* dataType, const QString& name)
{
    ElementBase* result{ nullptr };
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        result = static_cast<DataTypeStructure*>(dataType)->insertField(position, name);
        if (result != nullptr)
        {
            static_cast<FieldEntry*>(result)->validate(getCustomDataTypes());
        }
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        result = static_cast<DataTypeEnum*>(dataType)->insertField(position, name);
    }

    return result;
}

void SIDataTypeModel::deleteDataTypeChild(DataTypeCustom* dataType, uint32_t childId)
{
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        static_cast<DataTypeStructure*>(dataType)->removeElement(childId);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        static_cast<DataTypeEnum*>(dataType)->removeElement(childId);
    }
}

void SIDataTypeModel::deleteDataTypeChild(DataTypeCustom* dataType, const ElementBase& child)
{
    deleteDataTypeChild(dataType, child.getId());
}

ElementBase* SIDataTypeModel::findDataTypeChild(DataTypeCustom* dataType, uint32_t childId)
{
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<DataTypeStructure*>(dataType)->findElement(childId);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<DataTypeEnum*>(dataType)->findElement(childId);
    }

    return nullptr;
}

const ElementBase* SIDataTypeModel::findDataTypeChild(DataTypeCustom* dataType, uint32_t childId) const
{
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<DataTypeStructure*>(dataType)->findElement(childId);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<DataTypeEnum*>(dataType)->findElement(childId);
    }

    return nullptr;
}

const QList<FieldEntry>& SIDataTypeModel::getStructChildren(DataTypeCustom* dataType) const
{
    static const QList<FieldEntry> _empty{};
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<DataTypeStructure*>(dataType)->getElements();
    }
    else
    {
        return _empty;
    }
}

const QList<EnumEntry>& SIDataTypeModel::getEnumChildren(DataTypeCustom* dataType) const
{
    static const QList<EnumEntry> _empty{};
    if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<DataTypeEnum*>(dataType)->getElements();
    }
    else
    {
        return _empty;
    }
}

void SIDataTypeModel::sortDataTypeChildren(DataTypeCustom* dataType, bool ascending)
{
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        static_cast<DataTypeStructure*>(dataType)->sortElementsByName(ascending);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        static_cast<DataTypeEnum*>(dataType)->sortElementsByName(ascending);
    }
}

bool SIDataTypeModel::hasChildren(const DataTypeCustom* dataType) const
{
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<const DataTypeStructure*>(dataType)->hasElements();
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<const DataTypeEnum*>(dataType)->hasElements();
    }

    return false;
}

bool SIDataTypeModel::canHaveChildren(const DataTypeCustom* dataType) const
{
    return (dataType->getCategory() == DataTypeBase::eCategory::Structure) || (dataType->getCategory() == DataTypeBase::eCategory::Enumeration);
}

int SIDataTypeModel::getChildCount(const DataTypeCustom* dataType) const
{
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<int>(static_cast<const DataTypeStructure*>(dataType)->getElements().size());
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<int>(static_cast<const DataTypeEnum*>(dataType)->getElements().size());
    }

    return 0;
}

int SIDataTypeModel::findIndex(uint32_t id) const
{
    return mData.findIndex(id);
}

int SIDataTypeModel::findIndex(const DataTypeCustom* dataType) const
{
    return mData.findIndex(dataType->getId());
}

int SIDataTypeModel::findChildIndex(const DataTypeCustom* dataType, uint32_t childId) const
{
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<const DataTypeStructure*>(dataType)->findIndex(childId);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<const DataTypeEnum*>(dataType)->findIndex(childId);
    }

    return -1;
}

int SIDataTypeModel::findChildIndex(const DataTypeCustom* dataType, const ElementBase& child) const
{
    return findChildIndex(dataType, child.getId());
}

int SIDataTypeModel::findChildIndex(const DataTypeCustom* dataType, const QString& childName) const
{
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<const DataTypeStructure*>(dataType)->findIndex(childName);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<const DataTypeEnum*>(dataType)->findIndex(childName);
    }

    return -1;
}

ElementBase* SIDataTypeModel::findChild(const DataTypeCustom* dataType, uint32_t childId) const
{
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<ElementBase *>(static_cast<const DataTypeStructure*>(dataType)->findElement(childId));
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<ElementBase*>(static_cast<const DataTypeEnum*>(dataType)->findElement(childId));
    }

    return nullptr;
}

ElementBase* SIDataTypeModel::findChild(const DataTypeCustom* dataType, const QString& childName) const
{
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<ElementBase*>(static_cast<const DataTypeStructure*>(dataType)->findElement(childName));
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<ElementBase*>(static_cast<const DataTypeEnum*>(dataType)->findElement(childName));
    }

    return nullptr;
}

void SIDataTypeModel::updateDataType(DataTypeCustom* dataType, const QString& newName)
{
    mData.updateDataType(dataType, newName);
}

void SIDataTypeModel::updateDataType(uint32_t id, const QString& newName)
{
    mData.updateDataType(id, newName);
}

void SIDataTypeModel::normalizeDataType(DataTypeCustom* dataType) const
{
    mData.normalizeType(dataType);
}

DataTypeBase* SIDataTypeModel::getTypeFromName(const QString & typeName)
{
    return mData.findDataType(typeName);
}

const DataTypeBase* SIDataTypeModel::getTypeFromName(const QString& typeName) const
{
    return mData.findDataType(typeName);
}

DataTypeBase* SIDataTypeModel::getTypeFromId(uint32_t typeId)
{
    return mData.findDataType(typeId);
}

const DataTypeBase* SIDataTypeModel::getTypeFromId(uint32_t typeId) const
{
    return mData.findDataType(typeId);
}

DataTypeCustom* SIDataTypeModel::getCustomTypeFromName(const QString & typeName)
{
    return mData.findCustomDataType(typeName);
}

const DataTypeCustom* SIDataTypeModel::getCustomTypeFromName(const QString& typeName) const
{
    return mData.findCustomDataType(typeName);
}

DataTypeCustom* SIDataTypeModel::getCustomTypeFromId(uint32_t typeId)
{
    return mData.findCustomDataType(typeId);
}

const DataTypeCustom* SIDataTypeModel::getCustomTypeFromId(uint32_t typeId) const
{
    return mData.findCustomDataType(typeId);
}

void SIDataTypeModel::swapDataTypes(uint32_t firstId, uint32_t secondId)
{
    mData.swapElements(firstId, secondId);
}

void SIDataTypeModel::swapDataTypes(const DataTypeCustom& first, const DataTypeCustom& second)
{
    mData.swapElements(first.getId(), second.getId());
}

void SIDataTypeModel::swapStructureFields(DataTypeStructure& dataType, uint32_t firstId, uint32_t secondId)
{
    dataType.swapElements(firstId, secondId);
}

void SIDataTypeModel::swapStructureFields(DataTypeStructure& dataType, const FieldEntry& first, const FieldEntry& second)
{
    dataType.swapElements(first, second);
}

void SIDataTypeModel::swapEnumFields(DataTypeEnum& dataType, uint32_t firstId, uint32_t secondId)
{
    dataType.swapElements(firstId, secondId);
}

void SIDataTypeModel::swapEnumFields(DataTypeEnum& dataType, const EnumEntry& first, const EnumEntry& second)
{
    dataType.swapElements(first, second);
}
