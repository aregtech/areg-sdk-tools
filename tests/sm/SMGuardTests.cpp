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
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
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

        SMMethodEntry* trigger = data.getMethods().createMethod(QStringLiteral("RequestWalk"), SMMethodEntry::eMethodType::Trigger);
        MethodParameter* tp = trigger->addParam(QStringLiteral("count"));
        tp->setType(QStringLiteral("uint16"));

        SMMethodEntry* handler = data.getMethods().createMethod(QStringLiteral("HasWaiting"), SMMethodEntry::eMethodType::Condition);
        handler->setImplement(SMMethodEntry::eImplement::Handler);
        handler->addParam(QStringLiteral("count"))->setType(QStringLiteral("uint16"));

        SMMethodEntry* lambda = data.getMethods().createMethod(QStringLiteral("IsCalmHours"), SMMethodEntry::eMethodType::Condition);
        lambda->setImplement(SMMethodEntry::eImplement::Embedded);
        lambda->addParam(QStringLiteral("count"))->setType(QStringLiteral("uint16"));
        lambda->setBody(QStringLiteral("return count < MIN_WAITING;"));

        SMStateEntry* root = data.getStates().createState(QStringLiteral("Root"), SMStateEntry::eStateKind::Start);
        SMTransitionEntry* trans = root->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("RequestWalk"), QString());
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

    // Unknown name -> error, not a raw node (D4).
    SMGuardParser::Result bad = SMGuardParser::parse(data, tid, QStringLiteral("WalkRequsted && count > 2"));
    check(bad.hasError(), "typo'd attribute is an error");
    check(bad.resolved() == false, "typo does not resolve");
    delete bad.tree;

    // Explicit raw only via the flag (the "Keep as raw C++" quick-fix).
    SMGuardParser::Result rawOff = SMGuardParser::parse(data, tid, QStringLiteral("count $ 3"), false);
    check(rawOff.hasError(), "unparseable input is an error without allowRaw");
    delete rawOff.tree;

    SMGuardParser::Result rawOn = SMGuardParser::parse(data, tid, QStringLiteral("count $ 3"), true);
    check((rawOn.tree != nullptr) && (rawOn.tree->getKind() == eKind::Raw), "allowRaw wraps the fragment as a Raw node");
    check(rawOn.hasError() == false, "an accepted raw fragment carries no error");
    delete rawOn.tree;
}

static void testParamVsCall()
{
    std::printf("[U1] param vs param() disambiguation + shadowing warning (acceptance 18)\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    // A named lambda 'param' coexists with the stimulus parameter 'param' (call form vs bare).
    SMMethodEntry* lambda = data.getMethods().createMethod(QStringLiteral("param"), SMMethodEntry::eMethodType::Condition);
    lambda->setImplement(SMMethodEntry::eImplement::Embedded);
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

    // Shadowing: an attribute and a stimulus parameter share a name -> parameter wins + warning.
    data.getAttributes().createAttribute(QStringLiteral("shadow"));
    data.getMethods().findMethod(QStringLiteral("RequestWalk"))->addParam(QStringLiteral("shadow"))->setType(QStringLiteral("bool"));
    SMGuardParser::Result s = SMGuardParser::parse(data, tid, QStringLiteral("shadow"));
    bool warned = false;
    for (const SMGuardParser::Diagnostic& d : s.diagnostics)
    {
        if (d.severity == SMGuardParser::eSeverity::Warning) { warned = true; }
    }
    check((s.tree != nullptr) && (s.tree->getKind() == eKind::Param), "shadowed bare name binds the parameter");
    check(warned, "shadowing raises a warning diagnostic");
    delete s.tree;
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
    SMMethodEntry* cond = data.getMethods().createMethod(QStringLiteral("my_condition"), SMMethodEntry::eMethodType::Condition);
    cond->setImplement(SMMethodEntry::eImplement::Handler);
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
    for (SMAttributeEntry& a : data.getAttributes().getElements())
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

    // Draft losslessness: an unresolved guard saves its raw text and reloads verbatim (D9).
    SMGuard draft = SMGuardParser::parseToGuard(data, tid, QStringLiteral("WalkRequsted && count > 2"));
    check(draft.isDraft(), "unresolved text becomes a draft");
    trans->getGuard() = draft;
    const QString outD = tempFile("guard_draft.fsml");
    check(data.writeToFile(outD), "write draft guard");

    StateMachineData rd;
    check(rd.readFromFile(outD), "reread draft guard");
    SMTransitionEntry* rdt = rd.findTransitionById(tid);
    check((rdt != nullptr) && rdt->getGuard().isDraft(), "reloaded guard is a draft");
    checkEq((rdt != nullptr) ? rdt->getGuard().getDraftText() : QString(),
            QStringLiteral("WalkRequsted && count > 2"), "draft text is lossless");
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
    testParamVsCall();
    testRenderRoundTrip();
    testRenameByID();
    testCodegenPreview();
    testXmlRoundTrip();
    testCommands();
    testLegacyShim();

    std::printf("---- %d checks, %d failure(s) ----\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
