﻿/************************************************************************
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

SIIncludeData::SIIncludeData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer< IncludeEntry, DocumentElem>(parent)
{
}

SIIncludeData::SIIncludeData(const QList<IncludeEntry>& entries, ElementBase* parent /*= nullptr*/)
    : TEDataContainer<IncludeEntry, DocumentElem>(parent)
{
    setElements(entries);
}

bool SIIncludeData::isValid() const
{
    return true;
}

bool SIIncludeData::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementIncludeList))
        return false;

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementIncludeList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSI::xmlSIElementLocation)
            {
                IncludeEntry entry(this);
                if (entry.readFromXml(xml))
                {
                    addElement(std::move(entry), true);
                }
            }
        }

        xml.readNext();
    }

    return true;
}

void SIIncludeData::writeToXml(QXmlStreamWriter& xml) const
{
    const QList<IncludeEntry> & elements = getElements();
    if (elements.size() == 0)
        return;
    
    xml.writeStartElement(XmlSI::xmlSIElementIncludeList);
    for (const IncludeEntry& entry : elements)
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement();
}

void SIIncludeData::validate(const SIDataTypeData& /*dataTypes*/)
{
}

IncludeEntry* SIIncludeData::createInclude(const QString location)
{
    IncludeEntry* result{ nullptr };
    IncludeEntry entry(getNextId(), location, this);
    if (addElement(std::move(entry), false))
    {
        result = &mElementList[mElementList.size() - 1];
    }

    return result;
}

IncludeEntry* SIIncludeData::insertInclude(int position, const QString& location)
{
    IncludeEntry* result{ nullptr };
    IncludeEntry entry(getNextId(), location, this);
    if (insertElement(position, std::move(entry), false))
    {
        result = &mElementList[position];
    }

    return result;
}
