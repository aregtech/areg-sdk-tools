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
 *  \file        lusan/data/si/SIOverviewData.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview Data.
 *
 ************************************************************************/

#include "lusan/data/si/SIOverviewData.hpp"
#include "lusan/common/XmlSI.hpp"

SIOverviewData::eCategory SIOverviewData::fromString(const QString& category)
{
    if (category == STR_CATEGORY_PRIVATE)
    {
        return eCategory::InterfacePrivate;
    }
    else if (category == STR_CATEGORY_PUBLIC)
    {
        return eCategory::InterfacePublic;
    }
    else if (category == STR_CATEGORY_INTERNET)
    {
        return eCategory::InterfaceInternet;
    }
    else
    {
        return eCategory::InterfaceUnknown;
    }
}

const char* SIOverviewData::toString(SIOverviewData::eCategory category)
{
    switch (category)
    {
    case eCategory::InterfacePrivate:
        return STR_CATEGORY_PRIVATE;
    case eCategory::InterfacePublic:
        return STR_CATEGORY_PUBLIC;
    case eCategory::InterfaceInternet:
        return STR_CATEGORY_INTERNET;
    default:
        return STR_CATEGORY_UNKNOWN;
    }
}

SIOverviewData::SIOverviewData(ElementBase* parent /*= nullptr*/)
    : ElementBase   (parent)
    , mName         ("NewServiceInterface")
    , mVersion      (0, 0, 1)
    , mCategory     (eCategory::InterfacePrivate)
    , mDescription  ()
    , mIsDeprecated (false)
    , mDeprecateHint()
{
}

SIOverviewData::SIOverviewData(uint32_t id, const QString& name, ElementBase* parent)
    : ElementBase(id, parent)
    , mName         (name)
    , mVersion      (0, 0, 1)
    , mCategory     (eCategory::InterfacePrivate)
    , mDescription  ()
    , mIsDeprecated (false)
    , mDeprecateHint()
{
}

SIOverviewData::SIOverviewData( uint32_t id
                              , const QString& name
                              , const QString& version
                              , eCategory category
                              , const QString& description
                              , bool isDeprecated
                              , const QString& deprecateHint
                              , ElementBase* parent /*= nullptr*/)
    : ElementBase   (id, parent)
    , mName         (name)
    , mVersion      (version)
    , mCategory     (category)
    , mDescription  (description)
    , mIsDeprecated (isDeprecated)
    , mDeprecateHint(deprecateHint)
{
}

bool SIOverviewData::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementOverview))
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
    mVersion = attributes.value(XmlSI::xmlSIAttributeVersion).toString();
    QString categoryStr = attributes.value(XmlSI::xmlSIAttributeCategory).toString();
    mCategory = SIOverviewData::fromString(categoryStr);
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
        else
        {
            xml.skipCurrentElement();
        }
    }

    return true;
}

void SIOverviewData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementOverview);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);
    xml.writeAttribute(XmlSI::xmlSIAttributeVersion, mVersion.toString());
    xml.writeAttribute(XmlSI::xmlSIAttributeCategory, SIOverviewData::toString(mCategory));
    if (getIsDeprecated())
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
    }

    xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
    if (getIsDeprecated())
    {
        xml.writeTextElement(XmlSI::xmlSIElementDeprecateHint, getDeprecateHint());
    }

    xml.writeEndElement();
}
