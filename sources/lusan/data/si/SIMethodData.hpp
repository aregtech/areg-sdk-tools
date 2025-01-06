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

#include "lusan/common/ElementBase.hpp"

#include "lusan/data/si/SIMethodRequest.hpp"
#include "lusan/data/si/SIMethodResponse.hpp"
#include "lusan/data/si/SIMethodBroadcast.hpp"
#include "lusan/common/XmlSI.hpp"

#include <QList>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/**
 * \class   SIMethodData
 * \brief   Represents the method data of a service interface in the Lusan application.
 **/
class SIMethodData  : public ElementBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIMethodData(ElementBase * parent = nullptr);

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
     * \param   isUnique    If true, the method is added only if it is unique.
     * \return  True if the method was added, false otherwise.
     **/
    bool addMethod(SIMethodBase* method, bool isUnique = true);

    /**
     * \brief   Removes a method object from the list by name and type.
     * \param   name        The name of the method.
     * \param   methodType  The type of the method.
     * \return  True if the method was removed, false otherwise.
     **/
    bool removeMethod(const QString& name, SIMethodBase::eMethodType methodType);

    /**
     * \brief   Inserts a method object into the list at the specified position.
     * \param   index   The position to insert the method.
     * \param   method  The method object to insert.
     **/
    void insertMethod(int index, SIMethodBase* method);

    /**
     * \brief   Searches for a method object by name and type.
     * \param   name        The name of the method.
     * \param   methodType  The type of the method.
     * \return  The method object if found, nullptr otherwise.
     **/
    SIMethodBase* findMethod(const QString& name, SIMethodBase::eMethodType methodType) const;

    /**
     * \brief   Searches for a method object by name.
     * \param   name    The name of the method.
     * \return  The method object if found, nullptr otherwise.
     **/
    inline bool hasMethod(const SIMethodBase& method) const;

    /**
     * \brief   Searches for a request method by name.
     * \param   request The name of the request method.
     * \return  True if the request method exists, false otherwise.
     **/
    inline bool hasRequest(const QString& request) const;

    /**
     * \brief   Searches for a response method by name.
     * \param   response    The name of the response method.
     * \return  True if the response method exists, false otherwise.
     **/
    inline bool hasResponse(const QString& response) const;

    /**
     * \brief   Searches for a broadcast method by name.
     * \param   broadcast   The name of the broadcast method.
     * \return  True if the broadcast method exists, false otherwise.
     **/
    inline bool hasBroadcast(const QString& broadcast) const;

    /**
     * \brief   Gets the response method object by name, which is connected to the specified request.
     * \param   request The name of the request method to check connected response.
     * \return  The connected response method object if found, empty string otherwise.
     **/
    QString getRequestConnectedResponse(const QString& request) const;

    /**
     * \brief   Checks whether the response method is connected to any request method.
     * \param   response    The name of the response method to check connected request.
     * \return  True if the response method is connected to any request, false otherwise.
     **/
    bool hasResponseConnectedRequest(const QString& response) const;

    /**
     * \brief   Gets the list of method objects.
     * \return  The list of method objects.
     **/
    QList<SIMethodBase*> getAllMethods(void) const;

    /**
     * \brief   Gets the list of request method objects.
     * \return  The list of request method objects.
     **/
    inline const QList<SIMethodRequest*> & getRequests(void) const;

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
     * \brief   remove all entries and frees resources.
     **/
    void removeAll(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QList<SIMethodRequest*>    mRequestMethods;    //!< List of request methods.
    QList<SIMethodResponse*>   mResponseMethods;   //!< List of response methods.
    QList<SIMethodBroadcast*>  mBroadcastMethods;  //!< List of broadcast methods.
};

//////////////////////////////////////////////////////////////////////////
// SIMethodData inline functions
//////////////////////////////////////////////////////////////////////////

inline bool SIMethodData::hasMethod(const SIMethodBase& method) const
{
    return (findMethod(method.getName(), method.getMethodType()) != nullptr);
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

