#ifndef LUSAN_MODEL_SM_SMLITERALVALIDATOR_HPP
#define LUSAN_MODEL_SM_SMLITERALVALIDATOR_HPP
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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/sm/SMLiteralValidator.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM literal value syntax validator (spec 6.9).
 *
 ************************************************************************/

#include <QString>

/**
 * \brief   Validates a literal against a primitive data type's syntax (spec 6.9): bool,
 *          char, signed/unsigned integers (decimal or `0x` hexadecimal, range-checked per
 *          bit width), float/double, and String (unrestricted). Enumeration, structure,
 *          container and imported types have no literal syntax of their own — callers
 *          resolve an enumeration literal against the declared type's enumerators and
 *          disable literal entry entirely for structure/container/imported types.
 **/
class SMLiteralValidator
{
public:
    /**
     * \brief   Checks the given literal against the syntax of the named primitive type.
     * \param   typeName    One of the `.siml` primitive names (bool, char, int8..int64,
     *                      uint8..uint64, float, double, String). Any other name (a declared
     *                      enumeration/structure/container/imported type) always validates.
     * \param   literal     The literal text to check. An empty literal always validates — a
     *                      default/value is optional, so absence is not a syntax error.
     * \return  An empty string if the literal is valid; otherwise a short, user-facing reason.
     **/
    static QString validate(const QString& typeName, const QString& literal);

private:
    static QString validateBool(const QString& literal);
    static QString validateChar(const QString& literal);
    static QString validateSignedInteger(const QString& literal, int bits);
    static QString validateUnsignedInteger(const QString& literal, int bits);
    static QString validateFloatingPoint(const QString& literal);
};

#endif  // LUSAN_MODEL_SM_SMLITERALVALIDATOR_HPP
