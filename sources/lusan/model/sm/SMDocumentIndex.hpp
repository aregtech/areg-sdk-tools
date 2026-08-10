#ifndef LUSAN_MODEL_SM_SMDOCUMENTINDEX_HPP
#define LUSAN_MODEL_SM_SMDOCUMENTINDEX_HPP
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
 *  \file        lusan/model/sm/SMDocumentIndex.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM document lookup service (declarations and stimulus scope).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMMethodKind.hpp"
#include "lusan/data/sm/SMTransition.hpp"

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class ConstantEntry;
class AttributeEntry;
class SMEventEntry;
class IncludeEntry;
class SMTimerEntry;
class StateMachineData;

/**
 * \class   SMDocumentIndex
 * \brief   The one place a caller asks an `.fsml` document what it declares. Before this
 *          existed, "find the method / event / attribute / constant called X" was written out
 *          by hand in a dozen modules, each with its own loop and its own idea of what a match
 *          means. The index answers all of those questions once, so a change to a lookup rule
 *          lands in one file instead of twelve.
 *
 *          Plain lookups go straight to the registry, so building an index costs nothing and a
 *          single question through it is exactly as cheap as the old hand-written loop. What
 *          the index does keep is the expensive work: resolving a transition by ID walks the
 *          whole state tree, and the stimulus payload behind it needs a second registry hit.
 *          Guard rendering, validation and the symbol catalog ask that same question once per
 *          node, so one index shared across the pass turns a repeated tree walk into a single
 *          one.
 *
 *          Treat it as a snapshot: build it, run one batch of queries, drop it. It holds a
 *          reference to the document and caches resolved pointers, so keeping one alive across
 *          an edit would hand out stale answers.
 **/
class SMDocumentIndex
{
//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \struct  Stimulus
     * \brief   One entry of the shared stimulus name space: a trigger method, an internal
     *          event, or a timer, paired with the kind that tells the three apart.
     **/
    struct Stimulus
    {
        SMTransitionEntry::eStimulusKind    kind;   //!< Which registry the name comes from.
        QString                             name;   //!< The declared name.
    };

    /**
     * \class   ParamScope
     * \brief   The parameters a transition's stimulus puts in scope, as one object instead of
     *          the parallel name list and type list that callers used to fetch separately and
     *          then index by position. A timer expiry and an unresolved stimulus both give an
     *          empty scope, so callers need no special case for them.
     **/
    class ParamScope
    {
    public:
        explicit ParamScope(const MethodBase* payload = nullptr);

        //!< The declaration whose parameters are in scope, or nullptr when nothing is.
        inline const MethodBase* payload() const;

        //!< The in-scope parameters in declared order; empty when the scope is empty.
        const QList<MethodParameter>& parameters() const;

        inline bool isEmpty() const;
        inline int count() const;

        //!< The in-scope parameter named \p name, or nullptr.
        const MethodParameter* byName(const QString& name) const;

        //!< The in-scope parameter with the document ID \p id, or nullptr.
        const MethodParameter* byId(uint32_t id) const;

        //!< The in-scope parameter names, in declared order.
        QStringList names() const;

        //!< The declared types of the in-scope parameters, parallel to \ref names.
        QStringList types() const;

    private:
        const MethodBase*   mPayload;   //!< The trigger or event carrying the parameters.
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMDocumentIndex(const StateMachineData& data);

    SMDocumentIndex(const SMDocumentIndex& src) = delete;
    SMDocumentIndex& operator = (const SMDocumentIndex& other) = delete;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline const StateMachineData& document() const;

    // ---- Declarations by name ---------------------------------------------

    //!< The first method named \p name whatever its kind, or nullptr.
    const MethodEntry* method(const QString& name) const;

    //!< The method named \p name of exactly \p kind; a name is unique only per kind.
    const MethodEntry* method(const QString& name, int kind) const;

    const SMEventEntry* event(const QString& name) const;
    const SMTimerEntry* timer(const QString& name) const;
    const AttributeEntry* attribute(const QString& name) const;
    const ConstantEntry* constant(const QString& name) const;
    //!< The registered machine whose alias is \p name; matched by alias, never by location.
    const IncludeEntry* import(const QString& name) const;

    // ---- Declarations by document ID --------------------------------------

    const MethodEntry* method(uint32_t id) const;
    const SMEventEntry* event(uint32_t id) const;
    const SMTimerEntry* timer(uint32_t id) const;
    const AttributeEntry* attribute(uint32_t id) const;
    const ConstantEntry* constant(uint32_t id) const;
    const IncludeEntry* import(uint32_t id) const;

    // ---- Grouped views ----------------------------------------------------

    /**
     * \brief   Every method of \p kind in document order. Cached for the index's lifetime,
     *          so a picker that asks per row pays for the filter once.
     **/
    QList<const MethodEntry*> methodsOf(int kind) const;

    /**
     * \brief   The shared stimulus name space in the order the pickers show it: triggers
     *          first, then events, then timers. Both the properties panel and the canvas
     *          stimulus dialog read this list, so the two can never offer different choices.
     *          Returned by value (the list is implicitly shared) so a caller cannot end up
     *          holding a reference into an index that has already gone out of scope.
     **/
    QList<Stimulus> stimuli() const;

    // ---- Transition metadata ----------------------------------------------

    //!< The transition with the document ID \p transitionId, or nullptr. Cached: the raw
    //!< lookup walks the whole state tree.
    const SMTransitionEntry* transition(uint32_t transitionId) const;

    //!< The parameters the stimulus of \p transitionId puts in scope. Cached.
    ParamScope paramScope(uint32_t transitionId) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    const StateMachineData&                             mData;      //!< The document under query.
    mutable QHash<int, QList<const MethodEntry*>>     mByKind;    //!< Method groups already filtered.
    mutable QList<Stimulus>                             mStimuli;   //!< The merged stimulus name space.
    mutable bool                                        mStimuliReady;
    mutable QHash<uint32_t, const SMTransitionEntry*>   mTransitions;   //!< Transitions already found.
    mutable QHash<uint32_t, const MethodBase*>          mScopes;    //!< Stimulus payload per transition.
};

//////////////////////////////////////////////////////////////////////////
// SMDocumentIndex inline methods
//////////////////////////////////////////////////////////////////////////

inline const MethodBase* SMDocumentIndex::ParamScope::payload() const
{
    return mPayload;
}

inline bool SMDocumentIndex::ParamScope::isEmpty() const
{
    return parameters().isEmpty();
}

inline int SMDocumentIndex::ParamScope::count() const
{
    return static_cast<int>(parameters().size());
}

inline const StateMachineData& SMDocumentIndex::document() const
{
    return mData;
}

#endif  // LUSAN_MODEL_SM_SMDOCUMENTINDEX_HPP
