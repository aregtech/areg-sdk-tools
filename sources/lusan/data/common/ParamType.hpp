#ifndef LUSAN_DATA_COMMON_PARAMTYPE_HPP
#define LUSAN_DATA_COMMON_PARAMTYPE_HPP
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
 *  \file        lusan/data/common/ParamType.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
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
    TypeFinder(void) = default;
    ~TypeFinder(void) = default;
public:
    DataTypeBase* findObject(const QString name, const QList<DataTypeCustom*>& listTypes) const;
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
    ParamType(void) = default;
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

    operator const DataTypeCustom* (void) const;
    operator DataTypeCustom* (void);

    operator const DataTypeStructure* (void) const;
    operator DataTypeStructure* (void);

    operator const DataTypeEnum* (void) const;
    operator DataTypeEnum* (void);

    operator const DataTypeContainer* (void) const;
    operator DataTypeContainer* (void);

    operator const DataTypeImported* (void) const;
    operator DataTypeImported* (void);

    /**
     * \brief   Gets the category of the data type.
     * \return  The category of the data type.
     **/
    inline DataTypeBase::eCategory getCategory(void) const;

    /**
     * \brief   Checks if the data type is a primitive type.
     * \return  True if the data type is a primitive type, false otherwise.
     **/
    inline bool isPrimitive(void) const;

    /**
     * \brief   Checks if the data type is a custom defined type.
     * \return  True if the data type is a custom defined type, false otherwise.
     **/
    inline bool isCustomDefined(void) const;

    /**
     * \brief   Checks if the data type is a predefined type.
     * \return  True if the data type is a predefined type, false otherwise.
     **/
    inline bool isPredefined(void) const;

    /**
     * \brief   Checks if the data type is a boolean type.
     * \return  True if the data type is a boolean type, false otherwise.
     **/
    inline bool isPrimitiveBool(void) const;

    /**
     * \brief   Checks if the data type is a primitive signed integer type.
     * \return  True if the data type is a primitive integer type, false otherwise.
     **/
    inline bool isPrimitiveInt(void) const;

    /**
     * \brief   Checks if the data type is a primitive signed integer type.
     * \return  True if the data type is a primitive integer type, false otherwise.
     **/
    inline bool isPrimitiveSint(void) const;

    /**
     * \brief   Checks if the data type is a primitive unsigned integer type.
     * \return  True if the data type is a primitive unsigned integer type, false otherwise.
     **/
    inline bool isPrimitiveUint(void) const;

    /**
     * \brief   Checks if the data type is a primitive float type.
     * \return  True if the data type is a primitive float type, false otherwise.
     **/
    inline bool isPrimitiveFloat(void) const;

    /**
     * \brief   Checks if the data type is a basic object.
     * \return  True if the data type is a basic object, false otherwise.
     **/
    inline bool isBasicObject(void) const;

    /**
     * \brief   Checks if the data type is a basic container.
     * \return  True if the data type is a basic container, false otherwise.
     **/
    inline bool isBasicContainer(void) const;

    /**
     * \brief   Checks if the data type is an enumeration.
     * \return  True if the data type is an enumeration, false otherwise.
     **/
    inline bool isEnumeration(void) const;

    /**
     * \brief   Checks if the data type is a structure.
     * \return  True if the data type is a structure, false otherwise.
     **/
    inline bool isStructure(void) const;

    /**
     * \brief   Checks if the data type is imported.
     * \return  True if the data type is imported, false otherwise.
     **/
    inline bool isImported(void) const;

    /**
     * \brief   Checks if the data type is a container.
     * \return  True if the data type is a container, false otherwise.
     **/
    inline bool isContainer(void) const;

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

inline DataTypeBase::eCategory ParamType::getCategory(void) const
{
    return mTypeObj->getCategory();
}

inline bool ParamType::isPrimitive(void) const
{
    return mTypeObj->isPrimitive();
}

inline bool ParamType::isCustomDefined(void) const
{
    return mTypeObj->isCustomDefined();
}

inline bool ParamType::isPredefined(void) const
{
    return mTypeObj->isPredefined();
}

inline bool ParamType::isPrimitiveBool(void) const
{
    return mTypeObj->isPrimitiveBool();
}

inline bool ParamType::isPrimitiveInt(void) const
{
    return mTypeObj->isPrimitiveInt();
}

inline bool ParamType::isPrimitiveSint(void) const
{
    return mTypeObj->isPrimitiveSint();
}

inline bool ParamType::isPrimitiveUint(void) const
{
    return mTypeObj->isPrimitiveUint();
}

inline bool ParamType::isPrimitiveFloat(void) const
{
    return mTypeObj->isPrimitiveFloat();
}

inline bool ParamType::isBasicObject(void) const
{
    return mTypeObj->isBasicObject();
}

inline bool ParamType::isBasicContainer(void) const
{
    return mTypeObj->isBasicContainer();
}

inline bool ParamType::isEnumeration(void) const
{
    return mTypeObj->isEnumeration();
}

inline bool ParamType::isStructure(void) const
{
    return mTypeObj->isStructure();
}

inline bool ParamType::isImported(void) const
{
    return mTypeObj->isImported();
}

inline bool ParamType::isContainer(void) const
{
    return mTypeObj->isContainer();
}

inline bool ParamType::isTypeOf(const QString& theType) const
{
    return mTypeObj->isTypeOf(theType);
}

#endif  // LUSAN_DATA_COMMON_PARAMTYPE_HPP
