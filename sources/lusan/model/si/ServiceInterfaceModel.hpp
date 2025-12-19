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
#include "lusan/model/si/SIAttributeModel.hpp"
#include "lusan/model/si/SIConstantModel.hpp"
#include "lusan/model/si/SIDataTypeModel.hpp"
#include "lusan/model/si/SIIncludeModel.hpp"
#include "lusan/model/si/SIMethodModel.hpp"
#include "lusan/model/si/SIOverviewModel.hpp"

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
    ServiceInterfaceModel(const QString& filePath = QString());

    virtual ~ServiceInterfaceModel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the overview model.
     **/
    inline SIOverviewModel& getOverviewModel(void);

    /**
     * \brief   Returns the data attributes model.
     **/
    inline SIAttributeModel& getAttributeModel(void);

    /**
     * \brief   Returns the constant model.
     **/
    inline SIConstantModel& getConstantsModel(void);
    
    /**
     * \brief   Returns the include model.
     **/
    inline SIIncludeModel & getIncludesModel(void);

    /**
     * \brief   Returns the data type model.
     **/
    inline SIDataTypeModel& getDataTypeModel(void);

    /**
     * \brief   Returns the methods model.
     **/
    inline SIMethodModel& getMethodsModel(void);

    /**
     * \brief   Saves the service interface data to the file.
     * \param   filePath    The file path to save the data.
     * \return  Returns true if the data was successfully saved, false otherwise.
     **/
    inline bool saveToFile(const QString& filePath);
    
    /**
     * \brief   Returns the file format version.
     * \return  The file format version.
     **/
    inline QString getFileFormatVersion(void) const;

    /**
     * \brief   Gets the name of the service interface.
     * \return  The name of the service interface.
     **/
    inline const QString& getName() const;

    /**
     * \brief   Gets the version of the service interface.
     * \return  The version of the service interface.
     **/
    inline const VersionNumber& getVersion() const;

    /**
     * \brief   Gets the category of the service interface.
     * \return  The category of the service interface.
     **/
    inline SIOverviewData::eCategory getCategory() const;
    
    /**
     * \brief   Returns the file open operation success flag.
     **/
    inline bool openSucceeded(void) const;

    inline ServiceInterfaceData& getData(void);

    inline const ServiceInterfaceData& getData(void) const;
    
//////////////////////////////////////////////////////////////////////////
// Hidden class members
//////////////////////////////////////////////////////////////////////////
private:
    ServiceInterfaceData    mSIData;            //!< The service interface data.
    SIOverviewModel         mModelOverview;     //!< The overview model.
    SIDataTypeModel         mModelDataType;     //!< The data type model.
    SIAttributeModel        mModelAttributes;   //!< The data attributes model.
    SIMethodModel           mModelMethods;      //!< The methods model.
    SIConstantModel         mModelConstant;     //!< The constant model.
    SIIncludeModel          mModelInclude;      //!< The include model.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    ServiceInterfaceModel(const ServiceInterfaceModel& /*src*/) = delete;
    ServiceInterfaceModel(ServiceInterfaceModel&& /*src*/) noexcept = delete;
    ServiceInterfaceModel& operator = (const ServiceInterfaceModel& /*src*/) = delete;
    ServiceInterfaceModel& operator = (ServiceInterfaceModel&& /*src*/) noexcept = delete;
};

//////////////////////////////////////////////////////////////////////////
// ServiceInterfaceModel class inline functions
//////////////////////////////////////////////////////////////////////////

inline SIOverviewModel& ServiceInterfaceModel::getOverviewModel(void)
{
    return mModelOverview;
}

inline SIDataTypeModel& ServiceInterfaceModel::getDataTypeModel(void)
{
    return mModelDataType;
}

inline SIAttributeModel& ServiceInterfaceModel::getAttributeModel(void)
{
    return mModelAttributes;
}

inline SIMethodModel& ServiceInterfaceModel::getMethodsModel(void)
{
    return mModelMethods;
}

inline SIConstantModel& ServiceInterfaceModel::getConstantsModel(void)
{
    return mModelConstant;
}

inline SIIncludeModel & ServiceInterfaceModel::getIncludesModel(void)
{
    return mModelInclude;
}

inline bool ServiceInterfaceModel::saveToFile(const QString& filePath)
{
    return mSIData.writeToFile(filePath);
}

inline QString ServiceInterfaceModel::getFileFormatVersion(void) const
{
    return mSIData.getFileFormatVersion();
}

inline const QString& ServiceInterfaceModel::getName() const
{
    return mModelOverview.getName();
}

inline const VersionNumber& ServiceInterfaceModel::getVersion() const
{
    return mModelOverview.getVersion();
}

inline SIOverviewData::eCategory ServiceInterfaceModel::getCategory() const
{
    return mModelOverview.getCategory();
}

inline bool ServiceInterfaceModel::openSucceeded(void) const
{
    return mSIData.openSucceeded();
}

inline ServiceInterfaceData& ServiceInterfaceModel::getData(void)
{
    return mSIData;
}

inline const ServiceInterfaceData& ServiceInterfaceModel::getData(void) const
{
    return mSIData;
}


#endif  // LUSAN_MODEL_SI_SERVICEINTERFACEMODEL_HPP
