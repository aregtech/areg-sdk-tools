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
 *  \file        lusan/data/sm/SMEventData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM events registry.
 *
 ************************************************************************/

#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace
{
    //!< Writes a `.fsml` `ParamList` for a method/event; the default value is carried as
    //!< the `Default` attribute (not the `.siml` `<Value>` child). Empty lists are omitted.
    void writeParamList(QXmlStreamWriter& xml, const MethodBase& method)
    {
        const QList<MethodParameter>& params = method.getElements();
        if (params.isEmpty())
            return;

        xml.writeStartElement(XmlSM::xmlSMElementParamList);
        for (const MethodParameter& param : params)
        {
            xml.writeStartElement(XmlSM::xmlSMElementParameter);
            xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(param.getId()));
            xml.writeAttribute(XmlSM::xmlSMAttributeName, param.getName());
            xml.writeAttribute(XmlSM::xmlSMAttributeDataType, param.getType());
            if (param.hasDefault())
            {
                xml.writeAttribute(XmlSM::xmlSMAttributeDefault, param.getValue());
            }
            if (param.getDescription().isEmpty() == false)
            {
                xml.writeTextElement(XmlSM::xmlSMElementDescription, param.getDescription());
            }
            xml.writeEndElement();
        }

        xml.writeEndElement();
    }

    //!< Reads a `.fsml` `ParamList` (reader positioned on its start element) into \p method.
    void readParamList(QXmlStreamReader& xml, MethodBase& method)
    {
        while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementParamList))
        {
            if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementParameter)
            {
                MethodParameter param(&method);
                param.setParent(&method);   // MethodParameter's ctor does not forward the parent
                QXmlStreamAttributes attributes = xml.attributes();
                param.setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
                param.setName(attributes.value(XmlSM::xmlSMAttributeName).toString());
                param.setType(attributes.value(XmlSM::xmlSMAttributeDataType).toString());
                if (attributes.hasAttribute(XmlSM::xmlSMAttributeDefault))
                {
                    param.setValue(attributes.value(XmlSM::xmlSMAttributeDefault).toString());
                    param.setDefault(true);
                }

                while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementParameter))
                {
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementDescription)
                    {
                        param.setDescription(xml.readElementText());
                    }

                    xml.readNext();
                }

                method.addElement(std::move(param), true);
            }

            xml.readNext();
        }
    }
}

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

bool SMEventEntry::isValid() const
{
    return (getName().isEmpty() == false);
}

bool SMEventEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementEvent)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    setName(attributes.value(XmlSM::xmlSMAttributeName).toString());
    setDescription(QString());

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementEvent))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSM::xmlSMElementDescription)
            {
                setDescription(xml.readElementText());
            }
            else if (xml.name() == XmlSM::xmlSMElementParamList)
            {
                readParamList(xml, *this);
            }
        }

        xml.readNext();
    }

    return true;
}

void SMEventEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementEvent);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSM::xmlSMAttributeName, getName());
    writeTextElem(xml, XmlSM::xmlSMElementDescription, getDescription(), true);
    writeParamList(xml, *this);
    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMEventData implementation
//////////////////////////////////////////////////////////////////////////

SMEventData::SMEventData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMEventEntry*, DocumentElem>(parent)
{
}

SMEventData::~SMEventData()
{
    removeAll();
}

bool SMEventData::isValid() const
{
    return true;
}

bool SMEventData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementEventList)
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementEventList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementEvent)
        {
            SMEventEntry* entry = new SMEventEntry(this);
            if (entry->readFromXml(xml))
            {
                addElement(entry, true);
            }
            else
            {
                delete entry;
            }
        }

        xml.readNext();
    }

    return true;
}

void SMEventData::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementEventList);
    for (const SMEventEntry* entry : getElements())
    {
        entry->writeToXml(xml);
    }

    xml.writeEndElement();
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

void SMEventData::removeAll()
{
    for (SMEventEntry* entry : getElements())
    {
        delete entry;
    }

    removeAllElements();
}
