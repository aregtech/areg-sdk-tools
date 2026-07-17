#ifndef LUSAN_VIEW_SM_SMVALIDATIONPANEL_HPP
#define LUSAN_VIEW_SM_SMVALIDATIONPANEL_HPP
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
 *  \file        lusan/view/sm/SMValidationPanel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM document validation results panel (v7 B12 / S15).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QListWidget;
class QListWidgetItem;
class StateMachineModel;
enum class eDocElementKind;

/**
 * \class   SMValidationPanel
 * \brief   The document validation results (S15), hosted as a dockable panel of the Design
 *          page. Shows the guard entries of \ref SMGuardValidation -- drafts (ERR),
 *          shadowing (WARN), the raw-fragment audit (INFO), broken/stale references --
 *          each row colored by severity and navigating to its transition's Conditions tab
 *          on activation. Rebuilds (deferred, coalesced) on any document change.
 **/
class SMValidationPanel : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMValidationPanel(StateMachineModel& model, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Rebuilds the list now (tests; the panel otherwise coalesces on model changes).
    void refreshNow();

    //!< The findings list widget (tests).
    inline QListWidget* list() const;

signals:
    //!< A finding row was activated: navigate to the transition's Conditions tab.
    void navigateRequested(uint32_t transitionId);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onElementChanged(uint32_t id, eDocElementKind kind);
    void onElementRemoved(uint32_t id, eDocElementKind kind);
    void onDocumentReloaded();
    void onItemActivated(QListWidgetItem* item);

private:
    void scheduleRebuild();
    void rebuild();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;         //!< The document facade.
    QListWidget*        mList;          //!< The findings list.
    QLabel*             mSummary;       //!< The one-line count summary (`2 errors, 1 warning`).
    bool                mRebuildPending;//!< Coalesces deferred rebuilds.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline QListWidget* SMValidationPanel::list() const
{
    return mList;
}

#endif  // LUSAN_VIEW_SM_SMVALIDATIONPANEL_HPP
