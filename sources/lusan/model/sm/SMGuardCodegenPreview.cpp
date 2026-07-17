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
 *  \file        lusan/model/sm/SMGuardCodegenPreview.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard generated-C++ preview (display string only).
 *
 ************************************************************************/

#include "lusan/model/sm/SMGuardCodegenPreview.hpp"

#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"

#include <QStringList>

// The single place the runtime-contract spellings live (D5): a rename of the accessor or the
// data class touches only these constants, and stored guards (IDs) outlive the change.
const char* const SMGuardCodegenPreview::FSM_DATA_QUALIFIER  { "<FsmData>" };
const char* const SMGuardCodegenPreview::HANDLER_ACCESSOR    { "handler()" };
const char* const SMGuardCodegenPreview::LAMBDA_MEMBER_PREFIX{ "m" };

namespace
{
    using eKind = SMGuardNode::eKind;

    int precedence(const SMGuardNode& node)
    {
        switch (node.getKind())
        {
        case eKind::Or:  return 1;
        case eKind::And: return 2;
        case eKind::Cmp: return 3;
        case eKind::Not: return 4;
        default:         return 5;
        }
    }

    const char* cmpText(SMGuardNode::eCmpOp op)
    {
        switch (op)
        {
        case SMGuardNode::eCmpOp::Eq: return "==";
        case SMGuardNode::eCmpOp::Ne: return "!=";
        case SMGuardNode::eCmpOp::Lt: return "<";
        case SMGuardNode::eCmpOp::Le: return "<=";
        case SMGuardNode::eCmpOp::Gt: return ">";
        case SMGuardNode::eCmpOp::Ge: return ">=";
        default:                      return "==";
        }
    }

    class Gen
    {
    public:
        Gen(const StateMachineData& data, uint32_t transitionId)
            : mData(data), mTransId(transitionId)
        {
        }

        QString render(const SMGuardNode& node, int ctx)
        {
            const QString bare = renderBare(node);
            return (precedence(node) < ctx) ? (QStringLiteral("(") + bare + QStringLiteral(")")) : bare;
        }

    private:
        QString renderBare(const SMGuardNode& node)
        {
            switch (node.getKind())
            {
            case eKind::And:    return renderGroup(node, QStringLiteral(" && "), 3);
            case eKind::Or:     return renderGroup(node, QStringLiteral(" || "), 2);
            case eKind::Not:    return QStringLiteral("!") + ((node.getCount() == 1) ? render(*node.childAt(0), 4) : QString());
            case eKind::Cmp:    return renderCmp(node);
            case eKind::Call:   return renderCall(node);
            case eKind::Attr:   return SMGuardSymbols::attributeName(mData, node.getSymbolId()) + QStringLiteral("()");
            case eKind::Const:  return QString::fromLatin1(SMGuardCodegenPreview::FSM_DATA_QUALIFIER) + QStringLiteral("::") + SMGuardSymbols::constantName(mData, node.getSymbolId());
            case eKind::Param:  return SMGuardSymbols::paramName(mData, mTransId, node.getSymbolId());
            case eKind::Lit:    return node.getText();
            case eKind::Lambda: return renderLambda(node);
            case eKind::Raw:    return node.getText();
            default:            return QString();
            }
        }

        QString renderGroup(const SMGuardNode& node, const QString& joiner, int childCtx)
        {
            QStringList parts;
            for (const SMGuardNode* child : node.getChildren())
            {
                parts.append(render(*child, childCtx));
            }

            return parts.join(joiner);
        }

        QString renderCmp(const SMGuardNode& node)
        {
            if (node.getCount() != 2)
            {
                return QString();
            }

            return render(*node.childAt(0), 4) + QStringLiteral(" ") + QString::fromLatin1(cmpText(node.getOp()))
                 + QStringLiteral(" ") + render(*node.childAt(1), 4);
        }

        QString renderCall(const SMGuardNode& node)
        {
            const SMMethodEntry* method = SMGuardSymbols::method(mData, node.getSymbolId());
            const QString name = (method != nullptr) ? method->getName() : QString();

            QStringList args;
            for (const SMGuardNode* arg : node.getChildren())
            {
                args.append(render(*arg, 1));
            }
            const QString argList = args.join(QStringLiteral(", "));

            if ((method != nullptr) && method->isLambdaCondition())
            {
                // A named lambda is generated as a std::function member m<Name> (v7 A.1).
                return QString::fromLatin1(SMGuardCodegenPreview::LAMBDA_MEMBER_PREFIX) + name
                     + QStringLiteral("(") + argList + QStringLiteral(")");
            }

            return QString::fromLatin1(SMGuardCodegenPreview::HANDLER_ACCESSOR) + QStringLiteral(".") + name
                 + QStringLiteral("(") + argList + QStringLiteral(")");
        }

        QString renderLambda(const SMGuardNode& node)
        {
            // An anonymous island is an immediately-invoked lambda over the stimulus args.
            const QStringList names = SMGuardSymbols::paramNames(mData, mTransId);
            const QStringList types = SMGuardSymbols::paramTypes(mData, mTransId);

            QStringList sig;
            for (int i = 0; i < names.size(); ++i)
            {
                const QString type = (i < types.size()) ? types.at(i) : QStringLiteral("auto");
                sig.append(type + QStringLiteral(" ") + names.at(i));
            }

            return QStringLiteral("[this](") + sig.join(QStringLiteral(", ")) + QStringLiteral(") -> bool { ")
                 + node.getText() + QStringLiteral(" }(") + names.join(QStringLiteral(", ")) + QStringLiteral(")");
        }

    private:
        const StateMachineData& mData;
        uint32_t                mTransId;
    };
}

//////////////////////////////////////////////////////////////////////////
// SMGuardCodegenPreview
//////////////////////////////////////////////////////////////////////////

QString SMGuardCodegenPreview::expression(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node)
{
    Gen gen(data, transitionId);
    return gen.render(node, 1);
}

QString SMGuardCodegenPreview::ifStatement(const StateMachineData& data, uint32_t transitionId, const SMGuard& guard)
{
    if ((guard.isOk() == false) || (guard.getTree() == nullptr))
    {
        return QString();
    }

    return QStringLiteral("if (") + expression(data, transitionId, *guard.getTree()) + QStringLiteral(")");
}
