#ifndef LUSAN_MODEL_COMMON_DOCCOMMAND_HPP
#define LUSAN_MODEL_COMMON_DOCCOMMAND_HPP
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
 *  \file        lusan/model/common/DocCommand.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, document-agnostic undo/redo command base and composite,
 *               shared by the Service Interface and State Machine editors.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QUndoCommand>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DocModelNotifier;

/**
 * \class   DocCommand
 * \brief   The base of every document model mutation. It carries a reference to the
 *          document's notifier so that redo() applies a mutation and emits notifications,
 *          undo() applies the inverse and emits notifications; views never distinguish
 *          undo from a fresh edit — they see only notifier signals.
 *
 *          The base is document-agnostic on purpose: the generic element commands need
 *          only a container and a notifier. Editor-specific bases (SICommand,
 *          SMCommand) add access to their concrete document root for the few commands
 *          that touch it (layout, cross-section composites).
 **/
class DocCommand : public QUndoCommand
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    DocCommand(DocModelNotifier& notifier, const QString& text, QUndoCommand* parent = nullptr);

public:
    virtual ~DocCommand() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
protected:
    inline DocModelNotifier& notifier() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    DocModelNotifier&   mNotifier;
};

/**
 * \class   DocCompositeCommand
 * \brief   A single undo step composed of child commands (delete with subtree, atomic
 *          rename with references, paste). Children are appended with this command as
 *          their QUndoCommand parent; the default QUndoCommand traversal runs them forward
 *          on redo and in reverse on undo, so the whole gesture is one step.
 **/
class DocCompositeCommand : public DocCommand
{
public:
    DocCompositeCommand(DocModelNotifier& notifier, const QString& text, QUndoCommand* parent = nullptr);
};

//////////////////////////////////////////////////////////////////////////
// DocCommand inline methods
//////////////////////////////////////////////////////////////////////////

inline DocModelNotifier& DocCommand::notifier() const
{
    return mNotifier;
}

#endif  // LUSAN_MODEL_COMMON_DOCCOMMAND_HPP
