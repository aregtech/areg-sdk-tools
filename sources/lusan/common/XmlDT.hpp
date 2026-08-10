#ifndef LUSAN_COMMON_XMLDT_HPP
#define LUSAN_COMMON_XMLDT_HPP
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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/common/XmlDT.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type document (`.dtml`) XML tags.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/XmlSI.hpp"

/**
 * \brief   The element names of the Data Type (`.dtml`) document. Every section it carries is
 *          a section the other two documents also carry, so each constant references the same
 *          \ref XmlSI string value rather than repeating it: the three formats then cannot
 *          drift apart. Only the document element itself is its own.
 **/
namespace XmlDT
{
    //!< The document element. Named for the document rather than for the type, because
    //!< `DataType` is already one declared type inside `DataTypeList`.
    constexpr QLatin1StringView xmlDTElementDocument        { "DataTypeDocument" };

    constexpr QLatin1StringView xmlDTElementOverview        { XmlSI::xmlSIElementOverview };
    constexpr QLatin1StringView xmlDTElementDescription     { XmlSI::xmlSIElementDescription };
    constexpr QLatin1StringView xmlDTElementDeprecateHint   { XmlSI::xmlSIElementDeprecateHint };

    constexpr QLatin1StringView xmlDTElementDataTypeList    { XmlSI::xmlSIElementDataTypeList };
    constexpr QLatin1StringView xmlDTElementDataType        { XmlSI::xmlSIElementDataType };

    constexpr QLatin1StringView xmlDTElementIncludeList     { XmlSI::xmlSIElementIncludeList };
    constexpr QLatin1StringView xmlDTElementLocation        { XmlSI::xmlSIElementLocation };

    constexpr QLatin1StringView xmlDTAttributeFormatVersion { XmlSI::xmlSIAttributeFormatVersion };
}

#endif  // LUSAN_COMMON_XMLDT_HPP
