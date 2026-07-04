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
 *  \file        lusan/data/sm/SMState.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM states and the recursive state list
 *
 ************************************************************************/

#include "lusan/data/sm/SMState.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

//////////////////////////////////////////////////////////////////////////
// SMStateEntry static helpers
//////////////////////////////////////////////////////////////////////////

SMStateEntry::eStateKind SMStateEntry::fromKindString(const QString& kind)
{
    if (kind.compare(STR_KIND_START, Qt::CaseInsensitive) == 0)
        return eStateKind::Start;
    else if (kind.compare(STR_KIND_FINAL, Qt::CaseInsensitive) == 0)
        return eStateKind::Final;
    else
        return eStateKind::Normal;
}

const char* SMStateEntry::toString(SMStateEntry::eStateKind kind)
{
    switch (kind)
    {
    case eStateKind::Start:     return STR_KIND_START;
    case eStateKind::Final:     return STR_KIND_FINAL;
    case eStateKind::Normal:
    default:                    return STR_KIND_NORMAL;
    }
}

SMStateEntry::eHistory SMStateEntry::fromHistoryString(const QString& history)
{
    if (history.compare(STR_HISTORY_SHALLOW, Qt::CaseInsensitive) == 0)
        return eHistory::Shallow;
    else if (history.compare(STR_HISTORY_DEEP, Qt::CaseInsensitive) == 0)
        return eHistory::Deep;
    else
        return eHistory::None;
}

const char* SMStateEntry::toString(SMStateEntry::eHistory history)
{
    switch (history)
    {
    case eHistory::Shallow:     return STR_HISTORY_SHALLOW;
    case eHistory::Deep:        return STR_HISTORY_DEEP;
    case eHistory::None:
    default:                    return STR_HISTORY_NONE;
    }
}

//////////////////////////////////////////////////////////////////////////
// SMStateEntry implementation
//////////////////////////////////////////////////////////////////////////

