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
 *  \brief       Lusan application, FSM guard hover cards: symbol and
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
class StateMachineData;
class StateMachineModel;

/**
 * \class   SMHoverCard
 * \brief   A frameless card opened by a click on a chip's badge or a call pill, staying alive
 *          while the mouse is over it so its two buttons (`where used` / `map args`) are really
 *          clickable. The symbol face shows owner, signature, declared-where and the generated
 *          form; the call face shows the read-only mapping rows plus the generated line. Merely
 *          POINTING at a symbol is answered by a tooltip instead, rendered from \ref symbolTip so
 *          the two surfaces cannot describe the same element differently.
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
    /**
     * \brief   The same explanation the symbol face shows, as tooltip rich text: owner badge,
     *          signature, where it is declared, the generated call, and any finding this element
     *          is guilty of. The card and the field's tooltip must never disagree about an
     *          element, so both are rendered from here; the card adds only the two buttons, which
     *          is the one thing a tooltip cannot carry.
     * \return  The rich text, or an empty string when there is nothing to say.
     **/
    static QString symbolTip(StateMachineModel& model, uint32_t transitionId, const SMGuardSymbol& symbol);

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

    //!< Adds the leading `<badge>  --  <noun>` line, in the owner hue.
    QLabel* addBadgeLine(NEGuardStyle::eOwner owner, const QString& noun);

    /**
     * \brief   Adds one line per guard finding that is about THIS declaration in THIS transition's
     *          guard, colored by severity. Nothing is added for a sound element -- silence is the
     *          "no complaints" state, and a green "ok" line on every hover would train the eye to
     *          skip the line that matters.
     **/
    void addValidationLines(const StateMachineData& data, uint32_t transitionId, uint32_t symbolId);

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
