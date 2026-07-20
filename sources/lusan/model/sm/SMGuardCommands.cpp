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
 *  \file        lusan/model/sm/SMGuardCommands.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition guard edit commands (undoable).
 *
 ************************************************************************/

#include "lusan/model/sm/SMGuardCommands.hpp"

#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"

namespace
{
    using eKind = SMGuardNode::eKind;

    //!< The node reached by following the child-index \p path from \p root, or nullptr.
    SMGuardNode* nodeAt(SMGuardNode* root, const QList<int>& path)
    {
        SMGuardNode* node = root;
        for (int index : path)
        {
            if (node == nullptr) { return nullptr; }
            node = node->childAt(index);
        }

        return node;
    }

    //!< Replaces the node addressed by \p path with \p replacement (freeing the old node);
    //!< an empty path replaces the whole tree \p root. Returns false when the path is invalid
    //!< (in which case \p replacement is deleted and \p root is unchanged).
    bool replaceAt(SMGuardNode*& root, const QList<int>& path, SMGuardNode* replacement)
    {
        if (path.isEmpty())
        {
            delete root;
            root = replacement;
            return true;
        }

        QList<int> parentPath = path;
        const int index = parentPath.takeLast();
        SMGuardNode* parent = nodeAt(root, parentPath);
        if ((parent == nullptr) || (index < 0) || (index >= parent->getCount()))
        {
            delete replacement;
            return false;
        }

        delete parent->getChildren().at(index);
        parent->getChildren()[index] = replacement;
        return true;
    }

    //!< Builds an `ok` guard around \p tree with its `<Rendered>` cache filled.
    SMGuard okGuard(const StateMachineData& data, uint32_t transitionId, SMGuardNode* tree)
    {
        SMGuard guard;
        guard.setTree(tree);
        if (tree != nullptr)
        {
            guard.setRendered(SMGuardRender::text(data, transitionId, *tree));
        }

        return guard;
    }

    //!< A deep copy of a transition's current `ok` tree, or nullptr when it has none.
    SMGuardNode* cloneCurrentTree(StateMachineData& data, uint32_t transitionId)
    {
        const SMTransitionEntry* transition = data.findTransitionById(transitionId);
        if (transition == nullptr)
        {
            return nullptr;
        }

        const SMGuard& guard = transition->getGuard();
        return (guard.isOk() && (guard.getTree() != nullptr)) ? guard.getTree()->clone() : nullptr;
    }

    //!< Wraps a transformed tree in a set-guard command; deletes \p tree and returns nullptr
    //!< when the transform failed (\p ok is false).
    SMSetGuardCommand* commandFor(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, SMGuardNode* tree, bool ok, const QString& text)
    {
        if ((ok == false) || (tree == nullptr))
        {
            delete tree;
            return nullptr;
        }

        return new SMSetGuardCommand(data, notifier, transitionId, okGuard(data, transitionId, tree), text);
    }
}

//////////////////////////////////////////////////////////////////////////
// SMSetGuardCommand
//////////////////////////////////////////////////////////////////////////

SMSetGuardCommand::SMSetGuardCommand(  StateMachineData& data, DocModelNotifier& notifier
                                     , uint32_t transitionId, const SMGuard& guard
                                     , const QString& text, QUndoCommand* parent)
    : SMCommand (data, notifier, text, parent)
    , mTransId  (transitionId)
    , mNew      (guard)
    , mOld      ( )
    , mCaptured (false)
{
}

