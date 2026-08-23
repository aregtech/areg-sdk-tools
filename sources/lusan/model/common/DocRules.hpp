#ifndef LUSAN_MODEL_COMMON_DOCRULES_HPP
#define LUSAN_MODEL_COMMON_DOCRULES_HPP
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
 *  \file        lusan/model/common/DocRules.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the rule numbers every document kind reports under.
 *
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
// Generated from sources/lusan/res/rules/rules.xml. Do not edit by hand:
// the build regenerates this file and fails when the two disagree. Change
// the rule there, and both this tool and the code generator follow.
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The rule numbers, for every document kind at once.
 *
 *          A rule number is what an author reads and what the code generator files the same
 *          fault under, so one fault carries one number wherever it is found: in a state
 *          machine, in a service interface and in a data type document alike. They live here
 *          rather than in the three engines so that a number cannot mean one thing in one
 *          engine and something else in another.
 *
 *          The number below is the rule identity. The id a finding carries adds the band of
 *          its severity: an error keeps the bare number, a warning adds
 *          \a DocRuleChecks::WARNING_RULE_BASE, and information adds the information base.
 *          `4`, `104` and `204` are therefore three unrelated rules, not one rule at three
 *          severities, and a number reserved for one band is not free in another.
 **/
namespace DocRules
{
    /**
     * \brief   A level with no `Kind="Start"` state, or with more than one of them.
     **/
    constexpr int RULE_START_STATE          {  1 };

    /**
     * \brief   One element ID claimed by more than one element, so every reference to it binds
     *          to whichever was read first.
     **/
    constexpr int RULE_DUPLICATE_ID         {  2 };

    /**
     * \brief   A state name that is not unique in the document.
     **/
    constexpr int RULE_STATE_NAME           {  3 };

    /**
     * \brief   A name that is already taken: two entries of the same kind in one registry, a
     *          repeated parameter name, or a stimulus name claimed by a trigger, an event and a
     *          timer at once. Reported on every entry after the first, so the finding names the
     *          copy the author has to rename. The code generator reports it for two included
     *          documents that declare one name.
     **/
    constexpr int RULE_DUPLICATE_NAME       {  4 };

    /**
     * \brief   A declaration nothing in the document uses, or an included document it takes
     *          nothing from. Never an error -- code that nothing reaches still generates -- so
     *          the bare number is reserved and only the banded ids are ever reported.
     **/
    constexpr int RULE_UNREFERENCED         { 26 };

    /**
     * \brief   A document that declares no version. The version reaches the generated code and
     *          tells a client which contract it was built against. Every document kind carries
     *          one, so the fault is shared rather than owned by a single kind.
     **/
    constexpr int RULE_MISSING_VERSION      { 29 };

    /**
     * \brief   A name the generated code could not carry: it must start with a letter or an
     *          underscore and continue with letters, digits or underscores. A declaration with
     *          no name at all is the same fault and carries the same number.
     **/
    constexpr int RULE_INVALID_IDENTIFIER   {  5 };

    /**
     * \brief   A declared data type that answers to nothing: the type of an attribute, a
     *          parameter, a constant, a structure field, or a container key or value. The field
     *          to correct is the type itself, which is what tells it apart from a reference to a
     *          declared element (\a RULE_UNRESOLVED_ELEMENT).
     **/
    constexpr int RULE_UNRESOLVED_TYPE      {  6 };

    /**
     * \brief   A transition target that is not a sibling of the state it leaves.
     **/
    constexpr int RULE_TARGET_SIBLING       {  7 };

    /**
     * \brief   A `Final` state with outgoing transitions, or with substates.
     **/
    constexpr int RULE_FINAL_STATE          {  8 };

    /**
     * \brief   A `Kind="Start"` state that owns substates.
     **/
    constexpr int RULE_START_SUBSTATES      {  9 };

    /**
     * \brief   The argument-to-parameter mapping faults.
     **/
    constexpr int RULE_ARGUMENT_MAPPING     { 10 };

    /**
     * \brief   A submachine level with no `Kind="Start"` state. The root level is \a
     *          RULE_START_STATE; a nested one is told apart so the message can name the level
     *          that is missing its beginning.
     **/
    constexpr int RULE_NESTED_START         { 11 };

    /**
     * \brief   A value source or a guard operand that is out of scope where it stands.
     **/
    constexpr int RULE_SOURCE_SCOPE         { 12 };

