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
 *  \file        lusan/model/sm/SMGuardRender.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard renderer: AST -> canonical text + span roles.
 *
 ************************************************************************/

#include "lusan/model/sm/SMGuardRender.hpp"

#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"

namespace
{
    using eKind = SMGuardNode::eKind;
    using eRole = SMGuardRender::eRole;
    using Span  = SMGuardRender::Span;

    //!< The binding precedence of a node (loosest = 1); used to place minimal parentheses.
    int precedence(const SMGuardNode& node)
    {
        switch (node.getKind())
        {
        case eKind::Or:  return 1;
        case eKind::And: return 2;
        case eKind::Cmp: return 3;
        case eKind::Not: return 4;
        default:         return 5;  // calls and leaves bind tightest
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

    //!< Accumulates the rendered text and records the owner span of each name/literal token.
    class Builder
    {
    public:
        Builder(const StateMachineData& data, uint32_t transitionId)
            : mData(data), mTransId(transitionId)
        {
        }

        QString                             mOut;
        QList<Span>                         mSpans;
        QList<SMGuardRender::NodeSpan>*     mNodeSpans { nullptr };
        QList<int>                          mPath;

        void append(const QString& s)
        {
            mOut += s;
        }

        void appendRole(const QString& s, eRole role)
        {
            mSpans.append({ static_cast<int>(mOut.size()), static_cast<int>(s.size()), role });
            mOut += s;
        }

        //!< Renders \p node, wrapping it in parentheses when its precedence is below \p ctx.
        void render(const SMGuardNode& node, int ctx)
        {
            const int spanStart = static_cast<int>(mOut.size());
            const bool wrap = (precedence(node) < ctx);
            if (wrap) { append(QStringLiteral("(")); }
            renderBare(node);
            if (wrap) { append(QStringLiteral(")")); }
            if (mNodeSpans != nullptr)
            {
                mNodeSpans->append({ mPath, spanStart, static_cast<int>(mOut.size()) - spanStart });
            }
        }

        //!< Renders child \p index of the current node, tracking the node path.
        void renderChild(const SMGuardNode& child, int index, int ctx)
        {
            mPath.append(index);
            render(child, ctx);
            mPath.removeLast();
        }

    private:
        void renderBare(const SMGuardNode& node)
        {
            switch (node.getKind())
            {
            case eKind::And:    renderGroup(node, QStringLiteral(" && "), 3); break;
            case eKind::Or:     renderGroup(node, QStringLiteral(" || "), 2); break;
            case eKind::Not:    renderNot(node);   break;
            case eKind::Cmp:    renderCmp(node);   break;
            case eKind::Call:   renderCall(node);  break;
            case eKind::Attr:   appendRole(SMGuardSymbols::attributeName(mData, node.getSymbolId()), eRole::Fsm);  break;
            case eKind::Const:  appendRole(SMGuardSymbols::constantName(mData, node.getSymbolId()), eRole::Fsm);   break;
            case eKind::Param:  appendRole(SMGuardSymbols::paramName(mData, mTransId, node.getSymbolId()), eRole::Stim); break;
            case eKind::Lit:    appendRole(node.getText(), eRole::Literal); break;
            case eKind::Lambda: appendRole(QStringLiteral("{") + node.getText() + QStringLiteral("}"), eRole::Lambda); break;
            case eKind::Raw:    appendRole(node.getText(), eRole::Raw); break;
            default:            break;
            }
        }

        void renderGroup(const SMGuardNode& node, const QString& joiner, int childCtx)
        {
            const QList<SMGuardNode*>& kids = node.getChildren();
            for (int i = 0; i < kids.size(); ++i)
            {
                if (i > 0) { appendRole(joiner, eRole::Operator); }
                renderChild(*kids.at(i), i, childCtx);
            }
        }

        void renderNot(const SMGuardNode& node)
        {
            appendRole(QStringLiteral("!"), eRole::Operator);
            if (node.getCount() == 1)
            {
                renderChild(*node.childAt(0), 0, 4);
            }
        }

        void renderCmp(const SMGuardNode& node)
        {
            if (node.getCount() == 2)
            {
                renderChild(*node.childAt(0), 0, 4);
                append(QStringLiteral(" "));
                appendRole(QString::fromLatin1(cmpText(node.getOp())), eRole::Operator);
                append(QStringLiteral(" "));
                renderChild(*node.childAt(1), 1, 4);
            }
        }

        void renderCall(const SMGuardNode& node)
        {
            const SMMethodEntry* method = SMGuardSymbols::method(mData, node.getSymbolId());
            const QString name = (method != nullptr) ? method->getName() : QString();
            // A named-lambda call is an FSM symbol (teal); a handler condition is a handler (orange).
            const eRole role = ((method != nullptr) && method->isLambdaCondition()) ? eRole::Fsm : eRole::Handler;
            appendRole(name, role);
            append(QStringLiteral("("));
            const QList<SMGuardNode*>& args = node.getChildren();
            for (int i = 0; i < args.size(); ++i)
            {
                if (i > 0) { appendRole(QStringLiteral(", "), eRole::Punct); }
                renderChild(*args.at(i), i, 1);
            }
            append(QStringLiteral(")"));
        }

    private:
        const StateMachineData& mData;
        uint32_t                mTransId;
    };
}

//////////////////////////////////////////////////////////////////////////
// SMGuardRender
//////////////////////////////////////////////////////////////////////////

QString SMGuardRender::text(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node)
{
    Builder builder(data, transitionId);
    builder.render(node, 1);
    return builder.mOut;
}

SMGuardRender::Rendered SMGuardRender::render(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node)
{
    Builder builder(data, transitionId);
    builder.render(node, 1);
    Rendered result;
    result.text  = builder.mOut;
    result.spans = builder.mSpans;
    return result;
}

QList<SMGuardRender::NodeSpan> SMGuardRender::nodeSpans(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node)
{
    QList<NodeSpan> spans;
    Builder builder(data, transitionId);
    builder.mNodeSpans = &spans;
    builder.render(node, 1);
    return spans;
}

QString SMGuardRender::guardText(const StateMachineData& data, uint32_t transitionId, const SMGuard& guard)
{
    if (guard.isDraft())
    {
        return guard.getDraftText();
    }
    if (guard.isOk() && (guard.getTree() != nullptr))
    {
        return text(data, transitionId, *guard.getTree());
    }

    return QString();
}
