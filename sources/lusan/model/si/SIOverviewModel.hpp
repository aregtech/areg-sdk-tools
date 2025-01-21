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
 *  \file        lusan/model/si/SIOverviewModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview Model.
 *
 ************************************************************************/
#ifndef LUSAN_MODEL_SI_SIOVERVIEWMODEL_HPP
#define LUSAN_MODEL_SI_SIOVERVIEWMODEL_HPP

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/si/SIOverviewData.hpp"

class SIOverviewModel
{
public:
    SIOverviewModel(SIOverviewData& data);

    /**
     * \brief   Gets the ID of the service interface.
     * \return  The ID of the service interface.
     **/
    uint32_t getId() const;

    /**
     * \brief   Sets the ID of the service interface.
     * \param   id  The ID to set.
     **/
    void setId(uint32_t id);

    /**
     * \brief   Gets the name of the service interface.
     * \return  The name of the service interface.
     **/
    const QString& getName() const;

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
    void setVersion(uint32_t major, uint32_t minor, uint32_t patch);

    /**
     * \brief   Gets the category of the service interface.
     * \return  The category of the service interface.
     **/
    SIOverviewData::eCategory getCategory() const;

    /**
     * \brief   Sets the category of the service interface.
     * \param   category  The category to set.
     **/
    void setCategory(SIOverviewData::eCategory category);

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
    SIOverviewData& mData;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    SIOverviewModel(void) = delete;
    SIOverviewModel(const SIOverviewModel& /*src*/) = delete;
    SIOverviewModel& operator = (const SIOverviewModel& /*src*/) = delete;
    SIOverviewModel(SIOverviewModel&& /*src*/) = delete;
    SIOverviewModel& operator = (SIOverviewModel&& /*src*/) = delete;
};

#endif  // LUSAN_MODEL_SI_SIOVERVIEWMODEL_HPP
