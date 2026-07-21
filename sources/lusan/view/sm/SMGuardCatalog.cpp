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
 *  \file        lusan/view/sm/SMGuardCatalog.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard editable-surface symbol catalog (B4/B5 source).
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardCatalog.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/sm/SMGuardSymbols.hpp"

#include <QVector>

namespace
{
    //!< The Levenshtein edit distance between two strings (case-insensitive).
    int editDistance(const QString& a, const QString& b)
    {
        const int la = a.size();
        const int lb = b.size();
        if (la == 0) { return lb; }
        if (lb == 0) { return la; }

        QVector<int> prev(lb + 1);
        QVector<int> curr(lb + 1);
        for (int j = 0; j <= lb; ++j) { prev[j] = j; }

        for (int i = 1; i <= la; ++i)
        {
            curr[0] = i;
            for (int j = 1; j <= lb; ++j)
            {
                const int cost = (a.at(i - 1).toLower() == b.at(j - 1).toLower()) ? 0 : 1;
                curr[j] = qMin(qMin(curr[j - 1] + 1, prev[j] + 1), prev[j - 1] + cost);
            }

            prev = curr;
        }

        return prev[lb];
    }
}

QString SMGuardSymbol::display() const
{
    if (isCall == false)
    {
        return name;
    }

    return name + QLatin1Char('(') + paramTypes.join(QStringLiteral(", ")) + QLatin1Char(')');
}

QString SMGuardSymbol::kindWord() const
{
    switch (refkind)
    {
    case eRefKind::Param:   return QStringLiteral("param");
    case eRefKind::Attr:    return QStringLiteral("attr");
    case eRefKind::Const:   return QStringLiteral("const");
    case eRefKind::Cond:
    default:                return QStringLiteral("cond");
    }
}

QString SMGuardSymbol::mention() const
{
    return QLatin1Char('@') + kindWord() + QLatin1Char(':') + name;
}

QString SMGuardSymbol::kindNoun() const
{
    switch (refkind)
    {
    case eRefKind::Param:   return QStringLiteral("parameter");
    case eRefKind::Attr:    return QStringLiteral("attribute");
    case eRefKind::Const:   return QStringLiteral("constant");
    case eRefKind::Cond:
    default:                return QStringLiteral("condition");
    }
}

QList<SMGuardSymbol> SMGuardCatalog::build(const StateMachineData& data, uint32_t transitionId)
{
    QList<SMGuardSymbol> result;

    // STIMULUS -- the in-scope trigger/event parameters (blue).
    const QStringList paramNames = SMGuardSymbols::paramNames(data, transitionId);
    const QStringList paramTypes = SMGuardSymbols::paramTypes(data, transitionId);
    for (int i = 0; i < paramNames.size(); ++i)
    {
        SMGuardSymbol sym;
        sym.name        = paramNames.at(i);
        sym.glyph       = QStringLiteral("a");
        sym.owner       = NEGuardStyle::eOwner::Stimulus;
        sym.refkind     = SMGuardSymbol::eRefKind::Param;
        sym.typeText    = (i < paramTypes.size()) ? paramTypes.at(i) : QString();
        sym.isCall      = false;
        sym.symbolId    = SMGuardSymbols::paramId(data, transitionId, sym.name);
        result.append(sym);
    }

    // FSM -- attributes (#), constants (K), named lambdas ({}), all teal.
    for (const SMAttributeEntry& attr : data.getAttributes().getElements())
    {
        SMGuardSymbol sym;
        sym.name        = attr.getName();
        sym.glyph       = QStringLiteral("#");
        sym.owner       = NEGuardStyle::eOwner::Fsm;
        sym.refkind     = SMGuardSymbol::eRefKind::Attr;
        sym.typeText    = attr.getType();
        sym.isCall      = false;
        sym.symbolId    = attr.getId();
        result.append(sym);
    }

    for (const ConstantEntry& constant : data.getConstants().getElements())
    {
        SMGuardSymbol sym;
        sym.name        = constant.getName();
        sym.glyph       = QStringLiteral("K");
        sym.owner       = NEGuardStyle::eOwner::Fsm;
        sym.refkind     = SMGuardSymbol::eRefKind::Const;
        sym.typeText    = constant.getType();
        sym.provenance  = QStringLiteral("= ") + constant.getValue();
        sym.isCall      = false;
        sym.symbolId    = constant.getId();
        result.append(sym);
    }

    // Condition methods: lambdas stay in the FSM group, handlers in their own (D8).
    QList<SMGuardSymbol> handlers;
    for (const SMMethodEntry* method : data.getMethods().getElements())
    {
        if ((method == nullptr) || (method->isCondition() == false))
        {
            continue;
        }

        SMGuardSymbol sym;
        sym.name        = method->getName();
        sym.owner       = method->isLambdaCondition() ? NEGuardStyle::eOwner::Fsm : NEGuardStyle::eOwner::Handler;
        sym.refkind     = SMGuardSymbol::eRefKind::Cond;
        sym.glyph       = method->isLambdaCondition() ? QStringLiteral("{}") : QStringLiteral("h");
        sym.typeText    = method->getReturn();
        sym.provenance  = method->isLambdaCondition() ? QStringLiteral("lambda") : QStringLiteral("handler()");
        sym.symbolId    = method->getId();
        for (const MethodParameter& param : method->getElements())
        {
            sym.paramNames.append(param.getName());
            sym.paramTypes.append(param.getType());
        }
        sym.isCall = (sym.paramNames.isEmpty() == false);

        if (method->isLambdaCondition())
        {
            result.append(sym);
        }
        else
        {
            handlers.append(sym);
        }
    }

    result.append(handlers);
    return result;
}

