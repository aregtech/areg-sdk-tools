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
    : TEDataContainer< MethodParameter, DocumentElem >(parent)
    , mName         ()
    , mDescription  ()
{
}

MethodBase::MethodBase(uint32_t id, const QString& name, const QString& description, ElementBase* parent /*= nullptr*/)
    : TEDataContainer< MethodParameter, DocumentElem >(id, parent)
    , mName         (name)
    , mDescription  (description)
{
}

MethodBase::MethodBase(const MethodBase& src)
    : TEDataContainer< MethodParameter, DocumentElem >(src)
    , mName         (src.mName)
    , mDescription  (src.mDescription)
{
}

MethodBase::MethodBase(MethodBase&& src) noexcept
    : TEDataContainer< MethodParameter, DocumentElem >(std::move(src))
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
        TEDataContainer< MethodParameter, DocumentElem >::operator = (other);
        mName       = other.mName;
        mDescription= other.mDescription;
    }

    return *this;
}

MethodBase& MethodBase::operator = (MethodBase&& other) noexcept
{
    if (this != &other)
    {
        TEDataContainer< MethodParameter, DocumentElem >::operator = (std::move(other));
        mName       = std::move(other.mName);
        mDescription= std::move(other.mDescription);
    }

    return *this;
}

bool MethodBase::isValid() const
{
    return mName.isEmpty() == false;
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

MethodParameter* MethodBase::addParam(const QString& name)
{
    MethodParameter* result{ nullptr };
    bool isDefault = mElementList.size() > 0 ? mElementList.last().hasDefault() : false;
    MethodParameter entry(getNextId(), name, isDefault, this);
    if (addElement(std::move(entry), true))
    {
        Q_ASSERT(mElementList.size() > 0);
        result = &mElementList[mElementList.size() - 1];
    }

    return result;
}

MethodParameter* MethodBase::insertParam(int position, const QString& name)
{
    MethodParameter* result{ nullptr };
    if ((position >= 0) && (position <= mElementList.size()))
    {
        bool isDefault{ false };
        if ((position > 0) && (position <= mElementList.size()))
        {
            isDefault = mElementList[position - 1].hasDefault();
        }

        MethodParameter entry(getNextId(), name, isDefault, this);
        if (insertElement(position, std::move(entry), true))
        {
            result = &mElementList[position];
        }
    }

    return result;
}

void MethodBase::removeParam(const QString& name)
{
    removeElement(name);
}

void MethodBase::removeParam(uint32_t id)
{
    removeElement(id);
}

DataTypeBase* MethodBase::getParamType(const QString& name) const
{
    MethodParameter* param = findElement(name);
    return (param != nullptr ? param->getParamType() : nullptr);
}

DataTypeBase* MethodBase::getParamType(uint32_t id) const
{
    MethodParameter* param = findElement(id);
    return (param != nullptr ? param->getParamType() : nullptr);
}

bool MethodBase::validate(const QList<DataTypeCustom*>& customTypes)
{
    bool result = true;
    for (MethodParameter& param : getElements())
    {
        result &= param.validate(customTypes);
    }

    return result;
}

void MethodBase::invalidate(void)
{
    for (MethodParameter& param : getElements())
    {
        param.invalidate();
    }
}
