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
 *  \file        lusan/data/common/DataTypePrimitive.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Primitive Data Type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypePrimitive.hpp"
#include "lusan/common/XmlSI.hpp"

 //////////////////////////////////////////////////////////////////////////
 // DataTypePrimitive class implementation
 //////////////////////////////////////////////////////////////////////////

DataTypePrimitive::DataTypePrimitive(DataTypeBase::eCategory category)
    : DataTypeBase(category)
{
}

DataTypePrimitive::DataTypePrimitive(const QString& name)
    : DataTypeBase(DataTypeBase::eCategory::Primitive, name)
{
}

DataTypePrimitive::DataTypePrimitive(const DataTypePrimitive& src)
    : DataTypeBase(src)
{
}

DataTypePrimitive::DataTypePrimitive(DataTypePrimitive&& src) noexcept
    : DataTypeBase(std::move(src))
{
}

DataTypePrimitive::DataTypePrimitive(DataTypeBase::eCategory category, const QString& name)
    : DataTypeBase(category, name)
{
}

DataTypePrimitive& DataTypePrimitive::operator=(const DataTypePrimitive& other)
{
    if (this != &other)
    {
        DataTypeBase::operator = (other);
    }

    return *this;
}

DataTypePrimitive& DataTypePrimitive::operator=(DataTypePrimitive&& other) noexcept
{
    if (this != &other)
    {
        DataTypeBase::operator=(std::move(other));
    }

    return *this;
}

bool DataTypePrimitive::readFromXml(QXmlStreamReader& xml)
{
    // nothing to do
    return true;
}

void DataTypePrimitive::writeToXml(QXmlStreamWriter& xml) const
{
    // nothing to do
}

QString DataTypePrimitive::convertValue(const QString& value) const
{
    return (value.isEmpty() || (value.compare(XmlSI::xmlSIValueFalse, Qt::CaseSensitivity::CaseInsensitive) == 0) ? XmlSI::xmlSIValueFalse : XmlSI::xmlSIValueTrue);
}

//////////////////////////////////////////////////////////////////////////
// DataTypePrimitiveInt class implementation
//////////////////////////////////////////////////////////////////////////

DataTypePrimitiveInt::DataTypePrimitiveInt(void)
    : DataTypePrimitive(DataTypeBase::eCategory::PrimitiveSint)
{
}

DataTypePrimitiveInt::DataTypePrimitiveInt(const QString& name)
    : DataTypePrimitive(DataTypeBase::eCategory::PrimitiveSint, name)
{
}

DataTypePrimitiveInt::DataTypePrimitiveInt(const DataTypePrimitiveInt& src)
    : DataTypePrimitive(src)
{
}

DataTypePrimitiveInt::DataTypePrimitiveInt(DataTypePrimitiveInt&& src) noexcept
    : DataTypePrimitive(std::move(src))
{
}

DataTypePrimitiveInt& DataTypePrimitiveInt::operator=(const DataTypePrimitiveInt& other)
{
    if (this != &other)
    {
        DataTypePrimitive::operator=(other);
    }

    return *this;
}

DataTypePrimitiveInt& DataTypePrimitiveInt::operator=(DataTypePrimitiveInt&& other) noexcept
{
    if (this != &other)
    {
        DataTypePrimitive::operator=(std::move(other));
    }

    return *this;
}

QString DataTypePrimitiveInt::convertValue(const QString& value) const
{
    int base = value.startsWith("0x") ? 16 : 10;
    int64_t val = value.toLongLong(nullptr, base);
    return QString::number(val, base);
}

//////////////////////////////////////////////////////////////////////////
// DataTypePrimitiveUint class implementation
//////////////////////////////////////////////////////////////////////////

DataTypePrimitiveUint::DataTypePrimitiveUint(void)
    : DataTypePrimitive(DataTypeBase::eCategory::PrimitiveUint)
{
}

DataTypePrimitiveUint::DataTypePrimitiveUint(const QString& name)
    : DataTypePrimitive(DataTypeBase::eCategory::PrimitiveUint, name)
{
}

DataTypePrimitiveUint::DataTypePrimitiveUint(const DataTypePrimitiveUint& src)
    : DataTypePrimitive(src)
{
}

DataTypePrimitiveUint::DataTypePrimitiveUint(DataTypePrimitiveUint&& src) noexcept
    : DataTypePrimitive(std::move(src))
{
}

DataTypePrimitiveUint& DataTypePrimitiveUint::operator=(const DataTypePrimitiveUint& other)
{
    if (this != &other)
    {
        DataTypePrimitive::operator=(other);
    }

    return *this;
}

DataTypePrimitiveUint& DataTypePrimitiveUint::operator=(DataTypePrimitiveUint&& other) noexcept
{
    if (this != &other)
    {
        DataTypePrimitive::operator=(std::move(other));
    }

    return *this;
}

QString DataTypePrimitiveUint::convertValue(const QString& value) const
{
    int base = value.startsWith("0x") ? 16 : 10;
    uint64_t val = value.toULongLong(nullptr, base);
    return QString::number(val, base);
}

//////////////////////////////////////////////////////////////////////////
// DataTypePrimitiveFloat class implementation
//////////////////////////////////////////////////////////////////////////

DataTypePrimitiveFloat::DataTypePrimitiveFloat(void)
    : DataTypePrimitive(DataTypeBase::eCategory::PrimitiveFloat)
{
}

DataTypePrimitiveFloat::DataTypePrimitiveFloat(const QString& name)
    : DataTypePrimitive(DataTypeBase::eCategory::PrimitiveFloat, name)
{
}

DataTypePrimitiveFloat::DataTypePrimitiveFloat(const DataTypePrimitiveFloat& src)
    : DataTypePrimitive(src)
{
}

DataTypePrimitiveFloat::DataTypePrimitiveFloat(DataTypePrimitiveFloat&& src) noexcept
    : DataTypePrimitive(std::move(src))
{
}

DataTypePrimitiveFloat& DataTypePrimitiveFloat::operator=(const DataTypePrimitiveFloat& other)
{
    if (this != &other)
    {
        DataTypePrimitive::operator=(other);
    }

    return *this;
}

DataTypePrimitiveFloat& DataTypePrimitiveFloat::operator=(DataTypePrimitiveFloat&& other) noexcept
{
    if (this != &other)
    {
        DataTypePrimitive::operator=(std::move(other));
    }

    return *this;
}

QString DataTypePrimitiveFloat::convertValue(const QString& value) const
{
    int base = value.startsWith("0x") ? 16 : 10;
    double val = value.toDouble(nullptr);
    return QString::number(val);
}
