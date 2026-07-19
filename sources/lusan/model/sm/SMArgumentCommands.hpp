#ifndef LUSAN_MODEL_SM_SMARGUMENTCOMMANDS_HPP
#define LUSAN_MODEL_SM_SMARGUMENTCOMMANDS_HPP
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
 *  \file        lusan/model/sm/SMArgumentCommands.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM parameter-mapping argument edit commands (undoable).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocCommand.hpp"

#include "lusan/data/sm/SMOperation.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class ElementBase;

/**
 * \class   SMSetArgumentCommand
 * \brief   Maps or unmaps one declared parameter in an operation's argument list, keyed by
 *          parameter name (arguments bind by name -- spec 6.9). One command shape covers the
 *          three edits the mapping grid produces: set an existing mapping, add an explicit
 *          mapping to a previously-unmapped parameter, and clear a mapping back to its default
 *          (the argument is omitted). The name key makes the edit index-independent, so a
 *          sibling parameter's add/remove never invalidates this command's target.
 *
 *          The inverse state is captured on the first \ref redo, so undo restores the exact
 *          prior mapping (or its absence). Verbatim Expression text round-trips byte-exactly.
 **/
class SMSetArgumentCommand : public DocCommand
{
public:
    /**
     * \param   owner       The operation owning \p args (for ID delegation on a new argument);
     *                      its ID identifies the mutated element in change notifications.
     * \param   args        The operation's live argument list.
     * \param   paramName   The declared parameter this mapping binds to.
     * \param   mapped      True to set/add the explicit mapping; false to clear it (use default).
     **/
    SMSetArgumentCommand( DocModelNotifier& notifier
                        , ElementBase& owner
                        , QList<SMArgumentEntry>& args
                        , const QString& paramName
                        , bool mapped
                        , SMArgumentEntry::eValueSource source
                        , const QString& value
                        , const QString& expression
                        , const QString& text
                        , QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    //!< The index of the argument bound to \ref mParam, or -1 when unmapped.
    int findIndex() const;

    //!< Applies one mapping state (add/set when \p mapped, remove otherwise).
    void applyState(bool mapped, SMArgumentEntry::eValueSource source, const QString& value, const QString& expression);

private:
    ElementBase&                    mOwner;
    QList<SMArgumentEntry>&         mArgs;
    uint32_t                        mOwnerId;
    QString                         mParam;

    bool                            mNewMapped;
    SMArgumentEntry::eValueSource   mNewSource;
    QString                         mNewValue;
    QString                         mNewExpr;

    bool                            mCaptured;      //!< True once the prior state is recorded.
    bool                            mOldMapped;
    SMArgumentEntry::eValueSource   mOldSource;
    QString                         mOldValue;
    QString                         mOldExpr;
};

#endif  // LUSAN_MODEL_SM_SMARGUMENTCOMMANDS_HPP
