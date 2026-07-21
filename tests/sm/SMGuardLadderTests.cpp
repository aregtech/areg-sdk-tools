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
 *  \file        tests/sm/SMGuardLadderTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-21 (U3) unit tests: the promotion ladder composites, grid<->tree edit
 *               semantics (one undo restores both views), node spans, type ranking,
 *               where-used, and the island text round-trip. Self-contained (no display).
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
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardCodegenPreview.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardLadder.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMGuardWhereUsed.hpp"
#include "lusan/model/sm/SMTypeCompat.hpp"

#include <QString>
#include <QUndoStack>
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

    using eKind = SMGuardNode::eKind;
    using eRank = SMTypeCompat::eRank;

    /**
     * \brief   The running-example document: attributes, a constant, a trigger with a
     *          uint16 `count` payload, a handler condition and a named lambda. Returns the
     *          IDs of TWO transitions on the same trigger (the ladder round-trip needs a
     *          second call site).
     **/
    void buildDoc(StateMachineData& data, uint32_t& trans1, uint32_t& trans2)
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

        SMStateEntry* root = data.getStates().createState(QStringLiteral("Idle"), SMStateEntry::eStateKind::Start);
        trans1 = root->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("RequestWalk"), QString())->getId();
        trans2 = root->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("RequestWalk"), QString())->getId();
    }

    //!< Parses \p text and installs it as \p transitionId's guard through the command path.
    void setGuardText(StateMachineData& data, DocModelNotifier& notifier, QUndoStack& stack, uint32_t transitionId, const QString& text)
    {
        SMGuard guard = SMGuardParser::parseToGuard(data, transitionId, text);
        stack.push(SMGuardCommands::setGuard(data, notifier, transitionId, guard, QStringLiteral("set")));
    }

    QString guardText(const StateMachineData& data, uint32_t transitionId)
    {
        const SMTransitionEntry* transition = data.findTransitionById(transitionId);
        return (transition != nullptr) ? SMGuardRender::guardText(data, transitionId, transition->getGuard()) : QString();
    }

    QString preview(const StateMachineData& data, uint32_t transitionId)
    {
        const SMTransitionEntry* transition = data.findTransitionById(transitionId);
        return (transition != nullptr) ? SMGuardCodegenPreview::ifStatement(data, transitionId, transition->getGuard()) : QString();
    }
}

//////////////////////////////////////////////////////////////////////////
// Tests
//////////////////////////////////////////////////////////////////////////

//!< SMTypeCompat::rank: the grid's per-row status source.
static void testTypeRank()
{
    std::printf("[ RUN  ] typeRank\n");
    check(SMTypeCompat::rank(QStringLiteral("uint16"), QStringLiteral("uint16")) == eRank::Exact, "same type is Exact");
    check(SMTypeCompat::rank(QStringLiteral("uint16"), QStringLiteral("uint32")) == eRank::Converts, "uint16 -> uint32 Converts");
    check(SMTypeCompat::rank(QStringLiteral("uint32"), QStringLiteral("uint16")) == eRank::Narrows, "uint32 -> uint16 Narrows");
    check(SMTypeCompat::rank(QStringLiteral("uint16"), QStringLiteral("double")) == eRank::Converts, "int<=32 -> double Converts");
    check(SMTypeCompat::rank(QStringLiteral("double"), QStringLiteral("uint16")) == eRank::Narrows, "double -> uint16 Narrows");
    check(SMTypeCompat::rank(QStringLiteral("bool"), QStringLiteral("uint16")) == eRank::Mismatch, "bool vs uint16 Mismatch");
    check(SMTypeCompat::rank(QString(), QStringLiteral("uint16")) == eRank::Unknown, "empty type is Unknown");
    check(SMTypeCompat::rank(QStringLiteral("MyEnum"), QStringLiteral("MyEnum")) == eRank::Exact, "declared type by exact name");
    check(SMTypeCompat::rank(QStringLiteral("MyEnum"), QStringLiteral("uint16")) == eRank::Mismatch, "declared vs primitive Mismatch");
}

