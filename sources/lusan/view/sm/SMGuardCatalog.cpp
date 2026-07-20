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
