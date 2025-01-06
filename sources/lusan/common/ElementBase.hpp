#ifndef LUSAN_COMMON_ELEMENTBASE_HPP
#define LUSAN_COMMON_ELEMENTBASE_HPP
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
 *  \file        lusan/common/ElementBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Element base object.
 *
 ************************************************************************/

class ElementBase
{
protected:
    ElementBase(ElementBase * parent = nullptr);

    ElementBase(unsigned int id, ElementBase* parent = nullptr);
    
    ElementBase(const ElementBase& src);
    
    ElementBase(ElementBase&& src);

public:
    virtual ~ElementBase(void);

public:
    ElementBase& operator = (const ElementBase& src);

    ElementBase& operator = (ElementBase&& src);

    bool operator == (const ElementBase& src) const;

    bool operator != (const ElementBase& src) const;

public:

    inline void setId(unsigned int id);
    
    inline unsigned int getId(void) const;

    inline void setParent(ElementBase* parent);

    inline ElementBase* getParent(void) const;

protected:
    
    inline void setMaxId(unsigned int id);
    
    inline unsigned int getNextId(void) const;
    
private:
    mutable unsigned int    mId;
    mutable ElementBase *   mParent;
};

inline void ElementBase::setId(unsigned int id)
{
    setMaxId(id);
    mId = id;
}

inline unsigned int ElementBase::getId(void) const
{
    if (mId == 0)
    {
        mId = getNextId();
    }

    return mId;
}

inline void ElementBase::setMaxId(unsigned int id)
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

inline unsigned int ElementBase::getNextId(void) const
{
    return (mParent != nullptr ? mParent->getNextId() : ++mId);
}

inline void ElementBase::setParent(ElementBase* parent)
{
    mParent = parent;
}

inline ElementBase* ElementBase::getParent(void) const
{
    return mParent;
}

#endif  // LUSAN_COMMON_ELEMENTBASE_HPP
