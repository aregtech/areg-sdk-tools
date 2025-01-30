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
 *  \file        lusan/model/si/SIMethodModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Method Model implementation.
 *
 ************************************************************************/

#include "lusan/model/si/SIMethodModel.hpp"
#include "lusan/data/si/SIDataTypeData.hpp"

SIMethodModel::SIMethodModel(SIMethodData& data, SIDataTypeData& dataType)
    : mData     (data)
    , mDataType (dataType)
{
}

SIMethodBase* SIMethodModel::createMethod(const QString& name, SIMethodBase::eMethodType methodType)
{
    return mData.addMethod(name, methodType);
}

bool SIMethodModel::removeMethod(uint32_t id)
{
    return mData.removeMethod(id);
}

bool SIMethodModel::removeMethod(const QString& name, SIMethodBase::eMethodType methodType)
{
    return mData.removeMethod(name, methodType);
}

void SIMethodModel::removeMethod(SIMethodBase * method)
{
    mData.removeMethod(method);
}

SIMethodBase* SIMethodModel::findMethod(uint32_t id) const
{
    return mData.findMethod(id);
}

const QList<SIMethodBase*>& SIMethodModel::getMethodList(void) const
{
    return mData.getAllMethods();
}

const QList<SIMethodBroadcast*>& SIMethodModel::getBroadcastMethods(void) const
{
    return mData.getBroadcasts();
}

const QList<SIMethodRequest*>& SIMethodModel::getRequestMethods(void) const
{
    return mData.getRequests();
}

const QList<SIMethodResponse*>& SIMethodModel::getResponseMethods(void) const
{
    return mData.getResponses();
}

SIMethodBase* SIMethodModel::findMethod(const QString& name, SIMethodBase::eMethodType methodType) const
{
    return mData.findMethod(name, methodType);
}

const QList<MethodParameter>& SIMethodModel::getMethodParameters(uint32_t id) const
{
    static const QList<MethodParameter> _empty{};
    SIMethodBase* method = findMethod(id);
    return method != nullptr ? method->getElements() : _empty;
}

const QList<MethodParameter>& SIMethodModel::getMethodParameters(const QString& name, SIMethodBase::eMethodType methodType) const
{
    static const QList<MethodParameter> _empty{};
    SIMethodBase* method = findMethod(name, methodType);
    return method != nullptr ? method->getElements() : _empty;
}

QList<SIMethodRequest*> SIMethodModel::getConnectedRequests(SIMethodResponse* response) const
{
    return mData.getConnectedRequests(response);
}

SIMethodBase* SIMethodModel::convertMethod(SIMethodBase* method, SIMethodBase::eMethodType methodType)
{
    return mData.convertMethod(method, methodType);
}

MethodParameter* SIMethodModel::addParameter(SIMethodBase* method, const QString& name, const QString& type /*= "bool"*/)
{
    MethodParameter* param = mData.addParameter(method, name, type);
    if (param != nullptr)
    {
        param->validate(mDataType.getCustomDataTypes());
    }

    return param;
}
