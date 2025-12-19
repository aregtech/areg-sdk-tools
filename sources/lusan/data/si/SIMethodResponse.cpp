/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/si/SIMethodResponse.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Method Response.
 *
 ************************************************************************/

#include "lusan/data/si/SIMethodResponse.hpp"
#include "lusan/common/NELusanCommon.hpp"

SIMethodResponse::SIMethodResponse(ElementBase* parent /*= nullptr*/)
    : SIMethodBase(eMethodType::MethodResponse, parent)
{
}

SIMethodResponse::SIMethodResponse(uint32_t id, const QString& name, const QString& description, ElementBase* parent /*= nullptr*/)
    : SIMethodBase(id, name, description, eMethodType::MethodResponse, parent)
{
}

SIMethodResponse::SIMethodResponse(uint32_t id, const QString& name, ElementBase* parent /*= nullptr*/)
    : SIMethodBase(id, name, QString(), eMethodType::MethodResponse, parent)
{
}

SIMethodResponse::SIMethodResponse(const SIMethodResponse& src)
    : SIMethodBase(src)
{
}

SIMethodResponse::SIMethodResponse(SIMethodResponse&& src) noexcept
    : SIMethodBase(std::move(src))
{
}

SIMethodResponse::~SIMethodResponse(void)
{
}

SIMethodResponse& SIMethodResponse::operator = (const SIMethodResponse& other)
{
    if (this != &other)
    {
        SIMethodBase::operator = (other);
    }

    return *this;
}

SIMethodResponse& SIMethodResponse::operator = (SIMethodResponse&& other) noexcept
{
    if (this != &other)
    {
        SIMethodBase::operator = (std::move(other));
    }

    return *this;
}

bool SIMethodResponse::readFromXml(QXmlStreamReader& xml)
{
    QXmlStreamAttributes attributes = xml.attributes();
    if ((xml.name() == XmlSI::xmlSIElementMethod) && checkMethodType(attributes.value(XmlSI::xmlSIAttributeMethodType)))
    {
        setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
        mName = attributes.value(XmlSI::xmlSIAttributeName).toString();

        QString depValue = attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() : "";
        setIsDeprecated(depValue.compare(XmlSI::xmlSIValueTrue, Qt::CaseSensitivity::CaseInsensitive) == 0);

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

void SIMethodResponse::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementMethod);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    xml.writeAttribute(XmlSI::xmlSIAttributeMethodType, getType());
    if (getIsDeprecated())
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
        writeTextElem(xml, XmlSI::xmlSIElementDeprecateHint, getDeprecateHint(), true);
    }

    writeTextElem(xml, XmlSI::xmlSIElementDescription, mDescription, false);

    const QList<MethodParameter>& elements = getElements();
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

QIcon SIMethodResponse::getIcon(ElementBase::eDisplay display) const
{
    if (display == ElementBase::eDisplay::DisplayName)
    {
        return NELusanCommon::iconMethodResponse(NELusanCommon::SizeSmall);
    }
    else
    {
        return QIcon();
    }
}

QString SIMethodResponse::getString(ElementBase::eDisplay display) const
{
    return (display == ElementBase::eDisplay::DisplayName ? getName() : QString());
}
