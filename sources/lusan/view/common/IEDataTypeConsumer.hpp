#ifndef LUSAN_VIEW_COMMON_IEDATATYPECONSUMER_HPP
#define LUSAN_VIEW_COMMON_IEDATATYPECONSUMER_HPP
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
 *  \file        lusan/view/common/IEDataTypeConsumer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type consumer object.
 *
 ************************************************************************/

 /************************************************************************
  * Includes
  ************************************************************************/

 /************************************************************************
  * Dependencies
  ************************************************************************/

class DataTypeCustom;

class IEDataTypeConsumer
{

protected:
    IEDataTypeConsumer(void) = default;
    virtual ~IEDataTypeConsumer(void) = default;

public:
    virtual void dataTypeCreated(DataTypeCustom* dataType);

    virtual void dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType);

    virtual void dataTypeRemoved(DataTypeCustom* dataType);

    virtual void dataTypeUpdated(DataTypeCustom* dataType);

};

#endif // LUSAN_VIEW_COMMON_IEDATATYPECONSUMER_HPP
