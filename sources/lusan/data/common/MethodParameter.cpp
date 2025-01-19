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
 *  \file        lusan/data/common/MethodParameter.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Method Parameter.
 *
 ************************************************************************/

#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/common/XmlSI.hpp"

MethodParameter::MethodParameter(ElementBase* parent /*= nullptr*/)
    : ParamBase ( )
    , mValue    ( )
    , mIsDefault(false)
{
}

MethodParameter::MethodParameter( uint32_t id
                                , const QString& name
                                , const QString& type   /*= "bool"*/
                                , const QString& value  /*= ""*/
                                , bool isDefault        /*= false*/
                                , ElementBase* parent   /*= nullptr*/)
    : ParamBase (id, name, type, false, QString(), QString(), parent)
    , mValue    (value)
    , mIsDefault(isDefault)
{
}

MethodParameter::MethodParameter( uint32_t id
                                , const QString& name
                                , const QString& type
                                , bool isDeprecated
                                , const QString& description
                                , const QString& deprecateHint
                                , const QString& value
                                , bool isDefault
                                , ElementBase* parent /*= nullptr*/)
    : ParamBase (id, name, type, isDeprecated, description, deprecateHint, parent)
    , mValue    (value)
    , mIsDefault(isDefault)
{
}

MethodParameter::MethodParameter(const MethodParameter& src)
    : ParamBase (src)
    , mValue    (src.mValue)
    , mIsDefault(src.mIsDefault)
{
}

MethodParameter::MethodParameter(MethodParameter&& src) noexcept
    : ParamBase (std::move(src))
    , mValue    (std::move(src.mValue))
    , mIsDefault(src.mIsDefault)
{
}

MethodParameter& MethodParameter::operator = (const MethodParameter& other)
{
    if (this != &other)
    {
        ParamBase::operator = (other);
        mValue = other.mValue;
        mIsDefault = other.mIsDefault;
    }

    return *this;
}

MethodParameter& MethodParameter::operator = (MethodParameter&& other) noexcept
{
    if (this != &other)
    {
        ParamBase::operator = (std::move(other));
        mValue      = std::move(other.mValue);
        mIsDefault  = other.mIsDefault;
    }

    return *this;
}

const QString& MethodParameter::getValue() const
{
    return mValue;
}

void MethodParameter::setValue(const QString& value)
{
    mValue = value;
}

bool MethodParameter::hasDefault() const
{
    return mIsDefault;
}

void MethodParameter::setDefault(bool isDefault)
{
    mIsDefault = isDefault;
}

bool MethodParameter::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() == XmlSI::xmlSIElementParameter)
    {
        setId(xml.attributes().value(XmlSI::xmlSIAttributeID).toUInt());
        mType = xml.attributes().value(XmlSI::xmlSIAttributeDataType).toString();
        mName = xml.attributes().value(XmlSI::xmlSIAttributeName).toString();

        while (xml.readNextStartElement())
        {
            if (xml.name() == XmlSI::xmlSIElementValue)
            {
                mIsDefault  = xml.attributes().value(XmlSI::xmlSIAttributeIsDefault).toString() == XmlSI::xmlSIValueTrue;
                mValue      = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementDescription)
            {
                mDescription = xml.readElementText();
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

void MethodParameter::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementParameter);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeDataType, mType);
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);

    if (!mValue.isEmpty())
    {
        xml.writeStartElement(XmlSI::xmlSIElementValue);
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDefault, mIsDefault ? XmlSI::xmlSIValueTrue : XmlSI::xmlSIValueFalse);
        xml.writeCharacters(mValue);
        xml.writeEndElement();
    }

    if (!mDescription.isEmpty())
    {
        xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
    }

    xml.writeEndElement();
}
