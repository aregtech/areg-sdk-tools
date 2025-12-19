#ifndef LUSAN_DATA_COMMON_TETYPEWRAP_HPP
#define LUSAN_DATA_COMMON_TETYPEWRAP_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/common/TETypeWrap.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Template Data Container.
 *
 ************************************************************************/

#include <QString>
#include <QList>

////////////////////////////////////////////////////////////////////////
// TETypeFind class template declaration
////////////////////////////////////////////////////////////////////////

/**
 * \brief   A class template to find an entry with the given unique name.
 * \tparam  TypeBase    The return value type, which may differ from the list type.
 * \tparam  Type        The type of the list elements.
 **/
template<typename TypeBase, typename Type = TypeBase>
class TETypeFind
{
public:
    TETypeFind(void) = default;
    ~TETypeFind(void) = default;

public:
    /**
     * \brief   Searches for an entry with the given name in the list.
     * \param   name        The name of the object to search for.
     * \param   listTypes   The list to search in.
     * \return  Returns a valid pointer to the object if found, otherwise returns nullptr.
     **/
    TypeBase* findObject(const QString name, const QList<Type*>& listTypes) const;
};

////////////////////////////////////////////////////////////////////////
// TETypeFind class template methods
////////////////////////////////////////////////////////////////////////
template<typename TypeBase, typename Type /*= TypeBase*/>
inline TypeBase* TETypeFind<TypeBase, Type>::findObject(const QString name, const QList<Type*>& listTypes) const
{
    for (auto obj : listTypes)
    {
        if (obj->getName() == name)
            return static_cast<TypeBase *>(obj);
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////
// TETypeWrap class template declaration
////////////////////////////////////////////////////////////////////////

/**
 * \brief   A class template to wrap and manage type objects.
 * \tparam  Type        The type of the object to wrap.
 * \tparam  TypeSearch  The type used for searching, defaults to Type.
 * \tparam  Finder      The finder class used to search for objects, defaults to TETypeFind<TypeSearch>.
 **/
template<typename Type, typename TypeSearch = Type, class Finder = TETypeFind<TypeSearch>>
class TETypeWrap
{
////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    inline TETypeWrap(void);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move.
     **/
    inline TETypeWrap(TETypeWrap&& src) noexcept;

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy.
     **/
    inline TETypeWrap(const TETypeWrap& src);

    /**
     * \brief   Constructor with type name initialization.
     * \param   typeName    The name of the type.
     **/
    inline TETypeWrap(const QString& typeName);

    /**
     * \brief   Constructor with type name and list initialization.
     * \param   typeName    The name of the type.
     * \param   listTypes   The list of types to search in.
     **/
    inline TETypeWrap(const QString& typeName, const QList<TypeSearch *>& listTypes);

    /**
     * \brief   Constructor with type object initialization.
     * \param   objType     The type object.
     **/
    inline TETypeWrap(Type* objType);

    /**
     * \brief   Destructor.
     **/
    ~TETypeWrap(void) = default;

    /**
     * \brief   Copy assignment operator.
     * \param   src     The source object to copy.
     * \return  Reference to this object.
     **/
    inline TETypeWrap& operator = (const TETypeWrap& src);

    /**
     * \brief   Move assignment operator.
     * \param   src     The source object to move.
     * \return  Reference to this object.
     **/
    inline TETypeWrap& operator = (TETypeWrap&& src) noexcept;

    /**
     * \brief   Assignment operator with type object.
     * \param   objType     The type object.
     * \return  Reference to this object.
     **/
    inline TETypeWrap& operator = (const Type* objType);

    /**
     * \brief   Assignment operator with type object.
     * \param   objType     The type object.
     * \return  Reference to this object.
     **/
    inline TETypeWrap& operator = (Type* objType);

    /**
     * \brief   Assignment operator with type name.
     * \param   typeName    The name of the type.
     * \return  Reference to this object.
     **/
    inline TETypeWrap& operator = (const QString& typeName);

    /**
     * \brief   Equality operator.
     * \param   other   The other object to compare with.
     * \return  True if equal, false otherwise.
     **/
    inline bool operator == (const TETypeWrap& other) const;

    /**
     * \brief   Inequality operator.
     * \param   other   The other object to compare with.
     * \return  True if not equal, false otherwise.
     **/
    inline bool operator != (const TETypeWrap& other) const;

    /**
     * \brief   Equality operator with type object.
     * \param   objType     The type object to compare with.
     * \return  True if equal, false otherwise.
     **/
    inline bool operator == (const Type* objType) const;

    /**
     * \brief   Inequality operator with type object.
     * \param   objType     The type object to compare with.
     * \return  True if not equal, false otherwise.
     **/
    inline bool operator != (const Type* objType) const;

    /**
     * \brief   Equality operator with type name.
     * \param   typeName    The name of the type to compare with.
     * \return  True if equal, false otherwise.
     **/
    inline bool operator == (const QString& typeName) const;

    /**
     * \brief   Inequality operator with type name.
     * \param   typeName    The name of the type to compare with.
     * \return  True if not equal, false otherwise.
     **/
    inline bool operator != (const QString& typeName) const;

    /**
     * \brief   Conversion operator to QString.
     * \return  The name of the type.
     **/
    inline operator const QString& (void) const;

    /**
     * \brief   Conversion operator to const Type*.
     * \return  The type object.
     **/
    inline operator const Type* (void) const;

    /**
     * \brief   Conversion operator to Type*.
     * \return  The type object.
     **/
    inline operator Type* (void);

////////////////////////////////////////////////////////////////////////
// Attributes and operations
////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the type object.
     * \return  The type object.
     **/
    inline const Type* getType(void) const;

    /**
     * \brief   Gets the type object.
     * \return  The type object.
     **/
    inline Type* getType(void);

    /**
     * \brief   Sets the type object.
     * \param   objType     The type object.
     **/
    inline void setType(Type* objType);

    /**
     * \brief   Sets the type object.
     * \param   objType     The type object.
     **/
    inline void setType(const Type* objType);

    /**
     * \brief   Sets the type name.
     * \param   typeName    The name of the type.
     **/
    inline void setName(const QString& typeName);

    /**
     * \brief   Sets the type name and validates it against the list.
     * \param   typeName    The name of the type.
     * \param   listTypes   The list of types to search in.
     **/
    inline void setName(const QString& typeName, const QList<TypeSearch *>& listTypes);

    /**
     * \brief   Gets the type name.
     * \return  The name of the type.
     **/
    inline const QString& getName(void) const;

    /**
     * \brief   Checks if the type name is empty.
     * \return  True if empty, false otherwise.
     **/
    inline bool isEmpty(void) const;

    /**
     * \brief   Checks if the type object is valid.
     * \return  True if valid, false otherwise.
     **/
    inline bool isValid(void) const;

    /**
     * \brief   Invalidates the type object.
     **/
    inline void invalidate();

    /**
     * \brief   Validates the type object against the list.
     * \param   listTypes   The list of types to search in.
     * \return  True if valid, false otherwise.
     **/
    inline bool validate(const QList<TypeSearch *>& listTypes);

////////////////////////////////////////////////////////////////////////
// Protected member variables
////////////////////////////////////////////////////////////////////////
protected:
    QString     mTypeName;  //!< The name of the type.
    Type*       mTypeObj;   //!< The type object.
};

////////////////////////////////////////////////////////////////////////
// TETypeWrap class template methods
////////////////////////////////////////////////////////////////////////

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline TETypeWrap<Type, TypeSearch, Finder>::TETypeWrap(void)
    : mTypeName ()
    , mTypeObj  (nullptr)
{
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline TETypeWrap<Type, TypeSearch, Finder>::TETypeWrap(TETypeWrap&& src) noexcept
    : mTypeName (std::move(src.mTypeName))
    , mTypeObj  (src.mTypeObj)
{
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline TETypeWrap<Type, TypeSearch, Finder>::TETypeWrap(const TETypeWrap& src)
    : mTypeName (src.mTypeName)
    , mTypeObj  (src.mTypeObj)
{
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline TETypeWrap<Type, TypeSearch, Finder>::TETypeWrap(const QString& typeName)
    : mTypeName (typeName)
    , mTypeObj  (nullptr)
{
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline TETypeWrap<Type, TypeSearch, Finder>::TETypeWrap(const QString& typeName, const QList<TypeSearch *>& listTypes)
    : mTypeName (typeName)
    , mTypeObj  (nullptr)
{
    validate(listTypes);
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline TETypeWrap<Type, TypeSearch, Finder>::TETypeWrap(Type* objType)
    : mTypeName (objType != nullptr ? objType->getName() : QString())
    , mTypeObj  (objType)
{
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
TETypeWrap<Type, TypeSearch, Finder>& TETypeWrap<Type, TypeSearch, Finder>::operator = (const TETypeWrap<Type, TypeSearch, Finder>& src)
{
    mTypeName   = src.mTypeName;
    mTypeObj    = src.mTypeObj;
    return (*this);
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
TETypeWrap<Type, TypeSearch, Finder>& TETypeWrap<Type, TypeSearch, Finder>::operator = (TETypeWrap<Type, TypeSearch, Finder>&& src) noexcept
{
    mTypeName   = std::move(src.mTypeName);
    mTypeObj    = src.mTypeObj;
    return (*this);
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
TETypeWrap<Type, TypeSearch, Finder>& TETypeWrap<Type, TypeSearch, Finder>::operator = (const Type* objType)
{
    setType(objType);
    return (*this);
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
TETypeWrap<Type, TypeSearch, Finder>& TETypeWrap<Type, TypeSearch, Finder>::operator = (Type* objType)
{
    setType(objType);
    return (*this);
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
TETypeWrap<Type, TypeSearch, Finder>& TETypeWrap<Type, TypeSearch, Finder>::operator = (const QString& typeName)
{
    setName(typeName);
    return (*this);
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
bool TETypeWrap<Type, TypeSearch, Finder>::operator == (const TETypeWrap& other) const
{
    if (this == &other)
    {
        return true;
    }
    else if ((mTypeObj == other.mTypeObj) && (mTypeObj != nullptr))
    {
        return true;
    }
    else if ((mTypeObj != nullptr) && (other.mTypeObj != nullptr) && (mTypeObj->getName() == other.mTypeObj->getName()))
    {
        return true;
    }
    else if ((mTypeName == other.mTypeName) && (mTypeName.isEmpty() == false))
    {
        return true;
    }
    else
    {
        return false;
    }
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
bool TETypeWrap<Type, TypeSearch, Finder>::operator != (const TETypeWrap& other) const
{
    return !(operator == (other));
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
bool TETypeWrap<Type, TypeSearch, Finder>::operator == (const Type* objType) const
{
    return (mTypeObj != nullptr) && (mTypeObj == objType);
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
bool TETypeWrap<Type, TypeSearch, Finder>::operator != (const Type* objType) const
{
    return !(operator == (objType));
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
bool TETypeWrap<Type, TypeSearch, Finder>::operator == (const QString& typeName) const
{
    if (mTypeObj != nullptr)
    {
        return (mTypeObj->getName() == typeName);
    }
    else
    {
        return (mTypeName.isEmpty() == false) && (mTypeName == typeName);
    }
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
bool TETypeWrap<Type, TypeSearch, Finder>::operator != (const QString& typeName) const
{
    return !(operator == (typeName));
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline TETypeWrap<Type, TypeSearch, Finder>::operator const QString& (void) const
{
    return mTypeName;
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline TETypeWrap<Type, TypeSearch, Finder>::operator const Type* (void) const
{
    return mTypeObj;
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline TETypeWrap<Type, TypeSearch, Finder>::operator Type* (void)
{
    return mTypeObj;
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline const Type* TETypeWrap<Type, TypeSearch, Finder>::getType(void) const
{
    return mTypeObj;
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline Type* TETypeWrap<Type, TypeSearch, Finder>::getType(void)
{
    return mTypeObj;
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline void TETypeWrap<Type, TypeSearch, Finder>::setType(Type* objType)
{
    if (objType != nullptr)
    {
        mTypeObj = objType;
        mTypeName = mTypeObj->getName();
    }
    else
    {
        if (mTypeObj != nullptr)
        {
            mTypeName = mTypeObj->getName();
        }

        mTypeObj = nullptr;
    }
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline void TETypeWrap<Type, TypeSearch, Finder>::setType(const Type* objType)
{
    setType(const_cast<Type*>(objType));
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline void TETypeWrap<Type, TypeSearch, Finder>::setName(const QString& typeName)
{
    if (mTypeObj == nullptr)
    {
        mTypeName = typeName;
    }
    else if (typeName != mTypeName)
    {
        mTypeObj = nullptr;
        mTypeName = typeName;
    }
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline void TETypeWrap<Type, TypeSearch, Finder>::setName(const QString& typeName, const QList<TypeSearch*>& listTypes)
{
    setName(typeName);
    validate(listTypes);
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline const QString& TETypeWrap<Type, TypeSearch, Finder>::getName(void) const
{
    return (mTypeObj != nullptr ? mTypeObj->getName() : mTypeName);
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline bool TETypeWrap<Type, TypeSearch, Finder>::isEmpty(void) const
{
    return getName().isEmpty();
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline bool TETypeWrap<Type, TypeSearch, Finder>::isValid(void) const
{
    return (mTypeObj != nullptr);
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline void TETypeWrap<Type, TypeSearch, Finder>::invalidate()
{
    mTypeObj = nullptr;
}

template<typename Type, typename TypeSearch /*= Type*/, class Finder /*= TETypeFind<TypeSearch>*/>
inline bool TETypeWrap<Type, TypeSearch, Finder>::validate(const QList<TypeSearch*>& listTypes)
{
    if ((mTypeObj == nullptr) && (mTypeName.isEmpty() == false))
    {
        mTypeObj = static_cast<Type*>(Finder{}.findObject(mTypeName, listTypes));
    }

    return (mTypeObj != nullptr);
}

#endif // LUSAN_DATA_COMMON_TETYPEWRAP_HPP