    /**
     * \brief   An argument type with no conversion to the parameter it binds to, or a comparison
     *          between two types with no implicit conversion. A conversion that only narrows is
     *          a warning, and the generated code casts explicitly.
     **/
    constexpr int RULE_ARGUMENT_TYPE        { 13 };

    /**
     * \brief   A structure or container operand in a comparison, or an ordering operator on
     *          `bool`, `String` or an enumeration.
     **/
    constexpr int RULE_COMPARE_OPERAND      { 14 };

    /**
     * \brief   Advisory: a declaration with no description, so the generated element carries no
     *          comment. Only ever banded -- the bare number belongs to \a RULE_COMPARE_OPERAND.
     **/
    constexpr int RULE_MISSING_DESCRIPTION  { 14 };

    /**
     * \brief   A value that does not read as its declared type: a malformed literal, a name that
     *          is not an enumerator of its enumeration, or a literal on a type that has no
     *          literal form.
     **/
    constexpr int RULE_BAD_LITERAL          { 15 };

    /**
     * \brief   A predicate operand tested on its own that is not `bool`.
     **/
    constexpr int RULE_BOOLEAN_OPERAND      { 16 };

    /**
     * \brief   An `AttributeSet` whose source type does not convert to the attribute's type. A
     *          conversion that only narrows is a warning, and the generated code casts
     *          explicitly.
     **/
    constexpr int RULE_ATTRIBUTE_TYPE       { 17 };

    /**
     * \brief   A state both painted and imported, a `Submachine` on a `Start` or a `Final`
     *          state, `History` or `OnFinal` on a state that is not composite, and an import
     *          that carries no alias.
     **/
    constexpr int RULE_STATE_SHAPE          { 18 };

    /**
     * \brief   An include that does not resolve: no file named, the file missing, unreadable, a
     *          cycle, or nested too deep. An included data type document that could not be read
     *          is the same fault -- every type it was to contribute is missing. The editor
     *          always refuses; the code generator softens it to a warning, or to information
     *          when the generated output of the missing document is already on disk and is used
     *          as it stands.
     **/
    constexpr int RULE_BROKEN_IMPORT        { 19 };

    /**
     * \brief   An `Embedded` condition with an empty body, a body on a handler, or a body or a
     *          `Return` on a method that is not a condition.
     **/
    constexpr int RULE_CONDITION_BODY       { 20 };

    /**
     * \brief   A parameterized condition named as a value source, which only a left operand may
     *          be.
     **/
    constexpr int RULE_PARAMETERIZED_COND   { 21 };

    /**
     * \brief   An import whose pinned version differs from the file in its MAJOR part.
     **/
    constexpr int RULE_IMPORT_MAJOR         { 22 };

    /**
     * \brief   A value source the format does not define, or one written where another kind of
     *          row belongs.
     **/
    constexpr int RULE_SOURCE_KIND          { 23 };

    /**
     * \brief   A value source with nothing in it.
     **/
    constexpr int RULE_SOURCE_EMPTY         { 24 };

    /**
     * \brief   The rule guard findings are filed under. The guard checker owns the grammar and
     *          the symbol binding, but its findings are collected into the one document run.
     **/
    constexpr int RULE_GUARD                { 25 };

    /**
     * \brief   The `Kind="Start"` pseudo-state faults: operations on a Start, a stimulus on one
     *          of its initial transitions, a Start nothing leaves, a Start something enters (its
     *          own transition included), two or more initial transitions where any carries no
     *          condition, and a root Start that does not have exactly one unconditional
     *          transition. One id, because they are one rule (a Start is not a state) and
     *          because the code generator has to file the same faults under the same number.
     **/
    constexpr int RULE_PSEUDO_START         { 27 };

    /**
     * \brief   The transition `Kind`: an `External` transition with no target (the unfinished
     *          edge that used to be indistinguishable from an internal one), an `Internal` one
     *          that names a target, an `Initial` one with no target or with a stimulus, an
     *          `External`/`Internal` one with no stimulus, an `Initial` transition on a state
     *          that is not a `Kind="Start"`, and a `Start` owning anything other than `Initial`
     *          transitions. One id, because they are one rule (`Kind` says what the transition
     *          is, and `To` and `Stimulus` then mean only what they say) and because the code
     *          generator has to file the same faults under the same number.
     **/
    constexpr int RULE_TRANSITION_KIND      { 28 };

