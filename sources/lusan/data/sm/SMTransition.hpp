#ifndef LUSAN_DATA_SM_SMTRANSITION_HPP
#define LUSAN_DATA_SM_SMTRANSITION_HPP
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
 *  \file        lusan/data/sm/SMTransition.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transitions.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"
#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMOperation.hpp"

#include <QString>

/**
 * \class   SMTransitionEntry
 * \brief   One transition: a reaction of its owning state to a stimulus.
 *          Owned by (nested inside) its source state — there is no `From`. `To` names a
 *          sibling state (external transition); absent `To` means an internal transition.
 *          A transition names exactly one stimulus via `StimulusKind` + `Stimulus`, and
 *          owns an optional condition list and an operation list.
 **/
class SMTransitionEntry : public DocumentElem
{
public:
    /**
     * \enum    eStimulusKind
     * \brief   The stimulus registry that `Stimulus` is resolved.
     **/
    enum class eStimulusKind
    {
          Trigger   //!< A trigger method.
        , Event     //!< An internal event.
        , Timer     //!< A timer expiration.
    };

    static constexpr const char* const  STR_KIND_TRIGGER    { "Trigger" };
    static constexpr const char* const  STR_KIND_EVENT      { "Event"   };
    static constexpr const char* const  STR_KIND_TIMER      { "Timer"   };

    static SMTransitionEntry::eStimulusKind fromKindString(const QString& kind);
    static const char* toString(SMTransitionEntry::eStimulusKind kind);

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMTransitionEntry(ElementBase* parent = nullptr);

    SMTransitionEntry(  uint32_t id
                      , eStimulusKind kind
                      , const QString& stimulus
                      , ElementBase* parent = nullptr);

    SMTransitionEntry(const SMTransitionEntry& src);
    SMTransitionEntry(SMTransitionEntry&& src) noexcept;
    virtual ~SMTransitionEntry() = default;

    SMTransitionEntry& operator = (const SMTransitionEntry& other);
    SMTransitionEntry& operator = (SMTransitionEntry&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline eStimulusKind getStimulusKind() const;
    inline void setStimulusKind(eStimulusKind kind);

    inline const QString& getStimulus() const;
    inline void setStimulus(const QString& stimulus);

    /**
     * \brief   The target sibling state name; empty when this is an internal transition.
     **/
    inline const QString& getTo() const;
    inline void setTo(const QString& target);
    inline void clearTo();

    /**
     * \brief   True for an external transition (a `To` target is present).
     **/
    inline bool isExternal() const;

    inline const QString& getDescription() const;
    inline void setDescription(const QString& description);

    inline const SMConditionList& getConditions() const;
    inline SMConditionList& getConditions();

    /**
     * \brief   The transition's guard: the ID-bound resolved expression
     *          tree, a draft, or empty. This is the canonical guard storage; the legacy
     *          `getConditions()` tree is a read-shim only (see SMGuardParser::fromLegacy).
     **/
    inline const SMGuard& getGuard() const;
    inline SMGuard& getGuard();

    inline const SMOperationList& getOperations() const;
    inline SMOperationList& getOperations();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The transition's display name — its stimulus; required by the container.
     **/
    inline const QString& getName() const;

    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    eStimulusKind   mStimulusKind;  //!< The stimulus kind.
    QString         mStimulus;      //!< The stimulus name.
    QString         mTo;            //!< The target sibling state (empty = internal).
    bool            mHasTo;         //!< Whether a target is present.
    QString         mDescription;   //!< The description text.
    SMConditionList mConditions;    //!< Legacy condition tree (read-shim only; not written when a guard exists).
    SMGuard         mGuard;         //!< The canonical guard (ID-bound tree / draft / empty).
    SMOperationList mOperations;    //!< The transition operations.
};

//////////////////////////////////////////////////////////////////////////
// SMTransitionData class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMTransitionData
 * \brief   A state's `TransitionList`: an ordered, owning container of
 *          transitions. Document order is priority order. Held by pointer
 *          for stable addresses; deletes its transitions on destruction.
 **/
class SMTransitionData : public TEDataContainer<SMTransitionEntry*, DocumentElem>
{
public:
    SMTransitionData(ElementBase* parent = nullptr);
    SMTransitionData(const SMTransitionData& src);
    SMTransitionData(SMTransitionData&& src) noexcept;
    virtual ~SMTransitionData();

    SMTransitionData& operator = (const SMTransitionData& other);
    SMTransitionData& operator = (SMTransitionData&& other) noexcept;

    /**
     * \brief   Creates a new transition appended at the end (lowest priority).
     * \param   kind        The stimulus kind.
     * \param   stimulus    The stimulus name.
     * \param   target      The target sibling state, or empty for an internal transition.
     * \return  Pointer to the created transition.
     **/
    SMTransitionEntry* createTransition(SMTransitionEntry::eStimulusKind kind, const QString& stimulus, const QString& target = QString());

    /**
     * \brief   Deletes and removes all transitions.
     **/
    void removeAll();

    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

private:
    //!< Deep-copies the transitions of \p src into this (empty) list, re-parenting them.
    void cloneFrom(const SMTransitionData& src);
};

//////////////////////////////////////////////////////////////////////////
// SMTransitionEntry inline methods
//////////////////////////////////////////////////////////////////////////

inline SMTransitionEntry::eStimulusKind SMTransitionEntry::getStimulusKind() const
{
    return mStimulusKind;
}

inline void SMTransitionEntry::setStimulusKind(eStimulusKind kind)
{
    mStimulusKind = kind;
}

inline const QString& SMTransitionEntry::getStimulus() const
{
    return mStimulus;
}

inline void SMTransitionEntry::setStimulus(const QString& stimulus)
{
    mStimulus = stimulus;
}

inline const QString& SMTransitionEntry::getTo() const
{
    return mTo;
}

inline void SMTransitionEntry::setTo(const QString& target)
{
    mTo = target;
    mHasTo = true;
}

inline void SMTransitionEntry::clearTo()
{
    mTo.clear();
    mHasTo = false;
}

inline bool SMTransitionEntry::isExternal() const
{
    return mHasTo;
}

inline const QString& SMTransitionEntry::getDescription() const
{
    return mDescription;
}

inline void SMTransitionEntry::setDescription(const QString& description)
{
    mDescription = description;
}

inline const SMConditionList& SMTransitionEntry::getConditions() const
{
    return mConditions;
}

inline SMConditionList& SMTransitionEntry::getConditions()
{
    return mConditions;
}

inline const SMGuard& SMTransitionEntry::getGuard() const
{
    return mGuard;
}

inline SMGuard& SMTransitionEntry::getGuard()
{
    return mGuard;
}

inline const SMOperationList& SMTransitionEntry::getOperations() const
{
    return mOperations;
}

inline SMOperationList& SMTransitionEntry::getOperations()
{
    return mOperations;
}

inline const QString& SMTransitionEntry::getName() const
{
    return mStimulus;
}

#endif  // LUSAN_DATA_SM_SMTRANSITION_HPP
