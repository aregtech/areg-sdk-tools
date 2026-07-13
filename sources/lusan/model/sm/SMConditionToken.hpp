#ifndef LUSAN_MODEL_SM_SMCONDITIONTOKEN_HPP
#define LUSAN_MODEL_SM_SMCONDITIONTOKEN_HPP
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
 *  \file        lusan/model/sm/SMConditionToken.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM condition leaf token translator (prefix DSL).
 *
 ************************************************************************/

#include "lusan/data/sm/SMCondition.hpp"

#include <QString>

/**
 * \brief   Bidirectional translator between a single condition leaf (\ref SMConditionEntry)
 *          and its `prefix::` token text -- the surface experts type and the picker fills.
 *          The grammar is bounded (SM-21 design Section 4): the prefixes `arg:: attr::
 *          const:: cond:: val:: expr:: lambda::`, the six relops, a leading `!`, quoted
 *          literals, and (for `cond::`) a parenthesized argument list. This is NOT a C++
 *          parser: it recognizes only this closed grammar and runs only on live editor
 *          input; the token text is never persisted (the data tree is canonical).
 **/
class SMConditionToken
{
public:
    /**
     * \brief   Renders a single leaf to its token text, e.g. `attr::IsNightMode`,
     *          `arg::count >= const::MIN_WAITING`, `cond::HasWaiting(arg::count)`,
     *          `expr::{ ... }`, `!val::true`.
     **/
    static QString renderLeaf(const SMConditionEntry& leaf);

    /**
     * \brief   Parses token text into \p out (a leaf), overwriting its fields.
     * \param   text    The token text for one leaf (one comparison, bool test, or verbatim row).
     * \param   out     The leaf to fill (kind/ref/operator/negate/arguments/verbatim body).
     * \param   error   Set to a short reason when parsing fails.
     * \return  True on success; false with \p error set on a grammar violation.
     **/
    static bool parseLeaf(const QString& text, SMConditionEntry& out, QString& error);

    /**
     * \brief   Checks that a group line mixes at most one top-level combinator (`&&` XOR
     *          `||`). Combinators inside parentheses (an explicit sub-group) are ignored.
     * \param   line    The group's typed line.
     * \param   reason  Set to a short reason when both `&&` and `||` appear at top level.
     * \return  True when the line uses a single combinator (or none); false when mixed.
     **/
    static bool hasSingleCombinator(const QString& line, QString& reason);

    // Token prefixes (without the "::").
    static constexpr const char* const  PREFIX_ARG    { "arg"    };
    static constexpr const char* const  PREFIX_ATTR   { "attr"   };
    static constexpr const char* const  PREFIX_CONST  { "const"  };
    static constexpr const char* const  PREFIX_COND   { "cond"   };
    static constexpr const char* const  PREFIX_VAL    { "val"    };
    static constexpr const char* const  PREFIX_EXPR   { "expr"   };
    static constexpr const char* const  PREFIX_LAMBDA { "lambda" };

private:
    static QString operandToken(SMConditionEntry::eOperandKind kind, const QString& name, const QList<SMArgumentEntry>& args);
    static QString argToken(const SMArgumentEntry& arg);
    static QString sourcePrefix(SMArgumentEntry::eValueSource source);
    static bool prefixToKind(const QString& prefix, SMConditionEntry::eOperandKind& kind);
    static QString literalToken(const QString& literal);

    static bool parseOperand( const QString& token
                            , SMConditionEntry::eOperandKind& kind
                            , QString& name
                            , QList<SMArgumentEntry>* argsOut
                            , SMConditionEntry* parent
                            , QString& error);

    // Finds the leftmost top-level relop; returns its index, or -1. Sets \p len and \p op.
    static int findTopLevelRelop(const QString& s, int& len, SMConditionEntry::eOperator& op);
};

#endif  // LUSAN_MODEL_SM_SMCONDITIONTOKEN_HPP