    /**
     * \brief   Two hosted machines whose action handler parameters flatten to one name.
     **/
    constexpr int RULE_HANDLER_NAME         { 31 };

    /**
     * \brief   An attribute and a trigger that share a name, so both reach one function.
     **/
    constexpr int RULE_ATTRIBUTE_STIMULUS   { 32 };

    /**
     * \brief   A name that would place a member under a reserved prefix it does not own.
     **/
    constexpr int RULE_RESERVED_PREFIX      { 33 };

    /**
     * \brief   An element of the opened file the format does not place: an unknown tag, a real
     *          one written where the format does not allow it, or one the format has dropped.
     *          The block is kept as written, so a document carrying one still opens and still
     *          saves without losing it.
     **/
    constexpr int RULE_UNKNOWN_ELEMENT      { 34 };

    /**
     * \brief   Advisory: a parameter carrying the name of its own trigger or condition. The bare
     *          number belongs to \a RULE_UNKNOWN_ELEMENT; this one is only ever banded.
     **/
    constexpr int RULE_PARAM_SHADOWS        { 34 };

    /**
     * \brief   Advisory: a response no request leads to. Only ever banded -- the bare number is
     *          not in use.
     **/
    constexpr int RULE_UNBOUND_RESPONSE     { 36 };

    /**
     * \brief   A parameter carrying a default that another parameter follows.
     **/
    constexpr int RULE_DEFAULT_ORDER        { 38 };

    /**
     * \brief   Advisory: a condition whose name already begins with the prefix an action method
     *          carries. Only ever banded.
     **/
    constexpr int RULE_ACTION_PREFIX        { 39 };

    /**
     * \brief   A reference to a declared element that is not there: a trigger, an event, a
     *          timer, an action, an attribute, a constant, a condition, a parameter, or the
     *          alias of a hosted machine. The field to correct is the name that was written,
     *          which is what tells it apart from a data type that answers to nothing (\a
     *          RULE_UNRESOLVED_TYPE).
     **/
    constexpr int RULE_UNRESOLVED_ELEMENT   { 46 };

    /**
     * \brief   A data type document whose include list names another document. It declares types
     *          and nothing else, and its include list carries C++ headers.
     **/
    constexpr int RULE_NOT_A_HEADER         { 40 };

    /**
     * \brief   Two enumerators of one enumeration counting the same, so a value read back cannot
     *          be told apart from the other one.
     **/
    constexpr int RULE_DUPLICATE_ENUM_VALUE { 41 };

    /**
     * \brief   A declaration its author marked deprecated. Never an error -- a deprecation may
     *          not block a build -- so the bare number is reserved and only the banded ids are
     *          ever reported.
     **/
    constexpr int RULE_DEPRECATED           { 43 };

    /**
     * \brief   Advisory: a structure or an enumeration with no members. It generates an empty
     *          declaration, which compiles, so the note stands only until the first member is
     *          added.
     **/
    constexpr int RULE_EMPTY_TYPE           { 47 };

    /**
     * \brief   Advisory: the document declares nothing at all, so what includes it or connects
     *          to it gains nothing.
     **/
    constexpr int RULE_EMPTY_DOCUMENT       { 48 };

    /**
     * \brief   The document declares one name and lives in a file called another. Both are
     *          allowed -- the generated files follow the declared name -- but worth saying.
     **/
    constexpr int RULE_FILE_NAME_MISMATCH   { 49 };

    /**
     * \brief   A request whose Response names nothing, or names a method the document declares
     *          as something other than a response. The field to correct is the response the
     *          request is answered by.
     **/
    constexpr int RULE_RESPONSE_LINK        { 51 };

    /**
     * \brief   An element the format used to define in this place and no longer does. Told apart
     *          from a tag the format never had, because the author is moving something rather
     *          than correcting a spelling, and the finding can name what to do instead.
     **/
    constexpr int RULE_RETIRED_ELEMENT      { 30 };

//////////////////////////////////////////////////////////////////////////
// Rules that exist only in a band
//
// The rule number is the identity within one band, so a rule reported only as a warning may
// carry a number an unrelated error already uses. `1` is a missing Start state and `101` is a
// state nothing reaches: two rules, two bands, one bare number. The constants below name the
// banded half so that no call site or explanation is written against a bare integer.
//////////////////////////////////////////////////////////////////////////