//!< SMGuardRender::nodeSpans: the lens/pill caret spans come from the render walk.
static void testNodeSpans()
{
    std::printf("[ RUN  ] nodeSpans\n");
    StateMachineData data;
    uint32_t trans1 = 0;
    uint32_t trans2 = 0;
    buildDoc(data, trans1, trans2);

    SMGuardParser::Result result = SMGuardParser::parse(data, trans1
                                  , QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING)"));
    check(result.resolved(), "example B resolves");
    if (result.tree == nullptr)
    {
        return;
    }

    const QString text = SMGuardRender::text(data, trans1, *result.tree);
    const QList<SMGuardRender::NodeSpan> spans = SMGuardRender::nodeSpans(data, trans1, *result.tree);

    bool sawRoot = false;
    bool sawOr = false;
    bool sawCall = false;
    for (const SMGuardRender::NodeSpan& span : spans)
    {
        const QString piece = text.mid(span.start, span.length);
        if (span.path.isEmpty())
        {
            sawRoot = (piece == text);
        }
        else if (span.path == QList<int>{ 1 })
        {
            sawOr = (piece == QStringLiteral("(HasWaiting(count) || count >= MIN_WAITING)"));
        }
        else if (span.path == QList<int>{ 1, 0 })
        {
            sawCall = (piece == QStringLiteral("HasWaiting(count)"));
        }
    }

    check(sawRoot, "root span covers the whole text");
    check(sawOr, "group span includes its parens");
    check(sawCall, "call span covers the call");
    delete result.tree;
}

//!< Grid<->tree semantics (23b, model side): a replace-arg edit changes the rendered
//!< text, and ONE undo restores both the tree and the text.
static void testGridEditOneUndo()
{
    std::printf("[ RUN  ] gridEditOneUndo\n");
    StateMachineData data;
    DocModelNotifier notifier;
    QUndoStack stack;
    uint32_t trans1 = 0;
    uint32_t trans2 = 0;
    buildDoc(data, trans1, trans2);

    setGuardText(data, notifier, stack, trans1, QStringLiteral("HasWaiting(count)"));
    checkEq(guardText(data, trans1), QStringLiteral("HasWaiting(count)"), "initial call text");

    // The grid edit: argument 0 of the root call becomes the literal 5.
    SMGuardParser::Result arg = SMGuardParser::parse(data, trans1, QStringLiteral("5"));
    check(arg.resolved(), "the operand text resolves");
    SMGuardNode* argNode = arg.tree;
    arg.tree = nullptr;

    stack.push(SMGuardCommands::replaceArg(data, notifier, trans1, {}, 0, argNode, QStringLiteral("map")));
    checkEq(guardText(data, trans1), QStringLiteral("HasWaiting(5)"), "grid edit changed the visible text");
    checkEq(preview(data, trans1), QStringLiteral("if (handler().HasWaiting(5))"), "preview follows the tree");

    stack.undo();
    checkEq(guardText(data, trans1), QStringLiteral("HasWaiting(count)"), "ONE undo restored the text");
    checkEq(preview(data, trans1), QStringLiteral("if (handler().HasWaiting(count))"), "and the preview");
}

//!< The island body round-trips parse -> render byte-exact (23a, model side).
static void testIslandRoundTrip()
{
    std::printf("[ RUN  ] islandRoundTrip\n");
    StateMachineData data;
    uint32_t trans1 = 0;
    uint32_t trans2 = 0;
    buildDoc(data, trans1, trans2);

    const QString body = QStringLiteral(" const uint16_t effective = count + 2u;\n return effective >= MIN_WAITING; ");
    const QString text = QStringLiteral("WalkRequested && {") + body + QStringLiteral("}");

    SMGuardParser::Result result = SMGuardParser::parse(data, trans1, text);
    check(result.resolved(), "island guard resolves");
    if (result.tree == nullptr)
    {
        return;
    }

    const SMGuardNode* island = result.tree->childAt(1);
    check((island != nullptr) && (island->getKind() == eKind::Lambda), "child 1 is the island");
    if (island != nullptr)
    {
        checkEq(island->getText(), body, "island body stored byte-exact");
    }

    checkEq(SMGuardRender::text(data, trans1, *result.tree), text, "render reproduces the island byte-exact");
    delete result.tree;
}

