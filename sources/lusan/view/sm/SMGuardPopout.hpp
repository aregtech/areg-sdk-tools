#ifndef LUSAN_VIEW_SM_SMGUARDPOPOUT_HPP
#define LUSAN_VIEW_SM_SMGUARDPOPOUT_HPP
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
 *  \file        lusan/view/sm/SMGuardPopout.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard pop-out: a bigger multiline guard editor (SM-21-05).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QCloseEvent;
class StateMachineModel;
class SMGuardField;
class SMGuardStatusLine;
class SMIslandEditor;

/**
 * \class   SMGuardPopout
 * \brief   The "work in a bigger editor" pop-out (design 7, spec 10): a non-modal,
 *          always-on-top (`Qt::Tool | Qt::WindowStaysOnTopHint`) window whose body is its OWN
 *          \ref SMGuardField bound to the SAME transition as the base bar. It shares the model,
 *          parser, commands, catalog and style -- but NOT a QTextDocument (each field owns its
 *          document, so the two highlighters and slot cursors never collide).
 *
 *          Seeded from the base field's committable text: the bar commits the base first, then
 *          this field reflows from the model, so the pop-out opens showing exactly what the base
 *          showed (chips and islands folded identically). OK commits through the standard
 *          text -> \ref SMGuardParser::parseToGuard -> \ref SMGuardCommands path as ONE undoable
 *          command (the base then reflows from the model); Cancel or closing the window discards.
 *          The field's auto-commit-on-focus-out is disabled so Cancel cannot slip a commit in.
 **/
class SMGuardPopout : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMGuardPopout(StateMachineModel& model, uint32_t transitionId, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< The pop-out's own guard field (tests / focus).
    inline SMGuardField* field() const;

signals:
    //!< The pop-out is closing (OK, Cancel, or the window close box); the bar restores the base.
    void closed();

    //!< An island wants the `Name it...` / `Move to handler...` ladder step; the bar (which owns
    //!< the flow) runs it over the shared model. \p moveToHandler picks the target implement kind.
    void nameIslandRequested(int islandIndex, const QString& body, bool moveToHandler);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    void closeEvent(QCloseEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    //!< OK: commit the field through the standard path (one undo step), then close.
    void onCommit();

    //!< Mirrors the base bar's status line off the field's status signal.
    void onStatusUpdated(int severity, const QString& verdict, const QString& preview, const QStringList& chips);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;     //!< The shared document facade.
    uint32_t            mTransId;   //!< The edited transition (shared with the base bar).
    SMGuardField*       mField;     //!< The pop-out's own guard surface (its own document).
    SMGuardStatusLine*  mStatus;    //!< The one-line status / generated preview under the field.
    SMIslandEditor*     mIsland;    //!< The island editor (S4), hidden until an island opens.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline SMGuardField* SMGuardPopout::field() const
{
    return mField;
}

#endif  // LUSAN_VIEW_SM_SMGUARDPOPOUT_HPP
