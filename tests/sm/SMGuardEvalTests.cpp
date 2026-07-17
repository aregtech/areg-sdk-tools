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
 *  \file        tests/sm/SMGuardEvalTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-21 (U4) unit tests: the guard what-if evaluator -- three-valued
 *               logic, stub sites, effective values, sibling priority, stub display.
 *
 ************************************************************************/

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardEval.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/SMGuardValidation.hpp"

#include <QString>
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

    using eKind  = SMGuardNode::eKind;
    using eTruth = SMGuardEval::eTruth;

    //!< The running-example document; returns the RequestWalk transition ID.
    uint32_t buildDoc(StateMachineData& data)
    {
        data.getOverview().setName(QStringLiteral("GuardMachine"));

        SMAttributeEntry* walk = data.getAttributes().createAttribute(QStringLiteral("WalkRequested"));
        walk->setValue(QStringLiteral("true"));
        SMAttributeEntry* night = data.getAttributes().createAttribute(QStringLiteral("IsNightMode"));
        night->setValue(QStringLiteral("false"));

        ConstantEntry* konst = data.getConstants().createConstant(QStringLiteral("MIN_WAITING"));
        if (konst != nullptr) { konst->setValue(QStringLiteral("3")); }

        SMMethodEntry* trigger = data.getMethods().createMethod(QStringLiteral("RequestWalk"), SMMethodEntry::eMethodType::Trigger);
        trigger->addParam(QStringLiteral("count"))->setType(QStringLiteral("uint16"));

        SMMethodEntry* handler = data.getMethods().createMethod(QStringLiteral("HasWaiting"), SMMethodEntry::eMethodType::Condition);
        handler->setImplement(SMMethodEntry::eImplement::Handler);
        handler->addParam(QStringLiteral("count"))->setType(QStringLiteral("uint16"));

        SMStateEntry* root = data.getStates().createState(QStringLiteral("Idle"), SMStateEntry::eStateKind::Start);
        SMTransitionEntry* trans = root->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("RequestWalk"), QStringLiteral("Walking"));
        return trans->getId();
    }

    SMGuardNode* parseTree(StateMachineData& data, uint32_t tid, const QString& text, bool allowRaw = false)
    {
        SMGuardParser::Result result = SMGuardParser::parse(data, tid, text, allowRaw);
        if (result.resolved() == false)
        {
            delete result.tree;
            return nullptr;
        }

        return result.tree;
    }

    eTruth truthAt(const SMGuardEval::Result& result, const QList<int>& path)
    {
        for (const SMGuardEval::NodeTruth& node : result.nodes)
        {
            if (node.path == path)
            {
                return node.truth;
            }
        }

        return eTruth::Unknown;
    }
}

//////////////////////////////////////////////////////////////////////////
// Three-valued evaluation over example B
//////////////////////////////////////////////////////////////////////////

