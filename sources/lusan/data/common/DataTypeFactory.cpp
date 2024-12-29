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
 *  \file        lusan/data/common/DataTypeFactory.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type factory.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeDefined.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"

DataTypeCustom* DataTypeFactory::createDataType(const QString& type)
{
    return createDataType(DataTypeCustom::fromTypeString(type));
}

DataTypeCustom* DataTypeFactory::createDataType(DataTypeBase::eCategory category)
{
    switch (category)
    {
    case DataTypeBase::eCategory::Enumeration:
        return new DataTypeEnum();
    case DataTypeBase::eCategory::Structure:
        return new DataTypeStructure();
    case DataTypeBase::eCategory::Imported:
        return new DataTypeImported();
    case DataTypeBase::eCategory::Container:
        return new DataTypeDefined();
    default:
        return nullptr;
    }
}
