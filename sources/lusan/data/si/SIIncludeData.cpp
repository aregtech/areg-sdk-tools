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

const SIIncludeEntry SIIncludeData::InvalidInclude;

SIIncludeData::SIIncludeData(void)
{
}

SIIncludeData::SIIncludeData(const QList<SIIncludeEntry>& includes)
    : mIncludes(includes)
{
}

const QList<SIIncludeEntry>& SIIncludeData::getIncludes(void) const
{
    return mIncludes;
}

void SIIncludeData::setIncludes(const QList<SIIncludeEntry>& includes)
{
    mIncludes = includes;
}

int SIIncludeData::findInclude(const SIIncludeEntry& include) const
{
    return mIncludes.indexOf(include);
}

void SIIncludeData::addInclude(const SIIncludeEntry& include)
{
    if (!mIncludes.contains(include))
    {
        mIncludes.append(include);
    }
}

bool SIIncludeData::removeInclude(const SIIncludeEntry& include)
{
    int index = mIncludes.indexOf(include);
    if (index != -1)
    {
        mIncludes.removeAt(index);
        return true;
    }

    return false;
}

bool SIIncludeData::replaceInclude(const SIIncludeEntry& oldInclude, const SIIncludeEntry& newInclude)
{
    int index = mIncludes.indexOf(oldInclude);
    if (index != -1)
    {
        mIncludes[index] = newInclude;
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

const SIIncludeEntry & SIIncludeData::getInclude(const QString& location) const
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
                SIIncludeEntry entry;
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
    for (const SIIncludeEntry& include : mIncludes)
    {
        include.writeToXml(xml);
    }

    xml.writeEndElement();
}
