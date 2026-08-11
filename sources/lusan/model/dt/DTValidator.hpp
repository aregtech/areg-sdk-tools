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
// Constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   A warning or an advisory note `n` is reported with the rule id
     *          (`ADVISORY_RULE_BASE + n`). A band, not a second severity of the same rule.
     **/
    static constexpr int ADVISORY_RULE_BASE { DocRuleChecks::ADVISORY_RULE_BASE };

    // The shapes this engine shares with the service interface keep the service interface's
    // numbers, so one number means one thing to an author reading either document's findings.
    static constexpr int RULE_MISSING_NAME          { 1 };  //!< The document, or an entry, has no name.
    static constexpr int RULE_INVALID_IDENTIFIER    { 2 };  //!< A name the generated code could not carry.
    static constexpr int RULE_DUPLICATE_NAME        { 3 };  //!< A name already taken in the same registry.
    static constexpr int RULE_UNRESOLVED_TYPE       { 4 };  //!< A declared type no data type answers to.
    static constexpr int RULE_BAD_LITERAL           { 5 };  //!< A value that is not readable as its type.
    static constexpr int RULE_EMPTY_TYPE            { 7 };  //!< A structure or enumeration with no fields.
    static constexpr int RULE_DUPLICATE_ENUM_VALUE  { 9 };  //!< Two enumerators of one enumeration counting the same.
    static constexpr int RULE_UNREFERENCED         { 10 };  //!< Advisory: nothing in the document uses the declaration.
    static constexpr int RULE_DEPRECATED           { 14 };  //!< Advisory: the document, or a type in it, is deprecated.

    //!< The document declares nothing at all, so it contributes nothing to what includes it.
    static constexpr int RULE_EMPTY_DOCUMENT       { 15 };

    //!< An include that is not a C++ header. A data type document includes nothing else.
    static constexpr int RULE_NOT_A_HEADER         { 16 };

    //!< Advisory: the document declares one name and lives in a file called another.
    static constexpr int RULE_FILE_NAME_MISMATCH   { 17 };

    /**
     * \brief   An element of the opened file the format does not place: an unknown tag, a real
     *          one written where the format does not allow it, or one the format has dropped.
     *          The block is kept as written, so a document carrying one still opens and still
     *          saves without losing it.
     **/
    static constexpr int RULE_UNKNOWN_ELEMENT      { 18 };

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
