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
 *  \file        lusan/model/sm/SMValidator.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM structural and reference validation engine.
 *
 ************************************************************************/

#include "lusan/model/sm/SMValidator.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMImportData.hpp"
#include "lusan/data/sm/SMDataTypeData.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"

#include <QCoreApplication>
#include <QHash>
#include <QSet>

namespace
{
    using eSeverity = SMIssue::eSeverity;
    using eValueSource = SMArgumentEntry::eValueSource;

    //!< Translatable finding text.
    QString vtr(const char* text)
    {
        return QCoreApplication::translate("SMValidator", text);
    }

    /**
     * \brief   The stimulus context a value source is evaluated in, used to decide whether a
     *          `Param` reference is in scope. Entry/exit/do lists carry no stimulus; a
     *          transition carries the parameters of its trigger or event stimulus (a timer
     *          stimulus carries none).
     **/
    struct Scope
    {
        bool                                inTransition { false };                             //!< True inside a transition's ops/conditions.
        SMTransitionEntry::eStimulusKind    stimKind     { SMTransitionEntry::eStimulusKind::Timer };
        const MethodBase*                   stimParams   { nullptr };                           //!< The stimulus parameter owner (null for timer/entry/exit).
    };

    /**
     * \class   Ctx
     * \brief   One validation run: holds the document, the accumulated findings, and the
     *          registry lookups every rule shares.
     **/
    class Ctx
    {
    public:
        explicit Ctx(const StateMachineData& data)
            : mData(data)
        {
        }

        QList<SMIssue> run();

    private:
        void add(uint32_t id, eDocElementKind kind, eSeverity sev, int rule, const QString& message);

        //!< Collects every machine level (root + painted nested) with its owner state ID.
        struct LevelInfo { const SMStateData* level; bool isRoot; uint32_t ownerId; };
        void collectLevels(const SMStateData& level, bool isRoot, uint32_t ownerId, QList<LevelInfo>& out) const;

        void checkIdentifier(uint32_t id, eDocElementKind kind, const QString& name);
        bool typeResolves(const QString& typeName) const;
        void checkDataType(uint32_t id, eDocElementKind kind, const QString& typeName);

        void checkDuplicateIds();
        void collectIds(const SMStateData& level, QHash<uint32_t, int>& counts) const;
        void checkDuplicateStateNames(const QList<LevelInfo>& levels);
        void checkRegistryNames();

        void validateLevel(const LevelInfo& info);
        void validateState(const SMStateEntry& state, const SMStateData& level);
        void validateTransition(const SMStateEntry& owner, const SMStateData& level, const SMTransitionEntry& tr);
        void validateOperations(const SMOperationList& ops, const Scope& scope);
        void validateArguments(uint32_t ownerId, eDocElementKind kind, const MethodBase* target, const QList<SMArgumentEntry>& args, const Scope& scope);
        void validateValueSource(uint32_t ownerId, eDocElementKind kind, eValueSource source, const QString& ref, const QString& expr, const Scope& scope, bool valuePosition);
        void validateConditions(const SMConditionList& conditions, const Scope& scope);
        void validateOperand(uint32_t ownerId, eValueSource kind, const QString& ref, const Scope& scope, bool isRhs);
        void validateParamScope(uint32_t ownerId, eDocElementKind kind, const QString& paramName, const Scope& scope);

        bool hasParam(const MethodBase* owner, const QString& name) const;

    private:
        const StateMachineData& mData;
        QList<SMIssue>          mIssues;
    };

    void Ctx::add(uint32_t id, eDocElementKind kind, eSeverity sev, int rule, const QString& message)
    {
        SMIssue issue;
        issue.elementId = id;
        issue.kind      = kind;
        issue.severity  = sev;
        issue.rule      = rule;
        issue.message   = message;
        mIssues.append(issue);
    }

    bool Ctx::hasParam(const MethodBase* owner, const QString& name) const
    {
        return (owner != nullptr) && owner->hasElement(name);
    }

