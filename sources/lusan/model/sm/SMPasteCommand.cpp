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
 *  \file        lusan/model/sm/SMPasteCommand.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM paste command.
 *
 ************************************************************************/

#include "lusan/model/sm/SMPasteCommand.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"

#include <QSet>

//////////////////////////////////////////////////////////////////////////
// SMPasteCommand::RegistryEntryBase
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   One planned registry insertion. The entry is owned by the plan while it is
 *          out of the model; the first insertion allocates its ID and applies the
 *          ID-suffix rename when the plan marked a name conflict.
 **/
class SMPasteCommand::RegistryEntryBase
{
public:
    //!< The rename map a conflict rename of this entry feeds.
    enum class eRenameRole
    {
          None
        , Event
        , Timer
        , Method
        , Attribute
        , Constant
    };

protected:
    RegistryEntryBase(eDocElementKind kind, eRenameRole role, const QString& name, bool conflict)
        : mKind     (kind)
        , mRole     (role)
        , mName     (name)
        , mFinalName( )
        , mId       (0u)
        , mConflict (conflict)
    {
    }

public:
    virtual ~RegistryEntryBase() = default;

    //!< Inserts the entry; the first insertion allocates the ID and applies the rename.
    virtual void insertEntry(StateMachineData& data, DocModelNotifier& notifier, bool first) = 0;

    //!< Removes the entry, recapturing it for the next insertion.
    virtual void removeEntry(StateMachineData& data, DocModelNotifier& notifier) = 0;

public:
    eDocElementKind mKind;      //!< The notification kind.
    eRenameRole     mRole;      //!< The rename map the conflict rename feeds.
    QString         mName;      //!< The original entry name.
    QString         mFinalName; //!< The pasted name (suffixed on conflict).
    uint32_t        mId;        //!< The target-document ID, set on first insertion.
    bool            mConflict;  //!< True when the name is taken and the copy is renamed.
};

namespace
{
    using eRenameRole = SMPasteCommand::RegistryEntryBase::eRenameRole;

    //!< The rename map of a role, or nullptr when renames of the role remap nothing.
    QHash<QString, QString>* _mapForRole(SMClipboard::RenameMaps& renames, eRenameRole role)
    {
        switch (role)
        {
        case eRenameRole::Event:        return &renames.events;
        case eRenameRole::Timer:        return &renames.timers;
        case eRenameRole::Method:       return &renames.methods;
        case eRenameRole::Attribute:    return &renames.attributes;
        case eRenameRole::Constant:     return &renames.constants;
        case eRenameRole::None:
        default:                        return nullptr;
        }
    }

    /**
     * \brief   The typed registry insertion: works for value entries (SMAttributeEntry,
     *          SMTimerEntry, ConstantEntry, IncludeEntry) and owning-pointer entries
     *          (SMEventEntry*, SMMethodEntry*, DataTypeCustom*).
     **/
    template<typename Data>
    class TPasteRegistryEntry : public SMPasteCommand::RegistryEntryBase
    {
        using Container = TEDataContainer<Data, DocumentElem>;
        using ptr       = NELusanCommon::get_ptr<Data>;

    public:
        using SectionFn = Container& (*)(StateMachineData&);

        TPasteRegistryEntry(Data entry, SectionFn section, eDocElementKind kind, eRenameRole role, bool conflict)
            : RegistryEntryBase (kind, role, ptr{ }(entry)->getName(), conflict)
            , mEntry            (std::move(entry))
            , mSection          (section)
            , mOwned            (true)
        {
        }

        ~TPasteRegistryEntry() override
        {
            if constexpr (std::is_pointer_v<Data>)
            {
                if (mOwned)
                {
                    delete mEntry;
                }
            }
        }

        void insertEntry(StateMachineData& data, DocModelNotifier& notifier, bool first) override
        {
            Container& container = mSection(data);
            auto* elem = ptr{ }(mEntry);
            elem->setParent(&container);
            if (first)
            {
                elem->setId(data.getNextId());
                mId = elem->getId();
                mFinalName = mConflict ? (mName + QStringLiteral("_") + QString::number(mId)) : mName;
                if (mConflict)
                {
                    elem->setName(mFinalName);
                }
            }

            container.addElement(mEntry, false);
            mOwned = false;
            notifier.notifyElementAdded(mId, mKind);
        }

        void removeEntry(StateMachineData& data, DocModelNotifier& notifier) override
        {
            Container& container = mSection(data);
            container.removeElement(mId, &mEntry);
            mOwned = true;
            notifier.notifyElementRemoved(mId, mKind);
        }

