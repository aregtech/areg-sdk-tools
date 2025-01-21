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
    : mConstantData(constantData)
    , mDataTypeData(dataTypeData)
{
}

uint32_t SIConstantModel::createConstant(const QString& name)
{
    uint32_t id = mConstantData.getNextId();
    ConstantEntry entry(id, name, &mConstantData);
    return (mConstantData.addElement(std::move(entry), true) ? id : 0);
}

bool SIConstantModel::deleteConstant(uint32_t id)
{
    return mConstantData.removeElement(id);
}

const QList<ConstantEntry>& SIConstantModel::getConstants(void) const
{
    return mConstantData.getElements();
}

const ConstantEntry* SIConstantModel::findConstant(uint32_t id) const
{
    return mConstantData.findElement(id);
}

ConstantEntry* SIConstantModel::findConstant(uint32_t id)
{
    return mConstantData.findElement(id);
}

void SIConstantModel::sortConstants(bool ascending)
{
    mConstantData.sortElementsByName(ascending);
}
