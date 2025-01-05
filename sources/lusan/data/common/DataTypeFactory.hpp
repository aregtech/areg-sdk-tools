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

class DataTypeBase;
class DataTypePrimitive;
class DataTypeBasic;
class DataTypeCustom;

/**
 * \brief   The factory class to create data types.
 **/
class DataTypeFactory
{
public:

    static constexpr const char* const  TypeNameBool        { "bool"        };  //!< The boolean data type.
    static constexpr const char* const  TypeNameChar        { "char"        };  //!< The 8-bit signed integer data type, or char
    static constexpr const char* const  TypeNameUint8       { "uint8"       };  //!< The 8-bit unsigned integer data type, or byte
    static constexpr const char* const  TypeNameInt16       { "int16"       };  //!< The 16-bit signed integer data type, or short
    static constexpr const char* const  TypeNameUint16      { "uint16"      };  //!< The 16-bit unsigned integer data type, or word
    static constexpr const char* const  TypeNameInt32       { "int32"       };  //!< The 32-bit signed integer data type, or int
    static constexpr const char* const  TypeNameUint32      { "uint32"      };  //!< The 32-bit unsigned integer data type, or dword
    static constexpr const char* const  TypeNameInt64       { "int64"       };  //!< The 64-bit signed integer data type, or long
    static constexpr const char* const  TypeNameUint64      { "uint64"      };  //!< The 64-bit unsigned integer data type, or qword
    static constexpr const char* const  TypeNameFloat       { "float"       };  //!< The single precision floating point data type
    static constexpr const char* const  TypeNameDouble      { "double"      };  //!< The double precision floating point data type
    static constexpr const char* const  TypeNameString      { "String"      };  //!< The string data type
    static constexpr const char* const  TypeNameBinary      { "BinaryBuffer"};  //!< The binary (Byte Buffer) data type
    static constexpr const char* const  TypeNameDateTime    { "DateTime"    };  //!< The date and time data type
    static constexpr const char* const  TypeNameArray       { "Array"       };  //!< The array data type
    static constexpr const char* const  TypeNameLinkedList  { "LinkedList"  };  //!< The linked list data type
    static constexpr const char* const  TypeNameHashMap     { "HashMap"     };  //!< The hash map data type
    static constexpr const char* const  TypeNameMap         { "Map"         };  //!< The map data type
    static constexpr const char* const  TypeNamePair        { "Pair"        };  //!< The pair data type
    static constexpr const char* const  TypeNameNewType     { "NewType"     };  //!< The new container type data type
    static constexpr const char* const  TypeNameEnumerate   { "Enumerate"   };  //!< The custom enumeration data type
    static constexpr const char* const  TypeNameStructure   { "Structure"   };  //!< The custom structure data type
    static constexpr const char* const  TypeNameImported    { "Imported"    };  //!< The custom imported data type
    static constexpr const char* const  TypeNameDefined     { "Defined"     };  //!< The custom defined data type

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

    static const QList<DataTypePrimitive*>& getPrimitiveTypes(void);

    static const QList<DataTypeBasic*>& getBasicTypes(void);

    static const QList<DataTypeBasic*>& getContainerTypes(void);

private:

    static void _initPredefined(void);

private:
    static QList<DataTypePrimitive*>   mPredefinePrimitiveTypes;    //!< The list of primitive data types.
    static QList<DataTypeBasic*>       mPredefinedBasicTypes;       //!< The list of basic data types.
    static QList<DataTypeBasic*>       mPredefinedContainerTypes;   //!< The list of container data types.
};

#endif  // LUSAN_DATA_COMMON_DATATYPEFACTORY_HPP
