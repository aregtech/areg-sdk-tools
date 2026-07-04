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
 *  \file        lusan/data/sm/SMConstantData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM constants registry. Reuses ConstantEntry.
 *
 ************************************************************************/

#include "lusan/data/sm/SMConstantData.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

SMConstantData::SMConstantData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<ConstantEntry, DocumentElem>(parent)
{
}

bool SMConstantData::isValid(void) const
{
    return true;
}

bool SMConstantData::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMConstantData::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}

ConstantEntry* SMConstantData::createConstant(const QString& name)
{
    if (findElement(name) != nullptr)
    {
        return nullptr;
    }

    ConstantEntry entry(getNextId(), name, this);
    addElement(std::move(entry), true);
    return findElement(name);
}
