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
 *  \file        tests/sm/SMConditionEngineTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-21-03 unit tests: condition token translator, preview/codegen renderer,
 *               and the type-compatibility seed. Self-contained (no external framework).
 *
 ************************************************************************/

#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/model/sm/SMConditionText.hpp"
#include "lusan/model/sm/SMConditionToken.hpp"
#include "lusan/model/sm/SMTypeCompat.hpp"

#include <QString>
#include <cstdio>

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

    void checkEq(const QString& actual, const QString& expected, const char* what)
    {
        ++gChecks;
        if (actual != expected)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n         expected: %s\n         actual  : %s\n",
                        what, expected.toStdString().c_str(), actual.toStdString().c_str());
        }
    }
}

#define CHECK(cond)  check((cond), #cond)

//////////////////////////////////////////////////////////////////////////
// AC3: SMConditionText preview and C++ on the design Section 0 example
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testPreviewAndCpp()
    {
        std::printf("[SM-21-03] preview and C++ render the design example exactly\n");

        SMConditionList root;

        SMConditionEntry* c0 = root.addCondition();
        c0->setLhsKind(SMArgumentEntry::eValueSource::Attribute);
        c0->setLhs("WalkRequested");

        SMConditionGroup* grp = root.addGroup();
        grp->setCombine(SMConditionGroup::eCombine::Or);

        SMConditionEntry* g0 = grp->addCondition();
        g0->setLhsKind(SMArgumentEntry::eValueSource::Condition);
        g0->setLhs("HasWaiting");
        g0->addArgument("count", SMArgumentEntry::eValueSource::Param, "count");

        SMConditionEntry* g1 = grp->addCondition();
        g1->setLhsKind(SMArgumentEntry::eValueSource::Param);
        g1->setLhs("count");
        g1->setOperator(SMConditionEntry::eOperator::GreaterEqual);
        g1->setRhsKind(SMArgumentEntry::eValueSource::Constant);
        g1->setRhs("MIN_WAITING");

        SMConditionEntry* c2 = root.addCondition();
        c2->setLhsKind(SMArgumentEntry::eValueSource::Attribute);
        c2->setLhs("IsNightMode");
        c2->setNegated(true);

        checkEq(SMConditionText::preview(root),
                "WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode",
                "preview matches the design example");

        checkEq(SMConditionText::cpp(root),
                "walkRequested() && (hasWaiting(count) || count >= <Data>::MIN_WAITING) && !isNightMode()",
                "C++ codegen matches the design example");
    }
}

//////////////////////////////////////////////////////////////////////////
// AC1: SMConditionToken round-trip (render -> parse -> render is a fixed point)
//////////////////////////////////////////////////////////////////////////

namespace
{
    void roundtrip(const char* label, const QString& token)
    {
        SMConditionEntry leaf;
        QString error;
        const bool ok = SMConditionToken::parseLeaf(token, leaf, error);
        check(ok, label);
        if (ok == false)
        {
            std::printf("         parse error: %s\n", error.toStdString().c_str());
            return;
        }

        checkEq(SMConditionToken::renderLeaf(leaf), token, label);
    }

    void testTokenRoundTrip()
    {
        std::printf("[SM-21-03] token round-trip for every operand kind\n");

        roundtrip("attr bool test",        "attr::IsNightMode");
        roundtrip("negated bool test",     "!attr::IsNightMode");
        roundtrip("param >= const",        "arg::count >= const::MIN_WAITING");
        roundtrip("attr == literal",       "attr::mCount == val::5");
        roundtrip("cond, no args",         "cond::HasWaiting");
        roundtrip("cond, one arg",         "cond::HasWaiting(arg::count)");
        roundtrip("cond, two args",        "cond::InRange(arg::count, const::MIN_WAITING)");
        roundtrip("quoted string literal", "attr::name == val::\"a b\"");
        roundtrip("expression row",        "expr::{ a < b && c }");
        roundtrip("lambda row",            "lambda::{ return x; }");
        roundtrip("negated comparison",    "!arg::count == val::0");

        // Structural spot checks.
        SMConditionEntry leaf;
        QString error;
        CHECK(SMConditionToken::parseLeaf("arg::count >= const::MIN_WAITING", leaf, error));
        CHECK(leaf.getLhsKind() == SMArgumentEntry::eValueSource::Param);
        CHECK(leaf.getLhs() == "count");
        CHECK(leaf.getOperator() == SMConditionEntry::eOperator::GreaterEqual);
        CHECK(leaf.getRhsKind() == SMArgumentEntry::eValueSource::Constant);
        CHECK(leaf.getRhs() == "MIN_WAITING");

        CHECK(SMConditionToken::parseLeaf("cond::HasWaiting(arg::count)", leaf, error));
        CHECK(leaf.getLhsKind() == SMArgumentEntry::eValueSource::Condition);
        CHECK(leaf.getArguments().size() == 1);
        CHECK((leaf.getArguments().isEmpty() == false)
              && (leaf.getArguments().first().getSource() == SMArgumentEntry::eValueSource::Param)
              && (leaf.getArguments().first().getValue() == "count"));

        CHECK(SMConditionToken::parseLeaf("lambda::{ return x; }", leaf, error));
        CHECK(leaf.isLambdaRow());
        CHECK(leaf.getBody() == " return x; ");

        // A bad prefix is rejected with a reason.
        CHECK(SMConditionToken::parseLeaf("bogus::name", leaf, error) == false);
        CHECK(error.isEmpty() == false);
    }
}

