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

#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"

#include <QHash>
#include <QSet>

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
        QList<SMGuardRender::Chip>          mChips;
        QSet<QString>                       mCollisions;    //!< Display names carried by >1 kind.
        bool                                mGhosts { false };//!< Emit `<name>` ghosts (display mode).
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

        //!< Appends a reference token (its rendered text \p shown, which may carry a `@kind:`
        //!< disambiguation prefix) and records a Chip over that span so the field can fold it.
        void appendChip(const QString& shown, eRole role, const QString& kind, const QString& name)
        {
            SMGuardRender::Chip chip;
            chip.start  = static_cast<int>(mOut.size());
            chip.length = static_cast<int>(shown.size());
            chip.role   = role;
            chip.kind   = kind;
            chip.name   = name;
            chip.reveal = mCollisions.contains(name);
            mChips.append(chip);
            appendRole(shown, role);
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

        //!< Computes the same-name collision set, then renders the whole tree from the root.
        void renderRoot(const SMGuardNode& node)
        {
            QHash<QString, QString> seen;
            gatherRefs(node, seen);
            render(node, 1);
        }

    private:
        //!< The display name + kind word ("param"/"attr"/"const"/"cond") of a reference node.
        bool refNameKind(const SMGuardNode& node, QString& name, QString& kind) const
        {
            switch (node.getKind())
            {
            case eKind::Attr:  name = SMGuardSymbols::attributeName(mData, node.getSymbolId());        kind = QStringLiteral("attr");  return true;
            case eKind::Const: name = SMGuardSymbols::constantName(mData, node.getSymbolId());         kind = QStringLiteral("const"); return true;
            case eKind::Param: name = SMGuardSymbols::paramName(mData, mTransId, node.getSymbolId());  kind = QStringLiteral("param"); return true;
            case eKind::Call:
                {
                    const SMMethodEntry* method = SMGuardSymbols::method(mData, node.getSymbolId());
                    name = (method != nullptr) ? method->getName() : QString();
                    kind = QStringLiteral("cond");
                    return true;
                }
            default:
                return false;
            }
        }

        //!< Walks the tree flagging any display name that is carried by more than one kind.
        void gatherRefs(const SMGuardNode& node, QHash<QString, QString>& seen)
        {
            QString name;
            QString kind;
            if (refNameKind(node, name, kind) && (name.isEmpty() == false))
            {
                const auto it = seen.constFind(name);
                if (it == seen.constEnd())      { seen.insert(name, kind); }
                else if (it.value() != kind)    { mCollisions.insert(name); }
            }

            for (const SMGuardNode* child : node.getChildren())
            {
                gatherRefs(*child, seen);
            }
        }

        //!< The `@kind:` prefix a reference needs so a bare re-parse would not bind a different
        //!< symbol (minimal disambiguation, like minimal parentheses). A bare name binds
        //!< param > attr > const, so a Param never needs a prefix; an Attr needs `@attr:` only
        //!< when a same-named parameter shadows it; a Const needs `@const:` when a parameter or
        //!< attribute shares its name. This keeps the common guard bare and makes a same-name /
        //!< different-kind guard round-trip (parse(render(tree)) == tree).
        QString kindPrefix(eKind kind, const QString& name) const
        {
            switch (kind)
            {
            case eKind::Attr:
                return (SMGuardSymbols::paramId(mData, mTransId, name) != 0u)
                     ? NEGuardText::refPrefix(QStringLiteral("attr")) : QString();
            case eKind::Const:
                return ((SMGuardSymbols::paramId(mData, mTransId, name) != 0u)
                     || (SMGuardSymbols::attributeId(mData, name) != 0u))
                     ? NEGuardText::refPrefix(QStringLiteral("const")) : QString();
            default:
                return QString();
            }
        }

        void renderBare(const SMGuardNode& node)
        {
            switch (node.getKind())
            {
            case eKind::And:    renderGroup(node, QStringLiteral(" && "), 3); break;
            case eKind::Or:     renderGroup(node, QStringLiteral(" || "), 2); break;
            case eKind::Not:    renderNot(node);   break;
            case eKind::Cmp:    renderCmp(node);   break;
            case eKind::Call:   renderCall(node);  break;
            case eKind::Attr:
                {
                    const QString name = SMGuardSymbols::attributeName(mData, node.getSymbolId());
                    appendChip(kindPrefix(eKind::Attr, name) + name, eRole::Fsm, QStringLiteral("attr"), name);
                }
                break;
            case eKind::Const:
                {
                    const QString name = SMGuardSymbols::constantName(mData, node.getSymbolId());
                    appendChip(kindPrefix(eKind::Const, name) + name, eRole::Fsm, QStringLiteral("const"), name);
                }
                break;
            case eKind::Param:
                {
                    const QString name = SMGuardSymbols::paramName(mData, mTransId, node.getSymbolId());
                    appendChip(name, eRole::Stim, QStringLiteral("param"), name);
                }
                break;
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
            // The callee name is a chip (kind "cond"); its argument list follows as ordinary text.
            appendChip(name, role, QStringLiteral("cond"), name);
            append(QStringLiteral("("));

            const QList<SMGuardNode*>& args = node.getChildren();
            if (method == nullptr)
            {
                for (int i = 0; i < args.size(); ++i)
                {
                    if (i > 0) { appendRole(QStringLiteral(", "), eRole::Punct); }
                    renderChild(*args.at(i), i, 1);
                }
                append(QStringLiteral(")"));
                return;
            }

            renderArgs(node, method);
            append(QStringLiteral(")"));
        }

        /**
         * \brief   Renders a call's arguments in SIGNATURE order: each formal's
         *          bound value in place, an unmapped formal as a labeled ghost `<name>` (only in
         *          the ghost/display mode -- canonical text omits it so it round-trips), and any
         *          value mapped AFTER a hole as the named `@arg:name = value` so its target slot
         *          is unambiguous. Never an empty comma. Arguments key on the formal's id
         *          (Option A); a legacy arg with id 0 falls back to position.
         **/
        void renderArgs(const SMGuardNode& node, const SMMethodEntry* method)
        {
            const QList<SMGuardNode*>& args = node.getChildren();
            const QList<MethodParameter>& formals = method->getElements();

            QHash<uint32_t, int> childByFormal;
            QList<int>           positionalFallback;   // arg children with id 0, consumed in order
            for (int i = 0; i < args.size(); ++i)
            {
                const uint32_t fid = args.at(i)->getArgFormalId();
                if (fid != 0u) { childByFormal.insert(fid, i); }
                else           { positionalFallback.append(i); }
            }

            int  fallbackCursor = 0;
            bool holeSeen = false;
            bool first    = true;
            for (int fi = 0; fi < formals.size(); ++fi)
            {
                const uint32_t fid = formals.at(fi).getId();
                int childIndex = childByFormal.value(fid, -1);
                if ((childIndex < 0) && (fallbackCursor < positionalFallback.size()))
                {
                    childIndex = positionalFallback.at(fallbackCursor++);
                }

                if (childIndex < 0)
                {
                    // Unmapped formal. In display (ghost) mode show a labeled amber ghost; the
                    // canonical text omits it entirely (no empty comma, and it round-trips).
                    holeSeen = true;
                    if (mGhosts)
                    {
                        if (first == false) { appendRole(QStringLiteral(", "), eRole::Punct); }
                        first = false;
                        appendRole(QLatin1Char('<') + formals.at(fi).getName() + QLatin1Char('>'), eRole::Raw);
                    }
                    continue;
                }

                if (first == false) { appendRole(QStringLiteral(", "), eRole::Punct); }
                first = false;
                if (holeSeen)
                {
                    appendRole(NEGuardText::refPrefix(QStringLiteral("arg")) + formals.at(fi).getName() + QStringLiteral(" = "), eRole::Punct);
                }
                renderChild(*args.at(childIndex), childIndex, 1);
            }
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
    builder.renderRoot(node);
    return builder.mOut;
}

SMGuardRender::Rendered SMGuardRender::render(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node)
{
    Builder builder(data, transitionId);
    builder.mGhosts = true;     // the field display shows labeled ghosts for unmapped formals
    builder.renderRoot(node);
    Rendered result;
    result.text  = builder.mOut;
    result.spans = builder.mSpans;
    result.chips = builder.mChips;
    return result;
}

QList<SMGuardRender::NodeSpan> SMGuardRender::nodeSpans(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node)
{
    QList<NodeSpan> spans;
    Builder builder(data, transitionId);
    builder.mNodeSpans = &spans;
    builder.renderRoot(node);
    return spans;
}

namespace
{
    //!< The canvas summary of one node (see SMGuardRender::canvasSummary).
    QString summarize(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node)
    {
        using eKind = SMGuardNode::eKind;
        const QList<SMGuardNode*>& kids = node.getChildren();

        switch (node.getKind())
        {
        case eKind::And:
        case eKind::Or:
        {
            const QString joiner = (node.getKind() == eKind::And) ? QStringLiteral(" && ") : QStringLiteral(" || ");
            QStringList parts;
            for (const SMGuardNode* kid : kids)
            {
                if (kid != nullptr)
                {
                    parts.append(summarize(data, transitionId, *kid));
                }
            }

            return parts.join(joiner);
        }

        case eKind::Not:
            return (node.getCount() == 1)
                 ? (QStringLiteral("!") + summarize(data, transitionId, *node.childAt(0)))
                 : QString();

        case eKind::Cmp:
            return (node.getCount() == 2)
                 ? (summarize(data, transitionId, *node.childAt(0))
                    + QLatin1Char(' ') + QString::fromLatin1(cmpText(node.getOp())) + QLatin1Char(' ')
                    + summarize(data, transitionId, *node.childAt(1)))
                 : QString();

        case eKind::Call:
        {
            const SMMethodEntry* method = SMGuardSymbols::method(data, node.getSymbolId());
            if (method == nullptr)
            {
                return QStringLiteral("?()");
            }

            // A lambda condition captures its scope (`[this]` / `[&]`), so it is generated and
            // called WITHOUT arguments -- there is no argument list to collapse.
            if (method->isLambdaCondition())
            {
                return method->getName() + QStringLiteral("()");
            }

            return method->getName() + (kids.isEmpty() ? QStringLiteral("()") : QStringLiteral("(...)"));
        }

        case eKind::Attr:   return SMGuardSymbols::attributeName(data, node.getSymbolId());
        case eKind::Const:  return SMGuardSymbols::constantName(data, node.getSymbolId());
        case eKind::Param:  return SMGuardSymbols::paramName(data, transitionId, node.getSymbolId());
        case eKind::Lit:    return node.getText();

        // An inline block is an anonymous `[this]() -> bool { ... }`; its body says nothing at
        // canvas size, so only what it IS is shown.
        case eKind::Lambda: return QStringLiteral("this -> bool");

        // Raw C++ is not a lambda and has no signature to show.
        case eKind::Raw:    return QStringLiteral("{...}");

        default:            return QString();
        }
    }
}

QString SMGuardRender::canvasSummary(const StateMachineData& data, uint32_t transitionId, const SMGuard& guard)
{
    if (guard.isDraft())
    {
        return guard.getDraftText().simplified();
    }

    if (guard.isOk() && (guard.getTree() != nullptr))
    {
        return summarize(data, transitionId, *guard.getTree()).simplified();
    }

    return QString();
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
