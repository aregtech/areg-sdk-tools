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
#include "lusan/common/XmlSI.hpp"

namespace
{
    template<class Method>
    Method* removeMethodFromList(QList<Method*>& list, const QString& name)
    {
        Method* result{ nullptr };
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            if ((*it)->getName() == name)
            {
                result = (*it);
                list.erase(it);
                break;
            }
        }

        return result;
    }

    template<class Method>
    Method* removeMethodFromList(QList<Method*>& list, uint32_t id)
    {
        Method* result{ nullptr };
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            if ((*it)->getId() == id)
            {
                result = (*it);
                list.erase(it);
                break;
            }
        }

        return result;
    }
}

SIMethodData::SIMethodData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SIMethodBase*, ElementBase> (parent)
    , mRequestMethods   ( )
    , mResponseMethods  ( )
    , mBroadcastMethods ( )
{
}

SIMethodData::~SIMethodData(void)
{
    removeAll();
}

bool SIMethodData::addMethod(SIMethodBase* method)
{
    if (method == nullptr || hasMethod(method))
        return false;

    addMethodToList(method);
    return true;
}

SIMethodBase* SIMethodData::addMethod(const QString& name, SIMethodBase::eMethodType methodType)
{
    SIMethodBase* method = createMethod(methodType, name);
    if (method != nullptr)
    {
        addMethod(method);
    }

    return method;
}

bool SIMethodData::removeMethod(const QString& name, SIMethodBase::eMethodType methodType)
{
    SIMethodBase* method = findMethod(name, methodType);
    if (method != nullptr)
    {
        removeMethod(method);
        return true;
    }

    return false;
}

bool SIMethodData::removeMethod(uint32_t id)
{
    SIMethodBase* method = findMethod(id);
    if (method != nullptr)
    {
        removeMethod(method);
        return true;
    }

    return false;
}

void SIMethodData::removeMethod(SIMethodBase* method)
{
    if (method == nullptr)
        return;

    removeMethodFromList(method);
    delete method;
}

SIMethodBase* SIMethodData::findMethod(const QString& name, SIMethodBase::eMethodType methodType) const
{
    const QList<SIMethodBase*>& allMethods = getElements();
    for (SIMethodBase* method : allMethods)
    {
        if ((method->getName() == name) && (method->getMethodType() == methodType))
        {
            return method;
        }
    }

    return nullptr;
}

SIMethodBase* SIMethodData::findMethod(uint32_t id) const
{
    SIMethodBase** result = findElement(id);
    return (result != nullptr ? *result : nullptr);
}

SIMethodResponse* SIMethodData::findConnectedResponse(uint32_t reqId) const
{
    SIMethodBase* method = findMethod(reqId);
    if ((method == nullptr) || (method->getMethodType() != SIMethodBase::eMethodType::MethodRequest))
        return nullptr;
    else
        return static_cast<SIMethodRequest *>(method)->getConectedResponse();
}

bool SIMethodData::hasResponseConnectedRequest(const QString& response) const
{
    for (SIMethodRequest* request : mRequestMethods)
    {
        if (request->getConectedResponseName() == response)
        {
            return true;
        }
    }

    return false;
}

bool SIMethodData::hasResponseConnectedRequest(uint32_t respId) const
{
    for (SIMethodRequest* request : mRequestMethods)
    {
        SIMethodResponse* response = request->getConectedResponse();
        if ((response != nullptr) &&  (response->getId() == respId))
        {
            return true;
        }
    }

    return false;
}

bool SIMethodData::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementMethodList))
        return false;

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
                method = new SIMethodRequest(this);
                break;

            case SIMethodBase::eMethodType::MethodResponse:
                method = new SIMethodResponse(this);
                break;

            case SIMethodBase::eMethodType::MethodBroadcast:
                method = new SIMethodBroadcast(this);
                break;

            default:
                xml.skipCurrentElement();
                continue;
            }

            if ((method->readFromXml(xml) == false) || (addMethod(method) == false))
            {
                delete method;
            }
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    for (SIMethodRequest* req : mRequestMethods)
    {
        req->normalize(mResponseMethods);
    }

    return true;
}

void SIMethodData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementMethodList);

    const QList<SIMethodBase*>& allMethods = getElements();
    Q_ASSERT(allMethods.size() == (mBroadcastMethods.size() + mRequestMethods.size() + mResponseMethods.size()));
    for (const SIMethodBase* method : allMethods)
    {
        method->writeToXml(xml);
    }

    xml.writeEndElement();
}

void SIMethodData::removeAll(void)
{
    QList<SIMethodBase *>& allMethods = getElements();
    qDeleteAll(allMethods);
    allMethods.clear();
    mRequestMethods.clear();
    mResponseMethods.clear();
    mBroadcastMethods.clear();
}

