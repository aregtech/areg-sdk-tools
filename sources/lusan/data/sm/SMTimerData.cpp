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
 *  \file        lusan/data/sm/SMTimerData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM timers registry.
 *
 ************************************************************************/

#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

//////////////////////////////////////////////////////////////////////////
// SMTimerEntry implementation
//////////////////////////////////////////////////////////////////////////

SMTimerEntry::SMTimerEntry(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mName         ( )
    , mTimeout      (1)
    , mRepeat       (1)
    , mDescription  ( )
{
}

SMTimerEntry::SMTimerEntry(  uint32_t id
                           , const QString& name
                           , uint32_t timeout /*= 1*/
                           , uint32_t repeat  /*= 1*/
                           , ElementBase* parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mName         (name)
    , mTimeout      (timeout)
    , mRepeat       (repeat)
    , mDescription  ( )
{
}

SMTimerEntry::SMTimerEntry(const SMTimerEntry& src)
    : DocumentElem  (src)
    , mName         (src.mName)
    , mTimeout      (src.mTimeout)
    , mRepeat       (src.mRepeat)
    , mDescription  (src.mDescription)
{
}

SMTimerEntry::SMTimerEntry(SMTimerEntry&& src) noexcept
    : DocumentElem  (std::move(src))
    , mName         (std::move(src.mName))
    , mTimeout      (src.mTimeout)
    , mRepeat       (src.mRepeat)
    , mDescription  (std::move(src.mDescription))
{
}

SMTimerEntry& SMTimerEntry::operator = (const SMTimerEntry& other)
{
    if (this != &other)
    {
        DocumentElem::operator = (other);
        mName        = other.mName;
        mTimeout     = other.mTimeout;
        mRepeat      = other.mRepeat;
        mDescription = other.mDescription;
    }

    return *this;
}

SMTimerEntry& SMTimerEntry::operator = (SMTimerEntry&& other) noexcept
{
    if (this != &other)
    {
        DocumentElem::operator = (std::move(other));
        mName        = std::move(other.mName);
        mTimeout     = other.mTimeout;
        mRepeat      = other.mRepeat;
        mDescription = std::move(other.mDescription);
    }

    return *this;
}

bool SMTimerEntry::isValid() const
{
    return (mName.isEmpty() == false) && (mTimeout >= 1);
}

QIcon SMTimerEntry::getIcon(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return NELusanCommon::iconTimer(NELusanCommon::SizeSmall);
    case ElementBase::eDisplay::DisplayType:
        return (mTimeout >= 1) ? QIcon() : NELusanCommon::iconWarning(NELusanCommon::SizeSmall);
    default:
        return QIcon();
    }
}

QString SMTimerEntry::getString(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return mName;
    case ElementBase::eDisplay::DisplayType:
        return QStringLiteral("%1 ms").arg(mTimeout);
    case ElementBase::eDisplay::DisplayValue:
        return isContinuous() ? QStringLiteral("Continuous") : QStringLiteral("%1 time(s)").arg(mRepeat);
    default:
        return QString();
    }
}

bool SMTimerEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementTimer)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    mName    = attributes.value(XmlSM::xmlSMAttributeName).toString();
    mTimeout = attributes.value(XmlSM::xmlSMAttributeTimeout).toUInt();
    mRepeat  = attributes.value(XmlSM::xmlSMAttributeRepeat).toUInt();
    mDescription.clear();

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementTimer))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementDescription)
        {
            mDescription = xml.readElementText();
        }

        xml.readNext();
    }

    return true;
}

void SMTimerEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementTimer);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSM::xmlSMAttributeName, mName);
    xml.writeAttribute(XmlSM::xmlSMAttributeTimeout, QString::number(mTimeout));
    xml.writeAttribute(XmlSM::xmlSMAttributeRepeat, QString::number(mRepeat));
    writeTextElem(xml, XmlSM::xmlSMElementDescription, mDescription, true);
    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMTimerData implementation
//////////////////////////////////////////////////////////////////////////

SMTimerData::SMTimerData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMTimerEntry, DocumentElem>(parent)
{
}

SMTimerData::SMTimerData(const QList<SMTimerEntry>& entries, ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMTimerEntry, DocumentElem>(entries, parent)
{
}

bool SMTimerData::isValid() const
{
    return true;
}

bool SMTimerData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementTimerList)
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementTimerList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementTimer)
        {
            SMTimerEntry entry(this);
            if (entry.readFromXml(xml))
            {
                addElement(std::move(entry), true);
            }
        }

        xml.readNext();
    }

    return true;
}

void SMTimerData::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementTimerList);
    for (const SMTimerEntry& entry : getElements())
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement();
}

SMTimerEntry* SMTimerData::createTimer(const QString& name)
{
    if (findElement(name) != nullptr)
    {
        return nullptr;
    }

    SMTimerEntry entry(getNextId(), name, 1, 1, this);
    addElement(std::move(entry), true);
    return findElement(name);
}
