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
 *  \file        lusan/data/sm/SMReferences.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM name-reference traversal.
 *
 ************************************************************************/

#include "lusan/data/sm/SMReferences.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/SMOperation.hpp"

#include <functional>

namespace
{
    using eTarget = SMReferences::eTarget;

    /**
     * \struct  RefSite
     * \brief   One resolved reference field found by the single traversal. Name-based sites
     *          carry a setter so rewrite can flip them; the sole ID-based site (a transition
     *          target) carries \a toId and no setter.
     **/
    struct RefSite
    {
        eTarget                             target;     //!< What kind of element this field references.
        bool                                byId;       //!< True for the transition-target site (no name).
        QString                             name;       //!< The referenced name (empty when byId).
        uint32_t                            toId;       //!< The referenced state ID (byId only).
        SMReferences::Use                   use;        //!< The navigable place for where-used.
        std::function<void(const QString&)> setName;    //!< Rewrites the field (null when byId).
    };

    //!< The human-readable place of a transition: "Owner : Kind stimulus -> target".
    QString describeTransition(const SMStateEntry& owner, const SMTransitionEntry& tr)
    {
        const QString kind{ SMTransitionEntry::toString(tr.getStimulusKind()) };
        const QString target{ tr.isExternal() ? tr.getTargetName() : QStringLiteral("(internal)") };
        return QStringLiteral("%1 : %2 %3 -> %4").arg(owner.getName(), kind, tr.getStimulus(), target);
    }

    //!< The human-readable place of a state-owned list ("Idle [entry]").
    QString describeStateSite(const SMStateEntry& state, const QString& part)
    {
        return QStringLiteral("%1 [%2]").arg(state.getName(), part);
    }

    //!< Appends the registry-reference sites of one argument (by value source).
    void collectArgSites(  QList<SMArgumentEntry>& args, int index, const SMReferences::Use& use
                         , QList<RefSite>& out)
    {
        SMArgumentEntry& arg = args[index];
        eTarget target;
        switch (arg.getSource())
        {
        case SMArgumentEntry::eValueSource::Attribute:  target = eTarget::Attribute; break;
        case SMArgumentEntry::eValueSource::Constant:   target = eTarget::Constant;  break;
        case SMArgumentEntry::eValueSource::Condition:  target = eTarget::Condition; break;
        default:                                        return; // Value/Param/Expression/Lambda: no registry ref.
        }

        RefSite site;
        site.target  = target;
        site.byId    = false;
        site.name    = arg.getValue();
        site.toId    = 0;
        site.use     = use;
        site.setName = [&args, index](const QString& v) { args[index].setValue(v); };
        out.append(site);
    }

