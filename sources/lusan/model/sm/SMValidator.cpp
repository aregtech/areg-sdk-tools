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

#include "lusan/model/sm/SMGuardValidation.hpp"
#include "lusan/model/sm/SMOperationValidation.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/sm/SMImportResolver.hpp"
#include "lusan/data/sm/SMDataTypeData.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/common/ParamBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/FieldEntry.hpp"

#include "lusan/model/sm/SMTypeCompat.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/SMLiteralValidator.hpp"

#include <QCoreApplication>
#include <QHash>
#include <QRegularExpression>
#include <QSet>

namespace
{
    using eSeverity = SMIssue::eSeverity;
    using eValueSource = SMArgumentEntry::eValueSource;

    /**
     * \brief   The broad type family of an operand or mapping target, used to pick the right
     *          literal, comparison and assignment rules without re-inspecting the type name.
     **/
    enum class eTypeCat
    {
          Unknown       //!< Empty, unresolved, or an opaque/imported type: not judged.
        , Bool
        , Char
        , StringT
        , Numeric       //!< Any integer or floating-point primitive.
        , Enum
        , Structure
        , Container
    };

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
     * \struct  GuardUses
     * \brief   The declarations a transition guard references, by name.
     **/
    struct GuardUses
    {
        QSet<QString> attributes;
        QSet<QString> constants;
        QSet<QString> conditions;
        QSet<QString> types;
    };

    /**
     * \brief   Collects, from a committed guard tree, every declaration the guard references.
     *
     *          A guard is the SECOND place a declaration is used from, and the only one that
     *          binds by document ID rather than by name, so the usage sets (which are keyed by
     *          name) can only be filled through the reverse lookups. Without this walk an
     *          attribute used solely by a guard was reported as never referenced.
     **/
    void collectGuardUses(const StateMachineData& data, const SMGuardNode* node, GuardUses& out)
    {
        if (node == nullptr)
            return;

        switch (node->getKind())
        {
        case SMGuardNode::eKind::Attr:
        {
            const QString name = SMGuardSymbols::attributeName(data, node->getSymbolId());
            if (name.isEmpty() == false) out.attributes.insert(name);
            break;
        }
        case SMGuardNode::eKind::Const:
        {
            const QString name = SMGuardSymbols::constantName(data, node->getSymbolId());
            if (name.isEmpty() == false) out.constants.insert(name);
            break;
        }
        case SMGuardNode::eKind::Call:
        {
            const SMMethodEntry* method = SMGuardSymbols::method(data, node->getSymbolId());
            if (method != nullptr) out.conditions.insert(method->getName());
            break;
        }
        case SMGuardNode::eKind::Lit:
        {
            // `PowerState::On` names its enumeration as plainly as a declaration does, and for a
            // type used nowhere else the guard literal is the only thing keeping it alive.
            const int sep = static_cast<int>(node->getText().indexOf(QStringLiteral("::")));
            if (sep > 0) out.types.insert(node->getText().left(sep));
            break;
        }
        default:
            break;
        }

        for (const SMGuardNode* child : node->getChildren())
        {
            collectGuardUses(data, child, out);
        }
    }

    //!< Which kind a finding on one include row carries: the kind follows the row, so a `.fsml`
    //!< entry reports as Import and everything else as Include.
    eDocElementKind kindOfInclude(const IncludeEntry& entry)
    {
        return (includeKindOf(entry.getLocation(), QStringLiteral("fsml")) == eIncludeKind::Document
                ? eDocElementKind::Import : eDocElementKind::Include);
    }

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
        void add(uint32_t id, eDocElementKind kind, eSeverity sev, int rule, const QString& message, const QString& detail = QString());

        //!< Collects every machine level (root + painted nested) with its owner state ID.
        struct LevelInfo { const SMStateData* level; bool isRoot; uint32_t ownerId; };
        void collectLevels(const SMStateData& level, bool isRoot, uint32_t ownerId, QList<LevelInfo>& out) const;

        void checkIdentifier(uint32_t id, eDocElementKind kind, const QString& name);
        bool fragmentResolves(const QString& fragment) const;
        QString unresolvedFragment(const QString& typeName) const;
        bool typeResolves(const QString& typeName) const;
        void checkDataType(uint32_t id, eDocElementKind kind, const QString& typeName);

        void checkDuplicateIds();
        void collectIds(const SMStateData& level, QHash<uint32_t, int>& counts) const;
        void checkDuplicateStateNames(const QList<LevelInfo>& levels);
        void checkRegistryNames();

        void validateLevel(const LevelInfo& info);
        void checkAncestorShadowing(const SMStateData& level, QList<const SMStateEntry*>& ancestors);
        void validateState(const SMStateEntry& state, const SMStateData& level, bool isRootLevel);
        void validatePseudoStart(const SMStateEntry& state, bool isRootLevel);
        void validateTransitionKind(const SMStateEntry& owner, const SMTransitionEntry& tr);
        void validateTransition(const SMStateEntry& owner, const SMStateData& level, const SMTransitionEntry& tr);
        void validateOperations(const SMOperationList& ops, const Scope& scope);
        void validateArguments(uint32_t ownerId, eDocElementKind kind, const MethodBase* target, const QList<SMArgumentEntry>& args, const Scope& scope);
        void validateValueSource(uint32_t ownerId, eDocElementKind kind, eValueSource source, const QString& ref, const QString& expr, const Scope& scope, bool valuePosition);
        void validateConditions(const SMConditionList& conditions, const Scope& scope);
        void validateOperand(uint32_t ownerId, eValueSource kind, const QString& ref, const Scope& scope, bool isRhs);
        void validateParamScope(uint32_t ownerId, eDocElementKind kind, const QString& paramName, const Scope& scope);

        bool hasParam(const MethodBase* owner, const QString& name) const;

        // Type / literal rules (10.1 rules 13-17).
        const DataTypeCustom* customType(const QString& typeName) const;
        eTypeCat categorize(const QString& typeName) const;
        QString operandType(eValueSource kind, const QString& ref, const Scope& scope) const;
        void checkLiteral(uint32_t id, eDocElementKind kind, const QString& targetType, const QString& literal, int rule);
        void checkAssignment(uint32_t id, eDocElementKind kind, eValueSource src, const QString& ref, const QString& targetType, const Scope& scope, int mismatchRule);
        void checkComparisonTypes(const SMConditionEntry& leaf, const Scope& scope);

        // Warning rules (10.2 rules 1-11).
        void checkWarnings(const QList<LevelInfo>& levels);
        void checkGuards();
        void checkDescriptions();

        // Import rules.
        void checkImports(const QList<LevelInfo>& levels);

