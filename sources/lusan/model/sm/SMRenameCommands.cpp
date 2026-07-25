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
 *  \file        lusan/model/sm/SMRenameCommands.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM atomic-rename reference-rewrite command.
 *
 ************************************************************************/

#include "lusan/model/sm/SMRenameCommands.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"

SMRewriteReferencesCommand::SMRewriteReferencesCommand(  StateMachineData& data, DocModelNotifier& notifier
                                                       , SMReferences::eTarget target, uint32_t elementId
                                                       , const QString& oldName, const QString& newName
                                                       , const QString& text, QUndoCommand* parent)
    : SMCommand (data, notifier, text, parent)
    , mTarget   (target)
    , mElementId(elementId)
    , mOld      (oldName)
    , mNew      (newName)
{
}

void SMRewriteReferencesCommand::redo()
{
    apply(mOld, mNew);
}

void SMRewriteReferencesCommand::undo()
{
    apply(mNew, mOld);
}

void SMRewriteReferencesCommand::apply(const QString& from, const QString& to)
{
    // Resolve the owners before the rewrite so each referencing state/transition can be
    // told to refresh; then flip every matching reference field.
    const QList<SMReferences::Use> uses = SMReferences::whereUsed(data(), mTarget, from, mElementId);
    SMReferences::rewriteReferences(data(), mTarget, from, to);

    for (const SMReferences::Use& use : uses)
    {
        notifier().notifyElementChanged(use.navId, use.isState ? eDocElementKind::State : eDocElementKind::Transition);
    }
}
