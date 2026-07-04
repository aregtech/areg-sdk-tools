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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMIncludeData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM includes registry. Reuses IncludeEntry.
 *
 ************************************************************************/

#include "lusan/data/sm/SMIncludeData.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

SMIncludeData::SMIncludeData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<IncludeEntry, DocumentElem>(parent)
{
}

bool SMIncludeData::isValid(void) const
{
    return true;
}

bool SMIncludeData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementIncludeList)
        return false;

    // The `.fsml` `<Location>` shape matches `.siml` exactly, so IncludeEntry's own
    // reader/writer are reused verbatim.
    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementIncludeList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementLocation)
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

void SMIncludeData::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementIncludeList);
    for (const IncludeEntry& entry : getElements())
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement();
}

IncludeEntry* SMIncludeData::createInclude(const QString& location)
{
    if (findElement(location) != nullptr)
    {
        return nullptr;
    }

    IncludeEntry entry(getNextId(), location, this);
    addElement(std::move(entry), true);
    return findElement(location);
}