    private:
        const StateMachineData& mData;
        QList<SMIssue>          mIssues;
    };

    void Ctx::add(uint32_t id, eDocElementKind kind, eSeverity sev, int rule, const QString& message, const QString& detail)
    {
        SMIssue issue;
        issue.elementId = id;
        issue.kind      = kind;
        issue.severity  = sev;
        issue.rule      = (sev != eSeverity::Error) ? (SMValidator::WARNING_RULE_BASE + rule) : rule;
        issue.message   = message;
        issue.detail    = detail;
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

    bool Ctx::fragmentResolves(const QString& fragment) const
    {
        // Anything that is not a plain identifier after trimming -- a nested expression, a
        // pointer or reference suffix -- is left alone. This is a type registry lookup, not a
        // C++ parser.
        if (StateMachineData::isValidIdentifier(fragment) == false)
            return true;
        if (DataTypeFactory::fromString(fragment) != DataTypeBase::eCategory::Undefined)
            return true;
        return (mData.getDataTypes().findCustomDataType(fragment) != nullptr);
    }

    QString Ctx::unresolvedFragment(const QString& typeName) const
    {
        // A templated type is the container name plus its arguments, and every one of them is a
        // type that has to exist. Checking only the whole string let `NEArray<Foo>` through with
        // an undefined Foo.
        const QStringList fragments = typeName.split(QRegularExpression(QStringLiteral("[<>,]")), Qt::SkipEmptyParts);
        for (const QString& fragment : fragments)
        {
            const QString trimmed = fragment.trimmed();
            if ((trimmed.isEmpty() == false) && (fragmentResolves(trimmed) == false))
            {
                return trimmed;
            }
        }

        return QString();
    }

    bool Ctx::typeResolves(const QString& typeName) const
    {
        return unresolvedFragment(typeName).isEmpty();
    }

    void Ctx::checkDataType(uint32_t id, eDocElementKind kind, const QString& typeName)
    {
        if (typeName.isEmpty())
            return;

        // Report the offending fragment, not the whole string: "Foo does not resolve" is
        // actionable, "NEMap<String, Foo> does not resolve" is not.
        const QString unresolved = unresolvedFragment(typeName);
        if (unresolved.isEmpty() == false)
        {
            add(id, kind, eSeverity::Error, 6, vtr("Data type '%1' does not resolve").arg(unresolved));
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
        for (const IncludeEntry& i : mData.getIncludes().getElements())    counts[i.getId()] += 1;
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

        // Methods are unique per KIND, not per name: a trigger, an action and a condition may
        // all be called `on` -- each becomes a member of a different generated class, so they
        // can never collide. Two triggers named `on` is the error. Qualifying the name with
        // its kind expresses exactly that.
        QStringList mNames; QList<uint32_t> mIds;
        for (SMMethodEntry* m : mData.getMethods().getElements())
        {
            if (m != nullptr)
            {
                mNames << (QString::fromLatin1(SMMethodEntry::toString(m->getMethodType())) + QLatin1Char(' ') + m->getName());
                mIds << m->getId();
            }
        }

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

        // The include registry is keyed by location, so getName() is the path -- the alias is the
        // registry name here, and collecting getName() would quietly check the wrong thing.
        QStringList iNames; QList<uint32_t> iIds;
        for (const IncludeEntry* i : mData.machineImports()) { iNames << i->getAlias(); iIds << i->getId(); }
        dupWithin(iNames, iIds, eDocElementKind::Import);

        // A file listed twice is redundant rather than wrong: the second entry changes nothing
        // about the generated machine. The UI cannot create one, so this only fires on a
        // hand-edited or merged file, which is exactly when a quiet nudge helps.
        QHash<QString, int> seenLocations;
        for (const IncludeEntry& i : mData.getIncludes().getElements())
        {
            if (i.getLocation().isEmpty())
                continue;
            if (++seenLocations[i.getLocation()] > 1)
            {
                add(i.getId(), kindOfInclude(i), eSeverity::Warning, 4
                    , vtr("'%1' is included more than once").arg(i.getLocation()));
            }
        }

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
                validateState(*state, *info.level, info.isRoot);
        }
    }

    void Ctx::checkAncestorShadowing(const SMStateData& level, QList<const SMStateEntry*>& ancestors)
    {
        for (SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
                continue;

            for (const SMTransitionEntry* tr : state->getTransitions().getElements())
            {
                // An initial transition reacts to nothing, so nothing can shadow it -- and every one
                // of them would compare equal on the empty stimulus they all share.
                if ((tr == nullptr) || tr->isInitial() || tr->getStimulus().isEmpty())
                    continue;

                // Outermost ancestor first: that is the order the candidates are tried in, so the
                // first unguarded match is the one that actually wins and the one worth naming.
                const SMStateEntry* winner = nullptr;
                for (const SMStateEntry* ancestor : ancestors)
                {
                    for (const SMTransitionEntry* other : ancestor->getTransitions().getElements())
                    {
                        if ((other == nullptr) || other->isInitial() || other->hasCondition())
                            continue;

                        if ((other->getStimulusKind() == tr->getStimulusKind()) && (other->getStimulus() == tr->getStimulus()))
                        {
                            winner = ancestor;
                            break;
                        }
                    }

                    if (winner != nullptr)
                        break;
                }

                if (winner != nullptr)
                {
                    add(tr->getId(), eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_ANCESTOR_SHADOW
                       , vtr("The transition of '%1' on '%2' can never fire: its ancestor '%3' reacts to the same stimulus with no guard")
                            .arg(state->getName(), tr->getStimulus(), winner->getName())
                       , vtr("A composite's transition is eligible while any of its descendants is active and is tried before the descendant's own. "
                             "The first candidate with no guard always wins, so this one is dead code. Guard the transition on '%1', or remove this one.")
                            .arg(winner->getName()));
                }
            }

            if (state->hasNestedStates())
            {
                ancestors.append(state);
                checkAncestorShadowing(*state->getNestedStates(), ancestors);
                ancestors.removeLast();
            }
        }
    }

    void Ctx::validateState(const SMStateEntry& state, const SMStateData& level, bool isRootLevel)
    {
        const uint32_t id = state.getId();
        checkIdentifier(id, eDocElementKind::State, state.getName());

        const bool composite = state.isComposite();
        const bool hasSubstates = composite;

        // A Final state is terminal; a Start state cannot itself be a composite.
        if (state.getKind() == SMStateEntry::eStateKind::Final)
        {
            if (state.getTransitions().hasElements())
                add(id, eDocElementKind::State, eSeverity::Error, 8, vtr("A Final state cannot have outgoing transitions"));
            if (hasSubstates)
                add(id, eDocElementKind::State, eSeverity::Error, 8, vtr("A Final state cannot have substates"));
        }
        else if (state.isPseudoStart())
        {
            if (hasSubstates)
                add(id, eDocElementKind::State, eSeverity::Error, 9, vtr("A Start state cannot own substates"));

            validatePseudoStart(state, isRootLevel);
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
        if (state.isImportedSubmachine() && (mData.findImportByAlias(state.getSubmachine()) == nullptr))
            add(id, eDocElementKind::State, eSeverity::Error, 6, vtr("Submachine import '%1' is not declared").arg(state.getSubmachine()));
        if ((state.getOnFinal().isEmpty() == false) && (mData.getEvents().findEvent(state.getOnFinal()) == nullptr))
        {
            add(id, eDocElementKind::State, eSeverity::Error, 6, vtr("OnFinal event '%1' is not declared").arg(state.getOnFinal()));
        }
        else if (state.getOnFinal().isEmpty() == false)
        {
            // A completion hook is a bare attribute
            const SMEventEntry* onFinal = mData.getEvents().findEvent(state.getOnFinal());
            for (const MethodParameter& param : onFinal->getElements())
            {
                if (param.hasDefault() == false)
                {
                    add(id, eDocElementKind::State, eSeverity::Error, SMValidator::RULE_ARGUMENT_MAPPING
                        , vtr("OnFinal event '%1' has parameter '%2' with no default, and a completion hook cannot map one")
                            .arg(state.getOnFinal(), param.getName()));
                }
            }
        }

        // Entry / exit / do operations carry no stimulus scope.
        const Scope entryScope;
        validateOperations(state.getEntryList(), entryScope);
        validateOperations(state.getExitList(), entryScope);
        validateOperations(state.getDoList(), entryScope);

        // Do activity is a timer loop, so it has to say how often it ticks
        if ((state.getDoList().isEmpty() == false) && (state.getDoInterval() < SMStateEntry::MIN_DO_INTERVAL))
        {
            add(id, eDocElementKind::State, eSeverity::Error, SMValidator::RULE_DO_ACTIVITY
               , vtr("State '%1' has a Do activity with no repeat interval; a Do is a timer loop and Interval must be at least %2 ms. "
                     "To react to one stimulus without leaving the state, use an internal transition -- it names which stimulus.")
                    .arg(state.getName()).arg(SMStateEntry::MIN_DO_INTERVAL));
        }

        for (SMTransitionEntry* tr : state.getTransitions().getElements())
        {
            if (tr != nullptr)
                validateTransition(state, level, *tr);
        }
    }

    void Ctx::validatePseudoStart(const SMStateEntry& state, bool isRootLevel)
    {
        // Kind="Start" is a pseudo-state, not a state. It never becomes an entry
        const uint32_t id   = state.getId();
        const QString& name = state.getName();

        if (state.getEntryList().isEmpty() == false)
            add(id, eDocElementKind::State, eSeverity::Error, SMValidator::RULE_PSEUDO_START
               , vtr("Start state '%1' has entry operations; a Start is a marker and performs nothing").arg(name));
        if (state.getExitList().isEmpty() == false)
            add(id, eDocElementKind::State, eSeverity::Error, SMValidator::RULE_PSEUDO_START
               , vtr("Start state '%1' has exit operations; a Start is a marker and performs nothing").arg(name));
        if (state.getDoList().isEmpty() == false)
            add(id, eDocElementKind::State, eSeverity::Error, SMValidator::RULE_PSEUDO_START
               , vtr("Start state '%1' has a Do activity; a Start is a marker and performs nothing").arg(name));

        // The outgoing transitions are the level's initial transitions
        int outgoing = 0;
        int unguarded = 0;
        for (const SMTransitionEntry* tr : state.getTransitions().getElements())
        {
            if (tr == nullptr)
                continue;

            if (tr->hasTarget() == false)
                continue;

            ++outgoing;
            if (tr->hasCondition() == false)
                ++unguarded;

            if (isRootLevel && tr->hasCondition())
            {
                add(tr->getId(), eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_PSEUDO_START
                   , vtr("The root Start state '%1' has a conditional initial transition; there is no parent state to remain in, so the condition has no meaning")
                        .arg(name));
            }
        }

        if (outgoing == 0)
        {
            add(id, eDocElementKind::State, eSeverity::Error, SMValidator::RULE_PSEUDO_START
               , vtr("Start state '%1' has no outgoing transition, so the level never initialises").arg(name));
        }
        else if (isRootLevel)
        {
            if (outgoing > 1)
            {
                add(id, eDocElementKind::State, eSeverity::Error, SMValidator::RULE_PSEUDO_START
                   , vtr("The root Start state '%1' must have exactly one initial transition, not %2").arg(name).arg(outgoing));
            }
        }
        else if ((outgoing > 1) && (unguarded > 0))
        {
            // Document order is priority order
            add(id, eDocElementKind::State, eSeverity::Error, SMValidator::RULE_PSEUDO_START
               , vtr("Start state '%1' has %2 initial transitions, of which %3 carry no condition; with more than one every transition must carry one")
                    .arg(name).arg(outgoing).arg(unguarded));
        }
    }

    void Ctx::validateTransitionKind(const SMStateEntry& owner, const SMTransitionEntry& tr)
    {
        // `Kind` says what the transition is, `To` and `Stimulus` are then required or
        // forbidden rather than being the thing the meaning is read off.
        const uint32_t id    = tr.getId();
        const QString& where = owner.getName();
        const bool startOwned = owner.isPseudoStart();

        switch (tr.getKind())
        {
        case SMTransitionEntry::eTransitionKind::External:
            if (startOwned)
            {
                add(id, eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_TRANSITION_KIND
                   , vtr("Start state '%1' owns an External transition; a Start is left on entering the level, so everything it owns is an Initial transition").arg(where));
            }
            if (tr.hasTarget() == false)
            {
                // The whole reason `Kind` exists: this used to be byte-identical to an internal
                // transition, so a half-drawn edge silently meant "run the operations in place".
                add(id, eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_TRANSITION_KIND
                   , vtr("The transition on '%1' names no target state: it is an unfinished edge. Connect it, or make it Internal if it was meant to stay in the state").arg(where));
            }
            break;

        case SMTransitionEntry::eTransitionKind::Internal:
            if (startOwned)
            {
                add(id, eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_TRANSITION_KIND
                   , vtr("Start state '%1' owns an Internal transition; a Start performs nothing and everything it owns is an Initial transition").arg(where));
            }
            if (tr.hasTarget())
            {
                add(id, eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_TRANSITION_KIND
                   , vtr("The Internal transition on '%1' names a target state; an internal transition runs its operations without leaving the state").arg(where));
            }
            break;

        case SMTransitionEntry::eTransitionKind::Initial:
            if (startOwned == false)
            {
                add(id, eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_TRANSITION_KIND
                   , vtr("State '%1' owns an Initial transition, but only a Start state has one: an initial transition is taken on entering the level, and nothing enters a real state that way").arg(where));
            }
            if (tr.hasTarget() == false)
            {
                add(id, eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_TRANSITION_KIND
                   , vtr("The initial transition of '%1' names no target state, so the level never initialises").arg(where));
            }
            if (tr.getStimulus().isEmpty() == false)
            {
                add(id, eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_TRANSITION_KIND
                   , vtr("The initial transition of '%1' names a stimulus ('%2'); it is taken on entering the level, so nothing fires it").arg(where, tr.getStimulus()));
            }
            break;
        }

        // Required for the two kinds that react to something
        if ((tr.isInitial() == false) && tr.getStimulus().isEmpty())
        {
            add(id, eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_TRANSITION_KIND
               , vtr("The transition on '%1' names no stimulus; only an initial transition may leave it out").arg(where));
        }
    }

    void Ctx::validateTransition(const SMStateEntry& owner, const SMStateData& level, const SMTransitionEntry& tr)
    {
        const uint32_t id = tr.getId();

        validateTransitionKind(owner, tr);

        // A named target must resolve to a state, and that state must be a sibling of the owner.
        if (tr.hasTarget())
        {
            const uint32_t targetId = tr.getToId();
            const SMStateEntry* sibling = level.findStateById(targetId);
            if (sibling == nullptr)
            {
                const SMStateEntry* target = mData.findStateById(targetId);
                if (target == nullptr)
                    add(id, eDocElementKind::Transition, eSeverity::Error, 6, vtr("Transition target does not resolve"));
                else
                    add(id, eDocElementKind::Transition, eSeverity::Error, 7, vtr("Transition target '%1' is not a sibling state").arg(target->getName()));
            }
            else if (sibling->isPseudoStart())
            {
                // a pseudo-state exists to be passed through
                if (targetId == owner.getId())
                    add(id, eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_PSEUDO_START
                       , vtr("The initial transition of Start state '%1' targets the Start itself, so the level never initialises").arg(sibling->getName()));
                else
                    add(id, eDocElementKind::Transition, eSeverity::Error, SMValidator::RULE_PSEUDO_START
                       , vtr("Transition targets the Start state '%1'; a Start is a marker the machine never occupies").arg(sibling->getName()));
            }
        }

        // The stimulus must exist in the registry its kind selects; that registry also
        // supplies the parameter names a Param reference is later checked against.
        Scope scope;
        scope.inTransition = true;
        scope.stimKind = tr.getStimulusKind();
        const QString& stim = tr.getStimulus();

        // An initial transition names no stimulus at all
        if ((tr.isInitial() == false) && (stim.isEmpty() == false))
        {
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
                SMMethodEntry* action = mData.getMethods().findAction(call->getAction());
                if (action == nullptr)
                    add(id, eDocElementKind::Operation, eSeverity::Error, 6, vtr("Action '%1' is not declared").arg(call->getAction()));
                validateArguments(id, eDocElementKind::Operation, action, call->getArguments(), scope);
                break;
            }
            case SMOperationBase::eOperation::AttributeSet:
            {
                SMAttributeSet* set = static_cast<SMAttributeSet*>(op);
                const SMAttributeEntry* attr = mData.getAttributes().findElement(set->getAttribute());
                if (attr == nullptr)
                    add(id, eDocElementKind::Operation, eSeverity::Error, 6, vtr("Attribute '%1' is not declared").arg(set->getAttribute()));
                validateValueSource(id, eDocElementKind::Operation, set->getSource(), set->getValue(), set->getExpression(), scope, true);

                // The assigned value must fit the attribute's declared type
                if (attr != nullptr)
                    checkAssignment(id, eDocElementKind::Operation, set->getSource(), set->getValue(), attr->getType(), scope, 17);
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
        QHash<QString, QString> paramTypes;
        if (target != nullptr)
        {
            for (const MethodParameter& p : target->getElements())
            {
                paramTypes.insert(p.getName(), p.getType());
            }

            mIssues += SMOperationValidation::argumentIssues(*target, args, ownerId, kind, QString());
        }

        for (const SMArgumentEntry& arg : args)
        {
            validateValueSource(ownerId, kind, arg.getSource(), arg.getValue(), arg.getExpression(), scope, true);

            auto it = paramTypes.constFind(arg.getName());
            if (it != paramTypes.constEnd())
                checkAssignment(ownerId, kind, arg.getSource(), arg.getValue(), it.value(), scope, 13);
        }
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
            SMMethodEntry* c = mData.getMethods().findCondition(ref);
            if (c == nullptr)
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
                SMMethodEntry* c = mData.getMethods().findCondition(leaf->getLhs());
                if ((c != nullptr) && c->hasElements())
                    validateArguments(id, eDocElementKind::Condition, c, leaf->getArguments(), scope);
            }

            if (leaf->hasOperator())
                validateOperand(id, leaf->getRhsKind(), leaf->getRhs(), scope, true);

            checkComparisonTypes(*leaf, scope);
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
            SMMethodEntry* c = mData.getMethods().findCondition(ref);
            if (c == nullptr)
                add(ownerId, eDocElementKind::Condition, eSeverity::Error, 6, vtr("Condition '%1' is not declared").arg(ref));
            else if (isRhs && c->hasElements())
                add(ownerId, eDocElementKind::Condition, eSeverity::Error, 21, vtr("Parameterized condition '%1' can be used as a left operand only").arg(ref));
            break;
        }
        }
    }

    const DataTypeCustom* Ctx::customType(const QString& typeName) const
    {
        return mData.getDataTypes().findCustomDataType(typeName);
    }

    eTypeCat Ctx::categorize(const QString& typeName) const
    {
        if (typeName.isEmpty())
            return eTypeCat::Unknown;
        if (typeName == "bool")     return eTypeCat::Bool;
        if (typeName == "char")     return eTypeCat::Char;
        if (typeName == "String")   return eTypeCat::StringT;
        if (SMTypeCompat::isPrimitive(typeName))
            return eTypeCat::Numeric;   // every remaining primitive is an integer or float.

        const DataTypeCustom* custom = customType(typeName);
        if (custom == nullptr)
            return eTypeCat::Unknown;   // an unresolved name
        if (custom->isEnumeration())    return eTypeCat::Enum;
        if (custom->isStructure())      return eTypeCat::Structure;
        if (custom->isContainer())      return eTypeCat::Container;
        return eTypeCat::Unknown;       // imported / opaque: the user owns its correctness.
    }

    QString Ctx::operandType(eValueSource kind, const QString& ref, const Scope& scope) const
    {
        // The declared type of a non-literal source; a literal or verbatim operand has no
        // type of its own (it is judged against the position it fills).
        switch (kind)
        {
        case eValueSource::Param:
            if (scope.stimParams != nullptr)
            {
                for (const MethodParameter& p : scope.stimParams->getElements())
                {
                    if (p.getName() == ref)
                        return p.getType();
                }
            }
            return QString();
        case eValueSource::Attribute:
        {
            const SMAttributeEntry* a = mData.getAttributes().findElement(ref);
            return (a != nullptr) ? a->getType() : QString();
        }
        case eValueSource::Constant:
        {
            const ConstantEntry* c = mData.getConstants().findElement(ref);
            return (c != nullptr) ? c->getType() : QString();
        }
        case eValueSource::Condition:
        {
            SMMethodEntry* m = mData.getMethods().findCondition(ref);
            return (m != nullptr) ? m->getReturn() : QString();
        }
        default:
            return QString();
        }
    }

    void Ctx::checkLiteral(uint32_t id, eDocElementKind kind, const QString& targetType, const QString& literal, int rule)
    {
        // An absent literal is never a syntax error (a default may fill it).
        if (literal.isEmpty())
            return;

        switch (categorize(targetType))
        {
        case eTypeCat::Unknown:
            return;
        case eTypeCat::Structure:
        case eTypeCat::Container:
            add(id, kind, eSeverity::Error, rule, vtr("Type '%1' has no literal form").arg(targetType));
            return;
        case eTypeCat::Enum:
        {
            const DataTypeEnum* e = dynamic_cast<const DataTypeEnum*>(customType(targetType));
            if ((e != nullptr) && (e->hasElement(literal) == false))
                add(id, kind, eSeverity::Error, rule, vtr("'%1' is not an enumerator of '%2'").arg(literal, targetType));
            return;
        }
        default:
        {
            const QString reason = SMLiteralValidator::validate(targetType, literal);
            if (reason.isEmpty() == false)
                add(id, kind, eSeverity::Error, rule, vtr("Invalid %1 literal '%2': %3").arg(targetType, literal, reason));
            return;
        }
        }
    }

    void Ctx::checkAssignment(uint32_t id, eDocElementKind kind, eValueSource src, const QString& ref, const QString& targetType, const Scope& scope, int mismatchRule)
    {
        if (targetType.isEmpty())
            return;                                     // no declared target: nothing to judge against.

        if (src == eValueSource::Value)
        {
            checkLiteral(id, kind, targetType, ref, 15);
            return;
        }
        if ((src == eValueSource::Expression) || (src == eValueSource::Lambda))
            return;                                     // verbatim: the user owns type correctness.

        const QString sourceType = operandType(src, ref, scope);
        if (sourceType.isEmpty())
            return;                                     // unresolved source

        // narrowing is legal C++ that the generator writes as `static_cast<Target>(...)`
        switch (SMTypeCompat::rank(sourceType, targetType))
        {
        case SMTypeCompat::eRank::Narrows:
            add(id, kind, eSeverity::Warning, mismatchRule
              , vtr("Narrowing '%1' to '%2': the value may be truncated").arg(sourceType, targetType)
              , vtr("A legal but lossy conversion. The generated code casts it explicitly, so this does not block generation -- check that the value cannot exceed the target's range."));
            break;

        case SMTypeCompat::eRank::Mismatch:
            add(id, kind, eSeverity::Error, mismatchRule
              , vtr("No conversion from '%1' to '%2'").arg(sourceType, targetType)
              , vtr("The two types are unrelated, so there is no cast the generated code could write. Change the source or the declared type."));
            break;

        default:
            break;      // Exact, Converts, Unknown: nothing to report.
        }
    }

    void Ctx::checkComparisonTypes(const SMConditionEntry& leaf, const Scope& scope)
    {
        const uint32_t id = leaf.getId();
        const QString lhsType = operandType(leaf.getLhsKind(), leaf.getLhs(), scope);

        // A boolean-test row (no operator): the single operand must be bool (rule 16).
        if (leaf.hasOperator() == false)
        {
            if (leaf.getLhsKind() == eValueSource::Value)
            {
                if ((leaf.getLhs().isEmpty() == false) && (SMLiteralValidator::validate("bool", leaf.getLhs()).isEmpty() == false))
                    add(id, eDocElementKind::Condition, eSeverity::Error, 16, vtr("A boolean-test operand must be a bool value"));
            }
            else
            {
                const eTypeCat cat = categorize(lhsType);
                if ((cat != eTypeCat::Unknown) && (cat != eTypeCat::Bool))
                    add(id, eDocElementKind::Condition, eSeverity::Error, 16, vtr("A boolean-test operand must be bool, not '%1'").arg(lhsType));
            }
            return;
        }

        const QString rhsType = operandType(leaf.getRhsKind(), leaf.getRhs(), scope);
        const SMConditionEntry::eOperator op = leaf.getOperator();
        const bool ordering = (op != SMConditionEntry::eOperator::Equal) && (op != SMConditionEntry::eOperator::NotEqual);
        const eTypeCat lc = categorize(lhsType);
        const eTypeCat rc = categorize(rhsType);

        // no structure/container operand, and no ordering on bool/String/enumeration.
        bool flagged14 = false;
        if ((lc == eTypeCat::Structure) || (lc == eTypeCat::Container) || (rc == eTypeCat::Structure) || (rc == eTypeCat::Container))
        {
            add(id, eDocElementKind::Condition, eSeverity::Error, 14, vtr("A structure or container cannot be compared in a condition"));
            flagged14 = true;
        }
        else if (ordering && ((lc == eTypeCat::Bool) || (lc == eTypeCat::StringT) || (lc == eTypeCat::Enum)
                           || (rc == eTypeCat::Bool) || (rc == eTypeCat::StringT) || (rc == eTypeCat::Enum)))
        {
            add(id, eDocElementKind::Condition, eSeverity::Error, 14, vtr("An ordering operator is not allowed on bool, String or enumeration operands"));
            flagged14 = true;
        }

        // literal operand must parse against the other operand's declared type.
        if ((leaf.getLhsKind() == eValueSource::Value) && (rhsType.isEmpty() == false))
            checkLiteral(id, eDocElementKind::Condition, rhsType, leaf.getLhs(), 15);
        if ((leaf.getRhsKind() == eValueSource::Value) && (lhsType.isEmpty() == false))
            checkLiteral(id, eDocElementKind::Condition, lhsType, leaf.getRhs(), 15);

        // both operand types known and no implicit widening path between them.
        if ((flagged14 == false) && (lhsType.isEmpty() == false) && (rhsType.isEmpty() == false))
        {
            const QString reason = SMTypeCompat::areComparable(lhsType, op, rhsType);
            if (reason.isEmpty() == false)
                add(id, eDocElementKind::Condition, eSeverity::Error, 13, vtr("Type mismatch: %1").arg(reason));
        }

        // a comparison of two design-time constants (literals or Constant sources).
        const bool lConst = (leaf.getLhsKind() == eValueSource::Value) || (leaf.getLhsKind() == eValueSource::Constant);
        const bool rConst = (leaf.getRhsKind() == eValueSource::Value) || (leaf.getRhsKind() == eValueSource::Constant);
        if (lConst && rConst)
            add(id, eDocElementKind::Condition, eSeverity::Warning, 9, vtr("Both operands are constant at design time"));
    }

    QList<SMIssue> Ctx::run()
    {
        QList<LevelInfo> levels;
        collectLevels(mData.getStates(), true, 0, levels);

        for (const LevelInfo& info : levels)
            validateLevel(info);

        QList<const SMStateEntry*> ancestors;
        checkAncestorShadowing(mData.getStates(), ancestors);

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
            // Return belongs to a condition method and is a declared type like any other.
            if (m->isCondition())
                checkDataType(m->getId(), eDocElementKind::Method, m->getReturn());
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
        for (const IncludeEntry* i : mData.machineImports())
        {
            if (i->getAlias().isEmpty())
                add(i->getId(), eDocElementKind::Import, eSeverity::Error, 18
                    , vtr("Imported machine '%1' has no alias, so no state can host it").arg(i->getLocation()));
            else
                checkIdentifier(i->getId(), eDocElementKind::Import, i->getAlias());
        }
        for (DataTypeCustom* d : mData.getDataTypes().getCustomDataTypes())
            if (d != nullptr) checkIdentifier(d->getId(), eDocElementKind::DataType, d->getName());

        checkImports(levels);
        checkWarnings(levels);
        checkGuards();
        checkDescriptions();

        return mIssues;
    }

    void Ctx::checkImports(const QList<LevelInfo>& levels)
    {
        QSet<QString> brokenAliases;
        for (const IncludeEntry* import : mData.machineImports())
        {
            const IncludeEntry& entry = *import;
            const uint32_t id = entry.getId();
            const QString  name = entry.getAlias();

            QStringList cycle;
            if (SMImportResolver::findCycle(mData, entry, cycle))
            {
                add(id, eDocElementKind::Import, eSeverity::Error, 19
                    , vtr("Import '%1' closes a cycle: %2").arg(name, cycle.join(QStringLiteral(" imports "))));
                brokenAliases.insert(name);
                continue;
            }

            const SMImportResolver::Resolution resolution = SMImportResolver::resolve(mData, entry);
            switch (resolution.state)
            {
            case SMImportResolver::eState::NoLocation:
                add(id, eDocElementKind::Import, eSeverity::Error, 19
                    , vtr("Import '%1' names no file").arg(name));
                brokenAliases.insert(name);
                continue;

            case SMImportResolver::eState::NotFound:
                add(id, eDocElementKind::Import, eSeverity::Error, 19
                    , vtr("Import '%1' points at a file that does not exist: %2").arg(name, entry.getLocation()));
                brokenAliases.insert(name);
                continue;

            case SMImportResolver::eState::ParseFailed:
                add(id, eDocElementKind::Import, eSeverity::Error, 19
                    , vtr("Import '%1' cannot be read as a state machine: %2").arg(name, entry.getLocation()));
                brokenAliases.insert(name);
                continue;

            case SMImportResolver::eState::Resolved:
                break;
            }

            // The cycle check terminates but does not bound the work
            QStringList deep;
            if (SMImportResolver::importDepth(resolution.absolutePath, SMImportResolver::MAX_IMPORT_DEPTH, deep)
                > SMImportResolver::MAX_IMPORT_DEPTH)
            {
                add(id, eDocElementKind::Import, eSeverity::Error, 19
                    , vtr("Imported machine '%1' nests more than %2 levels of imports")
                        .arg(name).arg(SMImportResolver::MAX_IMPORT_DEPTH));
                brokenAliases.insert(name);
                continue;
            }

            const VersionNumber& pinned = entry.getVersion();
            const VersionNumber& actual = resolution.actualVersion;
            if (pinned.getMajor() != actual.getMajor())
            {
                add(id, eDocElementKind::Import, eSeverity::Error, 22
                    , vtr("Import '%1' is pinned to version %2 but the file is %3; use Update to accept it")
                        .arg(name, pinned.toString(), actual.toString()));
            }
            else if (pinned.getMinor() != actual.getMinor())
            {
                add(id, eDocElementKind::Import, eSeverity::Warning, 12
                    , vtr("Import '%1' is pinned to version %2, the file is %3; updating is recommended")
                        .arg(name, pinned.toString(), actual.toString()));
            }
            else if (pinned.getPatch() != actual.getPatch())
            {
                add(id, eDocElementKind::Import, eSeverity::Info, 12
                    , vtr("Import '%1' is pinned to version %2, the file is %3")
                        .arg(name, pinned.toString(), actual.toString()));
            }

            // Each document is generated by its own `Threading`
            const SMOverviewData::eThreading hostMode = mData.getOverview().getThreading();
            const SMOverviewData::eThreading impMode  = resolution.document->getOverview().getThreading();
            if ((hostMode == SMOverviewData::eThreading::Shared) && (impMode == SMOverviewData::eThreading::Local))
            {
                add(id, eDocElementKind::Import, eSeverity::Error, 26
                    , vtr("Import '%1' is a Local machine but this machine is Shared: the import would be reachable from several threads without locking").arg(name));
            }
            else if ((hostMode == SMOverviewData::eThreading::Local) && (impMode == SMOverviewData::eThreading::Shared))
            {
                add(id, eDocElementKind::Import, eSeverity::Warning, 13
                    , vtr("Import '%1' is a Shared machine but this machine is Local: the import pays for locking it cannot need").arg(name));
            }
        }

        if (brokenAliases.isEmpty())
        {
            return;
        }

        for (const LevelInfo& info : levels)
        {
            for (const SMStateEntry* state : info.level->getElements())
            {
                if ((state != nullptr) && brokenAliases.contains(state->getSubmachine()))
                {
                    add(state->getId(), eDocElementKind::State, eSeverity::Error, 19
                        , vtr("State '%1' hosts submachine '%2', which is not available").arg(state->getName(), state->getSubmachine()));
                }
            }
        }
    }

    void Ctx::checkGuards()
    {
        for (const SMGuardValidation::Finding& finding : SMGuardValidation::validate(mData))
        {
            SMIssue issue;
            issue.elementId = finding.target.getId();
            issue.kind      = (finding.target.getOwner() == SMGuardRef::eOwner::DoActivity)
                                    ? eDocElementKind::State
                                    : eDocElementKind::Transition;
            issue.severity  = finding.severity;
            issue.rule      = SMValidator::RULE_GUARD;
            issue.message   = finding.message;
            issue.location  = finding.location;
            issue.detail    = SMGuardValidation::describe(finding.kind);
            mIssues.append(issue);
        }
    }

    void Ctx::checkDescriptions()
    {
        // A description is what the code generator turns into the element's comment
        const QString detail = vtr("The code generator writes the description as the comment of the generated element. Add one so the generated code explains itself.");
        auto note = [this, &detail](uint32_t id, eDocElementKind kind, const QString& message, const QString& description)
        {
            if (description.trimmed().isEmpty())
            {
                add(id, kind, eSeverity::Info, SMValidator::RULE_MISSING_DESCRIPTION, message, detail);
            }
        };

        const SMOverviewData& overview = mData.getOverview();
        note(overview.getId(), eDocElementKind::Overview
            , vtr("Machine '%1' has no description").arg(overview.getName()), overview.getDescription());

        for (DataTypeCustom* d : mData.getDataTypes().getCustomDataTypes())
        {
            if (d == nullptr)
                continue;

            note(d->getId(), eDocElementKind::DataType, vtr("Data type '%1' has no description").arg(d->getName()), d->getDescription());
            if (d->isStructure())
            {
                for (const FieldEntry& f : static_cast<const DataTypeStructure*>(d)->getElements())
                    note(f.getId(), eDocElementKind::DataType
                        , vtr("Field '%1' of data type '%2' has no description").arg(f.getName(), d->getName()), f.getDescription());
            }
            else if (d->isEnumeration())
            {
                for (const EnumEntry& e : static_cast<const DataTypeEnum*>(d)->getElements())
                    note(e.getId(), eDocElementKind::DataType
                        , vtr("Field '%1' of data type '%2' has no description").arg(e.getName(), d->getName()), e.getDescription());
            }
        }

        for (const SMAttributeEntry& a : mData.getAttributes().getElements())
            note(a.getId(), eDocElementKind::Attribute, vtr("Attribute '%1' has no description").arg(a.getName()), a.getDescription());

        for (SMMethodEntry* m : mData.getMethods().getElements())
        {
            if (m == nullptr)
                continue;

            note(m->getId(), eDocElementKind::Method, vtr("Method '%1' has no description").arg(m->getName()), m->getDescription());
            for (const MethodParameter& p : m->getElements())
                note(p.getId(), eDocElementKind::Method
                    , vtr("Parameter '%1' of method '%2' has no description").arg(p.getName(), m->getName()), p.getDescription());
        }

        for (SMEventEntry* e : mData.getEvents().getElements())
        {
            if (e != nullptr)
                note(e->getId(), eDocElementKind::Event, vtr("Event '%1' has no description").arg(e->getName()), e->getDescription());
        }

        for (const SMTimerEntry& t : mData.getTimers().getElements())
            note(t.getId(), eDocElementKind::Timer, vtr("Timer '%1' has no description").arg(t.getName()), t.getDescription());

        for (const ConstantEntry& c : mData.getConstants().getElements())
            note(c.getId(), eDocElementKind::Constant, vtr("Constant '%1' has no description").arg(c.getName()), c.getDescription());
    }

    void Ctx::checkWarnings(const QList<LevelInfo>& levels)
    {
        using eOp = SMOperationBase::eOperation;

        // Usage sets, filled by one document-wide walk; W4/W5/W6 are decided against them.
        QSet<QString> triggersUsed, eventsReacted, eventsSent, timersReacted, timersStarted, timersStopped;
        QSet<QString> actionsUsed, conditionsUsed, attrsRead, attrsWritten, constsUsed, importsUsed, typesUsed;

        // The elements whose send/react status is resolved after the sets are complete.
        struct Use { uint32_t id; eDocElementKind kind; QString name; };
        QList<Use> reactedEvents, sentEvents, reactedTimers, startedTimers;

        auto noteType = [&typesUsed](const QString& t) { if (t.isEmpty() == false) typesUsed.insert(t); };
        auto noteSource = [&](eValueSource src, const QString& ref)
        {
            switch (src)
            {
            case eValueSource::Attribute:  attrsRead.insert(ref);      break;
            case eValueSource::Constant:   constsUsed.insert(ref);     break;
            case eValueSource::Condition:  conditionsUsed.insert(ref); break;
            default: break;
            }
        };
        auto noteOps = [&](const SMOperationList& ops)
        {
            for (SMOperationBase* op : ops.getOperations())
            {
                if (op == nullptr)
                    continue;
                switch (op->getOperationType())
                {
                case eOp::ActionCall:
                {
                    SMActionCall* a = static_cast<SMActionCall*>(op);
                    actionsUsed.insert(a->getAction());
                    for (const SMArgumentEntry& arg : a->getArguments()) noteSource(arg.getSource(), arg.getValue());
                    break;
                }
                case eOp::AttributeSet:
                {
                    SMAttributeSet* s = static_cast<SMAttributeSet*>(op);
                    attrsWritten.insert(s->getAttribute());
                    noteSource(s->getSource(), s->getValue());
                    break;
                }
                case eOp::TimerStart:
                {
                    SMTimerStart* t = static_cast<SMTimerStart*>(op);
                    timersStarted.insert(t->getTimer());
                    startedTimers.append({ op->getId(), eDocElementKind::Operation, t->getTimer() });
                    break;
                }
                case eOp::TimerStop:
                    timersStopped.insert(static_cast<SMTimerStop*>(op)->getTimer());
                    break;
                case eOp::EventSend:
                {
                    SMEventSend* e = static_cast<SMEventSend*>(op);
                    eventsSent.insert(e->getEvent());
                    sentEvents.append({ op->getId(), eDocElementKind::Operation, e->getEvent() });
                    for (const SMArgumentEntry& arg : e->getArguments()) noteSource(arg.getSource(), arg.getValue());
                    break;
                }
                case eOp::InlineCode:
                    if (static_cast<SMInlineCode*>(op)->getBody().trimmed().isEmpty())
                        add(op->getId(), eDocElementKind::Operation, eSeverity::Warning, 11, vtr("Inline code block is empty"));
                    break;
                }
            }
        };

        // Which state owns each level, so a substate can ask what its ancestors do. A composite is
        // left by its own transitions while any of its descendants is active, so an ancestor that
        // leaves takes the substate with it.
        QHash<uint32_t, uint32_t> ownerOf;
        for (const LevelInfo& info : levels)
        {
            for (const SMStateEntry* st : info.level->getElements())
            {
                if (st != nullptr)
                    ownerOf.insert(st->getId(), info.ownerId);
            }
        }

        // A way out of the state: any transition that is not internal. An internal one reacts and
        // stays, so a state that owns nothing else is still a place the machine cannot leave.
        auto hasExit = [](const SMStateEntry& state)
        {
            for (const SMTransitionEntry* tr : state.getTransitions().getElements())
            {
                if ((tr != nullptr) && (tr->isInternal() == false))
                    return true;
            }

            return false;
        };

        auto ancestorExits = [&](uint32_t stateId)
        {
            for (uint32_t owner = ownerOf.value(stateId, 0); owner != 0; owner = ownerOf.value(owner, 0))
            {
                const SMStateEntry* ancestor = mData.findStateById(owner);
                if (ancestor == nullptr)
                    break;
                if (hasExit(*ancestor))
                    return true;
            }

            return false;
        };

        // Incoming (sibling) transition targets per level -- reachability and history re-entry.
        // The second set drops everything the Start state points at: a Start is left once and can
        // never be targeted, so a transition out of it fires exactly once. That is an entry, not a
        // re-entry, and history is only worth anything on a state the machine comes back to.
        QHash<const SMStateData*, QSet<uint32_t>> incoming;
        QHash<const SMStateData*, QSet<uint32_t>> reentered;
        for (const LevelInfo& info : levels)
        {
            QSet<uint32_t>& targets = incoming[info.level];
            QSet<uint32_t>& repeats = reentered[info.level];
            for (SMStateEntry* st : info.level->getElements())
            {
                if (st == nullptr)
                    continue;
                const bool fromStart = (st->getKind() == SMStateEntry::eStateKind::Start);
                for (SMTransitionEntry* tr : st->getTransitions().getElements())
                {
                    if ((tr == nullptr) || (tr->hasTarget() == false))
                        continue;
                    targets.insert(tr->getToId());
                    if (fromStart == false)
                        repeats.insert(tr->getToId());
                }
            }
        }

        for (const LevelInfo& info : levels)
        {
            const QSet<uint32_t>& targets = incoming[info.level];
            const QSet<uint32_t>& repeats = reentered[info.level];
            for (SMStateEntry* st : info.level->getElements())
            {
                if (st == nullptr)
                    continue;
                const uint32_t sid = st->getId();
                const bool composite = st->isComposite();

                if (st->getOnFinal().isEmpty() == false)
                {
                    eventsSent.insert(st->getOnFinal());            // OnFinal counts as sending its event.
                    sentEvents.append({ sid, eDocElementKind::State, st->getOnFinal() });
                }
                if (st->isImportedSubmachine())
                    importsUsed.insert(st->getSubmachine());

                noteOps(st->getEntryList());
                noteOps(st->getExitList());
                noteOps(st->getDoList());

                // A Do stop condition is a use of everything it references, exactly as a guard is:
                // an attribute read only by an `Until` is read, and reporting it unreferenced was
                // the whole class of wrongness the reference-counting walk exists to avoid.
                {
                    GuardUses until;
                    collectGuardUses(mData, st->getDoUntil().getTree(), until);
                    attrsRead      += until.attributes;
                    constsUsed     += until.constants;
                    conditionsUsed += until.conditions;
                    typesUsed      += until.types;
                }

                // Unreachable: nothing at this level targets it, and the level's Start does not
                // descend into it either -- the Start's initial transitions are in `targets` too.
                // A warning and never an error: a half-drawn machine is the normal intermediate
                // state of an editor, and the author is the one who knows whether it is finished.
                if ((st->getKind() != SMStateEntry::eStateKind::Start) && (targets.contains(sid) == false))
                {
                    const SMStateEntry* owner = (info.isRoot ? nullptr : mData.findStateById(info.ownerId));
                    const QString where = (owner != nullptr ? QLatin1Char('\'') + owner->getName() + QLatin1Char('\'')
                                                            : vtr("the root level"));
                    add(sid, eDocElementKind::State, eSeverity::Warning, 1
                       , vtr("State '%1' on %2 is unreachable: no transition targets it and no Start enters it").arg(st->getName(), where));
                }

                // A substate is not trapped by having no transition of its own: an enclosing
                // composite that reacts to a stimulus carries every descendant out with it.
                if ((st->getKind() == SMStateEntry::eStateKind::Normal) && (composite == false)
                    && (hasExit(*st) == false) && (ancestorExits(sid) == false))
                {
                    add(sid, eDocElementKind::State, eSeverity::Warning, 2, vtr("State '%1' is a dead end (no outgoing transition)").arg(st->getName()));
                }

                if (composite && (st->getHistory() != SMStateEntry::eHistory::None) && (repeats.contains(sid) == false))
                    add(sid, eDocElementKind::State, eSeverity::Warning, 10, vtr("History on '%1' is never re-entered").arg(st->getName()));

                QSet<QString> unconditionalStimuli;
                for (SMTransitionEntry* tr : st->getTransitions().getElements())
                {
                    if (tr == nullptr)
                        continue;
                    const uint32_t tid = tr->getId();
                    const QString& stim = tr->getStimulus();
                    // An initial transition reacts to nothing, so it is not a use of anything in the
                    // stimulus registries -- and counting its empty name as one would let a declared
                    // trigger named "" look referenced.
                    if (tr->isInitial() == false)
                    {
                        switch (tr->getStimulusKind())
                        {
                        case SMTransitionEntry::eStimulusKind::Trigger:
                            triggersUsed.insert(stim);
                            break;
                        case SMTransitionEntry::eStimulusKind::Event:
                            eventsReacted.insert(stim);
                            reactedEvents.append({ tid, eDocElementKind::Transition, stim });
                            break;
                        case SMTransitionEntry::eStimulusKind::Timer:
                            timersReacted.insert(stim);
                            reactedTimers.append({ tid, eDocElementKind::Transition, stim });
                            break;
                        }
                    }

                    for (SMConditionEntry* leaf : tr->getConditions().collectLeaves())
                    {
                        if (leaf == nullptr)
                            continue;
                        noteSource(leaf->getLhsKind(), leaf->getLhs());
                        if (leaf->hasOperator()) noteSource(leaf->getRhsKind(), leaf->getRhs());
                        for (const SMArgumentEntry& arg : leaf->getArguments()) noteSource(arg.getSource(), arg.getValue());
                    }
                    noteOps(tr->getOperations());

                    // The canonical guard counts as a use exactly as a legacy condition row does.
                    // A draft's last-good tree is walked too: the names in it are still what the
                    // document refers to, and reporting them unused while the user is mid-edit
                    // would make the list flicker on every keystroke.
                    GuardUses guard;
                    collectGuardUses(mData, tr->getGuard().getTree(), guard);
                    attrsRead      += guard.attributes;
                    constsUsed     += guard.constants;
                    conditionsUsed += guard.conditions;
                    typesUsed      += guard.types;

                    // Condition-free means neither a legacy condition row nor a canonical guard.
                    const bool unconditional = tr->getConditions().collectLeaves().isEmpty() && (tr->getGuard().hasContent() == false);

                    if (tr->isInternal() && tr->getOperations().isEmpty() && unconditional)
                        add(tid, eDocElementKind::Transition, eSeverity::Warning, 7, vtr("Internal transition on '%1' has no operations or conditions").arg(stim));

                    if (tr->isInitial() == false)
                    {
                        const QString key = QString::number(static_cast<int>(tr->getStimulusKind())) + QLatin1Char(':') + stim;
                        if (unconditionalStimuli.contains(key))
                            add(tid, eDocElementKind::Transition, eSeverity::Warning, 3, vtr("Transition on '%1' is shadowed by an earlier unconditional transition").arg(stim));
                        if (unconditional)
                            unconditionalStimuli.insert(key);
                    }
                }
            }
        }

        // Declared types referenced by declarations count as used
        for (SMMethodEntry* m : mData.getMethods().getElements())
        {
            if (m == nullptr)
                continue;
            if (m->isCondition()) noteType(m->getReturn());
            for (const MethodParameter& p : m->getElements()) noteType(p.getType());
        }
        for (SMEventEntry* e : mData.getEvents().getElements())
            if (e != nullptr) for (const MethodParameter& p : e->getElements()) noteType(p.getType());
        for (const SMAttributeEntry& a : mData.getAttributes().getElements()) noteType(a.getType());
        for (const ConstantEntry& c : mData.getConstants().getElements()) noteType(c.getType());

        // Severity is decided per element kind, not per rule. An unused method, event, timer, data
        // type or import is behaviour or a contract that was written and then wired to nothing, and
        // that is worth a warning. An unused attribute or constant is just data the machine does not
        // happen to read yet: legitimate at any point in a design, so it is reported as information
        // and never as a warning.
        for (SMMethodEntry* m : mData.getMethods().getElements())
        {
            if (m == nullptr)
                continue;
            bool used = false;
            if (m->isAction())          used = actionsUsed.contains(m->getName());
            else if (m->isCondition())  used = conditionsUsed.contains(m->getName());
            else if (m->isTrigger())    used = triggersUsed.contains(m->getName());
            if (used == false)
                add(m->getId(), eDocElementKind::Method, eSeverity::Warning, 4, vtr("Method '%1' is never referenced").arg(m->getName()));
        }
        for (SMEventEntry* e : mData.getEvents().getElements())
        {
            if (e == nullptr)
                continue;
            if ((eventsSent.contains(e->getName()) == false) && (eventsReacted.contains(e->getName()) == false))
                add(e->getId(), eDocElementKind::Event, eSeverity::Warning, 4, vtr("Event '%1' is never referenced").arg(e->getName()));
        }
        for (const SMTimerEntry& t : mData.getTimers().getElements())
        {
            if ((timersStarted.contains(t.getName()) == false) && (timersStopped.contains(t.getName()) == false) && (timersReacted.contains(t.getName()) == false))
                add(t.getId(), eDocElementKind::Timer, eSeverity::Warning, 4, vtr("Timer '%1' is never referenced").arg(t.getName()));
        }
        for (const SMAttributeEntry& a : mData.getAttributes().getElements())
        {
            if ((attrsRead.contains(a.getName()) == false) && (attrsWritten.contains(a.getName()) == false))
                add(a.getId(), eDocElementKind::Attribute, eSeverity::Info, 4, vtr("Attribute '%1' is never referenced").arg(a.getName()));
        }
        for (const ConstantEntry& c : mData.getConstants().getElements())
        {
            if (constsUsed.contains(c.getName()) == false)
                add(c.getId(), eDocElementKind::Constant, eSeverity::Info, 4, vtr("Constant '%1' is never referenced").arg(c.getName()));
        }
        for (const IncludeEntry* i : mData.machineImports())
        {
            // An import is always another `.fsml` machine brought in to serve as a submachine
            if ((i->getAlias().isEmpty() == false) && (importsUsed.contains(i->getAlias()) == false))
                add(i->getId(), eDocElementKind::Import, eSeverity::Warning, 4, vtr("Import '%1' is never used as a submachine").arg(i->getAlias()));
        }
        for (DataTypeCustom* d : mData.getDataTypes().getCustomDataTypes())
        {
            if ((d != nullptr) && (typesUsed.contains(d->getName()) == false))
                add(d->getId(), eDocElementKind::DataType, eSeverity::Warning, 4, vtr("Data type '%1' is never referenced").arg(d->getName()));
        }

        // Warning 5: an event reacted to but never sent, or sent but never reacted to.
        for (const Use& u : reactedEvents)
            if (eventsSent.contains(u.name) == false)
                add(u.id, u.kind, eSeverity::Warning, 5, vtr("Event '%1' is reacted to but never sent").arg(u.name));
        for (const Use& u : sentEvents)
            if (eventsReacted.contains(u.name) == false)
                add(u.id, u.kind, eSeverity::Warning, 5, vtr("Event '%1' is sent but no transition reacts to it").arg(u.name));

        // Warning 6: a timer reacted to but never started, or started but never reacted to.
        for (const Use& u : reactedTimers)
            if (timersStarted.contains(u.name) == false)
                add(u.id, u.kind, eSeverity::Warning, 6, vtr("Timer '%1' is reacted to but never started").arg(u.name));
        for (const Use& u : startedTimers)
            if (timersReacted.contains(u.name) == false)
                add(u.id, u.kind, eSeverity::Warning, 6, vtr("Timer '%1' is started but no transition reacts to it").arg(u.name));
    }

} // namespace

QList<SMIssue> SMValidator::validate(const StateMachineData& data)
{
    Ctx ctx(data);
    return ctx.run();
}

eIssueField SMValidator::fieldOfRule(int rule)
{
    // Errors keep the plain number; everything advisory is stored shifted, so the two classes
    // have to be told apart before the number means anything.
    const bool advisory = (rule > SMValidator::WARNING_RULE_BASE);
    const int plain = advisory ? (rule - SMValidator::WARNING_RULE_BASE) : rule;

    if (advisory)
    {
        return (plain == SMValidator::RULE_MISSING_DESCRIPTION) ? eIssueField::Description : eIssueField::None;
    }

    switch (plain)
    {
    case SMValidator::RULE_DUPLICATE_NAME:
    case SMValidator::RULE_INVALID_IDENTIFIER:
        return eIssueField::Name;

    default:
        return eIssueField::None;
    }
}
