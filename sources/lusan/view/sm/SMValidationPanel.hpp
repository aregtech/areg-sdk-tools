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
 *  \brief       Lusan application, FSM document validation results panel.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/model/sm/SMValidator.hpp"

#include <QList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QListWidget;
class QListWidgetItem;
class StateMachineModel;

/**
 * \class   SMValidationPanel
 * \brief   The document validation results, hosted as a dockable panel of the Design page
 *          (bottom, tabbed with the output window). It renders the headless engine's findings
 *          (\ref SMValidator, via the facade's controller) together with the guard-specific
 *          entries of \ref SMGuardValidation, one live list ordered worst-severity first. Each
 *          row shows a severity icon and a severity word alongside the message (severity is
 *          never conveyed by color alone) and, on activation, asks the Design page to select
 *          the offending element -- switching page or level as needed. F8 / Shift+F8 step
 *          through the findings. The list rebuilds, deferred and coalesced, on every change.
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

    //!< Selects and activates the next finding (F8), wrapping at the end. No-op when empty.
    void focusNextIssue();

    //!< Selects and activates the previous finding (Shift+F8), wrapping at the start.
    void focusPreviousIssue();

    //!< The findings list widget (tests).
    inline QListWidget* list() const;

signals:
    //!< A finding row was activated: navigate to the offending element on the owning page.
    void navigateRequested(uint32_t elementId, eDocElementKind kind);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onModelChanged();
    void onEngineIssues(const QList<SMIssue>& issues);
    void onItemActivated(QListWidgetItem* item);

private:
    void scheduleRebuild();
    void rebuild();
    void step(int delta);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;         //!< The document facade.
    QListWidget*        mList;          //!< The findings list.
    QLabel*             mSummary;       //!< The one-line count summary (`2 errors, 1 warning`).
    QList<SMIssue>      mEngineIssues;  //!< The latest structural/reference/type findings.
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
