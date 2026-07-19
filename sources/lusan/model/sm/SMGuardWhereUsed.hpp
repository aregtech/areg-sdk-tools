#ifndef LUSAN_MODEL_SM_SMGUARDWHEREUSED_HPP
#define LUSAN_MODEL_SM_SMGUARDWHEREUSED_HPP
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
 *  \file        lusan/model/sm/SMGuardWhereUsed.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard where-used: which guards reference a symbol.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \namespace   SMGuardWhereUsed
 * \brief   Answers "which transition guards reference this declared symbol" over the
 *          ID-bound guard trees (Call / Attr / Const nodes only -- deterministic, no text
 *          scan). The Methods page uses it for the `used by N guards` navigation and the
 *          delete refusal; the hover card uses it for `where used`.
 **/
namespace SMGuardWhereUsed
{
    /**
     * \struct  Use
     * \brief   One guard that references the queried symbol.
     **/
    struct Use
    {
        uint32_t    transitionId;   //!< The owning transition's document ID.
        QString     location;       //!< Human-readable place: `State : stimulus -> target`.
    };

    //!< Every guard (ok tree or draft last-good tree) referencing the symbol \p symbolId.
    QList<Use> symbolUses(const StateMachineData& data, uint32_t symbolId);

    //!< The number of guards referencing \p symbolId.
    int useCount(const StateMachineData& data, uint32_t symbolId);
}

#endif  // LUSAN_MODEL_SM_SMGUARDWHEREUSED_HPP
