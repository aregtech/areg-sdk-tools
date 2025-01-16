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
 *  \copyright   � 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/model/si/SIMethodModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Method Model implementation.
 *
 ************************************************************************/

#include "lusan/model/si/SIMethodModel.hpp"

SIMethodModel::SIMethodModel(SIMethodData& data)
    : mData(data)
{
}

SIMethodBase* SIMethodModel::createMethod(const QString& name, SIMethodBase::eMethodType methodType)
{
    return mData.addMethod(name, methodType);
}

bool SIMethodModel::deleteMethod(uint32_t id)
{
    return mData.removeMethod(id);
}

bool SIMethodModel::deleteMethod(const QString& name, SIMethodBase::eMethodType methodType)
{
    return mData.removeMethod(name, methodType);
}

SIMethodBase* SIMethodModel::findMethod(uint32_t id) const
{
    return mData.findMethod(id);
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
