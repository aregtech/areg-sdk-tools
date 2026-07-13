#ifndef LUSAN_MODEL_SM_SMCONDITIONCOMMANDS_HPP
#define LUSAN_MODEL_SM_SMCONDITIONCOMMANDS_HPP
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
 *  \file        lusan/model/sm/SMConditionCommands.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition condition-tree edit commands.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/sm/SMCommand.hpp"

#include "lusan/data/sm/SMCondition.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMConditionGroup;

/**
 * The undoable edits of a transition's condition tree. Every command resolves its target
 * transition by ID, then the target node by ID within that transition's condition tree, so
 * history navigation never dangles a captured pointer (the tree may be rebuilt between
 * steps). All emit a single `elementChanged(transitionId, Transition)` notification: the
 * condition builder rebuilds from the model and the canvas edge re-reads its guard summary.
 * A node ID is stable because the ElementBase counter only grows and re-inserts reuse it.
 **/

/**
 * \class   SMAddConditionCommand
 * \brief   Appends a new empty leaf row (or empty sub-group) to a group of the tree. The
 *          created node's ID is allocated by the first redo and reused on later redos; read
 *          it back with getNodeId() after the push (to select/focus the new row).
 **/
class SMAddConditionCommand : public SMCommand
{
public:
    SMAddConditionCommand(  StateMachineData& data, DocModelNotifier& notifier
                          , uint32_t transitionId, uint32_t parentGroupId, bool asGroup
                          , const QString& text, QUndoCommand* parent = nullptr);
    ~SMAddConditionCommand() override;

    void redo() override;
    void undo() override;

    inline uint32_t getNodeId() const;

private:
    SMConditionGroup* parentGroup() const;

private:
    uint32_t            mTransId;       //!< The owning transition ID.
    uint32_t            mParentId;      //!< The parent group ID (the root group has its own ID).
    bool                mAsGroup;       //!< True to add a sub-group, false for a leaf row.
    uint32_t            mNodeId;        //!< The created node ID (allocated on first redo).
    int                 mIndex;         //!< The recorded child index for re-insertion.
    SMConditionNode*    mNode;          //!< The node while detached (undo state); nullptr in the tree.
    bool                mOwned;         //!< True while this command owns the detached node.
};

/**
 * \class   SMRemoveConditionCommand
 * \brief   Removes a node (leaf or whole sub-group) from its parent group, capturing the
 *          detached sub-tree for undo and re-inserting it at its original position.
 **/
class SMRemoveConditionCommand : public SMCommand
{
public:
    SMRemoveConditionCommand(  StateMachineData& data, DocModelNotifier& notifier
                             , uint32_t transitionId, uint32_t nodeId
                             , const QString& text, QUndoCommand* parent = nullptr);
    ~SMRemoveConditionCommand() override;

    void redo() override;
    void undo() override;

private:
    SMConditionGroup* parentGroup() const;

private:
    uint32_t            mTransId;
    uint32_t            mNodeId;
    uint32_t            mParentId;      //!< Captured on first redo.
    int                 mIndex;         //!< Captured on first redo.
    SMConditionNode*    mNode;          //!< The node while detached (redo state).
    bool                mOwned;
    bool                mCaptured;
};

/**
 * \class   SMSetConditionLeafCommand
 * \brief   Replaces a leaf row's whole content (operand kinds/refs, operator, negate,
 *          arguments, verbatim body) with a pre-built value, keeping the leaf's identity.
 *          Both the unified picker and the typed-token path funnel through this one command,
 *          so click and type produce the same model.
 **/
class SMSetConditionLeafCommand : public SMCommand
{
public:
    SMSetConditionLeafCommand(  StateMachineData& data, DocModelNotifier& notifier
                              , uint32_t transitionId, uint32_t leafId, const SMConditionEntry& content
                              , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    uint32_t            mTransId;
    uint32_t            mLeafId;
    SMConditionEntry    mNew;           //!< The new content (value copy; ID/parent ignored on apply).
    SMConditionEntry    mOld;           //!< The captured previous content.
    bool                mCaptured;
};

/**
 * \class   SMSetGroupCombineCommand
 * \brief   Sets a group's combinator (And/Or). One combinator per group is a UI rule; the
 *          data model simply stores the value.
 **/
class SMSetGroupCombineCommand : public SMCommand
{
public:
    SMSetGroupCombineCommand(  StateMachineData& data, DocModelNotifier& notifier
                             , uint32_t transitionId, uint32_t groupId, SMConditionGroup::eCombine combine
                             , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    uint32_t                    mTransId;
    uint32_t                    mGroupId;
    SMConditionGroup::eCombine  mNew;
    SMConditionGroup::eCombine  mOld;
    bool                        mCaptured;
};

/**
 * \class   SMSetGroupNegateCommand
 * \brief   Toggles a group's `Negate` flag (wrap the group result in `!( ... )`).
 **/
class SMSetGroupNegateCommand : public SMCommand
{
public:
    SMSetGroupNegateCommand(  StateMachineData& data, DocModelNotifier& notifier
                            , uint32_t transitionId, uint32_t groupId, bool negate
                            , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    uint32_t    mTransId;
    uint32_t    mGroupId;
    bool        mNew;
    bool        mOld;
    bool        mCaptured;
};

/**
 * \class   SMReorderConditionCommand
 * \brief   Swaps two direct children of a group (document = evaluation order). Each swap is
 *          its own inverse, so redo and undo are the same operation.
 **/
class SMReorderConditionCommand : public SMCommand
{
public:
    SMReorderConditionCommand(  StateMachineData& data, DocModelNotifier& notifier
                              , uint32_t transitionId, uint32_t groupId, int index1, int index2
                              , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void apply();

private:
    uint32_t    mTransId;
    uint32_t    mGroupId;
    int         mIndex1;
    int         mIndex2;
};

/**
 * \class   SMPromoteLambdaCommand
 * \brief   Promotes a one-off `lambda::` row to a reusable named Embedded condition method:
 *          adds the method (carrying the lambda body verbatim) to the Methods registry and
 *          replaces the row with a `cond::<name>` reference -- one undo step.
 **/
class SMPromoteLambdaCommand : public SMCompositeCommand
{
public:
    SMPromoteLambdaCommand(  StateMachineData& data, DocModelNotifier& notifier
                           , uint32_t transitionId, uint32_t leafId, const QString& methodName
                           , const QString& text, QUndoCommand* parent = nullptr);
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t SMAddConditionCommand::getNodeId() const
{
    return mNodeId;
}

#endif  // LUSAN_MODEL_SM_SMCONDITIONCOMMANDS_HPP
