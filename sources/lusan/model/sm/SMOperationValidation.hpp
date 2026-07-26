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
#include "lusan/model/common/DocIssue.hpp"

#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class MethodBase;
class SMArgumentEntry;
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
     * \brief   The mapping findings of one operation list, each with the message that says
     *          which formal is unmapped or which stored argument is orphaned. This is the
     *          SINGLE implementation of the argument-mapping check: the document-wide run
     *          (\ref SMValidator) and the canvas per-element queries below both call it, so
     *          a glyph on an edge and a row in the results list can never disagree.
     *          A callee that does not resolve is skipped -- that is a reference fault, which
     *          the reference rules report, not a mapping fault.
     * \param   ownerId     The element the findings navigate to.
     * \param   kind        The owning element's kind (page routing).
     * \param   location    The owner in the document's own words, for the results list.
     **/
    static QList<DocIssue> listIssues( const StateMachineData& data
                                     , const SMOperationList& list
                                     , uint32_t ownerId
                                     , eDocElementKind kind
                                     , const QString& location);

    /**
     * \brief   The mapping findings of one argument list against one callee's formals: an
     *          argument naming no formal (an orphan), and a formal without a default that no
     *          argument binds. This is the smallest shared unit -- an operation's call and a
     *          guard's parameterized condition call are both checked by it, so "is this call
     *          mapped" has exactly one answer in the code base.
     **/
    static QList<DocIssue> argumentIssues( const MethodBase& callee
                                         , const QList<SMArgumentEntry>& args
                                         , uint32_t ownerId
                                         , eDocElementKind kind
                                         , const QString& location);

    /**
     * \brief   The worst mapping severity of a transition's operation list. False when the
     *          mapping is clean, matching \ref SMGuardValidation::worstSeverity so the canvas
     *          asks both checkers the same way.
     **/
    static bool worstForTransition(const StateMachineData& data, uint32_t transitionId, DocIssue::eSeverity& worst);

    //!< The worst mapping severity of a state's entry, do AND exit lists.
    static bool worstForState(const StateMachineData& data, uint32_t stateId, DocIssue::eSeverity& worst);
};

#endif  // LUSAN_MODEL_SM_SMOPERATIONVALIDATION_HPP