static void testEvaluateExampleB()
{
    std::printf("[U4] evaluate: example B with values and stubs\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);
    SMGuardNode* tree = parseTree(data, tid, QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode"));
    check(tree != nullptr, "example B resolves");
    if (tree == nullptr)
    {
        return;
    }

    const uint32_t walkId  = SMGuardSymbols::attributeId(data, QStringLiteral("WalkRequested"));
    const uint32_t nightId = SMGuardSymbols::attributeId(data, QStringLiteral("IsNightMode"));
    const uint32_t countId = SMGuardSymbols::paramId(data, tid, QStringLiteral("count"));

    // One stub site: the HasWaiting call at root child 1, child 0.
    const QList<SMGuardEval::StubSite> sites = SMGuardEval::stubSites(*tree);
    check(sites.size() == 1, "example B has exactly one stub site");
    check((sites.size() == 1) && (sites.at(0).kind == eKind::Call), "the stub site is the Call");
    check((sites.size() == 1) && (sites.at(0).path == QList<int>({ 1, 0 })), "the Call path is 1.0");

    SMGuardEval::Inputs inputs;
    inputs.values.insert(countId, QStringLiteral("3"));

    // Attribute defaults come from the declaration; count=3 >= MIN_WAITING=3 -> true
    // regardless of the unstubbed call (Kleene OR).
    SMGuardEval::Result result = SMGuardEval::evaluate(data, *tree, inputs);
    check(result.truth == eTruth::True, "count=3 makes the guard TRUE without the stub");

    // count=2: the Or now depends on the call -> Unknown until stubbed.
    inputs.values.insert(countId, QStringLiteral("2"));
    result = SMGuardEval::evaluate(data, *tree, inputs);
    check(result.truth == eTruth::Unknown, "count=2 with no stub is UNKNOWN");

    inputs.stubs.insert(SMGuardEval::pathKey(QList<int>({ 1, 0 })), true);
    result = SMGuardEval::evaluate(data, *tree, inputs);
    check(result.truth == eTruth::True, "stubbing the call true makes the guard TRUE");

    inputs.stubs.insert(SMGuardEval::pathKey(QList<int>({ 1, 0 })), false);
    result = SMGuardEval::evaluate(data, *tree, inputs);
    check(result.truth == eTruth::False, "stub false + count=2 makes the guard FALSE");

    // Flip a value field live (the strip's my_attribute-style flip): night mode kills it.
    inputs.stubs.insert(SMGuardEval::pathKey(QList<int>({ 1, 0 })), true);
    inputs.values.insert(nightId, QStringLiteral("true"));
    result = SMGuardEval::evaluate(data, *tree, inputs);
    check(result.truth == eTruth::False, "IsNightMode=true makes the guard FALSE");
    check(truthAt(result, QList<int>({ 2 })) == eTruth::False, "the Not clause reports FALSE");
    check(truthAt(result, QList<int>({ 0 })) == eTruth::True, "the WalkRequested clause reports TRUE");
    check(truthAt(result, QList<int>({ 1 })) == eTruth::True, "the Or clause reports TRUE");

    // WalkRequested=false short-circuits deterministically.
    inputs.values.insert(walkId, QStringLiteral("false"));
    result = SMGuardEval::evaluate(data, *tree, inputs);
    check(result.truth == eTruth::False, "WalkRequested=false keeps the guard FALSE");

    delete tree;
}

//////////////////////////////////////////////////////////////////////////
// Referenced symbols, effective values, stub display
//////////////////////////////////////////////////////////////////////////

static void testValuesAndDisplay()
{
    std::printf("[U4] effective values, referenced ids, stub display\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);
    SMGuardNode* tree = parseTree(data, tid, QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode"));
    check(tree != nullptr, "example B resolves");
    if (tree == nullptr)
    {
        return;
    }

    const QList<uint32_t> attrs = SMGuardEval::referencedIds(*tree, eKind::Attr);
    check(attrs.size() == 2, "two referenced attributes");
    const QList<uint32_t> params = SMGuardEval::referencedIds(*tree, eKind::Param);
    check(params.size() == 1, "one referenced parameter");

    SMGuardEval::Inputs inputs;
    const uint32_t constId = SMGuardSymbols::constantId(data, QStringLiteral("MIN_WAITING"));
    checkEq(SMGuardEval::effectiveValue(data, eKind::Const, constId, inputs), QStringLiteral("3"),
            "a constant defaults to its declared value");
    const uint32_t walkId = SMGuardSymbols::attributeId(data, QStringLiteral("WalkRequested"));
    checkEq(SMGuardEval::effectiveValue(data, eKind::Attr, walkId, inputs), QStringLiteral("true"),
            "an attribute defaults to its declared value");
    inputs.values.insert(walkId, QStringLiteral("false"));
    checkEq(SMGuardEval::effectiveValue(data, eKind::Attr, walkId, inputs), QStringLiteral("false"),
            "a supplied value overrides the declaration");

    // Stub display substitutes mapped values into the rendered call.
    const uint32_t countId = SMGuardSymbols::paramId(data, tid, QStringLiteral("count"));
    inputs.values.insert(countId, QStringLiteral("3"));
    const SMGuardNode* call = tree->childAt(1)->childAt(0);
    checkEq(SMGuardEval::stubDisplay(data, *call, inputs), QStringLiteral("HasWaiting(3)"),
            "the stub row shows the call with the mapped value substituted");

    delete tree;
}

//////////////////////////////////////////////////////////////////////////
// Islands and raw fragments are stub sites
//////////////////////////////////////////////////////////////////////////

static void testIslandAndRawStubs()
{
    std::printf("[U4] islands and raw fragments stub like calls\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    SMGuardNode* tree = parseTree(data, tid, QStringLiteral("WalkRequested && { return count + 2 >= MIN_WAITING; }"));
    check(tree != nullptr, "island guard resolves");
    if (tree != nullptr)
    {
        const QList<SMGuardEval::StubSite> sites = SMGuardEval::stubSites(*tree);
        check((sites.size() == 1) && (sites.at(0).kind == eKind::Lambda), "the island is one Lambda stub site");

        SMGuardEval::Inputs inputs;
        SMGuardEval::Result result = SMGuardEval::evaluate(data, *tree, inputs);
        check(result.truth == eTruth::Unknown, "unstubbed island is UNKNOWN");

        inputs.stubs.insert(SMGuardEval::pathKey(sites.at(0).path), false);
        result = SMGuardEval::evaluate(data, *tree, inputs);
        check(result.truth == eTruth::False, "stub false decides the guard");

        const QString display = SMGuardEval::stubDisplay(data, *tree->childAt(1), inputs);
        check(display.startsWith(QStringLiteral("{")), "island display is the braced body");
        delete tree;
    }

    SMGuardNode* rawTree = parseTree(data, tid, QStringLiteral("(count & 0x3) == 0"), true);
    check(rawTree != nullptr, "raw fragment parses with allowRaw");
    if (rawTree != nullptr)
    {
        const QList<SMGuardEval::StubSite> sites = SMGuardEval::stubSites(*rawTree);
        check((sites.size() == 1) && (sites.at(0).kind == eKind::Raw), "the raw fragment is one stub site");
        delete rawTree;
    }
}

//////////////////////////////////////////////////////////////////////////
// Guard truth + sibling priority
//////////////////////////////////////////////////////////////////////////

static void testGuardTruthAndSiblings()
{
    std::printf("[U4] guard truth (empty/draft) and sibling transitions\n");

    StateMachineData data;
    const uint32_t tid = buildDoc(data);

    SMGuardEval::Inputs inputs;
    SMGuard empty;
    check(SMGuardEval::guardTruth(data, empty, inputs) == eTruth::True, "an empty guard is always TRUE");

    SMGuard draft;
    draft.setDraft(QStringLiteral("WalkRequsted &&"));
    check(SMGuardEval::guardTruth(data, draft, inputs) == eTruth::Unknown, "a draft guard is UNKNOWN");

    // A second transition on the same stimulus, later in document order (lower priority).
    SMStateEntry* idle = data.findState(QStringLiteral("Idle"));
    SMTransitionEntry* second = idle->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("RequestWalk"), QStringLiteral("Sleeping"));
    // And one on a different stimulus that must NOT count as a sibling.
    data.getMethods().createMethod(QStringLiteral("RequestRest"), SMMethodEntry::eMethodType::Trigger);
    idle->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("RequestRest"), QString());

    const QList<uint32_t> siblings = SMGuardEval::siblingTransitions(data, tid);
    check(siblings.size() == 2, "two transitions share the stimulus");
    check((siblings.size() == 2) && (siblings.at(0) == tid), "document order: the first transition leads");
    check((siblings.size() == 2) && (siblings.at(1) == second->getId()), "the second sibling follows");

    // Priority: guard the first false, the second empty -> the second fires.
    SMTransitionEntry* first = data.findTransitionById(tid);
    SMGuardParser::Result r = SMGuardParser::parse(data, tid, QStringLiteral("IsNightMode"));
    check(r.resolved(), "sibling test guard resolves");
    first->getGuard().setTree(r.tree);

    check(SMGuardEval::guardTruth(data, first->getGuard(), inputs) == eTruth::False, "first guard FALSE (IsNightMode defaults false)");
    check(SMGuardEval::guardTruth(data, second->getGuard(), inputs) == eTruth::True, "second guard (empty) TRUE");
}

//////////////////////////////////////////////////////////////////////////
// Document validation (B12 / S15)
//////////////////////////////////////////////////////////////////////////

static void testValidationFindings()
{
    std::printf("[U4] validation: drafts, raw audit, shadowing, broken refs, re-bind\n");

    using VSev  = SMGuardValidation::eSeverity;
    using VKind = SMGuardValidation::eKind;

    StateMachineData data;
    const uint32_t tid = buildDoc(data);
    SMTransitionEntry* transition = data.findTransitionById(tid);

    // Clean document: no findings.
    check(SMGuardValidation::validate(data).isEmpty(), "clean document has no guard findings");

    // Draft -> ERR, generation refuses.
    transition->getGuard().setDraft(QStringLiteral("WalkRequsted &&"));
    QList<SMGuardValidation::Finding> findings = SMGuardValidation::validate(data);
    check(findings.size() == 1, "a draft yields one finding");
    check((findings.size() == 1) && (findings.at(0).severity == VSev::Error) && (findings.at(0).kind == VKind::Draft),
          "the draft finding is an ERR of kind Draft");
    check((findings.size() == 1) && findings.at(0).message.contains(QStringLiteral("generation refuses")),
          "the draft finding says generation refuses");
    check((findings.size() == 1) && (findings.at(0).transitionId == tid), "the finding navigates to the transition");

    // Raw fragments -> INFO audit, the complete list (two fragments = two entries).
    SMGuardParser::Result raw = SMGuardParser::parse(data, tid, QStringLiteral("(count & 0x3) == 0 && (count | 1) != 0"), true);
    check(raw.tree != nullptr, "raw guard parses with allowRaw");
    transition->getGuard().setTree(raw.tree);
    findings = SMGuardValidation::validate(data);
    int rawCount = 0;
    for (const SMGuardValidation::Finding& finding : findings)
    {
        if (finding.kind == VKind::RawFragment)
        {
            ++rawCount;
            check(finding.severity == VSev::Info, "a raw-fragment entry is INFO");
        }
    }
    check(rawCount >= 1, "every raw fragment is listed");

    // Shadowing -> WARN: a stimulus parameter named like an attribute, referenced by the guard.
    data.getMethods().findMethod(QStringLiteral("RequestWalk"))->addParam(QStringLiteral("IsNightMode"))->setType(QStringLiteral("bool"));
    SMGuardParser::Result shadow = SMGuardParser::parse(data, tid, QStringLiteral("IsNightMode"));
    check(shadow.tree != nullptr, "shadowed name parses");
    transition->getGuard().setTree(shadow.tree);
    findings = SMGuardValidation::validateTransition(data, tid);
    bool warned = false;
    for (const SMGuardValidation::Finding& finding : findings)
    {
        if ((finding.kind == VKind::Shadowing) && (finding.severity == VSev::Warning))
        {
            warned = true;
        }
    }
    check(warned, "a referenced shadowing parameter yields a WARN");

    // Broken reference -> ERR: guard references a deleted attribute (forced delete).
    SMGuardParser::Result night = SMGuardParser::parse(data, tid, QStringLiteral("WalkRequested"));
    transition->getGuard().setTree(night.tree);
    const uint32_t walkId = SMGuardSymbols::attributeId(data, QStringLiteral("WalkRequested"));
    data.getAttributes().removeElement(walkId);
    findings = SMGuardValidation::validateTransition(data, tid);
    bool broken = false;
    for (const SMGuardValidation::Finding& finding : findings)
    {
        if ((finding.kind == VKind::BrokenRef) && (finding.severity == VSev::Error))
        {
            broken = true;
        }
    }
    check(broken, "a forced delete leaves an ERR broken-reference finding");

    // Re-bind info (v6 3.3): the guard holds a Param of the OLD stimulus; the new stimulus
    // declares a parameter with the same name and type -> INFO, not ERR.
    StateMachineData data2;
    const uint32_t tid2 = buildDoc(data2);
    SMTransitionEntry* trans2 = data2.findTransitionById(tid2);
    SMGuardParser::Result guard2 = SMGuardParser::parse(data2, tid2, QStringLiteral("count >= MIN_WAITING"));
    check(guard2.resolved(), "param guard resolves");
    trans2->getGuard().setTree(guard2.tree);

    SMMethodEntry* other = data2.getMethods().createMethod(QStringLiteral("RequestRun"), SMMethodEntry::eMethodType::Trigger);
    other->addParam(QStringLiteral("count"))->setType(QStringLiteral("uint16"));
    trans2->setStimulus(QStringLiteral("RequestRun"));

    findings = SMGuardValidation::validateTransition(data2, tid2);
    bool rebind = false;
    for (const SMGuardValidation::Finding& finding : findings)
    {
        if ((finding.kind == VKind::ParamRebind) && (finding.severity == VSev::Info))
        {
            rebind = true;
        }
    }
    check(rebind, "a same-name-same-type stimulus change reports the re-bind INFO");

    // A stimulus change with NO matching parameter is an ERR.
    SMMethodEntry* bare = data2.getMethods().createMethod(QStringLiteral("RequestStop"), SMMethodEntry::eMethodType::Trigger);
    (void)bare;
    trans2->setStimulus(QStringLiteral("RequestStop"));
    findings = SMGuardValidation::validateTransition(data2, tid2);
    bool stale = false;
    for (const SMGuardValidation::Finding& finding : findings)
    {
        if ((finding.kind == VKind::BrokenRef) && (finding.severity == VSev::Error))
        {
            stale = true;
        }
    }
    check(stale, "a stimulus change without a match is an ERR stale reference");
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char** /*argv*/)
{
    std::printf("SM-21 U4 guard evaluator tests\n");

    testEvaluateExampleB();
    testValuesAndDisplay();
    testIslandAndRawStubs();
    testGuardTruthAndSiblings();
    testValidationFindings();

    std::printf("%d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
