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
 *  \file        lusan/model/sm/SMConditionToken.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM condition leaf token translator (prefix DSL).
 *
 ************************************************************************/

#include "lusan/model/sm/SMConditionToken.hpp"

#include <QRegularExpression>
#include <QStringList>

namespace
{
    const QString kSep       = QStringLiteral("::");
    const QString kExprOpen  = QStringLiteral("expr::{");
    const QString kLambdaOpen= QStringLiteral("lambda::{");

    QString opText(SMConditionEntry::eOperator op)
    {
        switch (op)
        {
        case SMConditionEntry::eOperator::Equal:        return QStringLiteral("==");
        case SMConditionEntry::eOperator::NotEqual:     return QStringLiteral("!=");
        case SMConditionEntry::eOperator::Less:         return QStringLiteral("<");
        case SMConditionEntry::eOperator::LessEqual:    return QStringLiteral("<=");
        case SMConditionEntry::eOperator::Greater:      return QStringLiteral(">");
        case SMConditionEntry::eOperator::GreaterEqual: return QStringLiteral(">=");
        case SMConditionEntry::eOperator::None:
        default:                                        return QString();
        }
    }

    // Splits \p s on \p delim at parenthesis-depth 0 and outside double quotes.
    QStringList splitTopLevel(const QString& s, QChar delim)
    {
        QStringList out;
        int depth = 0;
        bool inQuote = false;
        int start = 0;
        for (int i = 0; i < s.length(); ++i)
        {
            const QChar c = s.at(i);
            if (c == QChar('"'))
            {
                inQuote = !inQuote;
            }
            else if (inQuote == false && (c == QChar('(')))
            {
                ++depth;
            }
            else if (inQuote == false && (c == QChar(')')))
            {
                if (depth > 0) { --depth; }
            }
            else if (inQuote == false && depth == 0 && c == delim)
            {
                out << s.mid(start, i - start);
                start = i + 1;
            }
        }

        out << s.mid(start);
        return out;
    }

    QString unquote(const QString& s)
    {
        if ((s.length() >= 2) && s.startsWith(QChar('"')) && s.endsWith(QChar('"')))
        {
            return s.mid(1, s.length() - 2);
        }

        return s;
    }
}

QString SMConditionToken::literalToken(const QString& literal)
{
    static const QRegularExpression simple(QStringLiteral("^-?[A-Za-z0-9_.]+$"));
    if ((literal.isEmpty() == false) && simple.match(literal).hasMatch())
    {
        return literal;
    }

    QString escaped = literal;
    escaped.replace(QChar('"'), QStringLiteral("\\\""));
    return QChar('"') + escaped + QChar('"');
}

QString SMConditionToken::sourcePrefix(SMArgumentEntry::eValueSource source)
{
    switch (source)
    {
    case SMArgumentEntry::eValueSource::Param:      return QString::fromLatin1(PREFIX_ARG);
    case SMArgumentEntry::eValueSource::Attribute:  return QString::fromLatin1(PREFIX_ATTR);
    case SMArgumentEntry::eValueSource::Constant:   return QString::fromLatin1(PREFIX_CONST);
    case SMArgumentEntry::eValueSource::Condition:  return QString::fromLatin1(PREFIX_COND);
    case SMArgumentEntry::eValueSource::Value:      return QString::fromLatin1(PREFIX_VAL);
    default:                                        return QString();
    }
}

QString SMConditionToken::argToken(const SMArgumentEntry& arg)
{
    switch (arg.getSource())
    {
    case SMArgumentEntry::eValueSource::Value:
        return QString::fromLatin1(PREFIX_VAL) + kSep + literalToken(arg.getValue());
    case SMArgumentEntry::eValueSource::Expression:
        return QString::fromLatin1(PREFIX_EXPR) + QStringLiteral("::{") + arg.getExpression() + QChar('}');
    case SMArgumentEntry::eValueSource::Lambda:
        return QString::fromLatin1(PREFIX_LAMBDA) + QStringLiteral("::{") + arg.getValue() + QChar('}');
    default:
        return sourcePrefix(arg.getSource()) + kSep + arg.getValue();
    }
}

QString SMConditionToken::operandToken(SMConditionEntry::eOperandKind kind, const QString& name, const QList<SMArgumentEntry>& args)
{
    switch (kind)
    {
    case SMArgumentEntry::eValueSource::Expression:
        return QString::fromLatin1(PREFIX_EXPR) + QStringLiteral("::{") + name + QChar('}');

    case SMArgumentEntry::eValueSource::Lambda:
        return QString::fromLatin1(PREFIX_LAMBDA) + QStringLiteral("::{") + name + QChar('}');

    case SMArgumentEntry::eValueSource::Condition:
    {
        QString token = QString::fromLatin1(PREFIX_COND) + kSep + name;
        if (args.isEmpty() == false)
        {
            QStringList parts;
            for (const SMArgumentEntry& arg : args)
            {
                parts << argToken(arg);
            }

            token += QChar('(') + parts.join(QStringLiteral(", ")) + QChar(')');
        }

        return token;
    }

    case SMArgumentEntry::eValueSource::Value:
        return QString::fromLatin1(PREFIX_VAL) + kSep + literalToken(name);

    default:    // Param / Attribute / Constant
        return sourcePrefix(kind) + kSep + name;
    }
}

