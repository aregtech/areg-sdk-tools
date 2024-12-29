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
 *  \file        lusan/data/common/DataTypeCustom.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Custom Data Type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypeCustom.hpp"
#include "DataTypeCustom.hpp"

DataTypeCustom::DataTypeCustom(DataTypeBase::eCategory category)
    : DataTypeBase(eCategory::CustomDefined)
    , mId(0)
{
}

DataTypeCustom::DataTypeCustom(DataTypeBase::eCategory category, uint32_t id, const QString& name)
    : DataTypeBase(category, name)
    , mId(id)
{
}

DataTypeCustom::DataTypeCustom(const DataTypeCustom& src)
    : DataTypeBase(src)
    , mId(src.mId)
{
}

DataTypeCustom::DataTypeCustom(DataTypeCustom&& src) noexcept
    : DataTypeBase(std::move(src))
    , mId(src.mId)
{
}

DataTypeCustom::~DataTypeCustom(void)
{
}

DataTypeCustom& DataTypeCustom::operator=(const DataTypeCustom& other)
{
    if (this != &other)
    {
        DataTypeBase::operator=(other);
        mId = other.mId;
    }

    return *this;
}

DataTypeCustom& DataTypeCustom::operator=(DataTypeCustom&& other) noexcept
{
    if (this != &other)
    {
        DataTypeBase::operator=(std::move(other));
        mId = other.mId;
    }

    return *this;
}

uint32_t DataTypeCustom::getId() const
{
    return mId;
}

void DataTypeCustom::setId(uint32_t id)
{
    mId = id;
}

const QString& DataTypeCustom::getDescription(void) const
{
    return mDescription;
}

void DataTypeCustom::setDescription(const QString& description)
{
    mDescription = description;
}

bool DataTypeCustom::isValid(void) const
{
    return (mId != 0) && DataTypeBase::isValid();
}

QString DataTypeCustom::getType(void) const
{
    return DataTypeCustom::getType(mCategory);
}

QString DataTypeCustom::getType(DataTypeBase::eCategory category)
{
    switch (category)
    {
    case DataTypeBase::eCategory::Enumeration:
        return "Enumerate";
    case DataTypeBase::eCategory::Structure:
        return "Structure";
    case DataTypeBase::eCategory::Imported:
        return "Imported";
    case DataTypeBase::eCategory::Container:
        return "DefinedType";
    default:
        return "";
    }
}

DataTypeBase::eCategory DataTypeCustom::fromTypeString(const QString& type)
{
    if (type == "Enumerate")
    {
        return DataTypeBase::eCategory::Enumeration;
    }
    else if (type == "Structure")
    {
        return DataTypeBase::eCategory::Structure;
    }
    else if (type == "Imported")
    {
        return DataTypeBase::eCategory::Imported;
    }
    else if (type == "DefinedType")
    {
        return DataTypeBase::eCategory::Container;
    }
    else
    {
        Q_ASSERT(false);
        return DataTypeBase::eCategory::CustomDefined;
    }
}

