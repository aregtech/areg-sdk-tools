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
 *  \file        lusan/model/sm/SMGuardEval.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard what-if evaluator (v7 B9, Try-it strip).
 *
 ************************************************************************/

#include "lusan/model/sm/SMGuardEval.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"

namespace
{
    using eKind  = SMGuardNode::eKind;
    using eTruth = SMGuardEval::eTruth;

    /**
     * \brief   A parsed literal: a number (bools count as 0/1), a string, or nothing.
     *          Comparison follows the literal text, not C++ conversion rules: numbers
     *          compare numerically, anything else (enum members, strings) compares as
     *          case-sensitive text -- deterministic and explainable in a what-if strip.
     **/
    struct Value
    {
        enum class eType { None, Number, Text };

        eType   type    { eType::None };
        double  number  { 0.0 };
        QString text;
    };

    Value parseLiteral(const QString& literal)
    {
        Value value;
        const QString trimmed = literal.trimmed();
        if (trimmed.isEmpty())
        {
            return value;
        }

        if ((trimmed == QStringLiteral("true")) || (trimmed == QStringLiteral("false")))
        {
            value.type   = Value::eType::Number;
            value.number = (trimmed == QStringLiteral("true")) ? 1.0 : 0.0;
            return value;
        }

        bool ok = false;
        // Base 0 accepts decimal, 0x hexadecimal and octal spellings.
        const qlonglong integer = trimmed.toLongLong(&ok, 0);
        if (ok)
        {
            value.type   = Value::eType::Number;
            value.number = static_cast<double>(integer);
            return value;
        }

        const double real = trimmed.toDouble(&ok);
        if (ok)
        {
            value.type   = Value::eType::Number;
            value.number = real;
            return value;
        }

        value.type = Value::eType::Text;
        value.text = trimmed;
        return value;
    }

    eTruth truthOfValue(const Value& value)
    {
        switch (value.type)
        {
        case Value::eType::Number:
            return (value.number != 0.0) ? eTruth::True : eTruth::False;
        case Value::eType::Text:
        case Value::eType::None:
        default:
            return eTruth::Unknown;
        }
    }

    eTruth compare(SMGuardNode::eCmpOp op, const Value& lhs, const Value& rhs)
    {
        if ((lhs.type == Value::eType::None) || (rhs.type == Value::eType::None))
        {
            return eTruth::Unknown;
        }

        int order = 0;
        if ((lhs.type == Value::eType::Number) && (rhs.type == Value::eType::Number))
        {
            order = (lhs.number < rhs.number) ? -1 : ((lhs.number > rhs.number) ? 1 : 0);
        }
        else
        {
            const QString& l = (lhs.type == Value::eType::Text) ? lhs.text : QString::number(lhs.number);
            const QString& r = (rhs.type == Value::eType::Text) ? rhs.text : QString::number(rhs.number);
            order = QString::compare(l, r);
            order = (order < 0) ? -1 : ((order > 0) ? 1 : 0);
        }

        bool result = false;
        switch (op)
        {
        case SMGuardNode::eCmpOp::Eq:   result = (order == 0);  break;
        case SMGuardNode::eCmpOp::Ne:   result = (order != 0);  break;
        case SMGuardNode::eCmpOp::Lt:   result = (order <  0);  break;
        case SMGuardNode::eCmpOp::Le:   result = (order <= 0);  break;
        case SMGuardNode::eCmpOp::Gt:   result = (order >  0);  break;
        case SMGuardNode::eCmpOp::Ge:   result = (order >= 0);  break;
        default:                                                break;
        }

        return result ? eTruth::True : eTruth::False;
    }

    //!< The literal value of an operand-position node; None when not derivable.
    Value operandValue(const StateMachineData& data, const SMGuardNode& node, const SMGuardEval::Inputs& inputs)
    {
        switch (node.getKind())
        {
        case eKind::Lit:
            return parseLiteral(node.getText());

        case eKind::Attr:
        case eKind::Const:
        case eKind::Param:
            return parseLiteral(SMGuardEval::effectiveValue(data, node.getKind(), node.getSymbolId(), inputs));

        default:
            // Calls/lambdas/raw fragments have no tool-side value (they never execute).
            return Value();
        }
    }

