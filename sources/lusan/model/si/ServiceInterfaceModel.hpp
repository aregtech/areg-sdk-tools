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
 *  \copyright   � 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/model/si/ServiceInterfaceModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Model.
 *
 ************************************************************************/
#ifndef LUSAN_MODEL_SI_SERVICEINTERFACEMODEL_HPP
#define LUSAN_MODEL_SI_SERVICEINTERFACEMODEL_HPP

 /************************************************************************
  * Includes
  ************************************************************************/
#include "lusan/data/si/ServiceInterfaceData.hpp"
#include "lusan/model/si/SIOverviewModel.hpp"
#include "lusan/model/si/SIIncludeModel.hpp"

/**
 * \class   ServiceInterfaceModel
 * \brief   The model of the service interface.
 **/
class ServiceInterfaceModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    ServiceInterfaceModel(void);

    virtual ~ServiceInterfaceModel(void) = default;


public:
    /**
     * \brief   Returns the overview model.
     **/
    inline SIOverviewModel& getOverviewModel(void);

    /**
     * \brief   Returns the include model.
     **/
    inline SIIncludeModel & getIncludesModel(void);

private:
    ServiceInterfaceData    mSIData;        //!< The service interface data.
    SIOverviewModel         mModelOverview; //!< The overview model.
    SIIncludeModel          mModelInclude;  //!< The include model.
};

//////////////////////////////////////////////////////////////////////////
// ServiceInterfaceModel class inline functions
//////////////////////////////////////////////////////////////////////////
inline SIOverviewModel& ServiceInterfaceModel::getOverviewModel(void)
{
    return mModelOverview;
}

inline SIIncludeModel & ServiceInterfaceModel::getIncludesModel(void)
{
    return mModelInclude;
}

#endif  // LUSAN_MODEL_SI_SERVICEINTERFACEMODEL_HPP