QStringList SMGuardCatalog::completionWords(const StateMachineData& data, uint32_t transitionId)
{
    QStringList words;
    for (const SMGuardSymbol& sym : build(data, transitionId))
    {
        words.append(sym.name);
    }

    return words;
}

QString SMGuardCatalog::nearestName(const QStringList& candidates, const QString& typed)
{
    if (typed.isEmpty())
    {
        return QString();
    }

    // Threshold: at most 2 edits and never more than a third of the typed length.
    const int limit = qMin(2, qMax(1, typed.size() / 3));
    QString best;
    int bestDistance = limit + 1;
    for (const QString& candidate : candidates)
    {
        const int distance = editDistance(candidate, typed);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = candidate;
        }
    }

    return (bestDistance <= limit) ? best : QString();
}

namespace
{
    //!< Accumulates bound-reference counts by symbol id over a guard sub-tree.
    void accumulateUseCounts(const SMGuardNode* node, QHash<uint32_t, int>& counts)
    {
        if (node == nullptr)
        {
            return;
        }

        const uint32_t id = node->getSymbolId();
        if ((id != 0u) && ((node->getKind() == SMGuardNode::eKind::Call) || node->isReference()))
        {
            counts[id] += 1;
        }

        for (const SMGuardNode* child : node->getChildren())
        {
            accumulateUseCounts(child, counts);
        }
    }
}

QHash<uint32_t, int> SMGuardCatalog::useCounts(const SMGuardNode* tree)
{
    QHash<uint32_t, int> counts;
    accumulateUseCounts(tree, counts);
    return counts;
}

namespace
{
    //!< True when \p text is exactly one bare C++ identifier: [A-Za-z_][A-Za-z0-9_]* and nothing
    //!< else. This is the ONLY shape W1 flags -- operators, calls, member access, literals, and
    //!< any span with a space are never bare identifiers, so the raw escape hatch stays unchecked.
    bool isBareIdentifier(const QString& text)
    {
        if (text.isEmpty())
        {
            return false;
        }

        const QChar first = text.at(0);
        if ((first.isLetter() == false) && (first != QLatin1Char('_')))
        {
            return false;
        }

        for (int i = 1; i < text.size(); ++i)
        {
            const QChar c = text.at(i);
            if ((c.isLetterOrNumber() == false) && (c != QLatin1Char('_')))
            {
                return false;
            }
        }

        return true;
    }

    //!< Pre-order walk collecting the bare-identifier Raw nodes and their child-index paths.
    void collectRawIdentifiers(const SMGuardNode* node, const QList<int>& path, QList<SMGuardRawCollision>& candidates)
    {
        if (node == nullptr)
        {
            return;
        }

        if (node->getKind() == SMGuardNode::eKind::Raw)
        {
            const QString name = node->getText().trimmed();
            if (isBareIdentifier(name))
            {
                SMGuardRawCollision candidate;
                candidate.path = path;
                candidate.name = name;
                candidates.append(candidate);
            }
        }

        const QList<SMGuardNode*>& kids = node->getChildren();
        for (int i = 0; i < kids.size(); ++i)
        {
            QList<int> childPath = path;
            childPath.append(i);
            collectRawIdentifiers(kids.at(i), childPath, candidates);
        }
    }
}

QList<SMGuardRawCollision> SMGuardCatalog::rawCollisions(const StateMachineData& data, uint32_t transitionId, const SMGuardNode* tree)
{
    QList<SMGuardRawCollision> out;
    if (tree == nullptr)
    {
        return out;
    }

    // Cheap first pass over the tree; the common bound guard has no bare-identifier raw nodes and
    // never pays for a catalog build (perf: this runs on every projection pass).
    QList<SMGuardRawCollision> candidates;
    collectRawIdentifiers(tree, QList<int>(), candidates);
    if (candidates.isEmpty())
    {
        return out;
    }

    // Match each candidate name against the in-scope symbols (closed world, D1); a name carried by
    // more than one kind keeps all matches so the caller opens the disambiguation picker.
    const QList<SMGuardSymbol> catalog = SMGuardCatalog::build(data, transitionId);
    for (SMGuardRawCollision& candidate : candidates)
    {
        for (const SMGuardSymbol& sym : catalog)
        {
            if (sym.name == candidate.name)
            {
                candidate.matches.append(sym);
            }
        }

        if (candidate.matches.isEmpty() == false)
        {
            out.append(candidate);
        }
    }

    return out;
}
