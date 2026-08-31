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
 *  \file        lusan/model/dt/DataTypeDocumentModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type document model.
 *
 ************************************************************************/

#include "lusan/model/dt/DataTypeDocumentModel.hpp"

#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/model/dt/DTValidator.hpp"

DataTypeDocumentModel::DataTypeDocumentModel(const QString& filePath /*= QString()*/)
    : mDTData        (filePath)
    , mNotifier      ( )
    , mUndoStack     ( )
    , mModelOverview (*this)
    , mModelDataType (*this)
    , mModelInclude  (*this)
    , mNoConstants   (nullptr)
    , mNoAttributes  (NEAttribute::ServiceInterface, nullptr)
    , mNoMethods     (NEMethod::serviceInterface(), nullptr)
    , mValidation    (*this, [this]() { return DTValidator::validate(mDTData); })
{
}

const QList<DataTypeCustom*>& DataTypeDocumentModel::getCustomDataTypes(void) const
{
    return mDTData.getDataTypeData().getResolutionTypes();
}
