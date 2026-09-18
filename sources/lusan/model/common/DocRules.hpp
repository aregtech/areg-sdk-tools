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
// Generated from sources/lusan/res/schema/rules.xml. Do not edit by hand:
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
     *          documents that declare one name. Two attributes of one interface or one machine
     *          whose generated accessors are one function are the same fault: an attribute name
     *          is converted to snake_case rather than copied, so `Count` and `count`, and
     *          `my_Value` and `my_value`, each pair reaches one `count()` and one `my_value()`.
     **/
    constexpr int RULE_DUPLICATE_NAME       {  4 };

    /**
     * \brief   A declaration nothing in the document uses, or an included document it takes
     *          nothing from. Never an error -- code that nothing reaches still generates -- so
     *          the bare number is reserved and only the banded ids are ever reported. The
     *          warning is for a document that can answer the question, because what it declares
     *          is its own. A data type document is judged against the documents that include it
     *          instead; one run sees only the consumers it was pointed at, so a type none of
     *          them names is information and not a warning, and a data type document generated
     *          with no consumer at all says nothing.
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
     *          no name at all is the same fault and carries the same number. A name that is a
     *          word C++ owns is the same fault wherever the generated code spells the name as
     *          the document writes it: a document, an interface, a machine, a type, an
     *          enumerator, a field, a constant, a parameter, a state, a trigger, a condition or
     *          a timer. A request, a response, a broadcast, an action and an event are written
     *          behind a prefix, so a keyword is accepted there. An attribute is judged by the
     *          accessor it generates instead of by its spelling: `Class` produces `class()`,
     *          which is a keyword.
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
     * \brief   A transition target that is not a sibling of the state it leaves. One exception:
     *          a transition from outside a composite targeting that composite's Kind="History"
     *          pseudo-state is legal even though the pseudo-state is not a sibling either -- the
     *          same target reached from inside its own level is refused instead, as
     *          RULE_HISTORY_SIBLING (error 57).
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
     *          literal form. The code generator reports one in a service interface or a data
     *          type document as a warning and generates the value as written.
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
     *          The block is kept as written while the document is open, so a document carrying
     *          one still opens, and it is dropped when the document is saved.
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
     *          timer, an action, an attribute, a constant, a condition, a parameter, the alias
     *          of a hosted machine, or the state a transition names as its target. The field to
     *          correct is the name that was written, which is what tells it apart from a data
     *          type that answers to nothing (\a RULE_UNRESOLVED_TYPE) and from a target that
     *          does exist but sits on another level (\a RULE_TARGET_SIBLING).
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
     * \brief   The document states a FormatVersion the reader does not know. An error only when
     *          the document is newer by a MAJOR, which says the document is shaped differently
     *          and would generate code describing a document nobody wrote. A warning when the
     *          document is newer by a MINOR or a PATCH: a minor only adds, and anything the
     *          reader does not recognise -- a section, an element, an attribute, a value the
     *          schema beside it does not allow -- is refused where it is met rather than dropped
     *          in silence, so the document is read as it stands and the difference is said
     *          aloud. A warning too when the document is older by a MAJOR, which is a migration
     *          and not a fault. An older minor or patch is readable by design and says nothing.
     **/
    constexpr int RULE_FORMAT_VERSION       { 50 };

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

    /**
     * \brief   A response that answers more than one request. Legal and sometimes meant, so it
     *          is information and never a warning.
     **/
    constexpr int RULE_SHARED_RESPONSE      { 37 };

    /**
     * \brief   An attribute the format does not define on an element it does define. Nothing
     *          reads it, so it reaches no generated code, and it is dropped when the document is
     *          saved.
     **/
    constexpr int RULE_UNKNOWN_ATTRIBUTE    { 52 };

    /**
     * \brief   An element block the format does not place, kept while the document is open and
     *          dropped when it is saved. Reported beside the fault itself so the loss is stated
     *          before it happens.
     **/
    constexpr int RULE_DROPPED_ELEMENT      { 53 };

    /**
     * \brief   A Kind="History" state carrying an EntryList, an ExitList, a TransitionList or a
     *          nested StateList. It is a pseudo-state, not a state: the machine never occupies
     *          it, so it owns none of the things a state owns.
     **/
    constexpr int RULE_HISTORY_SHAPE        { 54 };

    /**
     * \brief   More than one Kind="History" state on one level. At most one per level -- a
     *          second one would leave two markers naming the same re-entry.
     **/
    constexpr int RULE_HISTORY_DUPLICATE    { 55 };

    /**
     * \brief   A Kind="History" state at the root level. There is no composite there to resume,
     *          so the marker names nothing.
     **/
    constexpr int RULE_HISTORY_ROOT         { 56 };

    /**
     * \brief   A transition targeting a Kind="History" state from inside its own level. The
     *          marker is reached only by a transition entering the composite from outside (\a
     *          RULE_TARGET_SIBLING, widened); from inside, the state it names is an ordinary
     *          non-sibling target.
     **/
    constexpr int RULE_HISTORY_SIBLING      { 57 };

    /**
     * \brief   A composite carrying both its own History attribute and a Kind="History" child.
     *          Two spellings of one decision, disagreeing about what the composite does on
     *          re-entry.
     **/
    constexpr int RULE_HISTORY_CONFLICT     { 58 };

    /**
     * \brief   A HashMap or a Map whose key type cannot be a key. A HashMap finds a key by its
     *          hash and a Map keeps its keys ordered, so the key has to hash or to order. A
     *          primitive, an enumeration, `String` and `WideString` do both; `DateTime` orders
     *          and does not hash; `BinaryBuffer` and a container do neither; a structure does
     *          what every one of its fields does, and an empty structure does both. A type
     *          declared with Type="Imported" is the author's own C++ type and is not judged.
     **/
    constexpr int RULE_CONTAINER_KEY        { 59 };

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

    /**
     * \brief   Warning 108: an operation on the `EntryList` of a `Final` state that a composite
     *          encloses, where the composite raises `OnFinal`. The `EntryList` runs as the
     *          `Final` is entered, while the machine is still inside the composite, and the
     *          queued `OnFinal` event leaves the composite only later, so the operation runs
     *          against the state the machine is leaving rather than the one it is going to. The
     *          bare number belongs to \a RULE_FINAL_STATE; this one is only ever banded.
     **/
    constexpr int RULE_FINAL_ENTRY_ORDER    {  8 };

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
     * \enum    eDocument
     * \brief   The document kinds a rule applies to. A rule may apply to more than one.
     **/
    enum eDocument : unsigned int
    {
          DocDataType       = 1u << 0   //!< Applies to a data type document.
        , DocInterface      = 1u << 1   //!< Applies to a service interface document.
        , DocStateMachine   = 1u << 2   //!< Applies to a state machine document.
    };

    /**
     * \struct  Rule
     * \brief   One rule: its number, the bands it is reported in, the documents it applies to,
     *          and the two texts a reader is shown: what is wrong, and what to change.
     **/
    struct Rule
    {
        int             number;     //!< The rule number, without any band applied.
        unsigned int    bands;      //!< The bands it is reported in, as a set of \ref eBand flags.
        unsigned int    documents;  //!< The documents it applies to, as a set of \ref eDocument flags.
        const char *    summary;    //!< What is wrong with the document.
        const char *    fix;        //!< What the author has to change.
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
          { RULE_START_STATE         , BandError, DocStateMachine
          , "A level with no 'Kind=\"Start\"' state, or with more than one of them."
          , "Give the level exactly one state with Kind=\"Start\". If it already has one, a second state "
            "carries the same Kind: change it to Kind=\"Normal\"." }
        , { RULE_DUPLICATE_ID        , BandError, DocStateMachine
          , "One element ID claimed by more than one element, so every reference to it binds to whichever was "
            "read first."
          , "Every ID in the document comes from one counter and is never reused. Give the later element the "
            "next free number, then repoint anything that referenced it." }
        , { RULE_STATE_NAME          , BandError, DocStateMachine
          , "A state name that is not unique in the document."
          , "Rename one of the two states. State names are unique across the whole document, not only inside "
            "their level: the generator flattens every level into one C++ enumeration, so a substate of one "
            "region collides with a substate of another. Prefixing the name with its region, \"Idle\" as "
            "\"PumpIdle\" and \"ValveIdle\", is the rename that keeps both readable." }
        , { RULE_DUPLICATE_NAME      , BandError | BandWarning, DocDataType | DocInterface | DocStateMachine
          , "A name that is already taken: two entries of the same kind in one registry, a repeated parameter "
            "name, or a stimulus name claimed by a trigger, an event and a timer at once. Reported on every "
            "entry after the first, so the finding names the copy the author has to rename. The code "
            "generator reports it for two included documents that declare one name. Two attributes of one "
            "interface or one machine whose generated accessors are one function are the same fault: an "
            "attribute name is converted to snake_case rather than copied, so 'Count' and 'count', and "
            "'my_Value' and 'my_value', each pair reaches one 'count()' and one 'my_value()'."
          , "Rename the later declaration. A stimulus name is shared by triggers, events and timers, so a "
            "trigger may not carry the name of an event or a timer. Two attributes collide on what they "
            "generate rather than on what they are called, so rename the later one until the two converted "
            "names differ." }
        , { RULE_UNREFERENCED        , BandWarning | BandInformation, DocDataType | DocInterface | DocStateMachine
          , "A declaration nothing in the document uses, or an included document it takes nothing from. Never "
            "an error -- code that nothing reaches still generates -- so the bare number is reserved and only "
            "the banded ids are ever reported. The warning is for a document that can answer the question, "
            "because what it declares is its own. A data type document is judged against the documents that "
            "include it instead; one run sees only the consumers it was pointed at, so a type none of them "
            "names is information and not a warning, and a data type document generated with no consumer at "
            "all says nothing."
          , "Either use the declaration or delete it. Generation continues either way; the note stands so a "
            "leftover from an edit is not mistaken for something in use." }
        , { RULE_MISSING_VERSION     , BandError, DocDataType | DocInterface | DocStateMachine
          , "A document that declares no version. The version reaches the generated code and tells a client "
            "which contract it was built against. Every document kind carries one, so the fault is shared "
            "rather than owned by a single kind."
          , "Add Version to Overview, as MAJOR.MINOR.PATCH. The version reaches the generated code and tells "
            "a consumer which contract it was built against." }
        , { RULE_INVALID_IDENTIFIER  , BandError, DocDataType | DocInterface | DocStateMachine
          , "A name the generated code could not carry: it must start with a letter or an underscore and "
            "continue with letters, digits or underscores. A declaration with no name at all is the same "
            "fault and carries the same number. A name that is a word C++ owns is the same fault wherever the "
            "generated code spells the name as the document writes it: a document, an interface, a machine, a "
            "type, an enumerator, a field, a constant, a parameter, a state, a trigger, a condition or a "
            "timer. A request, a response, a broadcast, an action and an event are written behind a prefix, "
            "so a keyword is accepted there. An attribute is judged by the accessor it generates instead of "
            "by its spelling: 'Class' produces 'class()', which is a keyword."
          , "Rewrite the name as a C++ identifier: a letter or an underscore first, then letters, digits or "
            "underscores. No spaces, dots or dashes. A name that is a C++ keyword is renamed to something "
            "that is not one, because the generated file it lands in is not a file its author may edit. For "
            "an attribute the name to change is the one the accessor is built from: 'Class' becomes "
            "'ClassName', and the accessor becomes 'class_name()'." }
        , { RULE_UNRESOLVED_TYPE     , BandError, DocDataType | DocInterface | DocStateMachine
          , "A declared data type that answers to nothing: the type of an attribute, a parameter, a constant, "
            "a structure field, or a container key or value. The field to correct is the type itself, which "
            "is what tells it apart from a reference to a declared element (RULE_UNRESOLVED_ELEMENT)."
          , "Declare the type, or correct its spelling. A type from an included document is written "
            "Space::Type, where Space is that document's Overview name. A C++ type you already own is "
            "declared once with Type=\"Imported\"." }
        , { RULE_TARGET_SIBLING      , BandError, DocStateMachine
          , "A transition target that is not a sibling of the state it leaves. One exception: a transition "
            "from outside a composite targeting that composite's Kind=\"History\" pseudo-state is legal even "
            "though the pseudo-state is not a sibling either -- the same target reached from inside its own "
            "level is refused instead, as RULE_HISTORY_SIBLING (error 57)."
          , "A transition ends on a state of its own level. To leave a composite state, draw the transition "
            "from the composite itself, not from a state inside it." }
        , { RULE_FINAL_STATE         , BandError, DocStateMachine
          , "A 'Final' state with outgoing transitions, or with substates."
          , "A Final state ends its level: remove its outgoing transitions and its substates. To continue "
            "afterwards, transition out of the composite that owns it." }
        , { RULE_START_SUBSTATES     , BandError, DocStateMachine
          , "A 'Kind=\"Start\"' state that owns substates."
          , "A Start is a pseudo-state and holds nothing: it names where the level begins and is left in the "
            "same step it is entered, so anything nested inside it can never run. Add a Kind=\"Normal\" state, "
            "move the substates into it, and let the Start's outgoing transition target it." }
        , { RULE_ARGUMENT_MAPPING    , BandError, DocStateMachine
          , "The argument-to-parameter mapping faults."
          , "Map every parameter of the called element, once, in the order it declares them, and remove any "
            "argument that maps to nothing. The generator writes a direct call, so a missing, doubled or "
            "reordered mapping is not something it can repair: read the parameter list of the target and make "
            "the argument list match it position for position." }
        , { RULE_NESTED_START        , BandError, DocStateMachine
          , "A submachine level with no 'Kind=\"Start\"' state. The root level is RULE_START_STATE; a nested "
            "one is told apart so the message can name the level that is missing its beginning."
          , "The named submachine level needs its own Kind=\"Start\" state. Every level with substates has one, "
            "not only the root." }
        , { RULE_SOURCE_SCOPE        , BandError, DocStateMachine
          , "A value source or a guard operand that is out of scope where it stands."
          , "The value named is not visible where it is used. Use a parameter of this stimulus, an attribute "
            "of the machine, or a constant -- not a parameter of another trigger or a value of another level." }
        , { RULE_ARGUMENT_TYPE       , BandError | BandWarning, DocStateMachine
          , "An argument type with no conversion to the parameter it binds to, or a comparison between two "
            "types with no implicit conversion. A conversion that only narrows is a warning, and the "
            "generated code casts explicitly."
          , "Make the two types match, or introduce a conversion of your own. A narrowing conversion is "
            "allowed and generates an explicit cast; an unrelated type is not." }
        , { RULE_COMPARE_OPERAND     , BandError, DocStateMachine
          , "A structure or container operand in a comparison, or an ordering operator on 'bool', 'String' or "
            "an enumeration."
          , "Compare scalars. A structure or a container has no ordering, and bool, String and enumerations "
            "answer only to == and !=." }
        , { RULE_MISSING_DESCRIPTION , BandInformation, DocStateMachine
          , "Advisory: a declaration with no description, so the generated element carries no comment. Only "
            "ever banded -- the bare number belongs to RULE_COMPARE_OPERAND."
          , "Add a Description to the declaration so the generated element carries a comment. Advisory: "
            "generation succeeds without it." }
        , { RULE_BAD_LITERAL         , BandError | BandWarning, DocDataType | DocInterface | DocStateMachine
          , "A value that does not read as its declared type: a malformed literal, a name that is not an "
            "enumerator of its enumeration, or a literal on a type that has no literal form. The code "
            "generator reports one in a service interface or a data type document as a warning and generates "
            "the value as written."
          , "Write the value the way its declared type reads: a number for a numeric type, true or false for "
            "bool, and an enumerator this enumeration declares." }
        , { RULE_BOOLEAN_OPERAND     , BandError, DocStateMachine
          , "A predicate operand tested on its own that is not 'bool'."
          , "A predicate tested on its own is bool, and this operand is not: C++ would convert it silently "
            "and the guard would then be true for every non-zero value. Compare it with something (\"Level > "
            "MaxLevel\" rather than \"Level\"), or call a condition whose declared type is bool." }
        , { RULE_ATTRIBUTE_TYPE      , BandError | BandWarning, DocStateMachine
          , "An 'AttributeSet' whose source type does not convert to the attribute's type. A conversion that "
            "only narrows is a warning, and the generated code casts explicitly."
          , "Set the attribute from a source of its own type. A source that only narrows is accepted with a "
            "warning and generates an explicit cast." }
        , { RULE_STATE_SHAPE         , BandError, DocStateMachine
          , "A state both painted and imported, a 'Submachine' on a 'Start' or a 'Final' state, 'History' or "
            "'OnFinal' on a state that is not composite, and an import that carries no alias."
          , "The state carries a property its kind does not have. History and OnFinal belong to a composite "
            "state, a Submachine belongs to neither a Start nor a Final, and a state is either painted or "
            "imported, never both." }
        , { RULE_BROKEN_IMPORT       , BandError | BandWarning | BandInformation, DocInterface | DocStateMachine
          , "An include that does not resolve: no file named, the file missing, unreadable, a cycle, or "
            "nested too deep. An included data type document that could not be read is the same fault -- "
            "every type it was to contribute is missing. The editor always refuses; the code generator "
            "softens it to a warning, or to information when the generated output of the missing document is "
            "already on disk and is used as it stands."
          , "Correct the path in the include. It is spelled from the workspace root, the file must exist and "
            "be readable, and a document may not include itself through a chain. A data type document may not "
            "include another one." }
        , { RULE_CONDITION_BODY      , BandError, DocStateMachine
          , "An 'Embedded' condition with an empty body, a body on a handler, or a body or a 'Return' on a "
            "method that is not a condition."
          , "Only an Embedded condition carries a body, and its body may not be empty. A handler and a plain "
            "method carry none, and neither carries a Return." }
        , { RULE_PARAMETERIZED_COND  , BandError, DocStateMachine
          , "A parameterized condition named as a value source, which only a left operand may be."
          , "A parameterized condition is a left operand only. Give it a right operand to compare with, or "
            "use a plain value source where a value is wanted." }
        , { RULE_IMPORT_MAJOR        , BandError, DocStateMachine
          , "An import whose pinned version differs from the file in its MAJOR part."
          , "The imported file is a different major version from the one pinned. Update the pinned version "
            "after checking the import still means the same thing, or point the include at the version it was "
            "written against." }
        , { RULE_SOURCE_KIND         , BandError, DocStateMachine
          , "A value source the format does not define, or one written where another kind of row belongs."
          , "The row names a kind of source the format does not define here. Use one the format allows in "
            "this position: a parameter, an attribute, a constant or a literal." }
        , { RULE_SOURCE_EMPTY        , BandError, DocStateMachine
          , "A value source with nothing in it."
          , "Fill the value source in, or delete the row. An empty source generates nothing, so the "
            "assignment it belongs to silently does not happen: nothing fails at build time and the target "
            "keeps whatever it held. It usually marks an edit left half done, so check the rest of that "
            "element before moving on." }
        , { RULE_GUARD               , BandError | BandWarning | BandInformation, DocStateMachine
          , "The rule guard findings are filed under. The guard checker owns the grammar and the symbol "
            "binding, but its findings are collected into the one document run."
          , "The finding is in the guard expression itself: fix the operand, the operator or the name the "
            "message points at. Every guard finding is filed under this rule." }
        , { RULE_PSEUDO_START        , BandError, DocStateMachine
          , "The 'Kind=\"Start\"' pseudo-state faults: operations on a Start, a stimulus on one of its initial "
            "transitions, a Start nothing leaves, a Start something enters (its own transition included), two "
            "or more initial transitions where any carries no condition, and a root Start that does not have "
            "exactly one unconditional transition. One id, because they are one rule (a Start is not a state) "
            "and because the code generator has to file the same faults under the same number."
          , "A Start carries no operations and its initial transition carries no stimulus. Exactly one "
            "transition leaves it, and it names a state of the same level." }
        , { RULE_TRANSITION_KIND     , BandError, DocStateMachine
          , "The transition 'Kind': an 'External' transition with no target (the unfinished edge that used to "
            "be indistinguishable from an internal one), an 'Internal' one that names a target, an 'Initial' "
            "one with no target or with a stimulus, an 'External'/'Internal' one with no stimulus, an "
            "'Initial' transition on a state that is not a 'Kind=\"Start\"', and a 'Start' owning anything "
            "other than 'Initial' transitions. One id, because they are one rule ('Kind' says what the "
            "transition is, and 'To' and 'Stimulus' then mean only what they say) and because the code "
            "generator has to file the same faults under the same number."
          , "Match Kind to the transition: External needs a target, Internal has none and stays in the state, "
            "and Local stays inside the composite it starts in." }
        , { RULE_HANDLER_NAME        , BandError, DocStateMachine
          , "Two hosted machines whose action handler parameters flatten to one name."
          , "Two hosted machines generate one handler name, so one implementation would have to serve both "
            "and the second definition will not compile. The name is derived from the trigger and its "
            "parameter types together: rename the trigger in one of the two machines, or change a parameter "
            "type, and the names separate." }
        , { RULE_ATTRIBUTE_STIMULUS  , BandError, DocStateMachine
          , "An attribute and a trigger that share a name, so both reach one function."
          , "Rename the trigger, or the attribute. The generator derives one function name from both, so the "
            "second declaration silently takes the first one over." }
        , { RULE_RESERVED_PREFIX     , BandError, DocStateMachine
          , "A name that would place a member under a reserved prefix it does not own."
          , "The generated member would land under a prefix the generator owns (request_, response_, "
            "broadcast_, on_, notify_on_, EVENT_). Rename the declaration." }
        , { RULE_UNKNOWN_ELEMENT     , BandError, DocDataType | DocInterface | DocStateMachine
          , "An element of the opened file the format does not place: an unknown tag, a real one written "
            "where the format does not allow it, or one the format has dropped. The block is kept as written "
            "while the document is open, so a document carrying one still opens, and it is dropped when the "
            "document is saved."
          , "Remove the element or move it where the format places it. A tag the format does not know reaches "
            "no generated code and is dropped when the document is saved." }
        , { RULE_PARAM_SHADOWS       , BandWarning, DocStateMachine
          , "Advisory: a parameter carrying the name of its own trigger or condition. The bare number belongs "
            "to RULE_UNKNOWN_ELEMENT; this one is only ever banded."
          , "Rename the parameter: it carries the name of its own trigger or condition. Advisory, and "
            "generation succeeds." }
        , { RULE_UNBOUND_RESPONSE    , BandWarning, DocInterface
          , "Advisory: a response no request leads to. Only ever banded -- the bare number is not in use."
          , "Either name this response from a request, with Response=\"<name>\", or delete it. A response "
            "nothing leads to is never sent." }
        , { RULE_DEFAULT_ORDER       , BandError, DocInterface | DocStateMachine
          , "A parameter carrying a default that another parameter follows."
          , "Move the parameters carrying defaults to the end of the list, exactly as C++ requires: once one "
            "parameter has a default, every parameter after it must have one too. Either give the following "
            "parameters a default of their own, or reorder the list so the ones with defaults come last. The "
            "generated signature follows the document order literally." }
        , { RULE_ACTION_PREFIX       , BandWarning, DocStateMachine
          , "Advisory: a condition whose name already begins with the prefix an action method carries. Only "
            "ever banded."
          , "Rename the condition: its name already starts with the prefix the generated action method "
            "carries, so the two read as one. Advisory." }
        , { RULE_UNRESOLVED_ELEMENT  , BandError, DocStateMachine
          , "A reference to a declared element that is not there: a trigger, an event, a timer, an action, an "
            "attribute, a constant, a condition, a parameter, the alias of a hosted machine, or the state a "
            "transition names as its target. The field to correct is the name that was written, which is what "
            "tells it apart from a data type that answers to nothing (RULE_UNRESOLVED_TYPE) and from a target "
            "that does exist but sits on another level (RULE_TARGET_SIBLING)."
          , "The reference names something the document does not declare. Declare it, or correct the "
            "spelling; the message names both the reference and its kind." }
        , { RULE_NOT_A_HEADER        , BandError, DocDataType
          , "A data type document whose include list names another document. It declares types and nothing "
            "else, and its include list carries C++ headers."
          , "A data type document includes C++ headers only. Move the shared types into this document, or "
            "have the host document include both." }
        , { RULE_DUPLICATE_ENUM_VALUE, BandError, DocDataType | DocInterface | DocStateMachine
          , "Two enumerators of one enumeration counting the same, so a value read back cannot be told apart "
            "from the other one."
          , "Give the two enumerators different values. An enumerator with no value continues from the "
            "previous one, so an explicit value that repeats an implicit one is the usual cause." }
        , { RULE_DEPRECATED          , BandWarning | BandInformation, DocDataType | DocInterface | DocStateMachine
          , "A declaration its author marked deprecated. Never an error -- a deprecation may not block a "
            "build -- so the bare number is reserved and only the banded ids are ever reported."
          , "The declaration is marked deprecated by its author. Move to what its DeprecateHint names; "
            "nothing is blocked." }
        , { RULE_EMPTY_TYPE          , BandWarning, DocDataType | DocInterface
          , "Advisory: a structure or an enumeration with no members. It generates an empty declaration, "
            "which compiles, so the note stands only until the first member is added."
          , "Add the fields or the enumerators. An empty declaration compiles, so the note stands only until "
            "the first member is added." }
        , { RULE_EMPTY_DOCUMENT      , BandWarning, DocDataType | DocInterface
          , "Advisory: the document declares nothing at all, so what includes it or connects to it gains "
            "nothing."
          , "The document declares nothing, so the generator writes a header that nothing can use and every "
            "document including it gains no type. Add what it is for -- a data type, a method, an attribute "
            "-- or delete the file together with the Location row that includes it." }
        , { RULE_FILE_NAME_MISMATCH  , BandInformation, DocDataType | DocInterface | DocStateMachine
          , "The document declares one name and lives in a file called another. Both are allowed -- the "
            "generated files follow the declared name -- but worth saying."
          , "Rename the file to the declared name, or the declared name to the file. The generated files "
            "follow the declared name, so nothing breaks either way." }
        , { RULE_FORMAT_VERSION      , BandError | BandWarning, DocDataType | DocInterface | DocStateMachine
          , "The document states a FormatVersion the reader does not know. An error only when the document is "
            "newer by a MAJOR, which says the document is shaped differently and would generate code "
            "describing a document nobody wrote. A warning when the document is newer by a MINOR or a PATCH: "
            "a minor only adds, and anything the reader does not recognise -- a section, an element, an "
            "attribute, a value the schema beside it does not allow -- is refused where it is met rather than "
            "dropped in silence, so the document is read as it stands and the difference is said aloud. A "
            "warning too when the document is older by a MAJOR, which is a migration and not a fault. An "
            "older minor or patch is readable by design and says nothing."
          , "The FormatVersion on the root element is newer than the tool reading it. Use a newer code "
            "generator, or save the document from an editor that writes the version this one reads. Lowering "
            "the number by hand does not help: the elements the newer format added would be dropped in "
            "silence. An older MAJOR is only a warning and needs nothing done." }
        , { RULE_RESPONSE_LINK       , BandError, DocInterface
          , "A request whose Response names nothing, or names a method the document declares as something "
            "other than a response. The field to correct is the response the request is answered by."
          , "Point Response at a method this document declares with MethodType=\"Response\". A request with no "
            "answer omits the attribute entirely." }
        , { RULE_UNREACHABLE_STATE   , BandWarning, DocStateMachine
          , "Warning 101: a state no transition targets and no Start enters, so nothing it does ever runs."
          , "Nothing enters the state. Give it an incoming transition, make it the Start of its level, or "
            "delete it." }
        , { RULE_DEAD_END_STATE      , BandWarning, DocStateMachine
          , "Warning 102: a state with no outgoing transition. The machine stays in it once it is entered."
          , "The machine never leaves this state. Add an outgoing transition, or mark it Kind=\"Final\" if it "
            "really is the end." }
        , { RULE_SHADOWED_TRANSITION , BandWarning, DocStateMachine
          , "Warning 103: a transition an earlier unconditional transition on the same stimulus always takes "
            "first."
          , "An earlier unconditional transition on the same stimulus always wins. Give the earlier one a "
            "guard, or reorder the two." }
        , { RULE_ONE_SIDED_EVENT     , BandWarning, DocStateMachine
          , "Warning 105: an event that is sent and never reacted to, or reacted to and never sent."
          , "Either react to the event or stop sending it. An event with only one side is usually half of an "
            "edit." }
        , { RULE_ONE_SIDED_TIMER     , BandWarning, DocStateMachine
          , "Warning 106: a timer that is started and never reacted to, or reacted to and never started."
          , "Either react to the timer or stop starting it. A timer with only one side never has an effect." }
        , { RULE_EMPTY_INTERNAL      , BandWarning, DocStateMachine
          , "Warning 107: an internal transition that carries no operation and no condition, so reacting to "
            "the stimulus changes nothing."
          , "The internal transition changes nothing. Give it an operation or a condition, or delete it." }
        , { RULE_CONSTANT_COMPARE    , BandWarning, DocStateMachine
          , "Warning 109: a comparison whose operands are both fixed at design time, so the result is decided "
            "before the machine runs."
          , "Both operands are fixed at design time, so the result never changes. Compare against an "
            "attribute or a parameter, or drop the guard." }
        , { RULE_UNUSED_HISTORY      , BandWarning, DocStateMachine
          , "Warning 110: a state carrying History that no transition ever re-enters, so the remembered "
            "substate is never restored."
          , "Nothing re-enters the state, so its remembered substate is never restored. Remove History, or "
            "add the transition that returns to the state." }
        , { RULE_IMPORT_PATCH        , BandWarning | BandInformation, DocStateMachine
          , "Warning 112 and information 212: an import pinned to a version the file no longer carries, where "
            "the difference is below the major version."
          , "The imported file carries a different minor or patch version from the one pinned. Update the pin "
            "once the difference is understood." }
        , { RULE_FINAL_ENTRY_ORDER   , BandWarning, DocStateMachine
          , "Warning 108: an operation on the 'EntryList' of a 'Final' state that a composite encloses, where "
            "the composite raises 'OnFinal'. The 'EntryList' runs as the 'Final' is entered, while the "
            "machine is still inside the composite, and the queued 'OnFinal' event leaves the composite only "
            "later, so the operation runs against the state the machine is leaving rather than the one it is "
            "going to. The bare number belongs to RULE_FINAL_STATE; this one is only ever banded."
          , "Move the operation onto the transition the composite takes on its OnFinal event. That step runs "
            "once the level has been left and the machine has settled in the target state, which is what the "
            "placement on the Final reads as. Keep it on the Final only when it is meant to run before the "
            "composite is left." }
        , { RULE_RETIRED_ELEMENT     , BandError, DocDataType | DocInterface | DocStateMachine
          , "An element the format used to define in this place and no longer does. Told apart from a tag the "
            "format never had, because the author is moving something rather than correcting a spelling, and "
            "the finding can name what to do instead."
          , "The format no longer defines this element here. The message names what replaced it; move the "
            "content there." }
        , { RULE_SHARED_RESPONSE     , BandInformation, DocInterface
          , "A response that answers more than one request. Legal and sometimes meant, so it is information "
            "and never a warning."
          , "One response answers several requests. Legal and sometimes meant; split it only if a consumer "
            "must tell the two answers apart." }
        , { RULE_UNKNOWN_ATTRIBUTE   , BandWarning, DocDataType | DocInterface | DocStateMachine
          , "An attribute the format does not define on an element it does define. Nothing reads it, so it "
            "reaches no generated code, and it is dropped when the document is saved."
          , "Remove the attribute. The format does not define it on this element, so nothing reads it and it "
            "is dropped when the document is saved." }
        , { RULE_DROPPED_ELEMENT     , BandWarning, DocDataType | DocInterface | DocStateMachine
          , "An element block the format does not place, kept while the document is open and dropped when it "
            "is saved. Reported beside the fault itself so the loss is stated before it happens."
          , "The block was kept while the document was open and is lost on save. Fix the fault reported "
            "beside it, then re-add the block." }
        , { RULE_HISTORY_SHAPE       , BandError, DocStateMachine
          , "A Kind=\"History\" state carrying an EntryList, an ExitList, a TransitionList or a nested "
            "StateList. It is a pseudo-state, not a state: the machine never occupies it, so it owns none of "
            "the things a state owns."
          , "A history marker is a marker, not a state. Move the entry, exit, transitions or substates onto a "
            "state of the level, and give the marker a HistoryDepth." }
        , { RULE_HISTORY_DUPLICATE   , BandError, DocStateMachine
          , "More than one Kind=\"History\" state on one level. At most one per level -- a second one would "
            "leave two markers naming the same re-entry."
          , "One marker names one re-entry. Keep one and point every resuming transition at it; delete the "
            "others." }
        , { RULE_HISTORY_ROOT        , BandError, DocStateMachine
          , "A Kind=\"History\" state at the root level. There is no composite there to resume, so the marker "
            "names nothing."
          , "Move the marker inside the StateList of the composite whose history it names. The root level has "
            "no composite to resume." }
        , { RULE_HISTORY_SIBLING     , BandError, DocStateMachine
          , "A transition targeting a Kind=\"History\" state from inside its own level. The marker is reached "
            "only by a transition entering the composite from outside (RULE_TARGET_SIBLING, widened); from "
            "inside, the state it names is an ordinary non-sibling target."
          , "A marker is reached on the way IN, from a state outside the composite. From inside the level, "
            "name the state to enter instead." }
        , { RULE_HISTORY_CONFLICT    , BandError, DocStateMachine
          , "A composite carrying both its own History attribute and a Kind=\"History\" child. Two spellings of "
            "one decision, disagreeing about what the composite does on re-entry."
          , "The composite says the same thing twice. Keep the marker and remove the History attribute, or "
            "remove the marker and keep the attribute." }
        , { RULE_CONTAINER_KEY       , BandError, DocDataType | DocInterface | DocStateMachine
          , "A HashMap or a Map whose key type cannot be a key. A HashMap finds a key by its hash and a Map "
            "keeps its keys ordered, so the key has to hash or to order. A primitive, an enumeration, "
            "'String' and 'WideString' do both; 'DateTime' orders and does not hash; 'BinaryBuffer' and a "
            "container do neither; a structure does what every one of its fields does, and an empty structure "
            "does both. A type declared with Type=\"Imported\" is the author's own C++ type and is not judged."
          , "Change the key type, or change the structure field that stops it: a 'BinaryBuffer', a container, "
            "a field of a type declared Type=\"Imported\", or a 'DateTime' in the key of a HashMap. A key that "
            "has to carry a 'DateTime' can be the key of a Map instead." }
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

    /**
     * \brief   The wording of the rule carrying \p number in \p band.
     * \param   number  The rule number, without any band applied.
     * \param   band    The band the finding is reported in.
     * \return  The wording, or an empty string when no rule carries that pair. Never nullptr.
     **/
    inline const char * summaryOf(int number, eBand band)
    {
        for (const Rule & rule : REGISTRY)
        {
            if ((rule.number == number) && ((rule.bands & static_cast<unsigned int>(band)) != 0u))
            {
                return rule.summary;
            }
        }

        return "";
    }

    /**
     * \brief   The corrective action of the rule carrying \p number in \p band.
     * \param   number  The rule number, without any band applied.
     * \param   band    The band the finding is reported in.
     * \return  What to change, or an empty string when no rule carries that pair. Never nullptr.
     **/
    inline const char * fixOf(int number, eBand band)
    {
        for (const Rule & rule : REGISTRY)
        {
            if ((rule.number == number) && ((rule.bands & static_cast<unsigned int>(band)) != 0u))
            {
                return rule.fix;
            }
        }

        return "";
    }

    /**
     * \brief   True when the rule carrying \p number in \p band applies to \p document.
     * \param   number      The rule number, without any band applied.
     * \param   band        The band the finding is reported in.
     * \param   document    The document kind to test.
     * \return  True when the rule is declared for that document kind.
     **/
    inline bool appliesTo(int number, eBand band, eDocument document)
    {
        for (const Rule & rule : REGISTRY)
        {
            if ((rule.number == number) && ((rule.bands & static_cast<unsigned int>(band)) != 0u))
            {
                return (rule.documents & static_cast<unsigned int>(document)) != 0u;
            }
        }

        return false;
    }

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