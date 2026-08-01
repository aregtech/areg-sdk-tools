#ifndef LUSAN_DATA_SM_SMSTATE_HPP
#define LUSAN_DATA_SM_SMSTATE_HPP
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
 *  \file        lusan/data/sm/SMState.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM states and the recursive state list
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMTransition.hpp"

#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMStateData;

/**
 * \class   SMStateEntry
 * \brief   One state. Kinds: Start, Normal, Final. A Normal state may be
 *          composite in exactly one of two ways: painted in place (it owns a nested
 *          StateList) or imported (it carries a `Submachine` alias) — the two are
 *          mutually exclusive. A composite may carry `History` and `OnFinal`.
 **/
class SMStateEntry : public DocumentElem
{
public:
    /**
     * \enum    eStateKind
     * \brief   The kind of state
     **/
    enum class eStateKind
    {
          Start     //!< The level's entry PSEUDO-state; exactly one per level.
        , Normal    //!< A regular state.
        , Final     //!< The terminal state of its level.
    };

    /**
     * \enum    eHistory
     * \brief   Composite-state history mode.
     **/
    enum class eHistory
    {
          None      //!< Every entry descends via the Start chain (default).
        , Shallow   //!< Re-entry activates the last active direct substate.
        , Deep      //!< Re-entry restores the entire last active path to the leaf.
    };

    /**
     * \brief   The smallest legal `DoList@Interval`. A `Do` activity is a timer loop and
     *          nothing else, so it must say how often it ticks: `Interval="0"` -- which used to
     *          mean "run on every handled trigger" -- and an absent `Interval` are both refused
     *          (\ref SMValidator rule 29). Reacting to one named stimulus without leaving the
     *          state is what an INTERNAL TRANSITION is for, and it names which stimulus, which
     *          a `Do` never could.
     **/
    static constexpr uint32_t           MIN_DO_INTERVAL     { 1u };

    /**
     * \brief   The interval a `DoList` created in the editor starts with, so a new activity is
     *          valid the moment it exists. A document is never given this silently: what it
     *          says is what is stored, and an interval it does not have stays 0 so validation
     *          can report it (\ref MIN_DO_INTERVAL).
     **/
    static constexpr uint32_t           DEFAULT_DO_INTERVAL { 1000u };

    static constexpr const char* const  STR_KIND_START      { "Start"   };
    static constexpr const char* const  STR_KIND_NORMAL     { "Normal"  };
    static constexpr const char* const  STR_KIND_FINAL      { "Final"   };
    static constexpr const char* const  STR_HISTORY_NONE    { "None"    };
    static constexpr const char* const  STR_HISTORY_SHALLOW { "Shallow" };
    static constexpr const char* const  STR_HISTORY_DEEP    { "Deep"    };

    static SMStateEntry::eStateKind fromKindString(const QString& kind);
    static const char* toString(SMStateEntry::eStateKind kind);
    static SMStateEntry::eHistory fromHistoryString(const QString& history);
    static const char* toString(SMStateEntry::eHistory history);

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMStateEntry(ElementBase* parent = nullptr);
    SMStateEntry(uint32_t id, const QString& name, eStateKind kind, ElementBase* parent = nullptr);
    SMStateEntry(const SMStateEntry& src);
    SMStateEntry(SMStateEntry&& src) noexcept;
    virtual ~SMStateEntry();

