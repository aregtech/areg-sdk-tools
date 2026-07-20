#ifndef LUSAN_VIEW_SM_SMARGSINKOPERATION_HPP
#define LUSAN_VIEW_SM_SMARGSINKOPERATION_HPP
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
 *  \file        lusan/view/sm/SMArgSinkOperation.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM argument sink over an operation's argument list.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/sm/IArgSink.hpp"

#include <QCoreApplication>
#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class ElementBase;
class StateMachineModel;

/**
 * \class   SMArgSinkOperation
 * \brief   The action/event side of \ref IArgSink: commits one mapping row into an
 *          operation's live `SMArgumentEntry` list through an undoable
 *          `SMSetArgumentCommand`, keyed by parameter NAME.
 *
 *          Name keying is what makes the edit index-independent -- adding or removing a
 *          sibling parameter never invalidates a pending mapping. Clearing a slot removes
 *          the argument entirely, which is how a defaulted parameter reverts to its default
 *          and stays omitted from the saved file.
 *
 *          A commit that would not change the stored state is dropped, so re-committing the
 *          value already shown (a focus-out after no edit, say) never grows the undo stack:
 *          one real edit is exactly one undo step.
 **/
class SMArgSinkOperation : public IArgSink
{
    Q_DECLARE_TR_FUNCTIONS(SMArgSinkOperation)

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMArgSinkOperation(StateMachineModel& model);
    virtual ~SMArgSinkOperation(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Binds the sink to the argument list \p args of the operation \p owner.
     *          \p owner supplies ID delegation for a newly added argument and identifies
     *          the mutated element in change notifications. Passing a null \p owner or
     *          \p args makes the sink inert (every commit is ignored).
     **/
    void bind(ElementBase* owner, QList<SMArgumentEntry>* args);

    //!< Drops the binding; the sink becomes inert.
    void clearBinding(void);

    //!< True when the sink has an operation to commit into.
    inline bool isBound(void) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual void setArg( const QString& paramName
                       , SMArgumentEntry::eValueSource src
                       , const QString& value
                       , const QString& expr) override;

    virtual void clearArg(const QString& paramName) override;

    virtual const SMArgumentEntry* argFor(const QString& paramName) const override;

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Pushes one `SMSetArgumentCommand` unless the requested state is already the
     *          stored one.
     **/
    void push( const QString& paramName
             , bool mapped
             , SMArgumentEntry::eValueSource src
             , const QString& value
             , const QString& expr);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&      mModel;
    ElementBase*            mOwner;
    QList<SMArgumentEntry>* mArgs;
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline bool SMArgSinkOperation::isBound(void) const
{
    return (mOwner != nullptr) && (mArgs != nullptr);
}

#endif  // LUSAN_VIEW_SM_SMARGSINKOPERATION_HPP
