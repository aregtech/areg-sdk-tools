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
 *  \file        lusan/data/si/SIResponseLink.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Response link object.
 *
 ************************************************************************/

#include "lusan/data/si/SIResponseLink.hpp"

SIResponseLink::SIResponseLink(const QString& methodName)
    : TETypeWrap<SIMethodResponse>(methodName)
{
}

SIResponseLink::SIResponseLink(const QString& methodName, const QList<SIMethodResponse*>& listResponses)
    : TETypeWrap<SIMethodResponse>(methodName, listResponses)
{
}

SIResponseLink::SIResponseLink(SIMethodResponse* method)
    : TETypeWrap<SIMethodResponse>(method)
{
}

SIResponseLink& SIResponseLink::operator = (const SIMethodResponse* method)
{
    setType(method);
    return (*this);
}

SIResponseLink& SIResponseLink::operator = (SIMethodResponse* method)
{
    setType(method);
    return (*this);
}

SIResponseLink& SIResponseLink::operator = (const QString& methodName)
{
    setName(methodName);
    return (*this);
}

bool SIResponseLink::operator == (const SIResponseLink& other) const
{
    return TETypeWrap<SIMethodResponse>::operator == (static_cast<const TETypeWrap<SIMethodResponse> &>(other));
}

bool SIResponseLink::operator != (const SIResponseLink& other) const
{
    return TETypeWrap<SIMethodResponse>::operator != (static_cast<const TETypeWrap<SIMethodResponse> &>(other));
}

bool SIResponseLink::operator == (const SIMethodResponse* method) const
{
    return TETypeWrap<SIMethodResponse>::operator == (method);
}

bool SIResponseLink::operator != (const SIMethodResponse* method) const
{
    return TETypeWrap<SIMethodResponse>::operator != (method);
}

bool SIResponseLink::operator == (const QString& methodName) const
{
    return TETypeWrap<SIMethodResponse>::operator == (methodName);
}

bool SIResponseLink::operator != (const QString& methodName) const
{
    return TETypeWrap<SIMethodResponse>::operator != (methodName);
}
