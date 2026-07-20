#ifndef LUSAN_VIEW_SM_SMGUARDBAR_HPP
#define LUSAN_VIEW_SM_SMGUARDBAR_HPP
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
 *  \file        lusan/view/sm/SMGuardBar.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard bar: the Conditions tab content (v7 B1 / S1).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/view/sm/SMArgSinkGuard.hpp"

#include <QHash>
#include <QList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QToolBox;
class QToolButton;
class StateMachineModel;
class SMArgMapTable;
class SMArgSinkGuard;
class SMGuardCallsOutline;
class SMGuardCatalogView;
class SMGuardField;
class SMGuardStatusLine;
class SMFixBar;
class SMGuardHelpCard;
class SMHoverCard;
class SMIslandEditor;
class SMStructureLens;
class SMTryStrip;
class SMGuardNode;

/**
 * \class   SMGuardBar
 * \brief   The Conditions tab content (S1): the header row (`Guard` label, clear `{x}`, help
 *          `(?)`), the guard field, the status line + chips, the transient fix bar, the
 *          island editor (S4), and the structure lens (S5); the Try-it strip (U4) stays a
 *          collapsed stub. Besides the U2 status/fix/badge wiring, the bar owns the shared
 *          hover card (S10), the mapping grid (S9), and every ladder dialog -- so the lens
 *          pills, the island editor's footer, the hover card's buttons and the fix bar all
 *          converge on ONE implementation of each flow.
 **/
class SMGuardBar : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMGuardBar(StateMachineModel& model, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Points the bar at a transition (0 clears it).
    void setTransition(uint32_t transitionId);

    //!< The guard field (tests / focus).
    inline SMGuardField* field() const;

    //!< The structure lens (tests).
    inline SMStructureLens* lens() const;

    //!< The island editor (tests).
    inline SMIslandEditor* islandEditor() const;

    //!< The Try-it strip (tests).
    inline SMTryStrip* tryStrip() const;

    //!< The accordion (tests).
    inline QToolBox* accordion() const;

    //!< The Calls outline (tests).
    inline SMGuardCallsOutline* calls() const;

    //!< The shared Arguments table (tests).
    inline SMArgMapTable* args() const;

    //!< The Data catalog view (tests).
    inline SMGuardCatalogView* dataCatalog() const;

signals:
    //!< The Conditions tab badge state changed (draft => `*`, warnings => warn glyph).
    void badgeChanged(bool isDraft, bool hasWarnings);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onStatusUpdated(int severity, const QString& verdict, const QString& preview, const QStringList& chips);
    void onClearClicked();
    void onHelpClicked();

private:
    //!< The current transition's committed guard tree root, or nullptr.
    const class SMGuardNode* guardTree() const;

    //!< The pre-order path of the \p islandIndex-th Lambda node, empty when absent.
    QList<int> nthIslandPath(int islandIndex) const;

    //!< The pre-order path of the first Call node of \p methodId (or the first call at all).
    QList<int> firstCallPath(uint32_t methodId) const;

    //!< The ladder `Name it...` / island-to-handler flow (dialog + one composite command).
    void runNameIsland(const QList<int>& islandPath, const QString& body, SMMethodEntry::eImplement implement);

    //!< The ladder `Move to handler...` flow for a declared lambda condition.
    void runMoveToHandler(uint32_t methodId);

    //!< The ladder `Adopt body...` flow for a declared handler condition.
    void runAdoptBody(uint32_t methodId);

    //!< Opens the mapping grid for the first call of \p methodId (0 = any call).
    void openGridForCall(uint32_t methodId);

    //!< The `where used` popup: pick a guard, select its transition.
    void showWhereUsed(uint32_t symbolId);

    // ---- accordion + Arguments (SM-21-02 phase 2) -------------------------
    //!< Binds the shared Arguments table to the selected call and applies D-ACCORDION auto-open.
    void onCallSelected(const QList<int>& callPath, uint32_t methodId, int unmappedCount);

    //!< Selects \p callPath in the Calls outline and opens the Arguments section on it.
    void jumpToCall(const QList<int>& callPath);

    //!< Toggles the Data section (Data>> top-strip button).
    void toggleDataSection();

    //!< The generated-code preview dialog (Preview top-strip button).
    void showPreviewDialog();

    // ---- use-count + warnings (SM-21-04 phase 4) --------------------------
    //!< Recomputes the catalog `used-N` counts and the warning channel from the committed guard.
    void refreshDerived();

    //!< Re-enumerates the Data catalog symbols (a foreign rename/add/remove changed them).
    void scheduleCatalogRefresh();

    //!< Rebuilds the warning channel (unmapped-argument entries) from the committed guard.
    void refreshWarnings();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;     //!< The document facade.
    uint32_t            mTransId;   //!< The shown transition (0 = none).
    SMGuardField*       mField;     //!< The editable guard surface.
    SMGuardStatusLine*  mStatus;    //!< The status line + chips.
    SMFixBar*           mFixBar;    //!< The quick-fix bar (field diagnostics).
    SMFixBar*           mWarnBar;   //!< The one advisory warning channel (unmapped-argument).
    SMIslandEditor*     mIsland;    //!< The island editor (S4, hidden until an island opens).
    SMStructureLens*    mLens;      //!< The structure lens (S5).
    SMTryStrip*         mTry;       //!< The Try-it strip (S6).
    SMHoverCard*        mHover;     //!< The shared hover card (S10).
    SMGuardHelpCard*    mHelp;      //!< The help card (lazily shown).
    QToolButton*        mClear;     //!< The clear-guard button.
    QToolButton*        mHelpBtn;   //!< The help button.

    // ---- top strip + accordion (SM-21-02 / SM-21-04) ----------------------
    QToolButton*        mInsertBtn; //!< Insert: opens the completer at the caret (all kinds).
    QToolButton*        mPreviewBtn;//!< Preview: opens the generated-code dialog.
    QToolButton*        mDataBtn;   //!< Data>>: toggles the Data section.
    QToolButton*        mPopoutBtn; //!< Pop-out: disabled placeholder (wired in SM-21-05).
    QToolBox*           mAccordion; //!< The Calls / Arguments / Data accordion (one open at a time).
    SMGuardCallsOutline* mCalls;    //!< The Calls outline (drives the Arguments table).
    SMArgMapTable*      mArgs;      //!< The single shared Arguments table.
    SMArgSinkGuard      mArgSink;   //!< The guard-side argument sink behind the table.
    SMGuardCatalogView* mData;      //!< The Data catalog (model/view).
    int                 mLastSection;//!< The last-open accordion section (D-ACCORDION, per tab).
    bool                mDerivedPending;//!< Coalesces the deferred catalog re-enumeration.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline SMGuardField* SMGuardBar::field() const
{
    return mField;
}

inline SMStructureLens* SMGuardBar::lens() const
{
    return mLens;
}

inline SMIslandEditor* SMGuardBar::islandEditor() const
{
    return mIsland;
}

inline SMTryStrip* SMGuardBar::tryStrip() const
{
    return mTry;
}

inline QToolBox* SMGuardBar::accordion() const
{
    return mAccordion;
}

inline SMGuardCallsOutline* SMGuardBar::calls() const
{
    return mCalls;
}

inline SMArgMapTable* SMGuardBar::args() const
{
    return mArgs;
}

inline SMGuardCatalogView* SMGuardBar::dataCatalog() const
{
    return mData;
}

#endif  // LUSAN_VIEW_SM_SMGUARDBAR_HPP
