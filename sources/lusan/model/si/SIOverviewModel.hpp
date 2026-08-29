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
 *  \file        lusan/model/si/SIOverviewModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview Model.
 *
 ************************************************************************/
#ifndef LUSAN_MODEL_SI_SIOVERVIEWMODEL_HPP
#define LUSAN_MODEL_SI_SIOVERVIEWMODEL_HPP

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/OverviewModel.hpp"

#include "lusan/data/si/SIOverviewData.hpp"

/**
 * \class   SIOverviewModel
 * \brief   The shared Overview page model plus the service category, which only an interface
 *          declares. The category is edited through an undo command like every other row.
 **/
class SIOverviewModel : public OverviewModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SIOverviewModel(IEDocumentModel& document);

    virtual ~SIOverviewModel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the category of the service interface.
     **/
    SIOverviewData::eCategory getCategory(void) const;

    /**
     * \brief   Sets the category of the service interface.
     **/
    void setCategory(SIOverviewData::eCategory category);
};

#endif  // LUSAN_MODEL_SI_SIOVERVIEWMODEL_HPP