//!< The full ladder round-trip: island -> Name it -> called from a second transition ->
//!< Move to handler -> both previews show handler().Name(...); undo the move -> both
//!< guards, the Methods entry and the body restore. Each rung is ONE undo step.
static void testLadderRoundTrip()
{
    std::printf("[ RUN  ] ladderRoundTrip\n");
    StateMachineData data;
    DocModelNotifier notifier;
    QUndoStack stack;
    uint32_t trans1 = 0;
    uint32_t trans2 = 0;
    buildDoc(data, trans1, trans2);

    const QString body = QStringLiteral(" return count + 2 >= MIN_WAITING; ");
    setGuardText(data, notifier, stack, trans1, QStringLiteral("WalkRequested && {") + body + QStringLiteral("}"));
    const int baseline = stack.index();     // the island guard is the ladder's floor

    // ---- Name it... (island -> named lambda + mapped call, ONE undo step) ----
    SMNameIslandCommand* nameCmd = SMGuardLadder::nameIsland(data, notifier, trans1, { 1 }
                                  , QStringLiteral("IsBusy"), SMMethodEntry::eImplement::Embedded, QStringLiteral("name"));
    check(nameCmd != nullptr, "nameIsland command builds");
    if (nameCmd == nullptr)
    {
        return;
    }

    stack.push(nameCmd);
    const uint32_t methodId = nameCmd->methodId();
    check(methodId != 0u, "the declaration got an ID");

    SMMethodEntry* method = data.getMethods().findMethod(QStringLiteral("IsBusy"));
    check((method != nullptr) && method->isLambdaCondition(), "IsBusy declared as kind lambda");
    if (method != nullptr)
    {
        checkEq(method->getBody(), body, "the declaration owns the island body");
        check(method->getElementCount() == 1, "one parameter derived from the body");
        checkEq((method->getElementCount() > 0) ? method->getElements().at(0).getName() : QString()
                , QStringLiteral("count"), "the parameter is the referenced stimulus param");
    }

    checkEq(guardText(data, trans1), QStringLiteral("WalkRequested && IsBusy(count)"), "the island became a mapped call");
    checkEq(preview(data, trans1), QStringLiteral("if (WalkRequested() && mIsBusy(count))"), "lambda preview is the std::function member (D7)");

    // ---- the named lambda is called from a second transition ----
    setGuardText(data, notifier, stack, trans2, QStringLiteral("IsBusy(count)"));
    checkEq(preview(data, trans2), QStringLiteral("if (mIsBusy(count))"), "second call site previews the member");
    check(SMGuardWhereUsed::useCount(data, methodId) == 2, "where-used sees both guards");

    // ---- Move to handler... (kind flip, call sites untouched, ONE undo step) ----
    QUndoCommand* moveCmd = SMGuardLadder::moveToHandler(data, notifier, methodId, QStringLiteral("move"));
    check(moveCmd != nullptr, "moveToHandler command builds");
    if (moveCmd == nullptr)
    {
        return;
    }

    stack.push(moveCmd);
    check((method != nullptr) && method->isHandlerCondition(), "IsBusy is now kind handler");
    check((method != nullptr) && method->getBody().isEmpty(), "Lusan no longer owns the body");
    checkEq(preview(data, trans1), QStringLiteral("if (WalkRequested() && handler().IsBusy(count))"), "guard 1 previews handler().IsBusy");
    checkEq(preview(data, trans2), QStringLiteral("if (handler().IsBusy(count))"), "guard 2 previews handler().IsBusy");

    // ---- undo the move: both guards, the entry and the body restore ----
    stack.undo();
    check((method != nullptr) && method->isLambdaCondition(), "undo restored kind lambda");
    checkEq((method != nullptr) ? method->getBody() : QString(), body, "undo restored the body");
    checkEq(preview(data, trans1), QStringLiteral("if (WalkRequested() && mIsBusy(count))"), "guard 1 preview restored");
    checkEq(preview(data, trans2), QStringLiteral("if (mIsBusy(count))"), "guard 2 preview restored");

    // ---- Adopt body... after a real move (redo the move first) ----
    stack.redo();
    QUndoCommand* adoptCmd = SMGuardLadder::adoptBody(data, notifier, methodId, QStringLiteral("return count > 0;"), QStringLiteral("adopt"));
    check(adoptCmd != nullptr, "adoptBody command builds");
    if (adoptCmd != nullptr)
    {
        stack.push(adoptCmd);
        check((method != nullptr) && method->isLambdaCondition(), "adopt flipped back to lambda");
        checkEq((method != nullptr) ? method->getBody() : QString(), QStringLiteral("return count > 0;"), "adopt set the new body");
        stack.undo();
        check((method != nullptr) && method->isHandlerCondition(), "undo of adopt restored handler kind");
        stack.redo();
    }

    // ---- Inline body here (single-return gate + argument substitution) ----
    // Map guard 2's argument to a literal first, so the substitution is visible.
    SMGuardParser::Result arg = SMGuardParser::parse(data, trans2, QStringLiteral("5"));
    SMGuardNode* argNode = arg.tree;
    arg.tree = nullptr;
    stack.push(SMGuardCommands::replaceArg(data, notifier, trans2, {}, 0, argNode, QStringLiteral("map")));

    QUndoCommand* inlineCmd = SMGuardLadder::inlineBody(data, notifier, trans2, {}, QStringLiteral("inline"));
    check(inlineCmd != nullptr, "inlineBody builds for a single-return body");
    if (inlineCmd != nullptr)
    {
        stack.push(inlineCmd);
        checkEq(guardText(data, trans2), QStringLiteral("{ return 5 > 0; }"), "inline substituted the mapped argument");
        stack.undo();
        checkEq(guardText(data, trans2), QStringLiteral("IsBusy(5)"), "ONE undo restored the call");
    }

    // ---- undo everything back to the island ----
    while (stack.index() > baseline)
    {
        stack.undo();
    }

    checkEq(guardText(data, trans1), QStringLiteral("WalkRequested && {") + body + QStringLiteral("}"), "full undo restored the island");
    check(data.getMethods().findMethod(QStringLiteral("IsBusy")) == nullptr, "full undo removed the declaration");
}

