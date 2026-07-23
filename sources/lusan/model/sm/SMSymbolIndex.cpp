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
 *  \file        lusan/model/sm/SMSymbolIndex.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM verbatim-code symbol assistance.
 *
 ************************************************************************/

#include "lusan/model/sm/SMSymbolIndex.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

namespace
{
    using VerbatimBlock = SMSymbolIndex::VerbatimBlock;

    /**
     * \brief   Extracts the identifier tokens from verbatim C++ text. Comments and
     *          string/char literals are skipped, and a token that is a member access
     *          (`obj.name`, `p->name`) is dropped, so only free identifiers survive.
     *          This is a scan, not a parse: no grammar, no types.
     **/
    QStringList identifierTokens(const QString& text)
    {
        QStringList tokens;
        const int n = text.size();
        int i = 0;
        while (i < n)
        {
            const QChar c = text.at(i);

            if ((c == QLatin1Char('/')) && (i + 1 < n) && (text.at(i + 1) == QLatin1Char('/')))
            {
                i += 2;
                while ((i < n) && (text.at(i) != QLatin1Char('\n')))
                {
                    ++i;
                }
                continue;
            }

            if ((c == QLatin1Char('/')) && (i + 1 < n) && (text.at(i + 1) == QLatin1Char('*')))
            {
                i += 2;
                while ((i + 1 < n) && !((text.at(i) == QLatin1Char('*')) && (text.at(i + 1) == QLatin1Char('/'))))
                {
                    ++i;
                }
                i += 2;
                continue;
            }

            if (c == QLatin1Char('"'))
            {
                ++i;
                while (i < n)
                {
                    if (text.at(i) == QLatin1Char('\\')) { i += 2; continue; }
                    if (text.at(i) == QLatin1Char('"')) { ++i; break; }
                    ++i;
                }
                continue;
            }

            if (c == QLatin1Char('\''))
            {
                ++i;
                while (i < n)
                {
                    if (text.at(i) == QLatin1Char('\\')) { i += 2; continue; }
                    if (text.at(i) == QLatin1Char('\'')) { ++i; break; }
                    ++i;
                }
                continue;
            }

            if (c.isLetter() || (c == QLatin1Char('_')))
            {
                const int start = i;
                ++i;
                while ((i < n) && (text.at(i).isLetterOrNumber() || (text.at(i) == QLatin1Char('_'))))
                {
                    ++i;
                }

                // Drop 'obj.name' / 'p->name': the identifier belongs to another object,
                // not to the machine's own name space.
                bool memberAccess = false;
                int p = start - 1;
                while ((p >= 0) && text.at(p).isSpace())
                {
                    --p;
                }
                if (p >= 0)
                {
                    if (text.at(p) == QLatin1Char('.'))
                    {
                        memberAccess = true;
                    }
                    else if ((text.at(p) == QLatin1Char('>')) && (p >= 1) && (text.at(p - 1) == QLatin1Char('-')))
                    {
                        memberAccess = true;
                    }
                }

                if (memberAccess == false)
                {
                    tokens.append(text.mid(start, i - start));
                }
                continue;
            }

            ++i;
        }

        return tokens;
    }

    //!< Appends the verbatim expressions carried by an operation-list's arguments/bodies.
    void collectFromOperations(const SMOperationList& ops, QList<VerbatimBlock>& out)
    {
        for (const SMOperationBase* op : ops.getOperations())
        {
            if (op == nullptr)
            {
                continue;
            }

            switch (op->getOperationType())
            {
            case SMOperationBase::eOperation::InlineCode:
                {
                    const SMInlineCode* inl = static_cast<const SMInlineCode*>(op);
                    if (inl->getBody().isEmpty() == false)
                    {
                        out.append({ inl->getId(), inl->getBody(), QStringLiteral("inline code") });
                    }
                }
                break;

            case SMOperationBase::eOperation::AttributeSet:
                {
                    const SMAttributeSet* set = static_cast<const SMAttributeSet*>(op);
                    if ((set->getSource() == SMArgumentEntry::eValueSource::Expression) && (set->getExpression().isEmpty() == false))
                    {
                        out.append({ set->getId(), set->getExpression(), QStringLiteral("attribute-set expression") });
                    }
                }
                break;

            case SMOperationBase::eOperation::ActionCall:
                {
                    const SMActionCall* call = static_cast<const SMActionCall*>(op);
                    for (const SMArgumentEntry& arg : call->getArguments())
                    {
                        if ((arg.getSource() == SMArgumentEntry::eValueSource::Expression) && (arg.getExpression().isEmpty() == false))
                        {
                            out.append({ arg.getId(), arg.getExpression(), QStringLiteral("argument expression") });
                        }
                    }
                }
                break;

            case SMOperationBase::eOperation::EventSend:
                {
                    const SMEventSend* send = static_cast<const SMEventSend*>(op);
                    for (const SMArgumentEntry& arg : send->getArguments())
                    {
                        if ((arg.getSource() == SMArgumentEntry::eValueSource::Expression) && (arg.getExpression().isEmpty() == false))
                        {
                            out.append({ arg.getId(), arg.getExpression(), QStringLiteral("argument expression") });
                        }
                    }
                }
                break;

            default:
                break;
            }
        }
    }

