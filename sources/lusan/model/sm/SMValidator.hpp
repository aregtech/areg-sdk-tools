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
#include "lusan/model/common/DocRuleChecks.hpp"

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
     * \brief   A warning `n` is reported with the rule id (`WARNING_RULE_BASE + n`)
     **/
    static constexpr int WARNING_RULE_BASE { DocRuleChecks::ADVISORY_RULE_BASE };

    /**
     * \brief   An information `n` is reported with the rule id (`INFORMATION_RULE_BASE + n`).
     *          A band, not a second severity of the same rule: the id the author reads is the
     *          rule identity, so `n`, `WARNING_RULE_BASE + n` and `INFORMATION_RULE_BASE + n`
     *          are three unrelated rules.
     **/
    static constexpr int INFORMATION_RULE_BASE { 200 };

    /**
     * \brief   The argument-to-parameter mapping faults.
     **/
    static constexpr int RULE_ARGUMENT_MAPPING { 10 };

    /**
     * \brief   A name that is already taken: two entries of the same kind in one registry, a
     *          repeated parameter name, or a stimulus name claimed by a trigger, an event and
     *          a timer at once. Reported on every entry after the first, so the finding names
     *          the copy the author has to rename.
     **/
    static constexpr int RULE_DUPLICATE_NAME { 4 };

    /**
     * \brief   A name the generated code could not carry: it must start with a letter or an
     *          underscore and continue with letters, digits or underscores. A declaration with
     *          no name at all is the same fault and carries the same number.
     **/
    static constexpr int RULE_INVALID_IDENTIFIER { 5 };

    /**
     * \brief   A name referenced here and declared nowhere of that kind: a target, a stimulus,
     *          an action, a timer, an event, an attribute, a constant, a condition operand, or
     *          a declared data type.
     **/
    static constexpr int RULE_UNRESOLVED_REFERENCE { 6 };

    /**
     * \brief   A value that does not read as its declared type: a malformed literal, a name
     *          that is not an enumerator of its enumeration, or a literal on a type that has
     *          no literal form.
     **/
    static constexpr int RULE_BAD_LITERAL { 15 };

    /**
     * \brief   Advisory: nothing in the machine uses the declaration. Reported in the advisory
     *          band, so the emitted id is `WARNING_RULE_BASE + RULE_UNREFERENCED` and never
     *          the bare number, which belongs to \a RULE_DUPLICATE_NAME.
     **/
    static constexpr int RULE_UNREFERENCED { 4 };

    /**
     * \brief   An element with no description. Advisory: the document still generates, but the
     *          generated element carries no comment.
     **/
    static constexpr int RULE_MISSING_DESCRIPTION { 14 };

    /**
     * \brief   Two enumerators of one enumeration counting the same, so a value read back
     *          cannot be told apart from the other one.
     **/
    static constexpr int RULE_DUPLICATE_ENUM_VALUE { 40 };

    /**
     * \brief   Advisory: the machine, or a declaration in it, is marked deprecated.
     **/
    static constexpr int RULE_DEPRECATED { 41 };

    /**
     * \brief   An included data type document that could not be read, so every type it was to
     *          contribute is missing here.
     **/
    static constexpr int RULE_BROKEN_IMPORT { 42 };

    /**
     * \brief   Advisory: an included data type document the machine takes no type from.
     **/
    static constexpr int RULE_UNUSED_IMPORT { 43 };

    /**
     * \brief   Advisory: the machine declares one name and lives in a file called another. Both
     *          are allowed -- the generated files follow the declared name -- but worth saying.
     **/
    static constexpr int RULE_FILE_NAME_MISMATCH { 44 };

    /**
     * \brief   The rule guard findings are filed under. The guard checker owns the grammar and
     *          the symbol binding, but its findings are collected into the one document run.
     **/
    static constexpr int RULE_GUARD { 25 };

    /**
     * \brief   The `Kind="Start"` pseudo-state faults: operations on a Start, a stimulus on
     *          one of its initial transitions, a Start nothing leaves, a Start something enters
     *          (its own transition included), two or more initial transitions where any carries
     *          no condition, and a root Start that does not have exactly one unconditional
     *          transition. One id, because they are one rule (a Start is not a state) and
     *          because the code generator has to file the same faults under the same number.
     **/
    static constexpr int RULE_PSEUDO_START { 27 };

    /**
     * \brief   The transition `Kind`: an `External` transition with no target
     *          (the unfinished edge that used to be indistinguishable from an internal one),
     *          an `Internal` one that names a target, an `Initial` one with no target or with a stimulus,
     *          an `External`/`Internal` one with no stimulus, an `Initial` transition on
     *          a state that is not a `Kind="Start"`, and a `Start` owning anything other than `Initial` transitions.
     *
     *          One id, because they are one rule (`Kind` says what the transition is, and `To`
     *          and `Stimulus` then mean only what they say) and because the code generator has
     *          to file the same faults under the same number.
     **/
    static constexpr int RULE_TRANSITION_KIND { 28 };

    /**
     * \brief   An element of the opened file the format does not place: an unknown tag, a real
     *          one written where the format does not allow it, or one the format has dropped.
     *          The block is kept as written, so a document carrying one still opens and still
     *          saves without losing it.
     **/
    static constexpr int RULE_UNKNOWN_ELEMENT { 34 };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Runs every check over the document and returns the findings in document
     *          order (states and levels first, then the registries).
     **/
    static QList<SMIssue> validate(const StateMachineData& data);

    /**
     * \brief   The field a finding blames, for the checks that know it. Takes the rule id as
     *          stored on the finding (\ref DocIssue::rule), advisory offset included, so a
     *          caller hands over `issue.rule` untouched.
     * \return  \a eIssueField::None when the check names no single field.
     **/
    static eIssueField fieldOfRule(int rule);

    /**
     * \brief   Why a finding is a finding, and what resolves it, in the author's terms. Empty
     *          when the message already says everything there is to say.
     **/
    static QString explainRule(int rule, DocIssue::eSeverity severity);
};

#endif  // LUSAN_MODEL_SM_SMVALIDATOR_HPP
