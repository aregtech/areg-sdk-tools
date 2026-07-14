#ifndef LUSAN_VIEW_SM_SMSYMBOLPOPUP_HPP
#define LUSAN_VIEW_SM_SMSYMBOLPOPUP_HPP
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
 *  \file        lusan/view/sm/SMSymbolPopup.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard completion catalog popup (v7 B4).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/view/sm/SMGuardCatalog.hpp"

#include <QList>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QListView;
class QStandardItemModel;

/**
 * \class   SMSymbolPopup
 * \brief   The completion catalog (B4): a frameless owner-grouped list over the guard symbol
 *          universe, opened by typing or Ctrl+Space. Group headers (STIMULUS / FSM / HANDLER)
 *          are non-selectable; the rows filter as the user types; Enter accepts. It never
 *          steals focus from the field (shown without activation) -- the field forwards the
 *          navigation keys. The `New lambda here...` row is present but disabled (Session U3).
 **/
class SMSymbolPopup : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMSymbolPopup(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Sets the full symbol universe (the field rebuilds it when the transition changes).
    void setSymbols(const QList<SMGuardSymbol>& symbols);

    /**
     * \brief   Filters to symbols whose name contains \p prefix (fuzzy, case-insensitive),
     *          positions the popup at \p globalTopLeft and shows it. Returns false (and hides)
     *          when nothing matches.
     **/
    bool filterAndShow(const QString& prefix, const QPoint& globalTopLeft);

    //!< Moves the current row by \p delta, skipping headers and the disabled row.
    void moveCurrent(int delta);

    //!< The symbol on the current row, or nullptr when the row is a header / disabled / none.
    const SMGuardSymbol* currentSymbol() const;

    //!< Accepts the current row (emits \ref accepted for a real entry; a no-op otherwise).
    void acceptCurrent();

signals:
    //!< A real symbol row was accepted.
    void accepted(const SMGuardSymbol& symbol);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Rebuilds the model rows for the current filter; returns the count of real entries.
    int rebuild(const QString& prefix);

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

#endif  // LUSAN_VIEW_SM_SMSYMBOLPOPUP_HPP