    //!< Appends every reference site of one operation (owned by a state list or a transition).
    void collectOperationSites(SMOperationBase& op, const SMReferences::Use& use, QList<RefSite>& out)
    {
        switch (op.getOperationType())
        {
        case SMOperationBase::eOperation::ActionCall:
        {
            SMActionCall& call = static_cast<SMActionCall&>(op);
            out.append({ eTarget::Action, false, call.getAction(), 0, use
                       , [&call](const QString& v) { call.setAction(v); } });
            for (int i = 0; i < call.getArguments().size(); ++i)
                collectArgSites(call.getArguments(), i, use, out);
            break;
        }
        case SMOperationBase::eOperation::AttributeSet:
        {
            SMAttributeSet& set = static_cast<SMAttributeSet&>(op);
            out.append({ eTarget::Attribute, false, set.getAttribute(), 0, use
                       , [&set](const QString& v) { set.setAttribute(v); } });
            eTarget valueTarget = eTarget::Attribute;
            bool valueRef = true;
            switch (set.getSource())
            {
            case SMArgumentEntry::eValueSource::Attribute:  valueTarget = eTarget::Attribute; break;
            case SMArgumentEntry::eValueSource::Constant:   valueTarget = eTarget::Constant;  break;
            case SMArgumentEntry::eValueSource::Condition:  valueTarget = eTarget::Condition; break;
            default:                                        valueRef = false; break;
            }
            if (valueRef)
                out.append({ valueTarget, false, set.getValue(), 0, use
                           , [&set](const QString& v) { set.setValue(v); } });
            break;
        }
        case SMOperationBase::eOperation::TimerStart:
        {
            SMTimerStart& start = static_cast<SMTimerStart&>(op);
            out.append({ eTarget::Timer, false, start.getTimer(), 0, use
                       , [&start](const QString& v) { start.setTimer(v); } });
            break;
        }
        case SMOperationBase::eOperation::TimerStop:
        {
            SMTimerStop& stop = static_cast<SMTimerStop&>(op);
            out.append({ eTarget::Timer, false, stop.getTimer(), 0, use
                       , [&stop](const QString& v) { stop.setTimer(v); } });
            break;
        }
        case SMOperationBase::eOperation::EventSend:
        {
            SMEventSend& send = static_cast<SMEventSend&>(op);
            out.append({ eTarget::Event, false, send.getEvent(), 0, use
                       , [&send](const QString& v) { send.setEvent(v); } });
            for (int i = 0; i < send.getArguments().size(); ++i)
                collectArgSites(send.getArguments(), i, use, out);
            break;
        }
        case SMOperationBase::eOperation::InlineCode:
            break; // Verbatim code is never parsed for references.
        }
    }

    //!< Appends the reference sites of a state list's operations under a state-scoped location.
    void collectStateList(SMStateEntry& state, SMOperationList& list, const QString& part, QList<RefSite>& out)
    {
        const SMReferences::Use use{ state.getId(), true, describeStateSite(state, part) };
        for (SMOperationBase* op : list.getOperations())
            collectOperationSites(*op, use, out);
    }

    //!< The single recursive traversal of one machine level and every nested level.
    void collectLevel(SMStateData& level, QList<RefSite>& out)
    {
        for (SMStateEntry* state : level.getElements())
        {
            // State-owned operation lists.
            collectStateList(*state, state->getEntryList(), QStringLiteral("entry"), out);
            collectStateList(*state, state->getExitList(),  QStringLiteral("exit"),  out);
            collectStateList(*state, state->getDoList(),    QStringLiteral("do"),    out);

            // The hosted submachine: the state names an entry of the ImportList by alias.
            if (state->getSubmachine().isEmpty() == false)
            {
                const SMReferences::Use use{ state->getId(), true, describeStateSite(*state, QStringLiteral("Submachine")) };
                out.append({ eTarget::Import, false, state->getSubmachine(), 0, use
                           , [state](const QString& v) { state->setSubmachine(v); } });
            }

            // The OnFinal completion-hook event.
            if (state->getOnFinal().isEmpty() == false)
            {
                const SMReferences::Use use{ state->getId(), true, describeStateSite(*state, QStringLiteral("OnFinal")) };
                out.append({ eTarget::Event, false, state->getOnFinal(), 0, use
                           , [state](const QString& v) { state->setOnFinal(v); } });
            }

            for (SMTransitionEntry* tr : state->getTransitions().getElements())
            {
                const SMReferences::Use use{ tr->getId(), false, describeTransition(*state, *tr) };

                // The stimulus: a trigger, event, or timer depending on the transition kind.
                eTarget stimTarget = eTarget::Trigger;
                switch (tr->getStimulusKind())
                {
                case SMTransitionEntry::eStimulusKind::Trigger: stimTarget = eTarget::Trigger; break;
                case SMTransitionEntry::eStimulusKind::Event:   stimTarget = eTarget::Event;   break;
                case SMTransitionEntry::eStimulusKind::Timer:   stimTarget = eTarget::Timer;   break;
                }
                out.append({ stimTarget, false, tr->getStimulus(), 0, use
                           , [tr](const QString& v) { tr->setStimulus(v); } });

                // The external target: referenced by ID, so it is reported but never rewritten.
                if (tr->isExternal())
                    out.append({ eTarget::State, true, QString(), tr->getToId(), use, nullptr });

                // Transition operations.
                for (SMOperationBase* op : tr->getOperations().getOperations())
                    collectOperationSites(*op, use, out);
            }

            if (SMStateData* nested = state->getNestedStates())
                collectLevel(*nested, out);
        }
    }

