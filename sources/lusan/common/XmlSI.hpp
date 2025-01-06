#ifndef LUSAN_COMMON_XMLSI_HPP
#define LUSAN_COMMON_XMLSI_HPP
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
 *  \file        lusan/common/XmlSI.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface XML tags.
 *
 ************************************************************************/

/**
 * \brief   The list of tags and constants used in Service Interface XML file.
 **/
namespace XmlSI
{
    /**
     * \brief   Common XML element names of the Service Interface data.
     **/
    constexpr const char* const xmlSIElementAttribute       { "Attribute" };
    constexpr const char* const xmlSIElementAttributeList   { "AttributeList" };
    constexpr const char* const xmlSIElementBaseTypeKey     { "BaseTypeKey" };
    constexpr const char* const xmlSIElementBaseTypeValue   { "BaseTypeValue" };
    constexpr const char* const xmlSIElementConstant        { "Constant" };
    constexpr const char* const xmlSIElementConstantList    { "ConstantList" };
    constexpr const char* const xmlSIElementContainer       { "Container" };
    constexpr const char* const xmlSIElementDataType        { "DataType" };
    constexpr const char* const xmlSIElementDataTypeList    { "DataTypeList" };
    constexpr const char* const xmlSIElementDeprecateHint   { "DeprecateHint" };
    constexpr const char* const xmlSIElementDescription     { "Description" };
    constexpr const char* const xmlSIElementEnumEntry       { "EnumEntry" };
    constexpr const char* const xmlSIElementField           { "Field" };
    constexpr const char* const xmlSIElementFieldList       { "FieldList" };
    constexpr const char* const xmlSIElementIncludeList     { "IncludeList" };
    constexpr const char* const xmlSIElementLocation        { "Location" };
    constexpr const char* const xmlSIElementMethod          { "Method" };
    constexpr const char* const xmlSIElementMethodList      { "MethodList" };
    constexpr const char* const xmlSIElementNamespace       { "Namespace" };
    constexpr const char* const xmlSIElementOverview        { "Overview" };
    constexpr const char* const xmlSIElementParameter       { "Parameter" };
    constexpr const char* const xmlSIElementParamList       { "ParamList" };
    constexpr const char* const xmlSIElementServiceInterface{ "ServiceInterface" };
    constexpr const char* const xmlSIElementValue           { "Value" };

    /**
     * \brief   Common XML element attributes of the Service Interface data.
     **/
    constexpr const char* const xmlSIAttributeCategory      { "Category" };
    constexpr const char* const xmlSIAttributeDataType      { "DataType" };
    constexpr const char* const xmlSIAttributeHasKey        { "HasKey" };
    constexpr const char* const xmlSIAttributeHasValue      { "HasValue" };
    constexpr const char* const xmlSIAttributeID            { "ID" };
    constexpr const char* const xmlSIAttributeIsDefault     { "IsDefault" };
    constexpr const char* const xmlSIAttributeIsDeprecated  { "IsDeprecated" };
    constexpr const char* const xmlSIAttributeFormatVersion { "FormatVersion" };
    constexpr const char* const xmlSIAttributeMethodType    { "MethodType" };
    constexpr const char* const xmlSIAttributeName          { "Name" };
    constexpr const char* const xmlSIAttributeNotify        { "Notify" };
    constexpr const char* const xmlSIAttributeType          { "Type" };
    constexpr const char* const xmlSIAttributeResponse      { "Response" };
    constexpr const char* const xmlSIAttributeValues        { "Values" };
    constexpr const char* const xmlSIAttributeVersion       { "Version" };
    

    constexpr const char* const xmlSIValuePrimitive         { "Primitive" };
    constexpr const char* const xmlSIValueBasic             { "Basic" };
    constexpr const char* const xmlSIValueContainer         { "Container" };
    constexpr const char* const xmlSIValueTrue              { "true" };
    constexpr const char* const xmlSIValueFalse             { "false" };

    constexpr const unsigned int xmlElementId               { 50u };
}

#endif // LUSAN_COMMON_XMLSI_HPP