    //!< Appends the verbatim expression/lambda rows and condition-argument expressions.
    void collectFromConditions(const SMConditionGroup& group, QList<VerbatimBlock>& out)
    {
        for (const SMConditionEntry* leaf : group.collectLeaves())
        {
            if (leaf == nullptr)
            {
                continue;
            }

            if (leaf->isExpressionRow() && (leaf->getExpression().isEmpty() == false))
            {
                out.append({ leaf->getId(), leaf->getExpression(), QStringLiteral("condition expression row") });
            }
            else if (leaf->isLambdaRow() && (leaf->getBody().isEmpty() == false))
            {
                out.append({ leaf->getId(), leaf->getBody(), QStringLiteral("condition lambda row") });
            }

            for (const SMArgumentEntry& arg : leaf->getArguments())
            {
                if ((arg.getSource() == SMArgumentEntry::eValueSource::Expression) && (arg.getExpression().isEmpty() == false))
                {
                    out.append({ arg.getId(), arg.getExpression(), QStringLiteral("condition argument expression") });
                }
            }
        }
    }

    //!< Walks a machine level (and its nested levels) collecting every verbatim block.
    void collectFromLevel(const SMStateData& level, QList<VerbatimBlock>& out)
    {
        for (const SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
            {
                continue;
            }

            collectFromOperations(state->getEntryList(), out);
            collectFromOperations(state->getExitList(), out);

            for (const SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if (transition == nullptr)
                {
                    continue;
                }

                collectFromConditions(transition->getConditions(), out);
                collectFromOperations(transition->getOperations(), out);
            }

            if (state->hasNestedStates())
            {
                collectFromLevel(*state->getNestedStates(), out);
            }
        }
    }
}

QList<SMSymbolIndex::KnownSymbol> SMSymbolIndex::machineSymbols(const StateMachineData& data)
{
    QList<KnownSymbol> result;

    for (const SMAttributeEntry& attr : data.getAttributes().getElements())
    {
        result.append({ attr.getName(), eSymbolKind::Attribute });
    }
    for (const ConstantEntry& constant : data.getConstants().getElements())
    {
        result.append({ constant.getName(), eSymbolKind::Constant });
    }
    for (const SMMethodEntry* method : data.getMethods().getElements())
    {
        if (method != nullptr)
        {
            result.append({ method->getName(), eSymbolKind::Method });
        }
    }

    return result;
}

QSet<QString> SMSymbolIndex::machineSymbolNames(const StateMachineData& data)
{
    QSet<QString> names;
    for (const KnownSymbol& symbol : machineSymbols(data))
    {
        if (symbol.name.isEmpty() == false)
        {
            names.insert(symbol.name);
        }
    }

    return names;
}

QStringList SMSymbolIndex::stimulusParams(const StateMachineData& data, uint32_t transitionId)
{
    QStringList params;
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if ((transition == nullptr) || (transition->getStimulusKind() == SMTransitionEntry::eStimulusKind::Timer))
    {
        return params;
    }

    const MethodBase* payload = nullptr;
    if (transition->getStimulusKind() == SMTransitionEntry::eStimulusKind::Trigger)
    {
        payload = data.getMethods().findMethod(transition->getStimulus());
    }
    else
    {
        for (const SMEventEntry* event : data.getEvents().getElements())
        {
            if ((event != nullptr) && (event->getName() == transition->getStimulus()))
            {
                payload = event;
                break;
            }
        }
    }

    if (payload != nullptr)
    {
        for (const MethodParameter& param : payload->getElements())
        {
            params.append(param.getName());
        }
    }

    return params;
}

QStringList SMSymbolIndex::completionWords(const StateMachineData& data, uint32_t transitionId, bool includeParams)
{
    QStringList words;

    // Attributes are read through their generated getter; offer the call form.
    for (const SMAttributeEntry& attr : data.getAttributes().getElements())
    {
        words.append(attr.getName() + QStringLiteral("()"));
    }
    // Condition and action methods are callable from verbatim code; triggers are stimuli.
    for (const SMMethodEntry* method : data.getMethods().getElements())
    {
        if ((method != nullptr) && (method->isCondition() || method->isAction()))
        {
            words.append(method->getName() + QStringLiteral("()"));
        }
    }
    for (const ConstantEntry& constant : data.getConstants().getElements())
    {
        words.append(constant.getName());
    }
    if (includeParams)
    {
        words.append(stimulusParams(data, transitionId));
    }

    words.removeDuplicates();
    words.sort(Qt::CaseInsensitive);
    return words;
}

QStringList SMSymbolIndex::usedSymbols(const QString& text, const QSet<QString>& knownNames)
{
    QStringList result;
    for (const QString& token : identifierTokens(text))
    {
        if (knownNames.contains(token) && (result.contains(token) == false))
        {
            result.append(token);
        }
    }

    return result;
}

bool SMSymbolIndex::referencesSymbol(const QString& text, const QString& name)
{
    if (name.isEmpty())
    {
        return false;
    }

    for (const QString& token : identifierTokens(text))
    {
        if (token == name)
        {
            return true;
        }
    }

    return false;
}

QList<SMSymbolIndex::VerbatimBlock> SMSymbolIndex::collectVerbatimBlocks(const StateMachineData& data)
{
    QList<VerbatimBlock> result;

    for (const SMMethodEntry* method : data.getMethods().getElements())
    {
        if ((method != nullptr)
            && method->isCondition()
            && (method->getImplement() == SMMethodEntry::eImplement::Embedded)
            && (method->getBody().isEmpty() == false))
        {
            result.append({ method->getId(), method->getBody(), QStringLiteral("condition method body") });
        }
    }

    collectFromLevel(data.getStates(), result);
    return result;
}

QList<SMSymbolIndex::VerbatimRef> SMSymbolIndex::findReferences(const StateMachineData& data, const QString& name)
{
    QList<VerbatimRef> refs;
    if (name.isEmpty())
    {
        return refs;
    }

    for (const VerbatimBlock& block : collectVerbatimBlocks(data))
    {
        if (referencesSymbol(block.text, name))
        {
            refs.append({ block.elementId, block.location });
        }
    }

    return refs;
}