QString SMConditionToken::renderLeaf(const SMConditionEntry& leaf)
{
    const QString neg = leaf.isNegated() ? QStringLiteral("!") : QString();

    if (leaf.isExpressionRow())
    {
        return neg + kExprOpen + leaf.getExpression() + QChar('}');
    }

    if (leaf.isLambdaRow())
    {
        return neg + kLambdaOpen + leaf.getBody() + QChar('}');
    }

    const QString lhs = operandToken(leaf.getLhsKind(), leaf.getLhs(), leaf.getArguments());
    if (leaf.hasOperator())
    {
        const QString rhs = operandToken(leaf.getRhsKind(), leaf.getRhs(), QList<SMArgumentEntry>());
        return neg + lhs + QChar(' ') + opText(leaf.getOperator()) + QChar(' ') + rhs;
    }

    return neg + lhs;
}

bool SMConditionToken::prefixToKind(const QString& prefix, SMConditionEntry::eOperandKind& kind)
{
    if (prefix == QLatin1String(PREFIX_ARG))        { kind = SMArgumentEntry::eValueSource::Param;     return true; }
    if (prefix == QLatin1String(PREFIX_ATTR))       { kind = SMArgumentEntry::eValueSource::Attribute; return true; }
    if (prefix == QLatin1String(PREFIX_CONST))      { kind = SMArgumentEntry::eValueSource::Constant;  return true; }
    if (prefix == QLatin1String(PREFIX_COND))       { kind = SMArgumentEntry::eValueSource::Condition; return true; }
    if (prefix == QLatin1String(PREFIX_VAL))        { kind = SMArgumentEntry::eValueSource::Value;     return true; }
    return false;
}

int SMConditionToken::findTopLevelRelop(const QString& s, int& len, SMConditionEntry::eOperator& op)
{
    int depth = 0;
    bool inQuote = false;
    for (int i = 0; i < s.length(); ++i)
    {
        const QChar c = s.at(i);
        if (c == QChar('"'))
        {
            inQuote = !inQuote;
            continue;
        }
        if (inQuote)
        {
            continue;
        }
        if (c == QChar('(')) { ++depth; continue; }
        if (c == QChar(')')) { if (depth > 0) { --depth; } continue; }
        if (depth != 0)
        {
            continue;
        }

        const QChar next = (i + 1 < s.length()) ? s.at(i + 1) : QChar();
        if (next == QChar('='))
        {
            if (c == QChar('=')) { len = 2; op = SMConditionEntry::eOperator::Equal;        return i; }
            if (c == QChar('!')) { len = 2; op = SMConditionEntry::eOperator::NotEqual;     return i; }
            if (c == QChar('<')) { len = 2; op = SMConditionEntry::eOperator::LessEqual;    return i; }
            if (c == QChar('>')) { len = 2; op = SMConditionEntry::eOperator::GreaterEqual; return i; }
        }
        if (c == QChar('<')) { len = 1; op = SMConditionEntry::eOperator::Less;    return i; }
        if (c == QChar('>')) { len = 1; op = SMConditionEntry::eOperator::Greater; return i; }
    }

    len = 0;
    op = SMConditionEntry::eOperator::None;
    return -1;
}

bool SMConditionToken::parseOperand( const QString& token
                                   , SMConditionEntry::eOperandKind& kind
                                   , QString& name
                                   , QList<SMArgumentEntry>* argsOut
                                   , SMConditionEntry* parent
                                   , QString& error)
{
    const QString t = token.trimmed();
    if (t.startsWith(kExprOpen) || t.startsWith(kLambdaOpen))
    {
        error = QStringLiteral("a verbatim expr::/lambda:: is a whole row, not an operand");
        return false;
    }

    const int sep = t.indexOf(kSep);
    if (sep < 0)
    {
        error = QStringLiteral("expected 'prefix::' in '%1'").arg(token);
        return false;
    }

    const QString prefix = t.left(sep);
    const QString rest = t.mid(sep + 2);
    if (prefixToKind(prefix, kind) == false)
    {
        error = QStringLiteral("unknown operand prefix '%1'").arg(prefix);
        return false;
    }

    if (kind == SMArgumentEntry::eValueSource::Condition)
    {
        const int lp = rest.indexOf(QChar('('));
        if (lp >= 0)
        {
            if (rest.endsWith(QChar(')')) == false)
            {
                error = QStringLiteral("cond:: call is missing ')'");
                return false;
            }

            name = rest.left(lp).trimmed();
            const QString inner = rest.mid(lp + 1, rest.length() - lp - 2);
            if (argsOut != nullptr)
            {
                const QStringList argParts = splitTopLevel(inner, QChar(','));
                for (const QString& part : argParts)
                {
                    const QString a = part.trimmed();
                    if (a.isEmpty())
                    {
                        continue;
                    }

                    SMConditionEntry::eOperandKind ak;
                    QString an;
                    if (parseOperand(a, ak, an, nullptr, parent, error) == false)
                    {
                        return false;
                    }

                    if (parent != nullptr)
                    {
                        parent->addArgument(QString(), ak, an);
                    }
                }
            }
        }
        else
        {
            name = rest.trimmed();
        }
    }
    else if (kind == SMArgumentEntry::eValueSource::Value)
    {
        name = unquote(rest.trimmed());
    }
    else
    {
        name = rest.trimmed();
    }

    return true;
}