    eTruth boolAnd(eTruth lhs, eTruth rhs)
    {
        if ((lhs == eTruth::False) || (rhs == eTruth::False))
        {
            return eTruth::False;
        }

        return ((lhs == eTruth::True) && (rhs == eTruth::True)) ? eTruth::True : eTruth::Unknown;
    }

    eTruth boolOr(eTruth lhs, eTruth rhs)
    {
        if ((lhs == eTruth::True) || (rhs == eTruth::True))
        {
            return eTruth::True;
        }

        return ((lhs == eTruth::False) && (rhs == eTruth::False)) ? eTruth::False : eTruth::Unknown;
    }

    eTruth evalNode(const StateMachineData& data, const SMGuardNode& node, const QList<int>& path
                   , const SMGuardEval::Inputs& inputs, QList<SMGuardEval::NodeTruth>& outNodes)
    {
        eTruth truth = eTruth::Unknown;
        switch (node.getKind())
        {
        case eKind::And:
        case eKind::Or:
        {
            const bool isAnd = (node.getKind() == eKind::And);
            truth = isAnd ? eTruth::True : eTruth::False;
            const QList<SMGuardNode*>& kids = node.getChildren();
            for (int i = 0; i < kids.size(); ++i)
            {
                QList<int> childPath(path);
                childPath.append(i);
                const eTruth childTruth = evalNode(data, *kids.at(i), childPath, inputs, outNodes);
                truth = isAnd ? boolAnd(truth, childTruth) : boolOr(truth, childTruth);
            }
            break;
        }

        case eKind::Not:
        {
            if (node.getCount() == 1)
            {
                QList<int> childPath(path);
                childPath.append(0);
                const eTruth childTruth = evalNode(data, *node.childAt(0), childPath, inputs, outNodes);
                truth = (childTruth == eTruth::Unknown)
                        ? eTruth::Unknown
                        : ((childTruth == eTruth::True) ? eTruth::False : eTruth::True);
            }
            break;
        }

        case eKind::Cmp:
        {
            if (node.getCount() == 2)
            {
                truth = compare(node.getOp()
                               , operandValue(data, *node.childAt(0), inputs)
                               , operandValue(data, *node.childAt(1), inputs));
            }
            break;
        }

        case eKind::Call:
        case eKind::Lambda:
        case eKind::Raw:
        {
            const QString key = SMGuardEval::pathKey(path);
            const auto it = inputs.stubs.constFind(key);
            truth = (it != inputs.stubs.constEnd())
                    ? ((it.value()) ? eTruth::True : eTruth::False)
                    : eTruth::Unknown;
            break;
        }

        case eKind::Attr:
        case eKind::Const:
        case eKind::Param:
        case eKind::Lit:
        default:
            truth = truthOfValue(operandValue(data, node, inputs));
            break;
        }

        outNodes.append(SMGuardEval::NodeTruth{ path, truth });
        return truth;
    }

    void collectStubSites(const SMGuardNode& node, const QList<int>& path, bool booleanPosition
                         , QList<SMGuardEval::StubSite>& sites)
    {
        switch (node.getKind())
        {
        case eKind::Call:
        case eKind::Lambda:
        case eKind::Raw:
            if (booleanPosition)
            {
                sites.append(SMGuardEval::StubSite{ path, node.getKind(), node.getSymbolId(), node.getText() });
            }
            break;

        default:
            break;
        }

        // Call arguments and Cmp operands are value positions; group/Not children stay boolean.
        const bool childrenBoolean = node.isGroup() || (node.getKind() == eKind::Not);
        const QList<SMGuardNode*>& kids = node.getChildren();
        for (int i = 0; i < kids.size(); ++i)
        {
            QList<int> childPath(path);
            childPath.append(i);
            collectStubSites(*kids.at(i), childPath, childrenBoolean, sites);
        }
    }

