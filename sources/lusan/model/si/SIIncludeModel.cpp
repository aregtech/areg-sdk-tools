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
    , mEntries(mIncludeData.getIncludes())
{
}

int SIIncludeModel::rowCount() const
{
    return mEntries.size();
}

int SIIncludeModel::columnCount() const
{
    return 1; // Single column model
}

const IncludeEntry * SIIncludeModel::data(int row) const
{
    
    if ((row >= 0) && (row < static_cast<int>(mEntries.size())))
    {
        return &mEntries.at(row);
    }
    
    return nullptr;
}

const IncludeEntry* SIIncludeModel::data(const QString& location) const
{
    int index = findEntry(location);
    return (index != -1 ? &mEntries[index] : nullptr);
}

bool SIIncludeModel::addEntry(const QString& location, const QString& description, bool isDeprecated, const QString& deprecateHint)
{
    int index = findEntry(location);
    if (index != -1)
        return false;

    IncludeEntry newEntry(location, mEntries.size() + 1, description, isDeprecated, deprecateHint);
    mEntries.append(newEntry);
    return true;
}

bool SIIncludeModel::updateEntry(int index, const QString& location, const QString& description, bool isDeprecated, const QString& deprecateHint)
{
    if (index < 0 || index >= mEntries.size())
        return false;

    IncludeEntry& entry = mEntries[index];
    entry.setLocation(location);
    entry.setDescription(description);
    entry.setDeprecated(isDeprecated);
    entry.setDeprecationHint(deprecateHint);
    return true;
}

bool SIIncludeModel::updateEntry(const QString& oldLocation, const QString& newLocation, const QString& description, bool isDeprecated, const QString& deprecateHint)
{
    int index = findEntry(oldLocation);
    if (index == -1)
        return false;

    return updateEntry(index, newLocation, description, isDeprecated, deprecateHint);
}

bool SIIncludeModel::removeEntry(const QString& location)
{
    int index = findEntry(location);
    if (index == -1)
        return false;
    
    mEntries.removeAt(index);
    return true;
}

bool SIIncludeModel::removeEntry(int index)
{
    if (index < 0 || index >= mEntries.size())
        return false;

    mEntries.removeAt(index);
    return true;
}

bool SIIncludeModel::insertEntry(int index, const QString& location, const QString& description, bool isDeprecated, const QString& deprecateHint)
{
    if (index < 0 || index > mEntries.size())
        return false;

    int found = findEntry(location);
    if (found != -1)
        return false;

    IncludeEntry newEntry(location, index + 1, description, isDeprecated, deprecateHint);
    mEntries.insert(index, newEntry);
    return true;
}

bool SIIncludeModel::insertEntry(const QString& beforeLocation, const QString& location, const QString& description, bool isDeprecated, const QString& deprecateHint)
{
    int index = findEntry(beforeLocation);
    if (index == -1)
        return false;
    
    return insertEntry(index, location, description, isDeprecated, deprecateHint);
}

void SIIncludeModel::sortEntries(bool ascending)
{
    std::sort(mEntries.begin(), mEntries.end(), [ascending](const IncludeEntry& a, const IncludeEntry& b) {
        return ascending ? a.getLocation() < b.getLocation() : a.getLocation() > b.getLocation();
        });
}

int SIIncludeModel::findEntry(const QString& location) const
{
    for (int i = 0; i < mEntries.size(); ++i)
    {
        if (mEntries[i].getLocation() == location)
            return i;
    }
    
    return -1;
}

void SIIncludeModel::updateModel(void)
{
    mEntries = mIncludeData.getIncludes();
}

void SIIncludeModel::updateData(void)
{
    mIncludeData.setIncludes(mEntries);
}