SIMethodBase* SIMethodData::convertMethod(SIMethodBase* method, SIMethodBase::eMethodType methodType)
{
    if (method == nullptr)
        return nullptr;
    else if (method->getMethodType() == methodType)
        return method;

    SIMethodBase* newMethod = createMethod(methodType, method->getName(), method->getId());
    if (newMethod != nullptr)
    {
        newMethod->setElements(method->getElements());
        newMethod->setDescription(method->getDescription());
        newMethod->setIsDeprecated(method->getIsDeprecated());
        newMethod->setDeprecateHint(method->getDeprecateHint());

        replaceMethodInList(method, newMethod);
    }

    return newMethod;
}

bool SIMethodData::replaceMethod(SIMethodBase* oldMethod, SIMethodBase* newMethod)
{
    if (oldMethod == nullptr || newMethod == nullptr)
        return false;

    replaceMethodInList(oldMethod, newMethod);
    return true;
}

void SIMethodData::sortByName(bool ascending)
{
    sortElementsByName(ascending);
}

void SIMethodData::sortById(bool ascending)
{
    sortElementsById(ascending);
}

QList<SIMethodRequest*> SIMethodData::getConnectedRequests(SIMethodResponse* response) const
{
    QList<SIMethodRequest*> result;
    for (auto reqeust : mRequestMethods)
    {
        if (reqeust->getConectedResponse() == response)
        {
            result.append(reqeust);
        }
    }

    return result;
}

MethodParameter* SIMethodData::addParameter(SIMethodBase* method, const QString& name, const QString& type /*= "bool"*/)
{
    return (method != nullptr ? method->addParameter(name, type) : nullptr);
}

void SIMethodData::validate(const QList<DataTypeCustom*>& dataTypes)
{
    QList<SIMethodBase*>& list = getElements();
    for (SIMethodBase* entry : list)
    {
        entry->validate(dataTypes);
    }
}

SIMethodBase* SIMethodData::createMethod(SIMethodBase::eMethodType methodType, const QString& name)
{
    return createMethod(methodType, name, getNextId());
}

void SIMethodData::addMethodToList(SIMethodBase* method)
{
    if (method != nullptr)
    {
        addElement(method, false);
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
            break;
        }
    }
}

void SIMethodData::removeMethodFromList(SIMethodBase* method)
{
    if (method != nullptr)
    {
        removeElement(method->getId());
        switch (method->getMethodType())
        {
        case SIMethodBase::eMethodType::MethodRequest:
            mRequestMethods.removeOne(static_cast<SIMethodRequest*>(method));
            break;
        case SIMethodBase::eMethodType::MethodResponse:
            mResponseMethods.removeOne(static_cast<SIMethodResponse*>(method));
            break;
        case SIMethodBase::eMethodType::MethodBroadcast:
            mBroadcastMethods.removeOne(static_cast<SIMethodBroadcast*>(method));
            break;
        default:
            break;
        }
    }
}

void SIMethodData::replaceMethodInList(SIMethodBase* oldMethod, SIMethodBase* newMethod)
{
    if (oldMethod != nullptr && newMethod != nullptr)
    { 
        switch (oldMethod->getMethodType())
        {
        case SIMethodBase::eMethodType::MethodRequest:
            mRequestMethods.removeOne(static_cast<SIMethodRequest*>(oldMethod));
            break;
        case SIMethodBase::eMethodType::MethodResponse:
            mResponseMethods.removeOne(static_cast<SIMethodResponse*>(oldMethod));
            break;
        case SIMethodBase::eMethodType::MethodBroadcast:
            mBroadcastMethods.removeOne(static_cast<SIMethodBroadcast*>(oldMethod));
            break;
        default:
            break;
        }

        replaceElement(oldMethod, newMethod);
        
        switch (newMethod->getMethodType())
        {
        case SIMethodBase::eMethodType::MethodRequest:
            mRequestMethods.append(static_cast<SIMethodRequest*>(newMethod));
            break;
        case SIMethodBase::eMethodType::MethodResponse:
            mResponseMethods.append(static_cast<SIMethodResponse*>(newMethod));
            break;
        case SIMethodBase::eMethodType::MethodBroadcast:
            mBroadcastMethods.append(static_cast<SIMethodBroadcast*>(newMethod));
            break;
        default:
            break;
        }
    }
}

SIMethodBase* SIMethodData::createMethod(SIMethodBase::eMethodType methodType, const QString& name, uint32_t id)
{
    switch (methodType)
    {
    case SIMethodBase::eMethodType::MethodRequest:
        return new SIMethodRequest(id, name, this);
        break;

    case SIMethodBase::eMethodType::MethodResponse:
        return new SIMethodResponse(id, name, this);
        break;

    case SIMethodBase::eMethodType::MethodBroadcast:
        return new SIMethodBroadcast(id, name, this);
        break;

    default:
        return nullptr;
        break;
    }
}
