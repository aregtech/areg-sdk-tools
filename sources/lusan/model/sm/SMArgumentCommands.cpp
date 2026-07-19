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
 *  \file        lusan/model/sm/SMArgumentCommands.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM parameter-mapping argument edit commands (undoable).
 *
 ************************************************************************/

#include "lusan/model/sm/SMArgumentCommands.hpp"

#include "lusan/common/ElementBase.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"

SMSetArgumentCommand::SMSetArgumentCommand( DocModelNotifier& notifier
                                          , ElementBase& owner
                                          , QList<SMArgumentEntry>& args
                                          , const QString& paramName
                                          , bool mapped
                                          , SMArgumentEntry::eValueSource source
                                          , const QString& value
                                          , const QString& expression
                                          , const QString& text
                                          , QUndoCommand* parent /*= nullptr*/)
    : DocCommand    (notifier, text, parent)
    , mOwner        (owner)
    , mArgs         (args)
    , mOwnerId      (owner.getId())
    , mParam        (paramName)
    , mNewMapped    (mapped)
    , mNewSource    (source)
    , mNewValue     (value)
    , mNewExpr      (expression)
    , mCaptured     (false)
    , mOldMapped    (false)
    , mOldSource    (SMArgumentEntry::eValueSource::Value)
    , mOldValue     ( )
    , mOldExpr      ( )
{
}

int SMSetArgumentCommand::findIndex() const
{
    for (int i = 0; i < mArgs.size(); ++i)
    {
        if (mArgs.at(i).getName() == mParam)
        {
            return i;
        }
    }

    return -1;
}

void SMSetArgumentCommand::applyState(bool mapped, SMArgumentEntry::eValueSource source, const QString& value, const QString& expression)
{
    const int index = findIndex();
    if (mapped)
    {
        if (index < 0)
        {
            SMArgumentEntry entry(mOwner.getNextId(), mParam, source, value, &mOwner);
            entry.setExpression(expression);
            mArgs.append(entry);
        }
        else
        {
            SMArgumentEntry& entry = mArgs[index];
            entry.setSource(source);
            entry.setValue(value);
            entry.setExpression(expression);
        }
    }
    else if (index >= 0)
    {
        mArgs.removeAt(index);
    }
}

void SMSetArgumentCommand::redo()
{
    if (mCaptured == false)
    {
        const int index = findIndex();
        mOldMapped = (index >= 0);
        if (mOldMapped)
        {
            const SMArgumentEntry& entry = mArgs.at(index);
            mOldSource = entry.getSource();
            mOldValue  = entry.getValue();
            mOldExpr   = entry.getExpression();
        }

        mCaptured = true;
    }

    applyState(mNewMapped, mNewSource, mNewValue, mNewExpr);
    notifier().notifyElementChanged(mOwnerId, eDocElementKind::Operation);
}

void SMSetArgumentCommand::undo()
{
    applyState(mOldMapped, mOldSource, mOldValue, mOldExpr);
    notifier().notifyElementChanged(mOwnerId, eDocElementKind::Operation);
}
