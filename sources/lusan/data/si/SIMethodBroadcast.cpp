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
 * \file        lusan/data/si/SIMethodBroadcast.cpp
 * \ingroup     Lusan - GUI Tool for AREG SDK
 * \author      Artak Avetyan
 * \brief       Lusan application, Service Interface Method Broadcast.
 *
 ************************************************************************/
#include "lusan/data/si/SIMethodBroadcast.hpp"
#include "lusan/common/XmlSI.hpp"

SIMethodBroadcast::SIMethodBroadcast(void)
    : SIMethodBase()
{
    mMethodType = eMethodType::MethodBroadcast;
}

SIMethodBroadcast::SIMethodBroadcast(uint32_t id, const QString& name, const QString& description)
    : SIMethodBase(id, name, description, eMethodType::MethodBroadcast)
{
}

SIMethodBroadcast::SIMethodBroadcast(const SIMethodBroadcast& src)
    : SIMethodBase(src)
{
}

SIMethodBroadcast::SIMethodBroadcast(SIMethodBroadcast&& src) noexcept
    : SIMethodBase(std::move(src))
{
}

SIMethodBroadcast::~SIMethodBroadcast(void)
{
}

SIMethodBroadcast& SIMethodBroadcast::operator=(const SIMethodBroadcast& other)
{
    if (this != &other)
    {
        SIMethodBase::operator=(other);
    }
    return *this;
}

SIMethodBroadcast& SIMethodBroadcast::operator=(SIMethodBroadcast&& other) noexcept
{
    if (this != &other)
    {
        SIMethodBase::operator=(std::move(other));
    }
    return *this;
}

bool SIMethodBroadcast::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() == XmlSI::xmlSIElementAttribute && xml.attributes().value(XmlSI::xmlSIAttributeType) == getType())
    {
        QXmlStreamAttributes attributes = xml.attributes();
        mId = attributes.value(XmlSI::xmlSIAttributeID).toUInt();
        mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
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
            else if (xml.name() == XmlSI::xmlSIElementAttributeList)
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

void SIMethodBroadcast::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementAttribute);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(mId));
    xml.writeAttribute(XmlSI::xmlSIAttributeType, getType());
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);

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
        xml.writeStartElement(XmlSI::xmlSIElementAttributeList);
        for (const auto& parameter : mParameters)
        {
            parameter.writeToXml(xml);
        }
        xml.writeEndElement();
    }

    xml.writeEndElement();
}