    private:
        Data        mEntry;     //!< The entry; owned while out of the model.
        SectionFn   mSection;   //!< Returns the target document's section container.
        bool        mOwned;     //!< True when this plan entry, not the model, owns the entry.
    };

    // Section accessors passed into the typed plan entries.
    TEDataContainer<DataTypeCustom*, DocumentElem>&  _dataTypesOf(StateMachineData& data)  { return data.getDataTypes(); }
    TEDataContainer<SMAttributeEntry, DocumentElem>& _attributesOf(StateMachineData& data) { return data.getAttributes(); }
    TEDataContainer<SMEventEntry*, DocumentElem>&    _eventsOf(StateMachineData& data)     { return data.getEvents(); }
    TEDataContainer<SMTimerEntry, DocumentElem>&     _timersOf(StateMachineData& data)     { return data.getTimers(); }
    TEDataContainer<SMMethodEntry*, DocumentElem>&   _methodsOf(StateMachineData& data)    { return data.getMethods(); }
    TEDataContainer<ConstantEntry, DocumentElem>&    _constantsOf(StateMachineData& data)  { return data.getConstants(); }
    TEDataContainer<IncludeEntry, DocumentElem>&     _includesOf(StateMachineData& data)   { return data.getIncludes(); }
}

//////////////////////////////////////////////////////////////////////////
// SMPasteCommand implementation
//////////////////////////////////////////////////////////////////////////

