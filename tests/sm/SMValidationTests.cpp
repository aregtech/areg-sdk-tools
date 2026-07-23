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
 *  \file        tests/sm/SMValidationTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Unit tests for the FSM structural and reference validation engine. Each
 *               check has a positive (violating) and a negative (clean) case. The engine is
 *               headless -- no widgets -- so the code generator can reuse it. Self-contained
 *               (no external test framework).
 *
 ************************************************************************/

#include "lusan/model/sm/SMValidator.hpp"
#include "lusan/model/sm/SMTypeCompat.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMImportData.hpp"
#include "lusan/data/sm/SMDataTypeData.hpp"

#include "lusan/data/common/DataTypeEnum.hpp"

#include <cstdio>

//////////////////////////////////////////////////////////////////////////
// Minimal assertion harness
//////////////////////////////////////////////////////////////////////////

namespace
{
    int gChecks = 0;
    int gFailures = 0;

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }
}

#define CHECK(cond)  check((cond), #cond)

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////

namespace
{
    using eKind   = SMStateEntry::eStateKind;
    using eStim   = SMTransitionEntry::eStimulusKind;
    using eSource = SMArgumentEntry::eValueSource;
    using eMethod = SMMethodEntry::eMethodType;
    using eOp     = SMConditionEntry::eOperator;

    int countRule(const QList<SMIssue>& issues, int rule)
    {
        int n = 0;
        for (const SMIssue& i : issues)
            if (i.rule == rule) ++n;
        return n;
    }

    bool hasRule(const QList<SMIssue>& issues, int rule)
    {
        return countRule(issues, rule) > 0;
    }

    //!< A 10.2 warning is stored with the offset rule id; these target it by its 10.2 number.
    int countWarn(const QList<SMIssue>& issues, int warnNumber)
    {
        return countRule(issues, SMValidator::WARNING_RULE_BASE + warnNumber);
    }

    bool hasWarn(const QList<SMIssue>& issues, int warnNumber)
    {
        return countWarn(issues, warnNumber) > 0;
    }

    //!< A minimal single-level machine with one Start state, valid on its own.
    SMStateEntry* addStart(StateMachineData& doc, const QString& name = "Idle")
    {
        return doc.getStates().createState(name, eKind::Start);
    }
}

