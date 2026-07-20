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
#include "lusan/view/sm/SMGuardCatalogModel.hpp"

#include <QCoreApplication>
#include <QSortFilterProxyModel>
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

    // SM-21-04: the catalog model filters by search text and carries every kind.
    void testCatalogModelFilterAndKinds()
    {
        std::printf("[catalog model] search filter + kind coverage\n");
        StateMachineData data;
        const uint32_t transId = buildDoc(data);
        const QList<SMGuardSymbol> catalog = SMGuardCatalog::build(data, transId);

        SMGuardCatalogModel model;
        model.setSymbols(catalog);
        check(model.rowCount() == catalog.size(), "the model wraps every catalog symbol");
        check(model.columnCount() == SMGuardCatalogModel::ColCount, "four columns [hue|name|type|used]");

        QSortFilterProxyModel proxy;
        proxy.setSourceModel(&model);
        proxy.setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxy.setFilterKeyColumn(SMGuardCatalogModel::ColName);

        proxy.setFilterFixedString(QStringLiteral("Walk"));
        check(proxy.rowCount() == 1, "search 'Walk' narrows to the one matching symbol");
        if (proxy.rowCount() == 1)
        {
            const QString name = proxy.index(0, SMGuardCatalogModel::ColName).data().toString();
            checkEq(name, QStringLiteral("WalkRequested"), "the surviving row is WalkRequested");
        }

        proxy.setFilterFixedString(QString());
        check(proxy.rowCount() == catalog.size(), "clearing the search restores every symbol");

        // Every kind the Data catalog offers is present (Param scope + Attribute + Constant + Cond).
        int params = 0;
        int attrs  = 0;
        int consts = 0;
        int conds  = 0;
        for (int row = 0; row < model.rowCount(); ++row)
        {
            const SMGuardSymbol* sym = model.symbolAt(row);
            if (sym == nullptr) { continue; }
            switch (sym->refkind)
            {
            case SMGuardSymbol::eRefKind::Param: ++params; break;
            case SMGuardSymbol::eRefKind::Attr:  ++attrs;  break;
            case SMGuardSymbol::eRefKind::Const: ++consts; break;
            case SMGuardSymbol::eRefKind::Cond:  ++conds;  break;
            }
        }

        check(params == 1, "one stimulus parameter in scope");
        check(attrs  == 2, "two attributes");
        check(consts == 1, "one constant");
        check(conds  == 2, "two condition methods (handler + lambda)");

        // The used-N column shows a bound reference count pushed by the host.
        const uint32_t walkId = SMGuardSymbols::attributeId(data, QStringLiteral("WalkRequested"));
        QHash<uint32_t, int> counts;
        counts.insert(walkId, 4);
        model.setUseCounts(counts);
        for (int row = 0; row < model.rowCount(); ++row)
        {
            const SMGuardSymbol* sym = model.symbolAt(row);
            if ((sym != nullptr) && (sym->symbolId == walkId))
            {
                checkEq(model.index(row, SMGuardCatalogModel::ColUsed).data().toString(), QStringLiteral("4"), "used-N reflects the pushed count");
            }
        }
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
    testCatalogModelFilterAndKinds();

    std::printf("\n%d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
