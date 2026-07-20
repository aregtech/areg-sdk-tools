#ifndef LUSAN_MODEL_SM_SMOPERATIONCOMMANDS_HPP
#define LUSAN_MODEL_SM_SMOPERATIONCOMMANDS_HPP
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
 *  \file        lusan/model/sm/SMOperationCommands.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operation-list edit commands (undoable).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocCommand.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"

#include "lusan/data/sm/SMOperation.hpp"

#include <functional>

/**
 * \class   SMAddOperationCommand
 * \brief   Inserts one pre-built operation into an `EntryList` / `ExitList` /
 *          `OperationList` at a position. Redo inserts (allocating the operation's ID on the
 *          first run and reusing it verbatim afterwards, so history navigation never
 *          renumbers); undo removes and captures the operation for the next redo. While the
 *          operation is out of the list, the command owns it. Notifies `Operation` so the
 *          editor row, the state box body, and the mapping grid all refresh.
 **/
class SMAddOperationCommand : public DocCommand
{
public:
    SMAddOperationCommand( DocModelNotifier& notifier
                         , SMOperationList& list
                         , SMOperationBase* operation
                         , int index
                         , const QString& text
                         , QUndoCommand* parent = nullptr);
    ~SMAddOperationCommand() override;

    void redo() override;
    void undo() override;

private:
    SMOperationList&    mList;
    SMOperationBase*    mOperation;     //!< Owned while out of the list.
    uint32_t            mId;
    int                 mIndex;
    bool                mOwned;
};

/**
 * \class   SMRemoveOperationCommand
 * \brief   Removes one operation (by ID) from its list, capturing it for undo and
 *          re-inserting it at its original position under its original ID on undo.
 **/
class SMRemoveOperationCommand : public DocCommand
{
public:
    SMRemoveOperationCommand( DocModelNotifier& notifier
                            , SMOperationList& list
                            , uint32_t operationId
                            , const QString& text
                            , QUndoCommand* parent = nullptr);
    ~SMRemoveOperationCommand() override;

    void redo() override;
    void undo() override;

private:
    SMOperationList&    mList;
    SMOperationBase*    mOperation;     //!< Owned while out of the list.
    uint32_t            mId;
    int                 mIndex;
    bool                mOwned;
};

/**
 * \class   SMReorderOperationCommand
 * \brief   Swaps two operations of one list (execution order = list order). Its own inverse:
 *          redo and undo are the same swap. Notifies `listReordered(ownerId, Operation)`.
 **/
class SMReorderOperationCommand : public DocCommand
{
public:
    SMReorderOperationCommand( DocModelNotifier& notifier
                             , SMOperationList& list
                             , int index1
                             , int index2
                             , uint32_t ownerId
                             , const QString& text
                             , QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void apply();

private:
    SMOperationList&    mList;
    int                 mIndex1;
    int                 mIndex2;
    uint32_t            mOwnerId;
};

/**
 * \class   SMSetOperationPropertyCommand
 * \brief   Sets one property of an operation resolved by ID inside its list (the operation
 *          pointer is not stable across a sibling remove/re-add, so it is re-resolved on every
 *          run). One command shape for every scalar field: action/attribute/timer/event name,
 *          timeout/repeat override, attribute value/source, inline body. Captures the prior
 *          value on the first redo for exact undo.
 *
 * \tparam  Value   A default-constructible, copy-assignable field type.
 **/
template<typename Value>
class SMSetOperationPropertyCommand : public DocCommand
{
public:
    using Getter = std::function<Value(SMOperationBase&)>;
    using Setter = std::function<void(SMOperationBase&, const Value&)>;

    SMSetOperationPropertyCommand( DocModelNotifier& notifier
                                 , SMOperationList& list
                                 , uint32_t operationId
                                 , Getter getter
                                 , Setter setter
                                 , Value newValue
                                 , const QString& text
                                 , QUndoCommand* parent = nullptr)
        : DocCommand(notifier, text, parent)
        , mList     (list)
        , mId       (operationId)
        , mGet      (std::move(getter))
        , mSet      (std::move(setter))
        , mOld      ( )
        , mNew      (std::move(newValue))
    {
    }

    void redo() override
    {
        SMOperationBase* op = resolve();
        if (op != nullptr)
        {
            mOld = mGet(*op);
            mSet(*op, mNew);
            notifier().notifyElementChanged(mId, eDocElementKind::Operation);
        }
    }

    void undo() override
    {
        SMOperationBase* op = resolve();
        if (op != nullptr)
        {
            mSet(*op, mOld);
            notifier().notifyElementChanged(mId, eDocElementKind::Operation);
        }
    }

private:
    SMOperationBase* resolve() const
    {
        return mList.findById(mId);
    }

private:
    SMOperationList&    mList;
    uint32_t            mId;
    Getter              mGet;
    Setter              mSet;
    Value               mOld;
    Value               mNew;
};

#endif  // LUSAN_MODEL_SM_SMOPERATIONCOMMANDS_HPP
