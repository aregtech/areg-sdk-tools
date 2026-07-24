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

#include <QList>
#include <QRectF>

/**
 * \class   SMAddStateCommand
 * \brief   Adds a new state to a machine level. A convenience over the shared add command
 *          that builds the state entry so the caller supplies only name and kind. The name
 *          is assumed unique — paste-time de-duplication is a caller concern.
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
 * \class   SMCreateStateCommand
 * \brief   Places a new state on the canvas as one undo step: appends the state to its
 *          level and creates its Node layout entry at the drop geometry. The state's ID
 *          is allocated by the insertion; read it back with getStateId() after the push.
 **/
class SMCreateStateCommand : public SMCompositeCommand
{
public:
    SMCreateStateCommand(  StateMachineData& data, DocModelNotifier& notifier
                         , SMStateData& level, const QString& name, SMStateEntry::eStateKind kind
                         , const QRectF& geometry, const QString& text, QUndoCommand* parent = nullptr);

    /**
     * \brief   The created state's element ID; valid after the first redo (push).
     **/
    uint32_t getStateId() const;

private:
    SMStateEntry*   mState;     //!< The created entry (owned by the add child / container).
};

/**
 * \class   SMRenameStateCommand
 * \brief   Renames a state. Name syntax and document-wide uniqueness are the caller's
 *          responsibility — the command applies the change and broadcasts it.
 **/
class SMRenameStateCommand : public SMCommand
{
public:
    SMRenameStateCommand(  StateMachineData& data, DocModelNotifier& notifier
                         , uint32_t stateId, const QString& newName
                         , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void apply(const QString& name, const QString& previous);

private:
    uint32_t        mId;                //!< The renamed state's ID.
    QString         mNew;               //!< The new name.
    QString         mOld;               //!< The previous name, captured on first redo.
    bool            mCaptured { false };
};

/**
 * \class   SMConvertToCompositeCommand
 * \brief   Converts a Normal state into a painted composite as one undo step: attaches a
 *          new nested StateList, adds the level's mandatory Start state, and creates that
 *          state's Node layout entry. Built empty (no effect) when the state cannot become
 *          composite: Start/Final kind, imported submachine, or already composite. Undo
 *          detaches the list and restores the plain state; redo re-attaches the identical
 *          subtree, so IDs never change.
 **/
class SMConvertToCompositeCommand : public SMCompositeCommand
{
public:
    /**
     * \brief   Creates the command.
     * \param   data            The document root.
     * \param   notifier        The change-notification hub.
     * \param   stateId         The state to convert.
     * \param   startName       The document-unique name of the new Start state.
     * \param   startGeometry   The Start state's initial box geometry on the new level.
     * \param   text            The undo-stack display text.
     * \param   parent          The owning composite command.
     **/
    SMConvertToCompositeCommand(  StateMachineData& data, DocModelNotifier& notifier
                                , uint32_t stateId, const QString& startName
                                , const QRectF& startGeometry, const QString& text
                                , QUndoCommand* parent = nullptr);

    /**
     * \brief   True when the state can be converted and the command has an effect.
     **/
    bool isEffective() const;

    /**
     * \brief   The created Start state's element ID; valid after the first redo (push).
     **/
    uint32_t getStartId() const;

private:
    SMStateEntry*   mStart;     //!< The created Start entry (owned by the add child / list).
};

/**
 * \class   SMRemoveStateCommand
 * \brief   Deletes a state as one undo step: the state (with its owned substates,
 *          transitions and operations, captured verbatim by the pointer container), every
 *          transition of a surviving state that targets the deleted subtree, plus the
 *          layout entries of all of them. Composed of a layout-removal child, incoming-
 *          transition-removal children, and a state-removal child, so undo restores every
 *          ID exactly.
 **/
class SMRemoveStateCommand : public SMCompositeCommand
{
public:
    SMRemoveStateCommand(StateMachineData& data, DocModelNotifier& notifier, SMStateData& level, uint32_t stateId, const QString& text, QUndoCommand* parent = nullptr);
};

#endif  // LUSAN_MODEL_SM_SMSTATECOMMANDS_HPP
