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

DataTypeCustom::DataTypeCustom(ElementBase * parent /*= nullptr*/)
    : DataTypeBase(eCategory::CustomDefined, "", 0, parent)
    , mDescription()
{
}

DataTypeCustom::DataTypeCustom(uint32_t id, ElementBase* parent)
    : DataTypeBase(eCategory::CustomDefined, "", id, parent)
    , mDescription()
{
}

DataTypeCustom::DataTypeCustom(DataTypeBase::eCategory category, ElementBase* parent /*= nullptr*/)
    : DataTypeBase(category, "", 0, parent)
    , mDescription()
{
}

DataTypeCustom::DataTypeCustom(DataTypeBase::eCategory category, uint32_t id, const QString& name, ElementBase* parent /*= nullptr*/)
    : DataTypeBase(category, name, id, parent)
{
}

DataTypeCustom::DataTypeCustom(const DataTypeCustom& src)
    : DataTypeBase(src)
    , mDescription(src.mDescription)
{
}

DataTypeCustom::DataTypeCustom(DataTypeCustom&& src) noexcept
    : DataTypeBase(std::move(src))
    , mDescription(std::move(src.mDescription))
{
}

DataTypeCustom::~DataTypeCustom(void)
{
}

DataTypeCustom& DataTypeCustom::operator = (const DataTypeCustom& other)
{
    if (this != &other)
    {
        DataTypeBase::operator = (other);
        mDescription = other.mDescription;
    }

    return *this;
}

DataTypeCustom& DataTypeCustom::operator = (DataTypeCustom&& other) noexcept
{
    if (this != &other)
    {
        DataTypeBase::operator = (std::move(other));
        mDescription = std::move(other.mDescription);
    }

    return *this;
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
    return (getId() != 0) && DataTypeBase::isValid();
}

bool DataTypeCustom::getIsDeprecated(void) const
{
    return mIsDeprecated;
}

bool DataTypeCustom::setDeprecated(bool isDeprecated)
{
    mIsDeprecated = isDeprecated;
}

const QString& DataTypeCustom::getDeprecateHint(void) const
{
    return mDeprecateHint;
}

void DataTypeCustom::setDeprecateHint(const QString& hint)
{
    mDeprecateHint = hint;
}

void DataTypeCustom::setDeprecated(bool isDeprecated, const QString& reason)
{
    mIsDeprecated = isDeprecated;
    mDeprecateHint = reason;
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

