#ifndef LUSAN_DATA_COMMON_PARAMTYPE_HPP
#define LUSAN_DATA_COMMON_PARAMTYPE_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/common/ParamType.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, parameter type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/TETypeWrap.hpp"

class DataTypeContainer;
class DataTypeCustom;
class DataTypeEnum;
class DataTypeImported;
class DataTypeStructure;

//////////////////////////////////////////////////////////////////////////
// TypeFinder class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The class to search an entry in the list
 **/
class TypeFinder : public TETypeFind<DataTypeBase, DataTypeCustom>
{
public:
    TypeFinder() = default;
    ~TypeFinder() = default;
public:
    DataTypeBase* findObject(const QString& name, const QList<DataTypeCustom*>& listTypes) const;
};

//////////////////////////////////////////////////////////////////////////
// ParamType class declaration
//////////////////////////////////////////////////////////////////////////

class ParamType : public TETypeWrap<DataTypeBase, DataTypeCustom, TypeFinder>
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    ParamType() = default;
    ParamType(const ParamType& src) = default;
    ParamType(ParamType&& src) noexcept = default;

    ParamType(const QString& typeName);
    ParamType(const QString& typeName, const QList<DataTypeCustom*>& customTypes);
    ParamType(DataTypeBase* dataType);

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    ParamType& operator = (const ParamType& src) = default;
    ParamType& operator = (ParamType&& src) noexcept = default;

    ParamType& operator = (const DataTypeBase* dataType);
    ParamType& operator = (DataTypeBase* dataType);

    ParamType& operator = (const QString& typeName);

    bool operator == (const ParamType& other) const;
    bool operator != (const ParamType& other) const;

    bool operator == (const DataTypeBase* dataType) const;
    bool operator != (const DataTypeBase* dataType) const;

    bool operator == (const QString& typeName) const;
    bool operator != (const QString& typeName) const;

    operator const DataTypeCustom* () const;
    operator DataTypeCustom* ();

    operator const DataTypeStructure* () const;
    operator DataTypeStructure* ();

    operator const DataTypeEnum* () const;
    operator DataTypeEnum* ();

    operator const DataTypeContainer* () const;
    operator DataTypeContainer* ();

    operator const DataTypeImported* () const;
    operator DataTypeImported* ();

    /**
     * \brief   Gets the category of the data type.
     * \return  The category of the data type.
     **/
    inline DataTypeBase::eCategory getCategory() const;

    /**
     * \brief   Checks if the data type is a primitive type.
     * \return  True if the data type is a primitive type, false otherwise.
     **/
    inline bool isPrimitive() const;

    /**
     * \brief   Checks if the data type is a custom defined type.
     * \return  True if the data type is a custom defined type, false otherwise.
     **/
    inline bool isCustomDefined() const;

    /**
     * \brief   Checks if the data type is a predefined type.
     * \return  True if the data type is a predefined type, false otherwise.
     **/
    inline bool isPredefined() const;

    /**
     * \brief   Checks if the data type is a boolean type.
     * \return  True if the data type is a boolean type, false otherwise.
     **/
    inline bool isPrimitiveBool() const;

    /**
     * \brief   Checks if the data type is a primitive signed integer type.
     * \return  True if the data type is a primitive integer type, false otherwise.
     **/
    inline bool isPrimitiveInt() const;

    /**
     * \brief   Checks if the data type is a primitive signed integer type.
     * \return  True if the data type is a primitive integer type, false otherwise.
     **/
    inline bool isPrimitiveSint() const;

    /**
     * \brief   Checks if the data type is a primitive unsigned integer type.
     * \return  True if the data type is a primitive unsigned integer type, false otherwise.
     **/
    inline bool isPrimitiveUint() const;

    /**
     * \brief   Checks if the data type is a primitive float type.
     * \return  True if the data type is a primitive float type, false otherwise.
     **/
    inline bool isPrimitiveFloat() const;

    /**
     * \brief   Checks if the data type is a basic object.
     * \return  True if the data type is a basic object, false otherwise.
     **/
    inline bool isBasicObject() const;

    /**
     * \brief   Checks if the data type is a basic container.
     * \return  True if the data type is a basic container, false otherwise.
     **/
    inline bool isBasicContainer() const;

    /**
     * \brief   Checks if the data type is an enumeration.
     * \return  True if the data type is an enumeration, false otherwise.
     **/
    inline bool isEnumeration() const;

    /**
     * \brief   Checks if the data type is a structure.
     * \return  True if the data type is a structure, false otherwise.
     **/
    inline bool isStructure() const;

    /**
     * \brief   Checks if the data type is imported.
     * \return  True if the data type is imported, false otherwise.
     **/
    inline bool isImported() const;

    /**
     * \brief   Checks if the data type is a container.
     * \return  True if the data type is a container, false otherwise.
     **/
    inline bool isContainer() const;

    /**
     * \brief   Checks if the data type matches the specified type.
     * \param   theType The type to check against.
     * \return  True if the data type matches the specified type, false otherwise.
     **/
    inline bool isTypeOf(const QString& theType) const;
};

//////////////////////////////////////////////////////////////////////////
// ParamType class inline functions
//////////////////////////////////////////////////////////////////////////

inline DataTypeBase::eCategory ParamType::getCategory() const
{
    return mTypeObj->getCategory();
}

inline bool ParamType::isPrimitive() const
{
    return mTypeObj->isPrimitive();
}

inline bool ParamType::isCustomDefined() const
{
    return mTypeObj->isCustomDefined();
}

inline bool ParamType::isPredefined() const
{
    return mTypeObj->isPredefined();
}

inline bool ParamType::isPrimitiveBool() const
{
    return mTypeObj->isPrimitiveBool();
}

inline bool ParamType::isPrimitiveInt() const
{
    return mTypeObj->isPrimitiveInt();
}

inline bool ParamType::isPrimitiveSint() const
{
    return mTypeObj->isPrimitiveSint();
}

inline bool ParamType::isPrimitiveUint() const
{
    return mTypeObj->isPrimitiveUint();
}

inline bool ParamType::isPrimitiveFloat() const
{
    return mTypeObj->isPrimitiveFloat();
}

inline bool ParamType::isBasicObject() const
{
    return mTypeObj->isBasicObject();
}

inline bool ParamType::isBasicContainer() const
{
    return mTypeObj->isBasicContainer();
}

inline bool ParamType::isEnumeration() const
{
    return mTypeObj->isEnumeration();
}

inline bool ParamType::isStructure() const
{
    return mTypeObj->isStructure();
}

inline bool ParamType::isImported() const
{
    return mTypeObj->isImported();
}

inline bool ParamType::isContainer() const
{
    return mTypeObj->isContainer();
}

inline bool ParamType::isTypeOf(const QString& theType) const
{
    return mTypeObj->isTypeOf(theType);
}

#endif  // LUSAN_DATA_COMMON_PARAMTYPE_HPP
