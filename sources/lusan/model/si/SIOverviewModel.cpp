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
 *  \file        lusan/model/si/SIOverviewModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview Model.
 *
 ************************************************************************/

#include "lusan/model/si/SIOverviewModel.hpp"
#include "lusan/data/si/SIOverviewData.hpp"

SIOverviewModel::SIOverviewModel(SIOverviewData& data)
    : mData(data)
{
}

uint32_t SIOverviewModel::getId() const
{
    return mData.getId();
}

void SIOverviewModel::setId(uint32_t id)
{
    mData.setId(id);
}

const QString& SIOverviewModel::getName() const
{
    return mData.getName();
}

void SIOverviewModel::setName(const QString& name)
{
    mData.setName(name);
}

const VersionNumber& SIOverviewModel::getVersion() const
{
    return mData.getVersion();
}

void SIOverviewModel::setVersion(const QString& version)
{
    mData.setVersion(version);
}

void SIOverviewModel::setVersion(const VersionNumber& version)
{
    mData.setVersion(version);
}

void SIOverviewModel::setVersion(uint32_t major, uint32_t minor, uint32_t patch)
{
    mData.setVersion(VersionNumber(major, minor, patch));
}

SIOverviewData::eCategory SIOverviewModel::getCategory() const
{
    return mData.getCategory();
}

void SIOverviewModel::setCategory(SIOverviewData::eCategory category)
{
    mData.setCategory(category);
}

const QString& SIOverviewModel::getDescription() const
{
    return mData.getDescription();
}

void SIOverviewModel::setDescription(const QString& description)
{
    mData.setDescription(description);
}

bool SIOverviewModel::isDeprecated() const
{
    return mData.isDeprecated();
}

void SIOverviewModel::setIsDeprecated(bool isDeprecated)
{
    mData.setIsDeprecated(isDeprecated);
}

const QString& SIOverviewModel::getDeprecateHint() const
{
    return mData.getDeprecateHint();
}

void SIOverviewModel::setDeprecateHint(const QString& deprecateHint)
{
    mData.setDeprecateHint(deprecateHint);
}
