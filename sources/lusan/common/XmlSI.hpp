#ifndef LUSAN_COMMON_XMLSI_HPP
#define LUSAN_COMMON_XMLSI_HPP
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
 *  \file        lusan/common/XmlSI.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface XML tags.
 *
 ************************************************************************/

#include <QString>

/**
 * \brief   The list of tags and constants used in Service Interface XML file.
 **/
namespace XmlSI
{
    /**
     * \brief   Common XML element names of the Service Interface data.
     **/
    constexpr QLatin1StringView xmlSIElementAttribute       { "Attribute" };
    constexpr QLatin1StringView xmlSIElementAttributeList   { "AttributeList" };
    constexpr QLatin1StringView xmlSIElementBaseTypeKey     { "BaseTypeKey" };
    constexpr QLatin1StringView xmlSIElementBaseTypeValue   { "BaseTypeValue" };
    constexpr QLatin1StringView xmlSIElementConstant        { "Constant" };
    constexpr QLatin1StringView xmlSIElementConstantList    { "ConstantList" };
    constexpr QLatin1StringView xmlSIElementContainer       { "Container" };
    constexpr QLatin1StringView xmlSIElementDataType        { "DataType" };
    constexpr QLatin1StringView xmlSIElementDataTypeList    { "DataTypeList" };
    constexpr QLatin1StringView xmlSIElementDeprecateHint   { "DeprecateHint" };
    constexpr QLatin1StringView xmlSIElementDescription     { "Description" };
    constexpr QLatin1StringView xmlSIElementEnumEntry       { "EnumEntry" };
    constexpr QLatin1StringView xmlSIElementField           { "Field" };
    constexpr QLatin1StringView xmlSIElementFieldList       { "FieldList" };
    constexpr QLatin1StringView xmlSIElementImportedObject  { "ImportedObject" };
    constexpr QLatin1StringView xmlSIElementIncludeList     { "IncludeList" };
    constexpr QLatin1StringView xmlSIElementLocation        { "Location" };
    constexpr QLatin1StringView xmlSIElementMethod          { "Method" };
    constexpr QLatin1StringView xmlSIElementMethodList      { "MethodList" };
    constexpr QLatin1StringView xmlSIElementNamespace       { "Namespace" };
    constexpr QLatin1StringView xmlSIElementOverview        { "Overview" };
    constexpr QLatin1StringView xmlSIElementParameter       { "Parameter" };
    constexpr QLatin1StringView xmlSIElementParamList       { "ParamList" };
    constexpr QLatin1StringView xmlSIElementServiceInterface{ "ServiceInterface" };
    constexpr QLatin1StringView xmlSIElementValue           { "Value" };

    /**
     * \brief   Common XML element attributes of the Service Interface data.
     **/
    constexpr QLatin1StringView xmlSIAttributeCategory      { "Category" };
    constexpr QLatin1StringView xmlSIAttributeDataType      { "DataType" };
    constexpr QLatin1StringView xmlSIAttributeHasKey        { "HasKey" };
    constexpr QLatin1StringView xmlSIAttributeHasValue      { "HasValue" };
    constexpr QLatin1StringView xmlSIAttributeID            { "ID" };
    constexpr QLatin1StringView xmlSIAttributeIsDefault     { "IsDefault" };
    constexpr QLatin1StringView xmlSIAttributeIsDeprecated  { "IsDeprecated" };
    constexpr QLatin1StringView xmlSIAttributeFormatVersion { "FormatVersion" };
    constexpr QLatin1StringView xmlSIAttributeMethodType    { "MethodType" };
    constexpr QLatin1StringView xmlSIAttributeName          { "Name" };
    constexpr QLatin1StringView xmlSIAttributeNotify        { "Notify" };
    constexpr QLatin1StringView xmlSIAttributeType          { "Type" };
    constexpr QLatin1StringView xmlSIAttributeResponse      { "Response" };
    constexpr QLatin1StringView xmlSIAttributeValues        { "Values" };
    constexpr QLatin1StringView xmlSIAttributeVersion       { "Version" };
    
    constexpr QLatin1StringView xmlSIValueTrue              { "true" };
    constexpr QLatin1StringView xmlSIValueFalse             { "false" };

    constexpr QLatin1StringView xmlSIValuePrimitive         { "Primitive" };
    constexpr QLatin1StringView xmlSIValueBool              { "bool" };
    constexpr QLatin1StringView xmlSIValueChar              { "char" };
    constexpr QLatin1StringView xmlSIValueUint8             { "uint8" };
    constexpr QLatin1StringView xmlSIValueInt16             { "int16" };
    constexpr QLatin1StringView xmlSIValueUint16            { "uint16" };
    constexpr QLatin1StringView xmlSIValueInt32             { "int32" };
    constexpr QLatin1StringView xmlSIValueUint32            { "uint32" };
    constexpr QLatin1StringView xmlSIValueInt64             { "int64" };
    constexpr QLatin1StringView xmlSIValueUint64            { "uint64" };
    constexpr QLatin1StringView xmlSIValueFloat             { "float" };
    constexpr QLatin1StringView xmlSIValueDouble            { "double" };

    constexpr QLatin1StringView xmlSIValueBasicObject       { "BasicObject" };
    constexpr QLatin1StringView xmlSIValueString            { "String" };
    constexpr QLatin1StringView xmlSIValueBinary            { "BinaryBuffer" };
    constexpr QLatin1StringView xmlSIValueDateTime          { "DateTime" };

    constexpr QLatin1StringView xmlSIValueBasicContainer    { "BasicContainer" };
    constexpr QLatin1StringView xmlSIValueArray             { "Array" };
    constexpr QLatin1StringView xmlSIValueLinkedList        { "LinkedList" };
    constexpr QLatin1StringView xmlSIValueHashMap           { "HashMap" };
    constexpr QLatin1StringView xmlSIValueMap               { "Map" };
    constexpr QLatin1StringView xmlSIValuePair              { "Pair" };
    constexpr QLatin1StringView xmlSIValueNewType           { "NewType" };

    constexpr QLatin1StringView xmlSIValueStructure         { "Structure" };
    constexpr QLatin1StringView xmlSIValueEnumeration       { "Enumeration" };
    constexpr QLatin1StringView xmlSIValueImported          { "Imported" };
    constexpr QLatin1StringView xmlSIValueContainer         { "Container" };

    constexpr QLatin1StringView xmlSIMethodTypeRequest      { "Request" };
    constexpr QLatin1StringView xmlSIMethodTypeResponse     { "Response" };
    constexpr QLatin1StringView xmlSIMethodTypeBroadcast    { "Broadcast" };

    constexpr QLatin1StringView xmlSIDefaultType            { xmlSIValueBool };
    constexpr QLatin1StringView xmlSIDefaulValue            { xmlSIValueFalse };
    constexpr const unsigned int xmlElementId               { 50u };
}

#endif // LUSAN_COMMON_XMLSI_HPP