//!< The single-return gate and the where-used walk.
static void testGatesAndWhereUsed()
{
    std::printf("[ RUN  ] gatesAndWhereUsed\n");
    check(SMGuardLadder::isSingleReturnBody(QStringLiteral("return x > 0;")), "plain return passes");
    check(SMGuardLadder::isSingleReturnBody(QStringLiteral("  return (a && b);  ")), "whitespace tolerated");
    check(SMGuardLadder::isSingleReturnBody(QStringLiteral("int y = 1; return y > 0;")) == false, "two statements refused");
    check(SMGuardLadder::isSingleReturnBody(QStringLiteral("returnValue;")) == false, "identifier prefix refused");
    check(SMGuardLadder::isSingleReturnBody(QStringLiteral("if (x) return true;")) == false, "non-return start refused");

    StateMachineData data;
    DocModelNotifier notifier;
    QUndoStack stack;
    uint32_t trans1 = 0;
    uint32_t trans2 = 0;
    buildDoc(data, trans1, trans2);

    const SMMethodEntry* handler = data.getMethods().findMethod(QStringLiteral("HasWaiting"));
    check(handler != nullptr, "HasWaiting exists");
    if (handler == nullptr)
    {
        return;
    }

    check(SMGuardWhereUsed::useCount(data, handler->getId()) == 0, "no uses before any guard");
    setGuardText(data, notifier, stack, trans1, QStringLiteral("HasWaiting(count)"));
    check(SMGuardWhereUsed::useCount(data, handler->getId()) == 1, "one use after guard 1");

    const QList<SMGuardWhereUsed::Use> uses = SMGuardWhereUsed::symbolUses(data, handler->getId());
    check((uses.size() == 1) && (uses.at(0).transitionId == trans1), "the use points at transition 1");
    check(uses.at(0).location.contains(QStringLiteral("Idle")), "the location names the state");
}

//////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char** /*argv*/)
{
    std::printf("SM-21 (U3) guard ladder tests\n");

    testTypeRank();
    testNodeSpans();
    testGridEditOneUndo();
    testIslandRoundTrip();
    testLadderRoundTrip();
    testGatesAndWhereUsed();

    std::printf("Done: %d checks, %d failure%s\n", gChecks, gFailures, (gFailures == 1) ? "" : "s");
    return (gFailures == 0) ? 0 : 1;
}
