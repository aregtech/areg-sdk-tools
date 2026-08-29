#ifndef LUSAN_MODEL_SI_SIVALIDATOR_HPP
#define LUSAN_MODEL_SI_SIVALIDATOR_HPP
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
 *  \file        lusan/model/si/SIValidator.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, service interface structural and reference validation engine.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocIssue.hpp"
#include "lusan/model/common/DocRuleChecks.hpp"

#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class ServiceInterfaceData;

/**
 * \class   SIValidator
 * \brief   Checks a service interface document's structure and its cross-references: the
 *          interface name and version, identifier syntax and uniqueness across the registries,
 *          resolution of every declared type (attribute, method parameter, constant, structure
 *          field, container key and value), constant literals against their declared type,
 *          request-to-response wiring, the declarations nothing in the document uses, and the
 *          imported data type documents nothing in the document takes a type from.
 *
 *          Like the state machine engine it is a pure, headless transform -- input a
 *          `const ServiceInterfaceData&`, output a `QList<DocIssue>` -- with no widget, model or
 *          scheduling dependency, so a code generator can run the same checks the editor runs.
 **/
class SIValidator
{
//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Runs every check over the document and returns the findings in document order
     *          (the overview first, then the registries in page order).
     **/
    static QList<DocIssue> validate(const ServiceInterfaceData& data);

    /**
     * \brief   The field a finding blames, for the checks that know it. Takes the rule id as
     *          stored on the finding (\ref DocIssue::rule), advisory band included, so a caller
     *          hands over `issue.rule` untouched.
     * \return  \a eIssueField::None when the check names no single field.
     **/
    static eIssueField fieldOfRule(int rule);

    /**
     * \brief   Why a finding is a finding, and what resolves it, in the author's terms. Empty
     *          when the message already says everything there is to say.
     **/
    static QString explainRule(int rule, DocIssue::eSeverity severity);
};

#endif  // LUSAN_MODEL_SI_SIVALIDATOR_HPP
