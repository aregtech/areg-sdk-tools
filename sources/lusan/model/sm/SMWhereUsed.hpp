#ifndef LUSAN_MODEL_SM_SMWHEREUSED_HPP
#define LUSAN_MODEL_SM_SMWHEREUSED_HPP
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
 *  \file        lusan/model/sm/SMWhereUsed.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM where-used union: the single UI entry point that
 *               joins name/ID references with ID-bound guard references.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMReferences.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \namespace   SMWhereUsed
 * \brief   The complete where-used answer for a registry entry or state, and the one entry
 *          point every where-used UI (context menu, dialog, delete confirmation) calls.
 *
 *          It unions two reference mechanisms that deliberately live in two layers:
 *          - SMReferences (data layer): name-based fields plus the ID-based transition target;
 *          - SMGuardWhereUsed (model layer): the ID-bound guard trees for method / attribute / constant symbols.
 **/
namespace SMWhereUsed
{
    /**
     * \brief   Every place that references the element identified by \p target, \p name, and
     *          \p id -- name/ID references first (document order), guard references last for
     *          the guard-capable kinds (Condition, Attribute, Constant).
     **/
    QList<SMReferences::Use> collect(const StateMachineData& data, SMReferences::eTarget target, const QString& name, uint32_t id);
}

#endif  // LUSAN_MODEL_SM_SMWHEREUSED_HPP