    //!< Collects every reference site of the document in document order.
    QList<RefSite> collectSites(StateMachineData& data)
    {
        QList<RefSite> out;
        collectLevel(data.getStates(), out);
        return out;
    }
}

QList<SMReferences::Use> SMReferences::whereUsed(const StateMachineData& data, eTarget target, const QString& name, uint32_t targetId)
{
    // Collection only reads the tree; the const-cast keeps the one traversal in one place
    // (the state containers hold their entries by pointer, so no data is copied or mutated).
    QList<RefSite> sites = collectSites(const_cast<StateMachineData&>(data));

    QList<Use> out;
    for (const RefSite& site : sites)
    {
        if (site.target != target)
            continue;

        if (site.byId)
        {
            if ((target == eTarget::State) && (site.toId == targetId))
                out.append(site.use);
        }
        else if (site.name == name)
        {
            out.append(site.use);
        }
    }

    // Guard trees bind method/attribute/constant symbols by ID; that reference knowledge
    // lives in the model layer (SMGuardWhereUsed). The where-used UI unions those in through
    // SMWhereUsed so this headless data-layer walker stays free of any model dependency.
    (void)targetId;
    return out;
}

QList<SMReferences::Ref> SMReferences::definitionsOf(const StateMachineData& data, uint32_t elementId, bool isState)
{
    // The same single traversal, filtered to the sites owned by one element: every name-based
    // reference the element makes. The const-cast keeps one collector (nothing is mutated).
    QList<RefSite> sites = collectSites(const_cast<StateMachineData&>(data));

    QList<Ref> out;
    for (const RefSite& site : sites)
    {
        // ID-based sites (the transition-target state) are navigation on the canvas, not a
        // registry declaration, so they are not go-to-declaration targets.
        if (site.byId)
            continue;

        if ((site.use.navId != elementId) || (site.use.isState != isState))
            continue;

        out.append({ site.target, site.name });
    }

    return out;
}

QList<SMReferences::Ref> SMReferences::operationRefs(const SMOperationBase& op)
{
    // The primary declaration each operation kind names. This mirrors the primary reference of
    // collectOperationSites (the mutation walker), kept read-only here so the go-to-declaration
    // direction shares the same definition of "what an operation references".
    QList<Ref> refs;
    switch (op.getOperationType())
    {
    case SMOperationBase::eOperation::ActionCall:
        refs.append({ eTarget::Action,    static_cast<const SMActionCall&>(op).getAction() });
        break;
    case SMOperationBase::eOperation::AttributeSet:
        refs.append({ eTarget::Attribute, static_cast<const SMAttributeSet&>(op).getAttribute() });
        break;
    case SMOperationBase::eOperation::TimerStart:
        refs.append({ eTarget::Timer,     static_cast<const SMTimerStart&>(op).getTimer() });
        break;
    case SMOperationBase::eOperation::TimerStop:
        refs.append({ eTarget::Timer,     static_cast<const SMTimerStop&>(op).getTimer() });
        break;
    case SMOperationBase::eOperation::EventSend:
        refs.append({ eTarget::Event,     static_cast<const SMEventSend&>(op).getEvent() });
        break;
    case SMOperationBase::eOperation::InlineCode:
        break;      // Verbatim code is never parsed for references.
    }

    return refs;
}

int SMReferences::rewriteReferences(StateMachineData& data, eTarget target, const QString& oldName, const QString& newName)
{
    QList<RefSite> sites = collectSites(data);

    int count = 0;
    for (const RefSite& site : sites)
    {
        if (site.byId || (site.target != target) || (site.name != oldName))
            continue;

        site.setName(newName);
        ++count;
    }

    return count;
}
