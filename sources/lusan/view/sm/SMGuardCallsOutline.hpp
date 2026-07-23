#ifndef LUSAN_VIEW_SM_SMGUARDCALLSOUTLINE_HPP
#define LUSAN_VIEW_SM_SMGUARDCALLSOUTLINE_HPP
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
 *  \file        lusan/view/sm/SMGuardCallsOutline.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard Calls outline (accordion section 1).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/view/sm/SMGuardCatalog.hpp"

#include <QList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QListWidget;
class QListWidgetItem;
class StateMachineModel;
class SMGuardNode;
enum class eDocElementKind;

/**
 * \class   SMGuardCallsOutline
 * \brief   The accordion's Conditions section (design 8.1): a path-keyed projection of the
 *          committed guard tree PLUS a list of the defined condition methods to insert. One
 *          row per condition-method call (`name(sig) : ret [N unmapped]`), per top-level
 *          reference (`name -- attribute (read)`), and per verbatim island / raw fragment
 *          (`raw C++`) already in the guard; then, under an "Insert a condition:" header, one
 *          row per defined condition method. Selecting a call row emits \ref callSelected so the
 *          host drives the SINGLE Arguments table; double-clicking an insert row emits
 *          \ref insertRequested so the host inserts `@cond:name()` at the caret; double-clicking
 *          an island row emits \ref islandActivated so the host opens the island editor;
 *          right-clicking a reference row emits \ref whereUsedRequested (global where-used).
 *
 *          The outline is a display-only projection: it addresses every node by its
 *          child-index PATH (nth match in pre-order), never by a cached offset,
 *          and re-projects on every model change. Width-safe: its
 *          \ref minimumSizeHint width is 0 and the list scrolls horizontally inside, so a
 *          long signature can never widen the Properties dock.
 **/
class SMGuardCallsOutline : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMGuardCallsOutline(StateMachineModel& model, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Points the outline at a transition (0 clears it) and rebuilds the pickup list.
    void setTransition(uint32_t transitionId);

    //!< Rebuilds the pickup list of defined condition methods.
    void refresh(void);

    //!< The number of insertable condition-method rows in the pickup list (tests).
    int insertRowCount(void) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    //!< Width 0 so no signature length can widen the hosting dock.
    virtual QSize minimumSizeHint(void) const override;

signals:
    //!< An "available condition" row was activated: insert \p symbol's `@cond:name()` at the caret.
    void insertRequested(const SMGuardSymbol& symbol);
    //!< A pickup row's context menu asked for the global where-used of \p symbolId.
    void whereUsedRequested(uint32_t symbolId);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onElementAdded(uint32_t id, eDocElementKind kind);
    void onElementChanged(uint32_t id, eDocElementKind kind);
    void onElementRemoved(uint32_t id, eDocElementKind kind);
    void onDocumentReloaded(void);
    void onItemActivated(QListWidgetItem* item);
    void onContextMenu(const QPoint& pos);

private:
    void scheduleRebuild(void);
    void rebuild(void);

    //!< Appends one double-click-to-insert row per defined condition method.
    void appendInsertableConditions(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;         //!< The document facade.
    uint32_t            mTransId;       //!< The shown transition (0 = none).
    QListWidget*        mList;          //!< The outline rows.
    QList<SMGuardSymbol> mInsertable;   //!< The defined condition methods (insertable rows).
    bool                mRebuildPending;//!< Coalesces deferred rebuilds.
};

#endif  // LUSAN_VIEW_SM_SMGUARDCALLSOUTLINE_HPP
