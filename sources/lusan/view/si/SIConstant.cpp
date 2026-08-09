/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/si/SIConstant.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Constants page.
 *
 ************************************************************************/

#include "lusan/view/si/SIConstant.hpp"

#include "lusan/data/common/DataTypeCustom.hpp"

SIConstant::SIConstant(ConstantModel& model, QWidget* parent /*= nullptr*/)
    : ConstantPage       (model, tr("Service Interface Constants Editor ..."), parent)
    , IEDataTypeConsumer ( )
{
}

void SIConstant::dataTypeCreated(DataTypeCustom* /*dataType*/)
{
    refreshDataTypes();
}

void SIConstant::dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
{
    replaceDataType(static_cast<DataTypeBase*>(oldType), static_cast<DataTypeBase*>(newType));
}

void SIConstant::dataTypeDeleted(DataTypeCustom* dataType)
{
    // The declared type is gone: drop the reference, keep the name the author typed so the
    // finding says what is missing instead of silently emptying the field.
    replaceDataType(static_cast<DataTypeBase*>(dataType), nullptr);
}

void SIConstant::dataTypeUpdated(DataTypeCustom* /*dataType*/)
{
    refreshDataTypes();
}
