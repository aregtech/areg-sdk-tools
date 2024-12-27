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
 *  \file        lusan/data/common/DataTypeBase.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type Base.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeBase.hpp"

DataTypeBase::DataTypeBase(void)
    : mCategory (eCategory::Undefined)
    , mName     ( )
{
}

DataTypeBase::DataTypeBase(eCategory category, const QString& name /*= QString()*/)
    : mCategory (category)
    , mName     (name)
{
}

DataTypeBase::DataTypeBase(const DataTypeBase& other)
    : mCategory (other.mCategory)
    , mName     (other.mName)
{
}

DataTypeBase::DataTypeBase(DataTypeBase&& other) noexcept
    : mCategory (other.mCategory)
    , mName     (std::move(other.mName))
{
}

DataTypeBase& DataTypeBase::operator = (const DataTypeBase& other)
{
    if (this != &other)
    {
        mName = other.mName;
        mCategory = other.mCategory;
    }

    return *this;
}

DataTypeBase& DataTypeBase::operator=(DataTypeBase&& other) noexcept
{
    if (this != &other)
    {
        mName = std::move(other.mName);
        mCategory = other.mCategory;
    }
   
    return *this;
}

bool DataTypeBase::operator == (const DataTypeBase& other) const
{
    return  (mCategory == other.mCategory) && (mName == other.mName);
}

bool DataTypeBase::operator != (const DataTypeBase& other) const
{
    return  (mCategory != other.mCategory) || (mName != other.mName);
}

const QString& DataTypeBase::getName(void) const
{
    return mName;
}

void DataTypeBase::setName(const QString& name)
{
    mName = name;
}

DataTypeBase::eCategory DataTypeBase::getCategory(void) const
{
    return mCategory;
}

bool DataTypeBase::isValid(void) const
{
    return !mName.isEmpty() && (mCategory != eCategory::Undefined);
}

bool DataTypeBase::isPrimitive(void) const
{
    return ((static_cast<uint16_t>(mCategory) & static_cast<uint16_t>(eCategory::Primitive)) != 0);
}

bool DataTypeBase::isCustomDefined(void) const
{
    return ((static_cast<uint16_t>(mCategory) & static_cast<uint16_t>(eCategory::CustomDefined)) != 0);
}

bool DataTypeBase::isPredefined(void) const
{
    return ((static_cast<uint16_t>(mCategory) & static_cast<uint16_t>(eCategory::Primitive)) != 0) || (mCategory == eCategory::BasicObject);
}

bool DataTypeBase::isPrimitiveInt(void) const
{
    return (mCategory == eCategory::PrimitiveInt);
}

bool DataTypeBase::isPrimitiveUint(void) const
{
    return (mCategory == eCategory::PrimitiveUint);
}

bool DataTypeBase::isPrimitiveFloat(void) const
{
    return (mCategory == eCategory::PrimitiveFloat);
}

bool DataTypeBase::isBasicObject(void) const
{
    return (mCategory == eCategory::BasicObject);
}

bool DataTypeBase::isEnumeration(void) const
{
    return (mCategory == eCategory::Enumeration);
}

bool DataTypeBase::isStructure(void) const
{
    return (mCategory == eCategory::Structure);
}

bool DataTypeBase::isImported(void) const
{
    return (mCategory == eCategory::Imported);
}

bool DataTypeBase::isContainer(void) const
{
    return (mCategory == eCategory::Container);
}

bool DataTypeBase::isTypeOf(const QString& theType) const
{
    return (mName == theType);
}
