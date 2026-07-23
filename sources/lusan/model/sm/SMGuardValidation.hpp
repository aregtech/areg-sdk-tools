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
 **/
class SMGuardValidation
{
public:
    /**
     * \enum    eSeverity
     * \brief   The severity of one finding, ordered for worst-of comparisons.
     **/
    enum class eSeverity
    {
          Info      //!< The raw-fragment audit / the re-bind notice.
        , Warning   //!< Shadowing.
        , Error     //!< Drafts and broken references.
    };

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
     * \brief   One guard validation entry; navigates to its transition.
     **/
    struct Finding
    {
        eSeverity   severity;       //!< The severity.
        eKind       kind;           //!< The finding kind.
        uint32_t    transitionId;   //!< The owning transition (navigation target).
        QString     location;       //!< `State : stimulus -> target` (as where-used).
        QString     message;        //!< The human-readable finding text.
    };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Every guard finding of the document, in state/transition order.
    static QList<Finding> validate(const StateMachineData& data);

    //!< The findings of one transition's guard.
    static QList<Finding> validateTransition(const StateMachineData& data, uint32_t transitionId);

    /**
     * \brief   The worst severity of one transition's findings; false when the guard is
     *          clean (the canvas edge label and tab badge key off this).
     **/
    static bool worstSeverity(const StateMachineData& data, uint32_t transitionId, eSeverity& worst);
};

#endif  // LUSAN_MODEL_SM_SMGUARDVALIDATION_HPP
