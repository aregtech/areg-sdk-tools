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
 *  \file        tests/sm/SMConditionCommandTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-21-04 unit tests: undoable condition-tree edit commands (add/remove/
 *               set-leaf/combine/negate/reorder/promote). Self-contained (no framework).
 *
 ************************************************************************/

#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMConditionCommands.hpp"
#include "lusan/model/sm/SMConditionText.hpp"
#include "lusan/model/sm/SMConditionToken.hpp"

#include <QUndoStack>
#include <QString>
#include <cstdio>

namespace
{
    int gChecks = 0;
    int gFailures = 0;

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

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }

    //!< Builds a document with one state owning one (external) transition; returns its ID.
    uint32_t makeTransition(StateMachineData& data)
    {
        SMStateEntry* state = data.getStates().createState(QStringLiteral("S1"), SMStateEntry::eStateKind::Normal);
        SMTransitionEntry* transition = state->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("go"), QString());
        return transition->getId();
    }

    QString preview(StateMachineData& data, uint32_t tid)
    {
        SMTransitionEntry* transition = data.findTransitionById(tid);
        return SMConditionText::summary(transition->getConditions());
    }

    SMConditionEntry parsed(const QString& token)
    {
        SMConditionEntry leaf;
        QString error;
        SMConditionToken::parseLeaf(token, leaf, error);
        return leaf;
    }
}

//////////////////////////////////////////////////////////////////////////
// AC4: add + set-leaf round-trip through undo/redo
//////////////////////////////////////////////////////////////////////////

static void testAddSetUndo()
{
    std::printf("[SM-21-04] add rule + set operand, undo/redo\n");

    StateMachineData data;
    DocModelNotifier notifier;
    QUndoStack stack;
    const uint32_t tid = makeTransition(data);
    SMConditionList& root = data.findTransitionById(tid)->getConditions();
    const uint32_t rootId = root.getId();

    SMAddConditionCommand* add = new SMAddConditionCommand(data, notifier, tid, rootId, false, QStringLiteral("add"));
    stack.push(add);
    const uint32_t leafId = add->getNodeId();
    check(leafId != 0u, "added leaf has an ID");

    stack.push(new SMSetConditionLeafCommand(data, notifier, tid, leafId, parsed(QStringLiteral("attr::IsNightMode")), QStringLiteral("set")));
    checkEq(preview(data, tid), QStringLiteral("IsNightMode"), "preview after set");

    stack.undo();
    checkEq(preview(data, tid), QString(), "preview after undo of set (empty leaf)");
    stack.redo();
    checkEq(preview(data, tid), QStringLiteral("IsNightMode"), "preview after redo of set");

    stack.undo();
    stack.undo();
    checkEq(preview(data, tid), QString(), "preview after undo of add (empty tree)");
    stack.redo();
    stack.redo();
    checkEq(preview(data, tid), QStringLiteral("IsNightMode"), "preview after redo of add + set");
}

//////////////////////////////////////////////////////////////////////////
// AC3/AC4: nested a && (b || c), reorder, group negate
//////////////////////////////////////////////////////////////////////////

