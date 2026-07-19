#ifndef LUSAN_VIEW_SM_SMISLANDEDITOR_HPP
#define LUSAN_VIEW_SM_SMISLANDEDITOR_HPP
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
 *  \file        lusan/view/sm/SMIslandEditor.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard island editor (v7 B8 / S4): the anonymous
 *               lambda body editor shown below the guard field.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QFrame>

#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QPushButton;
class SMCodeEditor;
class StateMachineModel;

/**
 * \class   SMIslandEditor
 * \brief   The island editor (S4): header (`{} lambda -- must return bool`), the in-scope
 *          line, an \ref SMCodeEditor body with the machine's completion words, shallow
 *          warnings only (brace balance, missing `return` -- never parsed, never blocking,
 *          D4 scope), and the ladder footer (`Name it...` / `Move to handler...` / `Close`).
 *          The editor talks to the guard field only through signals carrying the token
 *          position; the field owns the token, the tree owns the truth.
 **/
class SMIslandEditor : public QFrame
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMIslandEditor(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Points the editor at island \p islandIndex (the nth island token of the
     *          field, in text order -- positions shift on reflow, the index does not) with
     *          the current \p body and shows it. The index also names the widget
     *          (`smIsland_<n>`).
     **/
    void openFor(StateMachineModel& model, uint32_t transitionId, int islandIndex, const QString& body);

    //!< The edited island's index among the field's islands.
    inline int islandIndex() const;

    //!< The current body text.
    QString body() const;

signals:
    //!< The body was applied to the token (Ctrl+Enter).
    void bodyCommitted(int islandIndex, const QString& body);
    //!< The Close button: fold back to the `{...}` token.
    void closeRequested(int islandIndex, const QString& body);
    //!< The `Name it...` ladder step.
    void nameRequested(int islandIndex, const QString& body);
    //!< The `Move to handler...` ladder step.
    void moveToHandlerRequested(int islandIndex, const QString& body);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Refreshes the shallow warnings (brace balance, missing `return`).
    void updateWarnings();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QLabel*         mHeader;    //!< `{} lambda` + the contract text.
    QLabel*         mScope;     //!< The in-scope line.
    SMCodeEditor*   mBody;      //!< The verbatim body editor.
    QLabel*         mWarn;      //!< The shallow-warning line (hidden when clean).
    QPushButton*    mNameBtn;   //!< `Name it...`.
    QPushButton*    mMoveBtn;   //!< `Move to handler...`.
    QPushButton*    mCloseBtn;  //!< `Close` (folds back to the token).
    int             mIsland;    //!< The edited island's index among the field's islands.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline int SMIslandEditor::islandIndex() const
{
    return mIsland;
}

#endif  // LUSAN_VIEW_SM_SMISLANDEDITOR_HPP
