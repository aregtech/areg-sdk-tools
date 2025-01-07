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
 ************************************************************************/

#ifndef LUSAN_COMMON_ELEMENTBASE_HPP
#define LUSAN_COMMON_ELEMENTBASE_HPP

/************************************************************************
 * \class ElementBase
 * \brief Base class for elements in the Lusan application.
 * 
 * This class provides the basic functionality for elements, including
 * setting and getting IDs, managing parent-child relationships, and
 * ensuring unique IDs.
 ************************************************************************/
class ElementBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief Constructor with optional parent element.
     * \param parent Pointer to the parent element.
     */
    ElementBase(ElementBase * parent = nullptr);

    /**
     * \brief Constructor with ID and optional parent element.
     * \param id The ID of the element.
     * \param parent Pointer to the parent element.
     */
    ElementBase(unsigned int id, ElementBase* parent = nullptr);
    
    /**
     * \brief Copy constructor.
     * \param src The source element to copy from.
     */
    ElementBase(const ElementBase& src);
    
    /**
     * \brief Move constructor.
     * \param src The source element to move from.
     */
    ElementBase(ElementBase&& src);

public:
    /**
     * \brief Destructor.
     */
    virtual ~ElementBase(void);

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief Copy assignment operator.
     * \param src The source element to copy from.
     * \return Reference to this element.
     */
    ElementBase& operator = (const ElementBase& src);

    /**
     * \brief Move assignment operator.
     * \param src The source element to move from.
     * \return Reference to this element.
     */
    ElementBase& operator = (ElementBase&& src);

    /**
     * \brief Equality operator.
     * \param src The source element to compare with.
     * \return True if elements are equal, false otherwise.
     */
    bool operator == (const ElementBase& src) const;

    /**
     * \brief Inequality operator.
     * \param src The source element to compare with.
     * \return True if elements are not equal, false otherwise.
     */
    bool operator != (const ElementBase& src) const;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief Sets the ID of the element.
     * \param id The ID to set.
     */
    inline void setId(unsigned int id) const;
    
    /**
     * \brief Gets the ID of the element.
     * \return The ID of the element.
     */
    inline unsigned int getId(void) const;

    /**
     * \brief Sets the parent element.
     * \param parent Pointer to the parent element.
     */
    inline void setParent(ElementBase* parent) const;

    /**
     * \brief Sets the parent element.
     * \param parent Pointer to the parent element (const).
     */
    inline void setParent(const ElementBase* parent) const;

    /**
     * \brief Gets the parent element.
     * \return Pointer to the parent element.
     */
    inline ElementBase* getParent(void) const;

    /**
     * \brief Gets the next available ID.
     * \return The next available ID.
     */
    inline unsigned int getNextId(void) const;
    
protected:
    /**
     * \brief Sets the maximum ID.
     * \param id The maximum ID to set.
     */
    inline void setMaxId(unsigned int id) const;
    
//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    mutable unsigned int    mId;      //!< The ID of the element.
    mutable ElementBase *   mParent;  //!< Pointer to the parent element.
};

//////////////////////////////////////////////////////////////////////////
// ElementBase class inline functions
//////////////////////////////////////////////////////////////////////////

inline void ElementBase::setId(unsigned int id) const
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

inline void ElementBase::setMaxId(unsigned int id) const
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

inline void ElementBase::setParent(ElementBase* parent) const
{
    mParent = parent;
}

inline void ElementBase::setParent(const ElementBase* parent) const
{
    mParent = const_cast<ElementBase *>(parent);
}

inline ElementBase* ElementBase::getParent(void) const
{
    return const_cast<ElementBase *>(mParent);
}

#endif  // LUSAN_COMMON_ELEMENTBASE_HPP
