#ifndef LUSAN_DATA_SI_SIOVERVIEWDATA_HPP
#define LUSAN_DATA_SI_SIOVERVIEWDATA_HPP

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
 *  \file        lusan/data/si/SIOverviewData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview Data.
 *
 ************************************************************************/

#include "lusan/common/ElementBase.hpp"
#include "lusan/common/VersionNumber.hpp"

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/**
 * \class   SIOverviewData
 * \brief   Represents the overview data of a service interface in the Lusan application.
 **/
class SIOverviewData    : public ElementBase
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants.
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eCategory
     * \brief   Represents the category of the service interface.
     **/
    enum class eCategory
    {
          InterfaceUnknown  //!< Unknown interface category
        , InterfacePrivate  //!< Private interface category
        , InterfacePublic   //!< Public interface category
        , InterfaceInternet //!< Internet interface category
    };

    /**
     * \brief   Converts string to eCategory value.
     * \param   category    The string to convert.
     * \return  Returns eCategory value.
     **/
    static SIOverviewData::eCategory fromString(const QString& category);

    /**
     * \brief   Converts eCategory value to string.
     * \param   category    The value to convert.
     * \return  Returns string value.
     **/
    static const char* toString(SIOverviewData::eCategory category);

    static constexpr char const* STR_CATEGORY_UNKNOWN   { "Unknown" };  //!< The string value of eCategory::InterfaceUnknown
    static constexpr char const* STR_CATEGORY_PRIVATE   { "Private" };  //!< The string value of eCategory::InterfacePrivate
    static constexpr char const* STR_CATEGORY_PUBLIC    { "Public" };   //!< The string value of eCategory::InterfacePublic
    static constexpr char const* STR_CATEGORY_INTERNET  { "Internet" }; //!< The string value of eCategory::InterfaceInternet

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIOverviewData(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the service interface.
     * \param   name            The name of the service interface.
     * \param   version         The version of the service interface.
     * \param   category        The category of the service interface.
     * \param   description     The description of the service interface.
     * \param   isDeprecated    Flag indicating whether the interface is deprecated.
     * \param   deprecateHint   The deprecation hint.
     **/
    SIOverviewData( uint32_t id
                  , const QString& name
                  , const QString& version
                  , eCategory category
                  , const QString& description
                  , bool isDeprecated
                  , const QString& deprecateHint
                  , ElementBase* parent = nullptr);

    /**
     * \brief   Destructor.
     **/
    virtual ~SIOverviewData(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Reads data from an XML stream.
     * \param   xml     The XML stream reader.
     * \return  True if the data was successfully read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes data to an XML stream.
     * \param   xml     The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

    /**
     * \brief   Gets the name of the service interface.
     * \return  The name of the service interface.
     **/
    const QString & getName() const;

    /**
     * \brief   Sets the name of the service interface.
     * \param   name  The name to set.
     **/
    void setName(const QString& name);

    /**
     * \brief   Gets the version of the service interface.
     * \return  The version of the service interface.
     **/
    const VersionNumber& getVersion() const;

    /**
     * \brief   Sets the version of the service interface.
     * \param   version  The version to set.
     **/
    void setVersion(const QString& version);
    void setVersion(const VersionNumber& version);
    void setVersion(int major, int minor, int patch);

    /**
     * \brief   Gets the category of the service interface.
     * \return  The category of the service interface.
     **/
    eCategory getCategory() const;

    /**
     * \brief   Sets the category of the service interface.
     * \param   category  The category to set.
     **/
    void setCategory(eCategory category);

    /**
     * \brief   Gets the description of the service interface.
     * \return  The description of the service interface.
     **/
    const QString& getDescription() const;

    /**
     * \brief   Sets the description of the service interface.
     * \param   description  The description to set.
     **/
    void setDescription(const QString& description);

    /**
     * \brief   Checks if the service interface is deprecated.
     * \return  True if the service interface is deprecated, false otherwise.
     **/
    bool isDeprecated() const;

    /**
     * \brief   Sets the deprecation status of the service interface.
     * \param   isDeprecated  The deprecation status to set.
     **/
    void setIsDeprecated(bool isDeprecated);

    /**
     * \brief   Gets the deprecation hint of the service interface.
     * \return  The deprecation hint of the service interface.
     **/
    const QString& getDeprecateHint() const;

    /**
     * \brief   Sets the deprecation hint of the service interface.
     * \param   deprecateHint  The deprecation hint to set.
     **/
    void setDeprecateHint(const QString& deprecateHint);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString         mName;          //!< The name of the service interface.
    VersionNumber   mVersion;       //!< The version of the service interface.
    eCategory       mCategory;      //!< The category of the service interface.
    QString         mDescription;   //!< The description of the service interface.
    bool            mIsDeprecated;  //!< Flag indicating whether the interface is deprecated.
    QString         mDeprecateHint; //!< The deprecation hint.
};

//////////////////////////////////////////////////////////////////////////
// SIOverviewData inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& SIOverviewData::getName() const
{
    return mName;
}

inline void SIOverviewData::setName(const QString& name)
{
    mName = name;
}

inline const VersionNumber& SIOverviewData::getVersion() const
{
    return mVersion;
}

inline void SIOverviewData::setVersion(const QString& version)
{
    mVersion.fromString(version);
}

inline void SIOverviewData::setVersion(const VersionNumber& version)
{
    mVersion = version;
}

inline void SIOverviewData::setVersion(int major, int minor, int patch)
{
    mVersion = VersionNumber(major, minor, patch);
}

inline SIOverviewData::eCategory SIOverviewData::getCategory() const
{
    return mCategory;
}

inline void SIOverviewData::setCategory(eCategory category)
{
    mCategory = category;
}

inline const QString& SIOverviewData::getDescription() const
{
    return mDescription;
}

inline void SIOverviewData::setDescription(const QString& description)
{
    mDescription = description;
}

inline bool SIOverviewData::isDeprecated() const
{
    return mIsDeprecated;
}

inline void SIOverviewData::setIsDeprecated(bool isDeprecated)
{
    mIsDeprecated = isDeprecated;
}

inline const QString& SIOverviewData::getDeprecateHint() const
{
    return mDeprecateHint;
}

inline void SIOverviewData::setDeprecateHint(const QString& deprecateHint)
{
    mDeprecateHint = deprecateHint;
}

#endif // LUSAN_DATA_SI_SIOVERVIEWDATA_HPP
