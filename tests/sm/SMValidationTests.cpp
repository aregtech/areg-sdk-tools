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
 *  \file        tests/sm/SMValidationTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Unit tests for the FSM structural and reference validation engine. Each
 *               check has a positive (violating) and a negative (clean) case. The engine is
 *               headless -- no widgets -- so the code generator can reuse it. Self-contained
 *               (no external test framework).
 *
 ************************************************************************/

#include "lusan/model/sm/SMValidator.hpp"
#include "lusan/model/common/DocRuleChecks.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/model/sm/SMTypeCompat.hpp"
#include "lusan/model/sm/SMOperationValidation.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMMethodKind.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/common/AttributeDataSection.hpp"
#include "lusan/data/common/ConstantDataSection.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/sm/SMImportResolver.hpp"
#include "lusan/data/sm/SMDocumentCache.hpp"
#include "lusan/data/common/DataTypeDataSection.hpp"

#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/FieldEntry.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <algorithm>
#include <cstdio>

//////////////////////////////////////////////////////////////////////////
// Minimal assertion harness
//////////////////////////////////////////////////////////////////////////

namespace
{
    int gChecks = 0;
    int gFailures = 0;

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }
}

#define CHECK(cond)  check((cond), #cond)

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////

namespace
{
    using eKind   = SMStateEntry::eStateKind;
    using eStim   = SMTransitionEntry::eStimulusKind;
    using eSource = SMArgumentEntry::eValueSource;
    using eOp     = SMConditionEntry::eOperator;
    using eTrans  = SMTransitionEntry::eTransitionKind;

    int countRule(const QList<SMIssue>& issues, int rule)
    {
        int n = 0;
        for (const SMIssue& i : issues)
            if (i.rule == rule) ++n;
        return n;
    }

    bool hasRule(const QList<SMIssue>& issues, int rule)
    {
        return countRule(issues, rule) > 0;
    }

    //!< For the cases that assert a document is accepted: any error at all is the failure, whichever
    //!< rule raised it, so a rule added later cannot start refusing a document these cases call legal.
    int countSeverity(const QList<SMIssue>& issues, SMIssue::eSeverity severity)
    {
        int n = 0;
        for (const SMIssue& i : issues)
            if (i.severity == severity) ++n;
        return n;
    }

    //!< A 10.2 warning is stored with the offset rule id; these target it by its 10.2 number.
    int countWarn(const QList<SMIssue>& issues, int warnNumber)
    {
        return countRule(issues, SMValidator::WARNING_RULE_BASE + warnNumber);
    }

    bool hasWarn(const QList<SMIssue>& issues, int warnNumber)
    {
        return countWarn(issues, warnNumber) > 0;
    }

    //!< True when EVERY finding of 10.2 rule \p warnNumber carries \p severity (vacuously true when
    //!< the rule produced none). Rule 4 is advisory for data and a warning for behaviour, so the
    //!< severity is part of the contract and not only the rule number.
    bool warnSeverityIs(const QList<SMIssue>& issues, int warnNumber, SMIssue::eSeverity severity)
    {
        for (const SMIssue& i : issues)
            if ((i.rule == (SMValidator::WARNING_RULE_BASE + warnNumber)) && (i.severity != severity))
                return false;
        return true;
    }

    //!< A minimal single-level machine with one Start state, valid on its own.
    SMStateEntry* addStart(StateMachineData& doc, const QString& name = "Idle")
    {
        return doc.getStates().createState(name, eKind::Start);
    }

    //!< A Start plus one ordinary state, wired by the level's initial transition, returning the
    //!< ORDINARY state. Anything that reacts to a stimulus belongs there: a Start is a pseudo-state
    //!< and its own transitions are the initial ones, which name no stimulus at all (rule 27), so a
    //!< check about stimulus resolution or parameter scope cannot be hung off one.
    SMStateEntry* addWorkingState(StateMachineData& doc, const QString& name = "Work")
    {
        SMStateEntry* start = doc.getStates().getStartState();
        if (start == nullptr)
        {
            start = doc.getStates().createState(QStringLiteral("Idle"), eKind::Start);
        }

        SMStateEntry* work = doc.getStates().createState(name, eKind::Normal);
        if ((start != nullptr) && (work != nullptr))
        {
            start->getTransitions().createTransition(eStim::Trigger, QString(), work->getId(), eTrans::Initial);
        }

        return work;
    }

    //!< Registers a machine import: an include entry whose location is a `.fsml`, plus the alias
    //!< a hosting state names.
    IncludeEntry* addImport(StateMachineData& doc, const QString& alias, const QString& location)
    {
        IncludeEntry* entry = doc.getIncludes().createInclude(location);
        if (entry != nullptr)
        {
            entry->setAlias(alias);
        }

        return entry;
    }

    //!< The document-wide ID of a state by name, for use as a transition target. An absent name
    //!< yields a deliberately-dangling ID so "unresolved target" cases still exercise rules 6/7.
    uint32_t stateId(const StateMachineData& doc, const QString& name)
    {
        const SMStateEntry* s = doc.findState(name);
        return (s != nullptr ? s->getId() : 0xFFFFFF00u);
    }
}

