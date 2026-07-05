#ifndef LUSAN_MODEL_SM_SMSTATECOMMANDS_HPP
#define LUSAN_MODEL_SM_SMSTATECOMMANDS_HPP
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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/sm/SMStateCommands.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM state add/delete commands.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/sm/SMCommand.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/data/sm/SMState.hpp"

/**
 * \class   SMAddStateCommand
 * \brief   Adds a new state to a machine level. A convenience over the shared add command
 *          that builds the state entry so the caller supplies only name and kind. The name
 *          is assumed unique — paste-time de-duplication (spec 9.8.2) is a caller concern.
 **/
class SMAddStateCommand : public TDocAddCommand<SMStateEntry*, DocumentElem>
{
public:
    SMAddStateCommand(DocModelNotifier& notifier, SMStateData& level, const QString& name, SMStateEntry::eStateKind kind, const QString& text, QUndoCommand* parent = nullptr)
        : TDocAddCommand<SMStateEntry*, DocumentElem>(notifier, level, new SMStateEntry(0, name, kind, &level), eDocElementKind::State, text, parent)
    {
    }
};

/**
 * \class   SMRemoveStateCommand
 * \brief   Deletes a state as one undo step: the state (with its owned substates,
 *          transitions and operations, captured verbatim by the pointer container) plus
 *          the layout entries of the whole subtree. Composed of a layout-removal child and
 *          a state-removal child, so undo restores every ID exactly.
 **/
class SMRemoveStateCommand : public SMCompositeCommand
{
public:
    SMRemoveStateCommand(StateMachineData& data, DocModelNotifier& notifier, SMStateData& level, uint32_t stateId, const QString& text, QUndoCommand* parent = nullptr);
};

#endif  // LUSAN_MODEL_SM_SMSTATECOMMANDS_HPP
