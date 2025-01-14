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
 *  \file        lusan/data/si/SIMethodBase.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Method Base.
 *
 ************************************************************************/

#include "lusan/data/si/SIMethodBase.hpp"

SIMethodBase::SIMethodBase(ElementBase* parent /*= nullptr*/)
    : MethodBase    (parent)
    , mMethodType   (eMethodType::MethodUnknown)
    , mIsDeprecated (false)
    , mDeprecateHint()
{
}

SIMethodBase::SIMethodBase(eMethodType methodType, ElementBase* parent /*= nullptr*/)
    : MethodBase    (parent)
    , mMethodType   (methodType)
    , mIsDeprecated (false)
    , mDeprecateHint()
{

}

SIMethodBase::SIMethodBase(eMethodType methodType, const QString& name, ElementBase* parent /*= nullptr*/)
    : MethodBase    (parent->getNextId(), name, QString(), parent)
    , mMethodType   (methodType)
    , mIsDeprecated (false)
    , mDeprecateHint()
{
}

SEMethodsBase::SEMethodsBase(uint32_t id, eMethodType methodType, const QString& name, ElementBase* parent /*= nullptr*/)
    : MethodBase    (id, name, QString(), parent)
    , mMethodType   (methodType)
    , mIsDeprecated (false)
    , mDeprecateHint()
{
}


SIMethodBase::SIMethodBase(uint32_t id, const QString& name, const QString& description, eMethodType methodType, ElementBase* parent /*= nullptr*/)
    : MethodBase    (id, name, description, parent)
    , mMethodType   (methodType)
    , mIsDeprecated (false)
    , mDeprecateHint()
{
}

SIMethodBase::SIMethodBase(const SIMethodBase& src)
    : MethodBase    (src)
    , mMethodType   (src.mMethodType)
    , mIsDeprecated (src.mIsDeprecated)
    , mDeprecateHint(src.mDeprecateHint)
{
}

SIMethodBase::SIMethodBase(SIMethodBase&& src) noexcept
    : MethodBase    (std::move(src))
    , mMethodType   (src.mMethodType)
    , mIsDeprecated (src.mIsDeprecated)
    , mDeprecateHint(std::move(src.mDeprecateHint))
{
}

SIMethodBase::~SIMethodBase(void)
{
}

SIMethodBase& SIMethodBase::operator = (const SIMethodBase& other)
{
    if (this != &other)
    {
        MethodBase::operator = (other);
        mMethodType     = other.mMethodType;
        mIsDeprecated   = other.mIsDeprecated;
        mDeprecateHint  = other.mDeprecateHint;
    }

    return *this;
}

SIMethodBase& SIMethodBase::operator = (SIMethodBase&& other) noexcept
{
    if (this != &other)
    {
        MethodBase::operator = (std::move(other));
        mMethodType     = other.mMethodType;
        mIsDeprecated   = other.mIsDeprecated;
        mDeprecateHint  = std::move(other.mDeprecateHint);
    }

    return *this;
}

bool SIMethodBase::operator == (const SIMethodBase& other) const
{
    return (this != &other ? (mName == other.mName) && (mMethodType == other.mMethodType) : true);
}

bool SIMethodBase::operator != (const SIMethodBase& other) const
{
    return (this != &other ? (mName != other.mName) || (mMethodType != other.mMethodType) : false);
}

SIMethodBase::eMethodType SIMethodBase::getMethodType() const
{
    return mMethodType;
}

void SIMethodBase::setMethodType(eMethodType methodType)
{
    mMethodType = methodType;
}

QString SIMethodBase::getType(void) const
{
    return SIMethodBase::toString(mMethodType);
}

void SIMethodBase::markDeprecated(bool isDeprecated, const QString& hint)
{
    if (isDeprecated)
    {
        mIsDeprecated = true;
        mDeprecateHint = hint;
    }
    else
    {
        mIsDeprecated = false;
        mDeprecateHint.clear();
    }
}

void SIMethodBase::setIsDeprecated(bool isDeprecated)
{
    mIsDeprecated = isDeprecated;
    if (isDeprecated)
    {
        mDeprecateHint.clear();
    }
}

bool SIMethodBase::isDeprecated(void) const
{
    return mIsDeprecated;
}

void SIMethodBase::setDeprecateHint(const QString& hint)
{
    mDeprecateHint = mIsDeprecated ? hint : "";
}

const QString& SIMethodBase::getDeprecateHint(void) const
{
    return mDeprecateHint;
}

QString SIMethodBase::toString(eMethodType methodType)
{
    switch (methodType)
    {
    case eMethodType::MethodRequest:
        return "request";
    case eMethodType::MethodResponse:
        return "response";
    case eMethodType::MethodBroadcast:
        return "broadcast";
    default:
        return "";
    }
}

SIMethodBase::eMethodType SIMethodBase::fromString(const QString& methodTypeStr)
{
    if (methodTypeStr == "request")
    {
        return eMethodType::MethodRequest;
    }
    else if (methodTypeStr == "response")
    {
        return eMethodType::MethodResponse;
    }
    else if (methodTypeStr == "broadcast")
    {
        return eMethodType::MethodBroadcast;
    }
    else
    {
        return eMethodType::MethodUnknown;
    }
}
