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
 *  \file        lusan/common/ElementBase.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Element base object.
 *
 ************************************************************************/
#include "lusan/common/ElementBase.hpp"

const QString   ElementBase::EmptyString;

ElementBase::ElementBase(ElementBase* parent /*= nullptr*/)
    : mId     (0)
    , mParent (parent)
{
}

ElementBase::ElementBase(unsigned int id, ElementBase* parent /*= nullptr*/)
    : mId     (id)
    , mParent (parent)
{
    setMaxId(id);
}

ElementBase::ElementBase(const ElementBase& src)
    : mId     (src.mId)
    , mParent (src.mParent)
{
    setMaxId(src.mId);
}

ElementBase::ElementBase(ElementBase&& src)
    : mId     (src.mId)
    , mParent (src.mParent)
{
    setMaxId(src.mId);
    src.mParent = nullptr;
}

ElementBase::~ElementBase(void)
{
    mParent = nullptr;
}

ElementBase& ElementBase::operator = (const ElementBase& src)
{
    mParent = src.mParent;
    setId(src.mId);
    return *this;
}

ElementBase& ElementBase::operator = (ElementBase&& src)
{
    mParent = src.mParent;
    setId(src.mId);
    src.mParent = nullptr;
    return *this;
}

bool ElementBase::operator == (const ElementBase& src) const
{
    return (mId == src.mId);
}

bool ElementBase::operator != (const ElementBase& src) const
{
    return (mId != src.mId);
}

unsigned int ElementBase::getNextId(void) const
{
    return (mParent != nullptr ? mParent->getNextId() : ++mId);
}

QIcon ElementBase::getIcon(ElementBase::eDisplay display) const
{
    return QIcon();
}

QString ElementBase::getString(ElementBase::eDisplay display) const
{
    return QString();
}

void ElementBase::setMaxId(unsigned int id) const
{
    if (mParent != nullptr)
    {
        mParent->setMaxId(id);
    }
    else
    {
        mId = (mId > id ? mId : id);
    }
}

