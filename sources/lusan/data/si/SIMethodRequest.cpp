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
 *  \file        lusan/data/si/SIMethodRequest.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Method Request.
 *
 ************************************************************************/

#include "lusan/data/si/SIMethodRequest.hpp"
#include "lusan/data/si/SIMethodResponse.hpp"

SIMethodRequest::SIMethodRequest(ElementBase* parent /*= nullptr*/)
    : SIMethodBase  (SIMethodBase::eMethodType::MethodRequest, parent)
    , mRespMethod   (nullptr)
    , mRespName     ()
{
}

SIMethodRequest::SIMethodRequest(uint32_t id, const QString& name, const QString& description, ElementBase* parent /*= nullptr*/)
    : SIMethodBase  (id, name, description, eMethodType::MethodRequest, parent)
    , mRespMethod   (nullptr)
    , mRespName     ()
{
}

SIMethodRequest::SIMethodRequest(uint32_t id, const QString& name, ElementBase* parent /*= nullptr*/)
    : SIMethodBase  (id, name, QString(), eMethodType::MethodRequest, parent)
    , mRespMethod   (nullptr)
    , mRespName     ()
{
}

SIMethodRequest::SIMethodRequest(const SIMethodRequest& src)
    : SIMethodBase(src)
    , mRespMethod (src.mRespMethod)
    , mRespName   (src.mRespName)
{
}

SIMethodRequest::SIMethodRequest(SIMethodRequest&& src) noexcept
    : SIMethodBase(std::move(src))
    , mRespMethod (std::move(src.mRespMethod))
    , mRespName   (std::move(src.mRespName))
{
}

SIMethodRequest::~SIMethodRequest(void)
{
}

SIMethodRequest& SIMethodRequest::operator = (const SIMethodRequest& other)
{
    if (this != &other)
    {
        SIMethodBase::operator = (other);
        mRespMethod = other.mRespMethod;
        mRespName   = other.mRespName;
    }

    return *this;
}

SIMethodRequest& SIMethodRequest::operator = (SIMethodRequest&& other) noexcept
{
    if (this != &other)
    {
        SIMethodBase::operator = (std::move(other));
        mRespMethod = std::move(other.mRespMethod);
        mRespName   = std::move(other.mRespName);
    }

    return *this;
}

void SIMethodRequest::normalize(const QList<SIMethodResponse*>& listResponses)
{
    if ((mRespName.isEmpty() == false) && (mRespMethod == nullptr))
    {
        for (auto it = listResponses.begin(); it != listResponses.end(); ++it)
        {
            if ((*it)->getName() == mRespName)
            {
                mRespMethod = (*it);
                break;
            }
        }
    }
}

void SIMethodRequest::connectResponse(SIMethodResponse* respMethod)
{
    if (mRespMethod != respMethod)
    {
        mRespName.clear();
        mRespMethod = respMethod;
        if (mRespMethod != nullptr)
        {
            mRespName   = respMethod->getName();
        }
    }
}

const QString& SIMethodRequest::getConectedResponseName(void) const
{
    return mRespMethod != nullptr ? mRespMethod->getName() : mRespName;
}

SIMethodResponse* SIMethodRequest::getConectedResponse(void) const
{
    return mRespMethod;
}

void SIMethodRequest::clearResponse(void)
{
    mRespName.clear();
    mRespMethod = nullptr;
}

bool SIMethodRequest::readFromXml(QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    if (xml.name() == XmlSI::xmlSIElementMethod && attributes.value(XmlSI::xmlSIAttributeMethodType) == getType())
    {
        setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
        mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
        mRespName = attributes.hasAttribute(XmlSI::xmlSIAttributeResponse) ? attributes.value(XmlSI::xmlSIAttributeResponse).toString() : "";
        setIsDeprecated(attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() == XmlSI::xmlSIValueTrue : false);

        while (xml.readNextStartElement())
        {
            if (xml.name() == XmlSI::xmlSIElementDescription)
            {
                mDescription = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementDeprecateHint)
            {
                setDeprecateHint(xml.readElementText());
            }
            else if (xml.name() == XmlSI::xmlSIElementParamList)
            {
                while (xml.readNextStartElement())
                {
                    if (xml.name() == XmlSI::xmlSIElementParameter)
                    {
                        MethodParameter parameter(this);
                        if (parameter.readFromXml(xml))
                        {
                            addElement(std::move(parameter), false);
                        }
                        else
                        {
                            xml.skipCurrentElement();
                        }
                    }
                    else
                    {
                        xml.skipCurrentElement();
                    }
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

void SIMethodRequest::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementMethod);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeMethodType, getType());
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    if (mRespName.isEmpty() == false)
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeResponse, mRespName);
    }

    if (getIsDeprecated())
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
    }

    xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
    if (getIsDeprecated())
    {
        xml.writeTextElement(XmlSI::xmlSIElementDeprecateHint, getDeprecateHint());
    }

    const QList<MethodParameter> & elements = getElements();
    if (elements.isEmpty() == false)
    {
        xml.writeStartElement(XmlSI::xmlSIElementParamList);
        for (const auto& parameter : elements)
        {
            parameter.writeToXml(xml);
        }

        xml.writeEndElement();
    }

    xml.writeEndElement();
}
