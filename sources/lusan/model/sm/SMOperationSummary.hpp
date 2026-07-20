#ifndef LUSAN_MODEL_SM_SMOPERATIONSUMMARY_HPP
#define LUSAN_MODEL_SM_SMOPERATIONSUMMARY_HPP
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
 *  \file        lusan/model/sm/SMOperationSummary.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operation one-line signature renderer (headless).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;
class SMOperationBase;
class SMTransitionEntry;

/**
 * \namespace   SMOperationSummary
 * \brief   Renders an operation as a compact, call-style one-liner for the operations list
 *          and the state-box body: `doWork(count, 5)`, `start blink (2000 ms, x3)`,
 *          `power = On`, `send evGo(x)`, `{ inline }`. Mapped arguments are inlined in the
 *          callee's parameter order; an unmapped parameter shows its default or its name.
 *          Headless (no widget/model dependency): shared by the editor and the canvas.
 **/
namespace SMOperationSummary
{
    /**
     * \brief   The one-line signature of \p op resolved against \p data (to look up the
     *          called action/event parameter order and defaults).
     **/
    QString text(const StateMachineData& data, const SMOperationBase& op);

    /**
     * \brief   The transition stimulus rendered as a method-like signature: `walk(count)` for a
     *          trigger or event (its declared parameters), and the bare name for a timer (no
     *          parameters). Used on the edge label, the state-box internal rows, and the
     *          transition properties.
     **/
    QString stimulusSignature(const StateMachineData& data, const SMTransitionEntry& transition);
}

#endif  // LUSAN_MODEL_SM_SMOPERATIONSUMMARY_HPP