    void Ctx::collectLevels(const SMStateData& level, bool isRoot, uint32_t ownerId, QList<LevelInfo>& out) const
    {
        out.append(LevelInfo{ &level, isRoot, ownerId });
        for (SMStateEntry* state : level.getElements())
        {
            if ((state != nullptr) && state->hasNestedStates())
            {
                collectLevels(*state->getNestedStates(), false, state->getId(), out);
            }
        }
    }

    void Ctx::checkIdentifier(uint32_t id, eDocElementKind kind, const QString& name)
    {
        if (StateMachineData::isValidIdentifier(name) == false)
        {
            add(id, kind, eSeverity::Error, 5, vtr("'%1' is not a valid identifier").arg(name));
        }
    }

    bool Ctx::typeResolves(const QString& typeName) const
    {
        // Only plain identifiers are resolved here; structured or templated type strings, and
        // type compatibility of the value against the type, are handled by other validators.
        if (StateMachineData::isValidIdentifier(typeName) == false)
            return true;
        if (DataTypeFactory::fromString(typeName) != DataTypeBase::eCategory::Undefined)
            return true;
        return (mData.getDataTypes().findCustomDataType(typeName) != nullptr);
    }

    void Ctx::checkDataType(uint32_t id, eDocElementKind kind, const QString& typeName)
    {
        if ((typeName.isEmpty() == false) && (typeResolves(typeName) == false))
        {
            add(id, kind, eSeverity::Error, 6, vtr("Data type '%1' does not resolve").arg(typeName));
        }
    }

    void Ctx::collectIds(const SMStateData& level, QHash<uint32_t, int>& counts) const
    {
        auto note = [&counts](uint32_t id) { if (id != 0) counts[id] += 1; };
        auto noteOps = [&](const SMOperationList& ops)
        {
            for (SMOperationBase* op : ops.getOperations())
            {
                if (op == nullptr)
                    continue;
                note(op->getId());
                if (SMActionCall* a = dynamic_cast<SMActionCall*>(op))
                    for (const SMArgumentEntry& arg : a->getArguments()) note(arg.getId());
                else if (SMEventSend* e = dynamic_cast<SMEventSend*>(op))
                    for (const SMArgumentEntry& arg : e->getArguments()) note(arg.getId());
            }
        };

        for (SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
                continue;
            note(state->getId());
            noteOps(state->getEntryList());
            noteOps(state->getExitList());
            noteOps(state->getDoList());
            for (SMTransitionEntry* tr : state->getTransitions().getElements())
            {
                if (tr == nullptr)
                    continue;
                note(tr->getId());
                noteOps(tr->getOperations());
                for (SMConditionEntry* leaf : tr->getConditions().collectLeaves())
                {
                    if (leaf == nullptr)
                        continue;
                    note(leaf->getId());
                    for (const SMArgumentEntry& arg : leaf->getArguments()) note(arg.getId());
                }
            }
            if (state->hasNestedStates())
            {
                collectIds(*state->getNestedStates(), counts);
            }
        }
    }

    void Ctx::checkDuplicateIds()
    {
        QHash<uint32_t, int> counts;
        collectIds(mData.getStates(), counts);
        for (SMMethodEntry* m : mData.getMethods().getElements())
        {
            if (m == nullptr) continue;
            counts[m->getId()] += 1;
            for (const MethodParameter& p : m->getElements()) counts[p.getId()] += 1;
        }
        for (SMEventEntry* e : mData.getEvents().getElements())
        {
            if (e == nullptr) continue;
            counts[e->getId()] += 1;
            for (const MethodParameter& p : e->getElements()) counts[p.getId()] += 1;
        }
        for (const SMTimerEntry& t : mData.getTimers().getElements())     counts[t.getId()] += 1;
        for (const SMAttributeEntry& a : mData.getAttributes().getElements()) counts[a.getId()] += 1;
        for (const ConstantEntry& c : mData.getConstants().getElements())  counts[c.getId()] += 1;
        for (const SMImportEntry& i : mData.getImports().getElements())    counts[i.getId()] += 1;
        for (DataTypeCustom* d : mData.getDataTypes().getCustomDataTypes())
            if (d != nullptr) counts[d->getId()] += 1;

        for (auto it = counts.constBegin(); it != counts.constEnd(); ++it)
        {
            if (it.value() > 1)
            {
                add(it.key(), eDocElementKind::State, eSeverity::Error, 2,
                    vtr("Duplicate element ID %1 (used %2 times)").arg(it.key()).arg(it.value()));
            }
        }
    }

