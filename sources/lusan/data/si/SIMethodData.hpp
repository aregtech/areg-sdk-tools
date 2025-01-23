#ifndef LUSAN_DATA_SI_SIMETHODDATA_HPP
#define LUSAN_DATA_SI_SIMETHODDATA_HPP

/************************************************************************
 * This file is part of the Lusan project, an official component of the AREG SDK.
 * Lusan is a graphical user interface (GUI) tool designed to support the development,
 * debugging, and testing of applications built with the AREG Framework.
 *
 * Lusan is available as free and open-source software under the MIT License,
 * providing essential features for developers.
 *
 * For detailed licensing terms, please refer to the LICENSE.txt file included
 * with this distribution or contact us at info[at]aregtech.com.
 *
 * \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 * \file        lusan/data/si/SIMethodData.hpp
 * \ingroup     Lusan - GUI Tool for AREG SDK
 * \author      Artak Avetyan
 * \brief       Lusan application, Service Interface Method Data.
 *
 ************************************************************************/

#include "lusan/data/common/TEDataContainer.hpp"
#include "lusan/common/ElementBase.hpp"
#include <QObject>

#include "lusan/data/si/SIMethodBroadcast.hpp"
#include "lusan/data/si/SIMethodRequest.hpp"
#include "lusan/data/si/SIMethodResponse.hpp"

#include <QList>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

 /**
  * \class   SIMethodData
  * \brief   Represents the method data of a service interface in the Lusan application.
  **/
class SIMethodData  : public QObject
                    , public TEDataContainer<SIMethodBase*, ElementBase>
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIMethodData(ElementBase* parent = nullptr);

    /**
     * \brief   Destructor.
     **/
    virtual ~SIMethodData(void);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Adds a method object to the list.
     * \param   method      The method object to add.
     * \return  True if the method was added, false otherwise.
     **/
    bool addMethod(SIMethodBase* method);

    /**
     * \brief   Adds a method object to the list by name and type.
     * \param   name        The name of the method.
     * \param   methodType  The type of the method.
     * \return  The created method object.
     **/
    SIMethodBase* addMethod(const QString& name, SIMethodBase::eMethodType methodType);

    /**
     * \brief   Removes a method object from the list by name and type.
     * \param   name        The name of the method.
     * \param   methodType  The type of the method.
     * \return  True if the method was removed, false otherwise.
     **/
    bool removeMethod(const QString& name, SIMethodBase::eMethodType methodType);

    /**
     * \brief   Removes a method object from the list by ID.
     * \param   id          The ID of the method.
     * \return  True if the method was removed, false otherwise.
     **/
    bool removeMethod(uint32_t id);

    /**
     * \brief   Removes a method object from the list.
     * \param   method      The method object to remove.
     **/
    void removeMethod(SIMethodBase* method);

    /**
     * \brief   Searches for a method object by name and type.
     * \param   name        The name of the method.
     * \param   methodType  The type of the method.
     * \return  The method object if found, nullptr otherwise.
     **/
    SIMethodBase* findMethod(const QString& name, SIMethodBase::eMethodType methodType) const;

    /**
     * \brief   Searches for a method object by ID.
     * \param   id          The ID of the method.
     * \return  The method object if found, nullptr otherwise.
     **/
    SIMethodBase* findMethod(uint32_t id) const;

    /**
     * \brief   Searches for a request-response pair by request ID.
     * \param   reqId       The ID of the request method.
     * \return  The response method object if found, nullptr otherwise.
     **/
    SIMethodResponse* findConnectedResponse(uint32_t reqId) const;

    /**
     * \brief   Checks if a method exists in the list.
     * \param   method      The method object to check.
     * \return  True if the method exists, false otherwise.
     **/
    inline bool hasMethod(SIMethodBase* method) const;

    /**
     * \brief   Checks if a request method exists by name.
     * \param   request     The name of the request method.
     * \return  True if the request method exists, false otherwise.
     **/
    inline bool hasRequest(const QString& request) const;

    /**
     * \brief   Checks if a response method exists by name.
     * \param   response    The name of the response method.
     * \return  True if the response method exists, false otherwise.
     **/
    inline bool hasResponse(const QString& response) const;

    /**
     * \brief   Checks if a broadcast method exists by name.
     * \param   broadcast   The name of the broadcast method.
     * \return  True if the broadcast method exists, false otherwise.
     **/
    inline bool hasBroadcast(const QString& broadcast) const;

    /**
     * \brief   Checks if a response method is connected to any request method.
     * \param   response    The name of the response method to check.
     * \return  True if the response method is connected to any request, false otherwise.
     **/
    bool hasResponseConnectedRequest(const QString& response) const;

    /**
     * \brief   Checks if a response method is connected to any request method.
     * \param   response    The name of the response method to check.
     * \return  True if the response method is connected to any request, false otherwise.
     **/
    bool hasResponseConnectedRequest(uint32_t respId) const;

    /**
     * \brief   Gets the list of all method objects.
     * \return  The list of all method objects.
     **/
    inline const QList<SIMethodBase*>& getAllMethods(void) const;

    /**
     * \brief   Gets the list of request method objects.
     * \return  The list of request method objects.
     **/
    inline const QList<SIMethodRequest*>& getRequests(void) const;

    /**
     * \brief   Gets the list of response method objects.
     * \return  The list of response method objects.
     **/
    inline const QList<SIMethodResponse*>& getResponses(void) const;

    /**
     * \brief   Gets the list of broadcast method objects.
     * \return  The list of broadcast method objects.
     **/
    inline const QList<SIMethodBroadcast*>& getBroadcasts(void) const;

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
     * \brief   Removes all entries and frees resources.
     **/
    void removeAll(void);

    /**
     * \brief   Converts a method to a new type.
     * \param   method      The method object to convert.
     * \param   methodType  The new type of the method.
     * \return  The converted method object.
     **/
    SIMethodBase* convertMethod(SIMethodBase* method, SIMethodBase::eMethodType methodType);

    /**
     * \brief   Replaces a method with a new method.
     * \param   oldMethod   The method object to replace.
     * \param   newMethod   The new method object.
     * \return  True if the method was replaced, false otherwise.
     **/
    bool replaceMethod(SIMethodBase* oldMethod, SIMethodBase* newMethod);

    /**
     * \brief   Creates a method object based on the given name and type.
     * \param   methodType  The type of the method.
     * \param   name        The name of the method.
     * \return  The created method object.
     **/
    SIMethodBase* createMethod(SIMethodBase::eMethodType methodType, const QString& name);

    /**
     * \brief   Sorts methods by name.
     * \param   ascending   If true, sorts in ascending order, otherwise in descending order.
     **/
    void sortByName(bool ascending);

    /**
     * \brief   Sorts methods by ID.
     * \param   ascending   If true, sorts in ascending order, otherwise in descending order.
     **/
    void sortById(bool ascending);

    /**
     * \brief   Gets the list of request methods connected to the given response method.
     * \param   response    The response method object.
     * \return  The list of request methods connected to the response method.
     **/
    QList<SIMethodRequest*> getConnectedRequests(SIMethodResponse* response) const;

    /**
     * \brief   Adds new parameter to the selected method. Returns new parameter object if operation succeeded.
     *          The parameters should have unique names in the parameter list of the method.
     * \param   method  The valid pointer to the method object to add new parameter.
     * \param   name    The name of the parameter.
     * \param   type    The type of the parameter.
     * \return  Returns new parameter object if operation succeeded. Otherwise, returns nullptr.
     *          The parameters should have unique names in the parameter list of the method.
     **/
    MethodParameter* addParameter(SIMethodBase* method, const QString& name, const QString& type = "bool");

