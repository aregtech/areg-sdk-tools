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
 *  \file        lusan/data/si/SIMethodRequest.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Method Request.
 *
 ************************************************************************/

#include "lusan/data/si/SIMethodRequest.hpp"

SIMethodRequest::SIMethodRequest(void)
    : SIMethodBase(SIMethodBase::eMethodType::MethodRequest)
    , mResponse   ()
{
}

SIMethodRequest::SIMethodRequest(uint32_t id, const QString& name, const QString& description)
    : SIMethodBase(id, name, description, eMethodType::MethodRequest)
    , mResponse   ()
{
}

SIMethodRequest::SIMethodRequest(const SIMethodRequest& src)
    : SIMethodBase(src)
    , mResponse   (src.mResponse)
{
}

SIMethodRequest::SIMethodRequest(SIMethodRequest&& src) noexcept
    : SIMethodBase(std::move(src))
    , mResponse   (std::move(mResponse))
{
}

SIMethodRequest::~SIMethodRequest(void)
{
}

SIMethodRequest& SIMethodRequest::operator=(const SIMethodRequest& other)
{
    if (this != &other)
    {
        SIMethodBase::operator=(other);
        mResponse = other.mResponse;
    }

    return *this;
}

SIMethodRequest& SIMethodRequest::operator=(SIMethodRequest&& other) noexcept
{
    if (this != &other)
    {
        SIMethodBase::operator=(std::move(other));
        mResponse = std::move(other.mResponse);
    }
    return *this;
}

void SIMethodRequest::connectResponse(const QString& response)
{
    mResponse = response;
}

const QString& SIMethodRequest::getConectedResponse(void) const
{
    return mResponse;
}

void SIMethodRequest::clearResponse(void)
{
    mResponse.clear();
}

bool SIMethodRequest::readFromXml(QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    if (xml.name() == XmlSI::xmlSIElementMethod && attributes.value(XmlSI::xmlSIAttributeMethodType) == getType())
    {
        mId = attributes.value(XmlSI::xmlSIAttributeID).toUInt();
        mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
        mResponse = attributes.hasAttribute(XmlSI::xmlSIAttributeResponse) ? attributes.value(XmlSI::xmlSIAttributeResponse).toString() : "";
        mIsDeprecated = attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() == "true" : false;
        mDeprecateHint.clear();

        while (xml.readNextStartElement())
        {
            if (xml.name() == XmlSI::xmlSIElementDescription)
            {
                mDescription = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementDeprecateHint && mIsDeprecated)
            {
                mDeprecateHint = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementParamList)
            {
                while (xml.readNextStartElement())
                {
                    if (xml.name() == XmlSI::xmlSIElementParameter)
                    {
                        MethodParameter parameter;
                        if (parameter.readFromXml(xml))
                        {
                            mParameters.append(parameter);
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
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(mId));
    xml.writeAttribute(XmlSI::xmlSIAttributeMethodType, getType());
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    if (mResponse.isEmpty() == false)
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeResponse, mResponse);
    }

    if (mIsDeprecated)
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, "true");
    }

    if (!mDescription.isEmpty())
    {
        xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
    }

    if (mIsDeprecated && !mDeprecateHint.isEmpty())
    {
        xml.writeTextElement(XmlSI::xmlSIElementDeprecateHint, mDeprecateHint);
    }

    if (!mParameters.isEmpty())
    {
        xml.writeStartElement(XmlSI::xmlSIElementParamList);
        for (const auto& parameter : mParameters)
        {
            parameter.writeToXml(xml);
        }

        xml.writeEndElement();
    }

    xml.writeEndElement();
}
