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
 *  \file        lusan/data/common/DataTypeBasic.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Basic Data Type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypeBasic.hpp"

DataTypeBasic::DataTypeBasic(void)
    : DataTypeBase  (eCategory::BasicObject)
    , mHasValue     (false)
    , mHasKey       (false)
{
}

DataTypeBasic::DataTypeBasic(const DataTypeBasic& src)
    : DataTypeBase  (src)
    , mHasValue     (src.mHasValue)
    , mHasKey       (src.mHasKey)
{
}

DataTypeBasic::DataTypeBasic(DataTypeBasic&& src) noexcept
    : DataTypeBase  (std::move(src))
    , mHasValue     (src.mHasValue)
    , mHasKey       (src.mHasKey)
{
}

DataTypeBasic::DataTypeBasic(const QString& name)
    : DataTypeBase  (eCategory::BasicObject, name)
    , mHasValue     (false)
    , mHasKey   (false)
{
}

DataTypeBasic& DataTypeBasic::operator=(const DataTypeBasic& other)
{
    if (this != &other)
    {
        DataTypeBase::operator=(other);
        mHasValue   = other.mHasValue;
        mHasKey     = other.mHasKey;
    }
    return *this;
}

DataTypeBasic& DataTypeBasic::operator=(DataTypeBasic&& other) noexcept
{
    if (this != &other)
    {
        DataTypeBase::operator=(std::move(other));
        mHasValue   = other.mHasValue;
        mHasKey     = other.mHasKey;
    }

    return *this;
}

void DataTypeBasic::setDataContainer(bool isDataContainer, bool hasKey)
{
    mHasValue   = isDataContainer;
    mHasKey     = isDataContainer ? hasKey : false;
}

bool DataTypeBasic::isDataContainer(void) const
{
    return mHasValue;
}

bool DataTypeBasic::hasKey(void) const
{
    return mHasKey;
}

bool DataTypeBasic::readFromXml(QXmlStreamReader& xml)
{
    // Implement XML reading logic here
    return true;
}

void DataTypeBasic::writeToXml(QXmlStreamWriter& xml) const
{
    // Implement XML writing logic here
}
