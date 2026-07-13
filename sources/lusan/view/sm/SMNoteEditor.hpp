#ifndef LUSAN_VIEW_SM_SMNOTEEDITOR_HPP
#define LUSAN_VIEW_SM_SMNOTEEDITOR_HPP
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
 *  \file        lusan/view/sm/SMNoteEditor.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas in-place note text editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QRectF>
#include <QString>

#include <functional>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QGraphicsItem;
class QGraphicsProxyWidget;

/**
 * \class   SMNoteEditor
 * \brief   A reusable in-place multi-line text editor for a note bound to a canvas item
 *          (a state box or a transition edge). It hosts a QPlainTextEdit inside a graphics
 *          proxy drawn on top of the owner item, so the note text is edited over the owner
 *          and collapses back to the owner's note badge when finished. Enter inserts a
 *          newline, Esc cancels, and the commit fires on focus-out (the note text-commit
 *          convention, matching SMNoteItem). Not a QObject; owned by value by the item.
 **/
class SMNoteEditor
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMNoteEditor() = default;
    ~SMNoteEditor();

    SMNoteEditor(const SMNoteEditor&) = delete;
    SMNoteEditor& operator=(const SMNoteEditor&) = delete;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   True while the editor is open.
     **/
    bool isActive() const;

    /**
     * \brief   Opens the editor over the host item.
     * \param   host    The graphics item to draw the editor on top of (parent of the proxy).
     * \param   rect    The editor rectangle in the host item's local coordinates.
     * \param   text    The initial text.
     * \param   commit  Invoked once on focus-out with the edited text (never on Esc-cancel).
     **/
    void open(QGraphicsItem* host, const QRectF& rect, const QString& text
             , std::function<void(const QString&)> commit);

    /**
     * \brief   Closes and destroys the editor without committing.
     **/
    void close();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QGraphicsProxyWidget*   mProxy    { nullptr };   //!< The open editor proxy, or nullptr.
    bool                    mClosing  { false };     //!< Guards re-entrant teardown.
};

#endif  // LUSAN_VIEW_SM_SMNOTEEDITOR_HPP
