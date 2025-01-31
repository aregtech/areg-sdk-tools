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
    , mResponse     ()
{
}

SIMethodRequest::SIMethodRequest(uint32_t id, const QString& name, const QString& description, ElementBase* parent /*= nullptr*/)
    : SIMethodBase  (id, name, description, eMethodType::MethodRequest, parent)
    , mResponse()
{
}

SIMethodRequest::SIMethodRequest(uint32_t id, const QString& name, ElementBase* parent /*= nullptr*/)
    : SIMethodBase  (id, name, QString(), eMethodType::MethodRequest, parent)
    , mResponse()
{
}

SIMethodRequest::SIMethodRequest(const SIMethodRequest& src)
    : SIMethodBase(src)
    , mResponse(src.mResponse)
{
}

SIMethodRequest::SIMethodRequest(SIMethodRequest&& src) noexcept
    : SIMethodBase  (std::move(src))
    , mResponse     (std::move(src.mResponse))
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
        mResponse = other.mResponse;
    }

    return *this;
}

SIMethodRequest& SIMethodRequest::operator = (SIMethodRequest&& other) noexcept
{
    if (this != &other)
    {
        SIMethodBase::operator = (std::move(other));
        mResponse = std::move(other.mResponse);
    }

    return *this;
}

void SIMethodRequest::normalize(const QList<SIMethodResponse*>& listResponses)
{
    mResponse.validate(listResponses);
}

void SIMethodRequest::connectResponse(SIMethodResponse* respMethod)
{
    mResponse.setType(respMethod);
}

const QString& SIMethodRequest::getConectedResponseName(void) const
{
    return mResponse.getName();
}

const SIMethodResponse* SIMethodRequest::getConectedResponse(void) const
{
    return mResponse.getType();
}

bool SIMethodRequest::hasValidResponse(void) const
{
    return mResponse.isValid();
}

void SIMethodRequest::clearResponse(void)
{
    mResponse.invalidate();
    mResponse.setName(QString());
}

bool SIMethodRequest::readFromXml(QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    if (xml.name() == XmlSI::xmlSIElementMethod && attributes.value(XmlSI::xmlSIAttributeMethodType) == getType())
    {
        setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
        mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
        QString response = attributes.hasAttribute(XmlSI::xmlSIAttributeResponse) ? attributes.value(XmlSI::xmlSIAttributeResponse).toString() : "";
        mResponse.setName(response);
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
    if (mResponse.isValid() && (mResponse.getName().isEmpty() == false))
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeResponse, mResponse.getName());
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

QIcon SIMethodRequest::getIcon(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return QIcon::fromTheme(QIcon::ThemeIcon::ImageLoading);
    case ElementBase::eDisplay::DisplayLink:
        if (mResponse.isValid() && (mResponse.getName().isEmpty() == false))
        {
            return QIcon();
        }
        else if ((mResponse.isValid() == false) && mResponse.getName().isEmpty())
        {
            return QIcon();
        }
        else
        {
            return QIcon::fromTheme(QIcon::ThemeIcon::DialogWarning);
        }
    default:
        return QIcon();
    }
}

QString SIMethodRequest::getString(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return getName();
    case ElementBase::eDisplay::DisplayLink:
        return mResponse.getName();
    default:
        return QString();
    }
}
