#ifndef LUSAN_DATA_COMMON_DATATYPEFACTORY_HPP
#define LUSAN_DATA_COMMON_DATATYPEFACTORY_HPP
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
 *  \file        lusan/data/common/DataTypeFactory.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type factory.
 *
 ************************************************************************/

/************************************************************************
 * Include files
 ************************************************************************/
#include <QString>
#include <memory>
#include "lusan/data/common/DataTypeBase.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeBase;
class DataTypePrimitive;
class DataTypeBasicObject;
class DataTypeBasicContainer;
class DataTypeCustom;

/**
 * \brief   The factory class to create data types.
 **/
class DataTypeFactory
{
public:

    /**
     * \brief   Converts the given data type to string value.
     * \param   dataType    The data type to convert to string.
     * \return  Returns the string value of the data type.
     **/
    static DataTypeBase::eCategory fromString(const QString& dataType);

    /**
     * \brief   Creates the data type object based on the given type.
     * \param   dataType    The type of the data type to create.
     * \return  Returns the created data type object.
     **/
    static DataTypeBase* createDataType(const QString& dataType);

    /**
     * \brief   Creates the data type object based on the given type.
     * \param   type    The type of the data type to create.
     * \return  Returns the created data type object.
     **/
    static DataTypeCustom* createCustomDataType(const QString& type);

    /**
     * \brief   Creates the data type object based on the given category.
     * \param   category    The category of the data type to create.
     * \return  Returns the created data type object.
     **/
    static DataTypeCustom* createCustomDataType(DataTypeBase::eCategory category);

    /**
     * \brief   Returns the list of primitive data type objects.
     **/
    static const QList<DataTypePrimitive*>& getPrimitiveTypes(void);

    /**
     * \brief   Returns the list of basic data type objects.
     **/
    static const QList<DataTypeBasicObject*>& getBasicTypes(void);

    /**
     * \brief   Returns the list of container data type objects.
     **/
    static const QList<DataTypeBasicContainer*>& getContainerTypes(void);

    /**
     * \brief   Returns the list of predefined data types.
     * \param   result      On output, the list of predefined data types.
     * \param   categories  The list of data type categories to search.
     * \return  Returns the number of predefined data types copied in the 'result'.
     **/
    static int getPredefinedTypes(QList<DataTypeBase *>& result, const QList<DataTypeBase::eCategory> & categories);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Initializes the predefined data types.
     **/
    static void _initPredefined(void);

//////////////////////////////////////////////////////////////////////////
// Hidden objects
//////////////////////////////////////////////////////////////////////////
private:
    static QList<DataTypePrimitive*>        mPredefinePrimitiveTypes;    //!< The list of primitive data types.
    static QList<DataTypeBasicObject*>      mPredefinedBasicTypes;       //!< The list of basic data types.
    static QList<DataTypeBasicContainer*>   mPredefinedContainerTypes;   //!< The list of container data types.
};

#endif  // LUSAN_DATA_COMMON_DATATYPEFACTORY_HPP
