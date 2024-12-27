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

SIIncludeData::SIIncludeData(void)
{
}

SIIncludeData::SIIncludeData(const QList<IncludeEntry>& entries)
    : mIncludes(entries)
{
}

const QList<IncludeEntry>& SIIncludeData::getIncludes(void) const
{
    return mIncludes;
}

void SIIncludeData::setIncludes(const QList<IncludeEntry>& entries)
{
    mIncludes = entries;
}

int SIIncludeData::findInclude(const IncludeEntry& entry) const
{
    return mIncludes.indexOf(entry);
}

void SIIncludeData::addInclude(const IncludeEntry& entry)
{
    if (!mIncludes.contains(entry))
    {
        mIncludes.append(entry);
    }
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

bool SIIncludeData::replaceInclude(const IncludeEntry& oldEntry, const IncludeEntry& newEntry)
{
    int index = mIncludes.indexOf(oldEntry);
    if (index != -1)
    {
        mIncludes[index] = newEntry;
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

void SIIncludeData::addInclude(const IncludeEntry& elem, bool ascending)
{
    if (mIncludes.contains(elem) == false)
    {
        mIncludes.append(elem);
        sortIncludes(ascending);
    }
}

void SIIncludeData::sortIncludes(bool ascending)
{
    std::sort(mIncludes.begin(), mIncludes.end(), [ascending](const IncludeEntry& left, const IncludeEntry& right)
        {
            return (ascending ? left < right : left > right);
        });
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
                IncludeEntry entry;
                if (entry.readFromXml(xml))
                {
                    addInclude(entry);
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
