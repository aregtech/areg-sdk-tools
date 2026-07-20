#ifndef LUSAN_VIEW_SM_SMGUARDCATALOGVIEW_HPP
#define LUSAN_VIEW_SM_SMGUARDCATALOGVIEW_HPP
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
 *  \file        lusan/view/sm/SMGuardCatalogView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard Data catalog view (spec 8, SM-21-04).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <QHash>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLineEdit;
class QTableView;
class QSortFilterProxyModel;
class QModelIndex;
class QPoint;
class StateMachineModel;
class SMGuardCatalogModel;
struct SMGuardSymbol;

/**
 * \class   SMGuardCatalogView
 * \brief   The accordion's Data section (spec 8): a search box over a sortable, filterable,
 *          virtualized `QTableView` of the in-scope symbol catalog (`[hue | name | type |
 *          used-N]`). Double-clicking a row inserts the symbol's canonical reference at the
 *          caret (through the host / \ref SMRefCompleter path); right-clicking a row asks for
 *          the global where-used. It never widens the dock (hazard 12.4): its
 *          \ref minimumSizeHint width is 0 and the table scrolls horizontally inside.
 **/
class SMGuardCatalogView : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMGuardCatalogView(StateMachineModel& model, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Rebuilds the catalog for \p transitionId (0 clears it) from the document registries.
    void setTransition(uint32_t transitionId);

    //!< Re-enumerates the catalog for the current transition (symbols may have changed).
    void refresh(void);

    //!< Updates the `used-N` column from the host's bound-reference counts (by symbol id).
    void setUseCounts(const QHash<uint32_t, int>& counts);

    //!< Puts the keyboard focus in the search box (Data>> opens the section here).
    void focusSearch(void);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    //!< Width 0 so a long name or signature can never widen the hosting dock (hazard 12.4).
    virtual QSize minimumSizeHint(void) const override;

signals:
    //!< A row was double-clicked: insert this symbol's canonical reference at the caret.
    void insertRequested(const SMGuardSymbol& symbol);
    //!< A row's context menu asked for the global where-used of \p symbolId.
    void whereUsedRequested(uint32_t symbolId);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onSearchChanged(const QString& text);
    void onDoubleClicked(const QModelIndex& index);
    void onContextMenu(const QPoint& pos);

private:
    //!< The unfiltered model row for a proxy \p index, or -1.
    int sourceRow(const QModelIndex& index) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&      mModel;     //!< The document facade.
    uint32_t                mTransId;   //!< The catalog scope (0 = none).
    QLineEdit*              mSearch;    //!< The name filter box.
    QTableView*             mTable;     //!< The catalog table.
    SMGuardCatalogModel*    mCatalog;   //!< The wrapped enumeration model.
    QSortFilterProxyModel*  mProxy;     //!< The sort/filter proxy.
};

#endif  // LUSAN_VIEW_SM_SMGUARDCATALOGVIEW_HPP