bool SMConditionToken::parseLeaf(const QString& text, SMConditionEntry& out, QString& error)
{
    error.clear();
    QString s = text.trimmed();

    // Leading '!' is negate, unless it is the start of a '!=' with no left operand.
    bool negate = false;
    if (s.startsWith(QChar('!')) && (s.startsWith(QStringLiteral("!=")) == false))
    {
        negate = true;
        s = s.mid(1).trimmed();
    }
    out.setNegated(negate);

    if (s.startsWith(kExprOpen))
    {
        if (s.endsWith(QChar('}')) == false)
        {
            error = QStringLiteral("expr:: must end with '}'");
            return false;
        }

        out.setLhsKind(SMArgumentEntry::eValueSource::Expression);
        out.setExpression(s.mid(kExprOpen.length(), s.length() - kExprOpen.length() - 1));
        out.setBody(QString());
        out.setOperator(SMConditionEntry::eOperator::None);
        out.setLhs(QString());
        out.getArguments().clear();
        return true;
    }

    if (s.startsWith(kLambdaOpen))
    {
        if (s.endsWith(QChar('}')) == false)
        {
            error = QStringLiteral("lambda:: must end with '}'");
            return false;
        }

        out.setLhsKind(SMArgumentEntry::eValueSource::Lambda);
        out.setBody(s.mid(kLambdaOpen.length(), s.length() - kLambdaOpen.length() - 1));
        out.setExpression(QString());
        out.setOperator(SMConditionEntry::eOperator::None);
        out.setLhs(QString());
        out.getArguments().clear();
        return true;
    }

    int len = 0;
    SMConditionEntry::eOperator op = SMConditionEntry::eOperator::None;
    const int pos = findTopLevelRelop(s, len, op);

    out.getArguments().clear();
    out.setExpression(QString());
    out.setBody(QString());

    if (pos >= 0)
    {
        const QString left = s.left(pos).trimmed();
        const QString right = s.mid(pos + len).trimmed();

        SMConditionEntry::eOperandKind lk;
        QString ln;
        if (parseOperand(left, lk, ln, &out.getArguments(), &out, error) == false)
        {
            return false;
        }

        SMConditionEntry::eOperandKind rk;
        QString rn;
        if (parseOperand(right, rk, rn, nullptr, &out, error) == false)
        {
            return false;
        }

        out.setLhsKind(lk);
        out.setLhs(ln);
        out.setOperator(op);
        out.setRhsKind(rk);
        out.setRhs(rn);
        return true;
    }

    SMConditionEntry::eOperandKind lk;
    QString ln;
    if (parseOperand(s, lk, ln, &out.getArguments(), &out, error) == false)
    {
        return false;
    }

    out.setLhsKind(lk);
    out.setLhs(ln);
    out.setOperator(SMConditionEntry::eOperator::None);
    out.setRhs(QString());
    return true;
}

bool SMConditionToken::hasSingleCombinator(const QString& line, QString& reason)
{
    int depth = 0;
    bool inQuote = false;
    bool hasAnd = false;
    bool hasOr = false;
    for (int i = 0; i + 1 < line.length(); ++i)
    {
        const QChar c = line.at(i);
        if (c == QChar('"'))
        {
            inQuote = !inQuote;
            continue;
        }
        if (inQuote)
        {
            continue;
        }
        if (c == QChar('(')) { ++depth; continue; }
        if (c == QChar(')')) { if (depth > 0) { --depth; } continue; }
        if (depth != 0)
        {
            continue;
        }

        const QChar n = line.at(i + 1);
        if ((c == QChar('&')) && (n == QChar('&'))) { hasAnd = true; ++i; }
        else if ((c == QChar('|')) && (n == QChar('|'))) { hasOr = true; ++i; }
    }

    if (hasAnd && hasOr)
    {
        reason = QStringLiteral("a group mixes && and || at one level; wrap one side in an explicit sub-group");
        return false;
    }

    return true;
}
