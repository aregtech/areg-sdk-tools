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
 *  \file        lusan/data/sm/SMAttributeData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM machine attributes registry.
 *
 ************************************************************************/

#include "lusan/data/sm/SMAttributeData.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

//////////////////////////////////////////////////////////////////////////
// SMAttributeEntry implementation
//////////////////////////////////////////////////////////////////////////

SMAttributeEntry::SMAttributeEntry(ElementBase* parent /*= nullptr*/)
    : ParamBase (parent)
    , mValue    ( )
{
}

SMAttributeEntry::SMAttributeEntry(  uint32_t id
                                   , const QString& name
                                   , const QString& type    /*= "bool"*/
                                   , const QString& value   /*= QString()*/
                                   , ElementBase* parent     /*= nullptr*/)
    : ParamBase (id, name, type, parent)
    , mValue    (value)
{
}

SMAttributeEntry::SMAttributeEntry(const SMAttributeEntry& src)
    : ParamBase (src)
    , mValue    (src.mValue)
{
}

SMAttributeEntry::SMAttributeEntry(SMAttributeEntry&& src) noexcept
    : ParamBase (std::move(src))
    , mValue    (std::move(src.mValue))
{
}

SMAttributeEntry& SMAttributeEntry::operator = (const SMAttributeEntry& other)
{
    if (this != &other)
    {
        ParamBase::operator = (other);
        mValue = other.mValue;
    }

    return *this;
}

SMAttributeEntry& SMAttributeEntry::operator = (SMAttributeEntry&& other) noexcept
{
    if (this != &other)
    {
        ParamBase::operator = (std::move(other));
        mValue = std::move(other.mValue);
    }

    return *this;
}

bool SMAttributeEntry::isValid(void) const
{
    return (getName().isEmpty() == false) && (getType().isEmpty() == false);
}

bool SMAttributeEntry::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMAttributeEntry::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}

//////////////////////////////////////////////////////////////////////////
// SMAttributeData implementation
//////////////////////////////////////////////////////////////////////////

SMAttributeData::SMAttributeData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMAttributeEntry, DocumentElem>(parent)
{
}

SMAttributeData::SMAttributeData(const QList<SMAttributeEntry>& entries, ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMAttributeEntry, DocumentElem>(entries, parent)
{
}

bool SMAttributeData::isValid(void) const
{
    return true;
}

bool SMAttributeData::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMAttributeData::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}

SMAttributeEntry* SMAttributeData::createAttribute(const QString& name)
{
    if (findElement(name) != nullptr)
    {
        return nullptr;
    }

    SMAttributeEntry entry(getNextId(), name, "bool", QString(), this);
    addElement(std::move(entry), true);
    return findElement(name);
}
