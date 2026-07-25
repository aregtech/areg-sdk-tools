#ifndef LUSAN_MODEL_SM_SMRENAMECOMMANDS_HPP
#define LUSAN_MODEL_SM_SMRENAMECOMMANDS_HPP
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
 *  \file        lusan/model/sm/SMRenameCommands.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM atomic-rename reference-rewrite command.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/sm/SMCommand.hpp"
#include "lusan/data/sm/SMReferences.hpp"

#include <QString>

/**
 * \class   SMRewriteReferencesCommand
 * \brief   The reference-rewriting half of an atomic rename. Paired as a child with the
 *          primary name-set command inside one composite so a registry-entry rename and
 *          every name-based reference to it move in a single undo step (spec 9.7). Redo flips
 *          every reference from the old name to the new; undo flips them back. ID-based
 *          references (transition targets, guard symbols) are not touched -- they already
 *          reflect the new name -- so this command never has to know about them.
 **/
class SMRewriteReferencesCommand : public SMCommand
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMRewriteReferencesCommand(  StateMachineData& data, DocModelNotifier& notifier
                               , SMReferences::eTarget target, uint32_t elementId
                               , const QString& oldName, const QString& newName
                               , const QString& text, QUndoCommand* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    void redo() override;
    void undo() override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Rewrites every reference of \p from to \p to and notifies each affected owner.
    void apply(const QString& from, const QString& to);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMReferences::eTarget   mTarget;    //!< The kind of element being renamed.
    uint32_t                mElementId; //!< The renamed element's ID (guard-use resolution).
    QString                 mOld;       //!< The name before the rename.
    QString                 mNew;       //!< The name after the rename.
};

#endif  // LUSAN_MODEL_SM_SMRENAMECOMMANDS_HPP
