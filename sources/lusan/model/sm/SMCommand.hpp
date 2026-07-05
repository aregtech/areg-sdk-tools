#ifndef LUSAN_MODEL_SM_SMCOMMAND_HPP
#define LUSAN_MODEL_SM_SMCOMMAND_HPP
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
 *  \file        lusan/model/sm/SMCommand.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM command base — the shared DocCommand plus the
 *               FSM document root, for the few commands that touch the whole document.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocCommand.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \class   SMCommand
 * \brief   The FSM-editor command base. It adds the concrete document root to the shared
 *          DocCommand so layout and cross-section composite commands can reach it; the
 *          generic per-section commands (add/remove/property/reorder) use the shared
 *          templates directly and need only the notifier.
 **/
class SMCommand : public DocCommand
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    SMCommand(StateMachineData& data, DocModelNotifier& notifier, const QString& text, QUndoCommand* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
protected:
    inline StateMachineData& data(void) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineData&   mData;
};

/**
 * \class   SMCompositeCommand
 * \brief   An FSM composite (one undo step of child commands) that also exposes the
 *          document root, so composites like delete-state can build children that reach
 *          the layout section.
 **/
class SMCompositeCommand : public SMCommand
{
public:
    SMCompositeCommand(StateMachineData& data, DocModelNotifier& notifier, const QString& text, QUndoCommand* parent = nullptr);
};

//////////////////////////////////////////////////////////////////////////
// SMCommand inline methods
//////////////////////////////////////////////////////////////////////////

inline StateMachineData& SMCommand::data(void) const
{
    return mData;
}

#endif  // LUSAN_MODEL_SM_SMCOMMAND_HPP
