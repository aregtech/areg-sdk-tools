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

#include <QList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QToolButton;
class StateMachineModel;
class SMGuardField;
class SMGuardStatusLine;
class SMFixBar;
class SMGuardHelpCard;
class SMHoverCard;
class SMIslandEditor;
class SMMappingGrid;
class SMStructureLens;

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

    //!< The mapping grid (tests).
    inline SMMappingGrid* grid() const;

    //!< The island editor (tests).
    inline SMIslandEditor* islandEditor() const;

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

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;     //!< The document facade.
    uint32_t            mTransId;   //!< The shown transition (0 = none).
    SMGuardField*       mField;     //!< The editable guard surface.
    SMGuardStatusLine*  mStatus;    //!< The status line + chips.
    SMFixBar*           mFixBar;    //!< The quick-fix bar.
    SMIslandEditor*     mIsland;    //!< The island editor (S4, hidden until an island opens).
    SMStructureLens*    mLens;      //!< The structure lens (S5).
    SMMappingGrid*      mGrid;      //!< The mapping grid popover (S9).
    SMHoverCard*        mHover;     //!< The shared hover card (S10).
    SMGuardHelpCard*    mHelp;      //!< The help card (lazily shown).
    QToolButton*        mClear;     //!< The clear-guard button.
    QToolButton*        mHelpBtn;   //!< The help button.
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

inline SMMappingGrid* SMGuardBar::grid() const
{
    return mGrid;
}

inline SMIslandEditor* SMGuardBar::islandEditor() const
{
    return mIsland;
}

#endif  // LUSAN_VIEW_SM_SMGUARDBAR_HPP