    void Ctx::checkDuplicateStateNames(const QList<LevelInfo>& levels)
    {
        QHash<QString, int> seen;
        for (const LevelInfo& info : levels)
        {
            for (SMStateEntry* state : info.level->getElements())
            {
                if (state == nullptr)
                    continue;
                const QString& name = state->getName();
                if (name.isEmpty())
                    continue;
                if (++seen[name] > 1)
                {
                    add(state->getId(), eDocElementKind::State, eSeverity::Error, 3,
                        vtr("Duplicate state name '%1'").arg(name));
                }
            }
        }
    }

    void Ctx::checkRegistryNames()
    {
        // Names must be unique inside each registry, and triggers, events and timers share one
        // stimulus name space, so a name may belong to only one of them.
        auto dupWithin = [this](const QStringList& names, const QList<uint32_t>& ids, eDocElementKind kind)
        {
            QHash<QString, int> seen;
            for (int i = 0; i < names.size(); ++i)
            {
                if (names.at(i).isEmpty())
                    continue;
                if (++seen[names.at(i)] > 1)
                    add(ids.at(i), kind, eSeverity::Error, 4, vtr("Duplicate name '%1' in registry").arg(names.at(i)));
            }
        };

        QStringList mNames; QList<uint32_t> mIds;
        for (SMMethodEntry* m : mData.getMethods().getElements())
            if (m != nullptr) { mNames << m->getName(); mIds << m->getId(); }
        dupWithin(mNames, mIds, eDocElementKind::Method);

        QStringList eNames; QList<uint32_t> eIds;
        for (SMEventEntry* e : mData.getEvents().getElements())
            if (e != nullptr) { eNames << e->getName(); eIds << e->getId(); }
        dupWithin(eNames, eIds, eDocElementKind::Event);

        QStringList tNames; QList<uint32_t> tIds;
        for (const SMTimerEntry& t : mData.getTimers().getElements()) { tNames << t.getName(); tIds << t.getId(); }
        dupWithin(tNames, tIds, eDocElementKind::Timer);

        QStringList aNames; QList<uint32_t> aIds;
        for (const SMAttributeEntry& a : mData.getAttributes().getElements()) { aNames << a.getName(); aIds << a.getId(); }
        dupWithin(aNames, aIds, eDocElementKind::Attribute);

        QStringList cNames; QList<uint32_t> cIds;
        for (const ConstantEntry& c : mData.getConstants().getElements()) { cNames << c.getName(); cIds << c.getId(); }
        dupWithin(cNames, cIds, eDocElementKind::Constant);

        QStringList iNames; QList<uint32_t> iIds;
        for (const SMImportEntry& i : mData.getImports().getElements()) { iNames << i.getName(); iIds << i.getId(); }
        dupWithin(iNames, iIds, eDocElementKind::Import);

        QStringList dNames; QList<uint32_t> dIds;
        for (DataTypeCustom* d : mData.getDataTypes().getCustomDataTypes())
            if (d != nullptr) { dNames << d->getName(); dIds << d->getId(); }
        dupWithin(dNames, dIds, eDocElementKind::DataType);

        // The shared stimulus name space: a name used by more than one of trigger/event/timer
        // is reported on each element after the first owner.
        QHash<QString, eDocElementKind> firstOwner;
        auto stimulus = [&](const QString& name, uint32_t id, eDocElementKind kind)
        {
            if (name.isEmpty())
                return;
            auto it = firstOwner.find(name);
            if (it == firstOwner.end())
                firstOwner.insert(name, kind);
            else if (it.value() != kind)
                add(id, kind, eSeverity::Error, 4, vtr("Stimulus name '%1' collides across trigger/event/timer").arg(name));
        };
        for (SMMethodEntry* m : mData.getMethods().getElements())
            if ((m != nullptr) && m->isTrigger()) stimulus(m->getName(), m->getId(), eDocElementKind::Method);
        for (SMEventEntry* e : mData.getEvents().getElements())
            if (e != nullptr) stimulus(e->getName(), e->getId(), eDocElementKind::Event);
        for (const SMTimerEntry& t : mData.getTimers().getElements())
            stimulus(t.getName(), t.getId(), eDocElementKind::Timer);

        // Parameter names must be unique within a single parameter list.
        auto dupParams = [this](const MethodBase* owner, eDocElementKind kind)
        {
            if (owner == nullptr) return;
            QHash<QString, int> seen;
            for (const MethodParameter& p : owner->getElements())
            {
                if (p.getName().isEmpty())
                    continue;
                if (++seen[p.getName()] > 1)
                    add(p.getId(), kind, eSeverity::Error, 4, vtr("Duplicate parameter name '%1'").arg(p.getName()));
            }
        };
        for (SMMethodEntry* m : mData.getMethods().getElements()) dupParams(m, eDocElementKind::Method);
        for (SMEventEntry* e : mData.getEvents().getElements())   dupParams(e, eDocElementKind::Event);
    }

