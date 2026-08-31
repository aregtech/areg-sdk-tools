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
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
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

SMTransitionEntry::eTransitionKind SMTransitionEntry::fromTransitionKindString(const QString& kind)
{
    if (kind.compare(STR_TRANS_INTERNAL, Qt::CaseInsensitive) == 0)
        return eTransitionKind::Internal;
    else if (kind.compare(STR_TRANS_INITIAL, Qt::CaseInsensitive) == 0)
        return eTransitionKind::Initial;
    else
        return eTransitionKind::External;
}

const char* SMTransitionEntry::toString(SMTransitionEntry::eTransitionKind kind)
{
    switch (kind)
    {
    case eTransitionKind::Internal: return STR_TRANS_INTERNAL;
    case eTransitionKind::Initial:  return STR_TRANS_INITIAL;
    case eTransitionKind::External:
    default:                        return STR_TRANS_EXTERNAL;
    }
}

//////////////////////////////////////////////////////////////////////////
// SMTransitionEntry implementation
//////////////////////////////////////////////////////////////////////////

SMTransitionEntry::SMTransitionEntry(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mKind         (eTransitionKind::External)
    , mStimulusKind (eStimulusKind::Trigger)
    , mStimulus     ( )
    , mToId         (0)
    , mToName       ( )
    , mDescription  ( )
    , mConditions   (this)
    , mGuard        ( )
    , mOperations   (this)
{
}

SMTransitionEntry::SMTransitionEntry(  uint32_t id
                                     , eStimulusKind kind
                                     , const QString& stimulus
                                     , ElementBase* parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mKind         (eTransitionKind::External)
    , mStimulusKind (kind)
    , mStimulus     (stimulus)
    , mToId         (0)
    , mToName       ( )
    , mDescription  ( )
    , mConditions   (this)
    , mGuard        ( )
    , mOperations   (this)
{
}

SMTransitionEntry::SMTransitionEntry(const SMTransitionEntry& src)
    : DocumentElem  (src)
    , mKind         (src.mKind)
    , mStimulusKind (src.mStimulusKind)
    , mStimulus     (src.mStimulus)
    , mToId         (src.mToId)
    , mToName       (src.mToName)
    , mDescription  (src.mDescription)
    , mConditions   (src.mConditions)
    , mGuard        (src.mGuard)
    , mOperations   (src.mOperations)
{
    mConditions.setParent(this);
    mOperations.setParent(this);
}

SMTransitionEntry::SMTransitionEntry(SMTransitionEntry&& src) noexcept
    : DocumentElem  (std::move(src))
    , mKind         (src.mKind)
    , mStimulusKind (src.mStimulusKind)
    , mStimulus     (std::move(src.mStimulus))
    , mToId         (src.mToId)
    , mToName       (std::move(src.mToName))
    , mDescription  (std::move(src.mDescription))
    , mConditions   (std::move(src.mConditions))
    , mGuard        (std::move(src.mGuard))
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
        mKind         = other.mKind;
        mStimulusKind = other.mStimulusKind;
        mStimulus     = other.mStimulus;
        mToId         = other.mToId;
        mToName       = other.mToName;
        mDescription  = other.mDescription;
        mConditions   = other.mConditions;
        mGuard        = other.mGuard;
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
        mKind         = other.mKind;
        mStimulusKind = other.mStimulusKind;
        mStimulus     = std::move(other.mStimulus);
        mToId         = other.mToId;
        mToName       = std::move(other.mToName);
        mDescription  = std::move(other.mDescription);
        mConditions   = std::move(other.mConditions);
        mGuard        = std::move(other.mGuard);
        mOperations   = std::move(other.mOperations);
        mConditions.setParent(this);
        mOperations.setParent(this);
    }

    return *this;
}

void SMTransitionEntry::setKind(SMTransitionEntry::eTransitionKind kind)
{
    mKind = kind;
    if (mKind == eTransitionKind::Internal)
    {
        // An internal transition has no target by definition, so `To` means the target and nothing
        // else, and no transition claims to be internal while still naming somewhere to go.
        mToId = 0;
        mToName.clear();
    }
    else if (mKind == eTransitionKind::Initial)
    {
        // Nothing fires an initial transition -- it is taken on entering the level -- so it names
        // no stimulus at all. The condition is the only thing allowed to decide between siblings.
        mStimulus.clear();
        mStimulusKind = eStimulusKind::Trigger;
    }
}

const SMStateEntry* SMTransitionEntry::owningState() const
{
    // parent chain: transition -> its state's TransitionList -> the state.
    const ElementBase* list = getParent();
    return (list != nullptr ? dynamic_cast<const SMStateEntry*>(list->getParent()) : nullptr);
}

