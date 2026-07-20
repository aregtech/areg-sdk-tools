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
 *  \file        lusan/model/sm/SMOperationCommands.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operation-list edit commands (undoable).
 *
 ************************************************************************/

#include "lusan/model/sm/SMOperationCommands.hpp"

//////////////////////////////////////////////////////////////////////////
// SMAddOperationCommand
//////////////////////////////////////////////////////////////////////////

SMAddOperationCommand::SMAddOperationCommand( DocModelNotifier& notifier
                                            , SMOperationList& list
                                            , SMOperationBase* operation
                                            , int index
                                            , const QString& text
                                            , QUndoCommand* parent /*= nullptr*/)
    : DocCommand    (notifier, text, parent)
    , mList         (list)
    , mOperation    (operation)
    , mId           (0u)
    , mIndex        (index)
    , mOwned        (true)
{
}

SMAddOperationCommand::~SMAddOperationCommand()
{
    if (mOwned)
    {
        delete mOperation;
    }
}

void SMAddOperationCommand::redo()
{
    mList.insertOperation(mIndex, mOperation);
    // First run: getId() lazily allocates from the document counter. Later runs reuse it.
    mId    = mOperation->getId();
    mIndex = mList.indexOf(mId);
    mOwned = false;
    notifier().notifyElementAdded(mId, eDocElementKind::Operation);
}

void SMAddOperationCommand::undo()
{
    mIndex = mList.indexOf(mId);
    mOperation = mList.takeAt(mIndex);
    mOwned = true;
    notifier().notifyElementRemoved(mId, eDocElementKind::Operation);
}

//////////////////////////////////////////////////////////////////////////
// SMRemoveOperationCommand
//////////////////////////////////////////////////////////////////////////

SMRemoveOperationCommand::SMRemoveOperationCommand( DocModelNotifier& notifier
                                                  , SMOperationList& list
                                                  , uint32_t operationId
                                                  , const QString& text
                                                  , QUndoCommand* parent /*= nullptr*/)
    : DocCommand    (notifier, text, parent)
    , mList         (list)
    , mOperation    (nullptr)
    , mId           (operationId)
    , mIndex        (-1)
    , mOwned        (false)
{
}

SMRemoveOperationCommand::~SMRemoveOperationCommand()
{
    if (mOwned)
    {
        delete mOperation;
    }
}

void SMRemoveOperationCommand::redo()
{
    mIndex = mList.indexOf(mId);
    mOperation = mList.takeAt(mIndex);
    mOwned = true;
    notifier().notifyElementRemoved(mId, eDocElementKind::Operation);
}

void SMRemoveOperationCommand::undo()
{
    mList.insertOperation(mIndex, mOperation);
    mOwned = false;
    notifier().notifyElementAdded(mId, eDocElementKind::Operation);
}

//////////////////////////////////////////////////////////////////////////
// SMReorderOperationCommand
//////////////////////////////////////////////////////////////////////////

SMReorderOperationCommand::SMReorderOperationCommand( DocModelNotifier& notifier
                                                    , SMOperationList& list
                                                    , int index1
                                                    , int index2
                                                    , uint32_t ownerId
                                                    , const QString& text
                                                    , QUndoCommand* parent /*= nullptr*/)
    : DocCommand    (notifier, text, parent)
    , mList         (list)
    , mIndex1       (index1)
    , mIndex2       (index2)
    , mOwnerId      (ownerId)
{
}

void SMReorderOperationCommand::redo()
{
    apply();
}

void SMReorderOperationCommand::undo()
{
    apply();
}

void SMReorderOperationCommand::apply()
{
    mList.swapOperations(mIndex1, mIndex2);
    notifier().notifyListReordered(mOwnerId, eDocElementKind::Operation);
}
