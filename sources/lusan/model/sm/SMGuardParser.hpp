#ifndef LUSAN_MODEL_SM_SMGUARDPARSER_HPP
#define LUSAN_MODEL_SM_SMGUARDPARSER_HPP
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
 *  \file        lusan/model/sm/SMGuardParser.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard parser: text -> ID-bound AST + diagnostics.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMGuardTree.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;
class SMConditionList;

/**
 * \class   SMGuardParser
 * \brief   Parses the guard editing surface (a C++ boolean-expression subset) into the
 *          resolved, ID-bound AST, resolving names against the document's registries scoped
 *          to a transition's stimulus (the Resolution Contract). Names are
 *          resolved at edit time only; the stored tree references IDs, so codegen never
 *          resolves names. Raw C++ fragments enter the tree ONLY when \p allowRaw is
 *          set (the explicit "Keep as raw C++" quick-fix) -- never silently.
 **/
class SMGuardParser
{
public:
    /**
     * \enum    eSeverity
     * \brief   The severity of a parse diagnostic.
     **/
    enum class eSeverity
    {
          Error     //!< Unresolved / malformed: the guard cannot commit (saved as a draft).
        , Warning   //!< Resolved but risky (e.g. a stimulus parameter shadows an attribute).
    };

    /**
     * \struct  Diagnostic
     * \brief   One diagnostic anchored to a span of the source text.
     **/
    struct Diagnostic
    {
        eSeverity   severity;   //!< The severity.
        int         start;      //!< The 0-based start offset in the source text.
        int         length;     //!< The span length in characters.
        QString     message;    //!< The human-readable message.
    };

    /**
     * \struct  Result
     * \brief   The outcome of a parse: the best-effort tree (owned by the CALLER; may be
     *          nullptr for empty input) and the diagnostics. \ref resolved is true only when
     *          a tree was produced with no error diagnostics. Do not copy a Result while it
     *          owns a tree; hand the tree to a guard/command or delete it.
     **/
    struct Result
    {
        SMGuardNode*        tree { nullptr };   //!< The parsed tree (caller owns it).
        QList<Diagnostic>   diagnostics;        //!< The diagnostics in source order.

        //!< True when a tree was produced and there is no error diagnostic.
        bool resolved() const;
        //!< True when any diagnostic is an error.
        bool hasError() const;
    };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Parses \p text scoped to \p transitionId. When \p allowRaw is false, an
     *          unresolved fragment is an error diagnostic; when true it becomes a `<Raw>`
     *          node.
     * \return  A \ref Result whose \c tree the caller owns.
     **/
    static Result parse(const StateMachineData& data, uint32_t transitionId, const QString& text, bool allowRaw = false);

    /**
     * \brief   Parses \p text into a committed guard: `ok(tree)` when fully resolved,
     *          otherwise `draft(text)` (nothing the user typed is lost). Blank text
     *          yields an empty guard.
     **/
    static SMGuard parseToGuard(const StateMachineData& data, uint32_t transitionId, const QString& text, bool allowRaw = false);

    /**
     * \brief   The legacy read-shim: renders a legacy condition tree to
     *          text with the legacy renderer and returns it as a `draft` guard, so a document
     *          that still carries `<ConditionList>` loads without losing anything and is
     *          re-resolved by the user in the editor.
     **/
    static SMGuard fromLegacy(const SMConditionList& legacy);
};

#endif  // LUSAN_MODEL_SM_SMGUARDPARSER_HPP
