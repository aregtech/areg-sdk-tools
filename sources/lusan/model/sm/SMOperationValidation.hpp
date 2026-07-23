#ifndef LUSAN_MODEL_SM_SMOPERATIONVALIDATION_HPP
#define LUSAN_MODEL_SM_SMOPERATIONVALIDATION_HPP
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
 *  \file        lusan/model/sm/SMOperationValidation.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operation argument-mapping validation (headless).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;
class SMOperationList;

/**
 * \class   SMOperationValidation
 * \brief   Headless checker for the argument mapping of a state's or transition's operations.
 *          An `ActionCall` / `EventSend` whose callee still exists but that leaves a required
 *          formal unmapped, or that keeps a stored argument for a formal since removed on the
 *          Methods page (an orphan), is an incomplete mapping. The canvas keys its warning
 *          glyph off the worst severity so a developer sees which transitions and states a
 *          method edit broke, without opening each Properties panel.
 *
 *          It is the operation-side counterpart of `SMGuardValidation`; the edge label and the
 *          state box tint combine both severities.
 **/
class SMOperationValidation
{
//////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eSeverity
     * \brief   The worst mapping state of an operation list, ordered for worst-of comparisons.
     **/
    enum class eSeverity
    {
          Ok        //!< Every action/event argument is mapped (or defaulted).
        , Warn      //!< Reserved for softer findings (kept for ordering).
        , Error     //!< A required argument is unmapped, or a stored argument is orphaned.
    };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The worst mapping severity across every `ActionCall` / `EventSend` in \p list.
     *          A callee that does not resolve is skipped (it is not a mapping fault).
     **/
    static eSeverity listSeverity(const StateMachineData& data, const SMOperationList& list);

    //!< The worst mapping severity of a transition's operation list (0 -> Ok when not found).
    static eSeverity transitionSeverity(const StateMachineData& data, uint32_t transitionId);

    //!< The worst mapping severity of a state's entry AND exit lists (0 -> Ok when not found).
    static eSeverity stateSeverity(const StateMachineData& data, uint32_t stateId);
};

#endif  // LUSAN_MODEL_SM_SMOPERATIONVALIDATION_HPP
