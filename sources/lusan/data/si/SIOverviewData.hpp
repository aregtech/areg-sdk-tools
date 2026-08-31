#ifndef LUSAN_DATA_SI_SIOVERVIEWDATA_HPP
#define LUSAN_DATA_SI_SIOVERVIEWDATA_HPP

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
 *  \file        lusan/data/si/SIOverviewData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview Data.
 *
 ************************************************************************/

#include "lusan/data/common/OverviewDataSection.hpp"

class DataTypeDataSection;

/**
 * \class   SIOverviewData
 * \brief   The `Overview` element of a `.siml` document: the shared name, version, description
 *          and deprecation mark, plus the service category, which only an interface declares.
 **/
class SIOverviewData    : public OverviewDataSection
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
     * \param   parent          The parent element.
     **/
    SIOverviewData(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the service interface.
     * \param   name            The name of the service interface.
     * \param   parent          The parent element.
     **/
    SIOverviewData(uint32_t id, const QString& name, ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the service interface.
     * \param   name            The name of the service interface.
     * \param   version         The version of the service interface.
     * \param   category        The category of the service interface.
     * \param   description     The description of the service interface.
     * \param   isDeprecated    Flag indicating whether the interface is deprecated.
     * \param   deprecateHint   The deprecation hint.
     * \param   parent          The parent element.
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
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Reads the service category, accepting the spelling the first published format
     *          used.
     **/
    virtual void readOwnAttributes(const QXmlStreamAttributes& attributes) override;

    /**
     * \brief   Writes the service category.
     **/
    virtual void writeOwnAttributes(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Gets the category of the service interface.
     * \return  The category of the service interface.
     **/
    inline eCategory getCategory(void) const;

    /**
     * \brief   Sets the category of the service interface.
     * \param   category  The category to set.
     **/
    inline void setCategory(eCategory category);

    /**
     * \brief   Validates the service interface data.
     * \param   dataTypes   The data type data to validate the service interface.
     **/
    void validate(const DataTypeDataSection& dataTypes);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    eCategory       mCategory;      //!< The category of the service interface.
};

//////////////////////////////////////////////////////////////////////////
// SIOverviewData inline methods
//////////////////////////////////////////////////////////////////////////

inline SIOverviewData::eCategory SIOverviewData::getCategory(void) const
{
    return mCategory;
}

inline void SIOverviewData::setCategory(eCategory category)
{
    mCategory = category;
}

#endif // LUSAN_DATA_SI_SIOVERVIEWDATA_HPP
