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
 *  \file        lusan/model/si/SIConstantModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Constant Model.
 *
 ************************************************************************/

#include "lusan/model/si/SIConstantModel.hpp"

SIConstantModel::SIConstantModel(SIConstantData& constantData, SIDataTypeData& dataTypeData)
    : mData(constantData)
    , mDataType(dataTypeData)
{
}

ConstantEntry * SIConstantModel::createConstant(const QString& name)
{
    ConstantEntry* result = mData.createConstant(name);
    if (result != nullptr)
    {
        result->validate(mDataType.getCustomDataTypes());
    }
    
    return result;
}

bool SIConstantModel::deleteConstant(uint32_t id)
{
    return mData.removeElement(id);
}

const QList<ConstantEntry>& SIConstantModel::getConstants(void) const
{
    return mData.getElements();
}

const ConstantEntry* SIConstantModel::findConstant(uint32_t id) const
{
    return mData.findElement(id);
}

ConstantEntry* SIConstantModel::findConstant(uint32_t id)
{
    return mData.findElement(id);
}

void SIConstantModel::sortConstants(bool ascending)
{
    mData.sortElementsByName(ascending);
}

QList<uint32_t> SIConstantModel::replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType)
{
    return std::move(mData.replaceDataType(oldDataType, newDataType));
}

void SIConstantModel::swapConstants(uint32_t firstId, uint32_t secondId)
{
    mData.swapElements(firstId, secondId);
}

void SIConstantModel::swapConstants(const ConstantEntry& first, const ConstantEntry& second)
{
    mData.swapElements(first, second);
}