    SMStateEntry& operator = (const SMStateEntry& other);
    SMStateEntry& operator = (SMStateEntry&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline const QString& getName() const;
    inline void setName(const QString& name);

    inline eStateKind getKind() const;
    inline void setKind(eStateKind kind);

    /**
     * \brief   True when this is the level's `Start` PSEUDO-state.
     *
     *          A `Start` is not a state: it is a decorative marker saying where a level begins.
     *          It never becomes a state in the generated code and the machine never occupies it,
     *          so it carries no operations and no description-worthy behaviour, it is never the
     *          target of a transition, and its own outgoing transitions -- the level's INITIAL
     *          transitions -- name no stimulus. They may carry a guard, which is the only thing
     *          allowed to decide between them, and document order is priority order. When no
     *          guard holds no substate is entered and the machine rests in the parent state.
     **/
    inline bool isPseudoStart() const;

    /**
     * \brief   True when the state carries any entry, exit or `do` operation.
     **/
    inline bool hasOperations() const;

    /**
     * \brief   True when this is a LEGACY merged-form `Start`: a document written before the
     *          pseudo-state rule spelled the level's first REAL state as `Kind="Start"`, giving
     *          it operations and stimulus-driven transitions. Detected by exactly those two
     *          marks -- any operation, or any outgoing transition naming a stimulus -- and
     *          rewritten on load into a pseudo-state plus a demoted `Normal` state
     *          (\ref StateMachineData::readFromXml).
     **/
    bool isLegacyMergedStart() const;

    inline eHistory getHistory() const;
    inline void setHistory(eHistory history);

    /**
     * \brief   The imported-submachine alias (empty when not imported). Setting it clears
     *          any painted nested StateList — the two are mutually exclusive.
     **/
    inline const QString& getSubmachine() const;
    void setSubmachine(const QString& alias);
    inline bool isImportedSubmachine() const;

    inline const QString& getOnFinal() const;
    inline void setOnFinal(const QString& event);

    inline const QString& getDescription() const;
    inline void setDescription(const QString& description);

    inline const SMOperationList& getEntryList() const;
    inline SMOperationList& getEntryList();
    inline const SMOperationList& getExitList() const;
    inline SMOperationList& getExitList();

    /**
     * \brief   The `do/` activity operations -- a TIMER LOOP and nothing else. The timer starts
     *          when the state is entered, after its entry operations, and stops when the state
     *          is exited, before its exit operations; the first tick is one \ref getDoInterval
     *          AFTER entry, never at entry (work that must happen on arrival belongs in
     *          \ref getEntryList). On a composite the activity runs for as long as the composite
     *          is active -- while a descendant is active, and equally while none is. Empty when
     *          the state has no activity, in which case no timer is generated at all.
     **/
    inline const SMOperationList& getDoList() const;
    inline SMOperationList& getDoList();

    /**
     * \brief   The `do/` tick period in milliseconds; at least \ref MIN_DO_INTERVAL. 0 is what a
     *          document that omits `Interval` -- or that still carries the removed trigger-driven
     *          `Interval="0"` -- reads back as, and it is kept exactly as read so validation can
     *          report it instead of silently inventing a period. The runtime semantics are
     *          realized by the external code generator, not by Lusan.
     **/
    inline uint32_t getDoInterval() const;
    inline void setDoInterval(uint32_t interval);

    /**
     * \brief   The optional `do/` stop condition: the same resolved, ID-bound guard tree a
     *          transition guard is, so a rename of an attribute it references re-renders it
     *          instead of quietly breaking it. It is tested BEFORE each tick and it stops the
     *          activity for the REST OF THE VISIT -- the timer is stopped and only a fresh entry
     *          to the state starts it again -- so the operations never run once it holds. Empty
     *          means the activity runs until the state is exited.
     *
     *          An author who wants a resumable activity writes the test inside the operation and
     *          sets no `Until`; that is why the permanent reading is the one stored -- it can
     *          express the other, and a re-evaluating `Until` has no way to say "stop for good".
     **/
    inline const SMGuard& getDoUntil() const;
    inline SMGuard& getDoUntil();
    inline void setDoUntil(const SMGuard& until);

    /**
     * \brief   The verbatim `DoList@Until` text of a document written before the stop condition
     *          became a tree, kept exactly as read and NOT resolved here: the data layer does not
     *          know the guard grammar. \ref StateMachineModel parses it once on load into
     *          \ref getDoUntil -- a single `Raw` node when it cannot be parsed, which is what the
     *          free text already was -- and then clears it. Empty in every other case.
     **/
    inline const QString& getDoUntilLegacy() const;
    inline void clearDoUntilLegacy();

    inline const SMTransitionData& getTransitions() const;
    inline SMTransitionData& getTransitions();

    /**
     * \brief   True if the state owns a painted nested StateList.
     **/
    inline bool hasNestedStates() const;

    /**
     * \brief   True when the state carries a submachine at all, painted here or imported.
     *          History and the completion hook need one, and this is the single definition
     *          the validator and every editor surface agree on.
     **/
    inline bool isComposite() const;

    /**
     * \brief   Returns the painted nested StateList, or nullptr if none.
     **/
    inline const SMStateData* getNestedStates() const;
    inline SMStateData* getNestedStates();

    /**
     * \brief   Creates (if needed) and returns the painted nested StateList. Clears any
     *          imported-submachine alias — the two are mutually exclusive.
     **/
    SMStateData* getOrCreateNestedStates();

    /**
     * \brief   Detaches the painted nested StateList and transfers its ownership to the
     *          caller; the state becomes non-composite.
     * \return  The detached list, or nullptr if the state had none.
     **/
    SMStateData* takeNestedStates();

    /**
     * \brief   Attaches a painted nested StateList, taking ownership. Replaces any
     *          existing list and clears the imported-submachine alias.
     * \param   nested  The list to attach; nullptr detaches nothing and leaves the
     *                  state non-composite.
     **/
    void attachNestedStates(SMStateData* nested);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString             mName;          //!< The state name (document-unique.
    eStateKind          mKind;          //!< The state kind.
    eHistory            mHistory;       //!< The history mode (composite states).
    QString             mSubmachine;    //!< The imported-submachine alias (or empty).
    QString             mOnFinal;       //!< The completion-hook event (or empty).
    QString             mDescription;   //!< The description text.
    SMOperationList     mEntryList;     //!< Operations executed on entry, in order.
    SMOperationList     mExitList;      //!< Operations executed on exit, in order.
    SMOperationList     mDoList;        //!< Operations repeated while active (the `do/` activity).
    uint32_t            mDoInterval;    //!< The `do/` tick period in ms (0 = the document said none).
    SMGuard             mDoUntil;       //!< The optional `do/` stop condition (an ID-bound tree).
    QString             mDoUntilLegacy; //!< The pre-L3 free-text `Until`, awaiting the load shim.
    SMTransitionData    mTransitions;   //!< The outgoing transitions (priority order).
    SMStateData*        mNested;        //!< The painted nested StateList (owned) or nullptr.
};

//////////////////////////////////////////////////////////////////////////
// SMStateData class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMStateData
 * \brief   A machine level's `StateList`: the recursive, owning container of
 *          states. The root document owns one at level 0; every painted composite state
 *          owns a nested one. States are held by pointer for stable addresses and are
 *          deleted on destruction.
 **/
class SMStateData : public TEDataContainer<SMStateEntry*, DocumentElem>
{
public:
    SMStateData(ElementBase* parent = nullptr);
    SMStateData(const SMStateData& src);
    SMStateData(SMStateData&& src) noexcept;
    virtual ~SMStateData();

