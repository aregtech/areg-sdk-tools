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
 *  \file        tests/sm/SMGuardTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-21 (U1) unit tests: the deterministic guard core -- parser resolution,
 *               render round-trip, rename-by-ID, drafts, XML round-trip, codegen preview,
 *               undoable commands and the legacy read-shim. Self-contained (no framework).
 *
 ************************************************************************/

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/common/AttributeDataSection.hpp"
#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/common/ConstantDataSection.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodKind.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardCodegenPreview.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"

#include <QDir>
#include <QFile>
#include <QString>
#include <QUndoStack>
#include <cstdio>
#include <memory>

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

    using eKind = SMGuardNode::eKind;

    /**
     * \brief   Builds a document with the running-example symbol universe and returns the
     *          transition ID triggered by RequestWalk(count : uint16).
     **/
    uint32_t buildDoc(StateMachineData& data)
    {
        data.getOverview().setName(QStringLiteral("GuardMachine"));

        data.getAttributes().createAttribute(QStringLiteral("WalkRequested"));
        data.getAttributes().createAttribute(QStringLiteral("IsNightMode"));

        ConstantEntry* konst = data.getConstants().createConstant(QStringLiteral("MIN_WAITING"));
        if (konst != nullptr) { konst->setValue(QStringLiteral("3")); }

        MethodEntry* trigger = data.getMethods().createMethod(QStringLiteral("RequestWalk"), NEMethod::SmTrigger);
        MethodParameter* tp = trigger->addParam(QStringLiteral("count"));
        tp->setType(QStringLiteral("uint16"));

        MethodEntry* handler = data.getMethods().createMethod(QStringLiteral("HasWaiting"), NEMethod::SmCondition);
        handler->setImplement(MethodEntry::eImplement::Handler);
        handler->addParam(QStringLiteral("count"))->setType(QStringLiteral("uint16"));

        MethodEntry* lambda = data.getMethods().createMethod(QStringLiteral("IsCalmHours"), NEMethod::SmCondition);
        lambda->setImplement(MethodEntry::eImplement::Embedded);
        lambda->addParam(QStringLiteral("count"))->setType(QStringLiteral("uint16"));
        lambda->setBody(QStringLiteral("return count < MIN_WAITING;"));

        // `Root` reacts to a stimulus, so it is an ordinary state: a Kind="Start" is a pseudo-state
        // whose own transitions are the level's initial ones and name no stimulus, and the read
        // shim rewrites a merged-form one on the next load.
        SMStateEntry* begin = data.getStates().createState(QStringLiteral("Begin"), SMStateEntry::eStateKind::Start);
        SMStateEntry* root  = data.getStates().createState(QStringLiteral("Root"), SMStateEntry::eStateKind::Normal);
        begin->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QString(), root->getId(), SMTransitionEntry::eTransitionKind::Initial);
        SMTransitionEntry* trans = root->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("RequestWalk"), 0u, SMTransitionEntry::eTransitionKind::Internal);
        return trans->getId();
    }

    //!< True if the guard tree references \p id via any Attr/Const/Param/Call node.
    bool referencesId(const SMGuardNode* node, uint32_t id)
    {
        if (node == nullptr) { return false; }
        if ((node->getKind() == eKind::Attr) || (node->getKind() == eKind::Const)
            || (node->getKind() == eKind::Param) || (node->getKind() == eKind::Call))
        {
            if (node->getSymbolId() == id) { return true; }
        }
        for (const SMGuardNode* child : node->getChildren())
        {
            if (referencesId(child, id)) { return true; }
        }

        return false;
    }

    QString tempFile(const char* name)
    {
        return QDir::tempPath() + QDir::separator() + QString::fromLatin1(name);
    }

    QByteArray readBytes(const QString& path)
    {
        QFile f(path);
        if (f.open(QIODevice::ReadOnly) == false) { return QByteArray(); }
        const QByteArray b = f.readAll();
        f.close();
        return b;
    }

    bool writeBytes(const QString& path, const QByteArray& content)
    {
        QFile f(path);
        if (f.open(QIODevice::WriteOnly) == false) { return false; }
        const bool ok = (f.write(content) == content.size());
        f.close();
        return ok;
    }
}

//////////////////////////////////////////////////////////////////////////
// Parser resolution (acceptance 16, 18)
//////////////////////////////////////////////////////////////////////////

static void testParserResolution()
{
    std::printf("[U1] parser resolution: bare/call forms, IDs, comparisons\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    const QString text = QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode");
    SMGuardParser::Result result = SMGuardParser::parse(data, tid, text);
    check(result.resolved(), "example B resolves with no error");

    SMGuardNode* root = result.tree;
    check((root != nullptr) && (root->getKind() == eKind::And) && (root->getCount() == 3), "root is a 3-child And");
    if ((root != nullptr) && (root->getCount() == 3))
    {
        // Only IDs are stored (acceptance 16): the Attr node's id equals the declaration's id.
        const uint32_t walkId = SMGuardSymbols::attributeId(data, QStringLiteral("WalkRequested"));
        check(root->childAt(0)->getKind() == eKind::Attr, "child 0 is Attr");
        check(root->childAt(0)->getSymbolId() == walkId, "Attr node carries the attribute's document ID");

        SMGuardNode* orNode = root->childAt(1);
        check((orNode->getKind() == eKind::Or) && (orNode->getCount() == 2), "child 1 is a 2-child Or");
        check(orNode->childAt(0)->getKind() == eKind::Call, "Or child 0 is a Call");
        check(orNode->childAt(0)->getSymbolId() == data.getMethods().findMethod(QStringLiteral("HasWaiting"))->getId(), "Call binds the handler condition ID");
        check(orNode->childAt(0)->getCount() == 1, "HasWaiting has one argument");
        check(orNode->childAt(0)->childAt(0)->getKind() == eKind::Param, "the argument is the stimulus Param");

        SMGuardNode* cmp = orNode->childAt(1);
        check((cmp->getKind() == eKind::Cmp) && (cmp->getOp() == SMGuardNode::eCmpOp::Ge), "Or child 1 is a >= comparison");
        check(cmp->childAt(0)->getKind() == eKind::Param, "cmp lhs is the Param count");
        check(cmp->childAt(1)->getKind() == eKind::Const, "cmp rhs is the Const MIN_WAITING");

        check(root->childAt(2)->getKind() == eKind::Not, "child 2 is Not");
        check(root->childAt(2)->childAt(0)->getKind() == eKind::Attr, "Not wraps the IsNightMode attribute");
    }
    delete root;

    // SM-536: a bare name that resolves to no defined parameter/attribute/constant is UNDEFINED --
    // an error, not silent raw C++. Attributes/parameters/constants are typed data objects that must
    // be declared; only the explicit "raw code" mode (allowRaw) accepts unvalidated C++. (This
    // supersedes the earlier D-RAW rule that a bare unknown name was raw by default.)
    SMGuardParser::Result bare = SMGuardParser::parse(data, tid, QStringLiteral("WalkRequsted && count > 2"));
    check(bare.hasError(), "an unmarked unknown identifier is an error, not raw (SM-536)");
    delete bare.tree;

    // The same text under the explicit "raw code" opt-in (allowRaw) is accepted without error: the
    // user has taken responsibility for correctness.
    SMGuardParser::Result bareRaw = SMGuardParser::parse(data, tid, QStringLiteral("WalkRequsted && count > 2"), true);
    check(bareRaw.hasError() == false, "the same bare name under explicit raw mode carries no error");
    delete bareRaw.tree;

    // An EXPLICIT kind that does not resolve IS an error -- the user stated the kind (contrast D-RAW).
    SMGuardParser::Result explicitBad = SMGuardParser::parse(data, tid, QStringLiteral("#attr:WalkRequsted && count > 2"));
    check(explicitBad.hasError(), "an explicit #attr:typo is an error (a stated kind must resolve)");
    delete explicitBad.tree;

    // Explicit raw only via the flag (the "Keep as raw C++" quick-fix).
    SMGuardParser::Result rawOff = SMGuardParser::parse(data, tid, QStringLiteral("count $ 3"), false);
    check(rawOff.hasError(), "unparseable input is an error without allowRaw");
    delete rawOff.tree;

    SMGuardParser::Result rawOn = SMGuardParser::parse(data, tid, QStringLiteral("count $ 3"), true);
    check((rawOn.tree != nullptr) && (rawOn.tree->getKind() == eKind::Raw), "allowRaw wraps the fragment as a Raw node");
    check(rawOn.hasError() == false, "an accepted raw fragment carries no error");
    delete rawOn.tree;
}

