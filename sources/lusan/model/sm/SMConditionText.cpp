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
 *  \file        lusan/model/sm/SMConditionText.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM condition tree text/codegen renderer.
 *
 ************************************************************************/

#include "lusan/model/sm/SMConditionText.hpp"

#include <QStringList>

QString SMConditionText::preview(const SMConditionGroup& root)
{
    return renderGroup(root, eMode::Preview, QString(), true);
}

QString SMConditionText::cpp(const SMConditionGroup& root, const QString& dataClass)
{
    return renderGroup(root, eMode::Cpp, dataClass, true);
}

QString SMConditionText::summary(const SMConditionGroup& root)
{
    return root.isEmpty() ? QString() : preview(root);
}

QString SMConditionText::lowerFirst(const QString& name)
{
    if (name.isEmpty())
        return name;

    return name.left(1).toLower() + name.mid(1);
}

QString SMConditionText::opSymbol(SMConditionEntry::eOperator op)
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

QString SMConditionText::renderGroup(const SMConditionGroup& group, eMode mode, const QString& dataClass, bool isRoot)
{
    QStringList parts;
    for (const SMConditionNode* child : group.getChildren())
    {
        if (child->getNodeKind() == SMConditionNode::eNodeKind::Leaf)
        {
            parts << renderLeaf(*static_cast<const SMConditionEntry*>(child), mode, dataClass);
        }
        else
        {
            parts << renderGroup(*static_cast<const SMConditionGroup*>(child), mode, dataClass, false);
        }
    }

    const QString joiner = (group.getCombine() == SMConditionGroup::eCombine::Or)
                                ? QStringLiteral(" || ")
                                : QStringLiteral(" && ");
    QString body = parts.join(joiner);

    // A nested group is always parenthesized so short-circuit order is explicit; a negated
    // group is parenthesized too so the leading '!' binds to the whole group.
    if ((isRoot == false) || group.isNegated())
    {
        body = QStringLiteral("(") + body + QStringLiteral(")");
    }

    if (group.isNegated())
    {
        body = QStringLiteral("!") + body;
    }

    return body;
}

QString SMConditionText::renderLeaf(const SMConditionEntry& leaf, eMode mode, const QString& dataClass)
{
    // Verbatim rows: pasted as-is, parenthesized as an operand.
    if (leaf.isExpressionRow())
    {
        QString core = QStringLiteral("(") + leaf.getExpression() + QStringLiteral(")");
        return leaf.isNegated() ? (QStringLiteral("!") + core) : core;
    }

    if (leaf.isLambdaRow())
    {
        QString core;
        if (mode == eMode::Cpp)
        {
            core = QStringLiteral("[this]() -> bool { ") + leaf.getBody() + QStringLiteral(" }()");
        }
        else
        {
            core = QStringLiteral("{...}");
        }

        return leaf.isNegated() ? (QStringLiteral("!") + core) : core;
    }

    const QString lhs = renderOperand(leaf.getLhsKind(), leaf.getLhs(), leaf.getArguments(), mode, dataClass);

    QString core;
    if (leaf.hasOperator())
    {
        const QString rhs = renderOperand(leaf.getRhsKind(), leaf.getRhs(), QList<SMArgumentEntry>(), mode, dataClass);
        core = lhs + QStringLiteral(" ") + opSymbol(leaf.getOperator()) + QStringLiteral(" ") + rhs;

        // A negated comparison is wrapped so the '!' binds to the whole comparison.
        return leaf.isNegated() ? (QStringLiteral("!(") + core + QStringLiteral(")")) : core;
    }

    // Boolean test of a single operand.
    core = lhs;
    return leaf.isNegated() ? (QStringLiteral("!") + core) : core;
}

QString SMConditionText::renderOperand( SMConditionEntry::eOperandKind kind
                                      , const QString& name
                                      , const QList<SMArgumentEntry>& args
                                      , eMode mode
                                      , const QString& dataClass)
{
    switch (kind)
    {
    case SMArgumentEntry::eValueSource::Param:
        return name;

    case SMArgumentEntry::eValueSource::Attribute:
        return (mode == eMode::Cpp) ? (lowerFirst(name) + QStringLiteral("()")) : name;

    case SMArgumentEntry::eValueSource::Constant:
        return (mode == eMode::Cpp) ? (dataClass + QStringLiteral("::") + name) : name;

    case SMArgumentEntry::eValueSource::Condition:
    {
        const QString call = (mode == eMode::Cpp) ? lowerFirst(name) : name;
        return call + QStringLiteral("(") + renderArgs(args, mode, dataClass) + QStringLiteral(")");
    }

    case SMArgumentEntry::eValueSource::Expression:
        return QStringLiteral("(") + name + QStringLiteral(")");

    case SMArgumentEntry::eValueSource::Value:
    case SMArgumentEntry::eValueSource::Lambda:
    default:
        return name;
    }
}

QString SMConditionText::renderArgs(const QList<SMArgumentEntry>& args, eMode mode, const QString& dataClass)
{
    QStringList out;
    for (const SMArgumentEntry& arg : args)
    {
        out << renderArgOperand(arg.getSource(), arg.getValue(), arg.getExpression(), mode, dataClass);
    }

    return out.join(QStringLiteral(", "));
}

QString SMConditionText::renderArgOperand( SMArgumentEntry::eValueSource source
                                         , const QString& value
                                         , const QString& expression
                                         , eMode mode
                                         , const QString& dataClass)
{
    switch (source)
    {
    case SMArgumentEntry::eValueSource::Param:
        return value;

    case SMArgumentEntry::eValueSource::Attribute:
        return (mode == eMode::Cpp) ? (lowerFirst(value) + QStringLiteral("()")) : value;

    case SMArgumentEntry::eValueSource::Constant:
        return (mode == eMode::Cpp) ? (dataClass + QStringLiteral("::") + value) : value;

    case SMArgumentEntry::eValueSource::Condition:
        return (mode == eMode::Cpp) ? (lowerFirst(value) + QStringLiteral("()"))
                                    : (value + QStringLiteral("()"));

    case SMArgumentEntry::eValueSource::Expression:
        return QStringLiteral("(") + expression + QStringLiteral(")");

    case SMArgumentEntry::eValueSource::Value:
    case SMArgumentEntry::eValueSource::Lambda:
    default:
        return value;
    }
}
