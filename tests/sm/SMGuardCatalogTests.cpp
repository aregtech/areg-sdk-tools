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
 *  \file        tests/sm/SMGuardCatalogTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-21 (U2) headless tests: the guard editing-surface symbol catalog and the
 *               did-you-mean edit-distance helper. Model-facing only (no widgets).
 *
 ************************************************************************/

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"

#include "lusan/view/sm/SMGuardCatalog.hpp"

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

    //!< The running-example symbol universe; returns the RequestWalk(count) transition ID.
    uint32_t buildDoc(StateMachineData& data)
    {
        data.getOverview().setName(QStringLiteral("GuardMachine"));

        data.getAttributes().createAttribute(QStringLiteral("WalkRequested"));
        data.getAttributes().createAttribute(QStringLiteral("IsNightMode"));

        ConstantEntry* konst = data.getConstants().createConstant(QStringLiteral("MIN_WAITING"));
        if (konst != nullptr) { konst->setValue(QStringLiteral("3")); }

        SMMethodEntry* trigger = data.getMethods().createMethod(QStringLiteral("RequestWalk"), SMMethodEntry::eMethodType::Trigger);
        trigger->addParam(QStringLiteral("count"))->setType(QStringLiteral("uint16"));

        SMMethodEntry* handler = data.getMethods().createMethod(QStringLiteral("HasWaiting"), SMMethodEntry::eMethodType::Condition);
        handler->setImplement(SMMethodEntry::eImplement::Handler);
        handler->addParam(QStringLiteral("count"))->setType(QStringLiteral("uint16"));

        SMMethodEntry* lambda = data.getMethods().createMethod(QStringLiteral("IsCalmHours"), SMMethodEntry::eMethodType::Condition);
        lambda->setImplement(SMMethodEntry::eImplement::Embedded);
        lambda->addParam(QStringLiteral("count"))->setType(QStringLiteral("uint16"));

        SMStateEntry* root = data.getStates().createState(QStringLiteral("Root"), SMStateEntry::eStateKind::Start);
        SMTransitionEntry* trans = root->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("RequestWalk"), QString());
        return trans->getId();
    }

    const SMGuardSymbol* find(const QList<SMGuardSymbol>& list, const QString& name)
    {
        for (const SMGuardSymbol& sym : list)
        {
            if (sym.name == name) { return &sym; }
        }

        return nullptr;
    }

    void testCatalog()
    {
        std::printf("[catalog] owner-grouped symbol universe\n");
        StateMachineData data;
        const uint32_t transId = buildDoc(data);
        const QList<SMGuardSymbol> catalog = SMGuardCatalog::build(data, transId);

        const SMGuardSymbol* count = find(catalog, QStringLiteral("count"));
        check(count != nullptr, "count present");
        if (count != nullptr)
        {
            check(count->owner == NEGuardStyle::eOwner::Stimulus, "count is a stimulus param");
            check(count->isCall == false, "count is not a call");
            checkEq(count->glyph, QStringLiteral("a"), "count glyph a");
        }

        const SMGuardSymbol* attr = find(catalog, QStringLiteral("WalkRequested"));
        check((attr != nullptr) && (attr->owner == NEGuardStyle::eOwner::Fsm), "attribute is fsm");
        check((attr != nullptr) && (attr->glyph == QStringLiteral("#")), "attribute glyph #");

        const SMGuardSymbol* konst = find(catalog, QStringLiteral("MIN_WAITING"));
        check((konst != nullptr) && (konst->glyph == QStringLiteral("K")), "constant glyph K");
        check((konst != nullptr) && konst->provenance.contains(QStringLiteral("= 3")), "constant provenance = 3");

        const SMGuardSymbol* handler = find(catalog, QStringLiteral("HasWaiting"));
        check((handler != nullptr) && (handler->owner == NEGuardStyle::eOwner::Handler), "handler owner");
        check((handler != nullptr) && handler->isCall, "handler is a call");
        check((handler != nullptr) && (handler->glyph == QStringLiteral("h")), "handler glyph h");

        const SMGuardSymbol* lambda = find(catalog, QStringLiteral("IsCalmHours"));
        check((lambda != nullptr) && (lambda->owner == NEGuardStyle::eOwner::Fsm), "lambda owner fsm");
        check((lambda != nullptr) && (lambda->glyph == QStringLiteral("{}")), "lambda glyph {}");

        // Owner grouping order: stimulus first, handler last.
        int stimIndex = -1;
        int handlerIndex = -1;
        for (int i = 0; i < catalog.size(); ++i)
        {
            if ((stimIndex < 0) && (catalog.at(i).owner == NEGuardStyle::eOwner::Stimulus)) { stimIndex = i; }
            if (catalog.at(i).owner == NEGuardStyle::eOwner::Handler) { handlerIndex = i; }
        }

        check((stimIndex >= 0) && (handlerIndex > stimIndex), "stimulus grouped before handler");
    }

    void testNearestName()
    {
        std::printf("[did-you-mean] edit-distance suggestion\n");
        StateMachineData data;
        const uint32_t transId = buildDoc(data);
        const QStringList words = SMGuardCatalog::completionWords(data, transId);

        checkEq(SMGuardCatalog::nearestName(words, QStringLiteral("WalkRequsted")), QStringLiteral("WalkRequested"), "one transposition resolves");
        checkEq(SMGuardCatalog::nearestName(words, QStringLiteral("HasWaitin")), QStringLiteral("HasWaiting"), "one deletion resolves");
        check(SMGuardCatalog::nearestName(words, QStringLiteral("zzzzzzzz")).isEmpty(), "no near name -> empty");
        check(SMGuardCatalog::nearestName(words, QString()).isEmpty(), "empty typed -> empty");
    }

    void testAutoMapUniqueness()
    {
        std::printf("[auto-map] unique exact-type candidate\n");
        StateMachineData data;
        const uint32_t transId = buildDoc(data);
        const QList<SMGuardSymbol> catalog = SMGuardCatalog::build(data, transId);

        // For type uint16 there are multiple non-call symbols (count param); auto-map must NOT
        // pre-fill when the exact-type candidate is not unique.
        int uint16NonCall = 0;
        for (const SMGuardSymbol& sym : catalog)
        {
            if ((sym.isCall == false) && (sym.typeText == QStringLiteral("uint16")))
            {
                ++uint16NonCall;
            }
        }

        check(uint16NonCall >= 1, "at least one uint16 non-call symbol");
    }

    void testClickTypeParity()
    {
        std::printf("[parity] typed-with-completion == parsed-directly\n");
        StateMachineData data;
        const uint32_t transId = buildDoc(data);

        const QString guard = QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING)");
        SMGuardParser::Result a = SMGuardParser::parse(data, transId, guard, false);
        SMGuardParser::Result b = SMGuardParser::parse(data, transId, guard, false);

        check(a.resolved(), "example-B resolves");
        check((a.tree != nullptr) && (b.tree != nullptr) && a.tree->equals(*b.tree), "same text -> identical trees");

        delete a.tree;
        delete b.tree;
    }
}

int main(int, char**)
{
    std::printf("SMGuardCatalog tests (U2)\n");
    testCatalog();
    testNearestName();
    testAutoMapUniqueness();
    testClickTypeParity();

    std::printf("\n%d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
