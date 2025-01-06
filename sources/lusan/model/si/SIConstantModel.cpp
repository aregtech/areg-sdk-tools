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

bool SIConstantModel::createConstant(const QString& name, const QString& type, const QString& value, bool isDeprecated, const QString& description, const QString& deprecateHint)
{
    if (mConstantData.exists(name))
        return false;

    ConstantEntry entry(0, name, type, value, isDeprecated, description, deprecateHint);
    mConstantData.addConstant(std::move(entry));
    return true;
}

bool SIConstantModel::deleteConstant(const QString& name)
{
    return mConstantData.removeConstant(name);
}

bool SIConstantModel::updateConstantValue(const QString& name, const QString& value)
{
    return mConstantData.updateValue(name, value);
}

bool SIConstantModel::updateConstantType(const QString& name, const QString& type)
{
    return mConstantData.updateType(name, type);
}

bool SIConstantModel::addContant(ConstantEntry&& newEntry, bool unique /*= true*/)
{
    return mConstantData.addConstant(std::move(newEntry), unique);
}

bool SIConstantModel::updateConstantName(const QString& oldName, const QString& newName)
{
    return mConstantData.updateName(oldName, newName);
}

bool SIConstantModel::updateConstantDeprecation(const QString& name, bool isDeprecated)
{
    return mConstantData.updateDeprecation(name, isDeprecated);
}

bool SIConstantModel::updateConstantDescription(const QString& name, const QString& description)
{
    return mConstantData.updateDescription(name, description);
}

bool SIConstantModel::updateConstantDeprecateHint(const QString& name, const QString& deprecateHint)
{
    return mConstantData.updateDeprecateHint(name, deprecateHint);
}

bool SIConstantModel::updateConstant(const QString& name, const QString& type, const QString& value, bool isDeprecated, const QString& description, const QString& deprecateHint)
{
    return mConstantData.update(name, type, value, isDeprecated, description, deprecateHint);
}

void SIConstantModel::getConstantTypes(QStringList& out_dataTypes) const
{
    QList<DataTypeBase*> dataTypes;
    mDataTypeData.getDataType(dataTypes, QList<DataTypeBase*>(), false);
    int index = static_cast<int>(out_dataTypes.size());
    out_dataTypes.resize(out_dataTypes.size() + dataTypes.size());
    
    for (const auto& dataType : dataTypes)
    {
        out_dataTypes[index ++] = dataType->getName();
    }
}

const QList<ConstantEntry>& SIConstantModel::getConstants(void) const
{
    return mConstantData.getConstants();
}

const ConstantEntry* SIConstantModel::findConstant(const QString& name) const
{
    return mConstantData.exists(name) ? &mConstantData.getConstant(name) : nullptr;
}
