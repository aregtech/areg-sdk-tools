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
 *  \file        lusan/view/sm/SMArgSinkOperation.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM argument sink over an operation's argument list.
 *
 ************************************************************************/

#include "lusan/view/sm/SMArgSinkOperation.hpp"

#include "lusan/common/ElementBase.hpp"
#include "lusan/model/sm/SMArgumentCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include <QUndoStack>

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMArgSinkOperation::SMArgSinkOperation(StateMachineModel& model)
    : IArgSink  ( )
    , mModel    (model)
    , mOwner    (nullptr)
    , mArgs     (nullptr)
{
}

//////////////////////////////////////////////////////////////////////////
// Binding
//////////////////////////////////////////////////////////////////////////

void SMArgSinkOperation::bind(ElementBase* owner, QList<SMArgumentEntry>* args)
{
    mOwner = owner;
    mArgs  = args;
}

void SMArgSinkOperation::clearBinding(void)
{
    mOwner = nullptr;
    mArgs  = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Commits
//////////////////////////////////////////////////////////////////////////

void SMArgSinkOperation::setArg( const QString& paramName
                               , SMArgumentEntry::eValueSource src
                               , const QString& value
                               , const QString& expr)
{
    push(paramName, true, src, value, expr);
}

void SMArgSinkOperation::clearArg(const QString& paramName)
{
    push(paramName, false, SMArgumentEntry::eValueSource::Value, QString(), QString());
}

void SMArgSinkOperation::push( const QString& paramName
                             , bool mapped
                             , SMArgumentEntry::eValueSource src
                             , const QString& value
                             , const QString& expr)
{
    if (isBound() == false)
    {
        return;
    }

    // A commit that changes nothing must not reach the undo stack: one real edit is exactly
    // one undo step, and a plain focus-out re-commits the value already shown.
    const SMArgumentEntry* current = argFor(paramName);
    const bool same = mapped
                      ? ((current != nullptr) && (current->getSource() == src)
                         && (current->getValue() == value) && (current->getExpression() == expr))
                      : (current == nullptr);
    if (same)
    {
        return;
    }

    mModel.getUndoStack().push(new SMSetArgumentCommand( mModel.getNotifier()
                                                       , *mOwner
                                                       , *mArgs
                                                       , paramName
                                                       , mapped
                                                       , src
                                                       , value
                                                       , expr
                                                       , tr("Map parameter '%1'").arg(paramName)));
}

//////////////////////////////////////////////////////////////////////////
// Queries
//////////////////////////////////////////////////////////////////////////

const SMArgumentEntry* SMArgSinkOperation::argFor(const QString& paramName) const
{
    if (mArgs != nullptr)
    {
        for (const SMArgumentEntry& entry : *mArgs)
        {
            if (entry.getName() == paramName)
            {
                return &entry;
            }
        }
    }

    return nullptr;
}
