/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/si/SIOverviewData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview Data.
 *
 ************************************************************************/

#include "lusan/data/si/SIOverviewData.hpp"
#include "lusan/common/XmlSI.hpp"
#include "lusan/data/si/ServiceInterfaceData.hpp"

namespace
{
    //!< A service interface writes the description element even when it holds no text.
    constexpr bool OMIT_EMPTY_DESCRIPTION{ false };
}

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
    : OverviewDataSection   (OMIT_EMPTY_DESCRIPTION, parent)
    , mCategory             (eCategory::InterfacePrivate)
{
    setName(QStringLiteral("NewServiceInterface"));
}

SIOverviewData::SIOverviewData(uint32_t id, const QString& name, ElementBase* parent)
    : OverviewDataSection   (id, name, OMIT_EMPTY_DESCRIPTION, parent)
    , mCategory             (eCategory::InterfacePrivate)
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
    : OverviewDataSection   (id, name, OMIT_EMPTY_DESCRIPTION, parent)
    , mCategory             (category)
{
    setVersion(version);
    setDescription(description);
    setIsDeprecated(isDeprecated);
    setDeprecateHint(deprecateHint);
}

void SIOverviewData::readOwnAttributes(const QXmlStreamAttributes& attributes)
{
    const QString categoryStr = attributes.hasAttribute(XmlSI::xmlSIAttributeCategory)
                                    ? attributes.value(XmlSI::xmlSIAttributeCategory).toString()
                                    : QString(STR_CATEGORY_PRIVATE);
    mCategory = SIOverviewData::fromString(categoryStr);

    const ServiceInterfaceData* siData = static_cast<const ServiceInterfaceData*>(getParent());
    Q_ASSERT(siData != nullptr);
    const VersionNumber& dataVersion = siData->getCurrentDocumentVersion();
    if (dataVersion == VersionNumber(ServiceInterfaceData::XML_VERRSION_100))
    {
        // The first published format said `isRemote` instead of naming a category.
        const QString isRemote = attributes.hasAttribute("isRemote") ? attributes.value("isRemote").toString() : QString("false");
        mCategory = isRemote.compare("true", Qt::CaseSensitivity::CaseInsensitive) == 0 ? eCategory::InterfacePublic : eCategory::InterfacePrivate;
    }
}

void SIOverviewData::writeOwnAttributes(QXmlStreamWriter& xml) const
{
    xml.writeAttribute(XmlSI::xmlSIAttributeCategory, SIOverviewData::toString(mCategory));
}

void SIOverviewData::validate(const DataTypeDataSection& /*dataTypes*/)
{
}
