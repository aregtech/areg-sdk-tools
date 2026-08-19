#ifndef LUSAN_MODEL_DT_DTVALIDATOR_HPP
#define LUSAN_MODEL_DT_DTVALIDATOR_HPP
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
 *  \file        lusan/model/dt/DTValidator.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type document validation engine.
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
class DataTypeDocumentData;

/**
 * \class   DTValidator
 * \brief   Checks a `.dtml` document: the name it carries into generated code, identifier syntax
 *          and uniqueness, resolution of every declared type, the literals of structure fields,
 *          the values of enumerators, and the declarations marked deprecated.
 *
 *          Like the other two engines it is a pure, headless transform -- input a
 *          `const DataTypeDocumentData&`, output a `QList<DocIssue>` -- so a code generator can
 *          run the same checks the editor runs.
 **/
class DTValidator
{
//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Runs every check over the document and returns the findings in document order.
     **/
    static QList<DocIssue> validate(const DataTypeDocumentData& data);

    /**
     * \brief   The field a finding blames, for the checks that know it. Takes the rule id as
     *          stored on the finding, advisory band included.
     **/
    static eIssueField fieldOfRule(int rule);

    /**
     * \brief   Why a finding is a finding, and what resolves it, in the author's terms.
     **/
    static QString explainRule(int rule, DocIssue::eSeverity severity);
};

#endif  // LUSAN_MODEL_DT_DTVALIDATOR_HPP
