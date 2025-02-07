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
    : mData(includeData)
{
}

IncludeEntry * SIIncludeModel::createInclude(const QString& location)
{
    return mData.createInclude(location);
}

IncludeEntry* SIIncludeModel::insertInclude(int position, const QString& location)
{
    return mData.insertInclude(position, location);
}

bool SIIncludeModel::deleteInclude(uint32_t id)
{
    return mData.removeElement(id);
}

const QList<IncludeEntry>& SIIncludeModel::getIncludes(void) const
{
    return mData.getElements();
}

const IncludeEntry* SIIncludeModel::findInclude(uint32_t id) const
{
    return mData.findElement(id);
}

IncludeEntry* SIIncludeModel::findInclude(uint32_t id)
{
    return mData.findElement(id);
}

void SIIncludeModel::sortInclude(bool ascending)
{
    mData.sortElementsByName(ascending);
}

void SIIncludeModel::swapIncludes(uint32_t firstId, uint32_t secondId)
{
    mData.swapElements(firstId, secondId);
}

void SIIncludeModel::swapIncludes(const IncludeEntry& first, const IncludeEntry& second)
{
    mData.swapElements(first, second);
}
