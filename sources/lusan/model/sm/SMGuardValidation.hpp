#ifndef LUSAN_MODEL_SM_SMGUARDVALIDATION_HPP
#define LUSAN_MODEL_SM_SMGUARDVALIDATION_HPP
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
 *  \file        lusan/model/sm/SMGuardValidation.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard document validation.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/model/common/DocIssue.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;
class SMGuardNode;

/**
 * \class   SMGuardValidation
 * \brief   The guard rows of the document validation results -- the complete
 *          honesty surface of a document's guards: draft guards (ERR, the
 *          generator refuses them), shadowing (WARN), the raw-C++ fragment audit (INFO,
 *          the complete list of verbatim fragments), and broken/stale references (ERR;
 *          a stale stimulus parameter that matches the new stimulus by name and type is
 *          the re-bind case, reported as INFO). Validation covers `.fsml` content
 *          only -- raw C++ bodies are never parsed. View-free and headless-testable.
 *
 *          Every predicate in the format is checked here, which since L3 means a state
 *          `DoList`'s `<Until>` stop condition as well as a transition's `<Guard>`: they are
 *          the same value, so a stale reference in one is the same fault as in the other, and
 *          one checker is what keeps them from being reported differently.
 **/
class SMGuardValidation
{
public:
    /**
     * \brief   Guard findings are document findings: one severity ladder for every checker
     *          (\ref DocIssue), so nothing has to translate between vocabularies.
     *          Info = the raw-fragment audit / the re-bind notice; Warning = shadowing;
     *          Error = drafts and broken references.
     **/
    using eSeverity = DocIssue::eSeverity;

    /**
     * \enum    eKind
     * \brief   The finding kind.
     **/
    enum class eKind
    {
          Draft         //!< The guard is a draft -- generation refuses.
        , Shadowing     //!< A referenced stimulus parameter hides an attribute/constant.
        , RawFragment   //!< One verbatim raw-C++ fragment (the audit).
        , BrokenRef     //!< A referenced symbol no longer exists / left the scope.
        , ParamRebind   //!< A stale parameter matches the new stimulus (info).
    };

    /**
     * \struct  Finding
     * \brief   One guard validation entry; navigates to the element that owns the predicate --
     *          the transition, or the state whose `DoList` carries the stop condition.
     **/
    struct Finding
    {
        eSeverity   severity;       //!< The severity.
        eKind       kind;           //!< The finding kind.
        SMGuardRef  target;         //!< The checked guard, and so the navigation target.
        QString     location;       //!< `State : stimulus -> target`, or `State : do/` (as where-used).
        QString     message;        //!< The human-readable finding text.
        /**
         * \brief   The declaration the finding is about, or 0 when it is about the guard as a whole
         *          (a draft, a raw fragment). It is what lets a surface showing one symbol -- the
         *          hover card over a chip, ask "is THIS element sound?" instead of dumping every
         *          finding of the transition. The results list ignores it.
         **/
        uint32_t    symbolId;
    };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Why a finding of this kind is a finding, and what resolves it (the results list detail).
    static QString describe(eKind kind);

    //!< Every guard finding of the document, in state/transition order (Do stop conditions included).
    static QList<Finding> validate(const StateMachineData& data);

    //!< The findings of one transition's guard.
    static QList<Finding> validateTransition(const StateMachineData& data, uint32_t transitionId);

    //!< The findings of one state's `DoList` stop condition.
    static QList<Finding> validateDoActivity(const StateMachineData& data, uint32_t stateId);

    /**
     * \brief   The worst severity of one transition's findings; false when the guard is
     *          clean (the canvas edge label and tab badge key off this).
     **/
    static bool worstSeverity(const StateMachineData& data, uint32_t transitionId, eSeverity& worst);
};

#endif  // LUSAN_MODEL_SM_SMGUARDVALIDATION_HPP
