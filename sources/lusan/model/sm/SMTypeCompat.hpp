#ifndef LUSAN_MODEL_SM_SMTYPECOMPAT_HPP
#define LUSAN_MODEL_SM_SMTYPECOMPAT_HPP
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
 *  \file        lusan/model/sm/SMTypeCompat.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM condition operand type-compatibility helper.
 *
 ************************************************************************/

#include "lusan/data/sm/SMCondition.hpp"

#include <QString>

/**
 * \brief   Minimal type-compatibility check for a condition comparison row, implementing
 *          the primitive rules of spec 6.9: the numeric widening ladders
 *          (`uint8..uint64`, `int8..int64`, `float -> double`, integer of at most 32 bits
 *          `-> double`), and the `bool`/`String`/enumeration `Equal`/`NotEqual`-only rule.
 *          This is a seed helper: it judges the `.siml` primitive vocabulary directly and
 *          treats any other (declared) type by exact-name equality only. SM-25 extends it
 *          with the document's type registry (enum vs structure vs container distinction,
 *          "no compare" for structure/container, enum literal resolution).
 **/
class SMTypeCompat
{
public:
    /**
     * \brief   Decides whether `lhsType op rhsType` is a valid condition comparison.
     * \param   lhsType     The declared type name of the LHS operand.
     * \param   op          The comparison operator (`None` for a boolean-test row).
     * \param   rhsType     The declared type name of the RHS operand.
     * \return  An empty string when the comparison is valid (including `op == None`, an
     *          empty operand type that cannot be judged yet, or a verbatim/opaque operand);
     *          otherwise a short, user-facing reason.
     **/
    static QString areComparable(const QString& lhsType, SMConditionEntry::eOperator op, const QString& rhsType);

    /**
     * \brief   True if \p typeName is one of the `.siml` primitive type names.
     **/
    static bool isPrimitive(const QString& typeName);

private:
    static bool isBoolOrString(const QString& typeName);
    static bool isChar(const QString& typeName);
    static bool isNumeric(const QString& typeName);
    static bool comparableNumeric(const QString& lhsType, const QString& rhsType);
    static bool widensTo(const QString& fromType, const QString& toType);
};

#endif  // LUSAN_MODEL_SM_SMTYPECOMPAT_HPP
