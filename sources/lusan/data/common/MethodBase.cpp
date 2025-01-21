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

MethodBase::MethodBase(ElementBase* parent /*= nullptr*/)
    : TEDataContainer< MethodParameter, ElementBase >(parent)
    , mName         ()
    , mDescription  ()
{
}

MethodBase::MethodBase(uint32_t id, const QString& name, const QString& description, ElementBase* parent /*= nullptr*/)
    : TEDataContainer< MethodParameter, ElementBase >(id, parent)
    , mName         (name)
    , mDescription  (description)
{
}

MethodBase::MethodBase(const MethodBase& src)
    : TEDataContainer< MethodParameter, ElementBase >(src)
    , mName         (src.mName)
    , mDescription  (src.mDescription)
{
}

MethodBase::MethodBase(MethodBase&& src) noexcept
    : TEDataContainer< MethodParameter, ElementBase >(std::move(src))
    , mName         (std::move(src.mName))
    , mDescription  (std::move(src.mDescription))
{
}

MethodBase::~MethodBase(void)
{
}

MethodBase& MethodBase::operator = (const MethodBase& other)
{
    if (this != &other)
    {
        TEDataContainer< MethodParameter, ElementBase >::operator = (other);
        mName       = other.mName;
        mDescription= other.mDescription;
    }

    return *this;
}

MethodBase& MethodBase::operator = (MethodBase&& other) noexcept
{
    if (this != &other)
    {
        TEDataContainer< MethodParameter, ElementBase >::operator = (std::move(other));
        mName       = std::move(other.mName);
        mDescription= std::move(other.mDescription);
    }

    return *this;
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
