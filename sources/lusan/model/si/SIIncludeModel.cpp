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
 *  \file        lusan/model/si/SIIncludeModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Includes Model.
 *
 ************************************************************************/
#include "lusan/model/si/SIIncludeModel.hpp"
#include "lusan/data/si/SIIncludeData.hpp"

SIIncludeModel::SIIncludeModel(SIIncludeData& includeData)
    : mIncludeData(includeData)
{
}

uint32_t SIIncludeModel::createInclude(const QString& name)
{
    IncludeEntry entry(0, name);
    return (mIncludeData.addElement(std::move(entry), true) ? entry.getId() : 0);
}

bool SIIncludeModel::deleteInclude(uint32_t id)
{
    return mIncludeData.removeElement(id);
}

const QList<IncludeEntry>& SIIncludeModel::getIncludes(void) const
{
    return mIncludeData.getElements();
}

const IncludeEntry* SIIncludeModel::findInclude(uint32_t id) const
{
    return mIncludeData.findElement(id);
}

IncludeEntry* SIIncludeModel::findInclude(uint32_t id)
{
    return mIncludeData.findElement(id);
}

void SIIncludeModel::sortInclude(bool ascending)
{
    mIncludeData.sortElementsByName(ascending);
}

