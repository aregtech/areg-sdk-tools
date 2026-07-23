#ifndef LUSAN_VIEW_SM_SMGUARDDATAPANEL_HPP
#define LUSAN_VIEW_SM_SMGUARDDATAPANEL_HPP
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
 *  \file        lusan/view/sm/SMGuardDataPanel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard Data catalog (accordion section).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/view/sm/SMGuardCatalog.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class StateMachineModel;
class SMGuardNode;
enum class eDocElementKind;

/**
 * \class   SMGuardDataPanel
 * \brief   The accordion's `Data` section: the WHOLE reference universe a guard may
 *          use, browsable and insertable -- stimulus parameters (transition scope only),
 *          attributes, declared constants and condition methods -- grouped by kind, filtered
 *          by a search box, each row showing its glyph, `type name(...)` and how many times
 *          the CURRENT guard already references it (`used N`).
 *
 *          It answers the question the completer cannot: "what can I even write here?" The
 *          completer is recall-with-a-hint (you must type `#` first); this panel is pure
 *          recognition, which is why the design keeps both.
 *
 *          Double-click inserts the canonical `#kind:name` at the field's caret; right-click
 *          asks for the global where-used. The panel owns no state: it re-projects from the
 *          document on every model change, so a rename or a new attribute shows up
 *          without a manual refresh.
 *
 *          Width-safe: \ref minimumSizeHint width is 0 and the list scrolls
 *          horizontally inside itself, so no long signature can widen the Properties dock.
 **/
class SMGuardDataPanel : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMGuardDataPanel(StateMachineModel& model, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Points the panel at a transition (0 clears it) and rebuilds the catalog.
    void setTransition(uint32_t transitionId);

    //!< Re-projects the catalog and the `used N` counts from the document.
    void refresh(void);

    //!< The number of insertable symbol rows currently shown (excludes group headers; tests).
    int symbolRowCount(void) const;

    //!< Applies \p text as the search filter (empty shows everything).
    void setFilter(const QString& text);

    //!< Moves the keyboard focus into the search box (the `Data` toolbutton entry point).
    void focusSearch(void);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    //!< Width 0 so no signature length can widen the hosting dock.
    virtual QSize minimumSizeHint(void) const override;

signals:
    //!< A symbol row was activated: insert \p symbol's canonical mention at the caret.
    void insertRequested(const SMGuardSymbol& symbol);

    //!< A row's context menu asked for the global where-used of \p symbolId.
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

    //!< How many times the committed guard tree references \p symbolId (bound refs only).
    int useCount(uint32_t symbolId) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&      mModel;         //!< The document facade.
    uint32_t                mTransId;       //!< The shown transition (0 = none).
    QLineEdit*              mSearch;        //!< The filter box.
    QListWidget*            mList;          //!< The catalog rows.
    QList<SMGuardSymbol>    mSymbols;       //!< The projected universe, in display order.
    bool                    mRebuildPending;//!< Coalesces deferred rebuilds.
};

#endif  // LUSAN_VIEW_SM_SMGUARDDATAPANEL_HPP
