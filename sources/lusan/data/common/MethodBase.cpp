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
 *  \file        lusan/data/common/MethodBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Method Base.
 *
 ************************************************************************/

#include "lusan/data/common/MethodBase.hpp"

MethodBase::MethodBase(void)
    : mId           (0)
    , mName         ()
    , mDescription  ()
    , mParameters   ()
{
}

MethodBase::MethodBase(uint32_t id, const QString& name, const QString& description)
    : mId           (id)
    , mName         (name)
    , mDescription  (description)
    , mParameters   ()
{
}

MethodBase::MethodBase(const MethodBase& src)
    : mId           (src.mId)
    , mName         (src.mName)
    , mDescription  (src.mDescription)
    , mParameters   (src.mParameters)
{
}

MethodBase::MethodBase(MethodBase&& src) noexcept
    : mId           (src.mId)
    , mName         (std::move(src.mName))
    , mDescription  (std::move(src.mDescription))
    , mParameters   (std::move(src.mParameters))
{
}

MethodBase::~MethodBase(void)
{
}

MethodBase& MethodBase::operator=(const MethodBase& other)
{
    if (this != &other)
    {
        mId = other.mId;
        mName = other.mName;
        mDescription = other.mDescription;
        mParameters = other.mParameters;
    }
    return *this;
}

MethodBase& MethodBase::operator=(MethodBase&& other) noexcept
{
    if (this != &other)
    {
        mId = other.mId;
        mName = std::move(other.mName);
        mDescription = std::move(other.mDescription);
        mParameters = std::move(other.mParameters);
    }

    return *this;
}

bool MethodBase::addParameter(MethodParameter parameter)
{
    if (findParameter(parameter.getName()) == nullptr)
    {
        mParameters.append(std::move(parameter));
        return true;
    }

    return false;
}

bool MethodBase::removeParameter(const QString& name)
{
    for (int i = 0; i < mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            mParameters.removeAt(i);
            return true;
        }
    }

    return false;
}

bool MethodBase::insertParameter(int index, MethodParameter parameter)
{
    if (findParameter(parameter.getName()) == nullptr && index >= 0 && index <= mParameters.size())
    {
        mParameters.insert(index, std::move(parameter));
        return true;
    }

    return false;
}

MethodParameter* MethodBase::findParameter(const QString& name) const
{
    for (const auto& parameter : mParameters)
    {
        if (parameter.getName() == name)
        {
            return const_cast<MethodParameter*>(&parameter);
        }
    }

    return nullptr;
}

uint32_t MethodBase::getId() const
{
    return mId;
}

void MethodBase::setId(uint32_t id)
{
    mId = id;
}

const QString& MethodBase::getName() const
{
    return mName;
}

void MethodBase::setName(const QString& name)
{
    mName = name;
}

const QString& MethodBase::getDescription() const
{
    return mDescription;
}

void MethodBase::setDescription(const QString& description)
{
    mDescription = description;
}