static void testConditionWeight()
{
    std::printf("[U1] guard resolution weight: param > condition > attribute > constant\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    // A zero-argument condition binds from a bare name, as a call.
    MethodEntry* ready = data.getMethods().createMethod(QStringLiteral("Ready"), NEMethod::SmCondition);
    ready->setImplement(MethodEntry::eImplement::Handler);
    SMGuardParser::Result r1 = SMGuardParser::parse(data, tid, QStringLiteral("Ready"));
    check(r1.resolved() && (r1.tree != nullptr) && (r1.tree->getKind() == eKind::Call)
            && (r1.tree->getSymbolId() == ready->getId()), "a bare zero-argument condition binds as a call");
    delete r1.tree;

    // A condition that declares a non-defaulted argument never binds from a bare name.
    MethodEntry* check2 = data.getMethods().createMethod(QStringLiteral("Check"), NEMethod::SmCondition);
    check2->setImplement(MethodEntry::eImplement::Handler);
    check2->addParam(QStringLiteral("n"))->setType(QStringLiteral("uint16"));
    SMGuardParser::Result r2 = SMGuardParser::parse(data, tid, QStringLiteral("Check"));
    check(r2.hasError(), "a bare multi-argument condition does not bind");
    delete r2.tree;

    // A condition outranks an attribute of the same name in a guard.
    data.getAttributes().createAttribute(QStringLiteral("Ping"))->setType(QStringLiteral("bool"));
    MethodEntry* pingCond = data.getMethods().createMethod(QStringLiteral("Ping"), NEMethod::SmCondition);
    pingCond->setImplement(MethodEntry::eImplement::Handler);
    SMGuardParser::Result r3 = SMGuardParser::parse(data, tid, QStringLiteral("Ping"));
    check(r3.resolved() && (r3.tree != nullptr) && (r3.tree->getKind() == eKind::Call)
            && (r3.tree->getSymbolId() == pingCond->getId()), "a condition outranks an attribute of the same name");
    delete r3.tree;

    // A stimulus parameter outranks a condition of the same name.
    data.getMethods().findMethod(QStringLiteral("RequestWalk"))->addParam(QStringLiteral("Ping"))->setType(QStringLiteral("bool"));
    SMGuardParser::Result r4 = SMGuardParser::parse(data, tid, QStringLiteral("Ping"));
    check(r4.resolved() && (r4.tree != nullptr) && (r4.tree->getKind() == eKind::Param)
          , "a stimulus parameter outranks a condition of the same name");
    delete r4.tree;
}

static void testParamVsCall()
{
    std::printf("[U1] param vs param() disambiguation + shadowing warning (acceptance 18)\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    // A named lambda 'param' coexists with the stimulus parameter 'param' (call form vs bare).
    MethodEntry* lambda = data.getMethods().createMethod(QStringLiteral("param"), NEMethod::SmCondition);
    lambda->setImplement(MethodEntry::eImplement::Embedded);
    // Add 'param' as a stimulus parameter of RequestWalk.
    data.getMethods().findMethod(QStringLiteral("RequestWalk"))->addParam(QStringLiteral("param"))->setType(QStringLiteral("uint16"));

    SMGuardParser::Result r = SMGuardParser::parse(data, tid, QStringLiteral("param() && param > 0"));
    check(r.resolved(), "'param() && param > 0' resolves");
    if ((r.tree != nullptr) && (r.tree->getKind() == eKind::And) && (r.tree->getCount() == 2))
    {
        check(r.tree->childAt(0)->getKind() == eKind::Call, "call form binds the lambda");
        check(r.tree->childAt(0)->getSymbolId() == lambda->getId(), "call form binds the lambda ID");
        SMGuardNode* cmp = r.tree->childAt(1);
        check((cmp->getKind() == eKind::Cmp) && (cmp->childAt(0)->getKind() == eKind::Param), "bare form binds the stimulus parameter");
    }
    delete r.tree;

    // A stimulus parameter and an attribute sharing a name: the parameter wins by weight, and the
    // badge already says which symbol is meant, so there is no diagnostic.
    data.getAttributes().createAttribute(QStringLiteral("shadow"));
    data.getMethods().findMethod(QStringLiteral("RequestWalk"))->addParam(QStringLiteral("shadow"))->setType(QStringLiteral("bool"));
    SMGuardParser::Result s = SMGuardParser::parse(data, tid, QStringLiteral("shadow"));
    bool warned = false;
    for (const SMGuardParser::Diagnostic& d : s.diagnostics)
    {
        if (d.severity == SMGuardParser::eSeverity::Warning) { warned = true; }
    }
    check((s.tree != nullptr) && (s.tree->getKind() == eKind::Param), "shadowed bare name binds the parameter");
    check(warned == false, "a parameter hiding an attribute is silent");
    delete s.tree;
}

//////////////////////////////////////////////////////////////////////////
// SM-21-03: the @kind:name grammar (typed == clicked), D-NOSET, named/mixed args
//////////////////////////////////////////////////////////////////////////

//!< Parses \p text and returns the resolved tree (caller owns), or nullptr when unresolved.
static SMGuardNode* parseOk(StateMachineData& data, uint32_t tid, const QString& text)
{
    SMGuardParser::Result r = SMGuardParser::parse(data, tid, text);
    if (r.resolved() == false) { delete r.tree; return nullptr; }
    return r.tree;
}

static void testKindGrammar()
{
    std::printf("[SM-21-03] @kind:name grammar: typed == clicked, prefixes, D-NOSET\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    // Typed (explicit @kind:) and clicked/bare forms resolve to IDENTICAL trees (P3).
    {
        SMGuardNode* typed   = parseOk(data, tid, QStringLiteral("#attr:WalkRequested && #cond:HasWaiting(#param:count) && !#attr:IsNightMode"));
        SMGuardNode* clicked = parseOk(data, tid, QStringLiteral("WalkRequested && HasWaiting(count) && !IsNightMode"));
        check((typed != nullptr) && (clicked != nullptr) && typed->equals(*clicked), "typed @kind: form == bare/clicked form (identical trees)");
        delete typed;
        delete clicked;
    }

    // Each explicit kind binds the right node kind and id.
    {
        SMGuardNode* p = parseOk(data, tid, QStringLiteral("#param:count >= #const:MIN_WAITING"));
        check((p != nullptr) && (p->getKind() == eKind::Cmp), "#param/#const comparison resolves");
        if ((p != nullptr) && (p->getKind() == eKind::Cmp))
        {
            check(p->childAt(0)->getKind() == eKind::Param, "#param: binds a Param node");
            check(p->childAt(1)->getKind() == eKind::Const, "#const: binds a Const node");
        }
        delete p;
        SMGuardNode* c = parseOk(data, tid, QStringLiteral("#cond:IsCalmHours(count)"));
        check((c != nullptr) && (c->getKind() == eKind::Call) && (c->getSymbolId() == data.getMethods().findMethod(QStringLiteral("IsCalmHours"))->getId()), "#cond: binds the method call");
        delete c;
    }

    // SM-536: a bare unknown name is an error (not silent raw); an explicit #param:unknown is also
    // an error. Both must resolve to a defined symbol -- only explicit "raw code" mode opts out.
    {
        SMGuardParser::Result raw = SMGuardParser::parse(data, tid, QStringLiteral("nonesuch"));
        check(raw.hasError(), "bare unknown is an error, not raw (SM-536)");
        delete raw.tree;
        SMGuardParser::Result bad = SMGuardParser::parse(data, tid, QStringLiteral("#param:nonesuch"));
        check(bad.hasError(), "#param:unknown is an error (explicit kind must resolve)");
        delete bad.tree;
        SMGuardParser::Result badArg = SMGuardParser::parse(data, tid, QStringLiteral("#arg:count"));
        check(badArg.hasError(), "a standalone #arg: is a misuse error");
        delete badArg.tree;
    }

    // D-NOSET: '=' in a guard is flagged (warning) and parsed as '==', never a setter.
    {
        SMGuardParser::Result eq = SMGuardParser::parse(data, tid, QStringLiteral("#attr:WalkRequested = #const:MIN_WAITING"));
        bool warned = false;
        for (const SMGuardParser::Diagnostic& d : eq.diagnostics)
        {
            if ((d.severity == SMGuardParser::eSeverity::Warning) && d.message.contains(QStringLiteral("=="))) { warned = true; }
        }
        check(warned, "D-NOSET: '=' raises a 'did you mean ==' warning");
        check((eq.tree != nullptr) && (eq.tree->getKind() == eKind::Cmp) && (eq.tree->getOp() == SMGuardNode::eCmpOp::Eq), "'=' is parsed as an equality comparison, not a setter");
        delete eq.tree;
    }

    // Collision (D-REVEAL substrate): a param and an attribute share a name -> render emits the
    // disambiguating @kind: prefix so the tree round-trips (parse(render(tree)) == tree).
    {
        data.getAttributes().createAttribute(QStringLiteral("value"));
        data.getMethods().findMethod(QStringLiteral("RequestWalk"))->addParam(QStringLiteral("value"))->setType(QStringLiteral("uint16"));
        SMGuardNode* tree = parseOk(data, tid, QStringLiteral("#param:value > #attr:value"));
        check(tree != nullptr, "same-name param/attr comparison resolves");
        if (tree != nullptr)
        {
            const QString rendered = SMGuardRender::text(data, tid, *tree);
            check(rendered.contains(QStringLiteral("#attr:value")), "render emits #attr: to disambiguate the collision");
            SMGuardNode* again = parseOk(data, tid, rendered);
            check((again != nullptr) && tree->equals(*again), "collision tree round-trips: parse(render(tree)) == tree");
            delete again;
        }
        delete tree;
    }
}

static void testCallArgGrammar()
{
    std::printf("[SM-21-03] call args: positional, named (#arg:), Python mixed rule (D-MIXED)\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    // A 3-formal condition method for argument mapping.
    MethodEntry* cond = data.getMethods().createMethod(QStringLiteral("check3"), NEMethod::SmCondition);
    cond->setImplement(MethodEntry::eImplement::Handler);
    const uint32_t p1 = cond->addParam(QStringLiteral("p1"))->getId();
    const uint32_t p2 = cond->addParam(QStringLiteral("p2"))->getId();
    const uint32_t p3 = cond->addParam(QStringLiteral("p3"))->getId();
    (void)p2;

    // Positional binds each formal by position (keyed on the formal id, not the index).
    {
        SMGuardNode* t = parseOk(data, tid, QStringLiteral("check3(count, 5, 9)"));
        check((t != nullptr) && (t->getKind() == eKind::Call) && (t->getCount() == 3), "3 positional args bind");
        if ((t != nullptr) && (t->getCount() == 3))
        {
            check(t->childAt(0)->getArgFormalId() == p1, "positional arg 0 -> formal p1");
            check(t->childAt(2)->getArgFormalId() == p3, "positional arg 2 -> formal p3");
        }
        delete t;
    }

    // Mixed: a positional then a named skipping p2 -> a binds p1, c binds p3, p2 unmapped (12.8).
    {
        SMGuardNode* t = parseOk(data, tid, QStringLiteral("check3(count, #arg:p3 = 9)"));
        check((t != nullptr) && (t->getKind() == eKind::Call) && (t->getCount() == 2), "mixed positional+named binds two args");
        if ((t != nullptr) && (t->getCount() == 2))
        {
            check(t->childAt(0)->getArgFormalId() == p1, "positional -> p1");
            check(t->childAt(1)->getArgFormalId() == p3, "named #arg:p3 -> p3 (p2 left as a ghost)");
        }
        delete t;
    }

    // Named-only, out of order.
    {
        SMGuardNode* t = parseOk(data, tid, QStringLiteral("check3(#arg:p3 = 9, #arg:p1 = count)"));
        check((t != nullptr) && (t->getCount() == 2), "named-only args resolve out of order");
        if ((t != nullptr) && (t->getCount() == 2))
        {
            check(t->childAt(0)->getArgFormalId() == p3, "first named binds p3");
            check(t->childAt(1)->getArgFormalId() == p1, "second named binds p1");
        }
        delete t;
    }

    // D-MIXED: a positional AFTER a named argument is a syntax error with the exact diagnostic.
    {
        SMGuardParser::Result r = SMGuardParser::parse(data, tid, QStringLiteral("check3(#arg:p1 = count, 9)"));
        check(r.resolved() == false, "positional after named does not resolve");
        bool fired = false;
        for (const SMGuardParser::Diagnostic& d : r.diagnostics)
        {
            if (d.message == QStringLiteral("positional argument after named argument")) { fired = true; }
        }
        check(fired, "the 'positional argument after named argument' diagnostic fires");
        delete r.tree;
    }

    // Interior ghost (12.8 / D-GHOST): a value mapped after a hole renders NAMED and the tree
    // round-trips through the canonical text -- the ghost is display-only, never in the text.
    {
        SMGuardNode* t = parseOk(data, tid, QStringLiteral("check3(count, #arg:p3 = 9)"));
        check(t != nullptr, "interior-ghost call resolves");
        if (t != nullptr)
        {
            const QString canonical = SMGuardRender::text(data, tid, *t);
            checkEq(canonical, QStringLiteral("check3(count, #arg:p3 = 9)"), "canonical text keeps count positional and p3 named (no empty comma, no ghost)");
            check(canonical.contains(QStringLiteral("<")) == false, "the canonical text carries no ghost marker");
            SMGuardNode* again = parseOk(data, tid, canonical);
            check((again != nullptr) && t->equals(*again), "interior-ghost tree round-trips: parse(text(tree)) == tree");
            delete again;

            // The field/display render DOES show the labeled ghost for the unmapped p2.
            const SMGuardRender::Rendered display = SMGuardRender::render(data, tid, *t);
            check(display.text.contains(QStringLiteral("<p2>")), "the display render shows the <p2> ghost");
        }
        delete t;
    }
}

static void testRenderChips()
{
    std::printf("[SM-21-03] render chips: kinds/names, D-REVEAL collision reveal\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    // A plain guard: one chip per reference, none revealed (no name collision).
    {
        SMGuardNode* t = parseOk(data, tid, QStringLiteral("WalkRequested && HasWaiting(count)"));
        check(t != nullptr, "guard resolves");
        if (t != nullptr)
        {
            const SMGuardRender::Rendered r = SMGuardRender::render(data, tid, *t);
            check(r.chips.size() == 3, "three chips: attr, cond, param");
            bool anyReveal = false;
            for (const SMGuardRender::Chip& c : r.chips) { anyReveal = anyReveal || c.reveal; }
            check(anyReveal == false, "no chip is revealed without a collision");
            // Each chip's span in the rendered text equals its (bare) name here.
            for (const SMGuardRender::Chip& c : r.chips)
            {
                checkEq(r.text.mid(c.start, c.length), c.name, "a chip span is the bare rendered name");
            }

            // Each chip's node path addresses its node in the tree (the kind-cycle relies on this):
            // root And [ Attr, Call [ Param ] ] -> chips carry [0], [1], [1,0].
            if (r.chips.size() == 3)
            {
                check(r.chips.at(0).path == QList<int>({ 0 }), "attr chip path is [0]");
                check(r.chips.at(1).path == QList<int>({ 1 }), "call chip path is [1]");
                check(r.chips.at(2).path == QList<int>({ 1, 0 }), "param arg chip path is [1,0]");
                check((t->childAt(1) != nullptr) && (t->childAt(1)->getKind() == eKind::Call), "the node at [1] is the call the chip path points to");
            }
        }
        delete t;
    }

    // D-REVEAL: a param and an attribute share the name "value" -> BOTH chips reveal, always.
    {
        data.getAttributes().createAttribute(QStringLiteral("value"));
        data.getMethods().findMethod(QStringLiteral("RequestWalk"))->addParam(QStringLiteral("value"))->setType(QStringLiteral("uint16"));
        SMGuardNode* t = parseOk(data, tid, QStringLiteral("#param:value > #attr:value"));
        check(t != nullptr, "collision guard resolves");
        if (t != nullptr)
        {
            const SMGuardRender::Rendered r = SMGuardRender::render(data, tid, *t);
            check(r.chips.size() == 2, "two chips for the collision");
            int revealed = 0;
            for (const SMGuardRender::Chip& c : r.chips)
            {
                if (c.reveal) { ++revealed; }
                checkEq(c.name, QStringLiteral("value"), "both chips display the same name");
            }
            check(revealed == 2, "D-REVEAL: every colliding chip is revealed");
        }
        delete t;
    }
}

static void testInvalidReferenceChips()
{
    std::printf("[SM-21-03b] invalid reference chips: broken target keeps its name, typed unknown vs raw\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);
    const uint32_t deadId = 0x7FFFFFFFu;    // an id no declaration owns

    // A reference whose target is gone shows as an invalid chip carrying its last known name.
    {
        SMGuardNode* node = SMGuardNode::makeRef(eKind::Attr, deadId);
        node->setCacheName(QStringLiteral("GoneAttr"));
        const SMGuardRender::Rendered r = SMGuardRender::render(data, tid, *node);
        check(r.chips.size() == 1, "the broken reference produces one chip");
        if (r.chips.isEmpty() == false)
        {
            check(r.chips.at(0).invalid, "the chip is marked invalid");
            check(r.chips.at(0).role == SMGuardRender::eRole::Invalid, "the chip carries the invalid role");
            checkEq(r.chips.at(0).name, QStringLiteral("GoneAttr"), "the chip keeps the last known name");
            check(r.chips.at(0).reveal == false, "an invalid chip never reveals a kind prefix");
            checkEq(r.text.mid(r.chips.at(0).start, r.chips.at(0).length), QStringLiteral("GoneAttr"), "the rendered span is the bare name (no #kind: prefix)");
        }
        delete node;
    }

    // A broken reference with no cached name never collapses to a zero-width chip.
    {
        SMGuardNode* node = SMGuardNode::makeRef(eKind::Const, deadId);
        const SMGuardRender::Rendered r = SMGuardRender::render(data, tid, *node);
        check((r.chips.size() == 1) && r.chips.at(0).invalid, "a nameless broken reference is still invalid");
        if (r.chips.isEmpty() == false)
        {
            checkEq(r.chips.at(0).name, QStringLiteral("?"), "a nameless broken reference shows a placeholder");
        }
        delete node;
    }

    // A typed name that resolves to nothing reads as an invalid reference, not a raw fragment.
    {
        SMGuardNode* node = SMGuardNode::makeVerbatim(eKind::Raw, QStringLiteral("ghostName"));
        const SMGuardRender::Rendered r = SMGuardRender::render(data, tid, *node);
        check((r.chips.size() == 1) && r.chips.at(0).invalid, "a bare unknown name is an invalid reference");
        if (r.chips.isEmpty() == false)
        {
            checkEq(r.chips.at(0).name, QStringLiteral("ghostName"), "the invalid chip keeps the typed name");
        }
        delete node;
    }

    // A deliberate verbatim fragment (not a plain identifier) stays a raw escape hatch, no chip.
    {
        SMGuardNode* node = SMGuardNode::makeVerbatim(eKind::Raw, QStringLiteral("a + b"));
        const SMGuardRender::Rendered r = SMGuardRender::render(data, tid, *node);
        check(r.chips.isEmpty(), "a raw C++ fragment produces no invalid chip");
        check(r.text.contains(QStringLiteral("a + b")), "the raw fragment renders verbatim");
        delete node;
    }

    // A raw node whose text happens to name a real symbol is not invalid (it resolves by weight).
    {
        SMGuardNode* node = SMGuardNode::makeVerbatim(eKind::Raw, QStringLiteral("WalkRequested"));
        const SMGuardRender::Rendered r = SMGuardRender::render(data, tid, *node);
        check(r.chips.isEmpty(), "a raw identifier that resolves is left as raw, not flagged invalid");
        delete node;
    }
}

//////////////////////////////////////////////////////////////////////////
// Render round-trip
//////////////////////////////////////////////////////////////////////////

static void testRenderRoundTrip()
{
    std::printf("[U1] render round-trip: render(parse(text)) canonical; parse(render(tree))==tree\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    const QString canonical = QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode");
    SMGuardParser::Result r = SMGuardParser::parse(data, tid, canonical);
    check(r.resolved(), "input parses");

    const QString rendered = SMGuardRender::text(data, tid, *r.tree);
    checkEq(rendered, canonical, "render(parse(text)) is the canonical text");

    SMGuardParser::Result r2 = SMGuardParser::parse(data, tid, rendered);
    check((r2.tree != nullptr) && r.tree->equals(*r2.tree), "parse(render(tree)) == tree");

    delete r.tree;
    delete r2.tree;

    // Spans exist for every owner token.
    SMGuardParser::Result r3 = SMGuardParser::parse(data, tid, canonical);
    SMGuardRender::Rendered rr = SMGuardRender::render(data, tid, *r3.tree);
    check(rr.spans.isEmpty() == false, "render produces owner spans");
    delete r3.tree;
}

//////////////////////////////////////////////////////////////////////////
// Rename by ID (acceptance 17)
//////////////////////////////////////////////////////////////////////////

static void testRenameByID()
{
    std::printf("[U1] rename-by-ID: three renames, zero guard edits, re-render changes (acceptance 17)\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    // A handler condition my_condition(param1, param2), an attribute my_attribute, a param.
    MethodEntry* cond = data.getMethods().createMethod(QStringLiteral("my_condition"), NEMethod::SmCondition);
    cond->setImplement(MethodEntry::eImplement::Handler);
    cond->addParam(QStringLiteral("p1"))->setType(QStringLiteral("uint16"));
    cond->addParam(QStringLiteral("p2"))->setType(QStringLiteral("bool"));
    data.getAttributes().createAttribute(QStringLiteral("my_attribute"));
    data.getMethods().findMethod(QStringLiteral("RequestWalk"))->addParam(QStringLiteral("param"))->setType(QStringLiteral("uint16"));

    SMGuardParser::Result r = SMGuardParser::parse(data, tid, QStringLiteral("my_condition(param, my_attribute)"));
    check(r.resolved(), "my_condition(param, my_attribute) resolves");
    SMGuardNode* tree = r.tree;

    checkEq(SMGuardRender::text(data, tid, *tree), QStringLiteral("my_condition(param, my_attribute)"), "renders bare names");
    checkEq(SMGuardCodegenPreview::expression(data, tid, *tree),
            QStringLiteral("handler().my_condition(param, my_attribute())"), "generated form before rename");

    // Three declaration renames -- the guard tree is NOT touched.
    cond->setName(QStringLiteral("CanPass"));
    data.getMethods().findMethod(QStringLiteral("RequestWalk"))->getElements();
    for (MethodParameter& p : data.getMethods().findMethod(QStringLiteral("RequestWalk"))->getElements())
    {
        if (p.getName() == QStringLiteral("param")) { p.setName(QStringLiteral("count2")); }
    }
    for (AttributeEntry& a : data.getAttributes().getElements())
    {
        if (a.getName() == QStringLiteral("my_attribute")) { a.setName(QStringLiteral("Backlog")); }
    }

    checkEq(SMGuardRender::text(data, tid, *tree), QStringLiteral("CanPass(count2, Backlog)"), "re-renders with new names, no guard edit");
    checkEq(SMGuardCodegenPreview::expression(data, tid, *tree),
            QStringLiteral("handler().CanPass(count2, Backlog())"), "generated form after rename");

    // Where-used data: the guard still references the (renamed) attribute's ID -> a delete
    // would be refused with a where-used hit.
    const uint32_t backlogId = SMGuardSymbols::attributeId(data, QStringLiteral("Backlog"));
    check(referencesId(tree, backlogId), "guard references the attribute ID (delete would be refused)");

    delete tree;
}

//////////////////////////////////////////////////////////////////////////
// Codegen preview (acceptance 20)
//////////////////////////////////////////////////////////////////////////

static void testCodegenPreview()
{
    std::printf("[U1] codegen preview: getter/handler()/m<Name>/<FsmData>:: spellings\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    SMGuardParser::Result r = SMGuardParser::parse(data, tid, QStringLiteral("WalkRequested && HasWaiting(count) && !IsNightMode"));
    checkEq(SMGuardCodegenPreview::expression(data, tid, *r.tree),
            QStringLiteral("WalkRequested() && handler().HasWaiting(count) && !IsNightMode()"), "attributes are getters, handler condition uses handler()");
    delete r.tree;

    // Named-lambda call generates as the std::function member m<Name> (v7 A.1).
    SMGuardParser::Result rl = SMGuardParser::parse(data, tid, QStringLiteral("IsCalmHours(count)"));
    checkEq(SMGuardCodegenPreview::expression(data, tid, *rl.tree), QStringLiteral("mIsCalmHours(count)"), "named lambda -> m<Name>()");
    delete rl.tree;

    // Constant qualifier.
    SMGuardParser::Result rc = SMGuardParser::parse(data, tid, QStringLiteral("count >= MIN_WAITING"));
    checkEq(SMGuardCodegenPreview::expression(data, tid, *rc.tree), QStringLiteral("count >= <FsmData>::MIN_WAITING"), "constant -> <FsmData>::NAME");
    delete rc.tree;

    // Typed receiver / getter forms are accepted and normalized (D6): same tree, same preview.
    SMGuardParser::Result rn = SMGuardParser::parse(data, tid, QStringLiteral("handler().HasWaiting(count) && WalkRequested()"));
    check(rn.resolved(), "typed handler().X and name() getter forms resolve");
    checkEq(SMGuardCodegenPreview::expression(data, tid, *rn.tree),
            QStringLiteral("handler().HasWaiting(count) && WalkRequested()"), "normalized forms generate identically");
    delete rn.tree;

    // ifStatement wraps the whole guard.
    SMGuard g = SMGuardParser::parseToGuard(data, tid, QStringLiteral("WalkRequested"));
    checkEq(SMGuardCodegenPreview::ifStatement(data, tid, g), QStringLiteral("if (WalkRequested())"), "if() wrapper");
}

//////////////////////////////////////////////////////////////////////////
// Drafts + XML round-trip (acceptance 16, 21) and byte-stability
//////////////////////////////////////////////////////////////////////////

static void testXmlRoundTrip()
{
    std::printf("[U1] guard XML round-trip: ok tree, draft losslessness, byte-stability, <Rendered> ignored on load\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    // An ok guard that exercises verbatim nodes (Lit, Lambda, Raw).
    SMGuardParser::Result r = SMGuardParser::parse(data, tid, QStringLiteral("WalkRequested && (count >= 3u || {return count < MIN_WAITING;})"), true);
    SMTransitionEntry* trans = data.findTransitionById(tid);
    trans->getGuard().setTree(r.tree);
    trans->getGuard().setRendered(SMGuardRender::text(data, tid, *trans->getGuard().getTree()));

    const QString outA = tempFile("guard_a.fsml");
    const QString outB = tempFile("guard_b.fsml");
    check(data.writeToFile(outA), "write guard document A");

    StateMachineData reread;
    check(reread.readFromFile(outA), "reread guard document A");
    SMTransitionEntry* rtrans = reread.findTransitionById(tid);
    check((rtrans != nullptr) && rtrans->getGuard().isOk(), "reloaded guard is ok");
    check((rtrans != nullptr) && (rtrans->getGuard().getTree() != nullptr)
          && trans->getGuard().getTree()->equals(*rtrans->getGuard().getTree()), "guard tree survives XML round-trip (IDs + verbatim byte-exact)");

    check(reread.writeToFile(outB), "resave reloaded document");
    check(readBytes(outA) == readBytes(outB), "guard XML is byte-stable across write/read/write");

    // <Rendered> is ignored on load: strip it by hand, reload, tree/preview identical (acceptance 16).
    QByteArray bytes = readBytes(outA);
    const int rs = bytes.indexOf("<Rendered>");
    const int re = bytes.indexOf("</Rendered>");
    check((rs >= 0) && (re > rs), "the <Rendered> cache was written");
    if ((rs >= 0) && (re > rs))
    {
        bytes.remove(rs, (re + 11) - rs);
        const QString stripped = tempFile("guard_norendered.fsml");
        writeBytes(stripped, bytes);

        StateMachineData reload;
        check(reload.readFromFile(stripped), "reload without <Rendered>");
        SMTransitionEntry* rt = reload.findTransitionById(tid);
        check((rt != nullptr) && rt->getGuard().isOk() && (rt->getGuard().getTree() != nullptr)
              && trans->getGuard().getTree()->equals(*rt->getGuard().getTree()), "guard is identical without the <Rendered> cache");
    }

    // Draft losslessness: a STRUCTURALLY unresolved guard saves its raw text and reloads
    // verbatim (D9). A dangling comparison ('count >' with no right side) is a syntax error,
    // so it stays a draft even under D-RAW (which only turns unknown identifiers into raw).
    SMGuard draft = SMGuardParser::parseToGuard(data, tid, QStringLiteral("WalkRequsted && count >"));
    check(draft.isDraft(), "structurally unresolved text becomes a draft");
    trans->getGuard() = draft;
    const QString outD = tempFile("guard_draft.fsml");
    check(data.writeToFile(outD), "write draft guard");

    StateMachineData rd;
    check(rd.readFromFile(outD), "reread draft guard");
    SMTransitionEntry* rdt = rd.findTransitionById(tid);
    check((rdt != nullptr) && rdt->getGuard().isDraft(), "reloaded guard is a draft");
    checkEq((rdt != nullptr) ? rdt->getGuard().getDraftText() : QString(),
            QStringLiteral("WalkRequsted && count >"), "draft text is lossless");
}

//////////////////////////////////////////////////////////////////////////
// Multi-line layout (R18): breaks + indent survive render and XML, codegen is unaffected
//////////////////////////////////////////////////////////////////////////

static void testMultiLineLayout()
{
    std::printf("[U1] multi-line guards: breaks/indent round-trip through render + XML; codegen single-line (R18)\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    // A guard the user typed across three lines: the operator stays at the line end, the operand
    // opens the next line with its own indent. This is exactly the canonical layout form.
    const QString multiline = QStringLiteral("WalkRequested &&\n    HasWaiting(count) &&\n    !IsNightMode");

    SMGuardParser::Result r = SMGuardParser::parse(data, tid, multiline);
    check(r.resolved(), "multi-line guard resolves");

    // Layout render restores the same three lines; the default (single-line) render flattens them,
    // so every inline caller (caches, argument projections) stays byte-stable.
    checkEq(SMGuardRender::text(data, tid, *r.tree, true), multiline, "render(layout) reopens the guard across the same three lines");
    checkEq(SMGuardRender::text(data, tid, *r.tree, false),
            QStringLiteral("WalkRequested && HasWaiting(count) && !IsNightMode"), "render(no layout) is the canonical single line");

    // Codegen never reads the layout attributes -> generated C++ is single-line and unchanged.
    check(SMGuardCodegenPreview::expression(data, tid, *r.tree).contains(QLatin1Char('\n')) == false,
          "generated C++ carries no line breaks");

    // Structural identity ignores layout: parse(render(tree)) == tree still holds.
    SMGuardParser::Result r2 = SMGuardParser::parse(data, tid, SMGuardRender::text(data, tid, *r.tree, true));
    check((r2.tree != nullptr) && r.tree->equals(*r2.tree), "parse(render(tree)) == tree (layout-independent)");

    // XML persistence: the layout attributes are written and read back, so a reload reopens across
    // the same three lines.
    SMTransitionEntry* trans = data.findTransitionById(tid);
    trans->getGuard().setTree(r.tree);

    const QString outML = tempFile("guard_multiline.fsml");
    check(data.writeToFile(outML), "write multi-line guard");

    const QByteArray bytes = readBytes(outML);
    check(bytes.contains("brk=\"1\""), "the brk layout attribute is written");
    check(bytes.contains("indent=\"4\""), "the indent layout attribute is written");

    StateMachineData reread;
    check(reread.readFromFile(outML), "reread multi-line guard");
    SMTransitionEntry* rtrans = reread.findTransitionById(tid);
    check((rtrans != nullptr) && rtrans->getGuard().isOk() && (rtrans->getGuard().getTree() != nullptr), "reloaded guard is ok");
    if ((rtrans != nullptr) && (rtrans->getGuard().getTree() != nullptr))
    {
        checkEq(SMGuardRender::text(reread, tid, *rtrans->getGuard().getTree(), true), multiline,
                "reloaded guard reopens across the same three lines");
        check(trans->getGuard().getTree()->equals(*rtrans->getGuard().getTree()), "reloaded tree is structurally identical");
    }

    delete r2.tree;
}

static void testAdvisoryName()
{
    std::printf("[U1] advisory name (R19): written from the id on save, ignored on load, updates on rename\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    SMGuardParser::Result r = SMGuardParser::parse(data, tid, QStringLiteral("WalkRequested && HasWaiting(count)"));
    check(r.resolved(), "guard resolves");

    SMTransitionEntry* trans = data.findTransitionById(tid);
    trans->getGuard().setTree(r.tree);
    SMGuardNode* tree = trans->getGuard().getTree();

    // The root is the And node; child 0 is the attribute reference, child 1 is the call.
    SMGuardNode* attrNode = tree->childAt(0);
    SMGuardNode* callNode = tree->childAt(1);
    check((attrNode != nullptr) && (callNode != nullptr), "the guard shape is attr && call");

    // Before a refresh the advisory name is empty, so nothing extra is written -- a tree whose
    // names were never resolved stays byte-identical.
    check((callNode != nullptr) && callNode->getCacheName().isEmpty(), "advisory name is empty before the first refresh");

    // The model refreshes the advisory names from the ids just before every save.
    SMGuardRender::refreshNames(data, tid, *tree);
    check((callNode != nullptr) && (callNode->getCacheName() == QStringLiteral("HasWaiting")), "the call node caches its method name");
    check((attrNode != nullptr) && (attrNode->getCacheName() == QStringLiteral("WalkRequested")), "the attr node caches its attribute name");

    const QString out = tempFile("guard_advisory_name.fsml");
    check(data.writeToFile(out), "write guard with refreshed advisory names");
    QByteArray bytes = readBytes(out);
    check(bytes.contains("name=\"WalkRequested\""), "the advisory name is written for the attribute reference");
    check(bytes.contains("name=\"HasWaiting\""), "the advisory name is written for the call");

    // The loader NEVER reads name back: the reloaded tree has empty caches, still binds by id, and
    // is structurally identical (name is not part of equals).
    StateMachineData reread;
    check(reread.readFromFile(out), "reread the guard");
    SMTransitionEntry* rtrans = reread.findTransitionById(tid);
    check((rtrans != nullptr) && (rtrans->getGuard().getTree() != nullptr), "reloaded guard is ok");
    if ((rtrans != nullptr) && (rtrans->getGuard().getTree() != nullptr))
    {
        check(rtrans->getGuard().getTree()->getCacheName().isEmpty(), "the advisory name is NOT read back (write-only)");
        check(tree->equals(*rtrans->getGuard().getTree()), "the reloaded tree is structurally identical (name excluded from equals)");
        checkEq(SMGuardRender::guardText(reread, tid, rtrans->getGuard()),
                QStringLiteral("WalkRequested && HasWaiting(count)"), "the reloaded guard resolves its names by id");
    }

    // A rename re-resolves the advisory name on the next refresh -- it can never go stale.
    for (AttributeEntry& a : data.getAttributes().getElements())
    {
        if (a.getName() == QStringLiteral("WalkRequested")) { a.setName(QStringLiteral("Crossing")); }
    }
    SMGuardRender::refreshNames(data, tid, *tree);
    check(data.writeToFile(out), "rewrite the guard after a rename");
    bytes = readBytes(out);
    check(bytes.contains("name=\"Crossing\""), "the advisory name follows the rename");
    check(bytes.contains("name=\"WalkRequested\"") == false, "the stale advisory name is gone");
}

//////////////////////////////////////////////////////////////////////////
// Undoable commands
//////////////////////////////////////////////////////////////////////////

static void testCommands()
{
    std::printf("[U1] guard commands: set tree, flip combinator, negate, draft, clear (undo/redo)\n");

    StateMachineData data;
    DocModelNotifier notifier;
    QUndoStack stack;
    const uint32_t tid = buildDoc(data);

    SMGuardParser::Result r = SMGuardParser::parse(data, tid, QStringLiteral("WalkRequested && IsNightMode"));
    stack.push(SMGuardCommands::setTree(data, notifier, tid, r.tree, QStringLiteral("set")));

    SMTransitionEntry* t = data.findTransitionById(tid);
    check(t->getGuard().isOk(), "set-tree command installs an ok guard");
    checkEq(SMGuardRender::guardText(data, tid, t->getGuard()), QStringLiteral("WalkRequested && IsNightMode"), "guard text after set");
    check(t->getGuard().getRendered().isEmpty() == false, "the command filled the <Rendered> cache");

    // Flip the root And -> Or.
    stack.push(SMGuardCommands::flipCombinator(data, notifier, tid, QList<int>(), QStringLiteral("flip")));
    checkEq(SMGuardRender::guardText(data, tid, t->getGuard()), QStringLiteral("WalkRequested || IsNightMode"), "combinator flipped to Or");
    stack.undo();
    checkEq(SMGuardRender::guardText(data, tid, t->getGuard()), QStringLiteral("WalkRequested && IsNightMode"), "undo restores And");
    stack.redo();
    checkEq(SMGuardRender::guardText(data, tid, t->getGuard()), QStringLiteral("WalkRequested || IsNightMode"), "redo re-applies Or");

    // Negate child 0.
    stack.push(SMGuardCommands::setNegated(data, notifier, tid, QList<int>{ 0 }, true, QStringLiteral("negate")));
    checkEq(SMGuardRender::guardText(data, tid, t->getGuard()), QStringLiteral("!WalkRequested || IsNightMode"), "child 0 negated");
    stack.undo();
    checkEq(SMGuardRender::guardText(data, tid, t->getGuard()), QStringLiteral("WalkRequested || IsNightMode"), "undo un-negates");

    // Clear -> empty guard.
    stack.push(SMGuardCommands::clearGuard(data, notifier, tid, QStringLiteral("clear")));
    check(t->getGuard().isEmpty(), "clear leaves an empty guard");
    stack.undo();
    check(t->getGuard().isOk(), "undo of clear restores the guard");
}

//////////////////////////////////////////////////////////////////////////
// Data types in a guard (issue #542)
//////////////////////////////////////////////////////////////////////////

static void testScopedValues()
{
    std::printf("[U1] scope-qualified operands: enum values, structure fields, imported types\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    DataTypeEnum* numbers = static_cast<DataTypeEnum*>(data.getDataTypes().addEnum(QStringLiteral("Numbers")));
    numbers->addField(QStringLiteral("Zero"));
    numbers->addField(QStringLiteral("One"));
    numbers->addField(QStringLiteral("Two"));

    DataTypeEnum* colors = static_cast<DataTypeEnum*>(data.getDataTypes().addEnum(QStringLiteral("Colors")));
    colors->addField(QStringLiteral("Red"));

    DataTypeStructure* point = static_cast<DataTypeStructure*>(data.getDataTypes().addStructure(QStringLiteral("Point")));
    point->addField(QStringLiteral("x"));

    data.getDataTypes().addImported(QStringLiteral("NEService"));

    AttributeEntry* number = data.getAttributes().createAttribute(QStringLiteral("number"));
    number->setType(QStringLiteral("Numbers"));

    // The reported case: an attribute compared with a value of its own enumeration.
    SMGuardParser::Result ok = SMGuardParser::parse(data, tid, QStringLiteral("#number == Numbers::Zero"));
    check(ok.resolved(), "'#number == Numbers::Zero' resolves");
    check((ok.tree != nullptr) && (ok.tree->getKind() == eKind::Cmp), "the scoped comparison is a Cmp node");
    if ((ok.tree != nullptr) && (ok.tree->getCount() == 2))
    {
        check(ok.tree->childAt(0)->getKind() == eKind::Attr, "'#number' without a kind binds the attribute");
        check(ok.tree->childAt(1)->getKind() == eKind::Lit, "the enum value is stored as a literal");
        checkEq(ok.tree->childAt(1)->getText(), QStringLiteral("Numbers::Zero"), "the literal keeps the qualified text");
    }
    delete ok.tree;

    // Order and sigil are both free: the validator resolves whichever side is a document symbol.
    SMGuardParser::Result flipped = SMGuardParser::parse(data, tid, QStringLiteral("Numbers::Zero == number"));
    check(flipped.resolved(), "the reversed, sigil-less form resolves the same way");
    delete flipped.tree;

    // Whitespace around the scope operator is C++-legal and must not change the stored value.
    SMGuardParser::Result spaced = SMGuardParser::parse(data, tid, QStringLiteral("number == Numbers :: One"));
    check(spaced.resolved(), "spaces around '::' resolve");
    if ((spaced.tree != nullptr) && (spaced.tree->getCount() == 2))
    {
        checkEq(spaced.tree->childAt(1)->getText(), QStringLiteral("Numbers::One"), "the stored literal is canonical");
    }
    delete spaced.tree;

    // An undeclared enumerator is an error, and the message names what the type does declare.
    SMGuardParser::Result badMember = SMGuardParser::parse(data, tid, QStringLiteral("number == Numbers::Three"));
    check(badMember.hasError(), "an undeclared enumerator is an error");
    check((badMember.diagnostics.isEmpty() == false)
          && badMember.diagnostics.first().message.contains(QStringLiteral("Zero")), "the message lists the declared values");
    delete badMember.tree;

    // An undeclared type is an error too -- it is not silently kept as raw C++.
    SMGuardParser::Result badType = SMGuardParser::parse(data, tid, QStringLiteral("number == Digits::Zero"));
    check(badType.hasError(), "an unknown type is an error");
    delete badType.tree;

    // A structure field resolves the same way as an enumerator.
    SMGuardParser::Result field = SMGuardParser::parse(data, tid, QStringLiteral("count == Point::x"));
    check(field.resolved(), "a declared structure field resolves");
    delete field.tree;

    // An imported type is opaque past its own name: the tail belongs to the foreign header, and
    // because the value's real type lives there too, the comparison is left unjudged.
    SMGuardParser::Result imported = SMGuardParser::parse(data, tid, QStringLiteral("number == NEService::eResult::Ok"));
    check(imported.resolved(), "an imported type accepts any tail");
    check(imported.diagnostics.isEmpty(), "an imported value is never second-guessed");
    delete imported.tree;

    // Two different enumerations cannot meet -- a WARNING, so the guard still commits.
    SMGuardParser::Result crossed = SMGuardParser::parse(data, tid, QStringLiteral("number == Colors::Red"));
    check(crossed.hasError() == false, "a type mismatch does not block the guard");
    check(crossed.resolved(), "a type mismatch still produces a resolved tree");
    check(crossed.diagnostics.isEmpty() == false, "a type mismatch is reported");
    if (crossed.diagnostics.isEmpty() == false)
    {
        check(crossed.diagnostics.first().severity == SMGuardParser::eSeverity::Warning, "the mismatch is a warning");
        check(crossed.diagnostics.first().message.contains(QStringLiteral("Colors")), "the message names both types");
    }
    delete crossed.tree;

    // An enumeration has no order, so an ordering test on one is worth saying out loud.
    SMGuardParser::Result ordered = SMGuardParser::parse(data, tid, QStringLiteral("number < Numbers::One"));
    check(ordered.hasError() == false, "an ordering test on an enumeration still commits");
    check(ordered.diagnostics.isEmpty() == false, "an ordering test on an enumeration warns");
    delete ordered.tree;

    // A number literal carries no type of its own: it must never provoke a false alarm.
    SMGuardParser::Result numeric = SMGuardParser::parse(data, tid, QStringLiteral("count >= 3"));
    check(numeric.resolved() && numeric.diagnostics.isEmpty(), "a bare number is compared without complaint");
    delete numeric.tree;

    // The kind-less sigil resolves like a bare name; a stated kind still has to resolve.
    SMGuardParser::Result kindless = SMGuardParser::parse(data, tid, QStringLiteral("#count >= 3"));
    check(kindless.resolved(), "'#count' resolves to the stimulus parameter");
    if (kindless.tree != nullptr)
    {
        check(kindless.tree->childAt(0)->getKind() == eKind::Param, "the kind-less mention binds the parameter");
    }
    delete kindless.tree;

    SMGuardParser::Result unknownBare = SMGuardParser::parse(data, tid, QStringLiteral("#nosuch"));
    check(unknownBare.hasError(), "a kind-less mention of an unknown name is still an error");
    delete unknownBare.tree;
}

//////////////////////////////////////////////////////////////////////////
// Legacy read-shim
//////////////////////////////////////////////////////////////////////////

static void testLegacyShim()
{
    std::printf("[U1] legacy <ConditionList> loads as a draft guard (read-shim)\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);
    SMTransitionEntry* trans = data.findTransitionById(tid);

    // Build a legacy condition tree directly (the SM-21-02 form).
    SMConditionList& conds = trans->getConditions();
    SMConditionEntry* c0 = conds.addCondition();
    c0->setLhsKind(SMArgumentEntry::eValueSource::Attribute);
    c0->setLhs(QStringLiteral("WalkRequested"));
    SMConditionEntry* c1 = conds.addCondition();
    c1->setLhsKind(SMArgumentEntry::eValueSource::Attribute);
    c1->setLhs(QStringLiteral("IsNightMode"));

    SMGuard shim = SMGuardParser::fromLegacy(conds);
    check(shim.isDraft(), "legacy condition list converts to a draft guard");
    check(shim.getDraftText().isEmpty() == false, "the draft carries the rendered legacy text");
}

//////////////////////////////////////////////////////////////////////////
// Entry point
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char** /*argv*/)
{
    std::printf("==== SM-21 (U1) guard core tests ====\n");

    testParserResolution();
    testConditionWeight();
    testParamVsCall();
    testKindGrammar();
    testCallArgGrammar();
    testRenderChips();
    testInvalidReferenceChips();
    testRenderRoundTrip();
    testRenameByID();
    testCodegenPreview();
    testXmlRoundTrip();
    testMultiLineLayout();
    testAdvisoryName();
    testCommands();
    testScopedValues();
    testLegacyShim();

    std::printf("---- %d checks, %d failure(s) ----\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
