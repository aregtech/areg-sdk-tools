#ifndef LUSAN_MODEL_SM_SMVALIDATOR_HPP
#define LUSAN_MODEL_SM_SMVALIDATOR_HPP
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
 *  \file        lusan/model/sm/SMValidator.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM structural and reference validation engine.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocModelNotifier.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \struct  SMIssue
 * \brief   One validation finding: the offending element (the navigation target), its
 *          severity, an identifier for the check it failed, and a human-readable message.
 *          The `kind` routes a results panel to the owning page; a zero `elementId` marks
 *          a document- or level-wide finding with no single owning element.
 **/
struct SMIssue
{
    /**
     * \enum    eSeverity
     * \brief   Finding severity, ordered so a worst-of comparison is a simple `max`.
     **/
    enum class eSeverity
    {
          Info      //!< Advisory.
        , Warning   //!< Should fix.
        , Error     //!< Must fix; blocks code generation.
    };

    uint32_t        elementId { 0 };                        //!< The offending element (navigation target).
    eDocElementKind kind      { eDocElementKind::State };   //!< The element kind (page routing).
    eSeverity       severity  { eSeverity::Error };         //!< The severity.
    int             rule      { 0 };                        //!< The identifier of the failed check.
    QString         message;                                //!< The finding text.
};

/**
 * \class   SMValidator
 * \brief   Checks a document's structure and its cross-references: start-state placement,
 *          duplicate identifiers and names, identifier syntax, resolution of every
 *          name-based reference (targets, stimuli, actions, timers, events, attributes,
 *          data types, condition operands and argument sources), argument-to-parameter
 *          matching, parameter-scope, composite-state constraints, and condition-row shape.
 *
 *          It is a pure, headless transform -- input a `const StateMachineData&`, output a
 *          `QList<SMIssue>` -- with no widget, model, or scheduling dependency, so the code
 *          generator can re-run it and block on errors exactly as the editor runs it in the
 *          background. Type compatibility, literal parsing, and import resolution are the
 *          concern of separate validators.
 **/
class SMValidator
{
//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   A 10.2 warning `n` is reported with the rule id (`WARNING_RULE_BASE + n`); the
     *          error rule ids stay the plain 10.1 numbers. The two numbering spaces share the
     *          single `SMIssue::rule` field, so the warning offset keeps them from colliding
     *          (warning 2 and error 2 are distinguishable by id, not only by severity).
     **/
    static constexpr int WARNING_RULE_BASE { 100 };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Runs every check over the document and returns the findings in document
     *          order (states and levels first, then the registries).
     **/
    static QList<SMIssue> validate(const StateMachineData& data);
};

#endif  // LUSAN_MODEL_SM_SMVALIDATOR_HPP
