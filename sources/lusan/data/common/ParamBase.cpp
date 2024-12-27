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
 *  \file        lusan/data/common/ParamBase.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Parameter Base.
 *
 ************************************************************************/
#include "lusan/data/common/ParamBase.hpp"

ParamBase::ParamBase(void)
    : mId(0)
    , mName()
    , mType()
    , mIsDeprecated(false)
    , mDescription()
    , mDeprecateHint()
{
}

ParamBase::ParamBase(uint32_t id, const QString& name, std::shared_ptr<DataTypeBase> type, bool isDeprecated, const QString& description, const QString& deprecateHint)
    : mId(id)
    , mName(name)
    , mType(type)
    , mIsDeprecated(isDeprecated)
    , mDescription(description)
    , mDeprecateHint(deprecateHint)
{
}

ParamBase::ParamBase(const ParamBase& src)
    : mId(src.mId)
    , mName(src.mName)
    , mType(src.mType)
    , mIsDeprecated(src.mIsDeprecated)
    , mDescription(src.mDescription)
    , mDeprecateHint(src.mDeprecateHint)
{
}

ParamBase::ParamBase(ParamBase&& src) noexcept
    : mId(src.mId)
    , mName(std::move(src.mName))
    , mType(std::move(src.mType))
    , mIsDeprecated(src.mIsDeprecated)
    , mDescription(std::move(src.mDescription))
    , mDeprecateHint(std::move(src.mDeprecateHint))
{
}

ParamBase& ParamBase::operator=(const ParamBase& other)
{
    if (this != &other)
    {
        mId = other.mId;
        mName = other.mName;
        mType = other.mType;
        mIsDeprecated = other.mIsDeprecated;
        mDescription = other.mDescription;
        mDeprecateHint = other.mDeprecateHint;
    }

    return *this;
}

ParamBase& ParamBase::operator=(ParamBase&& other) noexcept
{
    if (this != &other)
    {
        mId = other.mId;
        mName = std::move(other.mName);
        mType = std::move(other.mType);
        mIsDeprecated = other.mIsDeprecated;
        mDescription = std::move(other.mDescription);
        mDeprecateHint = std::move(other.mDeprecateHint);
    }

    return *this;
}

bool ParamBase::operator==(const ParamBase& other) const
{
    bool result{ this == &other };
    if (this != &other)
    {
        if ((mType != nullptr) && (other.mType != nullptr))
        {
            result = (mName == other.mName) && (mType == other.mType) && ((*mType) == (*other.mType));
        }
    }

    return result;
}

bool ParamBase::operator!=(const ParamBase& other) const
{
    return !(*this == other);
}

uint32_t ParamBase::getId() const
{
    return mId;
}

void ParamBase::setId(uint32_t id)
{
    mId = id;
}

const QString & ParamBase::getName() const
{
    return mName;
}

void ParamBase::setName(const QString& name)
{
    mName = name;
}

std::shared_ptr<DataTypeBase> ParamBase::getType() const
{
    return mType;
}

void ParamBase::setType(std::shared_ptr<DataTypeBase> type)
{
    mType = type;
}

bool ParamBase::isDeprecated() const
{
    return mIsDeprecated;
}

void ParamBase::setDeprecated(bool isDeprecated)
{
    mIsDeprecated = isDeprecated;
}

const QString& ParamBase::getDescription() const
{
    return mDescription;
}

void ParamBase::setDescription(const QString& description)
{
    mDescription = description;
}

const QString& ParamBase::getDeprecateHint() const
{
    return mDeprecateHint;
}

void ParamBase::setDeprecateHint(const QString& deprecateHint)
{
    mDeprecateHint = deprecateHint;
}

bool ParamBase::isValid() const
{
    return mId != 0 && !mName.isEmpty() && mType != nullptr;
}
