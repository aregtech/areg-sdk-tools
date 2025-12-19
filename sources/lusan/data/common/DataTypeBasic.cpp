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
 *  \file        lusan/data/common/DataTypeBasic.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Basic Data Type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypeBasic.hpp"
#include "DataTypeBasic.hpp"

 //////////////////////////////////////////////////////////////////////////
 // DataTypeBasicObject class implementation
 //////////////////////////////////////////////////////////////////////////

DataTypeBasicObject::DataTypeBasicObject(void)
    : DataTypeBase  (eCategory::BasicObject)
{
}

DataTypeBasicObject::DataTypeBasicObject(const DataTypeBasicObject& src)
    : DataTypeBase  (src)
{
}

DataTypeBasicObject::DataTypeBasicObject(DataTypeBasicObject&& src) noexcept
    : DataTypeBase  (std::move(src))
{
}

DataTypeBasicObject::DataTypeBasicObject(const QString& name)
    : DataTypeBase  (eCategory::BasicObject, name)
{
}

DataTypeBasicObject& DataTypeBasicObject::operator = (const DataTypeBasicObject& other)
{
    DataTypeBase::operator = (other);
    return *this;
}

DataTypeBasicObject& DataTypeBasicObject::operator = (DataTypeBasicObject&& other) noexcept
{
    DataTypeBase::operator=(std::move(other));
    return *this;
}

bool DataTypeBasicObject::readFromXml(QXmlStreamReader& xml)
{
    return true;
}

void DataTypeBasicObject::writeToXml(QXmlStreamWriter& xml) const
{
}

//////////////////////////////////////////////////////////////////////////
// DataTypeBasicContainer class implementation
//////////////////////////////////////////////////////////////////////////

DataTypeBasicContainer::DataTypeBasicContainer(void)
    : DataTypeBase  (eCategory::BasicContainer)
    , mHasKey       (false)
{
}

DataTypeBasicContainer::DataTypeBasicContainer(const DataTypeBasicContainer& src)
    : DataTypeBase  (src)
    , mHasKey       (src.mHasKey)
{
}

DataTypeBasicContainer::DataTypeBasicContainer(DataTypeBasicContainer&& src) noexcept
    : DataTypeBase  (std::move(src))
    , mHasKey       (src.mHasKey)
{
}

DataTypeBasicContainer::DataTypeBasicContainer(const QString& name)
    : DataTypeBase  (eCategory::BasicContainer, name)
    , mHasKey       (false)
{
}

DataTypeBasicContainer& DataTypeBasicContainer::operator = (const DataTypeBasicContainer& other)
{
    DataTypeBase::operator = (other);
    mHasKey = other.mHasKey;
    return *this;
}

DataTypeBasicContainer& DataTypeBasicContainer::operator = (DataTypeBasicContainer&& other) noexcept
{
    DataTypeBase::operator = (std::move(other));
    mHasKey = other.mHasKey;
    return *this;
}

void DataTypeBasicContainer::setKey(bool hasKey)
{
    mHasKey = hasKey;
}

bool DataTypeBasicContainer::hasKey(void) const
{
    return mHasKey;
}

bool DataTypeBasicContainer::readFromXml(QXmlStreamReader& xml)
{
    return true;
}

void DataTypeBasicContainer::writeToXml(QXmlStreamWriter& xml) const
{
}