//////////////////////////////////////////////////////////////////////////
// Start-state presence and uniqueness per machine level
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testStartState()
    {
        std::printf("- start-state constraints\n");
        {   // Positive: no Start on the root level.
            StateMachineData doc;
            doc.getStates().createState("A", eKind::Normal);
            CHECK(hasRule(SMValidator::validate(doc), 1));
        }
        {   // Positive: two Start states on one level.
            StateMachineData doc;
            addStart(doc, "S1");
            doc.getStates().createState("S2", eKind::Start);
            CHECK(hasRule(SMValidator::validate(doc), 1));
        }
        {   // Positive: a painted nested level with no Start.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Normal);
            CHECK(hasRule(SMValidator::validate(doc), 11));
        }
        {   // Negative: exactly one Start on every level.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 1) == 0);
            CHECK(countRule(issues, 11) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Duplicate element identifiers and duplicate state names
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testDuplicates()
    {
        std::printf("- duplicate id / state name\n");
        {   // Positive: forced duplicate ID.
            StateMachineData doc;
            SMStateEntry* a = addStart(doc);
            SMStateEntry* b = doc.getStates().createState("B", eKind::Normal);
            b->setId(a->getId());
            CHECK(hasRule(SMValidator::validate(doc), 2));
        }
        {   // Negative: monotonic IDs are unique.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("B", eKind::Normal);
            CHECK(countRule(SMValidator::validate(doc), 2) == 0);
        }
        {   // Positive: same state name on two levels.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            SMStateData* nested = comp->getOrCreateNestedStates();
            nested->createState("Idle", eKind::Start);          // clashes with the root Start.
            CHECK(hasRule(SMValidator::validate(doc), 3));
        }
        {   // Negative: distinct state names.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("Work", eKind::Normal);
            CHECK(countRule(SMValidator::validate(doc), 3) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Name collisions within a registry, the stimulus space, and a parameter list
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testNameCollisions()
    {
        std::printf("- name collisions\n");
        {   // Positive: a trigger and an event share a name (stimulus space).
            StateMachineData doc;
            addStart(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            doc.getEvents().createEvent("go");
            CHECK(hasRule(SMValidator::validate(doc), 4));
        }
        {   // Positive: duplicate parameter name within one ParamList.
            StateMachineData doc;
            addStart(doc);
            MethodEntry* m = doc.getMethods().createMethod("act", NEMethod::SmAction);
            m->addParam("p");
            m->addParam("p");
            CHECK(hasRule(SMValidator::validate(doc), 4));
        }
        {   // Negative: disjoint names.
            StateMachineData doc;
            addStart(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            doc.getEvents().createEvent("done");
            doc.getTimers().createTimer("tick");
            CHECK(countRule(SMValidator::validate(doc), 4) == 0);
        }
    }

    void testDeclarationCollisions()
    {
        std::printf("- declaration collisions (trigger/attribute, reserved member prefix, self-named param, invalid source)\n");
        {   // A trigger and an attribute of the same name both become members of the machine class.
            StateMachineData doc;
            addStart(doc);
            doc.getMethods().createMethod("Ready", NEMethod::SmTrigger);
            doc.getAttributes().createAttribute("Ready")->setType("bool");
            CHECK(hasRule(SMValidator::validate(doc), 32));
        }
        {   // Different names: no collision.
            StateMachineData doc;
            addStart(doc);
            doc.getMethods().createMethod("Ready", NEMethod::SmTrigger);
            doc.getAttributes().createAttribute("IsReady")->setType("bool");
            CHECK(countRule(SMValidator::validate(doc), 32) == 0);
        }
        {   // A condition and an attribute of the same name land on different classes: no collision.
            StateMachineData doc;
            addStart(doc);
            doc.getMethods().createMethod("Ready", NEMethod::SmCondition);
            doc.getAttributes().createAttribute("Ready")->setType("bool");
            CHECK(countRule(SMValidator::validate(doc), 32) == 0);
        }
        {   // A HOSTING state named 'Attr...' becomes a member that reads as a generated
            // attribute member. Only a hosting state generates one at all.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("AttrReady", eKind::Normal)->setSubmachine("Lib");
            CHECK(hasRule(SMValidator::validate(doc), 33));
        }
        {   // The same for 'Cond...', which belongs to an embedded condition member.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("CondReady", eKind::Normal)->setSubmachine("Lib");
            CHECK(hasRule(SMValidator::validate(doc), 33));
        }
        {   // A hosting substate is a member too, so the check reaches every level.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* parent = doc.getStates().createState("Work", eKind::Normal);
            parent->getOrCreateNestedStates()->createState("AttrInner", eKind::Normal)->setSubmachine("Lib");
            CHECK(hasRule(SMValidator::validate(doc), 33));
        }
        {   // A PLAIN state carrying a reserved prefix is legal, and used to be refused. It
            // hosts nothing, so it generates no member, and the build accepts it.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("AttrReady", eKind::Normal);
            doc.getStates().createState("ParamCount", eKind::Normal);
            CHECK(countRule(SMValidator::validate(doc), 33) == 0);
        }
        {   // 'Param...' is reserved as well: the two document types share one set of member
            // prefixes, so a hosting state spelled that way is refused by both tools.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("ParamCount", eKind::Normal)->setSubmachine("Lib");
            CHECK(hasRule(SMValidator::validate(doc), 33));
        }
        {   // Two fixed members the machine class always declares, added 2026-08-09.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("CurrentStates", eKind::Normal)->setSubmachine("Lib");
            CHECK(hasRule(SMValidator::validate(doc), 33));

            StateMachineData doc2;
            addStart(doc2);
            doc2.getStates().createState("HistoryRecord", eKind::Normal)->setSubmachine("Lib");
            CHECK(hasRule(SMValidator::validate(doc2), 33));
        }
        {   // A state name that owns no reserved prefix is silent.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("Ready", eKind::Normal);
            CHECK(countRule(SMValidator::validate(doc), 33) == 0);
        }
        {   // An attribute or a condition keeps its own prefix, so those names are legal: an
            // attribute 'AttrReady' becomes 'mAttrAttrReady', a condition 'CondReady' 'mCondCondReady'.
            StateMachineData doc;
            addStart(doc);
            doc.getAttributes().createAttribute("AttrReady")->setType("bool");
            MethodEntry* c = doc.getMethods().createMethod("CondReady", NEMethod::SmCondition);
            c->setImplement(MethodEntry::eImplement::Embedded);
            c->setBody("return true;");
            c->setReturn("bool");
            CHECK(countRule(SMValidator::validate(doc), 33) == 0);
        }
        {   // An attribute and an embedded condition of the same name land on two different members,
            // so the pair is legal and files nothing.
            StateMachineData doc;
            addStart(doc);
            doc.getAttributes().createAttribute("Ready")->setType("bool");
            MethodEntry* c = doc.getMethods().createMethod("Ready", NEMethod::SmCondition);
            c->setImplement(MethodEntry::eImplement::Embedded);
            c->setBody("return true;");
            c->setReturn("bool");
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK((countRule(issues, 32) == 0) && (countRule(issues, 33) == 0) && (countRule(issues, 4) == 0));
        }
        {   // OW-62: a declared timer named 'Consumer' becomes 'mTimerConsumer', which is already
            // the machine's own timer consumer member. The generator built this with zero errors.
            StateMachineData doc;
            addStart(doc);
            doc.getTimers().createTimer("Consumer")->setTimeout(1000);
            CHECK(hasRule(SMValidator::validate(doc), 33));
        }
        {   // A declared timer whose name does not land on a fixed member is silent.
            StateMachineData doc;
            addStart(doc);
            doc.getTimers().createTimer("Red")->setTimeout(1000);
            CHECK(countRule(SMValidator::validate(doc), 33) == 0);
        }
        {   // OW-62: a state hosting an import named 'State' or 'ActionHandler' becomes a member
            // that is already the machine's own bookkeeping, not merely another kind's prefix.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("State", eKind::Normal)->setSubmachine("Lib");
            CHECK(hasRule(SMValidator::validate(doc), 33));
        }
        {
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("ActionHandler", eKind::Normal)->setSubmachine("Lib");
            CHECK(hasRule(SMValidator::validate(doc), 33));
        }
        {   // The same name on a state that does NOT host an import is legal: a plain state
            // generates no member at all, so there is nothing to collide with.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("State", eKind::Normal);
            CHECK(countRule(SMValidator::validate(doc), 33) == 0);
        }
        {   // A hosting state whose name does not land on a fixed member is silent.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("Ready", eKind::Normal)->setSubmachine("Lib");
            CHECK(countRule(SMValidator::validate(doc), 33) == 0);
        }
        {   // 'OwnLock' and 'Epoch' only exist on a Shared machine, so a hosting state named
            // 'Epoch' is legal on a Local one and refused the moment the machine turns Shared.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("Epoch", eKind::Normal)->setSubmachine("Lib");
            doc.getOverview().setThreading(SMOverviewData::eThreading::Local);
            CHECK(countRule(SMValidator::validate(doc), 33) == 0);
            doc.getOverview().setThreading(SMOverviewData::eThreading::Shared);
            CHECK(hasRule(SMValidator::validate(doc), 33));
        }
        {   // A trigger whose parameter carries the method's own name hides the method in the body.
            StateMachineData doc;
            addStart(doc);
            MethodEntry* m = doc.getMethods().createMethod("Ready", NEMethod::SmTrigger);
            m->addParam("Ready");
            CHECK(hasWarn(SMValidator::validate(doc), 34));
        }
        {   // The same shape on an action is generated with a name prefix, so it is silent.
            StateMachineData doc;
            addStart(doc);
            MethodEntry* m = doc.getMethods().createMethod("Ready", NEMethod::SmAction);
            m->addParam("Ready");
            CHECK(countWarn(SMValidator::validate(doc), 34) == 0);
        }
        {   // An argument whose source no longer resolves (a condition source migrated on load) is
            // an error until it is re-mapped.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            MethodEntry* act = doc.getMethods().createMethod("act", NEMethod::SmAction);
            act->addParam("x")->setType("bool");
            SMActionCall* call = new SMActionCall(0, "act");
            call->addArgument("x", eSource::Invalid, "IsReady");
            s->getEntryList().addOperation(call);
            CHECK(hasRule(SMValidator::validate(doc), 6));
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Identifier syntax of element names
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testIdentifiers()
    {
        std::printf("- identifier syntax\n");
        {   // Positive: a state name with a space.
            StateMachineData doc;
            addStart(doc, "Bad Name");
            CHECK(hasRule(SMValidator::validate(doc), 5));
        }
        {   // Negative: a valid identifier.
            StateMachineData doc;
            addStart(doc, "Good_Name1");
            CHECK(countRule(SMValidator::validate(doc), 5) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Cross-reference resolution and the sibling-target constraint
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testReferences()
    {
        std::printf("- references and sibling target\n");
        {   // Positive: an unresolved trigger stimulus.
            StateMachineData doc;
            SMStateEntry* s = addWorkingState(doc);
            s->getTransitions().createTransition(eStim::Trigger, "ghost", stateId(doc, "Idle"));
            CHECK(hasRule(SMValidator::validate(doc), 6));
        }
        {   // Positive: an unresolved transition target.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            s->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Nowhere"));
            CHECK(hasRule(SMValidator::validate(doc), 6));
        }
        {   // Positive: a target that exists but is not a sibling (it is nested).
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            start->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Inner"));
            CHECK(hasRule(SMValidator::validate(doc), 7));
        }
        {   // Positive: an operation action that does not resolve.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            s->getEntryList().addOperation(new SMActionCall(0, "ghostAction"));
            CHECK(hasRule(SMValidator::validate(doc), 6));
        }
        {   // Negative: every reference resolves to a sibling / declared name.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            doc.getStates().createState("Work", eKind::Normal);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            start->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Work"));
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 6) == 0);
            CHECK(countRule(issues, 7) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Final and Start state structure
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testFinalStart()
    {
        std::printf("- final/start structure\n");
        {   // Positive: a Final state with an outgoing transition.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* fin = doc.getStates().createState("Done", eKind::Final);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            fin->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Idle"));
            CHECK(hasRule(SMValidator::validate(doc), 8));
        }
        {   // Positive: a Start state that owns substates.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            start->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            CHECK(hasRule(SMValidator::validate(doc), 9));
        }
        {   // Negative: a Final leaf and a childless Start.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("Done", eKind::Final);
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 8) == 0);
            CHECK(countRule(issues, 9) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// L1 -- Kind="Start" is a pseudo-state (10.1 rule 27)
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< Gives \p transition a resolved guard -- a call of a newly declared condition method --
    //!< which is what "carries a condition" means for an initial transition. The shape does not
    //!< matter here, only that the guard is not empty and that it resolves.
    void guardIt(StateMachineData& doc, SMTransitionEntry& transition, const QString& call)
    {
        MethodEntry* cond = doc.getMethods().createMethod(call, NEMethod::SmCondition);
        transition.getGuard().setTree(SMGuardNode::makeCall(cond != nullptr ? cond->getId() : 0u, QList<SMGuardNode*>()));
    }

    //!< A composite state whose sublevel holds a Start and two ordinary states, returning the
    //!< sublevel's Start. The nested level is where a guarded initial transition is legal.
    SMStateEntry* nestedLevel(StateMachineData& doc, SMStateData*& outLevel)
    {
        SMStateEntry* comp = doc.getStates().createState(QStringLiteral("Comp"), eKind::Normal);
        outLevel = comp->getOrCreateNestedStates();
        SMStateEntry* start = outLevel->createState(QStringLiteral("InnerStart"), eKind::Start);
        outLevel->createState(QStringLiteral("Left"), eKind::Normal);
        outLevel->createState(QStringLiteral("Right"), eKind::Normal);
        return start;
    }

    void testPseudoStartRules()
    {
        std::printf("- L1: Kind=\"Start\" is a pseudo-state (rule 27)\n");
        const int rule = SMValidator::RULE_PSEUDO_START;

        {   // Rule 1: a Start with an entry action is refused, and the message names the state.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc, QStringLiteral("Begin"));
            SMStateEntry* work  = doc.getStates().createState("Work", eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, QString(), work->getId(), eTrans::Initial);
            doc.getMethods().createMethod("Warmup", NEMethod::SmAction);
            start->getEntryList().addOperation(new SMActionCall(0, "Warmup"));

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, rule) == 1);
            bool named = false;
            for (const SMIssue& i : issues)
                named = named || ((i.rule == rule) && i.message.contains(QStringLiteral("'Begin'")));
            CHECK(named);
        }
        {   // Rule 1, the other list: exit operations on a Start are equally refused.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* work  = doc.getStates().createState("Work", eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, QString(), work->getId(), eTrans::Initial);
            doc.getMethods().createMethod("Cooldown", NEMethod::SmAction);
            start->getExitList().addOperation(new SMActionCall(0, "Cooldown"));
            CHECK(countRule(SMValidator::validate(doc), rule) == 1);
        }
        {   // Rule 2: nothing may target a Start, and a Start may not target itself.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* work  = doc.getStates().createState("Work", eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, QString(), work->getId(), eTrans::Initial);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            work->getTransitions().createTransition(eStim::Trigger, "go", start->getId());
            CHECK(countRule(SMValidator::validate(doc), rule) == 1);
        }
        {   // Rule 2 / conflicts 6.3: the Start's own transition looping back into it.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            doc.getStates().createState("Work", eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, QString(), start->getId(), eTrans::Initial);
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(hasRule(issues, rule));
            bool saysNeverInitialises = false;
            for (const SMIssue& i : issues)
                saysNeverInitialises = saysNeverInitialises || ((i.rule == rule) && i.message.contains(QStringLiteral("never initialises")));
            CHECK(saysNeverInitialises);
        }
        {   // Rule 3: a Start with no outgoing transition -- the level never initialises.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("Work", eKind::Normal);
            CHECK(countRule(SMValidator::validate(doc), rule) == 1);
        }
        {   // Rule 3: a transition that names no target initialises nothing, so it is not an
            // outgoing one -- the level still never begins. The missing target itself is rule 28's.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            doc.getStates().createState("Work", eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, QString(), 0u, eTrans::Initial);
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, rule) == 1);
            CHECK(countRule(issues, SMValidator::RULE_TRANSITION_KIND) == 1);
        }
        {   // Rule 5: two initial transitions where one carries no condition.
            StateMachineData doc;
            addStart(doc);
            SMStateData* level = nullptr;
            SMStateEntry* start = nestedLevel(doc, level);
            SMTransitionEntry* first  = start->getTransitions().createTransition(eStim::Trigger, QString(), level->findState("Left")->getId(), eTrans::Initial);
            start->getTransitions().createTransition(eStim::Trigger, QString(), level->findState("Right")->getId(), eTrans::Initial);
            guardIt(doc, *first, QStringLiteral("IsLeft"));
            // Wire the root Start too, so the nested level is the only thing judged.
            doc.getStates().getStartState()->getTransitions().createTransition(eStim::Trigger, QString(), doc.findState("Comp")->getId(), eTrans::Initial);
            CHECK(countRule(SMValidator::validate(doc), rule) == 1);
        }
        {   // Rule 5, negative: two initial transitions, both guarded, is the legal shape. Rule 6
            // rides on it -- when neither condition holds the machine simply rests in the parent
            // with no child active, which is a legal configuration and NOT a finding.
            StateMachineData doc;
            addStart(doc);
            SMStateData* level = nullptr;
            SMStateEntry* start = nestedLevel(doc, level);
            SMTransitionEntry* first  = start->getTransitions().createTransition(eStim::Trigger, QString(), level->findState("Left")->getId(), eTrans::Initial);
            SMTransitionEntry* second = start->getTransitions().createTransition(eStim::Trigger, QString(), level->findState("Right")->getId(), eTrans::Initial);
            guardIt(doc, *first, QStringLiteral("IsLeft"));
            guardIt(doc, *second, QStringLiteral("IsRight"));
            // The root Start still has nowhere to go; wire it so only the nested level is judged.
            doc.getStates().getStartState()->getTransitions().createTransition(eStim::Trigger, QString(), doc.findState("Comp")->getId(), eTrans::Initial);
            CHECK(countRule(SMValidator::validate(doc), rule) == 0);
        }
        {   // Rule 7: at the ROOT there is no parent to remain in, so a condition is refused.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* work  = doc.getStates().createState("Work", eKind::Normal);
            SMTransitionEntry* only = start->getTransitions().createTransition(eStim::Trigger, QString(), work->getId(), eTrans::Initial);
            guardIt(doc, *only, QStringLiteral("IsReady"));
            CHECK(countRule(SMValidator::validate(doc), rule) == 1);
        }
        {   // Rule 7: and it must have exactly one -- two guarded ones are two faults at the root.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* left  = doc.getStates().createState("Left", eKind::Normal);
            SMStateEntry* right = doc.getStates().createState("Right", eKind::Normal);
            SMTransitionEntry* first  = start->getTransitions().createTransition(eStim::Trigger, QString(), left->getId(), eTrans::Initial);
            SMTransitionEntry* second = start->getTransitions().createTransition(eStim::Trigger, QString(), right->getId(), eTrans::Initial);
            guardIt(doc, *first, QStringLiteral("IsLeft"));
            guardIt(doc, *second, QStringLiteral("IsRight"));
            // One "not exactly one" on the state, one "no meaning" per conditional transition.
            CHECK(countRule(SMValidator::validate(doc), rule) == 3);
        }
        {   // Negative: the corrected form -- one unguarded initial transition -- is clean.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* work  = doc.getStates().createState("Work", eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, QString(), work->getId(), eTrans::Initial);
            CHECK(countRule(SMValidator::validate(doc), rule) == 0);
        }
    }

    //!< L2 rule 28: `Kind` says what a transition is, so `To` and `Stimulus` mean only what they
    //!< say. Each case is one document with one fault, and each names the element it is on.
    void testTransitionKindRules()
    {
        std::printf("- L2: the transition Kind contract (rule 28)\n");
        const int rule = SMValidator::RULE_TRANSITION_KIND;

        {   // An External transition with no target: the unfinished edge that used to be
            // byte-identical to a deliberate internal transition, and so meant something.
            StateMachineData doc;
            SMStateEntry* work = addWorkingState(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            work->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::External);

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, rule) == 1);
            bool named = false;
            for (const SMIssue& i : issues)
                named = named || ((i.rule == rule) && i.message.contains(QStringLiteral("'Work'")));
            CHECK(named);
            // ...and it is NOT reported as an empty internal transition: it is not internal.
            CHECK(countWarn(issues, 7) == 0);
        }
        {   // The same shape as an Internal transition is the negative case: no target is correct.
            StateMachineData doc;
            SMStateEntry* work = addWorkingState(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            doc.getMethods().createMethod("act", NEMethod::SmAction);
            SMTransitionEntry* tr = work->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::Internal);
            tr->getOperations().addOperation(new SMActionCall(0, "act"));
            CHECK(countRule(SMValidator::validate(doc), rule) == 0);
        }
        {   // An Internal transition that still names a target -- only a hand-edited document can
            //  say this, and it has to be told rather than silently repaired on load.
            StateMachineData doc;
            SMStateEntry* work = addWorkingState(doc);
            SMStateEntry* other = doc.getStates().createState("Other", eKind::Normal);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMTransitionEntry* tr = work->getTransitions().createTransition(eStim::Trigger, "go", other->getId());
            tr->setToId(other->getId());
            // setKind(Internal) drops the target on purpose, so the fault is built by hand.
            tr->setKind(eTrans::Internal);
            tr->setToId(other->getId());
            CHECK(countRule(SMValidator::validate(doc), rule) == 1);
        }
        {   // An Initial transition on a state that is not a Start.
            StateMachineData doc;
            SMStateEntry* work = addWorkingState(doc);
            SMStateEntry* other = doc.getStates().createState("Other", eKind::Normal);
            work->getTransitions().createTransition(eStim::Trigger, QString(), other->getId(), eTrans::Initial);

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, rule) == 1);
            bool named = false;
            for (const SMIssue& i : issues)
                named = named || ((i.rule == rule) && i.message.contains(QStringLiteral("'Work'")));
            CHECK(named);
        }
        {   // A Start carrying an External transition, and one carrying an Internal transition:
            // everything a Start owns is the level's initial transition.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc, QStringLiteral("Begin"));
            SMStateEntry* work  = doc.getStates().createState("Work", eKind::Normal);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            start->getTransitions().createTransition(eStim::Trigger, "go", work->getId(), eTrans::External);

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, rule) == 1);
            bool named = false;
            for (const SMIssue& i : issues)
                named = named || ((i.rule == rule) && i.message.contains(QStringLiteral("'Begin'")));
            CHECK(named);
            // The level does initialise -- the transition has a target -- so rule 27 stays quiet
            // and one mistake is one finding.
            CHECK(countRule(issues, SMValidator::RULE_PSEUDO_START) == 0);
        }
        {
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* work  = doc.getStates().createState("Work", eKind::Normal);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            start->getTransitions().createTransition(eStim::Trigger, QString(), work->getId(), eTrans::Initial);
            start->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::Internal);
            CHECK(countRule(SMValidator::validate(doc), rule) == 1);
        }
        {   // An Initial transition naming a stimulus: nothing fires it, so there is none to name.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc, QStringLiteral("Begin"));
            SMStateEntry* work  = doc.getStates().createState("Work", eKind::Normal);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMTransitionEntry* tr = start->getTransitions().createTransition(eStim::Trigger, QString(), work->getId(), eTrans::Initial);
            tr->setStimulus(QStringLiteral("go"));      // setKind(Initial) clears it; forced back

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, rule) == 1);
            // ...and never a second time as an unresolved reference.
            CHECK(countRule(issues, 6) == 0);
        }
        {   // Negative: the three kinds in their correct shapes, in one document, are clean.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc, QStringLiteral("Begin"));
            SMStateEntry* work  = doc.getStates().createState("Work", eKind::Normal);
            SMStateEntry* done  = doc.getStates().createState("Done", eKind::Normal);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            doc.getMethods().createMethod("poke", NEMethod::SmTrigger);
            doc.getMethods().createMethod("act", NEMethod::SmAction);
            start->getTransitions().createTransition(eStim::Trigger, QString(), work->getId(), eTrans::Initial);
            work->getTransitions().createTransition(eStim::Trigger, "go", done->getId(), eTrans::External);
            SMTransitionEntry* internalTx = work->getTransitions().createTransition(eStim::Trigger, "poke", 0u, eTrans::Internal);
            internalTx->getOperations().addOperation(new SMActionCall(0, "act"));
            CHECK(countRule(SMValidator::validate(doc), rule) == 0);
        }
    }

    //!< The reachability rules of L8. Rules 3 and 4 (a Start that targets itself, a Start
    //!< nothing leaves) are rule 27's and were implemented with the pseudo-state; they are
    //!< re-checked here under their L8 numbering so a change to either is caught by both names.
    void testReachabilityRules()
    {
        std::printf("- L8: reachability (override, W1, 27)\n");

        //!< A composite `Session` holding `Step1`, both reacting to the trigger `poke` -- the
        //!< plain hierarchical override, and a legal document. The composite's transition is
        //!< returned so a case can guard it.
        auto shadowDoc = [](StateMachineData& doc) -> SMTransitionEntry*
        {
            SMStateEntry* start   = addStart(doc);
            SMStateEntry* session = doc.getStates().createState(QStringLiteral("Session"), eKind::Normal);
            SMStateEntry* other   = doc.getStates().createState(QStringLiteral("Other"), eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, QString(), session->getId(), eTrans::Initial);
            doc.getMethods().createMethod(QStringLiteral("poke"), NEMethod::SmTrigger);

            SMStateData* inner = session->getOrCreateNestedStates();
            SMStateEntry* innerStart = inner->createState(QStringLiteral("InnerStart"), eKind::Start);
            SMStateEntry* step1 = inner->createState(QStringLiteral("Step1"), eKind::Normal);
            SMStateEntry* step2 = inner->createState(QStringLiteral("Step2"), eKind::Normal);
            innerStart->getTransitions().createTransition(eStim::Trigger, QString(), step1->getId(), eTrans::Initial);
            step1->getTransitions().createTransition(eStim::Trigger, QStringLiteral("poke"), step2->getId());
            return session->getTransitions().createTransition(eStim::Trigger, QStringLiteral("poke"), other->getId());
        };

        {   // The override is legal and nothing reports it. A stimulus is searched from the active
            // leaf upwards, so `Step1` answers `poke` while it is active and `Session` answers it
            // from everywhere else. Both are reachable, neither is dead, and no error is raised --
            // guarded or not, which the following two cases pin down.
            StateMachineData doc;
            shadowDoc(doc);
            CHECK(countSeverity(SMValidator::validate(doc), SMIssue::eSeverity::Error) == 0);
        }
        {   // The same document with the composite guarded: still clean.
            StateMachineData doc;
            SMTransitionEntry* ancestor = shadowDoc(doc);
            guardIt(doc, *ancestor, QStringLiteral("IsBusy"));
            CHECK(countSeverity(SMValidator::validate(doc), SMIssue::eSeverity::Error) == 0);
        }
        {   // Two SIBLINGS reacting to the same stimulus: never eligible at the same time, so there
            // is nothing to compare. The same-owner case is 10.2 warning 3's.
            StateMachineData doc;
            SMStateEntry* work  = addWorkingState(doc);
            SMStateEntry* other = doc.getStates().createState(QStringLiteral("Other"), eKind::Normal);
            doc.getMethods().createMethod(QStringLiteral("poke"), NEMethod::SmTrigger);
            work->getTransitions().createTransition(eStim::Trigger, QStringLiteral("poke"), other->getId());
            other->getTransitions().createTransition(eStim::Trigger, QStringLiteral("poke"), work->getId());
            CHECK(countSeverity(SMValidator::validate(doc), SMIssue::eSeverity::Error) == 0);
        }
        {   // Rule 2: an unreachable state is a WARNING and one finding, and the document still
            // saves -- a half-drawn machine is the normal intermediate state of an editor.
            StateMachineData doc;
            addWorkingState(doc);
            doc.getStates().createState(QStringLiteral("Orphan"), eKind::Normal);

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countWarn(issues, 1) == 1);
            CHECK(warnSeverityIs(issues, 1, SMIssue::eSeverity::Warning));
            bool named = false;
            for (const SMIssue& i : issues)
                named = named || ((i.rule == (SMValidator::WARNING_RULE_BASE + 1)) && i.message.contains(QStringLiteral("'Orphan'")));
            CHECK(named);
        }
        {   // Rule 2, negative: the state the level's Start descends into is reached, not orphaned.
            StateMachineData doc;
            addWorkingState(doc);
            CHECK(countWarn(SMValidator::validate(doc), 1) == 0);
        }
        {   // Rule 3: a Start whose transition targets the Start itself. Error, under rule 27.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            doc.getStates().createState(QStringLiteral("Work"), eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, QString(), start->getId(), eTrans::Initial);

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(hasRule(issues, SMValidator::RULE_PSEUDO_START));
            for (const SMIssue& i : issues)
                if (i.rule == SMValidator::RULE_PSEUDO_START) CHECK(i.severity == SMIssue::eSeverity::Error);
        }
        {   // Rule 4: a Start with no outgoing transition. Error, under rule 27.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState(QStringLiteral("Work"), eKind::Normal);

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, SMValidator::RULE_PSEUDO_START) == 1);
            for (const SMIssue& i : issues)
                if (i.rule == SMValidator::RULE_PSEUDO_START) CHECK(i.severity == SMIssue::eSeverity::Error);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Argument-to-parameter matching
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testArguments()
    {
        std::printf("- argument matching\n");
        {   // Positive: a required parameter left unmapped.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            MethodEntry* act = doc.getMethods().createMethod("act", NEMethod::SmAction);
            act->addParam("p");
            s->getEntryList().addOperation(new SMActionCall(0, "act"));
            CHECK(hasRule(SMValidator::validate(doc), 10));
        }
        {   // Positive: an argument that names no declared parameter.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", NEMethod::SmAction);
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("stranger", eSource::Value, "1");
            CHECK(hasRule(SMValidator::validate(doc), 10));
        }
        {   // Negative: every declared parameter is mapped.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            MethodEntry* act = doc.getMethods().createMethod("act", NEMethod::SmAction);
            act->addParam("p");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Value, "1");
            CHECK(countRule(SMValidator::validate(doc), 10) == 0);
        }
        {   // An empty Stimulus is the INITIAL transition and is legal on a Start state only.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            doc.getStates().createState("Next", eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, QString(), stateId(doc, "Next"), eTrans::Initial);
            CHECK(countRule(SMValidator::validate(doc), 6) == 0);
        }
        {   // The same empty Stimulus on an ordinary state is a transition nobody finished writing.
            // Rule 28 says that in those words; it used to arrive as "Trigger '' is not declared".
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* mid = doc.getStates().createState("Mid", eKind::Normal);
            doc.getStates().createState("End", eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, QString(), stateId(doc, "Mid"), eTrans::Initial);
            mid->getTransitions().createTransition(eStim::Trigger, QString(), stateId(doc, "End"));
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(hasRule(issues, SMValidator::RULE_TRANSITION_KIND));
            CHECK(countRule(issues, 6) == 0);
        }
        {   // Positive: OnFinal names an event with a parameter that has no default. The hook is a
            // bare attribute with no ArgumentList anywhere, so nothing can ever supply that value.
            StateMachineData doc;
            addStart(doc);
            SMEventEntry* done = doc.getEvents().createEvent("Done");
            done->addParam("code");
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            comp->setOnFinal("Done");
            CHECK(hasRule(SMValidator::validate(doc), 10));
        }
        {   // Negative: the same hook, once every parameter carries its own default.
            StateMachineData doc;
            addStart(doc);
            SMEventEntry* done = doc.getEvents().createEvent("Done");
            // setDefaultValue only edits a parameter that is ALREADY defaulted, so the flag has
            // to be raised first -- otherwise the call silently does nothing.
            MethodParameter* code = done->addParam("code");
            code->setDefault(true);
            code->setValue(QStringLiteral("0"));
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            comp->setOnFinal("Done");
            CHECK(countRule(SMValidator::validate(doc), 10) == 0);
        }
        {   // Rule 10 on an ActionCall: a declared parameter with no default that no argument binds.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            MethodEntry* act = doc.getMethods().createMethod("DoWork", NEMethod::SmAction);
            act->addElement(MethodParameter(doc.getNextId(), "count", "int32", QString(), false, act), true);
            start->getEntryList().addOperation(new SMActionCall(0, "DoWork"));
            CHECK(countRule(SMValidator::validate(doc), 10) == 1);
        }
        {   // Rule 10 on an EventSend, the same fault on the other call site.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMEventEntry* ev = doc.getEvents().createEvent("Go");
            ev->addParam("code");
            start->getEntryList().addOperation(new SMEventSend(0, "Go"));
            CHECK(hasRule(SMValidator::validate(doc), 10));
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Rules allocated 2026-08-09: 38 (a default out of trailing position) and
    // 139 (a condition already named like a generated function).
    //////////////////////////////////////////////////////////////////////////
    void testDefaultOrderAndCallableNames()
    {
        std::printf("- a default out of trailing position, and a condition named like a generated function\n");

        {   // Error 38: `run(int32 a = 0, int32 b)` -- C++ takes defaults from the end of the
            // list. addParam() copies the previous parameter's flag, so the violation has to be
            // built with the constructor, exactly as a hand-edited document produces it.
            StateMachineData doc;
            addStart(doc);
            MethodEntry* act = doc.getMethods().createMethod("run", NEMethod::SmAction);
            act->addElement(MethodParameter(doc.getNextId(), "a", "int32", "0", true, act), true);
            act->addElement(MethodParameter(doc.getNextId(), "b", "int32", QString(), false, act), true);
            CHECK(hasRule(SMValidator::validate(doc), 38));
        }
        {   // Negative: the defaulted parameter is last, which is the only legal shape.
            StateMachineData doc;
            addStart(doc);
            MethodEntry* act = doc.getMethods().createMethod("run", NEMethod::SmAction);
            act->addElement(MethodParameter(doc.getNextId(), "a", "int32", QString(), false, act), true);
            act->addElement(MethodParameter(doc.getNextId(), "b", "int32", "0", true, act), true);
            CHECK(countRule(SMValidator::validate(doc), 38) == 0);
        }
        {   // The same rule on an event, because it lives on the shared parameter list.
            StateMachineData doc;
            addStart(doc);
            SMEventEntry* ev = doc.getEvents().createEvent("Go");
            ev->addElement(MethodParameter(doc.getNextId(), "a", "int32", "0", true, ev), true);
            ev->addElement(MethodParameter(doc.getNextId(), "b", "int32", QString(), false, ev), true);
            CHECK(hasRule(SMValidator::validate(doc), 38));
        }
        {   // Warning 139: an Action 'CheckLimit' generates 'action_CheckLimit'; a Condition
            // spelled that way generates the identical name.
            StateMachineData doc;
            addStart(doc);
            MethodEntry* cond = doc.getMethods().createMethod("action_CheckLimit", NEMethod::SmCondition);
            cond->setReturn("bool");
            CHECK(countWarn(SMValidator::validate(doc), 39) == 1);
        }
        {   // Negative: an ordinary condition name, and an ACTION spelled the same way (an
            // action is generated under the prefix, so it never collides with itself).
            StateMachineData doc;
            addStart(doc);
            MethodEntry* cond = doc.getMethods().createMethod("CheckLimit", NEMethod::SmCondition);
            cond->setReturn("bool");
            doc.getMethods().createMethod("action_Other", NEMethod::SmAction);
            CHECK(countWarn(SMValidator::validate(doc), 39) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Param source scope
//////////////////////////////////////////////////////////////////////////

namespace
{
    // Builds an ActionCall on the given list mapping act(p) := Param(paramValue).
    SMActionCall* mappedCall(StateMachineData& doc, SMOperationList& list, const QString& paramValue)
    {
        if (doc.getMethods().findMethod("act") == nullptr)
            doc.getMethods().createMethod("act", NEMethod::SmAction)->addParam("p");
        SMActionCall* call = new SMActionCall(0, "act");
        list.addOperation(call);
        call->addArgument("p", eSource::Param, paramValue);
        return call;
    }

    void testParamScope()
    {
        std::printf("- param scope\n");
        {   // Positive: Param used in an entry list (no stimulus scope).
            StateMachineData doc;
            SMStateEntry* s = addWorkingState(doc);
            mappedCall(doc, s->getEntryList(), "amount");
            CHECK(hasRule(SMValidator::validate(doc), 12));
        }
        {   // Positive: Param on a timer transition.
            StateMachineData doc;
            SMStateEntry* s = addWorkingState(doc);
            doc.getTimers().createTimer("tick");
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Timer, "tick", 0u, eTrans::Internal);
            mappedCall(doc, tr->getOperations(), "amount");
            CHECK(hasRule(SMValidator::validate(doc), 12));
        }
        {   // Positive: the trigger stimulus declares no such parameter.
            StateMachineData doc;
            SMStateEntry* s = addWorkingState(doc);
            doc.getMethods().createMethod("onTick", NEMethod::SmTrigger);  // no parameters.
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "onTick", 0u, eTrans::Internal);
            mappedCall(doc, tr->getOperations(), "amount");
            CHECK(hasRule(SMValidator::validate(doc), 12));
        }
        {   // Negative: Param names an actual stimulus parameter.
            StateMachineData doc;
            SMStateEntry* s = addWorkingState(doc);
            doc.getMethods().createMethod("onTick", NEMethod::SmTrigger)->addParam("amount");
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "onTick", 0u, eTrans::Internal);
            mappedCall(doc, tr->getOperations(), "amount");
            CHECK(countRule(SMValidator::validate(doc), 12) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Composite-state exclusivity and placement
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testComposite()
    {
        std::printf("- composite constraints\n");
        {   // Positive: History on a non-composite state.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* s = doc.getStates().createState("Work", eKind::Normal);
            s->setHistory(SMStateEntry::eHistory::Shallow);
            CHECK(hasRule(SMValidator::validate(doc), 18));
        }
        {   // Positive: a Submachine on a Final state.
            StateMachineData doc;
            addStart(doc);
            addImport(doc, "Lib", "./Lib.fsml");
            SMStateEntry* fin = doc.getStates().createState("Done", eKind::Final);
            fin->setSubmachine("Lib");
            CHECK(hasRule(SMValidator::validate(doc), 18));
        }
        {   // Negative: History on a painted composite.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            comp->setHistory(SMStateEntry::eHistory::Deep);
            CHECK(countRule(SMValidator::validate(doc), 18) == 0);
        }
        {   // Negative: an imported submachine is a composite too, so it may carry history.
            StateMachineData doc;
            addStart(doc);
            addImport(doc, "Lib", "./Lib.fsml");
            SMStateEntry* host = doc.getStates().createState("Host", eKind::Normal);
            host->setSubmachine("Lib");
            host->setHistory(SMStateEntry::eHistory::Shallow);
            CHECK(countRule(SMValidator::validate(doc), 18) == 0);
        }
        {   // Positive: the mode does not matter -- Deep on a leaf is the same rule-18 break.
            StateMachineData doc;
            addStart(doc);
            doc.getStates().createState("Leaf", eKind::Normal)->setHistory(SMStateEntry::eHistory::Deep);
            CHECK(hasRule(SMValidator::validate(doc), 18));
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Condition-method body and implementation mode
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testMethodBody()
    {
        std::printf("- condition body/implement\n");
        {   // Positive: an Embedded condition with an empty body.
            StateMachineData doc;
            addStart(doc);
            MethodEntry* c = doc.getMethods().createMethod("ready", NEMethod::SmCondition);
            c->setImplement(MethodEntry::eImplement::Embedded);
            c->setBody("   ");
            CHECK(hasRule(SMValidator::validate(doc), 20));
        }
        {   // Positive: a body on a non-embedded method.
            StateMachineData doc;
            addStart(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger)->setBody("do();");
            CHECK(hasRule(SMValidator::validate(doc), 20));
        }
        {   // Negative: an Embedded condition with a real body.
            StateMachineData doc;
            addStart(doc);
            MethodEntry* c = doc.getMethods().createMethod("ready", NEMethod::SmCondition);
            c->setImplement(MethodEntry::eImplement::Embedded);
            c->setBody("return true;");
            CHECK(countRule(SMValidator::validate(doc), 20) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// A parameterized condition call is a left operand only
//////////////////////////////////////////////////////////////////////////

namespace
{
    SMTransitionEntry* triggeredTransition(StateMachineData& doc, SMStateEntry* owner)
    {
        if (doc.getMethods().findMethod("go") == nullptr)
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
        return owner->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::Internal);
    }

    void testConditionLhsOnly()
    {
        std::printf("- parameterized condition LHS-only\n");
        {   // Positive: a parameterized condition on the RHS.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("isReady", NEMethod::SmCondition)->addParam("x");
            SMTransitionEntry* tr = triggeredTransition(doc, s);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Value);
            row->setLhs("1");
            row->setOperator(eOp::Equal);
            row->setRhsKind(eSource::Condition);
            row->setRhs("isReady");
            CHECK(hasRule(SMValidator::validate(doc), 21));
        }
        {   // Negative: the same parameterized condition on the LHS.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("isReady", NEMethod::SmCondition)->addParam("x");
            SMTransitionEntry* tr = triggeredTransition(doc, s);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Condition);
            row->setLhs("isReady");
            row->addArgument("x", eSource::Value, "1");     // map the parameter so the argument check stays quiet.
            CHECK(countRule(SMValidator::validate(doc), 21) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Expression-row shape and empty verbatim text
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testExpressionRows()
    {
        std::printf("- expression rows\n");
        {   // Positive: an expression row with no expression text.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMTransitionEntry* tr = triggeredTransition(doc, s);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Expression);
            row->setExpression("   ");
            CHECK(hasRule(SMValidator::validate(doc), 24));
        }
        {   // Positive: an expression row that also carries an operator and RHS.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMTransitionEntry* tr = triggeredTransition(doc, s);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Expression);
            row->setExpression("a > b");
            row->setOperator(eOp::Equal);
            row->setRhsKind(eSource::Value);
            row->setRhs("1");
            CHECK(hasRule(SMValidator::validate(doc), 23));
        }
        {   // Negative: a well-formed expression row.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMTransitionEntry* tr = triggeredTransition(doc, s);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Expression);
            row->setExpression("count > 0");
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 23) == 0);
            CHECK(countRule(issues, 24) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// A finding carries the offending element, a severity, and a message
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testFindingShape()
    {
        std::printf("- finding carries element id / severity / message\n");
        StateMachineData doc;
        SMStateEntry* s = addStart(doc);
        SMStateEntry* work = doc.getStates().createState("Work", eKind::Normal);
        work->setHistory(SMStateEntry::eHistory::Shallow);       // a history flag on a non-composite state.

        const QList<SMIssue> issues = SMValidator::validate(doc);
        bool navigable = false;
        for (const SMIssue& i : issues)
        {
            if ((i.rule == 18) && (i.elementId == work->getId()))
            {
                navigable = true;
                CHECK(i.severity == SMIssue::eSeverity::Error);
                CHECK(i.message.isEmpty() == false);
            }
        }
        CHECK(navigable);
        (void)s;
    }
}

//////////////////////////////////////////////////////////////////////////
// The 6.9 widening table, exhaustively over every ordered primitive pair
//////////////////////////////////////////////////////////////////////////

namespace
{
    // The oracle: does an implicit widening path exist from a to b (6.9 rules 1-4)?
    bool widensOracle(const QString& a, const QString& b)
    {
        auto uRank = [](const QString& t) -> int
        {
            if (t == "uint8") return 1; if (t == "uint16") return 2;
            if (t == "uint32") return 3; if (t == "uint64") return 4; return 0;
        };
        auto sRank = [](const QString& t) -> int
        {
            if (t == "int8") return 1; if (t == "int16") return 2;
            if (t == "int32") return 3; if (t == "int64") return 4; return 0;
        };
        auto fRank = [](const QString& t) -> int
        {
            if (t == "float") return 1; if (t == "double") return 2; return 0;
        };
        auto intUpTo32 = [](const QString& t) -> bool
        {
            return (t == "int8") || (t == "int16") || (t == "int32")
                || (t == "uint8") || (t == "uint16") || (t == "uint32");
        };

        if (a == b)
            return true;
        if ((uRank(a) != 0) && (uRank(b) != 0)) return uRank(a) <= uRank(b);
        if ((sRank(a) != 0) && (sRank(b) != 0)) return sRank(a) <= sRank(b);
        if ((fRank(a) != 0) && (fRank(b) != 0)) return fRank(a) <= fRank(b);
        if (intUpTo32(a) && (b == "double")) return true;
        return false;
    }

    // The other half of the oracle: bool and a number convert into each other without a cast, in
    // both directions, and both directions lose something. char is deliberately not in this.
    bool boolAndNumber(const QString& a, const QString& b)
    {
        const QStringList numbers = { "int8", "int16", "int32", "int64"
                                    , "uint8", "uint16", "uint32", "uint64", "float", "double" };
        return ((a == "bool") && numbers.contains(b)) || ((b == "bool") && numbers.contains(a));
    }

    void testWideningTable()
    {
        std::printf("- widening table (all primitive pairs)\n");
        const QStringList types = { "bool", "char", "int8", "int16", "int32", "int64",
                                    "uint8", "uint16", "uint32", "uint64", "float", "double", "String" };
        for (const QString& from : types)
        {
            for (const QString& to : types)
            {
                SMTypeCompat::eRank expected;
                if (from == to)
                    expected = SMTypeCompat::eRank::Exact;
                else if (widensOracle(from, to))
                    expected = SMTypeCompat::eRank::Converts;
                else if (widensOracle(to, from) || boolAndNumber(from, to))
                    expected = SMTypeCompat::eRank::Narrows;
                else
                    expected = SMTypeCompat::eRank::Mismatch;

                const SMTypeCompat::eRank actual = SMTypeCompat::rank(from, to);
                check(actual == expected, qPrintable(QString("rank(%1,%2)").arg(from, to)));
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Type compatibility, ordering/structure, literals, boolean tests, AttributeSet
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testTypeRules()
    {
        std::printf("- type / literal rules (13-17)\n");

        {   // Rule 13: mapping a String attribute onto a uint16 parameter (no widening).
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", NEMethod::SmAction)->addParam("p")->setType("uint16");
            doc.getAttributes().createAttribute("label")->setType("String");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Attribute, "label");
            CHECK(hasRule(SMValidator::validate(doc), 13));
        }
        {   // Negative rule 13: a uint8 attribute widens to a uint16 parameter.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", NEMethod::SmAction)->addParam("p")->setType("uint16");
            doc.getAttributes().createAttribute("small")->setType("uint8");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Attribute, "small");
            CHECK(countRule(SMValidator::validate(doc), 13) == 0);
        }
        {   // Rule 14: an ordering operator on a String operand.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("label")->setType("String");
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::Internal);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("label");
            row->setOperator(eOp::Greater);
            row->setRhsKind(eSource::Value); row->setRhs("x");
            CHECK(hasRule(SMValidator::validate(doc), 14));
        }
        {   // Rule 14: a structure operand in a condition row.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getDataTypes().addStructure("Rec");
            doc.getAttributes().createAttribute("r")->setType("Rec");
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::Internal);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("r");
            row->setOperator(eOp::Equal);
            row->setRhsKind(eSource::Attribute); row->setRhs("r");
            CHECK(hasRule(SMValidator::validate(doc), 14));
        }
        {   // Negative rule 14: numeric ordering is fine.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("n")->setType("uint16");
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::Internal);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("n");
            row->setOperator(eOp::Greater);
            row->setRhsKind(eSource::Value); row->setRhs("5");
            CHECK(countRule(SMValidator::validate(doc), 14) == 0);
        }
        {   // Rule 15: an out-of-range integer literal for a uint8 parameter.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", NEMethod::SmAction)->addParam("p")->setType("uint8");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Value, "999");
            CHECK(hasRule(SMValidator::validate(doc), 15));
        }
        {   // Rule 15: a literal that is not an enumerator of the declared enum.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            DataTypeEnum* color = static_cast<DataTypeEnum*>(doc.getDataTypes().addEnum("Color"));
            color->addField("Red"); color->addField("Green");
            doc.getMethods().createMethod("act", NEMethod::SmAction)->addParam("p")->setType("Color");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Value, "Blue");
            CHECK(hasRule(SMValidator::validate(doc), 15));
        }
        {   // Negative rule 15: an in-range literal and a valid enumerator.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            DataTypeEnum* color = static_cast<DataTypeEnum*>(doc.getDataTypes().addEnum("Color"));
            color->addField("Red");
            MethodEntry* act = doc.getMethods().createMethod("act", NEMethod::SmAction);
            act->addParam("p")->setType("uint8");
            act->addParam("c")->setType("Color");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Value, "200");
            call->addArgument("c", eSource::Value, "Red");
            CHECK(countRule(SMValidator::validate(doc), 15) == 0);
        }
        {   // Rule 16: a boolean-test row whose operand is not bool.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("cnt")->setType("uint16");
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::Internal);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("cnt");    // no operator: boolean test.
            CHECK(hasRule(SMValidator::validate(doc), 16));
        }
        {   // Negative rule 16: a bool operand is a valid boolean test.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("flag")->setType("bool");
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::Internal);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("flag");
            CHECK(countRule(SMValidator::validate(doc), 16) == 0);
        }
        {   // Rule 17: an AttributeSet whose source type is incompatible with the attribute.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("count")->setType("uint16");
            doc.getAttributes().createAttribute("label")->setType("String");
            SMAttributeSet* set = new SMAttributeSet(0, "count");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Attribute); set->setValue("label");
            CHECK(hasRule(SMValidator::validate(doc), 17));
        }
        {   // Negative rule 17: a widening source is fine.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("count")->setType("uint16");
            doc.getAttributes().createAttribute("small")->setType("uint8");
            SMAttributeSet* set = new SMAttributeSet(0, "count");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Attribute); set->setValue("small");
            CHECK(countRule(SMValidator::validate(doc), 17) == 0);
        }
    }

    //!< L7 Part 1: a narrowing is a warning, a mismatch stays an error. The two are one rule with
    //!< two verdicts -- the author may have meant the narrowing and the generator casts it, so an
    //!< error there would block a document that builds.
    void testNarrowingIsAWarning()
    {
        std::printf("- L7: narrowing warns, mismatch errors (rules 13/17)\n");

        {   // uint32 -> uint16 parameter: one warning, filed under the offset id, no error.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", NEMethod::SmAction)->addParam("p")->setType("uint16");
            doc.getAttributes().createAttribute("wide")->setType("uint32");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Attribute, "wide");

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countWarn(issues, 13) == 1);
            CHECK(warnSeverityIs(issues, 13, SMIssue::eSeverity::Warning));
            CHECK(countRule(issues, 13) == 0);       // the plain id is the error id, and no error fired
        }
        {   // The signed ladder narrows the same way the unsigned one does.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", NEMethod::SmAction)->addParam("p")->setType("int16");
            doc.getAttributes().createAttribute("wide")->setType("int32");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Attribute, "wide");

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countWarn(issues, 13) == 1);
            CHECK(countRule(issues, 13) == 0);
        }
        {   // AttributeSet narrows: same split, under its own rule (17 -> 117).
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("count")->setType("uint16");
            doc.getAttributes().createAttribute("total")->setType("uint32");
            SMAttributeSet* set = new SMAttributeSet(0, "count");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Attribute); set->setValue("total");

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countWarn(issues, 17) == 1);
            CHECK(warnSeverityIs(issues, 17, SMIssue::eSeverity::Warning));
            CHECK(countRule(issues, 17) == 0);
        }
        {   // bool and a number convert into each other in C++ without a cast, in both directions,
            // so neither pairing is a refusal. Both warn, and the code generator agrees.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", NEMethod::SmAction)->addParam("p")->setType("uint32");
            doc.getAttributes().createAttribute("flag")->setType("bool");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Attribute, "flag");

            doc.getAttributes().createAttribute("ready")->setType("bool");
            doc.getAttributes().createAttribute("count")->setType("uint32");
            SMAttributeSet* set = new SMAttributeSet(0, "ready");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Attribute); set->setValue("count");

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 13) == 0);
            CHECK(countWarn(issues, 13) == 1);
            CHECK(countRule(issues, 17) == 0);
            CHECK(countWarn(issues, 17) == 1);
        }
        {   // The same pairing in a CONDITION comparison (== / !=), not just an argument mapping:
            // areComparable used to refuse it outright ("cannot compare 'bool' with 'uint32'").
            // It warns now, like every other narrowing pair.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("flag")->setType("bool");
            doc.getAttributes().createAttribute("count")->setType("uint32");
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::Internal);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("flag");
            row->setOperator(eOp::Equal);
            row->setRhsKind(eSource::Attribute); row->setRhs("count");

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 13) == 0);
            CHECK(countWarn(issues, 13) == 1);
            CHECK(warnSeverityIs(issues, 13, SMIssue::eSeverity::Warning));
        }
        {   // char is deliberately not part of that: it stays a mismatch against a number.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", NEMethod::SmAction)->addParam("p")->setType("uint32");
            doc.getAttributes().createAttribute("letter")->setType("char");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Attribute, "letter");

            CHECK(countRule(SMValidator::validate(doc), 13) == 1);
        }
        {   // A mismatch is still an error, and says so in words a narrowing never uses.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("count")->setType("uint16");
            doc.getAttributes().createAttribute("label")->setType("String");
            SMAttributeSet* set = new SMAttributeSet(0, "count");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Attribute); set->setValue("label");

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 17) == 1);
            CHECK(countWarn(issues, 17) == 0);
            bool worded = false;
            for (const SMIssue& i : issues)
                worded = worded || ((i.rule == 17) && i.message.contains(QStringLiteral("No conversion")));
            CHECK(worded);
        }
        {   // Matching types raise neither, on either surface.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", NEMethod::SmAction)->addParam("p")->setType("uint16");
            doc.getAttributes().createAttribute("count")->setType("uint16");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Attribute, "count");
            SMAttributeSet* set = new SMAttributeSet(0, "count");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Attribute); set->setValue("count");

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 13) == 0);
            CHECK(countWarn(issues, 13) == 0);
            CHECK(countRule(issues, 17) == 0);
            CHECK(countWarn(issues, 17) == 0);
        }
        {   // A narrowing is a finding the user can act on: it names both types and navigates.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("act", NEMethod::SmAction)->addParam("p")->setType("uint16");
            doc.getAttributes().createAttribute("wide")->setType("uint32");
            SMActionCall* call = new SMActionCall(0, "act");
            s->getEntryList().addOperation(call);
            call->addArgument("p", eSource::Attribute, "wide");

            bool found = false;
            for (const SMIssue& i : SMValidator::validate(doc))
            {
                if (i.rule != (SMValidator::WARNING_RULE_BASE + 13))
                    continue;
                found = true;
                CHECK(i.message.contains(QStringLiteral("uint32")) && i.message.contains(QStringLiteral("uint16")));
                CHECK(i.detail.isEmpty() == false);
                CHECK(i.elementId == call->getId());
                CHECK(i.kind == eDocElementKind::Operation);
            }

            CHECK(found);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Warning rules (10.2 rules 1-11)
//////////////////////////////////////////////////////////////////////////

namespace
{
    // A reachable Normal state that has an outgoing transition, so it draws no W1/W2 itself.
    SMStateEntry* addReachedState(StateMachineData& doc, SMStateEntry* start, const QString& name, const QString& trigger)
    {
        SMStateEntry* st = doc.getStates().createState(name, eKind::Normal);
        if (doc.getMethods().findMethod(trigger) == nullptr)
            doc.getMethods().createMethod(trigger, NEMethod::SmTrigger);
        start->getTransitions().createTransition(eStim::Trigger, trigger, stateId(doc, name));
        return st;
    }

    void testWarnings()
    {
        std::printf("- warnings (10.2 rules 1-11)\n");

        {   // W1: an unreachable Normal state; negative once a transition targets it.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getStates().createState("Lost", eKind::Normal);
            CHECK(hasWarn(SMValidator::validate(doc), 1));

            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            s->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Lost"));
            CHECK(countWarn(SMValidator::validate(doc), 1) == 0);
        }
        {   // W2: a reachable Normal state with no outgoing transition; negative once it has one.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMStateEntry* work = addReachedState(doc, s, "Work", "go");
            CHECK(hasWarn(SMValidator::validate(doc), 2));

            doc.getMethods().createMethod("back", NEMethod::SmTrigger);
            work->getTransitions().createTransition(eStim::Trigger, "back", stateId(doc, "Idle"));
            CHECK(countWarn(SMValidator::validate(doc), 2) == 0);
        }
        {   // W2 negative: a substate with no transition of its own is not trapped, because the
            // composite around it reacts to a stimulus and takes the whole subtree with it.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMStateEntry* outer = addReachedState(doc, s, "Outer", "go");
            SMStateData* inner = outer->getOrCreateNestedStates();
            SMStateEntry* innerStart = inner->createState("InnerStart", eKind::Start);
            SMStateEntry* leaf = inner->createState("Leaf", eKind::Normal);
            innerStart->getTransitions().createTransition(eStim::Trigger, QString(), leaf->getId(), eTrans::Initial);
            CHECK(hasWarn(SMValidator::validate(doc), 2));      // nothing anywhere above the leaf leaves yet

            doc.getMethods().createMethod("back", NEMethod::SmTrigger);
            outer->getTransitions().createTransition(eStim::Trigger, "back", stateId(doc, "Idle"));
            CHECK(countWarn(SMValidator::validate(doc), 2) == 0);
        }
        {   // W2: an internal transition reacts and stays, so a state that owns nothing else is
            // still a state the machine cannot leave.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            SMStateEntry* work = addReachedState(doc, s, "Work", "go");
            doc.getMethods().createMethod("tick", NEMethod::SmTrigger);
            work->getTransitions().createTransition(eStim::Trigger, "tick", 0, eTrans::Internal);
            CHECK(hasWarn(SMValidator::validate(doc), 2));
        }
        {   // W3: a transition shadowed by an earlier unconditional one on the same stimulus.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            s->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Idle"));
            s->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Idle"));
            CHECK(hasWarn(SMValidator::validate(doc), 3));
        }
        {   // Negative W3: the first transition carries a condition, so it does not shadow.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMTransitionEntry* first = s->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Idle"));
            SMConditionEntry* row = first->getConditions().addCondition();
            row->setLhsKind(eSource::Expression); row->setExpression("count > 0");
            s->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Idle"));
            CHECK(countWarn(SMValidator::validate(doc), 3) == 0);
        }
        {   // W4: a declared constant that is never referenced. Data that nothing reads yet is
            // legitimate at any point in a design, so it is reported as information, never a warning.
            StateMachineData doc;
            addStart(doc);
            doc.getConstants().createConstant("Unused")->setType("int32");
            CHECK(hasWarn(SMValidator::validate(doc), 4));
            CHECK(warnSeverityIs(SMValidator::validate(doc), 4, SMIssue::eSeverity::Info));
        }
        {   // W4 severity, the other half: an unused ACTION is behaviour wired to nothing, and stays
            // a warning. One rule number, two severities, decided by the kind of the declaration.
            StateMachineData doc;
            addStart(doc);
            doc.getMethods().createMethod("orphan", NEMethod::SmAction);
            CHECK(hasWarn(SMValidator::validate(doc), 4));
            CHECK(warnSeverityIs(SMValidator::validate(doc), 4, SMIssue::eSeverity::Warning));
        }
        {   // Negative W4, the guard path: a canonical guard binds by symbol ID, not by name, so a
            // usage scan that only walked the legacy condition rows called this attribute unused.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            AttributeEntry* power = doc.getAttributes().createAttribute("Power");
            power->setType("int32");
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Idle"));
            tr->getGuard().setTree(SMGuardNode::makeCmp(SMGuardNode::eCmpOp::Ne
                                                       , SMGuardNode::makeRef(SMGuardNode::eKind::Attr, power->getId())
                                                       , SMGuardNode::makeVerbatim(SMGuardNode::eKind::Lit, "0")));
            CHECK(countWarn(SMValidator::validate(doc), 4) == 0);
        }
        {   // Same for a constant and for a condition method called by the guard.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            ConstantEntry* limit = doc.getConstants().createConstant("Limit");
            limit->setType("int32");
            MethodEntry* cond = doc.getMethods().createMethod("isReady", NEMethod::SmCondition);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Idle"));
            tr->getGuard().setTree(SMGuardNode::makeCmp(SMGuardNode::eCmpOp::Lt
                                                       , SMGuardNode::makeCall(cond->getId(), QList<SMGuardNode*>())
                                                       , SMGuardNode::makeRef(SMGuardNode::eKind::Const, limit->getId())));
            CHECK(countWarn(SMValidator::validate(doc), 4) == 0);
        }
        {   // A scope-qualified guard literal names its enumeration as plainly as a declaration does,
            // so the type it names is not "never referenced" either.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            doc.getDataTypes().addEnum("PowerState");
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Idle"));
            tr->getGuard().setTree(SMGuardNode::makeVerbatim(SMGuardNode::eKind::Lit, "PowerState::On"));
            CHECK(countWarn(SMValidator::validate(doc), 4) == 0);
        }
        {   // Negative W4: the constant is referenced by an AttributeSet.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getConstants().createConstant("Used")->setType("int32");
            doc.getAttributes().createAttribute("a")->setType("int32");
            SMAttributeSet* set = new SMAttributeSet(0, "a");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Constant); set->setValue("Used");
            CHECK(countWarn(SMValidator::validate(doc), 4) == 0);
        }
        {   // W5: an event sent but never reacted to; negative once a transition reacts.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getEvents().createEvent("ping");
            s->getEntryList().addOperation(new SMEventSend(0, "ping"));
            CHECK(hasWarn(SMValidator::validate(doc), 5));

            s->getTransitions().createTransition(eStim::Event, "ping", 0u, eTrans::Internal);
            CHECK(countWarn(SMValidator::validate(doc), 5) == 0);
        }
        {   // W6: a timer reacted to but never started; negative once a TimerStart appears.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getTimers().createTimer("tick");
            s->getTransitions().createTransition(eStim::Timer, "tick", 0u, eTrans::Internal);
            CHECK(hasWarn(SMValidator::validate(doc), 6));

            s->getEntryList().addOperation(new SMTimerStart(0, "tick"));
            CHECK(countWarn(SMValidator::validate(doc), 6) == 0);
        }
        {   // W7: an empty internal transition; negative once it carries an operation.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getTimers().createTimer("tick");
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Timer, "tick", 0u, eTrans::Internal);
            CHECK(hasWarn(SMValidator::validate(doc), 7));

            doc.getMethods().createMethod("act", NEMethod::SmAction);
            tr->getOperations().addOperation(new SMActionCall(0, "act"));
            CHECK(countWarn(SMValidator::validate(doc), 7) == 0);
        }
        {   // Direction is not a finding: an attribute the design only writes, and one it only
            // reads, are both fully referenced. The writing side of a read-only attribute lives in
            // the hand-written service code, which the document cannot see.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getAttributes().createAttribute("w")->setType("int32");
            SMAttributeSet* set = new SMAttributeSet(0, "w");
            s->getEntryList().addOperation(set);
            set->setSource(eSource::Value); set->setValue("1");
            CHECK(countWarn(SMValidator::validate(doc), 8) == 0);
            CHECK(countWarn(SMValidator::validate(doc), 4) == 0);

            doc.getAttributes().createAttribute("r")->setType("int32");
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::Internal);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Attribute); row->setLhs("r");
            row->setOperator(eOp::Greater);
            row->setRhsKind(eSource::Value); row->setRhs("0");
            CHECK(countWarn(SMValidator::validate(doc), 8) == 0);
            CHECK(countWarn(SMValidator::validate(doc), 4) == 0);

            // An unused attribute is not reported at all: it is public state, and being read only by
            // generated code outside the machine is normal. (An unused constant is still information.)
            doc.getAttributes().createAttribute("idle")->setType("int32");
            CHECK(countWarn(SMValidator::validate(doc), 4) == 0);
        }
        {   // W9: a comparison of two design-time constants; negative once one side is live.
            StateMachineData doc;
            SMStateEntry* s = addStart(doc);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            SMTransitionEntry* tr = s->getTransitions().createTransition(eStim::Trigger, "go", 0u, eTrans::Internal);
            SMConditionEntry* row = tr->getConditions().addCondition();
            row->setLhsKind(eSource::Value); row->setLhs("1");
            row->setOperator(eOp::Equal);
            row->setRhsKind(eSource::Value); row->setRhs("2");
            CHECK(hasWarn(SMValidator::validate(doc), 9));

            doc.getAttributes().createAttribute("x")->setType("int32");
            row->setLhsKind(eSource::Attribute); row->setLhs("x");
            CHECK(countWarn(SMValidator::validate(doc), 9) == 0);
        }
        {   // W10: history on a composite nothing re-enters. A transition out of the Start state
            // is the one-and-only entry, so it does not silence the warning; a transition from an
            // ordinary sibling does, because the machine can come back that way.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            comp->setHistory(SMStateEntry::eHistory::Shallow);
            CHECK(hasWarn(SMValidator::validate(doc), 10));

            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            start->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Comp"));
            CHECK(hasWarn(SMValidator::validate(doc), 10));

            SMStateEntry* pause = doc.getStates().createState("Pause", eKind::Normal);
            comp->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Pause"));
            pause->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Comp"));
            CHECK(countWarn(SMValidator::validate(doc), 10) == 0);
        }
        {   // W10 negative: a self-transition re-enters the composite through its own history.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            comp->setHistory(SMStateEntry::eHistory::Deep);
            doc.getMethods().createMethod("go", NEMethod::SmTrigger);
            start->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Comp"));
            comp->getTransitions().createTransition(eStim::Trigger, "go", stateId(doc, "Comp"));
            CHECK(countWarn(SMValidator::validate(doc), 10) == 0);
        }
        {   // W10 does not fire on a composite without history, however it is reached.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* comp = doc.getStates().createState("Comp", eKind::Normal);
            comp->getOrCreateNestedStates()->createState("Inner", eKind::Start);
            CHECK(countWarn(SMValidator::validate(doc), 10) == 0);
        }
        {   // W14: a declaration the generator has no comment for. Information and never a
            // warning: an undescribed element is legal, it just generates uncommented code.
            StateMachineData doc;
            addStart(doc);
            doc.getOverview().setName("Machine");
            doc.getTimers().createTimer("Tick");
            MethodEntry* action = doc.getMethods().createMethod("run", NEMethod::SmAction);
            action->addElement(MethodParameter(doc.getNextId(), "count", "int32"), true);
            CHECK(countWarn(SMValidator::validate(doc), 14) == 4);   // machine, timer, method, parameter
            CHECK(warnSeverityIs(SMValidator::validate(doc), 14, SMIssue::eSeverity::Info));

            doc.getOverview().setDescription("What the machine does");
            doc.getTimers().getElements()[0].setDescription("How often it ticks");
            action->setDescription("What it runs");
            action->getElements()[0].setDescription("How many");
            CHECK(countWarn(SMValidator::validate(doc), 14) == 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// A document with errors still validates (never throws / blocks)
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testErrorsDoNotBlock()
    {
        std::printf("- errors are reported, never fatal\n");
        StateMachineData doc;
        doc.getStates().createState("A", eKind::Normal);        // no Start: an error document.
        const QList<SMIssue> issues = SMValidator::validate(doc);
        CHECK(hasRule(issues, 1));
        // The engine returns findings and leaves the document intact and editable.
        CHECK(doc.getStates().getElements().isEmpty() == false);
    }
}

//////////////////////////////////////////////////////////////////////////
// Pseudo-state and per-kind name space (reported 2026-07-26)
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testPseudoStateAndKindNamespace()
    {
        std::printf("- Start pseudo-state and the per-kind method name space\n");

        {   // Negative: Start is a pseudo-state -- its outgoing transition is the initial one,
            // taken on entering the level, so it needs no stimulus at all.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            doc.getStates().createState("Work", eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, QString(), stateId(doc, "Work"), eTrans::Initial);
            CHECK(hasRule(SMValidator::validate(doc), 6) == false);
        }
        {   // Positive: an ordinary state still has to name a declared stimulus.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* work = doc.getStates().createState("Work", eKind::Normal);
            start->getTransitions().createTransition(eStim::Trigger, "begin", work->getId());
            doc.getMethods().createMethod("begin", NEMethod::SmTrigger);
            work->getTransitions().createTransition(eStim::Trigger, "ghost", stateId(doc, "Idle"));
            CHECK(hasRule(SMValidator::validate(doc), 6));
        }

        {   // Negative: a trigger, an action and a condition may all be called `on`. They become
            // members of different generated classes, so the names can never collide -- and each
            // must resolve to ITS OWN kind, not to whichever entry happens to be stored first.
            StateMachineData doc;
            SMStateEntry* start = addStart(doc);
            SMStateEntry* work = doc.getStates().createState("Work", eKind::Normal);
            CHECK(doc.getMethods().createMethod("on", NEMethod::SmTrigger) != nullptr);
            CHECK(doc.getMethods().createMethod("on", NEMethod::SmAction) != nullptr);
            CHECK(doc.getMethods().createMethod("on", NEMethod::SmCondition) != nullptr);

            start->getTransitions().createTransition(eStim::Trigger, "on", work->getId());
            work->getEntryList().addOperation(new SMActionCall(0, "on"));

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(hasRule(issues, 6) == false);     // neither the trigger nor the action is "undeclared"
            CHECK(hasRule(issues, 4) == false);     // and three kinds sharing one name is not a duplicate
        }
        {   // Positive: two entries of the SAME kind are still a duplicate.
            StateMachineData doc;
            addStart(doc);
            CHECK(doc.getMethods().createMethod("on", NEMethod::SmAction) != nullptr);
            CHECK(doc.getMethods().createMethod("on", NEMethod::SmAction) == nullptr);

            // The creator refuses the second one, so build the collision the way a hand-edited
            // document would carry it.
            MethodEntry* clone = new MethodEntry(doc.getNextId(), "on", NEMethod::SmAction, NEMethod::stateMachine(), &doc.getMethods());
            doc.getMethods().addElement(clone, false);
            CHECK(hasRule(SMValidator::validate(doc), 4));
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// One engine: the canvas query and the document run are the same check
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testUnifiedEngine()
    {
        std::printf("- one engine: canvas query and document run agree\n");

        StateMachineData doc;
        SMStateEntry* start = addStart(doc);
        SMStateEntry* work = doc.getStates().createState("Work", eKind::Normal);
        MethodEntry* act = doc.getMethods().createMethod("Walk", NEMethod::SmAction);
        CHECK(act != nullptr);
        MethodParameter required(act);
        required.setName("waiting");
        required.setType("uint32");
        act->addElement(std::move(required), true);

        SMActionCall* call = new SMActionCall(0, "Walk");
        work->getEntryList().addOperation(call);
        const uint32_t stateId = work->getId();
        start->getTransitions().createTransition(eStim::Trigger, QString(), stateId, eTrans::Initial);

        // The document run reports the unmapped formal WITH a message -- before unification the
        // canvas knew about this fault and the results list could not say what it was.
        {
            const QList<SMIssue> issues = SMValidator::validate(doc);
            int mappingFindings = 0;
            for (const SMIssue& issue : issues)
            {
                if (issue.rule == SMValidator::RULE_ARGUMENT_MAPPING)
                {
                    ++mappingFindings;
                    CHECK(issue.message.contains("waiting"));
                    CHECK(issue.detail.isEmpty() == false);
                }
            }

            CHECK(mappingFindings == 1);
        }

        // The canvas asks the SAME check, scoped to one element, and must agree with it.
        {
            DocIssue::eSeverity worst = DocIssue::eSeverity::Info;
            CHECK(SMOperationValidation::worstForState(doc, stateId, worst));
            CHECK(worst == DocIssue::eSeverity::Error);
        }

        // Mapping the formal clears both at once.
        call->getArguments().append(SMArgumentEntry(0u, "waiting", eSource::Value, "5"));
        {
            DocIssue::eSeverity worst = DocIssue::eSeverity::Info;
            CHECK(SMOperationValidation::worstForState(doc, stateId, worst) == false);
            CHECK(countRule(SMValidator::validate(doc), SMValidator::RULE_ARGUMENT_MAPPING) == 0);
        }

        // An orphan argument is a mapping fault on both paths too.
        call->getArguments().append(SMArgumentEntry(0u, "ghost", eSource::Value, "9"));
        {
            DocIssue::eSeverity worst = DocIssue::eSeverity::Info;
            CHECK(SMOperationValidation::worstForState(doc, stateId, worst));
            CHECK(countRule(SMValidator::validate(doc), SMValidator::RULE_ARGUMENT_MAPPING) == 1);
        }

        // Every finding of the one run speaks the one severity ladder, ordered so that a
        // worst-of is a max -- the property the canvas and the results list both rely on.
        CHECK(worstOf(DocIssue::eSeverity::Info, DocIssue::eSeverity::Error) == DocIssue::eSeverity::Error);
        CHECK(worstOf(DocIssue::eSeverity::Warning, DocIssue::eSeverity::Info) == DocIssue::eSeverity::Warning);
    }
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Submachine imports: resolution, hosting, cycles, version pinning
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< Writes a minimal, error-free machine to \p path, optionally importing other documents.
    //!< Gives a fresh document's root Start the one unguarded initial transition 10.1 rule 27
    //!< requires, pointing at a newly created Normal state. Returns that state.
    SMStateEntry* finishStart(StateMachineData& doc, const QString& first = QStringLiteral("Begin"))
    {
        SMStateEntry* target = doc.getStates().createState(first, eKind::Normal);
        SMStateEntry* start  = doc.getStates().getStartState();
        if ((start != nullptr) && (target != nullptr))
        {
            start->getTransitions().createTransition(eStim::Trigger, QString(), target->getId(), eTrans::Initial);
        }

        return target;
    }

    void writeMachine(const QString& path, const QString& name, const QString& version
                     , const QList<QPair<QString, QString> >& imports = QList<QPair<QString, QString> >()
                     , SMOverviewData::eThreading threading = SMOverviewData::eThreading::Local)
    {
        std::unique_ptr<StateMachineData> doc = StateMachineData::createNewDocument(name);
        doc->getOverview().setVersion(version);
        doc->getOverview().setThreading(threading);
        // A fresh document is a Start with nowhere to go, which is 10.1 rule 27 -- correctly, the
        // level never initialises. A host no longer inherits its import's findings, but these
        // machines are opened on their own in some of the tests, so each is a real machine.
        finishStart(*doc);
        for (const QPair<QString, QString>& one : imports)
        {
            IncludeEntry* entry = doc->getIncludes().createInclude(one.second);
            entry->setAlias(one.first);
            entry->setVersion(VersionNumber(QStringLiteral("1.0.0")));
        }

        doc->writeToFile(path);
        SMDocumentCache::getInstance().clear();
    }

    //!< One import of a written machine: the alias, the file it points at, and the single state
    //!< that hosts it.
    struct HostedAt { const char* alias; const char* location; const char* state; };

    //!< Writes a machine that hosts each of \p hosts at its own named state, so a chain of
    //!< hosting states can be built out of real documents on disk.
    void writeHostingMachine(const QString& path, const QString& name, const QList<HostedAt>& hosts)
    {
        std::unique_ptr<StateMachineData> doc = StateMachineData::createNewDocument(name);
        doc->getOverview().setVersion(QStringLiteral("1.0.0"));
        finishStart(*doc);
        for (const HostedAt& one : hosts)
        {
            IncludeEntry* entry = doc->getIncludes().createInclude(QString::fromLatin1(one.location));
            entry->setAlias(QString::fromLatin1(one.alias));
            entry->setVersion(VersionNumber(QStringLiteral("1.0.0")));
            SMStateEntry* state = doc->getStates().createState(QString::fromLatin1(one.state), eKind::Normal);
            state->setSubmachine(QString::fromLatin1(one.alias));
        }

        doc->writeToFile(path);
        SMDocumentCache::getInstance().clear();
    }

    //!< A host document at \p path with one registered import hosted by \p hostCount states.
    std::unique_ptr<StateMachineData> hostMachine(const QString& path, const QString& alias
                                                 , const QString& location, const QString& pinned
                                                 , int hostCount)
    {
        std::unique_ptr<StateMachineData> doc = StateMachineData::createNewDocument(QStringLiteral("Host"));
        finishStart(*doc);
        IncludeEntry* entry = doc->getIncludes().createInclude(location);
        entry->setAlias(alias);
        entry->setVersion(VersionNumber(pinned));
        for (int i = 0; i < hostCount; ++i)
        {
            SMStateEntry* state = doc->getStates().createState(QStringLiteral("Phase%1").arg(i + 1), eKind::Normal);
            state->setSubmachine(alias);
        }

        doc->setFilePath(path);
        return doc;
    }

    //!< Every reference fixture that ships next to the spec, validated. A fixture nobody
    //!< validates is a fixture that can go quietly wrong -- and one did: TurnCycle.fsml carried an
    //!< empty Stimulus on an ordinary state for as long as nothing checked it.
    void testReferenceFixtures()
    {
        std::printf("- reference fixtures validate as expected\n");
        // The expected ERROR rules of each fixture, pinned. "Deliberate" means the fixture exists
        // partly to exercise that refusal; anything not listed here is a defect in the fixture.
        struct Expect { const char* name; QList<int> errors; const char* why; };
        const QList<Expect> expected =
        {
              { "TrafficLight.fsml"     , {}          , "the golden machine: must be clean" }
            , { "FullFeature.fsml"      , {18, 19, 25, 34}, "deliberate: History on a non-composite, an unresolved import, a draft guard, a removed InlineCode tag."
                                                           " 'Operational' and its descendants both reacting to Dispensed is the plain"
                                                           " hierarchical override and is reported by nothing" }
            , { "GuardDemo.fsml"        , {}          , "every guard node kind, all resolved" }
            , { "SubmachineDemo.fsml"   , {}          , "one import hosted twice: the fixture that must GENERATE" }
            , { "UnresolvedImport.fsml" , {19}        , "deliberate: the import file does not exist" }
            , { "TurnCycle.fsml"        , {}          , "the imported machine: must be clean" }
            , { "LegacyImports.fsml"    , {}          , "1.0.0 migration path" }
            , { "LegacyStart.fsml"      , {}          , "the merged Kind=\"Start\": converts on load, then clean" }
            , { "LegacyKind.fsml"       , {}          , "no Transition Kind: the read shim recovers all three" }
            , { "ThreadingMismatch.fsml", {}          , "a Shared host with a Local import: legal, and the host decides for the tree" }
        };

        for (const Expect& one : expected)
        {
            const char* const name = one.name;
            const QString path = QString(LUSAN_TEST_DATA_DIR) + QDir::separator() + QString::fromLatin1(name);
            StateMachineData doc;
            if (doc.readFromFile(path) == false)
            {
                std::printf("    %-24s COULD NOT OPEN\n", name);
                continue;
            }

            SMDocumentCache::getInstance().clear();
            const QList<SMIssue> issues = SMValidator::validate(doc);
            QList<int> errs;
            for (const SMIssue& i : issues)
                if ((i.severity == SMIssue::eSeverity::Error) && (errs.contains(i.rule) == false))
                    errs.append(i.rule);
            std::sort(errs.begin(), errs.end());

            QStringList text;
            for (int r : errs) text << QString::number(r);
            std::printf("    %-24s errors: %s\n", name
                       , errs.isEmpty() ? "none" : text.join(QStringLiteral(",")).toStdString().c_str());

            // When a fixture does carry errors, the message is what tells you whether it is
            // deliberate or rotten. Printing it here is the difference between a number to
            // look up and a defect you can act on.
            for (const SMIssue& i : issues)
            {
                if (i.severity == SMIssue::eSeverity::Error)
                    std::printf("      rule %-3d %s\n", i.rule, i.message.toStdString().c_str());
            }

            QList<int> want = one.errors;
            std::sort(want.begin(), want.end());
            CHECK(errs == want);
            if (errs != want)
                std::printf("      [EXPECTED] %s\n", one.why);

            // The golden machine's leaves live inside composites that react to power_off and to
            // the phase timers, so nothing in it is a dead end -- reporting one was the whole of
            // the reported fault.
            if (QString::fromLatin1(name) == QStringLiteral("TrafficLight.fsml"))
            {
                CHECK(countWarn(issues, 2) == 0);
            }
        }
    }

    void testImports()
    {
        std::printf("- submachine imports\n");
        QTemporaryDir dir;
        CHECK(dir.isValid());
        const QString root = dir.path();
        const auto at = [&root](const char* file) -> QString { return QDir(root).absoluteFilePath(QString::fromLatin1(file)); };

        {   // A TurnCycle-style import instantiated by two states survives a save/reload round trip.
            writeMachine(at("turncycle.fsml"), QStringLiteral("TurnCycle"), QStringLiteral("1.2.0"));
            std::unique_ptr<StateMachineData> host = hostMachine(at("host.fsml"), QStringLiteral("TurnCycle"), QStringLiteral("./turncycle.fsml"), QStringLiteral("1.2.0"), 2);
            CHECK(host->writeToFile(at("host.fsml")));

            StateMachineData reloaded;
            CHECK(reloaded.readFromFile(at("host.fsml")));
            const IncludeEntry* entry = reloaded.findImportByAlias(QStringLiteral("TurnCycle"));
            CHECK(entry != nullptr);
            CHECK((entry != nullptr) && (entry->getLocation() == QStringLiteral("./turncycle.fsml")));
            CHECK((entry != nullptr) && (entry->getVersion().toString() == QStringLiteral("1.2.0")));

            int hosts = 0;
            for (const SMStateEntry* state : reloaded.getStates().getElements())
                if ((state != nullptr) && (state->getSubmachine() == QStringLiteral("TurnCycle"))) ++hosts;
            CHECK(hosts == 2);

            SMDocumentCache::getInstance().clear();
            const QList<SMIssue> issues = SMValidator::validate(reloaded);
            CHECK(countRule(issues, 19) == 0);
            CHECK(countRule(issues, 22) == 0);
        }

        {   // A missing file flags the registration AND every hosting state, and never blocks the open.
            std::unique_ptr<StateMachineData> host = hostMachine(at("broken.fsml"), QStringLiteral("Gone"), QStringLiteral("./no-such-machine.fsml"), QStringLiteral("1.0.0"), 2);
            CHECK(host->writeToFile(at("broken.fsml")));

            StateMachineData reloaded;
            CHECK(reloaded.readFromFile(at("broken.fsml")));
            CHECK(reloaded.openSucceeded());

            SMDocumentCache::getInstance().clear();
            const QList<SMIssue> issues = SMValidator::validate(reloaded);
            CHECK(countRule(issues, 19) == 3);      // one registration + two hosting states
            int onStates = 0;
            for (const SMIssue& i : issues)
                if ((i.rule == 19) && (i.kind == eDocElementKind::State)) ++onStates;
            CHECK(onStates == 2);
        }

        {   // An unreadable file is reported like a missing one, not swallowed.
            QFile garbage(at("garbage.fsml"));
            CHECK(garbage.open(QIODevice::WriteOnly | QIODevice::Text));
            garbage.write("this is not xml at all");
            garbage.close();

            std::unique_ptr<StateMachineData> host = hostMachine(at("host2.fsml"), QStringLiteral("Junk"), QStringLiteral("./garbage.fsml"), QStringLiteral("1.0.0"), 1);
            SMDocumentCache::getInstance().clear();
            const QList<SMIssue> issues = SMValidator::validate(*host);
            CHECK(countRule(issues, 19) >= 1);
        }

        {   // A direct cycle: the document imports itself.
            writeMachine(at("selfish.fsml"), QStringLiteral("Selfish"), QStringLiteral("1.0.0")
                        , QList<QPair<QString, QString> >{ qMakePair(QStringLiteral("Selfish"), QStringLiteral("./selfish.fsml")) });
            StateMachineData doc;
            CHECK(doc.readFromFile(at("selfish.fsml")));
            SMDocumentCache::getInstance().clear();
            CHECK(hasRule(SMValidator::validate(doc), 19));
        }

        {   // A transitive cycle: A imports B, B imports C, C imports A.
            writeMachine(at("cycA.fsml"), QStringLiteral("CycA"), QStringLiteral("1.0.0")
                        , QList<QPair<QString, QString> >{ qMakePair(QStringLiteral("CycB"), QStringLiteral("./cycB.fsml")) });
            writeMachine(at("cycB.fsml"), QStringLiteral("CycB"), QStringLiteral("1.0.0")
                        , QList<QPair<QString, QString> >{ qMakePair(QStringLiteral("CycC"), QStringLiteral("./cycC.fsml")) });
            writeMachine(at("cycC.fsml"), QStringLiteral("CycC"), QStringLiteral("1.0.0")
                        , QList<QPair<QString, QString> >{ qMakePair(QStringLiteral("CycA"), QStringLiteral("./cycA.fsml")) });

            StateMachineData doc;
            CHECK(doc.readFromFile(at("cycA.fsml")));
            SMDocumentCache::getInstance().clear();
            CHECK(hasRule(SMValidator::validate(doc), 19));
        }

        {   // A chain that never returns to the host is not a cycle.
            writeMachine(at("leaf.fsml"), QStringLiteral("Leaf"), QStringLiteral("1.0.0"));
            writeMachine(at("mid.fsml"), QStringLiteral("Mid"), QStringLiteral("1.0.0")
                        , QList<QPair<QString, QString> >{ qMakePair(QStringLiteral("Leaf"), QStringLiteral("./leaf.fsml")) });
            std::unique_ptr<StateMachineData> host = hostMachine(at("top.fsml"), QStringLiteral("Mid"), QStringLiteral("./mid.fsml"), QStringLiteral("1.0.0"), 1);
            SMDocumentCache::getInstance().clear();
            CHECK(countRule(SMValidator::validate(*host), 19) == 0);
        }

        {   // Version pinning: major = error 22, minor = warning 12, patch = information 12.
            writeMachine(at("pinned.fsml"), QStringLiteral("Pinned"), QStringLiteral("2.5.7"));

            std::unique_ptr<StateMachineData> major = hostMachine(at("h1.fsml"), QStringLiteral("Pinned"), QStringLiteral("./pinned.fsml"), QStringLiteral("1.5.7"), 1);
            SMDocumentCache::getInstance().clear();
            CHECK(hasRule(SMValidator::validate(*major), 22));

            std::unique_ptr<StateMachineData> minor = hostMachine(at("h2.fsml"), QStringLiteral("Pinned"), QStringLiteral("./pinned.fsml"), QStringLiteral("2.4.7"), 1);
            SMDocumentCache::getInstance().clear();
            const QList<SMIssue> minorIssues = SMValidator::validate(*minor);
            CHECK(hasWarn(minorIssues, 12));
            CHECK(warnSeverityIs(minorIssues, 12, SMIssue::eSeverity::Warning));
            CHECK(countRule(minorIssues, 22) == 0);

            std::unique_ptr<StateMachineData> patch = hostMachine(at("h3.fsml"), QStringLiteral("Pinned"), QStringLiteral("./pinned.fsml"), QStringLiteral("2.5.1"), 1);
            SMDocumentCache::getInstance().clear();
            const QList<SMIssue> patchIssues = SMValidator::validate(*patch);
            CHECK(hasWarn(patchIssues, 12));
            CHECK(warnSeverityIs(patchIssues, 12, SMIssue::eSeverity::Info));

            std::unique_ptr<StateMachineData> exact = hostMachine(at("h4.fsml"), QStringLiteral("Pinned"), QStringLiteral("./pinned.fsml"), QStringLiteral("2.5.7"), 1);
            SMDocumentCache::getInstance().clear();
            const QList<SMIssue> exactIssues = SMValidator::validate(*exact);
            CHECK(countWarn(exactIssues, 12) == 0);
            CHECK(countRule(exactIssues, 22) == 0);
        }

        {   // Threading pairing is not validated. A hosted machine runs under the synchronization
            // object of the machine that hosts it, so the host decides for the whole import tree and
            // no combination of the two values is worth a message. All four combinations are clean;
            // the two retired numbers, 26 and warning 13, must never come back.
            writeMachine(at("local.fsml"), QStringLiteral("LocalMachine"), QStringLiteral("1.0.0")
                        , QList<QPair<QString, QString> >(), SMOverviewData::eThreading::Local);
            writeMachine(at("shared.fsml"), QStringLiteral("SharedMachine"), QStringLiteral("1.0.0")
                        , QList<QPair<QString, QString> >(), SMOverviewData::eThreading::Shared);

            struct Pairing
            {
                const char*                 file;
                const char*                 alias;
                const char*                 import;
                SMOverviewData::eThreading  host;
            };

            const Pairing pairings[] =
            {
                  { "t1.fsml", "L", "./local.fsml" , SMOverviewData::eThreading::Shared }
                , { "t2.fsml", "S", "./shared.fsml", SMOverviewData::eThreading::Local  }
                , { "t3.fsml", "L", "./local.fsml" , SMOverviewData::eThreading::Local  }
                , { "t4.fsml", "S", "./shared.fsml", SMOverviewData::eThreading::Shared }
            };

            for (const Pairing& p : pairings)
            {
                std::unique_ptr<StateMachineData> doc = hostMachine(at(p.file), QString::fromLatin1(p.alias)
                                                                   , QString::fromLatin1(p.import), QStringLiteral("1.0.0"), 1);
                doc->getOverview().setThreading(p.host);
                SMDocumentCache::getInstance().clear();
                const QList<SMIssue> issues = SMValidator::validate(*doc);
                CHECK(countRule(issues, 26) == 0);
                CHECK(countWarn(issues, 13) == 0);
                CHECK(countSeverity(issues, SMIssue::eSeverity::Error) == 0);
            }
        }

        {   // L7 Part 2: an imported document is validated when it is OPENED, not through its host.
            // Its Start goes nowhere, which is rule 27 -- a real error, in that document, on an
            // element only that document contains.
            {
                std::unique_ptr<StateMachineData> inner = StateMachineData::createNewDocument(QStringLiteral("Inert"));
                inner->getOverview().setVersion(QStringLiteral("1.0.0"));
                CHECK(inner->writeToFile(at("inert.fsml")));
                SMDocumentCache::getInstance().clear();
            }

            // Opened directly, the finding is there and names its own element.
            StateMachineData opened;
            CHECK(opened.readFromFile(at("inert.fsml")));
            SMDocumentCache::getInstance().clear();
            CHECK(hasRule(SMValidator::validate(opened), SMValidator::RULE_PSEUDO_START));

            // Opened as a host, none of it appears: the host reports the relationship only, and
            // the relationship is sound -- the file is there, it parses, the pin matches.
            std::unique_ptr<StateMachineData> host = hostMachine(at("inerthost.fsml"), QStringLiteral("Inert"), QStringLiteral("./inert.fsml"), QStringLiteral("1.0.0"), 1);
            SMDocumentCache::getInstance().clear();
            const QList<SMIssue> hostIssues = SMValidator::validate(*host);
            CHECK(countRule(hostIssues, SMValidator::RULE_PSEUDO_START) == 0);
            CHECK(countRule(hostIssues, 19) == 0);
        }

        {   // An absolute location resolves too, and a picked file is stored relative to the host.
            writeMachine(at("abs.fsml"), QStringLiteral("Abs"), QStringLiteral("1.0.0"));
            std::unique_ptr<StateMachineData> host = hostMachine(at("h5.fsml"), QStringLiteral("Abs"), at("abs.fsml"), QStringLiteral("1.0.0"), 1);
            SMDocumentCache::getInstance().clear();
            CHECK(countRule(SMValidator::validate(*host), 19) == 0);
            CHECK(SMImportResolver::storableLocation(*host, at("abs.fsml")) == QStringLiteral("./abs.fsml"));
        }

        {   // Rule 31. The generated constructor names one action handler per machine below this
            // one, after the chain of hosting states joined by '_'. '_' is legal inside a state
            // name, so two different chains can join to one name -- and then neither can be
            // declared. `Crossing_Timer` hosting the leaf directly meets `Crossing` hosting a
            // machine that hosts the same leaf at `Timer`.
            writeMachine(at("clashleaf.fsml"), QStringLiteral("NameClashLeaf"), QStringLiteral("1.0.0"));
            writeHostingMachine(at("clashmid.fsml"), QStringLiteral("ClashMid")
                               , { { "Leaf", "./clashleaf.fsml", "Timer" } });

            writeHostingMachine(at("clashtop.fsml"), QStringLiteral("ClashTop")
                               , { { "Mid" , "./clashmid.fsml" , "Crossing"       }
                                 , { "Leaf", "./clashleaf.fsml", "Crossing_Timer" } });
            StateMachineData clash;
            CHECK(clash.readFromFile(at("clashtop.fsml")));
            SMDocumentCache::getInstance().clear();
            const QList<SMIssue> clashIssues = SMValidator::validate(clash);
            CHECK(countRule(clashIssues, 31) == 1);

            // The finding is only useful if it says which two chains met and what to rename.
            bool named = false;
            for (const SMIssue& i : clashIssues)
            {
                named = named || ((i.rule == 31)
                                  && i.message.contains(QStringLiteral("Crossing -> Timer"))
                                  && i.message.contains(QStringLiteral("Crossing_Timer"))
                                  && i.message.contains(QStringLiteral("crossing_Timer")));
            }
            CHECK(named);

            // One character apart, and nothing collides.
            writeHostingMachine(at("cleartop.fsml"), QStringLiteral("ClearTop")
                               , { { "Mid" , "./clashmid.fsml" , "Crossing"      }
                                 , { "Leaf", "./clashleaf.fsml", "CrossingTimer" } });
            StateMachineData clear;
            CHECK(clear.readFromFile(at("cleartop.fsml")));
            SMDocumentCache::getInstance().clear();
            CHECK(countRule(SMValidator::validate(clear), 31) == 0);

            // Two states hosting the SAME machine is the shape SubmachineDemo.fsml ships, and it
            // must stay clean at every level: the two chains differ from their first name on, so
            // the grandchild each of them carries differs too.
            std::unique_ptr<StateMachineData> twice = hostMachine(at("twicetop.fsml"), QStringLiteral("Mid")
                                                                 , QStringLiteral("./clashmid.fsml"), QStringLiteral("1.0.0"), 2);
            SMDocumentCache::getInstance().clear();
            CHECK(countRule(SMValidator::validate(*twice), 31) == 0);
        }
    }
}

namespace
{
    //!< Writes a chain of \p count machines where each imports the next, and returns the path of
    //!< the first. `link1.fsml` imports `link2.fsml`, and so on; the last imports nothing.
    QString writeImportChain(const QString& root, int count)
    {
        const auto at = [&root](int index) -> QString
        {
            return QDir(root).absoluteFilePath(QStringLiteral("link%1.fsml").arg(index));
        };

        for (int i = count; i >= 1; --i)
        {
            std::unique_ptr<StateMachineData> doc = StateMachineData::createNewDocument(QStringLiteral("Link%1").arg(i));
            finishStart(*doc);   // every link is a real machine, so the depth is the only fault
            if (i < count)
            {
                IncludeEntry* entry = doc->getIncludes().createInclude(QStringLiteral("./link%1.fsml").arg(i + 1));
                entry->setAlias(QStringLiteral("Next"));
                entry->setVersion(doc->getOverview().getVersion());
            }

            doc->writeToFile(at(i));
        }

        SMDocumentCache::getInstance().clear();
        return at(1);
    }

    //!< A host importing the chain head, hosted by one state.
    std::unique_ptr<StateMachineData> chainHost(const QString& path, const QString& head)
    {
        std::unique_ptr<StateMachineData> doc = StateMachineData::createNewDocument(QStringLiteral("DepthHost"));
        finishStart(*doc);
        doc->setFilePath(path);
        IncludeEntry* entry = doc->getIncludes().createInclude(head);
        entry->setAlias(QStringLiteral("Head"));
        {   // Pin what the imported file actually says, so a version-drift finding cannot be
            // mistaken for a depth finding.
            const SMImportResolver::Resolution resolution = SMImportResolver::resolve(*doc, *entry);
            entry->setVersion(resolution.isResolved() ? resolution.actualVersion : VersionNumber());
        }

        SMStateEntry* state = doc->getStates().createState(QStringLiteral("Phase"), eKind::Normal);
        state->setSubmachine(QStringLiteral("Head"));
        return doc;
    }

    void testImportDepth()
    {
        std::printf("- import depth limit\n");
        QTemporaryDir dir;
        CHECK(dir.isValid());
        const QString root = dir.path();
        const QString hostPath = QDir(root).absoluteFilePath(QStringLiteral("depthhost.fsml"));

        {   // Ten imported documents below the host is exactly the limit and stays clean.
            const QString head = writeImportChain(root, SMImportResolver::MAX_IMPORT_DEPTH);
            std::unique_ptr<StateMachineData> host = chainHost(hostPath, head);
            SMDocumentCache::getInstance().clear();
            const QList<SMIssue> issues = SMValidator::validate(*host);
            CHECK(countRule(issues, 19) == 0);
        }

        {   // One more crosses it: the registration and every hosting state carry the error.
            const QString head = writeImportChain(root, SMImportResolver::MAX_IMPORT_DEPTH + 1);
            std::unique_ptr<StateMachineData> host = chainHost(hostPath, head);
            SMDocumentCache::getInstance().clear();
            const QList<SMIssue> issues = SMValidator::validate(*host);
            CHECK(countRule(issues, 19) == 2);
            bool onRegistration = false;
            bool onState = false;
            for (const SMIssue& issue : issues)
            {
                if (issue.rule != 19)
                    continue;
                onRegistration = onRegistration || (issue.kind == eDocElementKind::Import);
                onState = onState || (issue.kind == eDocElementKind::State);
            }

            CHECK(onRegistration);
            CHECK(onState);
        }

        {   // Painted nesting carries no cross-document reference and gets no depth limit.
            StateMachineData doc;
            addStart(doc);
            SMStateEntry* level = doc.getStates().createState(QStringLiteral("Level1"), eKind::Normal);
            for (int i = 2; i <= 15; ++i)
            {
                SMStateData* nested = level->getOrCreateNestedStates();
                nested->createState(QStringLiteral("Start%1").arg(i), eKind::Start);
                level = nested->createState(QStringLiteral("Level%1").arg(i), eKind::Normal);
            }

            CHECK(countRule(SMValidator::validate(doc), 19) == 0);
        }

        {   // An unreadable import is still a registration; validation is what flags it.
            const QString badPath = QDir(root).absoluteFilePath(QStringLiteral("garbage.fsml"));
            QFile bad(badPath);
            CHECK(bad.open(QIODevice::WriteOnly));
            bad.write("this is not a state machine");
            bad.close();
            SMDocumentCache::getInstance().clear();

            std::unique_ptr<StateMachineData> host = chainHost(hostPath, badPath);
            CHECK(host->machineImports().size() == 1);
            const QList<SMIssue> issues = SMValidator::validate(*host);
            CHECK(hasRule(issues, 19));
        }
    }

    void testIncludeRegistry()
    {
        std::printf("- include registry: aliases, duplicates, unused imports\n");

        {   // Two machines under one alias is genuinely ambiguous: a state's Submachine cannot
            // say which it meant.
            StateMachineData doc;
            addStart(doc);
            addImport(doc, "Lib", "./one.fsml");
            addImport(doc, "Lib", "./two.fsml");
            CHECK(hasRule(SMValidator::validate(doc), 4));
        }

        {   // The same file twice changes nothing about the generated machine, so it is a nudge,
            // not an error. The UI cannot create one; a hand-edited file can.
            StateMachineData doc;
            addStart(doc);
            doc.getIncludes().createInclude("common/Global.hpp");
            IncludeEntry twice(doc.getIncludes().getNextId(), QStringLiteral("common/Global.hpp"), &doc.getIncludes());
            doc.getIncludes().addElement(std::move(twice), false);
            const QList<SMIssue> issues = SMValidator::validate(doc);
            bool warned = false;
            for (const SMIssue& issue : issues)
            {
                warned = warned || ((issue.rule == (SMValidator::WARNING_RULE_BASE + 4))
                                    && (issue.severity == SMIssue::eSeverity::Warning)
                                    && issue.message.contains(QStringLiteral("more than once")));
            }

            CHECK(warned);
        }

        {   // Nothing can host a machine that has no alias.
            StateMachineData doc;
            addStart(doc);
            doc.getIncludes().createInclude("./anonymous.fsml");
            CHECK(hasRule(SMValidator::validate(doc), 18));
        }

        {   // An unused import stays a warning after the merge: dead wiring, but it cannot make
            // the generated machine differ from the drawn one.
            StateMachineData doc;
            addStart(doc);
            addImport(doc, "Unused", "./unused.fsml");
            const QList<SMIssue> issues = SMValidator::validate(doc);
            bool unusedWarning = false;
            for (const SMIssue& issue : issues)
            {
                unusedWarning = unusedWarning || ((issue.kind == eDocElementKind::Import)
                                                  && (issue.severity == SMIssue::eSeverity::Warning)
                                                  && issue.message.contains(QStringLiteral("never used")));
            }

            CHECK(unusedWarning);
        }
    }

    void testDataTypeGaps()
    {
        std::printf("- declared type resolution: condition returns and templated types\n");

        {   // A condition's Return is a declared type like any other.
            StateMachineData doc;
            addStart(doc);
            MethodEntry* cond = doc.getMethods().createMethod("IsReady", NEMethod::SmCondition);
            cond->setReturn("NoSuchType");
            cond->setImplement(MethodEntry::eImplement::Handler);
            CHECK(hasRule(SMValidator::validate(doc), 6));
        }

        {   // A templated type is its container plus its arguments, and each has to exist.
            StateMachineData doc;
            addStart(doc);
            AttributeEntry* attr = doc.getAttributes().createAttribute("Items");
            attr->setType("Array<Foo>");
            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(hasRule(issues, 6));
            bool namesFragment = false;
            for (const SMIssue& issue : issues)
            {
                namesFragment = namesFragment || ((issue.rule == 6) && issue.message.contains(QStringLiteral("'Foo'")));
            }

            CHECK(namesFragment);
        }

        {   // Every fragment resolves, so nothing is reported.
            StateMachineData doc;
            addStart(doc);
            AttributeEntry* attr = doc.getAttributes().createAttribute("Counts");
            attr->setType("Array<uint32>");
            CHECK(countRule(SMValidator::validate(doc), 6) == 0);
        }

        {   // An unregistered container name is itself an unresolved fragment, and is the first
            // one, so that is what the message names.
            StateMachineData doc;
            addStart(doc);
            AttributeEntry* attr = doc.getAttributes().createAttribute("Unknown");
            attr->setType("NEArray<Foo>");
            const QList<SMIssue> issues = SMValidator::validate(doc);
            bool namesContainer = false;
            for (const SMIssue& issue : issues)
            {
                namesContainer = namesContainer || ((issue.rule == 6) && issue.message.contains(QStringLiteral("'NEArray'")));
            }

            CHECK(namesContainer);
        }

        {   // A nested argument is reported by name, not as the whole expression.
            StateMachineData doc;
            addStart(doc);
            AttributeEntry* attr = doc.getAttributes().createAttribute("Lookup");
            attr->setType("Map<String, Foo>");
            const QList<SMIssue> issues = SMValidator::validate(doc);
            bool namesFragment = false;
            for (const SMIssue& issue : issues)
            {
                namesFragment = namesFragment || ((issue.rule == 6) && issue.message.contains(QStringLiteral("'Foo'")));
            }

            CHECK(namesFragment);
        }

        {   // A structure field declares a type, so it is checked like any other declaration. The
            // second field resolves, which keeps the case honest about counting only the broken one.
            StateMachineData doc;
            addStart(doc);
            DataTypeStructure* rec = static_cast<DataTypeStructure*>(doc.getDataTypes().addStructure(QStringLiteral("Record")));
            CHECK(rec != nullptr);
            rec->addField(QStringLiteral("id"))->setType(QStringLiteral("uint32"));
            rec->addField(QStringLiteral("payload"))->setType(QStringLiteral("Blob"));

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 6) == 1);
            bool namesField = false;
            for (const SMIssue& issue : issues)
            {
                namesField = namesField || ((issue.rule == 6) && issue.message.contains(QStringLiteral("'Blob'")));
            }

            CHECK(namesField);
        }

        {   // Both element types of a container are declarations too, and each is reported on its own.
            StateMachineData doc;
            addStart(doc);
            DataTypeContainer* map = static_cast<DataTypeContainer*>(
                    doc.getDataTypes().addCustomDataType(QStringLiteral("Lookup"), DataTypeBase::eCategory::Container));
            CHECK(map != nullptr);
            map->setContainer(QStringLiteral("HashMap"));
            map->setKey(QStringLiteral("KeyKind"));
            map->setValue(QStringLiteral("ValueKind"));

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, 6) == 2);
        }

        {   // The same container with types that exist reports nothing, so the check is not simply
            // refusing every container.
            StateMachineData doc;
            addStart(doc);
            DataTypeContainer* map = static_cast<DataTypeContainer*>(
                    doc.getDataTypes().addCustomDataType(QStringLiteral("Lookup"), DataTypeBase::eCategory::Container));
            CHECK(map != nullptr);
            map->setContainer(QStringLiteral("HashMap"));
            map->setKey(QStringLiteral("String"));
            map->setValue(QStringLiteral("uint32"));
            CHECK(countRule(SMValidator::validate(doc), 6) == 0);
        }
    }
}

