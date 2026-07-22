#ifndef LUSAN_MODEL_SM_SMGUARDCODEGENPREVIEW_HPP
#define LUSAN_MODEL_SM_SMGUARDCODEGENPREVIEW_HPP
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
 *  \file        lusan/model/sm/SMGuardCodegenPreview.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard generated-C++ preview (display string only).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;
class SMGuardNode;
class SMGuard;

/**
 * \class   SMGuardCodegenPreview
 * \brief   Builds the generated-C++ display string of a guard tree -- the status line, the
 *          grid footer and hovers show it, byte-exact with what the external generator must
 *          emit. This is a DISPLAY string only: Lusan emits no C++ files; the
 *          generator is a separate project. This one file is the single place the
 *          receiver/getter/constant spellings live: a handler call is
 *          `handler().Name(args)`, an attribute is `Name()`, a constant is
 *          `<FsmData>::NAME`, a named-lambda call is `m<Name>(args)` (a `std::function`
 *          member), an anonymous lambda is an IIFE, a raw fragment is verbatim.
 **/
class SMGuardCodegenPreview
{
public:
    //!< The `<FsmData>` qualifier placeholder used for constants (the generator substitutes it).
    static const char* const FSM_DATA_QUALIFIER;
    //!< The handler accessor of the generated FSM class.
    static const char* const HANDLER_ACCESSOR;
    //!< The named-lambda `std::function` member prefix.
    static const char* const LAMBDA_MEMBER_PREFIX;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The generated boolean expression for a node sub-tree (no `if (...)` wrapper),
     *          e.g. `handler().HasWaiting(count) || count >= <FsmData>::MIN_WAITING`.
     **/
    static QString expression(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node);

    /**
     * \brief   The generated `if (...)` line for a whole guard: the expression wrapped in
     *          `if (...)` (ok), or an empty string (empty / draft -- the generator refuses a
     *          draft, so there is no truthful preview to show).
     **/
    static QString ifStatement(const StateMachineData& data, uint32_t transitionId, const SMGuard& guard);
};

#endif  // LUSAN_MODEL_SM_SMGUARDCODEGENPREVIEW_HPP
