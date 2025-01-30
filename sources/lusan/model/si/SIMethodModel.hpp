﻿#ifndef LUSAN_MODEL_SI_SIMETHODMODEL_HPP
#define LUSAN_MODEL_SI_SIMETHODMODEL_HPP
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
 *  \file        lusan/model/si/SIMethodModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Method Model.
 *
 ************************************************************************/

#include "lusan/data/si/SIMethodData.hpp"
#include <QList>

class SIMethodBase;
class SIMethodRequest;
class SIMethodResponse;
class SIMethodBroadcast;
class SIDataTypeData;

/**
 * \class SIMethodModel
 * \brief Model to manage service interface methods for the GUI.
 */
class SIMethodModel
{
public:
    /**
     * \brief Constructor with initialization.
     * \param data The method data object.
     */
    SIMethodModel(SIMethodData& data, SIDataTypeData& dataType);

    /**
     * \brief Creates a new method object with specified name and type.
     * \param name        The name of the method.
     * \param methodType  The type of the method.
     * \return  Returns the pointer to the created method object.
     **/
    SIMethodBase* createMethod(const QString& name, SIMethodBase::eMethodType methodType);

    /**
     * \brief Deletes a method object from the list by specified ID.
     * \param id  The ID of the method to delete.
     * \return  Returns true if the method object was deleted. Otherwise, returns false.
     **/
    bool removeMethod(uint32_t id);

    /**
     * \brief Deletes a method object from the list by name and type.
     * \param name        The name of the method.
     * \param methodType  The type of the method.
     * \return  Returns true if the method object was deleted. Otherwise, returns false.
     **/
    bool removeMethod(const QString& name, SIMethodBase::eMethodType methodType);

    /**
     * \brief Deletes a method object from the list.
     * \param method    The method object to delete.
     **/
    void removeMethod(SIMethodBase * method);

    /**
     * \brief Finds a method object by ID.
     * \param id  The ID of the method.
     * \return  Returns the method object if found, nullptr otherwise.
     **/
    SIMethodBase* findMethod(uint32_t id) const;

    /**
     * \brief Finds a method object by name and type.
     * \param name        The name of the method.
     * \param methodType  The type of the method.
     * \return  Returns the method object if found, nullptr otherwise.
     **/
    SIMethodBase* findMethod(const QString& name, SIMethodBase::eMethodType methodType) const;

    /**
     * \brief   Gets the list of methods.
     **/
    const QList<SIMethodBase*>& getMethodList(void) const;

    /**
     * \brief   Gets the list of broadcast methods.
     **/
    const QList<SIMethodBroadcast*>& getBroadcastMethods(void) const;

    /**
     * \brief   Gets the list of request methods.
     **/
    const QList<SIMethodRequest*>& getRequestMethods(void) const;

    /**
     * \brief   Gets the list of response methods.
     **/
    const QList<SIMethodResponse*>& getResponseMethods(void) const;

    /**
     * \brief Gets the list of parameters of the method by ID.
     * \param id  The ID of the method.
     * \return  Returns the list of parameters of the method.
     **/
    const QList<MethodParameter>& getMethodParameters(uint32_t id) const;

    /**
     * \brief Gets the list of parameters of the method by name and type.
     * \param name        The name of the method.
     * \param methodType  The type of the method.
     * \return  Returns the list of parameters of the method.
     **/
    const QList<MethodParameter>& getMethodParameters(const QString& name, SIMethodBase::eMethodType methodType) const;

    /**
     * \brief   Gets the list of request methods connected to the given response method.
     * \param   response    The response method object.
     * \return  The list of request methods connected to the response method.
     **/
    QList<SIMethodRequest*> getConnectedRequests(SIMethodResponse* response) const;

    SIMethodBase* convertMethod(SIMethodBase* method, SIMethodBase::eMethodType methodType);

    MethodParameter* addParameter(SIMethodBase* method, const QString& name, const QString& type = "bool");
    
    inline SIDataTypeData& getDataTypeData(void);
    
    inline const SIDataTypeData& getDataTypeData(void) const;

    inline SIMethodData& getMethodData(void);

    inline const SIMethodData& getMethodData(void) const;

private:
    SIMethodData&   mData;      //!< The method data object.
    SIDataTypeData& mDataType;  //!< The data type data object.
};

inline SIDataTypeData& SIMethodModel::getDataTypeData(void)
{
    return mDataType;
}

inline const SIDataTypeData& SIMethodModel::getDataTypeData(void) const
{
    return mDataType;
}

inline SIMethodData& SIMethodModel::getMethodData(void)
{
    return mData;
}

inline const SIMethodData& SIMethodModel::getMethodData(void) const
{
    return mData;
}

#endif // LUSAN_MODEL_SI_SIMETHODMODEL_HPP
