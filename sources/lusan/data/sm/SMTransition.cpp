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
 *  \file        lusan/data/sm/SMTransition.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transitions.
 *
 ************************************************************************/

#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

//////////////////////////////////////////////////////////////////////////
// SMTransitionEntry static helpers
//////////////////////////////////////////////////////////////////////////

SMTransitionEntry::eStimulusKind SMTransitionEntry::fromKindString(const QString& kind)
{
    if (kind.compare(STR_KIND_EVENT, Qt::CaseInsensitive) == 0)
        return eStimulusKind::Event;
    else if (kind.compare(STR_KIND_TIMER, Qt::CaseInsensitive) == 0)
        return eStimulusKind::Timer;
    else
        return eStimulusKind::Trigger;
}

const char* SMTransitionEntry::toString(SMTransitionEntry::eStimulusKind kind)
{
    switch (kind)
    {
    case eStimulusKind::Event:  return STR_KIND_EVENT;
    case eStimulusKind::Timer:  return STR_KIND_TIMER;
    case eStimulusKind::Trigger:
    default:                    return STR_KIND_TRIGGER;
    }
}

//////////////////////////////////////////////////////////////////////////
// SMTransitionEntry implementation
//////////////////////////////////////////////////////////////////////////

SMTransitionEntry::SMTransitionEntry(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mStimulusKind (eStimulusKind::Trigger)
    , mStimulus     ( )
    , mTo           ( )
    , mHasTo        (false)
    , mDescription  ( )
    , mConditions   (this)
    , mOperations   (this)
{
}

SMTransitionEntry::SMTransitionEntry(  uint32_t id
                                     , eStimulusKind kind
                                     , const QString& stimulus
                                     , ElementBase* parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mStimulusKind (kind)
    , mStimulus     (stimulus)
    , mTo           ( )
    , mHasTo        (false)
    , mDescription  ( )
    , mConditions   (this)
    , mOperations   (this)
{
}

SMTransitionEntry::SMTransitionEntry(const SMTransitionEntry& src)
    : DocumentElem  (src)
    , mStimulusKind (src.mStimulusKind)
    , mStimulus     (src.mStimulus)
    , mTo           (src.mTo)
    , mHasTo        (src.mHasTo)
    , mDescription  (src.mDescription)
    , mConditions   (src.mConditions)
    , mOperations   (src.mOperations)
{
    mConditions.setParent(this);
    mOperations.setParent(this);
}

SMTransitionEntry::SMTransitionEntry(SMTransitionEntry&& src) noexcept
    : DocumentElem  (std::move(src))
    , mStimulusKind (src.mStimulusKind)
    , mStimulus     (std::move(src.mStimulus))
    , mTo           (std::move(src.mTo))
    , mHasTo        (src.mHasTo)
    , mDescription  (std::move(src.mDescription))
    , mConditions   (std::move(src.mConditions))
    , mOperations   (std::move(src.mOperations))
{
    mConditions.setParent(this);
    mOperations.setParent(this);
}

SMTransitionEntry& SMTransitionEntry::operator = (const SMTransitionEntry& other)
{
    if (this != &other)
    {
        DocumentElem::operator = (other);
        mStimulusKind = other.mStimulusKind;
        mStimulus     = other.mStimulus;
        mTo           = other.mTo;
        mHasTo        = other.mHasTo;
        mDescription  = other.mDescription;
        mConditions   = other.mConditions;
        mOperations   = other.mOperations;
        mConditions.setParent(this);
        mOperations.setParent(this);
    }

    return *this;
}

SMTransitionEntry& SMTransitionEntry::operator = (SMTransitionEntry&& other) noexcept
{
    if (this != &other)
    {
        DocumentElem::operator = (std::move(other));
        mStimulusKind = other.mStimulusKind;
        mStimulus     = std::move(other.mStimulus);
        mTo           = std::move(other.mTo);
        mHasTo        = other.mHasTo;
        mDescription  = std::move(other.mDescription);
        mConditions   = std::move(other.mConditions);
        mOperations   = std::move(other.mOperations);
        mConditions.setParent(this);
        mOperations.setParent(this);
    }

    return *this;
}

bool SMTransitionEntry::isValid(void) const
{
    return (mStimulus.isEmpty() == false);
}

bool SMTransitionEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementTransition)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    mStimulusKind = fromKindString(attributes.value(XmlSM::xmlSMAttributeStimulusKind).toString());
    mStimulus     = attributes.value(XmlSM::xmlSMAttributeStimulus).toString();
    if (attributes.hasAttribute(XmlSM::xmlSMAttributeTo))
    {
        setTo(attributes.value(XmlSM::xmlSMAttributeTo).toString());
    }
    else
    {
        clearTo();
    }
    mDescription.clear();

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementTransition))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSM::xmlSMElementDescription)
            {
                mDescription = xml.readElementText();
            }
            else if (xml.name() == XmlSM::xmlSMElementConditionList)
            {
                mConditions.readFromXml(xml);
            }
            else if (xml.name() == XmlSM::xmlSMElementOperationList)
            {
                mOperations.readFromXml(xml, XmlSM::xmlSMElementOperationList);
            }
        }

        xml.readNext();
    }

    return true;
}

void SMTransitionEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementTransition);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSM::xmlSMAttributeStimulusKind, SMTransitionEntry::toString(mStimulusKind));
    xml.writeAttribute(XmlSM::xmlSMAttributeStimulus, mStimulus);
    if (mHasTo)
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeTo, mTo);
    }

    writeTextElem(xml, XmlSM::xmlSMElementDescription, mDescription, true);
    mConditions.writeToXml(xml);
    mOperations.writeToXml(xml, XmlSM::xmlSMElementOperationList);

    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMTransitionData implementation
//////////////////////////////////////////////////////////////////////////

SMTransitionData::SMTransitionData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMTransitionEntry*, DocumentElem>(parent)
{
}

SMTransitionData::SMTransitionData(const SMTransitionData& src)
    : TEDataContainer<SMTransitionEntry*, DocumentElem>(src.getParent())
{
    cloneFrom(src);
}

SMTransitionData::SMTransitionData(SMTransitionData&& src) noexcept
    : TEDataContainer<SMTransitionEntry*, DocumentElem>(std::move(src))
{
}

SMTransitionData::~SMTransitionData(void)
{
    removeAll();
}

SMTransitionData& SMTransitionData::operator = (const SMTransitionData& other)
{
    if (this != &other)
    {
        removeAll();
        setParent(other.getParent());
        cloneFrom(other);
    }

    return *this;
}

SMTransitionData& SMTransitionData::operator = (SMTransitionData&& other) noexcept
{
    if (this != &other)
    {
        removeAll();
        TEDataContainer<SMTransitionEntry*, DocumentElem>::operator = (std::move(other));
    }

    return *this;
}

void SMTransitionData::cloneFrom(const SMTransitionData& src)
{
    for (const SMTransitionEntry* entry : src.getElements())
    {
        SMTransitionEntry* copy = new SMTransitionEntry(*entry);
        copy->setParent(this);
        addElement(copy, false);
    }
}

SMTransitionEntry* SMTransitionData::createTransition(SMTransitionEntry::eStimulusKind kind, const QString& stimulus, const QString& target /*= QString()*/)
{
    SMTransitionEntry* entry = new SMTransitionEntry(getNextId(), kind, stimulus, this);
    if (target.isEmpty() == false)
    {
        entry->setTo(target);
    }

    addElement(entry, false);
    return entry;
}

void SMTransitionData::removeAll(void)
{
    for (SMTransitionEntry* entry : getElements())
    {
        delete entry;
    }

    removeAllElements();
}

bool SMTransitionData::isValid(void) const
{
    return true;
}

bool SMTransitionData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementTransitionList)
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementTransitionList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementTransition)
        {
            SMTransitionEntry* entry = new SMTransitionEntry(this);
            if (entry->readFromXml(xml))
            {
                addElement(entry, false);
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

void SMTransitionData::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementTransitionList);
    for (const SMTransitionEntry* entry : getElements())
    {
        entry->writeToXml(xml);
    }

    xml.writeEndElement();
}
