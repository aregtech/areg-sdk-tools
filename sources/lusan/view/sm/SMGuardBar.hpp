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

/**
 * \class   SMGuardBar
 * \brief   The Conditions tab content (S1): the header row (`Guard` label, clear `{x}`, help
 *          `(?)`), the guard field, the status line + chips, and the transient fix bar; the
 *          Structure lens (U3) and Try-it strip (U4) are reserved as collapsed stubs. It wires
 *          the field's status / fix / badge signals to the status line, the fix bar and the tab
 *          badge, and routes fix-bar actions back to the field. Replaces SMConditionEditor as
 *          the Conditions tab behind the same SM-19 tab structure.
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

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;     //!< The document facade.
    SMGuardField*       mField;     //!< The editable guard surface.
    SMGuardStatusLine*  mStatus;    //!< The status line + chips.
    SMFixBar*           mFixBar;    //!< The quick-fix bar.
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

#endif  // LUSAN_VIEW_SM_SMGUARDBAR_HPP
