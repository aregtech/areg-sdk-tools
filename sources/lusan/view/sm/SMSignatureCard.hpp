#ifndef LUSAN_VIEW_SM_SMSIGNATURECARD_HPP
#define LUSAN_VIEW_SM_SMSIGNATURECARD_HPP
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
 *  \file        lusan/view/sm/SMSignatureCard.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard call signature card (v7 B5).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QFrame>

#include "lusan/view/sm/SMGuardCatalog.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;

/**
 * \class   SMSignatureCard
 * \brief   The frameless signature card shown under the caret while mapping a call's arguments
 *          (B5): the full declared signature with the active parameter underlined, and a second
 *          line restating that parameter. It follows the caret and never takes focus.
 **/
class SMSignatureCard : public QFrame
{
    Q_OBJECT

public:
    explicit SMSignatureCard(QWidget* parent = nullptr);

    //!< Shows the \p call signature with parameter \p activeParam underlined, at \p globalPos.
    void showFor(const SMGuardSymbol& call, int activeParam, const QPoint& globalPos);

private:
    QLabel* mSignature; //!< The signature line (active parameter underlined).
    QLabel* mActive;    //!< The restated active-parameter line.
};

#endif  // LUSAN_VIEW_SM_SMSIGNATURECARD_HPP