SMPasteCommand::SMPasteCommand(  StateMachineData& data, DocModelNotifier& notifier
                               , std::unique_ptr<SMClipboardContent> content
                               , uint32_t targetLevelId, const QPointF& offset
                               , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand     (data, notifier, text, parent)
    , mRegistry     ( )
    , mStates       ( )
    , mSrcNodes     ( )
    , mSrcEdges     ( )
    , mSrcNotes     ( )
    , mNodes        ( )
    , mEdges        ( )
    , mNotes        ( )
    , mOldToNew     ( )
    , mPastedIds    ( )
    , mTargetLevel  (targetLevelId)
    , mOffset       (offset)
    , mAllocated    (false)
{
    if (content == nullptr)
    {
        return;
    }

    for (SMStateEntry* state : content->mStates.getElements())
    {
        PastedState pasted;
        pasted.state = state;
        pasted.oldId = state->getId();
        for (const SMTransitionEntry* transition : state->getTransitions().getElements())
        {
            pasted.oldTxIds.append(transition->getId());
        }

        // Detach from the payload: the parent stays unset until the first redo so no
        // ID traffic can reach the destroyed payload root.
        state->setParent(static_cast<ElementBase*>(nullptr));
        mStates.append(pasted);
    }

    content->mStates.removeAllElements();

    mSrcNodes = content->mLayout.getNodes();
    mSrcEdges = content->mLayout.getEdges();
    mSrcNotes = content->mLayout.getNotes();

    buildRegistryPlan(*content);
}

SMPasteCommand::~SMPasteCommand()
{
    for (PastedState& pasted : mStates)
    {
        if (pasted.owned)
        {
            delete pasted.state;
            pasted.state = nullptr;
        }
    }

    qDeleteAll(mRegistry);
    mRegistry.clear();
}

bool SMPasteCommand::isEffective() const
{
    const bool hasContent = (mStates.isEmpty() == false) || (mSrcNotes.isEmpty() == false) || (mRegistry.isEmpty() == false);
    const bool needsLevel = (mStates.isEmpty() == false) || (mSrcNotes.isEmpty() == false);
    return hasContent && ((needsLevel == false) || (data().findLevel(mTargetLevel) != nullptr));
}

void SMPasteCommand::buildRegistryPlan(SMClipboardContent& content)
{
    StateMachineData& doc = data();

    // Names this paste will occupy, per name space, so plan entries never collide with
    // each other; conflict copies claim nothing (their ID suffix is unique by itself).
    QSet<QString> claimedStimulus;
    QSet<QString> claimedMethods;
    QSet<QString> claimedAttributes;
    QSet<QString> claimedConstants;
    QSet<QString> claimedTypes;

    const auto stimulusTaken = [&doc, &claimedStimulus, &claimedMethods](const QString& name) -> bool
    {
        return doc.isStimulusName(name) || claimedStimulus.contains(name) || claimedMethods.contains(name);
    };

    // Referenced entries merge by name: an identical entry is reused (no plan entry),
    // anything else is inserted, renamed when its name is taken.
    const auto planEvents = [&](SMEventData& section, bool referenced)
    {
        for (SMEventEntry* entry : section.getElements())
        {
            const QString& name = entry->getName();
            const SMEventEntry* existing = doc.getEvents().findEvent(name);
            if (referenced && (existing != nullptr) && SMClipboard::structurallyEqual(*existing, *entry))
            {
                delete entry;
                continue;
            }

            const bool conflict = stimulusTaken(name) || ((referenced == false) && claimedStimulus.contains(name));
            if (conflict == false)
            {
                claimedStimulus.insert(name);
            }

            mRegistry.append(new TPasteRegistryEntry<SMEventEntry*>(entry, &_eventsOf, eDocElementKind::Event
                                                                    , eRenameRole::Event, conflict));
        }

        section.removeAllElements();
    };

    const auto planTimers = [&](SMTimerData& section, bool referenced)
    {
        for (SMTimerEntry& entry : section.getElements())
        {
            const QString& name = entry.getName();
            const SMTimerEntry* existing = doc.getTimers().findElement(name);
            if (referenced && (existing != nullptr) && SMClipboard::structurallyEqual(*existing, entry))
            {
                continue;
            }

            const bool conflict = stimulusTaken(name);
            if (conflict == false)
            {
                claimedStimulus.insert(name);
            }

            mRegistry.append(new TPasteRegistryEntry<SMTimerEntry>(entry, &_timersOf, eDocElementKind::Timer
                                                                   , eRenameRole::Timer, conflict));
        }

        section.removeAllElements();
    };

    const auto planMethods = [&](SMMethodData& section, bool referenced)
    {
        for (SMMethodEntry* entry : section.getElements())
        {
            const QString& name = entry->getName();
            const SMMethodEntry* existing = doc.getMethods().findMethod(name);
            if (referenced && (existing != nullptr) && (existing->getMethodType() == entry->getMethodType())
                && SMClipboard::structurallyEqual(*existing, *entry))
            {
                delete entry;
                continue;
            }

            const bool trigger  = (entry->getMethodType() == SMMethodEntry::eMethodType::Trigger);
            const bool conflict = (existing != nullptr) || claimedMethods.contains(name)
                                  || (trigger && stimulusTaken(name));
            if (conflict == false)
            {
                claimedMethods.insert(name);
            }

            mRegistry.append(new TPasteRegistryEntry<SMMethodEntry*>(entry, &_methodsOf, eDocElementKind::Method
                                                                     , eRenameRole::Method, conflict));
        }

        section.removeAllElements();
    };

    const auto planAttributes = [&](SMAttributeData& section, bool referenced)
    {
        for (SMAttributeEntry& entry : section.getElements())
        {
            const QString& name = entry.getName();
            const SMAttributeEntry* existing = doc.getAttributes().findElement(name);
            if (referenced && (existing != nullptr) && SMClipboard::structurallyEqual(*existing, entry))
            {
                continue;
            }

            const bool conflict = (existing != nullptr) || claimedAttributes.contains(name);
            if (conflict == false)
            {
                claimedAttributes.insert(name);
            }

            mRegistry.append(new TPasteRegistryEntry<SMAttributeEntry>(entry, &_attributesOf, eDocElementKind::Attribute
                                                                       , eRenameRole::Attribute, conflict));
        }

        section.removeAllElements();
    };

    const auto planConstants = [&](SMConstantData& section, bool referenced)
    {
        for (ConstantEntry& entry : section.getElements())
        {
            const QString& name = entry.getName();
            const ConstantEntry* existing = doc.getConstants().findElement(name);
            if (referenced && (existing != nullptr) && SMClipboard::structurallyEqual(*existing, entry))
            {
                continue;
            }

            const bool conflict = (existing != nullptr) || claimedConstants.contains(name);
            if (conflict == false)
            {
                claimedConstants.insert(name);
            }

            mRegistry.append(new TPasteRegistryEntry<ConstantEntry>(entry, &_constantsOf, eDocElementKind::Constant
                                                                    , eRenameRole::Constant, conflict));
        }

        section.removeAllElements();
    };

    planEvents(content.mRefEvents, true);
    planTimers(content.mRefTimers, true);
    planMethods(content.mRefMethods, true);
    planAttributes(content.mRefAttributes, true);
    planConstants(content.mRefConstants, true);

    // Explicitly copied entries are always pasted as new entries (that is the copy the
    // user asked for), renamed when the name is taken. Includes are the exception: a
    // location cannot be meaningfully renamed, so an already-present include is reused.
    for (DataTypeCustom* entry : content.mDataTypes.getElements())
    {
        const QString& name = entry->getName();
        const bool conflict = (doc.getDataTypes().findElement(name) != nullptr) || claimedTypes.contains(name);
        if (conflict == false)
        {
            claimedTypes.insert(name);
        }

        mRegistry.append(new TPasteRegistryEntry<DataTypeCustom*>(entry, &_dataTypesOf, eDocElementKind::DataType
                                                                  , eRenameRole::None, conflict));
    }

    content.mDataTypes.removeAllElements();

    planAttributes(content.mAttributes, false);
    planEvents(content.mEvents, false);
    planTimers(content.mTimers, false);
    planMethods(content.mMethods, false);
    planConstants(content.mConstants, false);

    for (const IncludeEntry& entry : content.mIncludes.getElements())
    {
        if (doc.getIncludes().findElement(entry.getName()) != nullptr)
        {
            continue;
        }

        mRegistry.append(new TPasteRegistryEntry<IncludeEntry>(entry, &_includesOf, eDocElementKind::Include
                                                               , eRenameRole::None, false));
    }

    content.mIncludes.removeAllElements();
}

void SMPasteCommand::allocateContent(SMStateData& level)
{
    StateMachineData& doc = data();

    SMClipboard::RenameMaps renames;
    for (const RegistryEntryBase* item : mRegistry)
    {
        QHash<QString, QString>* map = (item->mConflict ? _mapForRole(renames, item->mRole) : nullptr);
        if (map != nullptr)
        {
            map->insert(item->mName, item->mFinalName);
        }
    }

    QSet<uint32_t> topStateOldIds;
    QSet<uint32_t> topTxOldIds;
    for (PastedState& pasted : mStates)
    {
        pasted.state->setParent(&level);
        SMClipboard::reallocateIds(*pasted.state, doc, mOldToNew);
        pasted.newId = pasted.state->getId();
        topStateOldIds.insert(pasted.oldId);
        for (uint32_t oldTx : pasted.oldTxIds)
        {
            pasted.newTxIds.append(mOldToNew.value(oldTx));
            topTxOldIds.insert(oldTx);
        }
    }

    // Transition targets reference states by ID: re-point them to the pasted copies' new IDs.
    // A target outside the pasted set is not in the map and becomes internal.
    for (PastedState& pasted : mStates)
    {
        SMClipboard::remapTransitionTargets(*pasted.state, mOldToNew);
    }

    // State names are document-unique: a collision renames the copy with its fresh ID
    // as suffix; references to a renamed copied state remap inside the pasted set only.
    QSet<QString> usedNames;
    for (PastedState& pasted : mStates)
    {
        QList<SMStateEntry*> all;
        SMClipboard::collectStates(*pasted.state, all);
        for (SMStateEntry* state : all)
        {
            const QString base = state->getName();
            if ((doc.findState(base) == nullptr) && (usedNames.contains(base) == false))
            {
                usedNames.insert(base);
                continue;
            }

            QString candidate = base + QStringLiteral("_") + QString::number(state->getId());
            while ((doc.findState(candidate) != nullptr) || usedNames.contains(candidate))
            {
                candidate += QStringLiteral("_") + QString::number(state->getId());
            }

            renames.states.insert(base, candidate);
            state->setName(candidate);
            usedNames.insert(candidate);
        }
    }

    const bool remapNeeded = (renames.states.isEmpty() && renames.events.isEmpty() && renames.timers.isEmpty()
                              && renames.methods.isEmpty() && renames.attributes.isEmpty()
                              && renames.constants.isEmpty()) == false;
    if (remapNeeded)
    {
        for (PastedState& pasted : mStates)
        {
            SMClipboard::remapReferences(*pasted.state, renames);
        }
    }

    for (const SMLayoutNode& node : mSrcNodes)
    {
        const auto found = mOldToNew.constFind(node.owner);
        if (found == mOldToNew.constEnd())
        {
            continue;
        }

        SMLayoutNode copy = node;
        copy.owner = found.value();
        if (topStateOldIds.contains(node.owner))
        {
            copy.x += mOffset.x();
            copy.y += mOffset.y();
        }

        mNodes.append(copy);
    }

    for (const SMLayoutEdge& edge : mSrcEdges)
    {
        const auto found = mOldToNew.constFind(edge.owner);
        if (found == mOldToNew.constEnd())
        {
            continue;
        }

        SMLayoutEdge copy = edge;
        copy.owner = found.value();
        if (topTxOldIds.contains(edge.owner))
        {
            for (QPointF& point : copy.points)
            {
                point += mOffset;
            }

            if (copy.hasLabel)
            {
                copy.label += mOffset;
            }
        }

        mEdges.append(copy);
    }

    for (const SMLayoutNote& note : mSrcNotes)
    {
        SMLayoutNote copy = note;
        copy.id    = doc.getNextId();
        copy.level = mOldToNew.value(note.level, mTargetLevel);
        copy.owner = mOldToNew.value(note.owner, 0u);
        if (copy.level == mTargetLevel)
        {
            copy.x += mOffset.x();
            copy.y += mOffset.y();
        }

        mNotes.append(copy);
    }

    for (const PastedState& pasted : mStates)
    {
        mPastedIds.append(pasted.newId);
    }

    for (const SMLayoutNote& note : mNotes)
    {
        if ((note.owner == 0u) && (note.level == mTargetLevel))
        {
            mPastedIds.append(note.id);
        }
    }

    mAllocated = true;
}

void SMPasteCommand::insertLayout()
{
    SMLayoutData& layout = data().getLayout();
    for (const SMLayoutNode& node : mNodes)
    {
        layout.addNode(node.owner) = node;
    }

    for (const SMLayoutEdge& edge : mEdges)
    {
        layout.addEdge(edge.owner) = edge;
    }

    for (const SMLayoutNote& note : mNotes)
    {
        layout.restoreNote(note);
    }
}

void SMPasteCommand::notifyInserted()
{
    for (const PastedState& pasted : mStates)
    {
        notifier().notifyElementAdded(pasted.newId, eDocElementKind::State);
    }

    for (const PastedState& pasted : mStates)
    {
        for (uint32_t transitionId : pasted.newTxIds)
        {
            notifier().notifyElementAdded(transitionId, eDocElementKind::Transition);
        }
    }

    for (const SMLayoutNote& note : mNotes)
    {
        notifier().notifyElementAdded(note.id, eDocElementKind::Note);
    }
}

void SMPasteCommand::redo()
{
    SMStateData* level = data().findLevel(mTargetLevel);
    if ((level == nullptr) && ((mStates.isEmpty() && mSrcNotes.isEmpty()) == false))
    {
        return;
    }

    const bool first = (mAllocated == false);
    for (RegistryEntryBase* item : mRegistry)
    {
        item->insertEntry(data(), notifier(), first);
    }

    if (first && (level != nullptr))
    {
        allocateContent(*level);
    }
    else if (first)
    {
        mAllocated = true;
    }

    if (first)
    {
        for (const RegistryEntryBase* item : mRegistry)
        {
            mPastedIds.append(item->mId);
        }
    }

    for (PastedState& pasted : mStates)
    {
        pasted.state->setParent(level);
        level->addElement(pasted.state, false);
        pasted.owned = false;
    }

    insertLayout();
    notifyInserted();
}

void SMPasteCommand::undo()
{
    SMLayoutData& layout = data().getLayout();
    for (const SMLayoutNote& note : mNotes)
    {
        layout.removeNote(note.id);
    }

    QList<uint32_t> owned;
    for (auto it = mOldToNew.constBegin(); it != mOldToNew.constEnd(); ++it)
    {
        owned.append(it.value());
    }

    layout.removeOwned(owned);

    SMStateData* level = data().findLevel(mTargetLevel);
    if (level != nullptr)
    {
        for (int i = mStates.size() - 1; i >= 0; --i)
        {
            PastedState& pasted = mStates[i];
            if (level->removeElement(pasted.newId, &pasted.state))
            {
                pasted.owned = true;
            }
        }
    }

    for (int i = mRegistry.size() - 1; i >= 0; --i)
    {
        mRegistry[i]->removeEntry(data(), notifier());
    }

    for (int i = mStates.size() - 1; i >= 0; --i)
    {
        const PastedState& pasted = mStates.at(i);
        for (uint32_t transitionId : pasted.newTxIds)
        {
            notifier().notifyElementRemoved(transitionId, eDocElementKind::Transition);
        }

        notifier().notifyElementRemoved(pasted.newId, eDocElementKind::State);
    }

    for (const SMLayoutNote& note : mNotes)
    {
        notifier().notifyElementRemoved(note.id, eDocElementKind::Note);
    }
}
