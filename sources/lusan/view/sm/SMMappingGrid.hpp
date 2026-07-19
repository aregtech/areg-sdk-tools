#ifndef LUSAN_VIEW_SM_SMMAPPINGGRID_HPP
#define LUSAN_VIEW_SM_SMMAPPINGGRID_HPP
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
 *  \file        lusan/view/sm/SMMappingGrid.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard argument-mapping grid popover (v7 B6 / S9).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QFrame>

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QGridLayout;
class QLabel;
class StateMachineModel;
class SMOperandField;
enum class eDocElementKind;

/**
 * \class   SMMappingGrid
 * \brief   The two-way argument-mapping popover (S9): one row per declared parameter
 *          (`name : type <- source-field status`) over a call node of the guard tree, with
 *          the codegen-truth footer. A row edit rewrites the argument subtree through
 *          \ref SMGuardCommands::replaceArg (ONE undo step restores both the grid and the
 *          field text); a field edit while the grid is open refreshes the rows -- one truth
 *          (the tree), two views. Shown as a frameless tool window, not Qt::Popup, exactly
 *          so the field stays editable while the grid is open (the B6 two-way rule).
 **/
class SMMappingGrid : public QFrame
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMMappingGrid(StateMachineModel& model, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Opens the grid for the Call node at \p callPath of \p transitionId's
     *          committed guard tree, anchored below \p globalPos. Hides instead when the
     *          path does not address a call.
     **/
    void openFor(uint32_t transitionId, const QList<int>& callPath, const QPoint& globalPos);

    /**
     * \brief   Refreshes the rows from an uncommitted field text (the B6 two-way rule):
     *          re-parses \p fieldText and re-reads the first call of the same declared
     *          method. Keeps the last committed rows when the text does not parse to one.
     **/
    void refreshLive(const QString& fieldText);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    void keyPressEvent(QKeyEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onElementChanged(uint32_t id, eDocElementKind kind);
    void onElementRemoved(uint32_t id, eDocElementKind kind);

private:
    //!< Rebuilds the rows from the committed tree; false hides the grid (call vanished).
    bool rebuild();

    //!< Applies the committed text of row \p argIndex as a replace-arg command.
    void applyOperand(int argIndex, const QString& text);

    //!< The declared-type fit status text + severity for an argument expression text.
    QString statusFor(const QString& argText, const QString& paramType, int& severity) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&      mModel;     //!< The document facade.
    uint32_t                mTransId;   //!< The owning transition.
    QList<int>              mPath;      //!< The call node's child-index path.
    uint32_t                mMethodId;  //!< The called method's ID (relocates the call live).
    QLabel*                 mTitle;     //!< `map arguments -- name -- kind`.
    QGridLayout*            mGrid;      //!< The parameter rows.
    QLabel*                 mGenerated; //!< The codegen-truth footer (monospace).
    QList<SMOperandField*>  mFields;    //!< One operand field per parameter row.
    QList<QLabel*>          mStatus;    //!< One status label per parameter row.
    bool                    mApplying;  //!< Suppresses refresh loops while pushing a command.
};

#endif  // LUSAN_VIEW_SM_SMMAPPINGGRID_HPP
