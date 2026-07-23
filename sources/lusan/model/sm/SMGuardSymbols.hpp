#ifndef LUSAN_MODEL_SM_SMGUARDSYMBOLS_HPP
#define LUSAN_MODEL_SM_SMGUARDSYMBOLS_HPP
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
 *  \file        lusan/model/sm/SMGuardSymbols.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard symbol resolution (name <-> document ID).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QString>
#include <QStringList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;
class SMMethodEntry;

/**
 * \namespace   SMGuardSymbols
 * \brief   The one place the guard tree resolves names to symbol IDs (for the parser) and
 *          symbol IDs back to names (for the renderer and the codegen preview). All lookups
 *          are over the document's own registries -- the closed world. Forward lookups
 *          answer "what does this name bind to"; reverse lookups answer "what is this ID's
 *          current name". Because guards store IDs, a rename changes only the declaration and
 *          every reverse lookup returns the new name with no guard edit.
 **/
namespace SMGuardSymbols
{
    // ---- Forward lookups (name -> ID), used by the parser -----------------

    //!< The attribute ID for \p name, or 0 when no attribute has that name.
    uint32_t attributeId(const StateMachineData& data, const QString& name);

    //!< The constant ID for \p name, or 0 when no constant has that name.
    uint32_t constantId(const StateMachineData& data, const QString& name);

    //!< The in-scope stimulus parameter ID for \p name in \p transitionId, or 0.
    uint32_t paramId(const StateMachineData& data, uint32_t transitionId, const QString& name);

    //!< The condition method (handler or lambda) named \p name, or nullptr when none.
    const SMMethodEntry* conditionMethod(const StateMachineData& data, const QString& name);

    //!< The in-scope stimulus parameter names of \p transitionId, in declared order.
    QStringList paramNames(const StateMachineData& data, uint32_t transitionId);

    //!< The declared type names of the in-scope stimulus parameters, parallel to paramNames.
    QStringList paramTypes(const StateMachineData& data, uint32_t transitionId);

    // ---- Reverse lookups (ID -> name), used by render / codegen -----------

    //!< The current name of the attribute \p id, or empty when it no longer exists.
    QString attributeName(const StateMachineData& data, uint32_t id);

    //!< The current name of the constant \p id, or empty when it no longer exists.
    QString constantName(const StateMachineData& data, uint32_t id);

    //!< The current name of the stimulus parameter \p id in \p transitionId, or empty.
    QString paramName(const StateMachineData& data, uint32_t transitionId, uint32_t id);

    //!< The condition method with the document ID \p id, or nullptr.
    const SMMethodEntry* method(const StateMachineData& data, uint32_t id);
}

#endif  // LUSAN_MODEL_SM_SMGUARDSYMBOLS_HPP
