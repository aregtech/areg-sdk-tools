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
#include "lusan/data/common/DataTypePrimitive.hpp"

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

bool MethodBase::hasDefaultValue(uint32_t id) const
{
    for (const MethodParameter& param : getElements())
    {
        if (param.getId() == id)
        {
            return param.hasDefault();
        }
    }

    return false;
}

bool MethodBase::hasDefaultValue(const QString& name) const
{
    for (const MethodParameter& param : getElements())
    {
        if (param.getName() == name)
        {
            return param.hasDefault();
        }
    }

    return false;
}

bool MethodBase::canHaveDefaultValue(uint32_t id) const
{
    int pos = findIndex(id);
    return canHaveDefaultValue(pos);
}

bool MethodBase::canHaveDefaultValue(const QString& name) const
{
    int pos = findIndex(name);
    return canHaveDefaultValue(pos);
}

int MethodBase::firsPositionWithDefault(void) const
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        if (mElementList[i].hasDefault())
        {
            return i;
        }
    }

    return -1;
}

bool MethodBase::canSwitchDefaultValue(uint32_t id) const
{
    int pos = findIndex(id);
    int def = firsPositionWithDefault();
    if (pos >= 0)
    {
        return (pos == def) || canHaveDefaultValue(pos);
    }

    return false;
}

bool MethodBase::canSwitchDefaultValue(const QString& name) const
{
    int pos = findIndex(name);
    int def = firsPositionWithDefault();
    if (pos >= 0)
    {
        return (pos == def) || canHaveDefaultValue(pos);
    }

    return false;
}

bool MethodBase::isLastPositionWithDefault(uint32_t id) const
{
    int pos = findIndex(id);
    int def = firsPositionWithDefault();
    return (pos >= 0) && (pos == def);
}

bool MethodBase::isLastPositionWithDefault(const QString& name) const
{
    int pos = findIndex(name);
    int def = firsPositionWithDefault();
    return (pos >= 0) && (pos == def);
}

MethodParameter* MethodBase::setDefaultValue(uint32_t id, const QString& newValue)
{
    MethodParameter* param = findElement(id);
    if ((param != nullptr) && (param->hasDefault()))
    {
        param->setValue(newValue);
    }

    return param;
}

MethodParameter* MethodBase::setDefaultValue(const QString& name, const QString& newValue)
{
    MethodParameter* param = findElement(name);
    if ((param != nullptr) && (param->hasDefault()))
    {
        param->setDefault(true);
        param->setValue(newValue);
    }

    return param;
}

MethodParameter* MethodBase::makeValueDefault(uint32_t id, bool makeDefault, const QString& value)
{
    MethodParameter* param = findElement(id);
    if (makeDefault)
    {
        if (canHaveDefaultValue(id))
        {
            param->setDefault(true);
            DataTypeBase* dataType = param->getParamType();
            if ((dataType != nullptr) && (dataType->isPrimitive()))
            {
                QString val = static_cast<DataTypePrimitive*>(dataType)->convertValue(value);
                param->setValue(val);
            }
            else
            {
                param->setValue(value);
            }
        }
    }
    else
    {
        int def = firsPositionWithDefault();
        if ((def >= 0) && (param->getId() == mElementList[def].getId()))
        {
            param->setDefault(false);
            param->setValue(QString());
        }
    }

    return param;
}

MethodParameter* MethodBase::makeValueDefault(const QString& name, bool makeDefault, const QString& value)
{
    MethodParameter* param = findElement(name);
    if (makeDefault)
    {
        if (canHaveDefaultValue(name))
        {
            param->setDefault(true);
            DataTypeBase* dataType = param->getParamType();
            if ((dataType != nullptr) && (dataType->isPrimitive()))
            {
                QString val = static_cast<DataTypePrimitive*>(dataType)->convertValue(value);
                param->setValue(val);
            }
            else
            {
                param->setValue(value);
            }
        }
    }
    else
    {
        int def = firsPositionWithDefault();
        if ((def >= 0) && (param->getId() == mElementList[def].getId()))
        {
            param->setDefault(false);
            param->setValue(QString());
        }
    }

    return param;
}

bool MethodBase::canSwapParamLeft(uint32_t id) const
{
    int pos = findIndex(id);
    return canSwapParamLeft(pos);
}

bool MethodBase::canSwapParamRight(uint32_t id) const
{
    int pos = findIndex(id);
    return canSwapParamRight(pos);
}

bool MethodBase::canSwapParamLeft(const QString& name) const
{
    int pos = findIndex(name);
    return canSwapParamLeft(pos);
}

bool MethodBase::canSwapParamRight(const QString& name) const
{
    int pos = findIndex(name);
    return canSwapParamRight(pos);
}

bool MethodBase::canHaveDefaultValue(int index) const
{
    int def = firsPositionWithDefault();
    if (index >= 0)
    {
        if (def < 0)
        {
            return (index == (mElementList.size() - 1));
        }
        else
        {
            return (index == (def - 1));
        }
    }

    return false;
}

bool MethodBase::canSwapParamLeft(int position) const
{
    if (position <= 0)
        return false;

    int def = firsPositionWithDefault();
    int count = getElementCount();
    if (def < 0)
    {
        return (position < count);
    }
    else
    {
        return (position < def) || ((position < count) && (position > def));
    }
}

bool MethodBase::canSwapParamRight(int position) const
{
    int count = getElementCount();
    if ((position < 0) || (position == (count - 1)))
        return false;

    int def = firsPositionWithDefault();
    if (def < 0)
    {
        return (position < (count - 1));
    }
    else
    {
        return (position < (def - 1)) || ((position < (count - 1)) && (position >= def));
    }
}
