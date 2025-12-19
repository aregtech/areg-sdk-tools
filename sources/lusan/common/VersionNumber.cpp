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
 *  \file        lusan/common/VersionNumber.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, VersionNumber class.
 *
 ************************************************************************/

#include "lusan/common/VersionNumber.hpp"

#include <QAnyStringView>

VersionNumber::VersionNumber(void)
    : mVersion(0, 0, 1)
{
}

VersionNumber::VersionNumber(uint32_t major, uint32_t minor, uint32_t patch)
    : mVersion(major, minor, patch)
{
}

VersionNumber::VersionNumber(const QString& versionStr)
    : mVersion(0, 0, 1)
{
    fromString(versionStr);
}

VersionNumber::VersionNumber(const VersionNumber& src)
    : mVersion(src.mVersion)
{
}

VersionNumber& VersionNumber::operator=(const VersionNumber& other)
{
    mVersion = other.mVersion;
    return *this;
}

bool VersionNumber::operator==(const VersionNumber& other) const
{
    return mVersion == other.mVersion;
}

bool VersionNumber::operator!=(const VersionNumber& other) const
{
    return (mVersion != other.mVersion);
}

bool VersionNumber::operator < (const VersionNumber& other) const
{
    return mVersion < other.mVersion;
}

bool VersionNumber::operator > (const VersionNumber& other) const
{
    return mVersion > other.mVersion;
}

bool VersionNumber::operator <= (const VersionNumber& other) const
{
    return mVersion <= other.mVersion;
}

bool VersionNumber::operator >= (const VersionNumber& other) const
{
    return mVersion >= other.mVersion;
}

QString VersionNumber::toString(void) const
{
    return mVersion.toString();
}

bool VersionNumber::fromString(const QString& versionStr)
{
    QAnyStringView str(versionStr);
    qsizetype suffixIndex{};
    mVersion = QVersionNumber::fromString(str, &suffixIndex);
    return isValid();
}

bool VersionNumber::isValid(void) const
{
    return (mVersion.majorVersion() != 0) || (mVersion.minorVersion() != 0) || (mVersion.microVersion() != 0);
}

bool VersionNumber::isCompatible(const VersionNumber& other) const
{
    return QVersionNumber::compare(mVersion, other.mVersion) <= 0;
}
