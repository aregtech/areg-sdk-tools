/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/model/si/SIAttributeModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Data Attribute Model.
 *
 ************************************************************************/

 /************************************************************************
  * Includes
  ************************************************************************/
#include "lusan/model/si/SIAttributeModel.hpp"

SIAttributeModel::SIAttributeModel(SIAttributeData& attributeData, SIDataTypeData& dataTypeData)
    : mData(attributeData)
    , mDataType(dataTypeData)
{
}

AttributeEntry * SIAttributeModel::createAttribute(const QString& name, AttributeEntry::eNotification notification /*= AttributeEntry::eNotification::NotifyOnChange*/)
{
    AttributeEntry * result = mData.createAttribute(name, notification);
    if (result != nullptr)
    {
        result->validate(mDataType.getCustomDataTypes());
    }

    return result;
}

AttributeEntry* SIAttributeModel::insertAttribute(int position, const QString& name, AttributeEntry::eNotification notification /*= AttributeEntry::eNotification::NotifyOnChange*/)
{
    AttributeEntry* result = mData.insertAttribute(position, name, notification);
    if (result != nullptr)
    {
        result->validate(mDataType.getCustomDataTypes());
    }

    return result;
}

bool SIAttributeModel::deleteAttribute(uint32_t id)
{
    return mData.removeElement(id);
}

const QList<AttributeEntry>& SIAttributeModel::getAttributes(void) const
{
    return mData.getElements();
}

const AttributeEntry* SIAttributeModel::findAttribute(uint32_t id) const
{
    return mData.findElement(id);
}

AttributeEntry* SIAttributeModel::findAttribute(uint32_t id)
{
    return mData.findElement(id);
}

void SIAttributeModel::sortAttributes(bool ascending)
{
    mData.sortElementsByName(ascending);
}

QList<uint32_t> SIAttributeModel::replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType)
{
    return mData.replaceDataType(oldDataType, newDataType);
}

void SIAttributeModel::swapAttributes(uint32_t firstId, uint32_t secondId)
{
    mData.swapElements(firstId, secondId);
}

void SIAttributeModel::swapAttributes(const AttributeEntry& first, const AttributeEntry& second)
{
    mData.swapElements(first, second);
}