//////////////////////////////////////////////////////////////////////////
// Start-state presence and uniqueness per machine level
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testStartState()
    {
        std::printf("- start-state constraints\n");
        {   // Positive: no Start on the root level.
            StateMachineData doc;
            doc.getStates().createState("A", eKind::Normal);
            CHECK(hasRule(SMValidator::validate(doc), 1));
        }
        {   // Positive: two Start states on one level.
            StateMachineData doc;
            addStart(doc, "S1");
            doc.getStates().createState("S2", eKind::Start);
            CHECK(hasRule(SMValidator::validate(doc), 1));
        }
        {   // Positive: a painted nested level with no Start.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Normal);
            CHECK(hasRule(SMValidator::validate(doc), 11));
        }
        {   // Negative: exactly one Start on every level.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 1) == 0);
            CHECK(countRule(issues, 11) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Duplicate element identifiers and duplicate state names
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testDuplicates()
    {
        std::printf("- duplicate id / state name\n");
        {   // Positive: forced duplicate ID.
            StateMachineData doc;
            SMStateEntry* a = addStart(doc);
            SMStateEntry* b = doc.getStates().createState("B", eKind::Normal);
            b->setId(a->getId());
            CHECK(hasRule(SMValidator::validate(doc), 2));
        }
        {   // Negative: monotonic IDs are unique.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("B", eKind::Normal);
            CHECK(countRule(SMValidator::validate(doc), 2) == 0);
        }
        {   // Positive: same state name on two levels.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            SMStateData* nested = comp->getOrCreateNestedStates();
            nested->createState("Idle", eKind::Start);          // clashes with the root Start.
            CHECK(hasRule(SMValidator::validate(doc), 3));
        }
        {   // Negative: distinct state names.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("Work", eKind::Normal);
            CHECK(countRule(SMValidator::validate(doc), 3) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Name collisions within a registry, the stimulus space, and a parameter list
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testNameCollisions()
    {
        std::printf("- name collisions\n");
        {   // Positive: a trigger and an event share a name (stimulus space).
            StateMachineData doc;
            addStart(doc);
            doc.getMethods().createMethod("go", eMethod::Trigger);
            doc.getEvents().createEvent("go");
            CHECK(hasRule(SMValidator::validate(doc), 4));
        }
        {   // Positive: duplicate parameter name within one ParamList.
            StateMachineData doc;
            addStart(doc);
            SMMethodEntry* m = doc.getMethods().createMethod("act", eMethod::Action);
            m->addParam("p");
            m->addParam("p");
            CHECK(hasRule(SMValidator::validate(doc), 4));
        }
        {   // Negative: disjoint names.
            StateMachineData doc;
            addStart(doc);
            doc.getMethods().createMethod("go", eMethod::Trigger);
            doc.getEvents().createEvent("done");
            doc.getTimers().createTimer("tick");
            CHECK(countRule(SMValidator::validate(doc), 4) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Identifier syntax of element names
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testIdentifiers()
    {
        std::printf("- identifier syntax\n");
        {   // Positive: a state name with a space.
            StateMachineData doc;
            addStart(doc, "Bad Name");
            CHECK(hasRule(SMValidator::validate(doc), 5));
        }
        {   // Negative: a valid identifier.
            StateMachineData doc;
            addStart(doc, "Good_Name1");
            CHECK(countRule(SMValidator::validate(doc), 5) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Cross-reference resolution and the sibling-target constraint
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testReferences()
    {
        std::printf("- references and sibling target\n");
        {   // Positive: an unresolved trigger stimulus.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            s->getTransitions().createTransition(eStim::Trigger, "ghost", "Idle");
            CHECK(hasRule(SMValidator::validate(doc), 6));
        }
        {   // Positive: an unresolved transition target.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("go", eMethod::Trigger);
            s->getTransitions().createTransition(eStim::Trigger, "go", "Nowhere");
            CHECK(hasRule(SMValidator::validate(doc), 6));
        }
        {   // Positive: a target that exists but is not a sibling (it is nested).
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            doc.getMethods().createMethod("go", eMethod::Trigger);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            start->getTransitions().createTransition(eStim::Trigger, "go", "Inner");
            CHECK(hasRule(SMValidator::validate(doc), 7));
        }
        {   // Positive: an operation action that does not resolve.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            s->getEntryList().addOperation(new SMActionCall(0, "ghostAction"));
            CHECK(hasRule(SMValidator::validate(doc), 6));
        }
        {   // Negative: every reference resolves to a sibling / declared name.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            doc.getStates().createState("Work", eKind::Normal);
            doc.getMethods().createMethod("go", eMethod::Trigger);
            start->getTransitions().createTransition(eStim::Trigger, "go", "Work");
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 6) == 0);
            CHECK(countRule(issues, 7) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Final and Start state structure
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testFinalStart()
    {
        std::printf("- final/start structure\n");
        {   // Positive: a Final state with an outgoing transition.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* fin = doc.getStates().createState("Done", eKind::Final);
            doc.getMethods().createMethod("go", eMethod::Trigger);
            fin->getTransitions().createTransition(eStim::Trigger, "go", "Idle");
            CHECK(hasRule(SMValidator::validate(doc), 8));
        }
        {   // Positive: a Start state that owns substates.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            start->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            CHECK(hasRule(SMValidator::validate(doc), 9));
        }
        {   // Negative: a Final leaf and a childless Start.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("Done", eKind::Final);
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 8) == 0);
            CHECK(countRule(issues, 9) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Argument-to-parameter matching
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testArguments()
    {
        std::printf("- argument matching\n");
        {   // Positive: a required parameter left unmapped.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMMethodEntry* act = doc.getMethods().createMethod("act", eMethod::Action);
            act->addParam("p");
            s->getEntryList().addOperation(new SMActionCall(0, "act"));
            CHECK(hasRule(SMValidator::validate(doc), 10));
        }
        {   // Positive: an argument that names no declared parameter.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", eMethod::Action);
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("stranger", eSource::Value, "1");
            CHECK(hasRule(SMValidator::validate(doc), 10));
        }
        {   // Negative: every declared parameter is mapped.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMMethodEntry* act = doc.getMethods().createMethod("act", eMethod::Action);
            act->addParam("p");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Value, "1");
            CHECK(countRule(SMValidator::validate(doc), 10) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Param source scope
//////////////////////////////////////////////////////////////////////////

namespace
{
    // Builds an ActionCall on the given list mapping act(p) := Param(paramValue).
    SMActionCall* mappedCall(StateMachineData& doc, SMOperationList& list, const QString& paramValue)
    {
        if (doc.getMethods().findMethod("act") == nullptr)
            doc.getMethods().createMethod("act", eMethod::Action)->addParam("p");
        SMActionCall* call = new SMActionCall(0, "act");
        list.addOperation(call);
        call->addArgument("p", eSource::Param, paramValue);
        return call;
    }

    void testParamScope()
    {
        std::printf("- param scope\n");
        {   // Positive: Param used in an entry list (no stimulus scope).
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            mappedCall(doc, s->getEntryList(), "amount");
            CHECK(hasRule(SMValidator::validate(doc), 12));
        }
        {   // Positive: Param on a timer transition.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getTimers().createTimer("tick");
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Timer, "tick");
            mappedCall(doc, tr->getOperations(), "amount");
            CHECK(hasRule(SMValidator::validate(doc), 12));
        }
        {   // Positive: the trigger stimulus declares no such parameter.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("onTick", eMethod::Trigger);  // no parameters.
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "onTick");
            mappedCall(doc, tr->getOperations(), "amount");
            CHECK(hasRule(SMValidator::validate(doc), 12));
        }
        {   // Negative: Param names an actual stimulus parameter.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("onTick", eMethod::Trigger)->addParam("amount");
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "onTick");
            mappedCall(doc, tr->getOperations(), "amount");
            CHECK(countRule(SMValidator::validate(doc), 12) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Composite-state exclusivity and placement
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testComposite()
    {
        std::printf("- composite constraints\n");
        {   // Positive: History on a non-composite state.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* s = doc.getStates().createState("Work", eKind::Normal);
            s->setHistory(SMStateEntry::eHistory::Shallow);
            CHECK(hasRule(SMValidator::validate(doc), 18));
        }
        {   // Positive: a Submachine on a Final state.
            StateMachineData doc;
            addStart(doc);
            doc.getImports().createImport("Lib");
            SMStateEntry* fin = doc.getStates().createState("Done", eKind::Final);
            fin->setSubmachine("Lib");
            CHECK(hasRule(SMValidator::validate(doc), 18));
        }
        {   // Negative: History on a painted composite.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            comp->setHistory(SMStateEntry::eHistory::Deep);
            CHECK(countRule(SMValidator::validate(doc), 18) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Condition-method body and implementation mode
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testMethodBody()
    {
        std::printf("- condition body/implement\n");
        {   // Positive: an Embedded condition with an empty body.
            StateMachineData doc;
            addStart(doc);
            SMMethodEntry* c = doc.getMethods().createMethod("ready", eMethod::Condition);
            c->setImplement(SMMethodEntry::eImplement::Embedded);
            c->setBody("   ");
            CHECK(hasRule(SMValidator::validate(doc), 20));
        }
        {   // Positive: a body on a non-embedded method.
            StateMachineData doc;
            addStart(doc);
            doc.getMethods().createMethod("go", eMethod::Trigger)->setBody("do();");
            CHECK(hasRule(SMValidator::validate(doc), 20));
        }
        {   // Negative: an Embedded condition with a real body.
            StateMachineData doc;
            addStart(doc);
            SMMethodEntry* c = doc.getMethods().createMethod("ready", eMethod::Condition);
            c->setImplement(SMMethodEntry::eImplement::Embedded);
            c->setBody("return true;");
            CHECK(countRule(SMValidator::validate(doc), 20) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// A parameterized condition call is a left operand only
//////////////////////////////////////////////////////////////////////////

namespace
{
    SMTransitionEntry* triggeredTransition(StateMachineData& doc, SMStateEntry* owner)
    {
        if (doc.getMethods().findMethod("go") == nullptr)
            doc.getMethods().createMethod("go", eMethod::Trigger);
        return owner->getTransitions().createTransition(eStim::Trigger, "go");
    }

    void testConditionLhsOnly()
    {
        std::printf("- parameterized condition LHS-only\n");
        {   // Positive: a parameterized condition on the RHS.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("isReady", eMethod::Condition)->addParam("x");
            SMTransitionEntry* tr = triggeredTransition(doc, s);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Value);
            row->setLhs("1");
            row->setOperator(eOp::Equal);
            row->setRhsKind(eSource::Condition);
            row->setRhs("isReady");
            CHECK(hasRule(SMValidator::validate(doc), 21));
        }
        {   // Negative: the same parameterized condition on the LHS.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("isReady", eMethod::Condition)->addParam("x");
            SMTransitionEntry* tr = triggeredTransition(doc, s);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Condition);
            row->setLhs("isReady");
            row->addArgument("x", eSource::Value, "1");     // map the parameter so the argument check stays quiet.
            CHECK(countRule(SMValidator::validate(doc), 21) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Expression-row shape and empty verbatim text
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testExpressionRows()
    {
        std::printf("- expression rows\n");
        {   // Positive: an expression row with no expression text.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMTransitionEntry* tr = triggeredTransition(doc, s);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Expression);
            row->setExpression("   ");
            CHECK(hasRule(SMValidator::validate(doc), 24));
        }
        {   // Positive: an expression row that also carries an operator and RHS.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMTransitionEntry* tr = triggeredTransition(doc, s);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Expression);
            row->setExpression("a > b");
            row->setOperator(eOp::Equal);
            row->setRhsKind(eSource::Value);
            row->setRhs("1");
            CHECK(hasRule(SMValidator::validate(doc), 23));
        }
        {   // Negative: a well-formed expression row.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMTransitionEntry* tr = triggeredTransition(doc, s);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Expression);
            row->setExpression("count > 0");
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 23) == 0);
            CHECK(countRule(issues, 24) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// A finding carries the offending element, a severity, and a message
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testFindingShape()
    {
        std::printf("- finding carries element id / severity / message\n");
        StateMachineData doc;
        SMStateEntry* s = addStart(doc);
        SMStateEntry* work = doc.getStates().createState("Work", eKind::Normal);
        work->setHistory(SMStateEntry::eHistory::Shallow);       // a history flag on a non-composite state.

        const QList<SMIssue> issues = SMValidator::validate(doc);
        bool navigable = false;
        for (const SMIssue& i : issues)
        {
            if ((i.rule == 18) && (i.elementId == work->getId()))
            {
                navigable = true;
                CHECK(i.severity == SMIssue::eSeverity::Error);
                CHECK(i.message.isEmpty() == false);
            }
        }
        CHECK(navigable);
        (void)s;
    }
}

//////////////////////////////////////////////////////////////////////////
// The 6.9 widening table, exhaustively over every ordered primitive pair
//////////////////////////////////////////////////////////////////////////

namespace
{
    // The oracle: does an implicit widening path exist from a to b (6.9 rules 1-4)?
    bool widensOracle(const QString& a, const QString& b)
    {
        auto uRank = [](const QString& t) -> int
        {
            if (t == "uint8") return 1; if (t == "uint16") return 2;
            if (t == "uint32") return 3; if (t == "uint64") return 4; return 0;
        };
        auto sRank = [](const QString& t) -> int
        {
            if (t == "int8") return 1; if (t == "int16") return 2;
            if (t == "int32") return 3; if (t == "int64") return 4; return 0;
        };
        auto fRank = [](const QString& t) -> int
        {
            if (t == "float") return 1; if (t == "double") return 2; return 0;
        };
        auto intUpTo32 = [](const QString& t) -> bool
        {
            return (t == "int8") || (t == "int16") || (t == "int32")
                || (t == "uint8") || (t == "uint16") || (t == "uint32");
        };

        if (a == b)
            return true;
        if ((uRank(a) != 0) && (uRank(b) != 0)) return uRank(a) <= uRank(b);
        if ((sRank(a) != 0) && (sRank(b) != 0)) return sRank(a) <= sRank(b);
        if ((fRank(a) != 0) && (fRank(b) != 0)) return fRank(a) <= fRank(b);
        if (intUpTo32(a) && (b == "double")) return true;
        return false;
    }

    void testWideningTable()
    {
        std::printf("- widening table (all primitive pairs)\n");
        const QStringList types = { "bool", "char", "int8", "int16", "int32", "int64",
                                    "uint8", "uint16", "uint32", "uint64", "float", "double", "String" };
        for (const QString& from : types)
        {
            for (const QString& to : types)
            {
                SMTypeCompat::eRank expected;
                if (from == to)
                    expected = SMTypeCompat::eRank::Exact;
                else if (widensOracle(from, to))
                    expected = SMTypeCompat::eRank::Converts;
                else if (widensOracle(to, from))
                    expected = SMTypeCompat::eRank::Narrows;
                else
                    expected = SMTypeCompat::eRank::Mismatch;

                const SMTypeCompat::eRank actual = SMTypeCompat::rank(from, to);
                check(actual == expected, qPrintable(QString("rank(%1,%2)").arg(from, to)));
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Type compatibility, ordering/structure, literals, boolean tests, AttributeSet
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testTypeRules()
    {
        std::printf("- type / literal rules (13-17)\n");

        {   // Rule 13: mapping a String attribute onto a uint16 parameter (no widening).
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", eMethod::Action)->addParam("p")->setType("uint16");
            doc.getAttributes().createAttribute("label")->setType("String");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Attribute, "label");
            CHECK(hasRule(SMValidator::validate(doc), 13));
        }
        {   // Negative rule 13: a uint8 attribute widens to a uint16 parameter.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", eMethod::Action)->addParam("p")->setType("uint16");
            doc.getAttributes().createAttribute("small")->setType("uint8");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Attribute, "small");
            CHECK(countRule(SMValidator::validate(doc), 13) == 0);
        }
        {   // Rule 14: an ordering operator on a String operand.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("label")->setType("String");
            doc.getMethods().createMethod("go", eMethod::Trigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go");
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("label");
            row->setOperator(eOp::Greater);
            row->setRhsKind(eSource::Value); row->setRhs("x");
            CHECK(hasRule(SMValidator::validate(doc), 14));
        }
        {   // Rule 14: a structure operand in a condition row.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getDataTypes().addStructure("Rec");
            doc.getAttributes().createAttribute("r")->setType("Rec");
            doc.getMethods().createMethod("go", eMethod::Trigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go");
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("r");
            row->setOperator(eOp::Equal);
            row->setRhsKind(eSource::Attribute); row->setRhs("r");
            CHECK(hasRule(SMValidator::validate(doc), 14));
        }
        {   // Negative rule 14: numeric ordering is fine.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("n")->setType("uint16");
            doc.getMethods().createMethod("go", eMethod::Trigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go");
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("n");
            row->setOperator(eOp::Greater);
            row->setRhsKind(eSource::Value); row->setRhs("5");
            CHECK(countRule(SMValidator::validate(doc), 14) == 0);
        }
        {   // Rule 15: an out-of-range integer literal for a uint8 parameter.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", eMethod::Action)->addParam("p")->setType("uint8");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Value, "999");
            CHECK(hasRule(SMValidator::validate(doc), 15));
        }
        {   // Rule 15: a literal that is not an enumerator of the declared enum.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            DataTypeEnum* color = static_cast<DataTypeEnum*>(doc.getDataTypes().addEnum("Color"));
            color->addField("Red"); color->addField("Green");
            doc.getMethods().createMethod("act", eMethod::Action)->addParam("p")->setType("Color");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Value, "Blue");
            CHECK(hasRule(SMValidator::validate(doc), 15));
        }
        {   // Negative rule 15: an in-range literal and a valid enumerator.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            DataTypeEnum* color = static_cast<DataTypeEnum*>(doc.getDataTypes().addEnum("Color"));
            color->addField("Red");
            SMMethodEntry* act = doc.getMethods().createMethod("act", eMethod::Action);
            act->addParam("p")->setType("uint8");
            act->addParam("c")->setType("Color");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Value, "200");
            call->addArgument("c", eSource::Value, "Red");
            CHECK(countRule(SMValidator::validate(doc), 15) == 0);
        }
        {   // Rule 16: a boolean-test row whose operand is not bool.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("cnt")->setType("uint16");
            doc.getMethods().createMethod("go", eMethod::Trigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go");
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("cnt");    // no operator: boolean test.
            CHECK(hasRule(SMValidator::validate(doc), 16));
        }
        {   // Negative rule 16: a bool operand is a valid boolean test.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("flag")->setType("bool");
            doc.getMethods().createMethod("go", eMethod::Trigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go");
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("flag");
            CHECK(countRule(SMValidator::validate(doc), 16) == 0);
        }
        {   // Rule 17: an AttributeSet whose source type is incompatible with the attribute.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("count")->setType("uint16");
            doc.getAttributes().createAttribute("label")->setType("String");
            SMAttributeSet* set = new SMAttributeSet(0, "count");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Attribute); set->setValue("label");
            CHECK(hasRule(SMValidator::validate(doc), 17));
        }
        {   // Negative rule 17: a widening source is fine.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("count")->setType("uint16");
            doc.getAttributes().createAttribute("small")->setType("uint8");
            SMAttributeSet* set = new SMAttributeSet(0, "count");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Attribute); set->setValue("small");
            CHECK(countRule(SMValidator::validate(doc), 17) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Warning rules (10.2 rules 1-11)
//////////////////////////////////////////////////////////////////////////

namespace
{
    // A reachable Normal state that has an outgoing transition, so it draws no W1/W2 itself.
    SMStateEntry* addReachedState(StateMachineData& doc, SMStateEntry* start, const QString& name, const QString& trigger)
    {
        SMStateEntry* st = doc.getStates().createState(name, eKind::Normal);
        if (doc.getMethods().findMethod(trigger) == nullptr)
            doc.getMethods().createMethod(trigger, eMethod::Trigger);
        start->getTransitions().createTransition(eStim::Trigger, trigger, name);
        return st;
    }

    void testWarnings()
    {
        std::printf("- warnings (10.2 rules 1-11)\n");

        {   // W1: an unreachable Normal state; negative once a transition targets it.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getStates().createState("Lost", eKind::Normal);
            CHECK(hasWarn(SMValidator::validate(doc), 1));

            doc.getMethods().createMethod("go", eMethod::Trigger);
            s->getTransitions().createTransition(eStim::Trigger, "go", "Lost");
            CHECK(countWarn(SMValidator::validate(doc), 1) == 0);
        }
        {   // W2: a reachable Normal state with no outgoing transition; negative once it has one.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMStateEntry* work = addReachedState(doc, s, "Work", "go");
            CHECK(hasWarn(SMValidator::validate(doc), 2));

            doc.getMethods().createMethod("back", eMethod::Trigger);
            work->getTransitions().createTransition(eStim::Trigger, "back", "Idle");
            CHECK(countWarn(SMValidator::validate(doc), 2) == 0);
        }
        {   // W3: a transition shadowed by an earlier unconditional one on the same stimulus.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("go", eMethod::Trigger);
            s->getTransitions().createTransition(eStim::Trigger, "go", "Idle");
            s->getTransitions().createTransition(eStim::Trigger, "go", "Idle");
            CHECK(hasWarn(SMValidator::validate(doc), 3));
        }
        {   // Negative W3: the first transition carries a condition, so it does not shadow.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("go", eMethod::Trigger);
            SMTransitionEntry* first = s->getTransitions().createTransition(eStim::Trigger, "go", "Idle");
            SMConditionEntry* row = first->getConditions().addCondition();
            row->setLhsKind(eSource::Expression); row->setExpression("count > 0");
            s->getTransitions().createTransition(eStim::Trigger, "go", "Idle");
            CHECK(countWarn(SMValidator::validate(doc), 3) == 0);
        }
        {   // W4: a declared constant that is never referenced.
            StateMachineData doc;
            addStart(doc);
            doc.getConstants().createConstant("Unused")->setType("int32");
            CHECK(hasWarn(SMValidator::validate(doc), 4));
        }
        {   // Negative W4: the constant is referenced by an AttributeSet.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getConstants().createConstant("Used")->setType("int32");
            doc.getAttributes().createAttribute("a")->setType("int32");
            SMAttributeSet* set = new SMAttributeSet(0, "a");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Constant); set->setValue("Used");
            CHECK(countWarn(SMValidator::validate(doc), 4) == 0);
        }
        {   // W5: an event sent but never reacted to; negative once a transition reacts.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getEvents().createEvent("ping");
            s->getEntryList().addOperation(new SMEventSend(0, "ping"));
            CHECK(hasWarn(SMValidator::validate(doc), 5));

            s->getTransitions().createTransition(eStim::Event, "ping");
            CHECK(countWarn(SMValidator::validate(doc), 5) == 0);
        }
        {   // W6: a timer reacted to but never started; negative once a TimerStart appears.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getTimers().createTimer("tick");
            s->getTransitions().createTransition(eStim::Timer, "tick");
            CHECK(hasWarn(SMValidator::validate(doc), 6));

            s->getEntryList().addOperation(new SMTimerStart(0, "tick"));
            CHECK(countWarn(SMValidator::validate(doc), 6) == 0);
        }
        {   // W7: an empty internal transition; negative once it carries an operation.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getTimers().createTimer("tick");
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Timer, "tick");
            CHECK(hasWarn(SMValidator::validate(doc), 7));

            doc.getMethods().createMethod("act", eMethod::Action);
            tr->getOperations().addOperation(new SMActionCall(0, "act"));
            CHECK(countWarn(SMValidator::validate(doc), 7) == 0);
        }
        {   // W8: an attribute written but never read; negative once it is also read.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("w")->setType("int32");
            SMAttributeSet* set = new SMAttributeSet(0, "w");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Value); set->setValue("1");
            CHECK(hasWarn(SMValidator::validate(doc), 8));

            doc.getMethods().createMethod("go", eMethod::Trigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go");
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("w");
            row->setOperator(eOp::Greater);
            row->setRhsKind(eSource::Value); row->setRhs("0");
            CHECK(countWarn(SMValidator::validate(doc), 8) == 0);
        }
        {   // W9: a comparison of two design-time constants; negative once one side is live.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("go", eMethod::Trigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go");
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Value); row->setLhs("1");
            row->setOperator(eOp::Equal);
            row->setRhsKind(eSource::Value); row->setRhs("2");
            CHECK(hasWarn(SMValidator::validate(doc), 9));

            doc.getAttributes().createAttribute("x")->setType("int32");
            row->setLhsKind(eSource::Attribute); row->setLhs("x");
            CHECK(countWarn(SMValidator::validate(doc), 9) == 0);
        }
        {   // W10: history on a composite nothing re-enters; negative once a transition targets it.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            comp->setHistory(SMStateEntry::eHistory::Shallow);
            CHECK(hasWarn(SMValidator::validate(doc), 10));

            doc.getMethods().createMethod("go", eMethod::Trigger);
            start->getTransitions().createTransition(eStim::Trigger, "go", "Comp");
            CHECK(countWarn(SMValidator::validate(doc), 10) == 0);
        }
        {   // W11: an empty inline-code block; negative once it has content.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            s->getEntryList().addOperation(new SMInlineCode(0, "   "));
            CHECK(hasWarn(SMValidator::validate(doc), 11));

            StateMachineData doc2;
            SMStateEntry* s2 = addStart(doc2);
            s2->getEntryList().addOperation(new SMInlineCode(0, "doThing();"));
            CHECK(countWarn(SMValidator::validate(doc2), 11) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// A document with errors still validates (never throws / blocks)
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testErrorsDoNotBlock()
    {
        std::printf("- errors are reported, never fatal\n");
        StateMachineData doc;
        doc.getStates().createState("A", eKind::Normal);        // no Start: an error document.
        const QList<SMIssue> issues = SMValidator::validate(doc);
        CHECK(hasRule(issues, 1));
        // The engine returns findings and leaves the document intact and editable.
        CHECK(doc.getStates().getElements().isEmpty() == false);
    }
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char* /*argv*/[])
{
    std::printf("=== FSM validation engine tests ===\n");

    testStartState();
    testDuplicates();
    testNameCollisions();
    testIdentifiers();
    testReferences();
    testFinalStart();
    testArguments();
    testParamScope();
    testComposite();
    testMethodBody();
    testConditionLhsOnly();
    testExpressionRows();
    testFindingShape();
    testWideningTable();
    testTypeRules();
    testWarnings();
    testErrorsDoNotBlock();

    std::printf("=== %d checks, %d failure(s) ===\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
