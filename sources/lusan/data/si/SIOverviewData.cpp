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
 *  \file        lusan/data/si/SIOverviewData.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview Data.
 *
 ************************************************************************/

#include "lusan/data/si/SIOverviewData.hpp"
#include "lusan/common/XmlSI.hpp"
#include "lusan/data/si/ServiceInterfaceData.hpp"

SIOverviewData::eCategory SIOverviewData::fromString(const QString& category)
{
    if (category.compare(STR_CATEGORY_PRIVATE, Qt::CaseSensitivity::CaseInsensitive) == 0)
    {
        return eCategory::InterfacePrivate;
    }
    else if (category.compare(STR_CATEGORY_PUBLIC, Qt::CaseSensitivity::CaseInsensitive) == 0)
    {
        return eCategory::InterfacePublic;
    }
    else if (category.compare(STR_CATEGORY_INTERNET, Qt::CaseSensitivity::CaseInsensitive) == 0)
    {
        return eCategory::InterfaceInternet;
    }
    else
    {
        return eCategory::InterfacePrivate;
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
        return STR_CATEGORY_PRIVATE;
    }
}

SIOverviewData::SIOverviewData(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mName         ("NewServiceInterface")
    , mVersion      (0, 0, 1)
    , mCategory     (eCategory::InterfacePrivate)
    , mDescription  ()
    , mIsDeprecated (false)
    , mDeprecateHint()
{
}

SIOverviewData::SIOverviewData(uint32_t id, const QString& name, ElementBase* parent)
    : DocumentElem  (id, parent)
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
    : DocumentElem  (id, parent)
    , mName         (name)
    , mVersion      (version)
    , mCategory     (category)
    , mDescription  (description)
    , mIsDeprecated (isDeprecated)
    , mDeprecateHint(deprecateHint)
{
}

bool SIOverviewData::isValid() const
{
    return true;
}
    
bool SIOverviewData::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementOverview))
        return false;

    ServiceInterfaceData* siData = static_cast<ServiceInterfaceData*>(getParent());
    Q_ASSERT(siData != nullptr);
    const VersionNumber& dataVersion = siData->getCurrentDocumentVersion();
    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
    mVersion = attributes.value(XmlSI::xmlSIAttributeVersion).toString();
    QString categoryStr = attributes.hasAttribute(XmlSI::xmlSIAttributeCategory) ? attributes.value(XmlSI::xmlSIAttributeCategory).toString() : STR_CATEGORY_PRIVATE;
    mCategory = SIOverviewData::fromString(categoryStr);
    
    QString depValue = attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value(XmlSI::xmlSIAttributeIsDeprecated).toString() : "";
    setIsDeprecated(depValue.compare(XmlSI::xmlSIValueTrue, Qt::CaseSensitivity::CaseInsensitive) == 0);

    if (dataVersion != VersionNumber(ServiceInterfaceData::XML_FORMAT_DEFAULT))
    {
        if (dataVersion == VersionNumber(ServiceInterfaceData::XML_VERRSION_100))
        {
            QString isRemove = attributes.hasAttribute("isRemote") ? attributes.value("isRemote").toString() : "false";
            mCategory = isRemove.compare("true", Qt::CaseSensitivity::CaseInsensitive) == 0 ? eCategory::InterfacePublic : eCategory::InterfacePrivate;
        }
    }

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
        writeTextElem(xml, XmlSI::xmlSIElementDeprecateHint, getDeprecateHint(), true);
    }

    writeTextElem(xml, XmlSI::xmlSIElementDescription, mDescription, false);
    xml.writeEndElement();
}

void SIOverviewData::validate(const SIDataTypeData& /*dataTypes*/)
{
}
