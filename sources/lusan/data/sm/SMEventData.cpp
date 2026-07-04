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
 *  \file        lusan/data/sm/SMEventData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM events registry.
 *
 ************************************************************************/

#include "lusan/data/sm/SMEventData.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

//////////////////////////////////////////////////////////////////////////
// SMEventEntry implementation
//////////////////////////////////////////////////////////////////////////

SMEventEntry::SMEventEntry(ElementBase* parent /*= nullptr*/)
    : MethodBase(parent)
{
}

SMEventEntry::SMEventEntry(uint32_t id, const QString& name, ElementBase* parent /*= nullptr*/)
    : MethodBase(id, name, QString(), parent)
{
}

SMEventEntry::SMEventEntry(const SMEventEntry& src)
    : MethodBase(src)
{
}

SMEventEntry::SMEventEntry(SMEventEntry&& src) noexcept
    : MethodBase(std::move(src))
{
}

SMEventEntry& SMEventEntry::operator = (const SMEventEntry& other)
{
    if (this != &other)
    {
        MethodBase::operator = (other);
    }

    return *this;
}

SMEventEntry& SMEventEntry::operator = (SMEventEntry&& other) noexcept
{
    if (this != &other)
    {
        MethodBase::operator = (std::move(other));
    }

    return *this;
}

bool SMEventEntry::isValid(void) const
{
    return (getName().isEmpty() == false);
}

bool SMEventEntry::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMEventEntry::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}

//////////////////////////////////////////////////////////////////////////
// SMEventData implementation
//////////////////////////////////////////////////////////////////////////

SMEventData::SMEventData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMEventEntry*, DocumentElem>(parent)
{
}

SMEventData::~SMEventData(void)
{
    removeAll();
}

bool SMEventData::isValid(void) const
{
    return true;
}

bool SMEventData::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMEventData::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}

SMEventEntry* SMEventData::createEvent(const QString& name)
{
    if (findEvent(name) != nullptr)
    {
        return nullptr;
    }

    SMEventEntry* entry = new SMEventEntry(getNextId(), name, this);
    addElement(entry, true);
    return entry;
}

SMEventEntry* SMEventData::findEvent(const QString& name) const
{
    SMEventEntry* const* found = findElement(name);
    return (found != nullptr) ? *found : nullptr;
}

void SMEventData::removeAll(void)
{
    for (SMEventEntry* entry : getElements())
    {
        delete entry;
    }

    removeAllElements();
}
