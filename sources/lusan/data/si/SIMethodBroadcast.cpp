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

SIMethodBroadcast::SIMethodBroadcast(ElementBase* parent /*= nullptr*/)
    : SIMethodBase(eMethodType::MethodBroadcast, parent)
{
}

SIMethodBroadcast::SIMethodBroadcast(uint32_t id, const QString& name, const QString& description, ElementBase* parent /*= nullptr*/)
    : SIMethodBase(id, name, description, eMethodType::MethodBroadcast, parent)
{
}

SIMethodBroadcast::SIMethodBroadcast(uint32_t id, const QString& name, ElementBase* parent /*= nullptr*/)
    : SIMethodBase(id, name, QString(), eMethodType::MethodBroadcast, parent)
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

SIMethodBroadcast& SIMethodBroadcast::operator = (const SIMethodBroadcast& other)
{
    if (this != &other)
    {
        SIMethodBase::operator = (other);
    }
    return *this;
}

SIMethodBroadcast& SIMethodBroadcast::operator = (SIMethodBroadcast&& other) noexcept
{
    if (this != &other)
    {
        SIMethodBase::operator = (std::move(other));
    }
    return *this;
}

bool SIMethodBroadcast::readFromXml(QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    if (xml.name() == XmlSI::xmlSIElementMethod && attributes.value(XmlSI::xmlSIAttributeMethodType) == getType())
    {
        setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
        mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
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

void SIMethodBroadcast::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementMethod);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeMethodType, getType());
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
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

QIcon SIMethodBroadcast::getIcon(ElementBase::eDisplay display) const
{
    return (display == ElementBase::eDisplay::DisplayName ? QIcon::fromTheme(QIcon::ThemeIcon::ImageLoading) : QIcon());
}

QString SIMethodBroadcast::getString(ElementBase::eDisplay display) const
{
    return (display == ElementBase::eDisplay::DisplayName ? getName() : QString());
}
