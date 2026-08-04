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
    , mDoList       (this)
    , mDoInterval   (SMStateEntry::DEFAULT_DO_INTERVAL)
    , mDoUntil      ( )
    , mDoUntilLegacy( )
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
    , mDoList       (this)
    , mDoInterval   (SMStateEntry::DEFAULT_DO_INTERVAL)
    , mDoUntil      ( )
    , mDoUntilLegacy( )
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
    , mDoList       (src.mDoList)
    , mDoInterval   (src.mDoInterval)
    , mDoUntil      (src.mDoUntil)
    , mDoUntilLegacy(src.mDoUntilLegacy)
    , mTransitions  (src.mTransitions)
    , mNested       (src.mNested != nullptr ? new SMStateData(*src.mNested) : nullptr)
{
    mEntryList.setParent(this);
    mExitList.setParent(this);
    mDoList.setParent(this);
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
    , mDoList       (std::move(src.mDoList))
    , mDoInterval   (src.mDoInterval)
    , mDoUntil      (std::move(src.mDoUntil))
    , mDoUntilLegacy(std::move(src.mDoUntilLegacy))
    , mTransitions  (std::move(src.mTransitions))
    , mNested       (src.mNested)
{
    src.mNested = nullptr;
    mEntryList.setParent(this);
    mExitList.setParent(this);
    mDoList.setParent(this);
    mTransitions.setParent(this);
    if (mNested != nullptr)
    {
        mNested->setParent(this);
    }
}

SMStateEntry::~SMStateEntry()
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
        mDoList      = other.mDoList;
        mDoInterval  = other.mDoInterval;
        mDoUntil     = other.mDoUntil;
        mDoUntilLegacy = other.mDoUntilLegacy;
        mTransitions = other.mTransitions;

        delete mNested;
        mNested = (other.mNested != nullptr) ? new SMStateData(*other.mNested) : nullptr;

        mEntryList.setParent(this);
        mExitList.setParent(this);
        mDoList.setParent(this);
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
        mDoList      = std::move(other.mDoList);
        mDoInterval  = other.mDoInterval;
        mDoUntil     = std::move(other.mDoUntil);
        mDoUntilLegacy = std::move(other.mDoUntilLegacy);
        mTransitions = std::move(other.mTransitions);

        delete mNested;
        mNested = other.mNested;
        other.mNested = nullptr;

        mEntryList.setParent(this);
        mExitList.setParent(this);
        mDoList.setParent(this);
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

SMStateData* SMStateEntry::getOrCreateNestedStates()
{
    if (mNested == nullptr)
    {
        mNested = new SMStateData(this);
        mSubmachine.clear();
    }

    return mNested;
}

SMStateData* SMStateEntry::takeNestedStates()
{
    SMStateData* result = mNested;
    mNested = nullptr;
    return result;
}

void SMStateEntry::attachNestedStates(SMStateData* nested)
{
    if (mNested != nested)
    {
        delete mNested;
        mNested = nested;
    }

    if (mNested != nullptr)
    {
        mNested->setParent(this);
        mSubmachine.clear();
    }
}

bool SMStateEntry::isValid() const
{
    return (mName.isEmpty() == false);
}

bool SMStateEntry::isLegacyMergedStart() const
{
    if (isPseudoStart() == false)
    {
        return false;
    }

    if (hasOperations())
    {
        return true;
    }

    // A pseudo-state's transitions are the level's initial ones, taken on entry, so they can never
    // name what triggers them. One that does is a real state's reaction wearing the wrong kind.
    for (const SMTransitionEntry* transition : mTransitions.getElements())
    {
        if ((transition != nullptr) && (transition->getStimulus().isEmpty() == false))
        {
            return true;
        }
    }

    return false;
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
    // 0, not the editor default: a document that names no interval must read back as one that
    // names no interval, so validation can say so. Inventing a period here would hide the fault.
    mDoInterval = 0u;
    mDoUntil.clear();
    mDoUntilLegacy.clear();

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
            else if (xml.name() == XmlSM::xmlSMElementDoList)
            {
                // The tick period rides on the wrapper element, so capture it before delegating the
                // child operations. The legacy `Until` text is kept byte-for-byte for the load shim.
                const QXmlStreamAttributes doAttributes = xml.attributes();
                mDoInterval    = doAttributes.value(XmlSM::xmlSMAttributeInterval).toUInt();
                mDoUntilLegacy = doAttributes.value(XmlSM::xmlSMAttributeUntil).toString();
                // The stop condition is a tree, so it is a child element. It is the one child of an
                // operation-list wrapper that is not an operation, so the reader hands it back here.
                mDoList.readFromXml(xml, XmlSM::xmlSMElementDoList, [this](QXmlStreamReader& reader) -> bool
                {
                    return (reader.name() == XmlSM::xmlSMElementUntil)
                            ? mDoUntil.readFromXml(reader, XmlSM::xmlSMElementUntil)
                            : false;
                });
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
    // A half-built painted submachine is dropped below, and the attributes that only make sense
    // with one go with it, so the file cannot reload as a plain state carrying history.
    const bool droppingNested = (mNested != nullptr) && (mNested->hasRealState() == false);
    if ((mHistory != eHistory::None) && (droppingNested == false))
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeHistory, SMStateEntry::toString(mHistory));
    }
    if (mSubmachine.isEmpty() == false)
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeSubmachine, mSubmachine);
    }
    if ((mOnFinal.isEmpty() == false) && (droppingNested == false))
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeOnFinal, mOnFinal);
    }

    writeTextElem(xml, XmlSM::xmlSMElementDescription, mDescription, true);
    mEntryList.writeToXml(xml, XmlSM::xmlSMElementEntryList);
    mExitList.writeToXml(xml, XmlSM::xmlSMElementExitList);
    // The Do list carries a repeat policy, so it is written by hand: the tick period, the stop
    // condition, then the operations. Nothing is written when the list is empty.
    if (mDoList.isEmpty() == false)
    {
        xml.writeStartElement(XmlSM::xmlSMElementDoList);
        // Always written, including a 0 that no document should contain: a Do is a timer loop, so
        // the period is what makes it one, and writing the 0 back keeps the fault visible.
        xml.writeAttribute(XmlSM::xmlSMAttributeInterval, QString::number(mDoInterval));
        mDoUntil.writeToXml(xml, XmlSM::xmlSMElementUntil);
        for (const SMOperationBase* op : mDoList.getOperations())
        {
            op->writeToXml(xml);
        }
        xml.writeEndElement();
    }
    mTransitions.writeToXml(xml);
    // A submachine is persisted only when it owns at least one Normal state. Otherwise the state
    // serializes as a plain leaf and its layout is dropped with it.
    if ((mNested != nullptr) && mNested->hasRealState())
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
    // A state's id is its identity, not its position: transitions name it in `To` and every layout
    // entry names it in `Owner`, and none of those are rewritten when the container re-numbers.
    setIdReordering(false);
}

