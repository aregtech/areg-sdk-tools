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
 *  \file        lusan/model/sm/SMOperationValidation.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operation argument-mapping validation (headless).
 *
 ************************************************************************/

#include "lusan/model/sm/SMOperationValidation.hpp"

#include "lusan/data/common/MethodBase.hpp"
#include "lusan/model/sm/SMValidator.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMDocumentIndex.hpp"

#include <QObject>
#include <QSet>
#include <QString>

namespace
{
    using eOp = SMOperationBase::eOperation;

    //!< The declared callee of an action/event operation, and its stored argument list, or a null
    //!< callee when the operation is neither an action nor an event, or its callee does not resolve.
    const MethodBase* calleeOf(const SMDocumentIndex& index, SMOperationBase* op, const QList<SMArgumentEntry>*& args)
    {
        args = nullptr;
        if (op->getOperationType() == eOp::ActionCall)
        {
            SMActionCall* call = static_cast<SMActionCall*>(op);
            args = &call->getArguments();
            // By kind: a trigger sharing the action's name is a different declaration.
            return index.method(call->getAction(), SMMethodEntry::eMethodType::Action);
        }
        else if (op->getOperationType() == eOp::EventSend)
        {
            SMEventSend* send = static_cast<SMEventSend*>(op);
            args = &send->getArguments();
            return index.event(send->getEvent());
        }

        return nullptr;
    }
}


QList<DocIssue> SMOperationValidation::argumentIssues( const MethodBase& callee
                                                    , const QList<SMArgumentEntry>& args
                                                    , uint32_t ownerId
                                                    , eDocElementKind kind
                                                    , const QString& location)
{
    QList<DocIssue> issues;
    const auto add = [&issues, ownerId, kind, &location](const QString& message, const QString& detail)
    {
        DocIssue issue;
        issue.elementId = ownerId;
        issue.kind      = kind;
        issue.severity  = DocIssue::eSeverity::Error;
        issue.rule      = SMValidator::RULE_ARGUMENT_MAPPING;
        issue.message   = message;
        issue.location  = location;
        issue.detail    = detail;
        issues.append(issue);
    };

    QSet<QString> formalNames;
    for (const MethodParameter& formal : callee.getElements())
    {
        formalNames.insert(formal.getName());

        // A required formal (no default) that no argument binds is an incomplete mapping.
        if (formal.hasDefault())
        {
            continue;
        }

        bool mapped = false;
        for (const SMArgumentEntry& arg : args)
        {
            if (arg.getName() == formal.getName())
            {
                mapped = true;
                break;
            }
        }

        if (mapped == false)
        {
            add( QObject::tr("Required parameter '%1' of '%2' is not mapped").arg(formal.getName(), callee.getName())
               , QObject::tr("The parameter has no default, so the call cannot be generated until a value is bound to it."));
        }
    }

    // A stored argument for a formal that no longer exists is an orphan (value kept, red row).
    for (const SMArgumentEntry& arg : args)
    {
        if (formalNames.contains(arg.getName()) == false)
        {
            add( QObject::tr("Argument '%1' is not a parameter of '%2'").arg(arg.getName(), callee.getName())
               , QObject::tr("The parameter was renamed or removed on its declaration page. The bound value is kept so it can be re-bound; remove the row to discard it."));
        }
    }

    return issues;
}

QList<DocIssue> SMOperationValidation::listIssues( const StateMachineData& data
                                                 , const SMOperationList& list
                                                 , uint32_t ownerId
                                                 , eDocElementKind kind
                                                 , const QString& location)
{
    QList<DocIssue> issues;
    const SMDocumentIndex index(data);
    for (SMOperationBase* op : list.getOperations())
    {
        const QList<SMArgumentEntry>* args = nullptr;
        const MethodBase* callee = calleeOf(index, op, args);
        if ((callee == nullptr) || (args == nullptr))
        {
            continue;   // unresolved callee (the picker shows it blank): not a mapping fault here.
        }

        issues += argumentIssues(*callee, *args, ownerId, kind, location);
    }

    return issues;
}

bool SMOperationValidation::worstForTransition(const StateMachineData& data, uint32_t transitionId, DocIssue::eSeverity& worst)
{
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if (transition == nullptr)
    {
        return false;
    }

    // Scoped to one transition on purpose: the canvas asks per item while it repaints, so this
    // must stay proportional to that transition, never to the document.
    const QList<DocIssue> issues = listIssues(data, transition->getOperations(), transitionId, eDocElementKind::Transition, QString());
    if (issues.isEmpty())
    {
        return false;
    }

    worst = worstOf(issues, DocIssue::eSeverity::Info);
    return true;
}

bool SMOperationValidation::worstForState(const StateMachineData& data, uint32_t stateId, DocIssue::eSeverity& worst)
{
    const SMStateEntry* state = data.findStateById(stateId);
    if (state == nullptr)
    {
        return false;
    }

    QList<DocIssue> issues = listIssues(data, state->getEntryList(), stateId, eDocElementKind::State, QString());
    issues += listIssues(data, state->getDoList(), stateId, eDocElementKind::State, QString());
    issues += listIssues(data, state->getExitList(), stateId, eDocElementKind::State, QString());
    if (issues.isEmpty())
    {
        return false;
    }

    worst = worstOf(issues, DocIssue::eSeverity::Info);
    return true;
}
