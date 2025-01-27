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
#include "lusan/data/common/DataTypeBase.hpp"
#include <QList>
#include <QString>

class DataTypeContainer;
class DataTypeCustom;
class DataTypeEnum;
class DataTypeImported;
class DataTypeStructure; 

class ParamType
{
public:
    ParamType(void);
    ParamType(const ParamType& src);
    ParamType(ParamType&& src) noexcept;

    ParamType(const QString& typeName);
    ParamType(const QString& typeName, const QList<DataTypeCustom*>& customTypes);
    ParamType(DataTypeBase* dataType);

    ParamType& operator = (const ParamType& src);
    ParamType& operator = (ParamType&& src) noexcept;

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

    operator const QString& (void) const;

    inline const DataTypeBase* getDataType(void) const;
    
    inline DataTypeBase* getDataType(void);
    
    inline void setDataType(DataTypeBase* dataType);
    
    inline void setDataType(const DataTypeBase* dataType);
    
    inline void setName(const QString& typeName);
    
    inline void setName(const QString& typeName, const QList<DataTypeCustom *>& customTypes);
    
    inline const QString& getName(void) const;
    
    inline bool isEmpty(void) const;
    
    inline bool isValid(void) const;

    inline void invalidate();

    bool validate(const QList<DataTypeCustom *> & customTypes);

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

private:
    DataTypeBase*   mDataType;
    QString         mTypeName;
};

inline const DataTypeBase* ParamType::getDataType(void) const
{
    return mDataType;
}

inline DataTypeBase* ParamType::getDataType(void)
{
    return mDataType;
}

inline void ParamType::setDataType(DataTypeBase* dataType)
{
    if (dataType != nullptr)
    {
        mDataType = dataType;
        mTypeName = mDataType->getName();
    }
    else
    {
        if (mDataType != nullptr)
        {
            mTypeName = mDataType->getName();
        }

        mDataType = nullptr;
    }
}

inline void ParamType::setDataType(const DataTypeBase* dataType)
{
    setDataType(const_cast<DataTypeBase*>(dataType));
}

inline void ParamType::setName(const QString& typeName)
{
    if (mDataType == nullptr)
    {
        mTypeName = typeName;
    }
    else if (typeName != mTypeName)
    {
        mDataType = nullptr;
        mTypeName = typeName;
    }
}

inline void ParamType::setName(const QString& typeName, const QList<DataTypeCustom *>& customTypes)
{
    setName(typeName);
    validate(customTypes);
}

inline const QString& ParamType::getName(void) const
{
    return (mDataType != nullptr ? mDataType->getName() : mTypeName);
}

inline bool ParamType::isEmpty(void) const
{
    return mDataType != nullptr ? mDataType->getName().isEmpty() : mTypeName.isEmpty();
}

inline bool ParamType::isValid(void) const
{
    return (mDataType != nullptr);
}

inline void ParamType::invalidate()
{
    mDataType = nullptr;
}

inline DataTypeBase::eCategory ParamType::getCategory(void) const
{
    return mDataType->getCategory();
}

inline bool ParamType::isPrimitive(void) const
{
    return mDataType->isPrimitive();
}

inline bool ParamType::isCustomDefined(void) const
{
    return mDataType->isCustomDefined();
}

inline bool ParamType::isPredefined(void) const
{
    return mDataType->isPredefined();
}

inline bool ParamType::isPrimitiveBool(void) const
{
    return mDataType->isPrimitiveBool();
}

inline bool ParamType::isPrimitiveInt(void) const
{
    return mDataType->isPrimitiveInt();
}

inline bool ParamType::isPrimitiveSint(void) const
{
    return mDataType->isPrimitiveSint();
}

inline bool ParamType::isPrimitiveUint(void) const
{
    return mDataType->isPrimitiveUint();
}

inline bool ParamType::isPrimitiveFloat(void) const
{
    return mDataType->isPrimitiveFloat();
}

inline bool ParamType::isBasicObject(void) const
{
    return mDataType->isBasicObject();
}

inline bool ParamType::isBasicContainer(void) const
{
    return mDataType->isBasicContainer();
}

inline bool ParamType::isEnumeration(void) const
{
    return mDataType->isEnumeration();
}

inline bool ParamType::isStructure(void) const
{
    return mDataType->isStructure();
}

inline bool ParamType::isImported(void) const
{
    return mDataType->isImported();
}

inline bool ParamType::isContainer(void) const
{
    return mDataType->isContainer();
}

inline bool ParamType::isTypeOf(const QString& theType) const
{
    return mDataType->isTypeOf(theType);
}

#endif  // LUSAN_DATA_COMMON_PARAMTYPE_HPP
