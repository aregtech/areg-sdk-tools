#ifndef LUSAN_VIEW_SM_SMGUARDHELPCARD_HPP
#define LUSAN_VIEW_SM_SMGUARDHELPCARD_HPP
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
 *  \file        lusan/view/sm/SMGuardHelpCard.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard help card.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QFrame>

/**
 * \class   SMGuardHelpCard
 * \brief   The one-screen static help card, opened by the `(?)` button or F1: what a condition
 *          decides, what a guard can use (the owner glyphs), the `you write -> it runs` mapping
 *          table including the D7 `IsCalmHours(x) -> mIsCalmHours(x)` row (named lambdas generate
 *          as a `std::function` member), the gestures and the operators. Since pointing at the
 *          editor explains only the symbol under the pointer, this card is where the editor
 *          itself is explained, so it has to be complete. No pages, no wizard. Shown as a
 *          frameless popover.
 **/
class SMGuardHelpCard : public QFrame
{
    Q_OBJECT

public:
    explicit SMGuardHelpCard(QWidget* parent = nullptr);

    //!< Shows the card below \p anchor, opening toward the usable side and clamping to screen.
    void popupAt(const QWidget& anchor);

    //!< The size that shows the whole card, which is NOT what the scroll area would ask for.
    QSize sizeHint() const override;

private:
    void buildUi();

    QWidget*    mContent;   //!< The scrolled content, whose hint is the card's real size.
};

#endif  // LUSAN_VIEW_SM_SMGUARDHELPCARD_HPP