//////////////////////////////////////////////////////////////////////////
// AC2: one combinator per group
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testSingleCombinator()
    {
        std::printf("[SM-21-03] one combinator per group (mixed && / || rejected)\n");

        QString reason;
        CHECK(SMConditionToken::hasSingleCombinator("attr::a && attr::b && attr::c", reason));
        CHECK(SMConditionToken::hasSingleCombinator("attr::a || attr::b", reason));
        CHECK(SMConditionToken::hasSingleCombinator("attr::a", reason));

        reason.clear();
        CHECK(SMConditionToken::hasSingleCombinator("attr::a && attr::b || attr::c", reason) == false);
        CHECK(reason.isEmpty() == false);

        // Mixing inside an explicit sub-group (parentheses) is allowed at this level.
        CHECK(SMConditionToken::hasSingleCombinator("attr::a && (attr::b || attr::c)", reason));
    }
}

//////////////////////////////////////////////////////////////////////////
// AC4: type-compatibility seed (spec 6.9)
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testTypeCompat()
    {
        std::printf("[SM-21-03] type-compatibility seed follows spec 6.9\n");

        using Op = SMConditionEntry::eOperator;

        // Compatible pairs -> empty reason.
        CHECK(SMTypeCompat::areComparable("uint16", Op::Equal, "uint16").isEmpty());
        CHECK(SMTypeCompat::areComparable("uint8",  Op::Less,  "uint32").isEmpty());   // widen
        CHECK(SMTypeCompat::areComparable("int32",  Op::LessEqual, "double").isEmpty()); // rule 4
        CHECK(SMTypeCompat::areComparable("float",  Op::Equal, "double").isEmpty());    // float->double
        CHECK(SMTypeCompat::areComparable("bool",   Op::Equal, "bool").isEmpty());
        CHECK(SMTypeCompat::areComparable("char",   Op::Greater, "char").isEmpty());
        CHECK(SMTypeCompat::areComparable("uint16", Op::None,  "int16").isEmpty());     // no operator

        // Violations -> a non-empty reason.
        CHECK(SMTypeCompat::areComparable("bool",   Op::Less,  "bool").isEmpty() == false);      // ordering on bool
        CHECK(SMTypeCompat::areComparable("String", Op::Greater, "String").isEmpty() == false);  // ordering on String
        CHECK(SMTypeCompat::areComparable("uint16", Op::Greater, "int16").isEmpty() == false);   // sign mix
        CHECK(SMTypeCompat::areComparable("uint16", Op::Equal, "int16").isEmpty() == false);     // sign mix (equality)
        CHECK(SMTypeCompat::areComparable("int64",  Op::Less,  "double").isEmpty() == false);    // int64 !-> double
        CHECK(SMTypeCompat::areComparable("bool",   Op::Equal, "uint8").isEmpty() == false);     // bool vs numeric
    }
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char** /*argv*/)
{
    std::printf("==== SM condition engine tests (SM-21-03) ====\n");

    testPreviewAndCpp();
    testTokenRoundTrip();
    testSingleCombinator();
    testTypeCompat();

    std::printf("---- %d checks, %d failure(s) ----\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