    void Ctx::validateLevel(const LevelInfo& info)
    {
        int startCount = 0;
        SMStateEntry* firstStart = nullptr;
        for (SMStateEntry* state : info.level->getElements())
        {
            if ((state != nullptr) && (state->getKind() == SMStateEntry::eStateKind::Start))
            {
                ++startCount;
                if (firstStart == nullptr)
                    firstStart = state;
                else
                    add(state->getId(), eDocElementKind::State, eSeverity::Error, 1,
                        vtr("More than one Start state on this machine level"));
            }
        }

        if (startCount == 0)
        {
            if (info.isRoot)
                add(0, eDocElementKind::State, eSeverity::Error, 1, vtr("The machine has no Start state"));
            else
                add(info.ownerId, eDocElementKind::State, eSeverity::Error, 11,
                    vtr("Submachine level has no Start state"));
        }

        for (SMStateEntry* state : info.level->getElements())
        {
            if (state != nullptr)
                validateState(*state, *info.level);
        }
    }

    void Ctx::validateState(const SMStateEntry& state, const SMStateData& level)
    {
        const uint32_t id = state.getId();
        checkIdentifier(id, eDocElementKind::State, state.getName());

        const bool composite = state.hasNestedStates() || state.isImportedSubmachine();
        const bool hasSubstates = state.hasNestedStates() || state.isImportedSubmachine();

        // A Final state is terminal; a Start state cannot itself be a composite.
        if (state.getKind() == SMStateEntry::eStateKind::Final)
        {
            if (state.getTransitions().hasElements())
                add(id, eDocElementKind::State, eSeverity::Error, 8, vtr("A Final state cannot have outgoing transitions"));
            if (hasSubstates)
                add(id, eDocElementKind::State, eSeverity::Error, 8, vtr("A Final state cannot have substates"));
        }
        else if (state.getKind() == SMStateEntry::eStateKind::Start)
        {
            if (hasSubstates)
                add(id, eDocElementKind::State, eSeverity::Error, 9, vtr("A Start state cannot own substates"));
        }

        // A painted submachine and an imported one are mutually exclusive, neither belongs on
        // a Start or Final state, and history/completion hooks need a composite to act on.
        if (state.hasNestedStates() && state.isImportedSubmachine())
            add(id, eDocElementKind::State, eSeverity::Error, 18, vtr("A state cannot have both a Submachine and a painted StateList"));
        if (state.isImportedSubmachine() && (state.getKind() != SMStateEntry::eStateKind::Normal))
            add(id, eDocElementKind::State, eSeverity::Error, 18, vtr("A Submachine is not allowed on a Start or Final state"));
        if ((state.getHistory() != SMStateEntry::eHistory::None) && (composite == false))
            add(id, eDocElementKind::State, eSeverity::Error, 18, vtr("History is only allowed on a composite state"));
        if ((state.getOnFinal().isEmpty() == false) && (composite == false))
            add(id, eDocElementKind::State, eSeverity::Error, 18, vtr("OnFinal is only allowed on a composite state"));

        // A submachine alias must name a declared import; the completion hook must name an event.
        if (state.isImportedSubmachine() && (mData.getImports().findElement(state.getSubmachine()) == nullptr))
            add(id, eDocElementKind::State, eSeverity::Error, 6, vtr("Submachine import '%1' is not declared").arg(state.getSubmachine()));
        if ((state.getOnFinal().isEmpty() == false) && (mData.getEvents().findEvent(state.getOnFinal()) == nullptr))
            add(id, eDocElementKind::State, eSeverity::Error, 6, vtr("OnFinal event '%1' is not declared").arg(state.getOnFinal()));

        // Entry / exit / do operations carry no stimulus scope.
        const Scope entryScope;
        validateOperations(state.getEntryList(), entryScope);
        validateOperations(state.getExitList(), entryScope);
        validateOperations(state.getDoList(), entryScope);

        for (SMTransitionEntry* tr : state.getTransitions().getElements())
        {
            if (tr != nullptr)
                validateTransition(state, level, *tr);
        }
    }

