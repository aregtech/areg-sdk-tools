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
 *  \file        lusan/data/si/SIIncludeData.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Include Data.
 *
 ************************************************************************/

#include "lusan/data/si/SIIncludeData.hpp"
#include "lusan/common/XmlSI.hpp"

const IncludeEntry SIIncludeData::InvalidInclude;

SIIncludeData::SIIncludeData(ElementBase* parent /*= nullptr*/)
    : ElementBase   (parent)
    , mIncludes     ()
{
}

SIIncludeData::SIIncludeData(const QList<IncludeEntry>& entries, ElementBase* parent /*= nullptr*/)
    : ElementBase   (parent)
    , mIncludes(entries)
{
    for (IncludeEntry& entry : mIncludes)
    {
        if (entry.getParent() != this)
        {
            entry.setParent(this);
            entry.setId(getNextId());
        }
    }
}

const QList<IncludeEntry>& SIIncludeData::getIncludes(void) const
{
    return mIncludes;
}

void SIIncludeData::setIncludes(const QList<IncludeEntry>& entries)
{
    mIncludes = entries;
    for (IncludeEntry& entry : mIncludes)
    {
        if (entry.getParent() != this)
        {
            entry.setParent(this);
            entry.setId(getNextId());
        }
    }
}

int SIIncludeData::findInclude(const IncludeEntry& entry) const
{
    return mIncludes.indexOf(entry);
}

bool SIIncludeData::addInclude(IncludeEntry&& entry, bool unique)
{
    if ((unique == false) || (exists(entry.getLocation()) == false))
    {
        if (entry.getParent() != this)
        {
            entry.setParent(this);
            entry.setId(getNextId());
        }

        mIncludes.append(std::move(entry));
        return true;
    }

    return false;
}

bool SIIncludeData::removeInclude(const IncludeEntry& entry)
{
    int index = mIncludes.indexOf(entry);
    if (index != -1)
    {
        mIncludes.removeAt(index);
        return true;
    }

    return false;
}

bool SIIncludeData::replaceInclude(const IncludeEntry& oldEntry, IncludeEntry&& newEntry)
{
    int index = mIncludes.indexOf(oldEntry);
    if (index != -1)
    {
        if (newEntry.getParent() != this)
        {
            newEntry.setParent(this);
            newEntry.setId(getNextId());
        }

        mIncludes[index] = std::move(newEntry);
        return true;
    }

    return false;
}

int SIIncludeData::findInclude(const QString& location) const
{
    int index{ -1 };
    for (int i = 0; i < mIncludes.size(); ++i)
    {
        if (mIncludes[i].getLocation() == location)
        {
            index = i;
            break;
        }
    }

    return index;
}

bool SIIncludeData::exists(const QString& location) const
{
    return findInclude(location) != -1;
}

const IncludeEntry & SIIncludeData::getInclude(const QString& location) const
{
    int index = findInclude(location);
    return (index != -1 ? mIncludes[index] : InvalidInclude);
}

bool SIIncludeData::removeInclude(const QString& location)
{
    int index = findInclude(location);
    if (index != -1)
    {
        mIncludes.removeAt(index);
        return true;
    }

    return false;
}

void SIIncludeData::sortIncludes(bool ascending)
{
    std::sort(mIncludes.begin(), mIncludes.end(), [ascending](const IncludeEntry& left, const IncludeEntry& right)
        {
            return (ascending ? left < right : left > right);
        });
}

void SIIncludeData::removeAll(void)
{
    mIncludes.clear();
}

bool SIIncludeData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSI::xmlSIElementIncludeList)
    {
        return false;
    }

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementIncludeList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSI::xmlSIElementLocation)
            {
                IncludeEntry entry(this);
                if (entry.readFromXml(xml))
                {
                    addInclude(std::move(entry), true);
                }
            }
        }

        xml.readNext();
    }

    return true;
}

void SIIncludeData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementIncludeList);
    for (const IncludeEntry& entry : mIncludes)
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement();
}
