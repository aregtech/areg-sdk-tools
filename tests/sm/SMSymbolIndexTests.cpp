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
 *  \file        tests/sm/SMSymbolIndexTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-21-06 unit tests: verbatim-code symbol assistance -- in-scope
 *               completion words, the used-symbols scan, and verbatim where-used.
 *
 ************************************************************************/

#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMSymbolIndex.hpp"

#include <QSet>
#include <QString>
#include <QStringList>
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
}

#define CHECK(cond)  check((cond), #cond)

namespace
{
    //!< Builds a small machine: registries, one trigger with a payload param, a transition
    //!< with a verbatim condition row, an embedded condition body, and an inline-code op.
    struct Fixture
    {
        StateMachineData    data;
        uint32_t            transitionId { 0 };
        uint32_t            inlineId     { 0 };
        QString             conditionText;

        Fixture()
        {
            SMAttributeEntry* speed = data.getAttributes().createAttribute("Speed");
            speed->setType("uint32");
            data.getConstants().createConstant("MAX_SPEED");

            SMMethodEntry* go = data.getMethods().createMethod("Go", SMMethodEntry::eMethodType::Trigger);
            go->addParam("count");

            SMMethodEntry* ready = data.getMethods().createMethod("IsReady", SMMethodEntry::eMethodType::Condition);
            ready->setImplement(SMMethodEntry::eImplement::Embedded);
            ready->setBody("return Speed() > 0;");

            data.getMethods().createMethod("DoWork", SMMethodEntry::eMethodType::Action);

            data.getStates().createState("Idle", SMStateEntry::eStateKind::Start);
            SMStateEntry* run = data.getStates().createState("Run", SMStateEntry::eStateKind::Normal);

            SMTransitionEntry* tr = run->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, "Go", "Idle");
            transitionId = tr->getId();

            // A verbatim expression row referencing known symbols, plus member access, a string
            // literal, and a comment that must all be ignored by the scan.
            conditionText = QStringLiteral("Speed() >= MAX_SPEED && IsReady() && obj.Speed != 0 && x == \"Speed\" /* Speed */");
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(SMConditionEntry::eOperandKind::Expression);
            row->setExpression(conditionText);

            // An inline-code operation referencing a still-declared symbol and an undeclared one.
            SMInlineCode* inl = new SMInlineCode();
            inl->setBody("GhostAttr = 1; Speed();");
            run->getEntryList().addOperation(inl);
            inlineId = inl->getId();
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // AC1: completion offers the correct in-scope identifiers
    //////////////////////////////////////////////////////////////////////////
    void testCompletionScope()
    {
        std::printf("[SM-21-06] completion offers in-scope identifiers\n");
        Fixture fx;

        const QStringList scoped = SMSymbolIndex::completionWords(fx.data, fx.transitionId, true);
        CHECK(scoped.contains("Speed()"));      // attribute getter
        CHECK(scoped.contains("MAX_SPEED"));    // constant
        CHECK(scoped.contains("IsReady()"));    // condition method
        CHECK(scoped.contains("DoWork()"));     // action method
        CHECK(scoped.contains("count"));        // stimulus param (transition scope)
        CHECK(scoped.contains("Go()") == false);// triggers are stimuli, never called

        // Param scope: a non-transition cell must not offer stimulus parameters.
        const QStringList unscoped = SMSymbolIndex::completionWords(fx.data, fx.transitionId, false);
        CHECK(unscoped.contains("Speed()"));
        CHECK(unscoped.contains("count") == false);

        // A Timer stimulus (here: no transition) never yields parameters.
        CHECK(SMSymbolIndex::stimulusParams(fx.data, fx.transitionId) == QStringList{ QStringLiteral("count") });
        CHECK(SMSymbolIndex::stimulusParams(fx.data, 0).isEmpty());
    }

    //////////////////////////////////////////////////////////////////////////
    // AC2: the used-symbols scan lists known refs and ignores the rest
    //////////////////////////////////////////////////////////////////////////
    void testUsedSymbols()
    {
        std::printf("[SM-21-06] used-symbols scan ignores unknown tokens, literals, comments\n");
        Fixture fx;

        const QSet<QString> known = SMSymbolIndex::machineSymbolNames(fx.data);
        const QStringList used = SMSymbolIndex::usedSymbols(fx.conditionText, known);

        CHECK(used.contains("Speed"));          // Speed() -> the attribute name
        CHECK(used.contains("MAX_SPEED"));
        CHECK(used.contains("IsReady"));
        CHECK(used.count("Speed") == 1);        // de-duplicated
        CHECK(used.contains("obj") == false);   // member access owner, unknown
        CHECK(used.contains("x") == false);     // unknown token
        CHECK(used.contains("Go") == false);    // declared but not referenced
        CHECK(used.contains("DoWork") == false);

        // Keywords and pure literals never match a known name.
        CHECK(SMSymbolIndex::usedSymbols(QStringLiteral("return 42 && true;"), known).isEmpty());
    }

    //////////////////////////////////////////////////////////////////////////
    // AC3: verbatim where-used resolves a reference (even to a deleted element)
    //////////////////////////////////////////////////////////////////////////
    void testFindReferences()
    {
        std::printf("[SM-21-06] verbatim where-used resolves references by block\n");
        Fixture fx;

        // Speed is referenced by the condition row, the embedded body, and the inline code.
        const QList<SMSymbolIndex::VerbatimRef> speedRefs = SMSymbolIndex::findReferences(fx.data, "Speed");
        CHECK(speedRefs.size() == 3);

        // An undeclared identifier (a "deleted" element) still resolves to the block that
        // mentions it -- the answer SM-24/25/26 consume.
        const QList<SMSymbolIndex::VerbatimRef> ghost = SMSymbolIndex::findReferences(fx.data, "GhostAttr");
        CHECK(ghost.size() == 1);
        CHECK((ghost.isEmpty() == false) && (ghost.first().elementId == fx.inlineId));

        // The whole verbatim inventory: condition row + embedded body + inline code.
        CHECK(SMSymbolIndex::collectVerbatimBlocks(fx.data).size() == 3);
    }
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char** /*argv*/)
{
    std::printf("==== SM symbol-index tests (SM-21-06) ====\n");

    testCompletionScope();
    testUsedSymbols();
    testFindReferences();

    std::printf("---- %d checks, %d failure(s) ----\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
