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
 *  \file        lusan/data/common/ParamType.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, parameter type.
 *
 ************************************************************************/

#include "lusan/data/common/ParamType.hpp"

#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypePrimitive.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"

namespace
{
    template<typename DataType>
    inline DataType* findDataType(const QList<DataType*>& dataTypes, const QString& typeName)
    {
        auto it = std::find_if(dataTypes.begin(), dataTypes.end(), [typeName](const DataType* item) -> bool { return (item != nullptr) && (item->getName() == typeName); });
        return (it != dataTypes.end() ? (*it) : nullptr);
    }
}

//////////////////////////////////////////////////////////////////////////
// TypeFinder class implementation
//////////////////////////////////////////////////////////////////////////

DataTypeBase* TypeFinder::findObject(const QString name, const QList<DataTypeCustom*>& listTypes) const
{
    DataTypeBase* dataType{ nullptr };
    
    dataType = static_cast<DataTypeBase*>(findDataType(listTypes, name));
    if (dataType != nullptr)
    {
        return dataType;
    }

    dataType = static_cast<DataTypeBase*>(findDataType(DataTypeFactory::getPrimitiveTypes(), name));
    if (dataType != nullptr)
    {
        return dataType;
    }

    dataType = static_cast<DataTypeBase*>(findDataType(DataTypeFactory::getBasicTypes(), name));
    if (dataType != nullptr)
    {
        return dataType;
    }
    
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// ParamType class implementation
//////////////////////////////////////////////////////////////////////////


ParamType::ParamType(const QString& typeName)
    : TETypeWrap<DataTypeBase, DataTypeCustom, TypeFinder>(typeName)
{
}

ParamType::ParamType(const QString& typeName, const QList<DataTypeCustom*>& customTypes)
    : TETypeWrap<DataTypeBase, DataTypeCustom, TypeFinder>(typeName, customTypes)
{
}

ParamType::ParamType(DataTypeBase* dataType)
    : TETypeWrap<DataTypeBase, DataTypeCustom, TypeFinder>(dataType)
{
}

ParamType& ParamType::operator = (const DataTypeBase* dataType)
{
    setType(const_cast<DataTypeBase*>(dataType));
    return (*this);
}

ParamType& ParamType::operator = (DataTypeBase* dataType)
{
    setType(dataType);
    return (*this);
}

ParamType& ParamType::operator = (const QString& typeName)
{
    setName(typeName);
    return (*this);
}

bool ParamType::operator == (const ParamType& other) const
{
    if (this == &other)
    {
        return true;
    }
    else if ((mTypeObj == other.mTypeObj) && (mTypeObj != nullptr))
    {
        return true;
    }
    else if ((mTypeObj != nullptr) && (other.mTypeObj != nullptr) && (mTypeObj->getName() == other.mTypeObj->getName()))
    {
        return true;
    }
    else if ((mTypeName == other.mTypeName) && (mTypeName.isEmpty() == false))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool ParamType::operator != (const ParamType& other) const
{
    return !(operator == (other));
}

bool ParamType::operator == (const DataTypeBase* dataType) const
{
    return (mTypeObj != nullptr) && (mTypeObj == dataType);
}

bool ParamType::operator != (const DataTypeBase* dataType) const
{
    return !(operator == (dataType));
}

bool ParamType::operator == (const QString& typeName) const
{
    if (mTypeObj != nullptr)
    {
        return (mTypeObj->getName() == typeName);
    }
    else
    {
        return (mTypeName.isEmpty() == false) && (mTypeName == typeName);
    }
}

bool ParamType::operator != (const QString& typeName) const
{
    return !(operator == (typeName));
}

ParamType::operator const DataTypeCustom* (void) const
{
    return (isCustomDefined() ? static_cast<const DataTypeCustom*>(mTypeObj) : static_cast<const DataTypeCustom *>(nullptr));
}

ParamType::operator DataTypeCustom* (void)
{
    return (isCustomDefined() ? static_cast<DataTypeCustom*>(mTypeObj) : static_cast<DataTypeCustom*>(nullptr));
}

ParamType::operator const DataTypeStructure* (void) const
{
    return (isStructure() ? static_cast<const DataTypeStructure*>(mTypeObj) : static_cast<const DataTypeStructure*>(nullptr));
}

ParamType::operator DataTypeStructure* (void)
{
    return (isStructure() ? static_cast<DataTypeStructure*>(mTypeObj) : static_cast<DataTypeStructure*>(nullptr));
}

ParamType::operator const DataTypeEnum* (void) const
{
    return (isEnumeration() ? static_cast<const DataTypeEnum*>(mTypeObj) : static_cast<const DataTypeEnum*>(nullptr));
}

ParamType::operator DataTypeEnum* (void)
{
    return (isEnumeration() ? static_cast<DataTypeEnum*>(mTypeObj) : static_cast<DataTypeEnum*>(nullptr));
}

ParamType::operator const DataTypeContainer* (void) const
{
    return (isContainer() ? static_cast<const DataTypeContainer*>(mTypeObj) : static_cast<const DataTypeContainer*>(nullptr));
}

ParamType::operator DataTypeContainer* (void)
{
    return (isContainer() ? static_cast<DataTypeContainer*>(mTypeObj) : static_cast<DataTypeContainer*>(nullptr));
}

ParamType::operator const DataTypeImported* (void) const
{
    return (isImported() ? static_cast<const DataTypeImported*>(mTypeObj) : static_cast<const DataTypeImported*>(nullptr));
}

ParamType::operator DataTypeImported* (void)
{
    return (isImported() ? static_cast<DataTypeImported*>(mTypeObj) : static_cast<DataTypeImported*>(nullptr));
}
