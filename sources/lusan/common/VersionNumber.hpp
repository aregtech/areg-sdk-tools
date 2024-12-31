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
 *  \file        lusan/common/VersionNumber.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, VersionNumber class.
 *
 ************************************************************************/

#ifndef LUSAN_COMMON_VERSIONNUMBER_HPP
#define LUSAN_COMMON_VERSIONNUMBER_HPP

#include <QString>
#include <QVersionNumber>

/**
 * \class   VersionNumber
 * \brief   Represents a version number in the format <major>.<minor>.<patch>.
 **/
class VersionNumber
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    VersionNumber(void);

    /**
     * \brief   Constructor with initialization.
     * \param   major   The major version number.
     * \param   minor   The minor version number.
     * \param   patch   The patch version number.
     **/
    VersionNumber(uint32_t major, uint32_t minor, uint32_t patch);

    /**
     * \brief   Constructor with initialization.
     * \param   versionStr  The string representation of the version.
     **/
    VersionNumber(const QString& versionStr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    VersionNumber(const VersionNumber& src);

    /**
     * \brief   Destructor.
     **/
    virtual ~VersionNumber(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    VersionNumber& operator = (const VersionNumber& other);

    /**
     * \brief   Equality operator.
     * \param   other   The other object to compare with.
     * \return  True if the versions are equal, false otherwise.
     **/
    bool operator == (const VersionNumber& other) const;

    /**
     * \brief   Inequality operator.
     * \param   other   The other object to compare with.
     * \return  True if the versions are not equal, false otherwise.
     **/
    bool operator != (const VersionNumber& other) const;

    /**
     * \brief   Less than operator.
     * \param   other   The other object to compare with.
     * \return  True if this version is less than the other, false otherwise.
     **/
    bool operator < (const VersionNumber& other) const;

    /**
     * \brief   Greater than operator.
     * \param   other   The other object to compare with.
     * \return  True if this version is greater than the other, false otherwise.
     **/
    bool operator > (const VersionNumber& other) const;

    /**
     * \brief   Less than or equal to operator.
     * \param   other   The other object to compare with.
     * \return  True if this version is less than or equal to the other, false otherwise.
     **/
    bool operator <= (const VersionNumber& other) const;

    /**
     * \brief   Greater than or equal to operator.
     * \param   other   The other object to compare with.
     * \return  True if this version is greater than or equal to the other, false otherwise.
     **/
    bool operator >= (const VersionNumber& other) const;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Converts the version to a string in the format <major>.<minor>.<patch>.
     * \return  The string representation of the version.
     **/
    QString toString(void) const;

    /**
     * \brief   Parses a string to extract the version numbers.
     * \param   versionStr  The string representation of the version.
     * \return  True if the version was successfully parsed, false otherwise.
     **/
    bool fromString(const QString& versionStr);

    /**
     * \brief   Checks if the version is valid.
     * \return  True if the version is valid, false otherwise.
     **/
    bool isValid(void) const;

    /**
     * \brief   Checks if this version is compatible with another version.
     * \param   other   The other version to check compatibility with.
     * \return  True if this version is compatible with the other, false otherwise.
     **/
    bool isCompatible(const VersionNumber& other) const;

    /**
     * \brief   Returns the major number of the version.
     **/
    inline uint32_t getMajor(void) const;

    /**
     * \brief   Sets the major number of the version.
     * \param   major   The major version number.
     **/
    inline void setMajor(uint32_t major);

    /**
     * \brief   Returns the minor number of the version.
     **/
    inline uint32_t getMinor(void) const;

    /**
     * \brief   Sets the minor number of the version.
     * \param   minor   The minor version number.
     **/
    inline void setMinor(uint32_t minor);

    /**
     * \brief   Returns the patch number of the version.
     **/
    inline uint32_t getPatch(void) const;

    /**
     * \brief   Sets the patch number of the version.
     * \param   patch   The patch version number.
     **/
    inline void setPatch(uint32_t patch);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QVersionNumber mVersion; //!< The version number.
};

//////////////////////////////////////////////////////////////////////////
// VersionNumber class inline function implementation
//////////////////////////////////////////////////////////////////////////

inline uint32_t VersionNumber::getMajor(void) const
{
    return static_cast<uint32_t>(mVersion.majorVersion());
}

inline void VersionNumber::setMajor(uint32_t major)
{
    mVersion = QVersionNumber(major, mVersion.minorVersion(), mVersion.microVersion());
}

inline uint32_t VersionNumber::getMinor(void) const
{
    return static_cast<uint32_t>(mVersion.minorVersion());
}

inline void VersionNumber::setMinor(uint32_t minor)
{
    mVersion = QVersionNumber(mVersion.majorVersion(), minor, mVersion.microVersion());
}

inline uint32_t VersionNumber::getPatch(void) const
{
    return static_cast<uint32_t>(mVersion.microVersion());
}

inline void VersionNumber::setPatch(uint32_t patch)
{
    mVersion = QVersionNumber(mVersion.majorVersion(), mVersion.minorVersion(), patch);
}

#endif // LUSAN_COMMON_VERSIONNUMBER_HPP