    void collectReferencedIds(const SMGuardNode& node, eKind kind, QList<uint32_t>& ids)
    {
        if ((node.getKind() == kind) && (ids.contains(node.getSymbolId()) == false))
        {
            ids.append(node.getSymbolId());
        }

        for (const SMGuardNode* child : node.getChildren())
        {
            collectReferencedIds(*child, kind, ids);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// SMGuardEval
//////////////////////////////////////////////////////////////////////////

QString SMGuardEval::pathKey(const QList<int>& path)
{
    QString key(QStringLiteral("root"));
    for (int index : path)
    {
        key += QLatin1Char('_');
        key += QString::number(index);
    }

    return key;
}

QList<SMGuardEval::StubSite> SMGuardEval::stubSites(const SMGuardNode& tree)
{
    QList<StubSite> sites;
    collectStubSites(tree, QList<int>(), true, sites);
    return sites;
}

QList<uint32_t> SMGuardEval::referencedIds(const SMGuardNode& tree, SMGuardNode::eKind kind)
{
    QList<uint32_t> ids;
    collectReferencedIds(tree, kind, ids);
    return ids;
}

QString SMGuardEval::effectiveValue(const StateMachineData& data, SMGuardNode::eKind kind
                                   , uint32_t symbolId, const Inputs& inputs)
{
    const auto it = inputs.values.constFind(symbolId);
    if (it != inputs.values.constEnd())
    {
        return it.value();
    }

    if (kind == eKind::Attr)
    {
        for (const SMAttributeEntry& attr : data.getAttributes().getElements())
        {
            if (attr.getId() == symbolId)
            {
                return attr.getValue();
            }
        }
    }
    else if (kind == eKind::Const)
    {
        for (const ConstantEntry& constant : data.getConstants().getElements())
        {
            if (constant.getId() == symbolId)
            {
                return constant.getValue();
            }
        }
    }

    return QString();
}

SMGuardEval::Result SMGuardEval::evaluate(const StateMachineData& data, const SMGuardNode& tree, const Inputs& inputs)
{
    Result result;
    result.truth = evalNode(data, tree, QList<int>(), inputs, result.nodes);
    return result;
}

SMGuardEval::eTruth SMGuardEval::guardTruth(const StateMachineData& data, const SMGuard& guard, const Inputs& inputs)
{
    if (guard.isEmpty())
    {
        return eTruth::True;
    }

    if (guard.isOk() && (guard.getTree() != nullptr))
    {
        QList<NodeTruth> nodes;
        return evalNode(data, *guard.getTree(), QList<int>(), inputs, nodes);
    }

    return eTruth::Unknown;
}

QString SMGuardEval::stubDisplay(const StateMachineData& data, const SMGuardNode& node, const Inputs& inputs)
{
    switch (node.getKind())
    {
    case eKind::Call:
    {
        const SMMethodEntry* method = SMGuardSymbols::method(data, node.getSymbolId());
        QString display = (method != nullptr) ? method->getName() : QStringLiteral("?");
        display += QLatin1Char('(');
        const QList<SMGuardNode*>& args = node.getChildren();
        for (int i = 0; i < args.size(); ++i)
        {
            if (i > 0)
            {
                display += QStringLiteral(", ");
            }

            const SMGuardNode& arg = *args.at(i);
            QString argText;
            if (arg.isReference())
            {
                argText = effectiveValue(data, arg.getKind(), arg.getSymbolId(), inputs);
            }
            else if (arg.getKind() == eKind::Lit)
            {
                argText = arg.getText();
            }

            display += argText.isEmpty() ? QStringLiteral("?") : argText;
        }

        display += QLatin1Char(')');
        return display;
    }

    case eKind::Lambda:
    {
        QString body = node.getText().simplified();
        if (body.size() > 24)
        {
            body = body.left(21) + QStringLiteral("...");
        }

        return QStringLiteral("{ %1 }").arg(body);
    }

    case eKind::Raw:
    default:
        return node.getText().simplified();
    }
}

QList<uint32_t> SMGuardEval::siblingTransitions(const StateMachineData& data, uint32_t transitionId)
{
    QList<uint32_t> siblings;
    const SMStateEntry* owner = data.findTransitionOwner(transitionId);
    const SMTransitionEntry* self = data.findTransitionById(transitionId);
    if ((owner == nullptr) || (self == nullptr))
    {
        return siblings;
    }

    for (const SMTransitionEntry* transition : owner->getTransitions().getElements())
    {
        if ((transition != nullptr)
            && (transition->getStimulusKind() == self->getStimulusKind())
            && (transition->getStimulus() == self->getStimulus()))
        {
            siblings.append(transition->getId());
        }
    }

    return siblings;
}
