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
#include <QPointer>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;
class StateMachineModel;

#include <QMetaObject>

/**
 * \class   SMValidationPanel
 * \brief   The validation results of every open document, in the output window's Validation
 *          tab: one tree root per document, its findings beneath it, so a message always says
 *          which document it belongs to. Rows come from the one document engine
 *          (\ref SMValidator, via each document's controller) already carrying severity,
 *          location and the explanation -- this view sorts and shows them, and knows nothing
 *          about which check produced a row.
 *
 *          Each row shows a severity icon and a severity word beside the message (severity is
 *          never conveyed by colour alone) and, on activation, asks its own document to reveal
 *          the offending element. F8 / Shift+F8 step through the findings across all documents.
 *          The tree rebuilds, deferred and coalesced, on every change.
 **/
class SMValidationPanel : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMValidationPanel(QWidget* parent = nullptr);

    //!< Convenience for a single-document host (tests): constructs and adds \p model.
    explicit SMValidationPanel(StateMachineModel& model, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Shows \p model's findings under a root labelled \p name, and keeps them live.
     *          Re-adding a model already shown only refreshes its label.
     * \param   owner   Emitted back with a navigation request so the host can reveal the
     *                  element in the right document; may be nullptr in a single-document host.
     **/
    void addDocument(StateMachineModel& model, const QString& name, QObject* owner = nullptr);

    //!< Drops a document's root and stops tracking it.
    void removeDocument(StateMachineModel& model);

    //!< The documents currently shown, in tree order.
    int documentCount() const;

    /**
     * \brief   How many findings deserve the user's attention right now: errors and warnings
     *          across every shown document. Advisory notes are excluded on purpose -- the tab
     *          badge must mean "something is wrong", not "something was said".
     **/
    int pendingCount() const;

    //!< Rebuilds the list now (tests; the panel otherwise coalesces on model changes).
    void refreshNow();

    //!< Selects and activates the next finding (F8), wrapping at the end. No-op when empty.
    void focusNextIssue();

    //!< Selects and activates the previous finding (Shift+F8), wrapping at the start.
    void focusPreviousIssue();

    //!< The findings table widget (tests).
    inline QTreeWidget* list() const;

signals:
    //!< A finding row was activated: navigate to the offending element on the owning page.
    void navigateRequested(uint32_t elementId, eDocElementKind kind);

    /**
     * \brief   As \ref navigateRequested, naming the document that owns the finding (the
     *          `owner` passed to \ref addDocument), so a multi-document host reveals it in the
     *          right window.
     **/
    void navigateRequestedIn(QObject* owner, uint32_t elementId, eDocElementKind kind);

    //!< Emitted whenever \ref pendingCount changes, so a tab can show `Validation (5)`.
    void pendingCountChanged(int count);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onItemActivated(QTreeWidgetItem* item, int column);

private:
    void buildUi();
    void scheduleRebuild();
    void rebuild();
    void step(int delta);

    /**
     * \brief   Puts the selected findings on the clipboard as whole rows, the columns separated
     *          by `|`, so a finding can be pasted into a report or an issue tracker as it reads
     *          in the panel.
     **/
    void copySelection() const;

    //!< Drops the documents whose model is gone, so a closed window leaves no findings behind.
    void purgeClosedDocuments();

    /**
     * \brief   One shown document: its facade, display name, live findings, and signal bindings.
     *          The facade and the owning window are held weakly -- a document window can be closed
     *          and destroyed while its findings are still listed, and a raw pointer would then be
     *          dereferenced by the next rebuild or by activating one of its rows.
     **/
    struct Source
    {
        QPointer<StateMachineModel> model;
        QPointer<QObject>           owner;
        QString                     name;
        QList<SMIssue>              issues;
        QList<QMetaObject::Connection> bindings;
    };

    //!< The index of \p model in mSources, or -1.
    int indexOf(const StateMachineModel* model) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QTreeWidget*        mList;          //!< The findings tree (one root per document).
    QLabel*             mSummary;       //!< The one-line count summary (`2 errors, 1 warning`).
    QList<Source>       mSources;       //!< The shown documents, in tree order.
    bool                mRebuildPending;//!< Coalesces deferred rebuilds.
    int                 mPending;       //!< Last published error+warning count.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline QTreeWidget* SMValidationPanel::list() const
{
    return mList;
}

#endif  // LUSAN_VIEW_SM_SMVALIDATIONPANEL_HPP
