#ifndef LUSAN_MODEL_SM_SMGUARDCOMMANDS_HPP
#define LUSAN_MODEL_SM_SMGUARDCOMMANDS_HPP
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
 *  \file        lusan/model/sm/SMGuardCommands.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition guard edit commands (undoable).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/sm/SMCommand.hpp"

#include "lusan/data/sm/SMGuardTree.hpp"

#include <QList>

/**
 * The undoable edits of a transition's guard. Every command resolves its target transition
 * by ID each redo/undo (history never dangles a captured pointer) and emits a single
 * `elementChanged(transitionId, Transition)` notification. Because a guard is a small value,
 * every edit is a whole-guard swap: the command captures the previous guard on first redo and
 * restores it on undo. The subtree factories compute the new tree from the current one and
 * hand it to \ref SMSetGuardCommand, so the editor gets one undo step per gesture with no bespoke
 * inverse to maintain.
 **/

/**
 * \class   SMSetGuardCommand
 * \brief   Replaces a transition's guard with a pre-built one (a resolved tree, a draft, or
 *          empty). The single primitive behind set-tree / set-draft / clear and every subtree
 *          factory below.
 **/
class SMSetGuardCommand : public SMCommand
{
public:
    SMSetGuardCommand(  StateMachineData& data, DocModelNotifier& notifier
                      , uint32_t transitionId, const SMGuard& guard
                      , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void apply(const SMGuard& guard);

private:
    uint32_t    mTransId;   //!< The owning transition ID.
    SMGuard     mNew;       //!< The guard to install.
    SMGuard     mOld;       //!< The captured previous guard.
    bool        mCaptured;  //!< True after the previous guard has been captured.
};

/**
 * \namespace   SMGuardCommands
 * \brief   Factories that build ready-to-push \ref SMSetGuardCommand instances. The
 *          whole-guard ones (setGuard/setTree/setDraft/clear) always succeed; the subtree
 *          ones read the transition's current `ok` tree, apply a deterministic transform to a
 *          clone and wrap the result -- returning nullptr when the guard has no tree or the
 *          target path is invalid. Node paths are child indices from the root (empty path =
 *          the root). Every command populates the guard's `<Rendered>` cache from the tree.
 **/
namespace SMGuardCommands
{
    //!< Installs \p guard verbatim (any state).
    SMSetGuardCommand* setGuard(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const SMGuard& guard, const QString& text);

    //!< Installs an `ok` guard around \p tree (takes ownership of \p tree).
    SMSetGuardCommand* setTree(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, SMGuardNode* tree, const QString& text);

    //!< Installs a `draft` guard with the raw \p draftText (takes ownership of \p lastGood).
    SMSetGuardCommand* setDraft(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QString& draftText, SMGuardNode* lastGood, const QString& text);

    //!< Clears to the empty guard (the transition always fires).
    SMSetGuardCommand* clearGuard(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QString& text);

    //!< Replaces argument \p argIndex of the Call node at \p callPath (takes ownership of \p newArg).
    SMSetGuardCommand* replaceArg(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& callPath, int argIndex, SMGuardNode* newArg, const QString& text);

    /**
     * \brief   Binds the formal parameter \p formalId of the Call node at \p callPath to
     *          \p newArg. The arg is keyed on the formal's id, never its
     *          position: an existing binding for the formal is replaced in place; a new one
     *          is inserted in declared-parameter order so the child list stays render-ordered.
     *          Takes ownership of \p newArg. Returns nullptr (freeing \p newArg) when the
     *          guard has no tree, the path is not a Call, or \p formalId is 0.
     **/
    SMSetGuardCommand* setArgByFormal(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& callPath, uint32_t formalId, SMGuardNode* newArg, const QString& text);

    /**
     * \brief   Unmaps the formal parameter \p formalId of the Call node at \p callPath -- the
     *          slot falls back to a ghost. Removing the last mapped formal keeps the
     *          CALL. Returns nullptr (a no-op, so the undo stack never grows) when
     *          nothing is bound to \p formalId.
     **/
    SMSetGuardCommand* clearArgByFormal(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& callPath, uint32_t formalId, const QString& text);

    //!< Flips the And/Or combinator of the group node at \p path.
    SMSetGuardCommand* flipCombinator(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& path, const QString& text);

    //!< Negates (wraps in Not) or un-negates (unwraps a Not) the node at \p path.
    SMSetGuardCommand* setNegated(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& path, bool negate, const QString& text);

    //!< Inserts \p clause at \p index into the group at \p groupPath (takes ownership of \p clause).
    SMSetGuardCommand* insertClause(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& groupPath, int index, SMGuardNode* clause, const QString& text);

    //!< Removes the child at \p index from the group at \p groupPath (collapses a 1-child group).
    SMSetGuardCommand* removeClause(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& groupPath, int index, const QString& text);

    //!< Swaps children \p index1 and \p index2 of the group at \p groupPath.
    SMSetGuardCommand* reorderClause(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& groupPath, int index1, int index2, const QString& text);

    //!< Wraps the node at \p path in a new \p groupKind group (And/Or) as its single child.
    SMSetGuardCommand* wrapInGroup(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& path, SMGuardNode::eKind groupKind, const QString& text);

    //!< Replaces the sub-tree at \p path (takes ownership of \p newSubtree).
    SMSetGuardCommand* replaceSubtree(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& path, SMGuardNode* newSubtree, const QString& text);
}

#endif  // LUSAN_MODEL_SM_SMGUARDCOMMANDS_HPP