    SMStateData& operator = (const SMStateData& other);
    SMStateData& operator = (SMStateData&& other) noexcept;

    /**
     * \brief   Creates a new state appended at the end of this level.
     * \param   name    The unique name of the new state.
     * \param   kind    The state kind.
     * \return  Pointer to the created state, or nullptr if the name already exists here.
     **/
    SMStateEntry* createState(const QString& name, SMStateEntry::eStateKind kind);

    /**
     * \brief   Finds a state by name in this level only.
     **/
    SMStateEntry* findState(const QString& name) const;

    /**
     * \brief   Finds a state by name across this level and every nested level:
     *          state names are unique within the whole document).
     **/
    SMStateEntry* findStateRecursive(const QString& name) const;

    /**
     * \brief   Finds a state by element ID in this level only.
     **/
    SMStateEntry* findStateById(uint32_t id) const;

    /**
     * \brief   Finds a state by element ID across this level and every nested level.
     **/
    SMStateEntry* findStateByIdRecursive(uint32_t id) const;

    /**
     * \brief   Returns the single Start state of this level, or nullptr if none.
     **/
    SMStateEntry* getStartState() const;

    /**
     * \brief   Finds the state whose transition list owns the transition with the given
     *          ID, searching this level and every nested level.
     **/
    SMStateEntry* findTransitionOwnerRecursive(uint32_t transitionId) const;