namespace
{
    //!< True when the rule was reported at all, and every finding of it carries the one
    //!< explanation the shape has. The service interface tests assert the same texts, so a
    //!< defect both engines can find is described once.
    bool explains(const QList<SMIssue>& issues, int rule, DocRuleChecks::eShape shape)
    {
        const QString expected = DocRuleChecks::explainShape(shape);
        int found = 0;
        for (const SMIssue& issue : issues)
        {
            if (issue.rule != rule)
                continue;
            if (issue.detail != expected)
                return false;

            ++found;
        }

        return (found > 0);
    }

    //!< The rule shapes both document engines share now come from one place. The numbers are
    //!< unchanged -- the emitted id is what an author reads and what the generator files under.
    void testSharedRuleShapes()
    {
        std::printf("- shared rule shapes: one answer, one wording, this engine's numbers\n");

        {   // A name no compiler would take is the identifier fault, and the bound is the one
            // every editor field caps at.
            StateMachineData doc;
            addStart(doc);
            doc.getAttributes().createAttribute(QString(NELusanCommon::MAX_IDENTIFIER_LENGTH + 1, QLatin1Char('a')))
                              ->setType(QStringLiteral("uint32"));
            CHECK(countRule(SMValidator::validate(doc), SMValidator::RULE_INVALID_IDENTIFIER) == 1);
        }

        {   // A declaration with no name at all is the same fault under the same number, and it
            // now says what is missing instead of quoting an empty string.
            StateMachineData doc;
            addStart(doc);
            doc.getConstants().createConstant(QString());

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, SMValidator::RULE_INVALID_IDENTIFIER) == 1);
            bool saysMissing = false;
            for (const SMIssue& issue : issues)
            {
                saysMissing = saysMissing || ((issue.rule == SMValidator::RULE_INVALID_IDENTIFIER)
                                              && issue.message.contains(QStringLiteral("has no name")));
            }

