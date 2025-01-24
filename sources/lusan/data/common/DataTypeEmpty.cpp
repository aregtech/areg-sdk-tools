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
 *  \file        lusan/data/common/DataTypeEmpty.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Empty Data Type.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeEmpty.hpp"

DataTypeEmpty::DataTypeEmpty(void)
    : DataTypeBase(DataTypeBase::eCategory::Undefined, QString(), 0, nullptr)
{
}

inline unsigned int DataTypeEmpty::getId(void) const
{
    return 0;
}

bool DataTypeEmpty::isValid(void) const
{
    return false;
}

bool DataTypeEmpty::readFromXml(QXmlStreamReader& xml)
{
    return false;
}

void DataTypeEmpty::writeToXml(QXmlStreamWriter& xml) const
{
}

unsigned int DataTypeEmpty::getNextId(void) const
{
    return 0;
}
