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
 *  \file        lusan/data/common/IncludeDataSection.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the `IncludeList` section shared by every document editor.
 *
 ************************************************************************/

#include "lusan/data/common/IncludeDataSection.hpp"
#include "lusan/common/XmlSI.hpp"

IncludeDataSection::IncludeDataSection(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<IncludeEntry, DocumentElem>(parent)
{
}

IncludeDataSection::IncludeDataSection(const QList<IncludeEntry>& entries, ElementBase* parent /*= nullptr*/)
    : TEDataContainer<IncludeEntry, DocumentElem>(parent)
{
    setElements(entries);
}

bool IncludeDataSection::isValid() const
{
    return true;
}

bool IncludeDataSection::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementIncludeList))
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementIncludeList))
    {
        if ((xml.tokenType() == QXmlStreamReader::StartElement) && (xml.name() == XmlSI::xmlSIElementLocation))
        {
            IncludeEntry entry(this);
            if (entry.readFromXml(xml))
            {
                addElement(std::move(entry), true);
            }
        }

        xml.readNext();
    }

    return true;
}

void IncludeDataSection::writeToXml(QXmlStreamWriter& xml) const
{
    const QList<IncludeEntry>& elements = getElements();
    if (elements.isEmpty())
        return;

    xml.writeStartElement(XmlSI::xmlSIElementIncludeList);
    for (const IncludeEntry& entry : elements)
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement();
}

IncludeEntry* IncludeDataSection::createInclude(const QString& location)
{
    if (findElement(location) != nullptr)
        return nullptr;

    IncludeEntry entry(getNextId(), location, this);
    return (addElement(std::move(entry), true) ? findElement(location) : nullptr);
}

IncludeEntry* IncludeDataSection::insertInclude(int position, const QString& location)
{
    if (findElement(location) != nullptr)
        return nullptr;

    IncludeEntry entry(getNextId(), location, this);
    return (insertElement(position, std::move(entry), true) ? findElement(location) : nullptr);
}
