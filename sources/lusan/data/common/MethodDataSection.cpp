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
 *  \file        lusan/data/common/MethodDataSection.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the `MethodList` section shared by every document editor.
 *
 ************************************************************************/

#include "lusan/data/common/MethodDataSection.hpp"

#include "lusan/common/XmlSI.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeDataSection.hpp"

#include <QObject>

//////////////////////////////////////////////////////////////////////////
// The two documents' method sets
//////////////////////////////////////////////////////////////////////////

const MethodConfig& NEMethod::serviceInterface()
{
    static const MethodConfig _config
    {
        QList<MethodKind>
        {
              { QStringLiteral("Request")  , QObject::tr("Request")  , QStringLiteral(":/icons/data method request")  , true , false, false, false }
            , { QStringLiteral("Response") , QObject::tr("Response") , QStringLiteral(":/icons/data method response") , false, true , false, false }
            , { QStringLiteral("Broadcast"), QObject::tr("Broadcast"), QStringLiteral(":/icons/data method broadcast"), false, false, false, false }
        }
        , QString()
        , false
        , false
    };

    return _config;
}

const MethodConfig& NEMethod::stateMachine()
{
    static const MethodConfig _config
    {
        QList<MethodKind>
        {
              { QStringLiteral("Trigger")  , QObject::tr("Trigger")  , QStringLiteral(":/icons/sm method trigger")  , false, false, false, false }
            , { QStringLiteral("Action")   , QObject::tr("Action")   , QStringLiteral(":/icons/sm method action")   , false, false, false, false }
            , { QStringLiteral("Condition"), QObject::tr("Condition"), QStringLiteral(":/icons/sm method condition"), false, false, true , true  }
        }
        , QString::fromLatin1(MethodEntry::DEFAULT_RETURN)
        , true
        , true
    };

    return _config;
}

//////////////////////////////////////////////////////////////////////////
// MethodDataSection implementation
//////////////////////////////////////////////////////////////////////////

MethodDataSection::MethodDataSection(const MethodConfig& config, ElementBase* parent /*= nullptr*/)
    : TEDataContainer<MethodEntry*, DocumentElem>(parent)
    , mConfig(config)
{
}

MethodDataSection::~MethodDataSection()
{
    removeAll();
}

bool MethodDataSection::isValid() const
{
    return true;
}

bool MethodDataSection::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementMethodList))
        return false;

    while (xml.readNextStartElement())
    {
        if (xml.name() != XmlSI::xmlSIElementMethod)
        {
            xml.skipCurrentElement();
            continue;
        }

        MethodEntry* entry = new MethodEntry(this);
        entry->setConfig(mConfig);
        if (entry->readFromXml(xml))
        {
            addElement(entry, false);
        }
        else
        {
            delete entry;
        }
    }

    return true;
}

void MethodDataSection::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSI::xmlSIElementMethodList);
    for (const MethodEntry* entry : getElements())
    {
        if (entry != nullptr)
        {
            entry->writeToXml(xml);
        }
    }

    xml.writeEndElement();
}

MethodEntry* MethodDataSection::createMethod(const QString& name, int kind)
{
    // Unique per kind, not per name, so the container's own name check is too strict here.
    if (findMethod(name, kind) != nullptr)
        return nullptr;

    MethodEntry* entry = new MethodEntry(getNextId(), name, kind, mConfig, this);
    addElement(entry, false);
    return entry;
}

MethodEntry* MethodDataSection::insertMethod(int position, const QString& name, int kind)
{
    if (findMethod(name, kind) != nullptr)
        return nullptr;

    MethodEntry* entry = new MethodEntry(getNextId(), name, kind, mConfig, this);
    insertElement(position, entry, false);
    return entry;
}

MethodEntry* MethodDataSection::findMethod(const QString& name) const
{
    MethodEntry* const* found = findElement(name);
    return (found != nullptr) ? *found : nullptr;
}

MethodEntry* MethodDataSection::findMethod(uint32_t id) const
{
    MethodEntry* const* found = findElement(id);
    return (found != nullptr) ? *found : nullptr;
}

MethodEntry* MethodDataSection::findMethod(const QString& name, int kind) const
{
    // Scanned rather than looked up by name: the name space is per kind, so the first entry
    // carrying the name may well be a different kind that happens to share it.
    for (MethodEntry* entry : getElements())
    {
        if ((entry != nullptr) && (entry->getKind() == kind) && (entry->getName() == name))
        {
            return entry;
        }
    }

    return nullptr;
}

QList<MethodEntry*> MethodDataSection::methodsOfKind(int kind) const
{
    QList<MethodEntry*> result;
    for (MethodEntry* entry : getElements())
    {
        if ((entry != nullptr) && (entry->getKind() == kind))
        {
            result.append(entry);
        }
    }

    return result;
}

void MethodDataSection::validate(const DataTypeDataSection& dataTypes)
{
    const QList<DataTypeCustom*>& customTypes = dataTypes.getResolutionTypes();
    for (MethodEntry* entry : getElements())
    {
        if (entry != nullptr)
        {
            entry->validate(customTypes);
        }
    }
}

QList<uint32_t> MethodDataSection::replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType)
{
    QList<uint32_t> result;
    for (MethodEntry* entry : getElements())
    {
        if (entry == nullptr)
            continue;

        bool changed{ false };
        for (MethodParameter& param : entry->getElements())
        {
            if (param.getParamType() == oldDataType)
            {
                param.setParamType(newDataType);
                changed = true;
            }
        }

        if (changed)
        {
            result.push_back(entry->getId());
        }
    }

    return result;
}

void MethodDataSection::removeAll()
{
    for (MethodEntry* entry : getElements())
    {
        delete entry;
    }

    removeAllElements();
}
