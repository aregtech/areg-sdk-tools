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
 *  \file        lusan/model/sm/SMConditionCommands.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition condition-tree edit commands.
 *
 ************************************************************************/

#include "lusan/model/sm/SMConditionCommands.hpp"

#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"

namespace
{
    //!< Resolves a transition's root condition group, or nullptr when the transition is gone.
    SMConditionList* rootConditions(StateMachineData& data, uint32_t transitionId)
    {
        SMTransitionEntry* transition = data.findTransitionById(transitionId);
        return (transition != nullptr) ? &transition->getConditions() : nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////
// SMAddConditionCommand
//////////////////////////////////////////////////////////////////////////

SMAddConditionCommand::SMAddConditionCommand(  StateMachineData& data, DocModelNotifier& notifier
                                             , uint32_t transitionId, uint32_t parentGroupId, bool asGroup
                                             , const QString& text, QUndoCommand* parent)
    : SMCommand (data, notifier, text, parent)
    , mTransId  (transitionId)
    , mParentId (parentGroupId)
    , mAsGroup  (asGroup)
    , mNodeId   (0u)
    , mIndex    (-1)
    , mNode     (nullptr)
    , mOwned    (false)
{
}

SMAddConditionCommand::~SMAddConditionCommand()
{
    if (mOwned)
    {
        delete mNode;
    }
}

SMConditionGroup* SMAddConditionCommand::parentGroup() const
{
    SMConditionList* root = rootConditions(data(), mTransId);
    return (root != nullptr) ? root->findGroup(mParentId) : nullptr;
}

void SMAddConditionCommand::redo()
{
    SMConditionGroup* parent = parentGroup();
    if (parent == nullptr)
    {
        return;
    }

    if (mNodeId == 0u)
    {
        SMConditionNode* node = mAsGroup ? static_cast<SMConditionNode*>(parent->addGroup())
                                         : static_cast<SMConditionNode*>(parent->addCondition());
        mNodeId = node->getId();
        mIndex  = parent->indexOfChild(node);
    }
    else
    {
        parent->insertChild(mIndex, mNode);
        mNode  = nullptr;
        mOwned = false;
    }

    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

void SMAddConditionCommand::undo()
{
    SMConditionGroup* parent = parentGroup();
    if (parent == nullptr)
    {
        return;
    }

    mNode  = parent->detachChild(parent->findNode(mNodeId));
    mOwned = (mNode != nullptr);
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

//////////////////////////////////////////////////////////////////////////
// SMRemoveConditionCommand
//////////////////////////////////////////////////////////////////////////

SMRemoveConditionCommand::SMRemoveConditionCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                   , uint32_t transitionId, uint32_t nodeId
                                                   , const QString& text, QUndoCommand* parent)
    : SMCommand (data, notifier, text, parent)
    , mTransId  (transitionId)
    , mNodeId   (nodeId)
    , mParentId (0u)
    , mIndex    (-1)
    , mNode     (nullptr)
    , mOwned    (false)
    , mCaptured (false)
{
}

SMRemoveConditionCommand::~SMRemoveConditionCommand()
{
    if (mOwned)
    {
        delete mNode;
    }
}

SMConditionGroup* SMRemoveConditionCommand::parentGroup() const
{
    SMConditionList* root = rootConditions(data(), mTransId);
    return (root != nullptr) ? root->findGroup(mParentId) : nullptr;
}

void SMRemoveConditionCommand::redo()
{
    SMConditionList* root = rootConditions(data(), mTransId);
    if (root == nullptr)
    {
        return;
    }

    SMConditionNode* node = root->findNode(mNodeId);
    if (node == nullptr)
    {
        return;
    }

    SMConditionGroup* parent = static_cast<SMConditionGroup*>(node->getParent());
    if (parent == nullptr)
    {
        return;
    }

    if (mCaptured == false)
    {
        mParentId = parent->getId();
        mIndex    = parent->indexOfChild(node);
        mCaptured = true;
    }

    mNode  = parent->detachChild(node);
    mOwned = (mNode != nullptr);
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

void SMRemoveConditionCommand::undo()
{
    SMConditionGroup* parent = parentGroup();
    if (parent == nullptr)
    {
        return;
    }

    parent->insertChild(mIndex, mNode);
    mNode  = nullptr;
    mOwned = false;
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

//////////////////////////////////////////////////////////////////////////
// SMSetConditionLeafCommand
//////////////////////////////////////////////////////////////////////////

SMSetConditionLeafCommand::SMSetConditionLeafCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                     , uint32_t transitionId, uint32_t leafId, const SMConditionEntry& content
                                                     , const QString& text, QUndoCommand* parent)
    : SMCommand (data, notifier, text, parent)
    , mTransId  (transitionId)
    , mLeafId   (leafId)
    , mNew      (content)
    , mOld      ( )
    , mCaptured (false)
{
}

void SMSetConditionLeafCommand::redo()
{
    SMConditionList* root = rootConditions(data(), mTransId);
    SMConditionEntry* leaf = (root != nullptr) ? root->findLeaf(mLeafId) : nullptr;
    if (leaf == nullptr)
    {
        return;
    }

    if (mCaptured == false)
    {
        mOld.assignContent(*leaf);
        mCaptured = true;
    }

    leaf->assignContent(mNew);
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

void SMSetConditionLeafCommand::undo()
{
    SMConditionList* root = rootConditions(data(), mTransId);
    SMConditionEntry* leaf = (root != nullptr) ? root->findLeaf(mLeafId) : nullptr;
    if (leaf == nullptr)
    {
        return;
    }

    leaf->assignContent(mOld);
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

//////////////////////////////////////////////////////////////////////////
// SMSetGroupCombineCommand
//////////////////////////////////////////////////////////////////////////

SMSetGroupCombineCommand::SMSetGroupCombineCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                   , uint32_t transitionId, uint32_t groupId, SMConditionGroup::eCombine combine
                                                   , const QString& text, QUndoCommand* parent)
    : SMCommand (data, notifier, text, parent)
    , mTransId  (transitionId)
    , mGroupId  (groupId)
    , mNew      (combine)
    , mOld      (SMConditionGroup::eCombine::And)
    , mCaptured (false)
{
}

void SMSetGroupCombineCommand::redo()
{
    SMConditionList* root = rootConditions(data(), mTransId);
    SMConditionGroup* group = (root != nullptr) ? root->findGroup(mGroupId) : nullptr;
    if (group == nullptr)
    {
        return;
    }

    if (mCaptured == false)
    {
        mOld = group->getCombine();
        mCaptured = true;
    }

    group->setCombine(mNew);
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

void SMSetGroupCombineCommand::undo()
{
    SMConditionList* root = rootConditions(data(), mTransId);
    SMConditionGroup* group = (root != nullptr) ? root->findGroup(mGroupId) : nullptr;
    if (group == nullptr)
    {
        return;
    }

    group->setCombine(mOld);
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

//////////////////////////////////////////////////////////////////////////
// SMSetGroupNegateCommand
//////////////////////////////////////////////////////////////////////////

SMSetGroupNegateCommand::SMSetGroupNegateCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                 , uint32_t transitionId, uint32_t groupId, bool negate
                                                 , const QString& text, QUndoCommand* parent)
    : SMCommand (data, notifier, text, parent)
    , mTransId  (transitionId)
    , mGroupId  (groupId)
    , mNew      (negate)
    , mOld      (false)
    , mCaptured (false)
{
}

void SMSetGroupNegateCommand::redo()
{
    SMConditionList* root = rootConditions(data(), mTransId);
    SMConditionGroup* group = (root != nullptr) ? root->findGroup(mGroupId) : nullptr;
    if (group == nullptr)
    {
        return;
    }

    if (mCaptured == false)
    {
        mOld = group->isNegated();
        mCaptured = true;
    }

    group->setNegated(mNew);
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

void SMSetGroupNegateCommand::undo()
{
    SMConditionList* root = rootConditions(data(), mTransId);
    SMConditionGroup* group = (root != nullptr) ? root->findGroup(mGroupId) : nullptr;
    if (group == nullptr)
    {
        return;
    }

    group->setNegated(mOld);
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

//////////////////////////////////////////////////////////////////////////
// SMReorderConditionCommand
//////////////////////////////////////////////////////////////////////////

SMReorderConditionCommand::SMReorderConditionCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                     , uint32_t transitionId, uint32_t groupId, int index1, int index2
                                                     , const QString& text, QUndoCommand* parent)
    : SMCommand (data, notifier, text, parent)
    , mTransId  (transitionId)
    , mGroupId  (groupId)
    , mIndex1   (index1)
    , mIndex2   (index2)
{
}

void SMReorderConditionCommand::apply()
{
    SMConditionList* root = rootConditions(data(), mTransId);
    SMConditionGroup* group = (root != nullptr) ? root->findGroup(mGroupId) : nullptr;
    if (group == nullptr)
    {
        return;
    }

    group->swapChildren(mIndex1, mIndex2);
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

void SMReorderConditionCommand::redo()
{
    apply();
}

void SMReorderConditionCommand::undo()
{
    apply();
}

//////////////////////////////////////////////////////////////////////////
// SMPromoteLambdaCommand
//////////////////////////////////////////////////////////////////////////

SMPromoteLambdaCommand::SMPromoteLambdaCommand(  StateMachineData& data, DocModelNotifier& notifier
                                               , uint32_t transitionId, uint32_t leafId, const QString& methodName
                                               , const QString& text, QUndoCommand* parent)
    : SMCompositeCommand(data, notifier, text, parent)
{
    SMConditionList* root = rootConditions(data, transitionId);
    SMConditionEntry* leaf = (root != nullptr) ? root->findLeaf(leafId) : nullptr;
    if (leaf == nullptr)
    {
        return;
    }

    // Move the verbatim body to a new Embedded condition method carrying it verbatim.
    SMMethodEntry* method = new SMMethodEntry(0u, methodName, SMMethodEntry::eMethodType::Condition);
    method->setImplement(SMMethodEntry::eImplement::Embedded);
    method->setReturn(SMMethodEntry::DEFAULT_RETURN);
    method->setBody(leaf->getBody());
    new TDocAddCommand<SMMethodEntry*, DocumentElem>(notifier, data.getMethods(), method, eDocElementKind::Method, text, this);

    // Replace the row with a parameterless cond:: reference to the promoted method.
    SMConditionEntry content;
    content.setLhsKind(SMArgumentEntry::eValueSource::Condition);
    content.setLhs(methodName);
    content.setOperator(SMConditionEntry::eOperator::None);
    content.setNegated(leaf->isNegated());
    new SMSetConditionLeafCommand(data, notifier, transitionId, leafId, content, text, this);
}
