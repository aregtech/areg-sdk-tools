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
    Method* findMethodInList(const QList<Method*>& list, const uint32_t id)
    {
        for (const auto& method : list)
        {
            if (method->getId() == id)
            {
                return method;
            }
        }

        return nullptr;
    }

    template<class Method>
    int32_t findMethodIndex(const QList<Method*>& list, const QString& name)
    {
        int32_t result = -1;
        for (int i = 0; i < list.size(); ++i)
        {
            if (list.at(i)->getName() == name)
            {
                result = i;
                break;
            }
        }

        return result;
    }

    template<class Method>
    int32_t findMethodIndex(const QList<Method*>& list, uint32_t id)
    {
        int32_t result = -1;
        for (int i = 0; i < list.size(); ++i)
        {
            if (list.at(i)->getId() == id)
            {
                result = i;
                break;
            }
        }

        return result;
    }

    template<class Method>
    void appendMethodList(const QList<Method*>& list, QList<SIMethodBase *>& result)
    {
        for (Method* method : list)
        {
            result.append(method);
        }
    }

    template<class Method>
    void sortList(QList<Method*>& list, bool ascending)
    {
        std::sort(list.begin(), list.end(), [ascending](const Method* lhs, const Method* rhs)
            {
                return (ascending ? (lhs->getId() < rhs->getId()) : (lhs->getId() > rhs->getId()));
            });
    }

    template<class Method>
    void sortListByName(QList<Method*>& list, bool ascending)
    {
        std::sort(list.begin(), list.end(), [ascending](const Method* lhs, const Method* rhs)
            {
                return (ascending ? (lhs->getName() < rhs->getName()) : (lhs->getName() > rhs->getName()));
            });
    }
}

SIMethodData::SIMethodData(ElementBase* parent /*= nullptr*/)
    : ElementBase       (parent)
    , mRequestMethods   ( )
    , mResponseMethods  ( )
    , mBroadcastMethods ( )
    , mAllMethods       ( )
{
}

SIMethodData::~SIMethodData(void)
{
    qDeleteAll(mAllMethods);
    mAllMethods.clear();
    mRequestMethods.clear();
    mResponseMethods.clear();
    mBroadcastMethods.clear();
}

bool SIMethodData::addMethod(SIMethodBase* method)
{
    if ((method->getParent() != this) || (method->getParent() == nullptr))
    {
        method->setParent(this);
        method->setId(getNextId());
    }
    else if (findMethodInList(mAllMethods, method->getId()) != nullptr)
    {
        return false;
    }

    
    switch (method->getMethodType())
    {
    case SIMethodBase::eMethodType::MethodRequest:
        mAllMethods.append(method);
        mRequestMethods.append(static_cast<SIMethodRequest*>(method));
        break;

    case SIMethodBase::eMethodType::MethodResponse:
        mAllMethods.append(method);
        mResponseMethods.append(static_cast<SIMethodResponse*>(method));
        break;

    case SIMethodBase::eMethodType::MethodBroadcast:
        mAllMethods.append(method);
        mBroadcastMethods.append(static_cast<SIMethodBroadcast*>(method));
        break;

    default:
        delete method;
        return false;
        break;
    }

    return true;
}

SIMethodBase* SIMethodData::addMethod(const QString& name, SIMethodBase::eMethodType methodType)
{
    return _createMethod(methodType, name);
}

bool SIMethodData::removeMethod(const QString& name, SIMethodBase::eMethodType methodType)
{
    SIMethodBase* method {nullptr};
    switch (methodType)
    {
    case SIMethodBase::eMethodType::MethodRequest:
        method = removeMethodFromList(mRequestMethods, name);
        break;

    case SIMethodBase::eMethodType::MethodResponse:
        method =  removeMethodFromList(mResponseMethods, name);
        break;

    case SIMethodBase::eMethodType::MethodBroadcast:
        method = removeMethodFromList(mBroadcastMethods, name);
        break;

    default:
        return false;
    }

    bool result = mAllMethods.removeOne(method);
    delete method;
    return result;
}

bool SIMethodData::removeMethod(uint32_t id)
{

    SIMethodBase * method = findMethodInList(mAllMethods, id);
    removeMethod(method);
    return (method != nullptr);
}

void SIMethodData::removeMethod(SIMethodBase* method)
{
    if (method == nullptr)
        return;

    switch (method->getMethodType())
    {
    case SIMethodBase::eMethodType::MethodRequest:
        mRequestMethods.removeOne(method);
        break;

    case SIMethodBase::eMethodType::MethodResponse:
        mResponseMethods.removeOne(method);
        break;

    case SIMethodBase::eMethodType::MethodBroadcast:
        mBroadcastMethods.removeOne(method);
        break;

    default:
        break;
    }

    mAllMethods.removeOne(method);
    delete method;
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

SIMethodBase* SIMethodData::findMethod(uint32_t id) const
{
    return findMethodInList(mAllMethods, id);
}


QList<SIMethodBase*> SIMethodData::getAllMethods(void) const
{
    return mAllMethods;
}

QString SIMethodData::getResponse(const QString& request) const
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
    qDeleteAll(mAllMethods);
    mRequestMethods.clear();
    mResponseMethods.clear();
    mBroadcastMethods.clear();
    mAllMethods.clear();
}

SIMethodBase* SIMethodData::_convertMethod(SIMethodBase* method, SIMethodBase::eMethodType methodType)
{
    SIMethodBase* result{ nullptr };
    switch (methodType)
    {
    case SIMethodBase::eMethodType::MethodRequest:
        result = new SIMethodRequest(method->getId(), method->getName(), method->getDescription(), this);
        break;

    case SIMethodBase::eMethodType::MethodResponse:
        result = new SIMethodResponse(method->getId(), method->getName(), method->getDescription(), this);
        break;

    case SIMethodBase::eMethodType::MethodBroadcast:
        result = new SIMethodBroadcast(method->getId(), method->getName(), method->getDescription(), this);
        break;

    default:
        break;
    }

    if (result != nullptr)
    {
        result->setIsDeprecated(method->isDeprecated());
        result->setDeprecateHint(method->getDeprecateHint());
        result->setElements(method->getElements());

        removeMethod(method);
        _addMethod(result);
    }

    return result;
}

SIMethodBase* SIMethodData::_createMethod(SIMethodBase::eMethodType methodType, const QString& name)
{
    SIMethodBase* result{ nullptr };
    switch (methodType)
    {
    case SIMethodBase::eMethodType::MethodRequest:
        result = new SIMethodRequest(getNextId(), name, this);
        break;

    case SIMethodBase::eMethodType::MethodResponse:
        result = new SIMethodResponse(getNextId(), name, this);
        break;

    case SIMethodBase::eMethodType::MethodBroadcast:
        result = new SIMethodBroadcast(getNextId(), name, this);
        break;

    default:
        break;
    }

    _addMethod(result);
    return result;
}

void SIMethodData::_addMethod(SIMethodBase* method)
{
    if (method != nullptr)
    {
        mAllMethods.append(method);
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