SMStateEntry::SMStateEntry(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mName         ( )
    , mKind         (eStateKind::Normal)
    , mHistory      (eHistory::None)
    , mSubmachine   ( )
    , mOnFinal      ( )
    , mDescription  ( )
    , mEntryList    (this)
    , mExitList     (this)
    , mTransitions  (this)
    , mNested       (nullptr)
{
}

SMStateEntry::SMStateEntry(uint32_t id, const QString& name, eStateKind kind, ElementBase* parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mName         (name)
    , mKind         (kind)
    , mHistory      (eHistory::None)
    , mSubmachine   ( )
    , mOnFinal      ( )
    , mDescription  ( )
    , mEntryList    (this)
    , mExitList     (this)
    , mTransitions  (this)
    , mNested       (nullptr)
{
}

SMStateEntry::SMStateEntry(const SMStateEntry& src)
    : DocumentElem  (src)
    , mName         (src.mName)
    , mKind         (src.mKind)
    , mHistory      (src.mHistory)
    , mSubmachine   (src.mSubmachine)
    , mOnFinal      (src.mOnFinal)
    , mDescription  (src.mDescription)
    , mEntryList    (src.mEntryList)
    , mExitList     (src.mExitList)
    , mTransitions  (src.mTransitions)
    , mNested       (src.mNested != nullptr ? new SMStateData(*src.mNested) : nullptr)
{
    mEntryList.setParent(this);
    mExitList.setParent(this);
    mTransitions.setParent(this);
    if (mNested != nullptr)
    {
        mNested->setParent(this);
    }
}

SMStateEntry::SMStateEntry(SMStateEntry&& src) noexcept
    : DocumentElem  (std::move(src))
    , mName         (std::move(src.mName))
    , mKind         (src.mKind)
    , mHistory      (src.mHistory)
    , mSubmachine   (std::move(src.mSubmachine))
    , mOnFinal      (std::move(src.mOnFinal))
    , mDescription  (std::move(src.mDescription))
    , mEntryList    (std::move(src.mEntryList))
    , mExitList     (std::move(src.mExitList))
    , mTransitions  (std::move(src.mTransitions))
    , mNested       (src.mNested)
{
    src.mNested = nullptr;
    mEntryList.setParent(this);
    mExitList.setParent(this);
    mTransitions.setParent(this);
    if (mNested != nullptr)
    {
        mNested->setParent(this);
    }
}

SMStateEntry::~SMStateEntry(void)
{
    delete mNested;
    mNested = nullptr;
}

SMStateEntry& SMStateEntry::operator = (const SMStateEntry& other)
{
    if (this != &other)
    {
        DocumentElem::operator = (other);
        mName        = other.mName;
        mKind        = other.mKind;
        mHistory     = other.mHistory;
        mSubmachine  = other.mSubmachine;
        mOnFinal     = other.mOnFinal;
        mDescription = other.mDescription;
        mEntryList   = other.mEntryList;
        mExitList    = other.mExitList;
        mTransitions = other.mTransitions;

        delete mNested;
        mNested = (other.mNested != nullptr) ? new SMStateData(*other.mNested) : nullptr;

        mEntryList.setParent(this);
        mExitList.setParent(this);
        mTransitions.setParent(this);
        if (mNested != nullptr)
        {
            mNested->setParent(this);
        }
    }

    return *this;
}

SMStateEntry& SMStateEntry::operator = (SMStateEntry&& other) noexcept
{
    if (this != &other)
    {
        DocumentElem::operator = (std::move(other));
        mName        = std::move(other.mName);
        mKind        = other.mKind;
        mHistory     = other.mHistory;
        mSubmachine  = std::move(other.mSubmachine);
        mOnFinal     = std::move(other.mOnFinal);
        mDescription = std::move(other.mDescription);
        mEntryList   = std::move(other.mEntryList);
        mExitList    = std::move(other.mExitList);
        mTransitions = std::move(other.mTransitions);

        delete mNested;
        mNested = other.mNested;
        other.mNested = nullptr;

        mEntryList.setParent(this);
        mExitList.setParent(this);
        mTransitions.setParent(this);
        if (mNested != nullptr)
        {
            mNested->setParent(this);
        }
    }

    return *this;
}

void SMStateEntry::setSubmachine(const QString& alias)
{
    mSubmachine = alias;
    if ((alias.isEmpty() == false) && (mNested != nullptr))
    {
        delete mNested;
        mNested = nullptr;
    }
}

SMStateData* SMStateEntry::getOrCreateNestedStates(void)
{
    if (mNested == nullptr)
    {
        mNested = new SMStateData(this);
        mSubmachine.clear();
    }

    return mNested;
}

bool SMStateEntry::isValid(void) const
{
    return (mName.isEmpty() == false);
}

bool SMStateEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementState)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    mName = attributes.value(XmlSM::xmlSMAttributeName).toString();
    mKind = fromKindString(attributes.value(XmlSM::xmlSMAttributeKind).toString());
    mHistory = attributes.hasAttribute(XmlSM::xmlSMAttributeHistory)
                    ? fromHistoryString(attributes.value(XmlSM::xmlSMAttributeHistory).toString())
                    : eHistory::None;
    if (attributes.hasAttribute(XmlSM::xmlSMAttributeSubmachine))
    {
        setSubmachine(attributes.value(XmlSM::xmlSMAttributeSubmachine).toString());
    }
    if (attributes.hasAttribute(XmlSM::xmlSMAttributeOnFinal))
    {
        setOnFinal(attributes.value(XmlSM::xmlSMAttributeOnFinal).toString());
    }
    mDescription.clear();

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementState))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSM::xmlSMElementDescription)
            {
                mDescription = xml.readElementText();
            }
            else if (xml.name() == XmlSM::xmlSMElementEntryList)
            {
                mEntryList.readFromXml(xml, XmlSM::xmlSMElementEntryList);
            }
            else if (xml.name() == XmlSM::xmlSMElementExitList)
            {
                mExitList.readFromXml(xml, XmlSM::xmlSMElementExitList);
            }
            else if (xml.name() == XmlSM::xmlSMElementTransitionList)
            {
                mTransitions.readFromXml(xml);
            }
            else if (xml.name() == XmlSM::xmlSMElementStateList)
            {
                getOrCreateNestedStates()->readFromXml(xml);
            }
        }

        xml.readNext();
    }

    return true;
}

void SMStateEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementState);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSM::xmlSMAttributeName, mName);
    xml.writeAttribute(XmlSM::xmlSMAttributeKind, SMStateEntry::toString(mKind));
    if (mHistory != eHistory::None)
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeHistory, SMStateEntry::toString(mHistory));
    }
    if (mSubmachine.isEmpty() == false)
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeSubmachine, mSubmachine);
    }
    if (mOnFinal.isEmpty() == false)
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeOnFinal, mOnFinal);
    }

    writeTextElem(xml, XmlSM::xmlSMElementDescription, mDescription, true);
    mEntryList.writeToXml(xml, XmlSM::xmlSMElementEntryList);
    mExitList.writeToXml(xml, XmlSM::xmlSMElementExitList);
    mTransitions.writeToXml(xml);
    if (mNested != nullptr)
    {
        mNested->writeToXml(xml);
    }

    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMStateData implementation
//////////////////////////////////////////////////////////////////////////

SMStateData::SMStateData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMStateEntry*, DocumentElem>(parent)
{
}

SMStateData::SMStateData(const SMStateData& src)
    : TEDataContainer<SMStateEntry*, DocumentElem>(src.getParent())
{
    cloneFrom(src);
}

SMStateData::SMStateData(SMStateData&& src) noexcept
    : TEDataContainer<SMStateEntry*, DocumentElem>(std::move(src))
{
}

SMStateData::~SMStateData(void)
{
    removeAll();
}

SMStateData& SMStateData::operator = (const SMStateData& other)
{
    if (this != &other)
    {
        removeAll();
        setParent(other.getParent());
        cloneFrom(other);
    }

    return *this;
}

SMStateData& SMStateData::operator = (SMStateData&& other) noexcept
{
    if (this != &other)
    {
        removeAll();
        TEDataContainer<SMStateEntry*, DocumentElem>::operator = (std::move(other));
    }

    return *this;
}

void SMStateData::cloneFrom(const SMStateData& src)
{
    for (const SMStateEntry* state : src.getElements())
    {
        SMStateEntry* copy = new SMStateEntry(*state);
        copy->setParent(this);
        addElement(copy, false);
    }
}

SMStateEntry* SMStateData::createState(const QString& name, SMStateEntry::eStateKind kind)
{
    if (findState(name) != nullptr)
    {
        return nullptr;
    }

    SMStateEntry* entry = new SMStateEntry(getNextId(), name, kind, this);
    addElement(entry, true);
    return entry;
}

SMStateEntry* SMStateData::findState(const QString& name) const
{
    SMStateEntry* const* found = findElement(name);
    return (found != nullptr) ? *found : nullptr;
}

SMStateEntry* SMStateData::findStateRecursive(const QString& name) const
{
    for (SMStateEntry* state : getElements())
    {
        if (state->getName() == name)
        {
            return state;
        }

        if (state->hasNestedStates())
        {
            SMStateEntry* found = state->getNestedStates()->findStateRecursive(name);
            if (found != nullptr)
            {
                return found;
            }
        }
    }

    return nullptr;
}

SMStateEntry* SMStateData::getStartState(void) const
{
    for (SMStateEntry* state : getElements())
    {
        if (state->getKind() == SMStateEntry::eStateKind::Start)
        {
            return state;
        }
    }

    return nullptr;
}

int SMStateData::countStatesRecursive(void) const
{
    int count = getElementCount();
    for (SMStateEntry* state : getElements())
    {
        if (state->hasNestedStates())
        {
            count += state->getNestedStates()->countStatesRecursive();
        }
    }

    return count;
}

void SMStateData::removeAll(void)
{
    for (SMStateEntry* state : getElements())
    {
        delete state;
    }

    removeAllElements();
}

bool SMStateData::isValid(void) const
{
    return true;
}

bool SMStateData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementStateList)
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementStateList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementState)
        {
            SMStateEntry* state = new SMStateEntry(this);
            if (state->readFromXml(xml))
            {
                addElement(state, true);
            }
            else
            {
                delete state;
            }
        }

        xml.readNext();
    }

    return true;
}

void SMStateData::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementStateList);
    for (const SMStateEntry* state : getElements())
    {
        state->writeToXml(xml);
    }

    xml.writeEndElement();
}
