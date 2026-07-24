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
 *  \file        lusan/model/sm/SMGuardWhereUsed.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard where-used: which guards reference a symbol.
 *
 ************************************************************************/

#include "lusan/model/sm/SMGuardWhereUsed.hpp"

#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

namespace
{
    //!< True when any Call/Attr/Const/Param node of the tree references \p symbolId.
    bool treeReferences(const SMGuardNode* node, uint32_t symbolId)
    {
        if (node == nullptr)
        {
            return false;
        }

        switch (node->getKind())
        {
        case SMGuardNode::eKind::Call:
        case SMGuardNode::eKind::Attr:
        case SMGuardNode::eKind::Const:
        case SMGuardNode::eKind::Param:
            if (node->getSymbolId() == symbolId)
            {
                return true;
            }
            break;

        default:
            break;
        }

        for (const SMGuardNode* child : node->getChildren())
        {
            if (treeReferences(child, symbolId))
            {
                return true;
            }
        }

        return false;
    }

    void collectFromLevel(const SMStateData& level, uint32_t symbolId, QList<SMGuardWhereUsed::Use>& out)
    {
        for (const SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
            {
                continue;
            }

            for (const SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if (transition == nullptr)
                {
                    continue;
                }

                const SMGuard& guard = transition->getGuard();
                if (treeReferences(guard.getTree(), symbolId))
                {
                    QString location = state->getName() + QStringLiteral(" : ") + transition->getStimulus();
                    if (transition->isExternal())
                    {
                        location += QStringLiteral(" -> ") + transition->getTargetName();
                    }

                    out.append({ transition->getId(), location });
                }
            }

            if (state->hasNestedStates())
            {
                collectFromLevel(*state->getNestedStates(), symbolId, out);
            }
        }
    }
}

QList<SMGuardWhereUsed::Use> SMGuardWhereUsed::symbolUses(const StateMachineData& data, uint32_t symbolId)
{
    QList<Use> result;
    if (symbolId != 0u)
    {
        collectFromLevel(data.getStates(), symbolId, result);
    }

    return result;
}

int SMGuardWhereUsed::useCount(const StateMachineData& data, uint32_t symbolId)
{
    return static_cast<int>(symbolUses(data, symbolId).size());
}