static void testNestedReorderNegate()
{
    std::printf("[SM-21-04] nested group build, reorder, group negate\n");

    StateMachineData data;
    DocModelNotifier notifier;
    QUndoStack stack;
    const uint32_t tid = makeTransition(data);
    SMConditionList& root = data.findTransitionById(tid)->getConditions();
    const uint32_t rootId = root.getId();

    SMAddConditionCommand* a = new SMAddConditionCommand(data, notifier, tid, rootId, false, QStringLiteral("a"));
    stack.push(a);
    stack.push(new SMSetConditionLeafCommand(data, notifier, tid, a->getNodeId(), parsed(QStringLiteral("attr::A")), QStringLiteral("setA")));

    SMAddConditionCommand* g = new SMAddConditionCommand(data, notifier, tid, rootId, true, QStringLiteral("grp"));
    stack.push(g);
    const uint32_t groupId = g->getNodeId();
    stack.push(new SMSetGroupCombineCommand(data, notifier, tid, groupId, SMConditionGroup::eCombine::Or, QStringLiteral("or")));

    SMAddConditionCommand* b = new SMAddConditionCommand(data, notifier, tid, groupId, false, QStringLiteral("b"));
    stack.push(b);
    stack.push(new SMSetConditionLeafCommand(data, notifier, tid, b->getNodeId(), parsed(QStringLiteral("attr::B")), QStringLiteral("setB")));
    SMAddConditionCommand* c = new SMAddConditionCommand(data, notifier, tid, groupId, false, QStringLiteral("c"));
    stack.push(c);
    stack.push(new SMSetConditionLeafCommand(data, notifier, tid, c->getNodeId(), parsed(QStringLiteral("attr::C")), QStringLiteral("setC")));

    checkEq(preview(data, tid), QStringLiteral("A && (B || C)"), "nested preview");

    // Swap the root's two children.
    stack.push(new SMReorderConditionCommand(data, notifier, tid, rootId, 0, 1, QStringLiteral("swap")));
    checkEq(preview(data, tid), QStringLiteral("(B || C) && A"), "preview after reorder");
    stack.undo();
    checkEq(preview(data, tid), QStringLiteral("A && (B || C)"), "preview after undo reorder");

    // Negate the sub-group.
    stack.push(new SMSetGroupNegateCommand(data, notifier, tid, groupId, true, QStringLiteral("neg")));
    checkEq(preview(data, tid), QStringLiteral("A && !(B || C)"), "preview after group negate");
    stack.undo();
    checkEq(preview(data, tid), QStringLiteral("A && (B || C)"), "preview after undo negate");

    // Remove a leaf from the group and undo.
    stack.push(new SMRemoveConditionCommand(data, notifier, tid, c->getNodeId(), QStringLiteral("rm")));
    checkEq(preview(data, tid), QStringLiteral("A && (B)"), "preview after remove leaf");
    stack.undo();
    checkEq(preview(data, tid), QStringLiteral("A && (B || C)"), "preview after undo remove");
}

//////////////////////////////////////////////////////////////////////////
// AC3: promote lambda to a named condition method (undoable)
//////////////////////////////////////////////////////////////////////////

static void testPromoteLambda()
{
    std::printf("[SM-21-04] promote lambda to method, undo/redo\n");

    StateMachineData data;
    DocModelNotifier notifier;
    QUndoStack stack;
    const uint32_t tid = makeTransition(data);
    SMConditionList& root = data.findTransitionById(tid)->getConditions();
    const uint32_t rootId = root.getId();

    SMAddConditionCommand* a = new SMAddConditionCommand(data, notifier, tid, rootId, false, QStringLiteral("a"));
    stack.push(a);
    const uint32_t leafId = a->getNodeId();

    SMConditionEntry lambda;
    lambda.setLhsKind(SMArgumentEntry::eValueSource::Lambda);
    lambda.setBody(QStringLiteral("return mReady;"));
    stack.push(new SMSetConditionLeafCommand(data, notifier, tid, leafId, lambda, QStringLiteral("lambda")));
    checkEq(preview(data, tid), QStringLiteral("{...}"), "preview shows a lambda placeholder");

    stack.push(new SMPromoteLambdaCommand(data, notifier, tid, leafId, QStringLiteral("IsReady"), QStringLiteral("promote")));
    checkEq(preview(data, tid), QStringLiteral("IsReady()"), "preview after promote (cond call)");
    check(data.getMethods().findMethod(QStringLiteral("IsReady")) != nullptr, "promoted method exists");

    stack.undo();
    checkEq(preview(data, tid), QStringLiteral("{...}"), "preview after undo promote (lambda back)");
    check(data.getMethods().findMethod(QStringLiteral("IsReady")) == nullptr, "promoted method removed on undo");

    stack.redo();
    check(data.getMethods().findMethod(QStringLiteral("IsReady")) != nullptr, "promoted method restored on redo");
}

int main(int /*argc*/, char** /*argv*/)
{
    std::printf("SM-21-04 condition command tests\n");
    testAddSetUndo();
    testNestedReorderNegate();
    testPromoteLambda();

    std::printf("\n%d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
