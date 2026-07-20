#ifndef LUSAN_VIEW_SM_SMREFCOMPLETER_HPP
#define LUSAN_VIEW_SM_SMREFCOMPLETER_HPP
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
 *  \file        lusan/view/sm/SMRefCompleter.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard reference completer (SM-21-03, D-POPUP).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/view/sm/SMGuardCatalog.hpp"

#include <QList>
#include <QSet>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QListView;
class QStandardItemModel;

/**
 * \class   SMRefCompleter
 * \brief   The ONE guard reference completer (D-POPUP), replacing the old SMSymbolPopup and its
 *          keystroke-swallow bug (D-BUG / 12.6). It is a TOP-LEVEL frameless, non-activating
 *          window: the field keeps focus and owns every keystroke, so a key that neither
 *          navigates nor commits reaches the document AND re-filters the list -- the completer
 *          filters, it never consumes (the swallow-bug fix). It positions itself at the caret
 *          and CLAMPS to the screen, flipping above the caret when it would overflow below
 *          (12.5); being a top-level window its width never touches the dock (12.4).
 *
 *          Three entry points (the field decides which): typing `@` (all kinds), typing
 *          `@kind:` (filtered to that kind), and later the Insert button (SM-21-04). Accepting
 *          a symbol emits \ref accepted; the field inserts the canonical `@kind:name`.
 **/
class SMRefCompleter : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMRefCompleter(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Sets the full symbol universe (the field rebuilds it when the transition changes).
    void setSymbols(const QList<SMGuardSymbol>& symbols);

    /**
     * \brief   Filters to symbols of a kind in \p kindFilter (empty = all kinds) whose name
     *          contains \p nameFilter, positions the window at \p caretGlobal (the caret's
     *          screen rect) -- clamped to the screen, flipped above on overflow -- and shows it
     *          without activation. Returns false (and hides) when nothing matches.
     **/
    bool showFor(const QSet<SMGuardSymbol::eRefKind>& kindFilter, const QString& nameFilter, const QRect& caretGlobal);

    //!< Moves the current row by \p delta, skipping headers.
    void moveCurrent(int delta);

    //!< The symbol on the current row, or nullptr when the row is a header / none.
    const SMGuardSymbol* currentSymbol() const;

    //!< Accepts the current row (emits \ref accepted for a real entry; a no-op otherwise).
    void acceptCurrent();

signals:
    //!< A real symbol row was accepted; the field inserts its canonical `@kind:name`.
    void accepted(const SMGuardSymbol& symbol);
    //!< The `New lambda here...` row was accepted: create an island at the caret (E4).
    void newLambdaRequested();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Rebuilds the model rows for the current filter; returns the count of real entries.
    int rebuild(const QSet<SMGuardSymbol::eRefKind>& kindFilter, const QString& nameFilter);

    //!< The mVisible index of the current row, or -1.
    int currentEntryIndex() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QListView*              mView;      //!< The catalog list view.
    QStandardItemModel*     mModel;     //!< The rebuilt row model.
    QList<SMGuardSymbol>    mAll;       //!< The full symbol universe.
    QList<SMGuardSymbol>    mVisible;   //!< The filtered entries, parallel to entry rows.
};

#endif  // LUSAN_VIEW_SM_SMREFCOMPLETER_HPP