    /**
     * \brief   Warning 101: a state no transition targets and no Start enters, so nothing it
     *          does ever runs.
     **/
    constexpr int RULE_UNREACHABLE_STATE    {  1 };

    /**
     * \brief   Warning 102: a state with no outgoing transition. The machine stays in it once it
     *          is entered.
     **/
    constexpr int RULE_DEAD_END_STATE       {  2 };

    /**
     * \brief   Warning 103: a transition an earlier unconditional transition on the same
     *          stimulus always takes first.
     **/
    constexpr int RULE_SHADOWED_TRANSITION  {  3 };

    /**
     * \brief   Warning 105: an event that is sent and never reacted to, or reacted to and never
     *          sent.
     **/
    constexpr int RULE_ONE_SIDED_EVENT      {  5 };

    /**
     * \brief   Warning 106: a timer that is started and never reacted to, or reacted to and
     *          never started.
     **/
    constexpr int RULE_ONE_SIDED_TIMER      {  6 };

    /**
     * \brief   Warning 107: an internal transition that carries no operation and no condition,
     *          so reacting to the stimulus changes nothing.
     **/
    constexpr int RULE_EMPTY_INTERNAL       {  7 };

    /**
     * \brief   Warning 109: a comparison whose operands are both fixed at design time, so the
     *          result is decided before the machine runs.
     **/
    constexpr int RULE_CONSTANT_COMPARE     {  9 };

    /**
     * \brief   Warning 110: a state carrying History that no transition ever re-enters, so the
     *          remembered substate is never restored.
     **/
    constexpr int RULE_UNUSED_HISTORY       { 10 };

    /**
     * \brief   Warning 112 and information 212: an import pinned to a version the file no longer
     *          carries, where the difference is below the major version.
     **/
    constexpr int RULE_IMPORT_PATCH         { 12 };

//////////////////////////////////////////////////////////////////////////
// The registry, and the check that keeps it honest
//////////////////////////////////////////////////////////////////////////

    /**
     * \enum    eBand
     * \brief   The severity bands a rule may be reported in. A rule number identifies one fault
     *          within one band, so the same number in two bands is two unrelated rules.
     **/
    enum eBand : unsigned int
    {
          BandError         = 1u << 0   //!< Reported as an error, under the bare number.
        , BandWarning       = 1u << 1   //!< Reported as a warning, under the number plus 100.
        , BandInformation   = 1u << 2   //!< Reported as information, under the number plus 200.
    };

    /**
     * \struct  Rule
     * \brief   One rule: its number, and every band it is reported in.
     **/
    struct Rule
    {
        int             number; //!< The rule number, without any band applied.
        unsigned int    bands;  //!< The bands it is reported in, as a set of \ref eBand flags.
    };