            CHECK(saysMissing);
        }

        {   // A duplicate name names the entry the author has to rename, whichever registry it
            // is in, and keeps its number.
            StateMachineData doc;
            addStart(doc);
            doc.getAttributes().createAttribute(QStringLiteral("speed"))->setType(QStringLiteral("uint32"));
            AttributeEntry* clash = doc.getAttributes().createAttribute(QStringLiteral("velocity"));
            CHECK(clash != nullptr);
            clash->setType(QStringLiteral("uint32"));
            clash->setName(QStringLiteral("speed"));

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(countRule(issues, SMValidator::RULE_DUPLICATE_NAME) == 1);
            bool namesIt = false;
            for (const SMIssue& issue : issues)
            {
                namesIt = namesIt || ((issue.rule == SMValidator::RULE_DUPLICATE_NAME)
                                      && issue.message.contains(QStringLiteral("Attribute 'speed'")));
            }

            CHECK(namesIt);
        }

        {   // One shape, one explanation. Both engines attach the shared text to the shared
            // shapes, so the results panel cannot describe one defect two ways.
            StateMachineData doc;
            addStart(doc);
            doc.getAttributes().createAttribute(QStringLiteral("shape"))->setType(QStringLiteral("Missing"));
            doc.getConstants().createConstant(QStringLiteral("Unused"))->setType(QStringLiteral("uint32"));

            const QList<SMIssue> issues = SMValidator::validate(doc);
            CHECK(explains(issues, SMValidator::RULE_UNRESOLVED_REFERENCE, DocRuleChecks::eShape::UnresolvedType));
            CHECK(explains(issues, SMValidator::WARNING_RULE_BASE + SMValidator::RULE_UNREFERENCED
                          , DocRuleChecks::eShape::Unreferenced));
        }
    }
}

int main(int /*argc*/, char* /*argv*/[])
{
    std::printf("=== FSM validation engine tests ===\n");

    testStartState();
    testDuplicates();
    testNameCollisions();
    testDeclarationCollisions();
    testIdentifiers();
    testReferences();
    testFinalStart();
    testPseudoStartRules();
    testTransitionKindRules();
    testReachabilityRules();
    testArguments();
    testParamScope();
    testComposite();
    testMethodBody();
    testConditionLhsOnly();
    testExpressionRows();
    testFindingShape();
    testWideningTable();
    testTypeRules();
    testNarrowingIsAWarning();
    testWarnings();
    testErrorsDoNotBlock();
    testPseudoStateAndKindNamespace();
    testUnifiedEngine();
    testReferenceFixtures();
    testImports();
    testImportDepth();
    testIncludeRegistry();
    testDataTypeGaps();
    testDefaultOrderAndCallableNames();
    testSharedRuleShapes();

    std::printf("=== %d checks, %d failure(s) ===\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
