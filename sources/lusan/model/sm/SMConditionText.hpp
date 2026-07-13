#ifndef LUSAN_MODEL_SM_SMCONDITIONTEXT_HPP
#define LUSAN_MODEL_SM_SMCONDITIONTEXT_HPP
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
 *  \file        lusan/model/sm/SMConditionText.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM condition tree text/codegen renderer.
 *
 ************************************************************************/

#include "lusan/data/sm/SMCondition.hpp"

#include <QString>

/**
 * \brief   Renders a condition tree (\ref SMConditionGroup) to a single-line string, in
 *          two modes: a human-readable **preview** (bare model names) and the **C++**
 *          guard the generator is expected to emit (attribute getters, qualified
 *          constants, condition-method calls, verbatim rows, and the lambda IIFE). Both
 *          implement the codegen mapping of the SM-21 design so the preview is a faithful,
 *          parenthesized picture of the generated `if (...)`. This is a derived view; the
 *          data tree stays canonical (nothing here parses or mutates the model).
 **/
class SMConditionText
{
public:
    /**
     * \brief   Renders the tree as a readable preview using bare model names, e.g.
     *          `WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode`.
     **/
    static QString preview(const SMConditionGroup& root);

    /**
     * \brief   Renders the tree as the generated C++ guard, e.g.
     *          `walkRequested() && (hasWaiting(count) || count >= <Data>::MIN_WAITING) && !isNightMode()`.
     * \param   dataClass   Qualifier used for constants (`<qualifier>::NAME`). Defaults to
     *                      the `<Data>` placeholder; the generator substitutes the machine's
     *                      data class name.
     **/
    static QString cpp(const SMConditionGroup& root, const QString& dataClass = QString("<Data>"));

    /**
     * \brief   The compact guard summary shown next to the stimulus on a canvas edge label
     *          (`stimulus[summary]`). It is the readable preview; the caller truncates it to
     *          fit and keeps the full text in the tooltip. Empty for an empty guard (so the
     *          canvas shows the stimulus alone, with no brackets).
     **/
    static QString summary(const SMConditionGroup& root);

private:
    enum class eMode
    {
          Preview
        , Cpp
    };

    static QString renderGroup(const SMConditionGroup& group, eMode mode, const QString& dataClass, bool isRoot);
    static QString renderLeaf(const SMConditionEntry& leaf, eMode mode, const QString& dataClass);
    static QString renderOperand( SMConditionEntry::eOperandKind kind
                                , const QString& name
                                , const QList<SMArgumentEntry>& args
                                , eMode mode
                                , const QString& dataClass);
    static QString renderArgs(const QList<SMArgumentEntry>& args, eMode mode, const QString& dataClass);
    static QString renderArgOperand( SMArgumentEntry::eValueSource source
                                   , const QString& value
                                   , const QString& expression
                                   , eMode mode
                                   , const QString& dataClass);
    static QString opSymbol(SMConditionEntry::eOperator op);
    static QString lowerFirst(const QString& name);
};

#endif  // LUSAN_MODEL_SM_SMCONDITIONTEXT_HPP
