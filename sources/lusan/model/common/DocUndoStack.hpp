#ifndef LUSAN_MODEL_COMMON_DOCUNDOSTACK_HPP
#define LUSAN_MODEL_COMMON_DOCUNDOSTACK_HPP
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
 *  \file        lusan/model/common/DocUndoStack.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the undo stack shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QUndoCommand>
#include <QUndoStack>

/**
 * \class   DocUndoStack
 * \brief   The document's undo stack, with one addition: while the document is read-only it
 *          accepts nothing. A command reaches the model only through `push()`, so this is the
 *          single place a read-only document has to defend -- rather than every one of the
 *          call sites that build commands.
 *
 *          `push()` deliberately shadows the non-virtual `QUndoStack::push()`. Every caller
 *          holds this type (the document hands it out), so the shadow is what they call; Qt never
 *          pushes anything itself. A refused command is deleted, exactly as `QUndoStack` would
 *          delete a merged one, so no caller has to think about ownership.
 **/
class DocUndoStack : public QUndoStack
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit DocUndoStack(QObject* parent = nullptr)
        : QUndoStack(parent)
        , mReadOnly (false)
    {
    }

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline bool isReadOnly() const;
    inline void setReadOnly(bool readOnly);

    /**
     * \brief   Applies the command and records it, or drops it while the document is read-only.
     * \param   command The command to apply; taken over in both cases.
     * \return  True when the command was applied.
     **/
    inline bool push(QUndoCommand* command);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    bool    mReadOnly;
};

//////////////////////////////////////////////////////////////////////////
// DocUndoStack inline methods
//////////////////////////////////////////////////////////////////////////

inline bool DocUndoStack::isReadOnly() const
{
    return mReadOnly;
}

inline void DocUndoStack::setReadOnly(bool readOnly)
{
    mReadOnly = readOnly;
}

inline bool DocUndoStack::push(QUndoCommand* command)
{
    if (mReadOnly)
    {
        delete command;
        return false;
    }

    QUndoStack::push(command);
    return true;
}

#endif  // LUSAN_MODEL_COMMON_DOCUNDOSTACK_HPP