    void Ctx::validateTransition(const SMStateEntry& owner, const SMStateData& level, const SMTransitionEntry& tr)
    {
        const uint32_t id = tr.getId();

        // An external target must name a state, and that state must be a sibling of the owner.
        if (tr.isExternal())
        {
            const QString& target = tr.getTo();
            if (level.findState(target) == nullptr)
            {
                if (mData.findState(target) == nullptr)
                    add(id, eDocElementKind::Transition, eSeverity::Error, 6, vtr("Transition target '%1' does not resolve").arg(target));
                else
                    add(id, eDocElementKind::Transition, eSeverity::Error, 7, vtr("Transition target '%1' is not a sibling state").arg(target));
            }
        }

        // The stimulus must exist in the registry its kind selects; that registry also
        // supplies the parameter names a Param reference is later checked against.
        Scope scope;
        scope.inTransition = true;
        scope.stimKind = tr.getStimulusKind();
        const QString& stim = tr.getStimulus();
        switch (tr.getStimulusKind())
        {
        case SMTransitionEntry::eStimulusKind::Trigger:
        {
            SMMethodEntry* m = mData.getMethods().findTrigger(stim);
            if (m == nullptr)
                add(id, eDocElementKind::Transition, eSeverity::Error, 6, vtr("Trigger '%1' is not declared").arg(stim));
            scope.stimParams = m;
            break;
        }
        case SMTransitionEntry::eStimulusKind::Event:
        {
            SMEventEntry* e = mData.getEvents().findEvent(stim);
            if (e == nullptr)
                add(id, eDocElementKind::Transition, eSeverity::Error, 6, vtr("Event '%1' is not declared").arg(stim));
            scope.stimParams = e;
            break;
        }
        case SMTransitionEntry::eStimulusKind::Timer:
            if (mData.getTimers().findElement(stim) == nullptr)
                add(id, eDocElementKind::Transition, eSeverity::Error, 6, vtr("Timer '%1' is not declared").arg(stim));
            break;
        }

        validateOperations(tr.getOperations(), scope);
        validateConditions(tr.getConditions(), scope);
    }

