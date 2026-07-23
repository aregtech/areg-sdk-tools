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
 *  \file        lusan/model/sm/SMTypeCompat.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM condition operand type-compatibility helper.
 *
 ************************************************************************/

#include "lusan/model/sm/SMTypeCompat.hpp"

namespace
{
    // Widening rank inside a single ladder; 0 means "not in this ladder".
    int unsignedRank(const QString& t)
    {
        if (t == "uint8")  return 1;
        if (t == "uint16") return 2;
        if (t == "uint32") return 3;
        if (t == "uint64") return 4;
        return 0;
    }

    int signedRank(const QString& t)
    {
        if (t == "int8")  return 1;
        if (t == "int16") return 2;
        if (t == "int32") return 3;
        if (t == "int64") return 4;
        return 0;
    }

    int floatRank(const QString& t)
    {
        if (t == "float")  return 1;
        if (t == "double") return 2;
        return 0;
    }

    // An integer of at most 32 bits (signed or unsigned).
    bool isIntUpTo32(const QString& t)
    {
        return (t == "int8")  || (t == "int16")  || (t == "int32")
            || (t == "uint8") || (t == "uint16") || (t == "uint32");
    }
}

SMTypeCompat::eRank SMTypeCompat::rank(const QString& fromType, const QString& toType)
{
    if (fromType.isEmpty() || toType.isEmpty())
        return eRank::Unknown;

    if (fromType == toType)
        return eRank::Exact;

    // A non-primitive (declared) type matches by exact name only.
    if ((isPrimitive(fromType) == false) || (isPrimitive(toType) == false))
        return eRank::Mismatch;

    if (widensTo(fromType, toType))
        return eRank::Converts;

    if (widensTo(toType, fromType))
        return eRank::Narrows;

    return eRank::Mismatch;
}

bool SMTypeCompat::isBoolOrString(const QString& typeName)
{
    return (typeName == "bool") || (typeName == "String");
}

bool SMTypeCompat::isChar(const QString& typeName)
{
    return (typeName == "char");
}

bool SMTypeCompat::isNumeric(const QString& typeName)
{
    return (unsignedRank(typeName) != 0) || (signedRank(typeName) != 0) || (floatRank(typeName) != 0);
}

bool SMTypeCompat::isPrimitive(const QString& typeName)
{
    return isBoolOrString(typeName) || isChar(typeName) || isNumeric(typeName);
}

bool SMTypeCompat::widensTo(const QString& fromType, const QString& toType)
{
    if (fromType == toType)
        return true;

    const int uf = unsignedRank(fromType);
    const int ut = unsignedRank(toType);
    if ((uf != 0) && (ut != 0))
        return (uf <= ut);

    const int sf = signedRank(fromType);
    const int st = signedRank(toType);
    if ((sf != 0) && (st != 0))
        return (sf <= st);

    const int ff = floatRank(fromType);
    const int ft = floatRank(toType);
    if ((ff != 0) && (ft != 0))
        return (ff <= ft);

    // Any integer of at most 32 bits widens to double (rule 4).
    if (isIntUpTo32(fromType) && (toType == "double"))
        return true;

    return false;
}

bool SMTypeCompat::comparableNumeric(const QString& lhsType, const QString& rhsType)
{
    return widensTo(lhsType, rhsType) || widensTo(rhsType, lhsType);
}

QString SMTypeCompat::areComparable(const QString& lhsType, SMConditionEntry::eOperator op, const QString& rhsType)
{
    // A boolean-test row (no operator) has nothing to compare.
    if (op == SMConditionEntry::eOperator::None)
        return QString();

    // An operand whose type is not resolved yet cannot be judged.
    if (lhsType.isEmpty() || rhsType.isEmpty())
        return QString();

    const bool ordering = (op != SMConditionEntry::eOperator::Equal)
                       && (op != SMConditionEntry::eOperator::NotEqual);

    // bool / String: Equal / NotEqual only, and only against the same type.
    if (isBoolOrString(lhsType) || isBoolOrString(rhsType))
    {
        if (ordering)
        {
            return QString("'%1' supports only Equal / NotEqual")
                    .arg(isBoolOrString(lhsType) ? lhsType : rhsType);
        }

        if (lhsType != rhsType)
        {
            return QString("cannot compare '%1' with '%2'").arg(lhsType, rhsType);
        }

        return QString();
    }

    // Numerics: any operator, subject to a widening path in either direction.
    if (isNumeric(lhsType) && isNumeric(rhsType))
    {
        if (comparableNumeric(lhsType, rhsType))
            return QString();

        return QString("cannot compare '%1' with '%2' without an implicit widening path")
                .arg(lhsType, rhsType);
    }

    // char: any operator, but only against char.
    if (isChar(lhsType) && isChar(rhsType))
        return QString();

    // Remaining: char-vs-other, or a declared (non-primitive) type. In the seed these
    // support Equal / NotEqual only, and only by exact-name match.
    if (ordering)
    {
        const QString& reported = (isPrimitive(lhsType) == false) ? lhsType
                                : ((isPrimitive(rhsType) == false) ? rhsType : lhsType);
        return QString("'%1' supports only Equal / NotEqual").arg(reported);
    }

    if (lhsType != rhsType)
        return QString("cannot compare '%1' with '%2'").arg(lhsType, rhsType);

    return QString();
}