SMStateData::SMStateData(const SMStateData& src)
    : TEDataContainer<SMStateEntry*, DocumentElem>(src.getParent())
{
    setIdReordering(false);
    cloneFrom(src);
}

SMStateData::SMStateData(SMStateData&& src) noexcept
    : TEDataContainer<SMStateEntry*, DocumentElem>(std::move(src))
{
    setIdReordering(false);
}

SMStateData::~SMStateData()
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

SMStateEntry* SMStateData::findStateById(uint32_t id) const
{
    SMStateEntry* const* found = findElement(id);
    return (found != nullptr) ? *found : nullptr;
}

SMStateEntry* SMStateData::findStateByIdRecursive(uint32_t id) const
{
    for (SMStateEntry* state : getElements())
    {
        if (state->getId() == id)
        {
            return state;
        }

        if (state->hasNestedStates())
        {
            SMStateEntry* found = state->getNestedStates()->findStateByIdRecursive(id);
            if (found != nullptr)
            {
                return found;
            }
        }
    }

    return nullptr;
}

SMStateEntry* SMStateData::getStartState() const
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

SMStateEntry* SMStateData::findTransitionOwnerRecursive(uint32_t transitionId) const
{
    for (SMStateEntry* state : getElements())
    {
        if (state->getTransitions().findElement(transitionId) != nullptr)
        {
            return state;
        }

        if (state->hasNestedStates())
        {
            SMStateEntry* found = state->getNestedStates()->findTransitionOwnerRecursive(transitionId);
            if (found != nullptr)
            {
                return found;
            }
        }
    }

    return nullptr;
}

int SMStateData::countStatesRecursive() const
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

bool SMStateData::hasRealState() const
{
    for (const SMStateEntry* state : getElements())
    {
        if ((state != nullptr) && (state->getKind() == SMStateEntry::eStateKind::Normal))
        {
            return true;
        }
    }

    return false;
}

void SMStateData::removeAll()
{
    for (SMStateEntry* state : getElements())
    {
        delete state;
    }

    removeAllElements();
}

bool SMStateData::isValid() const
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