void SMSetGuardCommand::apply(const SMGuard& guard)
{
    SMTransitionEntry* transition = data().findTransitionById(mTransId);
    if (transition == nullptr)
    {
        return;
    }

    transition->getGuard() = guard;
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

void SMSetGuardCommand::redo()
{
    if (mCaptured == false)
    {
        SMTransitionEntry* transition = data().findTransitionById(mTransId);
        if (transition != nullptr)
        {
            mOld = transition->getGuard();
        }
        mCaptured = true;
    }

    apply(mNew);
}

void SMSetGuardCommand::undo()
{
    apply(mOld);
}

//////////////////////////////////////////////////////////////////////////
// SMGuardCommands factories
//////////////////////////////////////////////////////////////////////////

SMSetGuardCommand* SMGuardCommands::setGuard(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const SMGuard& guard, const QString& text)
{
    SMGuard withCache(guard);
    if (withCache.isOk() && (withCache.getTree() != nullptr) && withCache.getRendered().isEmpty())
    {
        withCache.setRendered(SMGuardRender::text(data, transitionId, *withCache.getTree()));
    }

    return new SMSetGuardCommand(data, notifier, transitionId, withCache, text);
}

SMSetGuardCommand* SMGuardCommands::setTree(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, SMGuardNode* tree, const QString& text)
{
    return new SMSetGuardCommand(data, notifier, transitionId, okGuard(data, transitionId, tree), text);
}

SMSetGuardCommand* SMGuardCommands::setDraft(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QString& draftText, SMGuardNode* lastGood, const QString& text)
{
    SMGuard guard;
    guard.setDraft(draftText, lastGood);
    return new SMSetGuardCommand(data, notifier, transitionId, guard, text);
}

SMSetGuardCommand* SMGuardCommands::clearGuard(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QString& text)
{
    return new SMSetGuardCommand(data, notifier, transitionId, SMGuard(), text);
}

SMSetGuardCommand* SMGuardCommands::replaceArg(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& callPath, int argIndex, SMGuardNode* newArg, const QString& text)
{
    SMGuardNode* root = cloneCurrentTree(data, transitionId);
    if (root == nullptr) { delete newArg; return nullptr; }

    SMGuardNode* call = nodeAt(root, callPath);
    bool ok = false;
    if ((call != nullptr) && (call->getKind() == eKind::Call) && (argIndex >= 0) && (argIndex < call->getCount()))
    {
        delete call->getChildren().at(argIndex);
        call->getChildren()[argIndex] = newArg;
        ok = true;
    }
    else
    {
        delete newArg;
    }

    return commandFor(data, notifier, transitionId, root, ok, text);
}

namespace
{
    //!< The index of the Call arg child bound to \p formalId, or -1 when none is.
    int indexOfFormal(const SMGuardNode* call, uint32_t formalId)
    {
        for (int i = 0; i < call->getCount(); ++i)
        {
            if (call->childAt(i)->getArgFormalId() == formalId)
            {
                return i;
            }
        }

        return -1;
    }

    //!< The declared position of \p formalId among \p method's formals, or a large value when
    //!< it is not a formal (a legacy positional arg, id 0, sorts to the end).
    int declaredPos(const SMMethodEntry* method, uint32_t formalId)
    {
        if ((method != nullptr) && (formalId != 0u))
        {
            const QList<MethodParameter>& formals = method->getElements();
            for (int i = 0; i < formals.size(); ++i)
            {
                if (formals.at(i).getId() == formalId)
                {
                    return i;
                }
            }
        }

        return 1000000;
    }

    //!< The child index at which a new arg for \p formalId keeps the list in declared order.
    int insertionIndex(const SMGuardNode* call, const SMMethodEntry* method, uint32_t formalId)
    {
        const int pos = declaredPos(method, formalId);
        int index = 0;
        for (int i = 0; i < call->getCount(); ++i)
        {
            if (declaredPos(method, call->childAt(i)->getArgFormalId()) < pos)
            {
                ++index;
            }
        }

        return index;
    }
}

SMSetGuardCommand* SMGuardCommands::setArgByFormal(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& callPath, uint32_t formalId, SMGuardNode* newArg, const QString& text)
{
    SMGuardNode* root = cloneCurrentTree(data, transitionId);
    if ((root == nullptr) || (formalId == 0u)) { delete newArg; delete root; return nullptr; }

    SMGuardNode* call = nodeAt(root, callPath);
    bool ok = false;
    if ((call != nullptr) && (call->getKind() == eKind::Call) && (newArg != nullptr))
    {
        newArg->setArgFormalId(formalId);
        const int existing = indexOfFormal(call, formalId);
        if (existing >= 0)
        {
            delete call->getChildren().at(existing);
            call->getChildren()[existing] = newArg;
        }
        else
        {
            const SMMethodEntry* method = SMGuardSymbols::method(data, call->getSymbolId());
            call->getChildren().insert(insertionIndex(call, method, formalId), newArg);
        }

        ok = true;
    }
    else
    {
        delete newArg;
    }

    return commandFor(data, notifier, transitionId, root, ok, text);
}

SMSetGuardCommand* SMGuardCommands::clearArgByFormal(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& callPath, uint32_t formalId, const QString& text)
{
    SMGuardNode* root = cloneCurrentTree(data, transitionId);
    if (root == nullptr) { return nullptr; }

    SMGuardNode* call = nodeAt(root, callPath);
    bool ok = false;
    if ((call != nullptr) && (call->getKind() == eKind::Call))
    {
        const int existing = indexOfFormal(call, formalId);
        if (existing >= 0)
        {
            // A1: removing the last mapped formal keeps the CALL; only the child vanishes.
            delete call->getChildren().at(existing);
            call->getChildren().removeAt(existing);
            ok = true;
        }
    }

    // A no-op (nothing was bound) returns nullptr, so a plain focus-out never grows the stack.
    return commandFor(data, notifier, transitionId, root, ok, text);
}

SMSetGuardCommand* SMGuardCommands::flipCombinator(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& path, const QString& text)
{
    SMGuardNode* root = cloneCurrentTree(data, transitionId);
    if (root == nullptr) { return nullptr; }

    SMGuardNode* node = nodeAt(root, path);
    bool ok = false;
    if ((node != nullptr) && node->isGroup())
    {
        node->setKind(node->getKind() == eKind::And ? eKind::Or : eKind::And);
        ok = true;
    }

    return commandFor(data, notifier, transitionId, root, ok, text);
}

SMSetGuardCommand* SMGuardCommands::setNegated(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& path, bool negate, const QString& text)
{
    SMGuardNode* root = cloneCurrentTree(data, transitionId);
    if (root == nullptr) { return nullptr; }

    SMGuardNode* node = nodeAt(root, path);
    bool ok = false;
    if (node != nullptr)
    {
        const bool isNot = (node->getKind() == eKind::Not);
        if (negate && (isNot == false))
        {
            ok = replaceAt(root, path, SMGuardNode::makeNot(node->clone()));
        }
        else if ((negate == false) && isNot && (node->getCount() == 1))
        {
            SMGuardNode* inner = node->childAt(0)->clone();
            ok = replaceAt(root, path, inner);
        }
        else
        {
            ok = true;  // already in the requested state
        }
    }

    return commandFor(data, notifier, transitionId, root, ok, text);
}

SMSetGuardCommand* SMGuardCommands::insertClause(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& groupPath, int index, SMGuardNode* clause, const QString& text)
{
    SMGuardNode* root = cloneCurrentTree(data, transitionId);
    if (root == nullptr) { delete clause; return nullptr; }

    SMGuardNode* group = nodeAt(root, groupPath);
    bool ok = false;
    if ((group != nullptr) && group->isGroup())
    {
        const int at = qBound(0, index, group->getCount());
        group->getChildren().insert(at, clause);
        ok = true;
    }
    else
    {
        delete clause;
    }

    return commandFor(data, notifier, transitionId, root, ok, text);
}

SMSetGuardCommand* SMGuardCommands::removeClause(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& groupPath, int index, const QString& text)
{
    SMGuardNode* root = cloneCurrentTree(data, transitionId);
    if (root == nullptr) { return nullptr; }

    SMGuardNode* group = nodeAt(root, groupPath);
    bool ok = false;
    if ((group != nullptr) && group->isGroup() && (index >= 0) && (index < group->getCount()))
    {
        delete group->getChildren().at(index);
        group->getChildren().removeAt(index);
        ok = true;

        // An And/Or needs 2+ operands: a group left with one child collapses to that child.
        if (group->getCount() == 1)
        {
            SMGuardNode* only = group->childAt(0)->clone();
            ok = replaceAt(root, groupPath, only);
        }
    }

    return commandFor(data, notifier, transitionId, root, ok, text);
}

SMSetGuardCommand* SMGuardCommands::reorderClause(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& groupPath, int index1, int index2, const QString& text)
{
    SMGuardNode* root = cloneCurrentTree(data, transitionId);
    if (root == nullptr) { return nullptr; }

    SMGuardNode* group = nodeAt(root, groupPath);
    bool ok = false;
    if ((group != nullptr) && group->isGroup()
        && (index1 >= 0) && (index1 < group->getCount())
        && (index2 >= 0) && (index2 < group->getCount()))
    {
        group->getChildren().swapItemsAt(index1, index2);
        ok = true;
    }

    return commandFor(data, notifier, transitionId, root, ok, text);
}

SMSetGuardCommand* SMGuardCommands::wrapInGroup(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& path, SMGuardNode::eKind groupKind, const QString& text)
{
    SMGuardNode* root = cloneCurrentTree(data, transitionId);
    if (root == nullptr) { return nullptr; }

    SMGuardNode* node = nodeAt(root, path);
    bool ok = false;
    if ((node != nullptr) && ((groupKind == eKind::And) || (groupKind == eKind::Or)))
    {
        QList<SMGuardNode*> kids;
        kids.append(node->clone());
        ok = replaceAt(root, path, SMGuardNode::makeGroup(groupKind, kids));
    }

    return commandFor(data, notifier, transitionId, root, ok, text);
}

SMSetGuardCommand* SMGuardCommands::replaceSubtree(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& path, SMGuardNode* newSubtree, const QString& text)
{
    SMGuardNode* root = cloneCurrentTree(data, transitionId);
    if (root == nullptr) { delete newSubtree; return nullptr; }

    const bool ok = replaceAt(root, path, newSubtree);
    return commandFor(data, notifier, transitionId, root, ok, text);
}
