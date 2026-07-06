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
 *  \file        lusan/model/sm/SMLiteralValidator.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM literal value syntax validator.
 *
 ************************************************************************/

#include "lusan/model/sm/SMLiteralValidator.hpp"

#include <QObject>
#include <limits>

namespace
{
    //!< Parses an optional leading '-' followed by a decimal or `0x`-hexadecimal magnitude.
    bool parseInteger(const QString& text, bool& negative, quint64& magnitude)
    {
        QString t = text.trimmed();
        negative = t.startsWith(QLatin1Char('-'));
        if (negative)
        {
            t.remove(0, 1);
        }

        if (t.isEmpty())
            return false;

        bool ok = false;
        magnitude = t.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)
                  ? t.mid(2).toULongLong(&ok, 16)
                  : t.toULongLong(&ok, 10);
        return ok;
    }
}

QString SMLiteralValidator::validate(const QString& typeName, const QString& literal)
{
    // A default/value literal is optional; absence is never a syntax error.
    if (literal.isEmpty())
        return QString();

    if (typeName == QStringLiteral("bool"))
        return validateBool(literal);
    if (typeName == QStringLiteral("char"))
        return validateChar(literal);
    if (typeName == QStringLiteral("int8"))
        return validateSignedInteger(literal, 8);
    if (typeName == QStringLiteral("int16"))
        return validateSignedInteger(literal, 16);
    if (typeName == QStringLiteral("int32"))
        return validateSignedInteger(literal, 32);
    if (typeName == QStringLiteral("int64"))
        return validateSignedInteger(literal, 64);
    if (typeName == QStringLiteral("uint8"))
        return validateUnsignedInteger(literal, 8);
    if (typeName == QStringLiteral("uint16"))
        return validateUnsignedInteger(literal, 16);
    if (typeName == QStringLiteral("uint32"))
        return validateUnsignedInteger(literal, 32);
    if (typeName == QStringLiteral("uint64"))
        return validateUnsignedInteger(literal, 64);
    if ((typeName == QStringLiteral("float")) || (typeName == QStringLiteral("double")))
        return validateFloatingPoint(literal);

    // String and any declared enumeration/structure/container/imported type: resolved by
    // the caller (enumerator lookup, or literal entry disabled entirely).
    return QString();
}

QString SMLiteralValidator::validateBool(const QString& literal)
{
    if ((literal == QStringLiteral("true")) || (literal == QStringLiteral("false")))
        return QString();

    return QObject::tr("Expected 'true' or 'false'");
}

QString SMLiteralValidator::validateChar(const QString& literal)
{
    if (literal.length() == 1)
        return QString();

    return QObject::tr("Expected exactly one character");
}

QString SMLiteralValidator::validateSignedInteger(const QString& literal, int bits)
{
    bool negative = false;
    quint64 magnitude = 0;
    if (parseInteger(literal, negative, magnitude) == false)
        return QObject::tr("Expected a decimal or 0x-hexadecimal integer");

    const quint64 maxMagnitude = (bits >= 64)
        ? (negative ? (quint64{ 1 } << 63) : static_cast<quint64>(std::numeric_limits<qint64>::max()))
        : (negative ? (quint64{ 1 } << (bits - 1)) : ((quint64{ 1 } << (bits - 1)) - 1));

    if (magnitude > maxMagnitude)
        return QObject::tr("Value out of range for a %1-bit signed integer").arg(bits);

    return QString();
}

QString SMLiteralValidator::validateUnsignedInteger(const QString& literal, int bits)
{
    bool negative = false;
    quint64 magnitude = 0;
    if (parseInteger(literal, negative, magnitude) == false)
        return QObject::tr("Expected a decimal or 0x-hexadecimal integer");

    if (negative)
        return QObject::tr("Unsigned type cannot be negative");

    const quint64 maxValue = (bits >= 64) ? std::numeric_limits<quint64>::max() : ((quint64{ 1 } << bits) - 1);
    if (magnitude > maxValue)
        return QObject::tr("Value out of range for a %1-bit unsigned integer").arg(bits);

    return QString();
}

QString SMLiteralValidator::validateFloatingPoint(const QString& literal)
{
    if (literal.trimmed().isEmpty())
        return QObject::tr("Value is required");

    bool ok = false;
    literal.toDouble(&ok);
    if (ok == false)
        return QObject::tr("Expected a floating-point number");

    return QString();
}
