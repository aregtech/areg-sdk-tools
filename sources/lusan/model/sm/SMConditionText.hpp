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
 *  \brief       Lusan application, FSM legacy condition renderer (LEGACY-LOAD-ONLY).
 *
 ************************************************************************/

#include "lusan/data/sm/SMCondition.hpp"

#include <QString>

/**
 * \brief   LEGACY-LOAD-ONLY. The sole remaining consumer is
 *          \ref SMGuardParser::fromLegacy, which renders a loaded legacy
 *          `<ConditionList>` tree to text so it can be kept as a `<Draft>` guard.
 *          Do not add new callers; the guard classes (SMGuardRender /
 *          SMGuardCodegenPreview) own every live rendering path.
 **/
class SMConditionText
{
public:
    /**
     * \brief   Renders the tree as a readable preview using bare model names, e.g.
     *          `WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode`.
     **/
    static QString preview(const SMConditionGroup& root);

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