    void Ctx::validateOperations(const SMOperationList& ops, const Scope& scope)
    {
        for (SMOperationBase* op : ops.getOperations())
        {
            if (op == nullptr)
                continue;

            const uint32_t id = op->getId();
            switch (op->getOperationType())
            {
            case SMOperationBase::eOperation::ActionCall:
            {
                SMActionCall* call = static_cast<SMActionCall*>(op);
                SMMethodEntry* action = mData.getMethods().findMethod(call->getAction());
                if ((action == nullptr) || (action->isAction() == false))
                    add(id, eDocElementKind::Operation, eSeverity::Error, 6, vtr("Action '%1' is not declared").arg(call->getAction()));
                validateArguments(id, eDocElementKind::Operation, (action != nullptr && action->isAction()) ? action : nullptr, call->getArguments(), scope);
                break;
            }
            case SMOperationBase::eOperation::AttributeSet:
            {
                SMAttributeSet* set = static_cast<SMAttributeSet*>(op);
                if (mData.getAttributes().findElement(set->getAttribute()) == nullptr)
                    add(id, eDocElementKind::Operation, eSeverity::Error, 6, vtr("Attribute '%1' is not declared").arg(set->getAttribute()));
                validateValueSource(id, eDocElementKind::Operation, set->getSource(), set->getValue(), set->getExpression(), scope, true);
                break;
            }
            case SMOperationBase::eOperation::TimerStart:
            {
                SMTimerStart* start = static_cast<SMTimerStart*>(op);
                if (mData.getTimers().findElement(start->getTimer()) == nullptr)
                    add(id, eDocElementKind::Operation, eSeverity::Error, 6, vtr("Timer '%1' is not declared").arg(start->getTimer()));
                break;
            }
            case SMOperationBase::eOperation::TimerStop:
            {
                SMTimerStop* stop = static_cast<SMTimerStop*>(op);
                if (mData.getTimers().findElement(stop->getTimer()) == nullptr)
                    add(id, eDocElementKind::Operation, eSeverity::Error, 6, vtr("Timer '%1' is not declared").arg(stop->getTimer()));
                break;
            }
            case SMOperationBase::eOperation::EventSend:
            {
                SMEventSend* send = static_cast<SMEventSend*>(op);
                SMEventEntry* event = mData.getEvents().findEvent(send->getEvent());
                if (event == nullptr)
                    add(id, eDocElementKind::Operation, eSeverity::Error, 6, vtr("Event '%1' is not declared").arg(send->getEvent()));
                validateArguments(id, eDocElementKind::Operation, event, send->getArguments(), scope);
                break;
            }
            case SMOperationBase::eOperation::InlineCode:
                break;
            }
        }
    }

    void Ctx::validateArguments(uint32_t ownerId, eDocElementKind kind, const MethodBase* target, const QList<SMArgumentEntry>& args, const Scope& scope)
    {
        if (target != nullptr)
        {
            // Every argument must name a declared parameter, and every parameter without a
            // default must be mapped.
            QSet<QString> mapped;
            for (const SMArgumentEntry& arg : args)
                mapped.insert(arg.getName());
            QSet<QString> declared;
            for (const MethodParameter& p : target->getElements())
                declared.insert(p.getName());

            for (const SMArgumentEntry& arg : args)
            {
                if (declared.contains(arg.getName()) == false)
                    add(ownerId, kind, eSeverity::Error, 10, vtr("Argument '%1' is not a declared parameter").arg(arg.getName()));
            }
            for (const MethodParameter& p : target->getElements())
            {
                if ((mapped.contains(p.getName()) == false) && (target->hasDefaultValue(p.getName()) == false))
                    add(ownerId, kind, eSeverity::Error, 10, vtr("Required parameter '%1' is not mapped").arg(p.getName()));
            }
        }

        for (const SMArgumentEntry& arg : args)
            validateValueSource(ownerId, kind, arg.getSource(), arg.getValue(), arg.getExpression(), scope, true);
    }

