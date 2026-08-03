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
        // An internal transition has no target by definition. Dropping it here is what keeps the
        // document honest: `To` then means "the target" and nothing else, and there is no state
        // in which a transition claims to be internal while still naming somewhere to go.
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
    // Targets became IDs in SM-26. A document written before that -- or hand-authored from the
    // older spec examples -- names the target state; keep the name for the document root to
    // resolve once the whole tree is in memory, or the transition silently becomes internal.
    const QStringView to = attributes.value(XmlSM::xmlSMAttributeTo);
    bool numeric = false;
    const uint32_t toId = to.toUInt(&numeric);
    mToId   = numeric ? toId : 0;
    mToName = (numeric || to.isEmpty()) ? QString() : to.toString();
    mDescription.clear();

    // `Kind` says what the transition IS. A document written before it said nothing, and the
    // meaning had to be inferred from which attributes were missing -- which is precisely the
    // ambiguity the attribute removes, so the inference survives here as a READ SHIM and nowhere
    // else. What the attribute states is taken as written, faults included: a `Kind="Internal"`
    // that still names a target, or a `Kind="Initial"` that still names a stimulus, is kept and
    // reported rather than quietly repaired, because only the author knows which half was meant.
    const QStringView kindText = attributes.value(XmlSM::xmlSMAttributeKind);
    if (kindText.isEmpty() == false)
    {
        mKind = fromTransitionKindString(kindText.toString());
    }
    else
    {
        // The stimulus test is what keeps this composable with the `Kind="Start"` read shim: a
        // Start-owned transition that names a stimulus belongs to a legacy MERGED start, whose
        // state is demoted to Normal on load -- so it is an ordinary external transition, and its
        // stimulus is real content, not the placeholder an initial transition used to carry.
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
    // Always written, including at its `External` default. Omitting it would put the format back
    // where it started: an unconnected external edge and an internal transition would again be the
    // same bytes, and the reader would have to guess which one the author meant.
    xml.writeAttribute(XmlSM::xmlSMAttributeKind, SMTransitionEntry::toString(mKind));
    // An initial transition names no stimulus, so the two placeholder attributes it used to fill
    // are simply not written -- unless a hand-edited document actually put a name there, which is
    // a fault the file has to keep carrying until its author resolves it.
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
    // Document order here is PRIORITY order, so transitions are reordered on purpose -- and the
    // layout `Edge` entries name them by ID. Re-numbering on a list change would re-key the
    // geometry of every sibling. Same reason as SMStateData.
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
