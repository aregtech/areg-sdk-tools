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
 * \file        lusan/data/si/SIMethodData.cpp
 * \ingroup     Lusan - GUI Tool for AREG SDK
 * \author      Artak Avetyan
 * \brief       Lusan application, Service Interface Method Data.
 *
 ************************************************************************/

#include "lusan/data/si/SIMethodData.hpp"
#include "SIMethodData.hpp"

namespace
{
    template<class Method>
    bool removeMethodFromList(QList<Method*>& list, const QString& name)
    {
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            if ((*it)->getName() == name)
            {
                delete *it;
                list.erase(it);
                return true;
            }
        }
        
        return false;
    }
    
    template<class Method>    
    Method* findMethodInList(const QList<Method*>& list, const QString& name)
    {
        for (const auto& method : list)
        {
            if (method->getName() == name)
            {
                return method;
            }
        }
        
        return nullptr;
    }
    
    template<class Method>
    void appendMethodList(const QList<Method*>& list, QList<SIMethodBase *>& result)
    {
        for (Method* method : list)
        {
            result.append(method);
        }
    }
}

SIMethodData::SIMethodData(void)
    : mRequestMethods   ( )
    , mResponseMethods  ( )
    , mBroadcastMethods ( )
{
}

SIMethodData::~SIMethodData(void)
{
    qDeleteAll(mRequestMethods);
    qDeleteAll(mResponseMethods);
    qDeleteAll(mBroadcastMethods);
    mRequestMethods.clear();
    mResponseMethods.clear();
    mBroadcastMethods.clear();
}

bool SIMethodData::addMethod(SIMethodBase* method, bool isUnique /*= true*/)
{
    if (isUnique)
    {
        if (findMethod(method->getName(), method->getMethodType()) != nullptr)
        {
            return false;
        }
    }

    switch (method->getMethodType())
    {
    case SIMethodBase::eMethodType::MethodRequest:
        mRequestMethods.append(static_cast<SIMethodRequest*>(method));
        break;
    case SIMethodBase::eMethodType::MethodResponse:
        mResponseMethods.append(static_cast<SIMethodResponse*>(method));
        break;
    case SIMethodBase::eMethodType::MethodBroadcast:
        mBroadcastMethods.append(static_cast<SIMethodBroadcast*>(method));
        break;
    default:
        delete method;
        break;
    }

    return true;
}

bool SIMethodData::removeMethod(const QString& name, SIMethodBase::eMethodType methodType)
{
    switch (methodType)
    {
    case SIMethodBase::eMethodType::MethodRequest:
        return removeMethodFromList(mRequestMethods, name);
    case SIMethodBase::eMethodType::MethodResponse:
        return removeMethodFromList(mResponseMethods, name);
    case SIMethodBase::eMethodType::MethodBroadcast:
        return removeMethodFromList(mBroadcastMethods, name);
    default:
        return false;
    }
}

void SIMethodData::insertMethod(int index, SIMethodBase* method)
{
    switch (method->getMethodType())
    {
    case SIMethodBase::eMethodType::MethodRequest:
        mRequestMethods.insert(index, static_cast<SIMethodRequest*>(method));
        break;
    case SIMethodBase::eMethodType::MethodResponse:
        mResponseMethods.insert(index, static_cast<SIMethodResponse*>(method));
        break;
    case SIMethodBase::eMethodType::MethodBroadcast:
        mBroadcastMethods.insert(index, static_cast<SIMethodBroadcast*>(method));
        break;
    default:
        delete method;
        break;
    }
}

SIMethodBase* SIMethodData::findMethod(const QString& name, SIMethodBase::eMethodType methodType) const
{
    switch (methodType)
    {
    case SIMethodBase::eMethodType::MethodRequest:
        return findMethodInList(mRequestMethods, name);
    case SIMethodBase::eMethodType::MethodResponse:
        return findMethodInList(mResponseMethods, name);
    case SIMethodBase::eMethodType::MethodBroadcast:
        return findMethodInList(mBroadcastMethods, name);
    default:
        return nullptr;
    }
}

QList<SIMethodBase*> SIMethodData::getAllMethods(void) const
{
    QList<SIMethodBase*> methods;
    appendMethodList(mRequestMethods, methods);
    appendMethodList(mResponseMethods, methods);
    appendMethodList(mBroadcastMethods, methods);
    return methods;
}

inline QString SIMethodData::getRequestConnectedResponse(const QString& request) const
{
    QString result;

    for (const SIMethodRequest* method : mRequestMethods)
    {
        if (method->getName() == request)
        {
            result = method->getConectedResponse();
            break;
        }
    }

    return result;
}

bool SIMethodData::hasResponseConnectedRequest(const QString& response) const
{
    bool result{ false };

    for (const SIMethodRequest* method : mRequestMethods)
    {
        if (method->getConectedResponse() == response)
        {
            result = true;
            break;
        }
    }

    return result;
}

bool SIMethodData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() == XmlSI::xmlSIElementMethodList)
    {
        while (xml.readNextStartElement())
        {
            if (xml.name() == XmlSI::xmlSIElementMethod)
            {
                QXmlStreamAttributes attributes = xml.attributes();
                QString methodTypeStr = attributes.value(XmlSI::xmlSIAttributeMethodType).toString();
                SIMethodBase::eMethodType methodType = SIMethodBase::fromString(methodTypeStr);

                SIMethodBase* method = nullptr;
                switch (methodType)
                {
                case SIMethodBase::eMethodType::MethodRequest:
                    method = new SIMethodRequest();
                    break;
                case SIMethodBase::eMethodType::MethodResponse:
                    method = new SIMethodResponse();
                    break;
                case SIMethodBase::eMethodType::MethodBroadcast:
                    method = new SIMethodBroadcast();
                    break;
                default:
                    xml.skipCurrentElement();
                    continue;
                }

                if ((method->readFromXml(xml) == false) || (addMethod(method, true) == false))
                {
                    delete method;
                }
            }
            else
            {
                xml.skipCurrentElement();
            }
        }

        return true;
    }

    return false;
}

void SIMethodData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementMethodList);

    for (const auto& method : mRequestMethods)
    {
        method->writeToXml(xml);
    }

    for (const auto& method : mResponseMethods)
    {
        method->writeToXml(xml);
    }

    for (const auto& method : mBroadcastMethods)
    {
        method->writeToXml(xml);
    }

    xml.writeEndElement();
}

void SIMethodData::removeAll(void)
{
    qDeleteAll(mRequestMethods);
    qDeleteAll(mResponseMethods);
    qDeleteAll(mBroadcastMethods);
    mRequestMethods.clear();
    mResponseMethods.clear();
    mBroadcastMethods.clear();
}

