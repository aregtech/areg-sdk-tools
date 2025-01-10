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

DataTypeCustom* SIDataTypeModel::createDataType(const QString& name, DataTypeBase::eCategory category)
{
    return mData.addCustomDataType(name, category);
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
    int index = mData.findCustomDataType(name);
    return (index < 0 ? nullptr : mData.getCustomDataTypes().at(index));
}

const DataTypeCustom* SIDataTypeModel::findDataType(const QString& name) const
{
    int index = mData.findCustomDataType(name);
    return (index < 0 ? nullptr : mData.getCustomDataTypes().at(index));
}

DataTypeCustom* SIDataTypeModel::findDataType(uint32_t id)
{
    int index = mData.findCustomDataType(id);
    return (index < 0 ? nullptr : mData.getCustomDataTypes().at(index));
}

const DataTypeCustom* SIDataTypeModel::findDataType(uint32_t id) const
{
    int index = mData.findCustomDataType(id);
    return (index < 0 ? nullptr : mData.getCustomDataTypes().at(index));
}

void SIDataTypeModel::sortDataTypes(bool ascending)
{
    mData.sort(ascending);
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
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<DataTypeStructure*>(dataType)->addField(name);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<DataTypeEnum*>(dataType)->addField(name);
    }

    return nullptr;
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

QList<FieldEntry> SIDataTypeModel::getStructChildren(DataTypeCustom* dataType) const
{
    QList<FieldEntry> result;
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        result = static_cast<DataTypeStructure*>(dataType)->getElements();
    }

    return result;
}

QList<EnumEntry> SIDataTypeModel::getEnumChildren(DataTypeCustom* dataType) const
{
    QList<EnumEntry> result;
    if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        result = static_cast<DataTypeEnum*>(dataType)->getElements();
    }

    return result;
}

int SIDataTypeModel::getDataTypeChildCount(const DataTypeCustom* dataType) const
{
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<const DataTypeStructure*>(dataType)->getElements().size();
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<const DataTypeEnum*>(dataType)->getElements().size();
    }

    return 0;
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
    return mData.findCustomDataType(id);
}

int SIDataTypeModel::findIndex(const DataTypeCustom* dataType) const
{
    return mData.findCustomDataType(dataType->getId());
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