    void Ctx::validateValueSource(uint32_t ownerId, eDocElementKind kind, eValueSource source, const QString& ref, const QString& expr, const Scope& scope, bool valuePosition)
    {
        switch (source)
        {
        case eValueSource::Value:
            break;      // A literal; parsing it against the target type is another validator's job.
        case eValueSource::Param:
            validateParamScope(ownerId, kind, ref, scope);
            break;
        case eValueSource::Attribute:
            if (mData.getAttributes().findElement(ref) == nullptr)
                add(ownerId, kind, eSeverity::Error, 6, vtr("Attribute '%1' is not declared").arg(ref));
            break;
        case eValueSource::Constant:
            if (mData.getConstants().findElement(ref) == nullptr)
                add(ownerId, kind, eSeverity::Error, 6, vtr("Constant '%1' is not declared").arg(ref));
            break;
        case eValueSource::Condition:
        {
            SMMethodEntry* c = mData.getMethods().findMethod(ref);
            if ((c == nullptr) || (c->isCondition() == false))
                add(ownerId, kind, eSeverity::Error, 6, vtr("Condition '%1' is not declared").arg(ref));
            else if (valuePosition && c->hasElements())
                add(ownerId, kind, eSeverity::Error, 21, vtr("Parameterized condition '%1' can be used as a left operand only").arg(ref));
            break;
        }
        case eValueSource::Expression:
            if (expr.trimmed().isEmpty())
                add(ownerId, kind, eSeverity::Error, 24, vtr("Expression source has an empty expression"));
            if (ref.isEmpty() == false)
                add(ownerId, kind, eSeverity::Error, 23, vtr("Expression source must not also carry a Value"));
            break;
        case eValueSource::Lambda:
            break;      // A lambda is only valid as a condition left operand; degenerate as a value source.
        }
    }

    void Ctx::validateParamScope(uint32_t ownerId, eDocElementKind kind, const QString& paramName, const Scope& scope)
    {
        if (scope.inTransition == false)
        {
            add(ownerId, kind, eSeverity::Error, 12, vtr("Param source '%1' is only valid on a transition").arg(paramName));
        }
        else if (scope.stimKind == SMTransitionEntry::eStimulusKind::Timer)
        {
            add(ownerId, kind, eSeverity::Error, 12, vtr("Param source '%1' is not available on a timer transition").arg(paramName));
        }
        else if ((scope.stimParams != nullptr) && (hasParam(scope.stimParams, paramName) == false))
        {
            add(ownerId, kind, eSeverity::Error, 12, vtr("The stimulus declares no parameter '%1'").arg(paramName));
        }
    }

    void Ctx::validateConditions(const SMConditionList& conditions, const Scope& scope)
    {
        for (SMConditionEntry* leaf : conditions.collectLeaves())
        {
            if (leaf == nullptr)
                continue;

            const uint32_t id = leaf->getId();

            if (leaf->isExpressionRow())
            {
                // An expression row carries only its verbatim text, and that text must be present.
                if (leaf->getExpression().trimmed().isEmpty())
                    add(id, eDocElementKind::Condition, eSeverity::Error, 24, vtr("Expression row has an empty expression"));
                if (leaf->hasOperator() || (leaf->getRhs().isEmpty() == false) || (leaf->getLhs().isEmpty() == false))
                    add(id, eDocElementKind::Condition, eSeverity::Error, 23, vtr("An expression row must not carry LHS/Operator/RHS"));
                continue;
            }
            if (leaf->isLambdaRow())
            {
                if (leaf->getBody().trimmed().isEmpty())
                    add(id, eDocElementKind::Condition, eSeverity::Error, 24, vtr("Lambda row has an empty body"));
                continue;
            }

            // A comparison row keeps its operands in the LHS/RHS fields, never as verbatim text.
            if (leaf->getExpression().isEmpty() == false)
                add(id, eDocElementKind::Condition, eSeverity::Error, 23, vtr("A comparison row must not carry an Expression"));

            // A parameterized condition call is allowed as the left operand (but not the right).
            validateOperand(id, leaf->getLhsKind(), leaf->getLhs(), scope, false);
            if (leaf->getLhsKind() == eValueSource::Condition)
            {
                SMMethodEntry* c = mData.getMethods().findMethod(leaf->getLhs());
                if ((c != nullptr) && c->isCondition() && c->hasElements())
                    validateArguments(id, eDocElementKind::Condition, c, leaf->getArguments(), scope);
            }

            if (leaf->hasOperator())
                validateOperand(id, leaf->getRhsKind(), leaf->getRhs(), scope, true);
        }
    }

