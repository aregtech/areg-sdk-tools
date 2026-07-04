/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
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
    xml.skipCurrentElement();
    return true;
}

void SMIncludeData::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
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
