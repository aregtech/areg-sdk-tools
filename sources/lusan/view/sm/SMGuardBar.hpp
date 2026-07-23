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
 *  \brief       Lusan application, FSM guard bar: the Conditions tab content.
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
#include <QPointer>
#include <QSet>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QPlainTextEdit;
class SMAccordion;
class QToolButton;
class StateMachineModel;
class SMArgMapTable;
class SMArgSinkGuard;
class SMGuardCallsOutline;
class SMGuardDataPanel;
class SMGuardField;
class SMGuardPopout;
class SMGuardStatusLine;
class SMGuardHelpCard;
class SMSectionChrome;
class SMHoverCard;
class SMIslandEditor;
class SMTryStrip;
class SMGuardNode;
struct SMGuardSymbol;

/**
 * \class   SMGuardBar
 * \brief   The Conditions tab content: the header row (`Guard` label, clear `{x}`, help
 *          `(?)`), the guard field, the status line + chips, the transient fix bar, the
 *          island editor, and the structure lens; the Try-it strip stays a
 *          collapsed stub. Besides the status/fix/badge wiring, the bar owns the shared
 *          hover card, the mapping grid, and every ladder dialog -- so the lens
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

    //!< The island editor (tests).
    inline SMIslandEditor* islandEditor() const;

    //!< The Try-it strip (tests).
    inline SMTryStrip* tryStrip() const;

    //!< The accordion (tests).
    SMAccordion* accordion() const;

    //!< The Calls outline (tests).
    inline SMGuardCallsOutline* calls() const;

    //!< The Data catalog panel (tests).
    inline SMGuardDataPanel* dataPanel() const;

    //!< The shared Arguments table (tests).
    inline SMArgMapTable* args() const;

    //!< The open pop-out editor, or null (tests). Not inline: QPointer<SMGuardPopout> needs the
    //!< complete type, and callers of this header only forward-declare it.
    SMGuardPopout* popout() const;

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

    // ---- Caret-driven Arguments -------------------------------------------
    //!< The guard node addressed by \p path (child-index walk), or nullptr when unaddressable.
    const class SMGuardNode* nodeAtPath(const QList<int>& path) const;

    //!< Binds the shared Arguments table to the call at \p callPath (method \p methodId).
    void bindArgumentsTo(const QList<int>& callPath, uint32_t methodId);

    //!< Re-binds the Arguments table to the call the caret sits in, else the single/first call.
    void syncArgumentsToCaret();

    //!< Inserts a condition method's `#cond:name()` reference, commits, and shows Arguments.
    void insertCondition(const SMGuardSymbol& symbol);

    //!< Inserts ANY catalog symbol: a call routes to \ref insertCondition, else a plain reference.
    void insertSymbol(const SMGuardSymbol& symbol);

    // ---- pop-out editor ---------------------------------------------------
    //!< Opens the non-modal always-on-top pop-out over the same transition (Pop-out top strip).
    void openPopout();

    //!< Re-projects the Arguments table off a model pass (a foreign rename/add/remove, deferred).
    void scheduleCatalogRefresh();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;     //!< The document facade.
    uint32_t            mTransId;   //!< The shown transition (0 = none).
    SMGuardField*       mField;     //!< The editable guard surface.
    SMGuardStatusLine*  mStatus;    //!< The status line (the one place a guard verdict/error shows).
    SMIslandEditor*     mIsland;    //!< The island editor (hidden until an island opens).
    SMTryStrip*         mTry;       //!< The Try-it strip (kept hidden; not on the Conditions tab).
    SMHoverCard*        mHover;     //!< The shared hover card.
    SMGuardHelpCard*    mHelp;      //!< The help card (lazily shown).
    QToolButton*        mClear;     //!< The clear-guard button.
    QToolButton*        mHelpBtn;   //!< The help button.

    // ---- top strip + accordion --------------------------------------------
    QToolButton*        mInsertBtn; //!< Insert: opens the completer at the caret (all kinds).
    QToolButton*        mPopoutBtn; //!< Pop-out: opens the bigger editor.
    SMSectionChrome*    mChrome;    //!< The shared chrome: section jump buttons + compact toggle + accordion.
    QPlainTextEdit*     mGenCode;   //!< The generated C++ shown in the `Generated` section (read-only).
    QLabel*             mGenChips;  //!< The `uses handler:` list shown in the `Generated` section.
    SMGuardDataPanel*   mData;      //!< The `Data` section: the browsable symbol catalog.
    SMGuardCallsOutline* mCalls;    //!< The Conditions pickup list (double-click inserts a condition).
    SMArgMapTable*      mArgs;      //!< The single shared Arguments table.
    SMArgSinkGuard      mArgSink;   //!< The guard-side argument sink behind the table.
    bool                mDerivedPending;//!< Coalesces the deferred Arguments re-projection.
    QList<int>          mBoundCallPath; //!< The call path the Arguments table is currently bound to.
    bool                mBoundCallValid;//!< True when \ref mBoundCallPath addresses a live call.
    QPointer<SMGuardPopout> mPopout; //!< The open pop-out editor, or null.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline SMGuardField* SMGuardBar::field() const
{
    return mField;
}

inline SMIslandEditor* SMGuardBar::islandEditor() const
{
    return mIsland;
}

inline SMTryStrip* SMGuardBar::tryStrip() const
{
    return mTry;
}

inline SMGuardCallsOutline* SMGuardBar::calls() const
{
    return mCalls;
}

inline SMGuardDataPanel* SMGuardBar::dataPanel() const
{
    return mData;
}

inline SMArgMapTable* SMGuardBar::args() const
{
    return mArgs;
}

#endif  // LUSAN_VIEW_SM_SMGUARDBAR_HPP
