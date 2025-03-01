#ifndef LUSAN_MODEL_SI_SIMETHODMODEL_HPP
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
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief Constructor with initialization.
     * \param data The method data object.
     */
    SIMethodModel(SIMethodData& data, SIDataTypeData& dataType);

    ~SIMethodModel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and Operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief Returns instance of data type data object.
     **/
    inline SIDataTypeData& getDataTypeData(void);
    inline const SIDataTypeData& getDataTypeData(void) const;

    /**
     * \brief Returns instance of method data object.
     **/
    inline SIMethodData& getMethodData(void);
    inline const SIMethodData& getMethodData(void) const;

    /**
     * \brief Creates a new method object with specified name and type.
     * \param name        The name of the method.
     * \param methodType  The type of the method.
     * \return  Returns the pointer to the created method object.
     **/
    SIMethodBase* addMethod(const QString& name, SIMethodBase::eMethodType methodType);

    /**
     * \brief Inserts a new method object with specified name and type at the specified position.
     * \param position    The position to insert the method.
     * \param name        The name of the method.
     * \param methodType  The type of the method.
     * \return  Returns the pointer to the created method object.
     **/
    SIMethodBase* insertMethod(int position, const QString& name, SIMethodBase::eMethodType methodType);

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

    /**
     * \brief   Converts the method object to new type.
     * \param   method      The method object to convert.
     * \param   methodType  The new type of the method.
     * \return  Returns the pointer to the converted method object.
     **/
    SIMethodBase* convertMethod(SIMethodBase* method, SIMethodBase::eMethodType methodType);

    /**
     * \brief   Adds a new parameter to the method.
     * \param   method  The method object to add a parameter.
     * \param   name    The name of the parameter.
     * \return  Returns the pointer to the created parameter object.
     **/
    MethodParameter* addParameter(SIMethodBase* method, const QString& name);

    /**
     * \brief   Inserts a new parameter to the method at the specified position.
     * \param   method      The method object to add a parameter.
     * \param   position    The position to insert the parameter.
     * \param   name        The name of the parameter.
     * \return  Returns the pointer to the created parameter object.
     **/
    MethodParameter* insertParameter(SIMethodBase* method, int position, const QString& name);

    /**
     * \brief   Removes the parameter from the method by ID.
     * \param   method  The method object to remove the parameter.
     * \param   id      The ID of the parameter to remove.
     **/
    void removeMethodParameter(SIMethodBase& method, uint32_t id);

    /**
     * \brief   Swaps 2 methods in the method list without changing the order of IDs.
     * \param   first   The first method to swap.
     * \param   second  The second method to swap.
     **/
    void swapMethods(const SIMethodBase& first, const SIMethodBase& second);

    /**
     * \brief   Swaps 2 methods in the method list without changing the order of IDs.
     * \param   firstId     The ID of the first method to swap.
     * \param   secondId    The ID of the second method to swap.
     **/
    void swapMethods(uint32_t firstId, uint32_t secondId);

    /**
     * \brief   Swaps 2 parameters in the method without changing the order of IDs.
     * \param   method      The method object to swap the parameters.
     * \param   firstId     The ID of the first parameter to swap.
     * \param   secondId    The ID of the second parameter to swap.
     **/
    void swapMethodParams(SIMethodBase& method, uint32_t firstId, uint32_t secondId);

    /**
     * \brief   Swaps 2 parameters in the method without changing the order of IDs.
     * \param   method  The method object to swap the parameters.
     * \param   first   The first parameter to swap.
     * \param   second  The second parameter to swap.
     **/
    void swapMethodParams(SIMethodBase& method, const MethodParameter& first, const MethodParameter& second);



//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SIMethodData&   mData;      //!< The method data object.
    SIDataTypeData& mDataType;  //!< The data type data object.
};

//////////////////////////////////////////////////////////////////////////
// SIMethodModel class inline methods
//////////////////////////////////////////////////////////////////////////

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
