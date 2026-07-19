#ifndef LUSAN_VIEW_SM_SMHOVERCARD_HPP
#define LUSAN_VIEW_SM_SMHOVERCARD_HPP
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
 *  \file        lusan/view/sm/SMHoverCard.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard hover cards (v7 B10 / S10): symbol and
 *               call-pill hovers with live buttons.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QFrame>

#include "lusan/view/sm/SMGuardCatalog.hpp"

#include <QList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QPushButton;
class QTimer;
class QVBoxLayout;
class StateMachineModel;

/**
 * \class   SMHoverCard
 * \brief   The hover card (S10): a frameless card shown 300 ms after a symbol or call-pill
 *          hover (the delay is the caller's), staying alive while the mouse is over it so
 *          its two buttons (`where used` / `map args`) are really clickable. The symbol
 *          face shows owner, signature, declared-where and the generated form; the call
 *          face shows the read-only mapping rows plus the generated line.
 **/
class SMHoverCard : public QFrame
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMHoverCard(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Shows the symbol face for \p symbol at \p globalPos.
    void showSymbol(StateMachineModel& model, uint32_t transitionId, const SMGuardSymbol& symbol, const QPoint& globalPos);

    //!< Shows the call face (read-only grid) for the call at \p callPath at \p globalPos.
    void showCall(StateMachineModel& model, uint32_t transitionId, const QList<int>& callPath, const QPoint& globalPos);

    //!< Hides after a grace delay unless the mouse has moved onto the card.
    void scheduleHide();

    //!< Cancels a scheduled hide (the mouse is back on the source or the card).
    void cancelHide();

signals:
    //!< The `where used` button of the symbol face.
    void whereUsedRequested(uint32_t symbolId);
    //!< The `map args` button of the symbol face (call symbols only).
    void mapArgsRequested(uint32_t symbolId);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Clears the content rows, keeping the button row.
    void clearContent();

    //!< Adds one content label; \p monospace selects the code font.
    QLabel* addLine(const QString& text, bool monospace = false);

    void placeAt(const QPoint& globalPos);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QVBoxLayout*    mContent;       //!< The content rows (rebuilt per show).
    QPushButton*    mWhereUsed;     //!< The `where used` button.
    QPushButton*    mMapArgs;       //!< The `map args` button.
    QWidget*        mButtonRow;     //!< The button row (hidden on the call face).
    QTimer*         mHideTimer;     //!< The leave-grace timer.
    uint32_t        mSymbolId;      //!< The shown symbol's ID (button payload).
};

#endif  // LUSAN_VIEW_SM_SMHOVERCARD_HPP