    /**
     * \brief   Counts the states in this level and all nested levels.
     **/
    int countStatesRecursive() const;

    /**
     * \brief   True when this level owns at least one Normal state (a composite counts, being
     *          Normal-kind). A level with only Start/Final markers, or an empty one, is not a
     *          "real" submachine: it exists only in RAM while being built and is never persisted
     *          -- writeToXml omits it and its layout, so it is discarded on save/reload.
     **/
    bool hasRealState() const;

    /**
     * \brief   Deletes and removes all states of this level (recursively via ownership).
     **/
    void removeAll();

    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

private:
    //!< Deep-copies the states of \p src into this (empty) level, re-parenting them.
    void cloneFrom(const SMStateData& src);
};

//////////////////////////////////////////////////////////////////////////
// SMStateEntry inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& SMStateEntry::getName() const
{
    return mName;
}

inline void SMStateEntry::setName(const QString& name)
{
    mName = name;
}

inline SMStateEntry::eStateKind SMStateEntry::getKind() const
{
    return mKind;
}

inline void SMStateEntry::setKind(eStateKind kind)
{
    mKind = kind;
}

inline bool SMStateEntry::isPseudoStart() const
{
    return (mKind == eStateKind::Start);
}

inline bool SMStateEntry::hasOperations() const
{
    return ((mEntryList.isEmpty() == false) || (mExitList.isEmpty() == false) || (mDoList.isEmpty() == false));
}

inline SMStateEntry::eHistory SMStateEntry::getHistory() const
{
    return mHistory;
}

inline void SMStateEntry::setHistory(eHistory history)
{
    mHistory = history;
}

inline const QString& SMStateEntry::getSubmachine() const
{
    return mSubmachine;
}

inline bool SMStateEntry::isImportedSubmachine() const
{
    return (mSubmachine.isEmpty() == false);
}

inline const QString& SMStateEntry::getOnFinal() const
{
    return mOnFinal;
}

inline void SMStateEntry::setOnFinal(const QString& event)
{
    mOnFinal = event;
}

inline const QString& SMStateEntry::getDescription() const
{
    return mDescription;
}

inline void SMStateEntry::setDescription(const QString& description)
{
    mDescription = description;
}

inline const SMOperationList& SMStateEntry::getEntryList() const
{
    return mEntryList;
}

inline SMOperationList& SMStateEntry::getEntryList()
{
    return mEntryList;
}

inline const SMOperationList& SMStateEntry::getExitList() const
{
    return mExitList;
}

inline SMOperationList& SMStateEntry::getExitList()
{
    return mExitList;
}

inline const SMOperationList& SMStateEntry::getDoList() const
{
    return mDoList;
}

inline SMOperationList& SMStateEntry::getDoList()
{
    return mDoList;
}

inline uint32_t SMStateEntry::getDoInterval() const
{
    return mDoInterval;
}

inline void SMStateEntry::setDoInterval(uint32_t interval)
{
    mDoInterval = interval;
}

inline const SMGuard& SMStateEntry::getDoUntil() const
{
    return mDoUntil;
}

inline SMGuard& SMStateEntry::getDoUntil()
{
    return mDoUntil;
}

inline void SMStateEntry::setDoUntil(const SMGuard& until)
{
    mDoUntil = until;
}

inline const QString& SMStateEntry::getDoUntilLegacy() const
{
    return mDoUntilLegacy;
}

inline void SMStateEntry::clearDoUntilLegacy()
{
    mDoUntilLegacy.clear();
}

inline const SMTransitionData& SMStateEntry::getTransitions() const
{
    return mTransitions;
}

inline SMTransitionData& SMStateEntry::getTransitions()
{
    return mTransitions;
}

inline bool SMStateEntry::hasNestedStates() const
{
    return (mNested != nullptr);
}

inline const SMStateData* SMStateEntry::getNestedStates() const
{
    return mNested;
}

inline SMStateData* SMStateEntry::getNestedStates()
{
    return mNested;
}

inline bool SMStateEntry::isComposite() const
{
    return (mNested != nullptr) || (mSubmachine.isEmpty() == false);
}

#endif  // LUSAN_DATA_SM_SMSTATE_HPP