bool SMTransitionEntry::isSelfTransition() const
{
    const SMStateEntry* owner = owningState();
    return (mToId != 0) && (owner != nullptr) && (owner->getId() == mToId);
}

QString SMTransitionEntry::getTargetName() const
{
    if (mToId == 0)
        return QString();

    // Walk the element parent chain to the document root, then resolve the target by ID.
    const ElementBase* root = this;
    while (root->getParent() != nullptr)
    {
        root = root->getParent();
    }

    const StateMachineData* doc = dynamic_cast<const StateMachineData*>(root);
    const SMStateEntry* target = (doc != nullptr ? doc->findStateById(mToId) : nullptr);
    return (target != nullptr ? target->getName() : QString());
}

bool SMTransitionEntry::isValid() const
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
    const QStringView to = attributes.value(XmlSM::xmlSMAttributeTo);
    bool numeric = false;
    const uint32_t toId = to.toUInt(&numeric);
    mToId   = numeric ? toId : 0;
    mToName = (numeric || to.isEmpty()) ? QString() : to.toString();
    mDescription.clear();

    const QStringView kindText = attributes.value(XmlSM::xmlSMAttributeKind);
    if (kindText.isEmpty() == false)
    {
        mKind = fromTransitionKindString(kindText.toString());
    }
    else
    {
        const SMStateEntry* owner = owningState();
        const bool initial = (owner != nullptr) && (owner->getKind() == SMStateEntry::eStateKind::Start)
                          && mStimulus.isEmpty();
        if (initial)
            mKind = eTransitionKind::Initial;
        else if ((mToId != 0) || (mToName.isEmpty() == false))
            mKind = eTransitionKind::External;
        else
            mKind = eTransitionKind::Internal;
    }

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
            else if (xml.name() == XmlSM::xmlSMElementGuard)
            {
                mGuard.readFromXml(xml);
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
    // Always written, including at its `External` default. Omitting it would make an unconnected
    // external edge and an internal transition the same bytes again.
    xml.writeAttribute(XmlSM::xmlSMAttributeKind, SMTransitionEntry::toString(mKind));
    // An initial transition names no stimulus, so the placeholder attributes are not written. A
    // hand-edited document that put a name there keeps carrying it until its author resolves it.
    if ((mKind != eTransitionKind::Initial) || (mStimulus.isEmpty() == false))
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeStimulusKind, SMTransitionEntry::toString(mStimulusKind));
        xml.writeAttribute(XmlSM::xmlSMAttributeStimulus, mStimulus);
    }
    if (mToId != 0)
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeTo, QString::number(mToId));
    }

    writeTextElem(xml, XmlSM::xmlSMElementDescription, mDescription, true);
    // Canonical guard when present; otherwise the untouched legacy condition tree (read-shim
    // migration writes a <Guard> only once the guard is populated by the editor).
    if (mGuard.hasContent())
    {
        mGuard.writeToXml(xml);
    }
    else
    {
        mConditions.writeToXml(xml);
    }
    mOperations.writeToXml(xml, XmlSM::xmlSMElementOperationList);

    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMTransitionData implementation
//////////////////////////////////////////////////////////////////////////

SMTransitionData::SMTransitionData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMTransitionEntry*, DocumentElem>(parent)
{
    // Document order is priority order, and layout edges name transitions by id, so re-numbering
    // on a list change would re-key the geometry of every sibling.
    setIdReordering(false);
}

SMTransitionData::SMTransitionData(const SMTransitionData& src)
    : TEDataContainer<SMTransitionEntry*, DocumentElem>(src.getParent())
{
    setIdReordering(false);
    cloneFrom(src);
}

SMTransitionData::SMTransitionData(SMTransitionData&& src) noexcept
    : TEDataContainer<SMTransitionEntry*, DocumentElem>(std::move(src))
{
    setIdReordering(false);
}

SMTransitionData::~SMTransitionData()
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

SMTransitionEntry* SMTransitionData::createTransition(  SMTransitionEntry::eStimulusKind kind
                                                      , const QString& stimulus
                                                      , uint32_t targetId /*= 0*/
                                                      , SMTransitionEntry::eTransitionKind transKind /*= External*/)
{
    SMTransitionEntry* entry = new SMTransitionEntry(getNextId(), kind, stimulus, this);
    if (targetId != 0)
    {
        entry->setToId(targetId);
    }

    // After the target, because setting the kind to Internal is what drops one.
    entry->setKind(transKind);
    addElement(entry, false);
    return entry;
}

void SMTransitionData::removeAll()
{
    for (SMTransitionEntry* entry : getElements())
    {
        delete entry;
    }

    removeAllElements();
}

bool SMTransitionData::isValid() const
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
