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
 *  \brief       Lusan application, FSM guard Calls outline (accordion section 1, spec 10 / 8.1).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

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
 * \brief   The accordion's Calls section (design 8.1): a path-keyed projection of the
 *          committed guard tree. One row per condition-method call (`name(sig) : ret
 *          [N unmapped]`), per top-level reference (`name -- attribute (read)`), and per
 *          verbatim island / raw fragment (`raw C++`). Selecting a call row emits
 *          \ref callSelected so the host drives the SINGLE Arguments table; double-clicking
 *          an island row emits \ref islandActivated so the host opens the island editor;
 *          right-clicking a reference row emits \ref whereUsedRequested (global where-used).
 *
 *          The outline is a display-only projection: it addresses every node by its
 *          child-index PATH (nth match in pre-order, hazard 12.1), never by a cached offset,
 *          and re-projects on every model change (D-SYNC). Width-safe (hazard 12.4): its
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
    //!< Points the outline at a transition (0 clears it) and re-projects.
    void setTransition(uint32_t transitionId);

    //!< Re-projects the rows from the current committed guard tree.
    void refresh(void);

    //!< The call path currently selected, or empty when no call row is selected.
    QList<int> selectedCallPath(void) const;

    //!< Selects the call row addressing \p callPath (no-op when absent); emits \ref callSelected.
    void selectCallPath(const QList<int>& callPath);

    //!< The number of call rows in the outline (tests).
    int callRowCount(void) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    //!< Width 0 so no signature length can widen the hosting dock (hazard 12.4).
    virtual QSize minimumSizeHint(void) const override;

signals:
    //!< A call row was selected: drive the Arguments table for \p callPath of \p methodId.
    void callSelected(const QList<int>& callPath, uint32_t methodId, int unmappedCount);
    //!< An island (verbatim lambda) row was activated: open its editor by \p islandIndex.
    void islandActivated(int islandIndex);
    //!< A reference row's context menu asked for the global where-used of \p symbolId.
    void whereUsedRequested(uint32_t symbolId);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onElementAdded(uint32_t id, eDocElementKind kind);
    void onElementChanged(uint32_t id, eDocElementKind kind);
    void onElementRemoved(uint32_t id, eDocElementKind kind);
    void onDocumentReloaded(void);
    void onCurrentRowChanged(void);
    void onItemActivated(QListWidgetItem* item);
    void onContextMenu(const QPoint& pos);

private:
    void scheduleRebuild(void);
    void rebuild(void);

    //!< The committed guard tree root, or nullptr.
    const SMGuardNode* guardTree(void) const;

    //!< Pre-order walk emitting one row per Call / reference / island node.
    void walk(const SMGuardNode& node, const QList<int>& path, bool parentIsCall, int& islandCounter);

    //!< The count of formals of \p call not mapped by an argument child.
    int unmappedCount(const SMGuardNode& call) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;         //!< The document facade.
    uint32_t            mTransId;       //!< The shown transition (0 = none).
    QListWidget*        mList;          //!< The outline rows.
    bool                mRebuildPending;//!< Coalesces deferred rebuilds.
    bool                mEmitting;      //!< True while emitting a selection (guards re-entry).
};

#endif  // LUSAN_VIEW_SM_SMGUARDCALLSOUTLINE_HPP