    /**
     * \brief   Every rule the three validators emit, with the bands each is reported in.
     *
     *          The numbers come from the constants above rather than being written again, so a
     *          row cannot drift from the rule it describes. The bands are read from the severity
     *          each rule is emitted at.
     **/
    constexpr Rule REGISTRY[]
    {
          { RULE_START_STATE         , BandError }
        , { RULE_DUPLICATE_ID        , BandError }
        , { RULE_STATE_NAME          , BandError }
        , { RULE_DUPLICATE_NAME      , BandError | BandWarning }
        , { RULE_UNREFERENCED        , BandWarning | BandInformation }
        , { RULE_MISSING_VERSION     , BandError }
        , { RULE_INVALID_IDENTIFIER  , BandError }
        , { RULE_UNRESOLVED_TYPE     , BandError }
        , { RULE_TARGET_SIBLING      , BandError }
        , { RULE_FINAL_STATE         , BandError }
        , { RULE_START_SUBSTATES     , BandError }
        , { RULE_ARGUMENT_MAPPING    , BandError }
        , { RULE_NESTED_START        , BandError }
        , { RULE_SOURCE_SCOPE        , BandError }
        , { RULE_ARGUMENT_TYPE       , BandError | BandWarning }
        , { RULE_COMPARE_OPERAND     , BandError }
        , { RULE_MISSING_DESCRIPTION , BandInformation }
        , { RULE_BAD_LITERAL         , BandError }
        , { RULE_BOOLEAN_OPERAND     , BandError }
        , { RULE_ATTRIBUTE_TYPE      , BandError | BandWarning }
        , { RULE_STATE_SHAPE         , BandError }
        , { RULE_BROKEN_IMPORT       , BandError }
        , { RULE_CONDITION_BODY      , BandError }
        , { RULE_PARAMETERIZED_COND  , BandError }
        , { RULE_IMPORT_MAJOR        , BandError }
        , { RULE_SOURCE_KIND         , BandError }
        , { RULE_SOURCE_EMPTY        , BandError }
        , { RULE_GUARD               , BandError | BandWarning | BandInformation }
        , { RULE_PSEUDO_START        , BandError }
        , { RULE_TRANSITION_KIND     , BandError }
        , { RULE_HANDLER_NAME        , BandError }
        , { RULE_ATTRIBUTE_STIMULUS  , BandError }
        , { RULE_RESERVED_PREFIX     , BandError }
        , { RULE_UNKNOWN_ELEMENT     , BandError }
        , { RULE_PARAM_SHADOWS       , BandWarning }
        , { RULE_UNBOUND_RESPONSE    , BandWarning }
        , { RULE_DEFAULT_ORDER       , BandError }
        , { RULE_ACTION_PREFIX       , BandWarning }
        , { RULE_UNRESOLVED_ELEMENT  , BandError }
        , { RULE_NOT_A_HEADER        , BandError }
        , { RULE_DUPLICATE_ENUM_VALUE, BandError }
        , { RULE_DEPRECATED          , BandWarning | BandInformation }
        , { RULE_EMPTY_TYPE          , BandWarning }
        , { RULE_EMPTY_DOCUMENT      , BandWarning }
        , { RULE_FILE_NAME_MISMATCH  , BandInformation }
        , { RULE_RESPONSE_LINK       , BandError }
        , { RULE_UNREACHABLE_STATE   , BandWarning }
        , { RULE_DEAD_END_STATE      , BandWarning }
        , { RULE_SHADOWED_TRANSITION , BandWarning }
        , { RULE_ONE_SIDED_EVENT     , BandWarning }
        , { RULE_ONE_SIDED_TIMER     , BandWarning }
        , { RULE_EMPTY_INTERNAL      , BandWarning }
        , { RULE_CONSTANT_COMPARE    , BandWarning }
        , { RULE_UNUSED_HISTORY      , BandWarning }
        , { RULE_IMPORT_PATCH        , BandWarning | BandInformation }
        , { RULE_RETIRED_ELEMENT     , BandError }
    };

    /**
     * \brief   True when no two rules report the same number at the same severity.
     *
     *          Two rules may share a number in different bands, which is what makes `1` a
     *          missing Start state and `101` a state nothing reaches. Sharing a number within
     *          one band means one reported id stands for two faults, which no reader can tell
     *          apart.
     **/
    constexpr bool registryIsUnique()
    {
        constexpr int count = static_cast<int>(sizeof(REGISTRY) / sizeof(REGISTRY[0]));
        for (int i = 0; i < count; ++i)
        {
            for (int j = i + 1; j < count; ++j)
            {
                if ((REGISTRY[i].number == REGISTRY[j].number)
                    && ((REGISTRY[i].bands & REGISTRY[j].bands) != 0u))
                {
                    return false;
                }
            }
        }

        return true;
    }

    static_assert(registryIsUnique()
                , "Two rules report the same number at the same severity. One reported id would "
                  "stand for two different faults. Give one of them a number of its own, and give "
                  "the same number to the code generator.");

//////////////////////////////////////////////////////////////////////////
// Elements a format has retired
//////////////////////////////////////////////////////////////////////////

    /**
     * \struct  Retired
     * \brief   One element a format used to define in a place it no longer defines it, and what
     *          the author writes instead.
     **/
    struct Retired
    {
        const char* tag;        //!< The element as the older document spells it.
        const char* parent;     //!< The element it used to sit in, empty when it stood anywhere.
        const char* documents;  //!< The document extensions it was retired from, space separated.
        const char* fix;        //!< What to do instead, in words the author can act on.
    };

    /**
     * \brief   Every retired element, so one mistake reads as one finding.
     *
     *          Without this an older document produces two findings that do not look related:
     *          an element nobody recognises, and then whatever that element used to supply
     *          reported as missing. Matched here, the reader gets one finding that names the fix.
     **/
    constexpr Retired RETIRED[]
    {
          { "Value", "Constant", "siml", "Write it as the Value attribute, or run tools/migrate-siml-constants" }
    };
}

#endif  // LUSAN_MODEL_COMMON_DOCRULES_HPP