    void Ctx::validateOperand(uint32_t ownerId, eValueSource kind, const QString& ref, const Scope& scope, bool isRhs)
    {
        switch (kind)
        {
        case eValueSource::Value:
        case eValueSource::Expression:
        case eValueSource::Lambda:
            break;      // Literal or verbatim: not a name reference.
        case eValueSource::Param:
            validateParamScope(ownerId, eDocElementKind::Condition, ref, scope);
            break;
        case eValueSource::Attribute:
            if (mData.getAttributes().findElement(ref) == nullptr)
                add(ownerId, eDocElementKind::Condition, eSeverity::Error, 6, vtr("Attribute '%1' is not declared").arg(ref));
            break;
        case eValueSource::Constant:
            if (mData.getConstants().findElement(ref) == nullptr)
                add(ownerId, eDocElementKind::Condition, eSeverity::Error, 6, vtr("Constant '%1' is not declared").arg(ref));
            break;
        case eValueSource::Condition:
        {
            SMMethodEntry* c = mData.getMethods().findMethod(ref);
            if ((c == nullptr) || (c->isCondition() == false))
                add(ownerId, eDocElementKind::Condition, eSeverity::Error, 6, vtr("Condition '%1' is not declared").arg(ref));
            else if (isRhs && c->hasElements())
                add(ownerId, eDocElementKind::Condition, eSeverity::Error, 21, vtr("Parameterized condition '%1' can be used as a left operand only").arg(ref));
            break;
        }
        }
    }

    QList<SMIssue> Ctx::run()
    {
        QList<LevelInfo> levels;
        collectLevels(mData.getStates(), true, 0, levels);

        for (const LevelInfo& info : levels)
            validateLevel(info);

        checkDuplicateIds();
        checkDuplicateStateNames(levels);
        checkRegistryNames();

        // Registry entries: their names must be identifiers, and their declared types must resolve.
        for (SMMethodEntry* m : mData.getMethods().getElements())
        {
            if (m == nullptr) continue;
            checkIdentifier(m->getId(), eDocElementKind::Method, m->getName());
            // An embedded condition owns its body and must supply one; every other method has none.
            const bool embedded = m->isCondition() && (m->getImplement() == SMMethodEntry::eImplement::Embedded);
            if (embedded && m->getBody().trimmed().isEmpty())
                add(m->getId(), eDocElementKind::Method, eSeverity::Error, 20, vtr("Embedded condition '%1' has an empty body").arg(m->getName()));
            if ((embedded == false) && (m->getBody().trimmed().isEmpty() == false))
                add(m->getId(), eDocElementKind::Method, eSeverity::Error, 20, vtr("A body is only allowed on an Embedded condition"));
            for (const MethodParameter& p : m->getElements())
                checkDataType(p.getId(), eDocElementKind::Method, p.getType());
        }
        for (SMEventEntry* e : mData.getEvents().getElements())
        {
            if (e == nullptr) continue;
            checkIdentifier(e->getId(), eDocElementKind::Event, e->getName());
            for (const MethodParameter& p : e->getElements())
                checkDataType(p.getId(), eDocElementKind::Event, p.getType());
        }
        for (const SMTimerEntry& t : mData.getTimers().getElements())
            checkIdentifier(t.getId(), eDocElementKind::Timer, t.getName());
        for (const SMAttributeEntry& a : mData.getAttributes().getElements())
        {
            checkIdentifier(a.getId(), eDocElementKind::Attribute, a.getName());
            checkDataType(a.getId(), eDocElementKind::Attribute, a.getType());
        }
        for (const ConstantEntry& c : mData.getConstants().getElements())
        {
            checkIdentifier(c.getId(), eDocElementKind::Constant, c.getName());
            checkDataType(c.getId(), eDocElementKind::Constant, c.getType());
        }
        for (const SMImportEntry& i : mData.getImports().getElements())
            checkIdentifier(i.getId(), eDocElementKind::Import, i.getName());
        for (DataTypeCustom* d : mData.getDataTypes().getCustomDataTypes())
            if (d != nullptr) checkIdentifier(d->getId(), eDocElementKind::DataType, d->getName());

        return mIssues;
    }

} // namespace

QList<SMIssue> SMValidator::validate(const StateMachineData& data)
{
    Ctx ctx(data);
    return ctx.run();
}
