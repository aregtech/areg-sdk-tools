#ifndef LUSAN_DATA_SI_SIRESPONSELINK_HPP
#define LUSAN_DATA_SI_SIRESPONSELINK_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/si/SIResponseLink.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Response link object.
 *
 ************************************************************************/
#include "lusan/data/si/SIMethodResponse.hpp"
#include "lusan/data/common/TETypeWrap.hpp"

//////////////////////////////////////////////////////////////////////////
// SIResponseLink class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The link to response method object.
 **/
class SIResponseLink  : public TETypeWrap<SIMethodResponse>
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SIResponseLink(void) = default;
    SIResponseLink(const SIResponseLink& src) = default;
    SIResponseLink(SIResponseLink&& src) noexcept = default;

    SIResponseLink(const QString& methodName);
    SIResponseLink(const QString& methodName, const QList<SIMethodResponse*>& listResponses);
    SIResponseLink(SIMethodResponse* method);

    ~SIResponseLink(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    SIResponseLink& operator = (const SIResponseLink& src) = default;
    SIResponseLink& operator = (SIResponseLink&& src) noexcept = default;

    SIResponseLink& operator = (const SIMethodResponse* method);
    SIResponseLink& operator = (SIMethodResponse* method);

    SIResponseLink& operator = (const QString& methodName);

    bool operator == (const SIResponseLink& other) const;
    bool operator != (const SIResponseLink& other) const;

    bool operator == (const SIMethodResponse* method) const;
    bool operator != (const SIMethodResponse* method) const;

    bool operator == (const QString& methodName) const;
    bool operator != (const QString& methodName) const;
};

#endif  // LUSAN_DATA_SI_SIRESPONSELINK_HPP
