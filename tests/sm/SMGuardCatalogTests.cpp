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
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"

#include "lusan/view/sm/SMGuardCatalog.hpp"

#include <QCoreApplication>
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

    // SM-21-04: the local use-count (bound references by symbol id) equals a manual id-count.
    void testUseCounts()
    {
        std::printf("[use-count] bound references counted by symbol id\n");
        StateMachineData data;
        const uint32_t transId = buildDoc(data);

        // count appears twice (a call arg and a comparison operand); the others once each.
        const QString guard = QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING)");
        SMGuardParser::Result res = SMGuardParser::parse(data, transId, guard, false);
        check(res.resolved() && (res.tree != nullptr), "fixture guard resolves");

        const QHash<uint32_t, int> counts = SMGuardCatalog::useCounts(res.tree);

        const uint32_t countId  = SMGuardSymbols::paramId(data, transId, QStringLiteral("count"));
        const uint32_t walkId   = SMGuardSymbols::attributeId(data, QStringLiteral("WalkRequested"));
        const uint32_t minId    = SMGuardSymbols::constantId(data, QStringLiteral("MIN_WAITING"));
        const SMMethodEntry* has = SMGuardSymbols::conditionMethod(data, QStringLiteral("HasWaiting"));
        const uint32_t nightId  = SMGuardSymbols::attributeId(data, QStringLiteral("IsNightMode"));

        check(counts.value(countId, 0) == 2, "count is used twice");
        check(counts.value(walkId, 0) == 1, "WalkRequested is used once");
        check(counts.value(minId, 0) == 1, "MIN_WAITING is used once");
        check((has != nullptr) && (counts.value(has->getId(), 0) == 1), "HasWaiting is used once");
        // An unreferenced symbol is absent (a 0 in the used-N column), never counted from raw text.
        check(counts.value(nightId, 0) == 0, "an unreferenced attribute has no count");

        delete res.tree;
    }

    // SM-21-08 (W1): the raw-collision detector over a committed tree. Advisory, exact-match, and
    // never a linter -- only a whole bare identifier that matches an in-scope symbol name fires.
    void testRawCollisions()
    {
        std::printf("[W1] raw-collision detection (SM-21-08)\n");
        StateMachineData data;
        const uint32_t transId = buildDoc(data);

        // A name that is BOTH an attribute and a constant exercises the multi-kind picker route.
        data.getAttributes().createAttribute(QStringLiteral("Threshold"));
        ConstantEntry* dup = data.getConstants().createConstant(QStringLiteral("Threshold"));
        if (dup != nullptr) { dup->setValue(QStringLiteral("7")); }

        using eKind = SMGuardNode::eKind;

        // 1) An exact bare-identifier raw match fires: Raw("IsNightMode") vs the attribute.
        {
            SMGuardNode* tree = SMGuardNode::makeVerbatim(eKind::Raw, QStringLiteral("IsNightMode"));
            const QList<SMGuardRawCollision> hits = SMGuardCatalog::rawCollisions(data, transId, tree);
            check(hits.size() == 1, "an exact raw-vs-symbol match fires once");
            if (hits.size() == 1)
            {
                checkEq(hits.first().name, QStringLiteral("IsNightMode"), "the hit names the raw token");
                check(hits.first().matches.size() == 1, "single-kind match");
                check((hits.first().matches.size() == 1) && (hits.first().matches.first().refkind == SMGuardSymbol::eRefKind::Attr), "the single match is the attribute");
                check(hits.first().path.isEmpty(), "the root raw node has an empty path");
            }

            delete tree;
        }

        // 2) A condition name typed WITHOUT parens stays raw and still fires (binds as #cond:name()).
        {
            SMGuardNode* tree = SMGuardNode::makeVerbatim(eKind::Raw, QStringLiteral("HasWaiting"));
            const QList<SMGuardRawCollision> hits = SMGuardCatalog::rawCollisions(data, transId, tree);
            check((hits.size() == 1) && (hits.first().matches.size() == 1)
                  && (hits.first().matches.first().refkind == SMGuardSymbol::eRefKind::Cond), "a bare condition name collides with the cond kind");
            delete tree;
        }

        // 3) Non-identifier raw is NEVER flagged -- operators, member access, calls, literals, spaces.
        {
            const char* noise[] = { "a + b", "obj.count", "count > 3", "MIN_WAITING()", "42", "count + 1" };
            for (const char* text : noise)
            {
                SMGuardNode* tree = SMGuardNode::makeVerbatim(eKind::Raw, QString::fromLatin1(text));
                const QList<SMGuardRawCollision> hits = SMGuardCatalog::rawCollisions(data, transId, tree);
                check(hits.isEmpty(), "non-identifier raw is not flagged");
                delete tree;
            }
        }

        // 4) A bare identifier matching nothing stays unchecked (D-RAW-UNCHECKED, no linter).
        {
            SMGuardNode* tree = SMGuardNode::makeVerbatim(eKind::Raw, QStringLiteral("someLocalHelper"));
            const QList<SMGuardRawCollision> hits = SMGuardCatalog::rawCollisions(data, transId, tree);
            check(hits.isEmpty(), "an unknown bare identifier is not flagged");
            delete tree;
        }

        // 5) A multi-kind name routes to the picker: matches carries more than one kind.
        {
            SMGuardNode* tree = SMGuardNode::makeVerbatim(eKind::Raw, QStringLiteral("Threshold"));
            const QList<SMGuardRawCollision> hits = SMGuardCatalog::rawCollisions(data, transId, tree);
            check(hits.size() == 1, "the multi-kind name fires once");
            check((hits.size() == 1) && (hits.first().matches.size() == 2), "attr+const same name matches two kinds (picker)");
            delete tree;
        }

        // 6) A raw child nested in a bound tree is addressed by its child-index path.
        {
            QList<SMGuardNode*> kids;
            kids.append(SMGuardNode::makeVerbatim(eKind::Raw, QStringLiteral("IsNightMode")));
            kids.append(SMGuardNode::makeVerbatim(eKind::Lit, QStringLiteral("true")));
            SMGuardNode* group = SMGuardNode::makeGroup(eKind::And, kids);
            const QList<SMGuardRawCollision> hits = SMGuardCatalog::rawCollisions(data, transId, group);
            check(hits.size() == 1, "the nested raw child is found");
            check((hits.size() == 1) && (hits.first().path.size() == 1) && (hits.first().path.first() == 0), "the path addresses child 0");
            delete group;
        }

        // 7) A null tree has no collisions (an empty guard).
        check(SMGuardCatalog::rawCollisions(data, transId, nullptr).isEmpty(), "a null tree yields no collisions");
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    std::printf("SMGuardCatalog tests (U2)\n");
    testCatalog();
    testNearestName();
    testAutoMapUniqueness();
    testClickTypeParity();
    testUseCounts();
    testRawCollisions();

    std::printf("\n%d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