private:
    /**
     * \brief   Adds a method object to the appropriate list.
     * \param   method      The method object to add.
     **/
    void addMethodToList(SIMethodBase* method);

    /**
     * \brief   Removes a method object from the appropriate list.
     * \param   method      The method object to remove.
     **/
    void removeMethodFromList(SIMethodBase* method);

    /**
     * \brief   Replaces a method object in the list.
     * \param   oldMethod   The old method object.
     * \param   newMethod   The new method object.
     **/
    void replaceMethodInList(SIMethodBase* oldMethod, SIMethodBase* newMethod);

    /**
     * \brief   Creates a method object based on the given name and type.
     * \param   methodType  The type of the method.
     * \param   name        The name of the method.
     * \param   id          The ID of the method.
     * \return  The created method object.
     **/
    SIMethodBase* createMethod(SIMethodBase::eMethodType methodType, const QString& name, uint32_t id);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QList<SIMethodRequest*>    mRequestMethods;    //!< List of request methods.
    QList<SIMethodResponse*>   mResponseMethods;   //!< List of response methods.
    QList<SIMethodBroadcast*>  mBroadcastMethods;  //!< List of broadcast methods.
};

//////////////////////////////////////////////////////////////////////////
// SIMethodData class inline methods.
//////////////////////////////////////////////////////////////////////////

inline bool SIMethodData::hasMethod(SIMethodBase* method) const
{
    return mElementList.contains(method);
}

inline bool SIMethodData::hasRequest(const QString& request) const
{
    return (findMethod(request, SIMethodBase::eMethodType::MethodRequest) != nullptr);
}

inline bool SIMethodData::hasResponse(const QString& response) const
{
    return (findMethod(response, SIMethodBase::eMethodType::MethodResponse) != nullptr);
}

inline bool SIMethodData::hasBroadcast(const QString& broadcast) const
{
    return (findMethod(broadcast, SIMethodBase::eMethodType::MethodBroadcast) != nullptr);
}

inline const QList<SIMethodBase*>& SIMethodData::getAllMethods(void) const
{
    return getElements();
}

inline const QList<SIMethodRequest*>& SIMethodData::getRequests(void) const
{
    return mRequestMethods;
}

inline const QList<SIMethodResponse*>& SIMethodData::getResponses(void) const
{
    return mResponseMethods;
}

inline const QList<SIMethodBroadcast*>& SIMethodData::getBroadcasts(void) const
{
    return mBroadcastMethods;
}

#endif // LUSAN_DATA_SI_SIMETHODDATA_HPP
