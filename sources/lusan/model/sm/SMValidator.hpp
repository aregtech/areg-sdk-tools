#ifndef LUSAN_MODEL_SM_SMVALIDATOR_HPP
#define LUSAN_MODEL_SM_SMVALIDATOR_HPP
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
 *  \file        lusan/model/sm/SMValidator.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM structural and reference validation engine.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocIssue.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \brief   An FSM finding is a document finding -- there is one finding type for every
 *          document kind (\ref DocIssue). The name is kept because it reads better at the
 *          FSM call sites and because the rule ids below are FSM rule ids.
 **/
using SMIssue = DocIssue;

/**
 * \class   SMValidator
 * \brief   Checks a document's structure and its cross-references: start-state placement,
 *          duplicate identifiers and names, identifier syntax, resolution of every
 *          name-based reference (targets, stimuli, actions, timers, events, attributes,
 *          data types, condition operands and argument sources), argument-to-parameter
 *          matching, parameter-scope, composite-state constraints, and condition-row shape.
 *
 *          It is a pure, headless transform -- input a `const StateMachineData&`, output a
 *          `QList<SMIssue>` -- with no widget, model, or scheduling dependency, so the code
 *          generator can re-run it and block on errors exactly as the editor runs it in the
 *          background. Type compatibility, literal parsing, and import resolution are the
 *          concern of separate validators.
 **/
class SMValidator
{
//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   A 10.2 warning `n` is reported with the rule id (`WARNING_RULE_BASE + n`); the
     *          error rule ids stay the plain 10.1 numbers. The two numbering spaces share the
     *          single `SMIssue::rule` field, so the warning offset keeps them from colliding
     *          (warning 2 and error 2 are distinguishable by id, not only by severity).
     *
     *          A 10.1 rule whose SEVERITY depends on what it found is offset too, under its own
     *          10.1 number: a narrowing conversion is rule 13 (17 for an `AttributeSet`) reported
     *          as a warning, so it arrives as 113 (117). Id 113 therefore has two producers --
     *          the narrowing, and 10.2 rule 13's threading advisory -- told apart by the element
     *          each blames. A finding in that position should carry its own `DocIssue::detail`
     *          rather than rely on a display table keyed by the number alone.
     **/
    static constexpr int WARNING_RULE_BASE { 100 };

    /**
     * \brief   The 10.1 rule that argument-to-parameter mapping faults are reported under.
     *          Named because the check lives in \ref SMOperationValidation (one implementation,
     *          called both document-wide and per canvas element) and must file its findings
     *          under the same rule the rest of the engine uses.
     **/
    static constexpr int RULE_ARGUMENT_MAPPING { 10 };

    /**
     * \brief   The rule guard findings are filed under. The guard checker owns the grammar and
     *          the symbol binding, but its findings are collected into the one document run.
     **/
    static constexpr int RULE_GUARD { 25 };

    /**
     * \brief   The 10.1 rule the `Kind="Start"` pseudo-state faults are filed under, all of them,
     *          the way rule 25 collects every guard fault: operations on a Start, a stimulus on
     *          one of its initial transitions, a Start nothing leaves, a Start something enters
     *          (its own transition included), two or more initial transitions where any carries
     *          no condition, and a ROOT Start that does not have exactly one unconditional
     *          transition. One id, because they are one rule -- a Start is not a state -- and
     *          because the code generator has to file the same faults under the same number.
     **/
    static constexpr int RULE_PSEUDO_START { 27 };

    /**
     * \brief   The 10.1 rule the transition `Kind` contract is filed under: an `External`
     *          transition with no target (the unfinished edge that used to be indistinguishable
     *          from an internal one), an `Internal` one that names a target, an `Initial` one
     *          with no target or with a stimulus, an `External`/`Internal` one with no stimulus,
     *          an `Initial` transition on a state that is not a `Kind="Start"`, and a `Start`
     *          owning anything other than `Initial` transitions.
     *
     *          One id, because they are one rule -- `Kind` says what the transition is, and `To`
     *          and `Stimulus` then mean only what they say -- and because the code generator has
     *          to file the same faults under the same number.
     **/
    static constexpr int RULE_TRANSITION_KIND { 28 };

    /**
     * \brief   The 10.1 rule the `DoList` contract is filed under. A `Do` activity is a TIMER
     *          LOOP and nothing else, so it must say how often it ticks: `Interval="0"` -- the
     *          removed trigger-driven mode -- and an absent `Interval` are both refused, naming
     *          the state.
     *
     *          Reacting to one stimulus without leaving the state is what an INTERNAL TRANSITION
     *          is for, and it names WHICH stimulus, which a `Do` never could: its operations get
     *          no stimulus context, so a trigger-driven activity was the same idea with the
     *          discrimination removed, and how often it ran depended on unrelated traffic. The
     *          document that still carries one is not converted silently -- there is no way to
     *          guess which stimulus was meant -- it is refused so the author re-authors it.
     *
     *          The stop condition is a guard, so its own faults stay rule 25 with every other
     *          predicate's; only the timer is this rule's business.
     **/
    static constexpr int RULE_DO_ACTIVITY { 29 };

    /**
     * \brief   The rule a transition that can never fire is filed under: a descendant reacting to a
     *          stimulus one of its ancestors already reacts to with no guard.
     *
     *          A composite's transitions are eligible while the composite or ANY of its descendants
     *          is active, and a composite's candidate is tried before its children's. The first
     *          candidate whose guard holds wins, and a candidate with no guard always holds -- so an
     *          unguarded ancestor candidate on a stimulus makes every descendant candidate on that
     *          same stimulus dead: it is emitted, it compiles, it never runs.
     *
     *          A GUARDED ancestor candidate is not a fault and produces no finding. The descendant's
     *          then fires whenever the ancestor's guard is false, which is the normal way to write
     *          "handle this here unless the outer condition applies".
     *
     *          The finding blames the descendant's transition -- the one that will never fire -- and
     *          names the ancestor whose candidate wins, because either end is a valid place to fix
     *          it: guard the ancestor's, or delete the descendant's.
     **/
    static constexpr int RULE_ANCESTOR_SHADOW { 30 };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Runs every check over the document and returns the findings in document
     *          order (states and levels first, then the registries).
     **/
    static QList<SMIssue> validate(const StateMachineData& data);
};

#endif  // LUSAN_MODEL_SM_SMVALIDATOR_HPP
