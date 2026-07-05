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
 *  \file        lusan/model/common/DocCommand.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, document-agnostic undo/redo command base and composite.
 *
 ************************************************************************/

#include "lusan/model/common/DocCommand.hpp"

DocCommand::DocCommand(DocModelNotifier& notifier, const QString& text, QUndoCommand* parent)
    : QUndoCommand  (text, parent)
    , mNotifier     (notifier)
{
}

DocCompositeCommand::DocCompositeCommand(DocModelNotifier& notifier, const QString& text, QUndoCommand* parent)
    : DocCommand(notifier, text, parent)
{
}
