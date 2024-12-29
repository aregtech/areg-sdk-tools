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

#include <QString>
#include <memory>
#include "lusan/data/common/DataTypeBase.hpp"

class DataTypeCustom;

/**
 * \brief   The factory class to create data types.
 **/
class DataTypeFactory
{
public:
    /**
     * \brief   Creates the data type object based on the given type.
     * \param   type    The type of the data type to create.
     * \return  Returns the created data type object.
     **/
    static DataTypeCustom* createDataType(const QString& type);

    /**
     * \brief   Creates the data type object based on the given category.
     * \param   category    The category of the data type to create.
     * \return  Returns the created data type object.
     **/
    static DataTypeCustom* createDataType(DataTypeBase::eCategory category);
};

#endif  // LUSAN_DATA_COMMON_DATATYPEFACTORY_HPP
