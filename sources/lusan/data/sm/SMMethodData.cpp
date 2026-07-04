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
 *  \file        lusan/data/sm/SMMethodData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM methods registry: triggers, actions, conditions
 *
 ************************************************************************/

#include "lusan/data/sm/SMMethodData.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

//////////////////////////////////////////////////////////////////////////
// SMMethodEntry static helpers
//////////////////////////////////////////////////////////////////////////

SMMethodEntry::eMethodType SMMethodEntry::fromTypeString(const QString& type)
{
    if (type.compare(SMMethodEntry::STR_TYPE_ACTION, Qt::CaseInsensitive) == 0)
        return eMethodType::Action;
    else if (type.compare(SMMethodEntry::STR_TYPE_CONDITION, Qt::CaseInsensitive) == 0)
        return eMethodType::Condition;
    else
        return eMethodType::Trigger;
}

const char* SMMethodEntry::toString(SMMethodEntry::eMethodType type)
{
    switch (type)
    {
    case eMethodType::Action:
        return SMMethodEntry::STR_TYPE_ACTION;
    case eMethodType::Condition:
        return SMMethodEntry::STR_TYPE_CONDITION;
    case eMethodType::Trigger:
    default:
        return SMMethodEntry::STR_TYPE_TRIGGER;
    }
}

SMMethodEntry::eImplement SMMethodEntry::fromImplementString(const QString& implement)
{
    return (implement.compare(SMMethodEntry::STR_IMPL_EMBEDDED, Qt::CaseInsensitive) == 0)
                ? eImplement::Embedded
                : eImplement::Handler;
}

const char* SMMethodEntry::toString(SMMethodEntry::eImplement implement)
{
    switch (implement)
    {
    case eImplement::Embedded:
        return SMMethodEntry::STR_IMPL_EMBEDDED;
    case eImplement::Handler:
    default:
        return SMMethodEntry::STR_IMPL_HANDLER;
    }
}

//////////////////////////////////////////////////////////////////////////
// SMMethodEntry implementation
//////////////////////////////////////////////////////////////////////////

SMMethodEntry::SMMethodEntry(ElementBase* parent /*= nullptr*/)
    : MethodBase    (parent)
    , mMethodType   (eMethodType::Trigger)
    , mReturn       (SMMethodEntry::DEFAULT_RETURN)
    , mImplement    (eImplement::Handler)
    , mBody         ( )
{
}

SMMethodEntry::SMMethodEntry(uint32_t id, const QString& name, eMethodType type, ElementBase* parent /*= nullptr*/)
    : MethodBase    (id, name, QString(), parent)
    , mMethodType   (type)
    , mReturn       (SMMethodEntry::DEFAULT_RETURN)
    , mImplement    (eImplement::Handler)
    , mBody         ( )
{
}

SMMethodEntry::SMMethodEntry(const SMMethodEntry& src)
    : MethodBase    (src)
    , mMethodType   (src.mMethodType)
    , mReturn       (src.mReturn)
    , mImplement    (src.mImplement)
    , mBody         (src.mBody)
{
}

SMMethodEntry::SMMethodEntry(SMMethodEntry&& src) noexcept
    : MethodBase    (std::move(src))
    , mMethodType   (src.mMethodType)
    , mReturn       (std::move(src.mReturn))
    , mImplement    (src.mImplement)
    , mBody         (std::move(src.mBody))
{
}

SMMethodEntry& SMMethodEntry::operator = (const SMMethodEntry& other)
{
    if (this != &other)
    {
        MethodBase::operator = (other);
        mMethodType = other.mMethodType;
        mReturn     = other.mReturn;
        mImplement  = other.mImplement;
        mBody       = other.mBody;
    }

    return *this;
}

SMMethodEntry& SMMethodEntry::operator = (SMMethodEntry&& other) noexcept
{
    if (this != &other)
    {
        MethodBase::operator = (std::move(other));
        mMethodType = other.mMethodType;
        mReturn     = std::move(other.mReturn);
        mImplement  = other.mImplement;
        mBody       = std::move(other.mBody);
    }

    return *this;
}

bool SMMethodEntry::isValid(void) const
{
    return (getName().isEmpty() == false);
}

bool SMMethodEntry::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMMethodEntry::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}

//////////////////////////////////////////////////////////////////////////
// SMMethodData implementation
//////////////////////////////////////////////////////////////////////////

SMMethodData::SMMethodData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMMethodEntry*, DocumentElem>(parent)
{
}

SMMethodData::~SMMethodData(void)
{
    removeAll();
}

bool SMMethodData::isValid(void) const
{
    return true;
}

bool SMMethodData::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMMethodData::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}

SMMethodEntry* SMMethodData::createMethod(const QString& name, SMMethodEntry::eMethodType type)
{
    if (findMethod(name) != nullptr)
    {
        return nullptr;
    }

    SMMethodEntry* entry = new SMMethodEntry(getNextId(), name, type, this);
    addElement(entry, true);
    return entry;
}

SMMethodEntry* SMMethodData::findMethod(const QString& name) const
{
    SMMethodEntry* const* found = findElement(name);
    return (found != nullptr) ? *found : nullptr;
}

SMMethodEntry* SMMethodData::findTrigger(const QString& name) const
{
    SMMethodEntry* method = findMethod(name);
    return ((method != nullptr) && method->isTrigger()) ? method : nullptr;
}

void SMMethodData::removeAll(void)
{
    for (SMMethodEntry* entry : getElements())
    {
        delete entry;
    }

    removeAllElements();
}
