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
 *  \file        lusan/model/si/ServiceInterfaceModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Model.
 *
 ************************************************************************/

#include "lusan/model/si/ServiceInterfaceModel.hpp"

#include "lusan/data/common/AttributeEntry.hpp"
#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/MethodEntry.hpp"
#include "lusan/model/si/SIValidator.hpp"

ServiceInterfaceModel::ServiceInterfaceModel(const QString& filePath /*= QString()*/)
    : mSIData           (filePath)
    , mNotifier         ( )
    , mUndoStack        ( )
    , mModelOverview    (*this)
    , mModelDataType    (*this)
    , mModelAttributes  (*this)
    , mModelMethods     (*this)
    , mModelConstant    (*this)
    , mModelInclude     (*this)
    , mValidation       (*this, [this]() { return SIValidator::validate(mSIData); })
{
}

void ServiceInterfaceModel::refreshTypeReferences()
{
    DataTypeDataSection& types = mSIData.getDataTypeData();
    types.refreshTypeReferences();

    // Dropped before being looked up again: validate() only fills an empty slot, so a reference
    // to a type that has been replaced would otherwise survive as it is.
    for (AttributeEntry& entry : mSIData.getAttributeData().getElements())
    {
        entry.invalidate();
    }

    for (MethodEntry* entry : mSIData.getMethodData().getElements())
    {
        if (entry != nullptr)
        {
            entry->invalidate();
        }
    }

    for (ConstantEntry& entry : mSIData.getConstantData().getElements())
    {
        entry.invalidate();
    }

    mSIData.getAttributeData().validate(types);
    mSIData.getMethodData().validate(types);
    mSIData.getConstantData().validate(types.getResolutionTypes());
}

const QList<DataTypeCustom*>& ServiceInterfaceModel::getCustomDataTypes() const
{
    return mSIData.getDataTypeData().getResolutionTypes();
}
