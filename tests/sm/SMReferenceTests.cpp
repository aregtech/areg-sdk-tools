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
 *  \file        tests/sm/SMReferenceTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-26 unit tests: the single reference walker (where-used completeness) and
 *               the reference-rewrite on rename (round-trip), against a document that
 *               exercises every reference kind (AC2 data-level, AC3).
 *
 ************************************************************************/

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/data/sm/SMReferences.hpp"
#include "lusan/model/sm/SMGoToDef.hpp"

#include <QString>
#include <cstdint>
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
// Reference-exercising document
//////////////////////////////////////////////////////////////////////////

namespace
{
    using eSrc = SMArgumentEntry::eValueSource;
    using eKind = SMTransitionEntry::eStimulusKind;
    using eTarget = SMReferences::eTarget;

    struct Doc
    {
        StateMachineData    data;
        uint32_t            runId   { 0 };  //!< The state that is a transition target.
    };

    // Builds a machine where every reference kind appears at least once:
    //   Trigger "PowerOn"   -> one transition stimulus
    //   Event   "Started"   -> transition stimulus + EventSend + State@OnFinal
    //   Timer   "Tick"      -> transition stimulus + TimerStart + TimerStop
    //   Action  "DoWork"    -> one ActionCall
    //   Attr    "Level"     -> AttributeSet@Attribute + AttributeSet value(Attribute) + Argument value(Attribute)
    //   Const   "MaxCount"  -> one Argument value(Constant)
    //   State   "Run"       -> one transition target (by ID)
    void build(Doc& d)
    {
        StateMachineData& doc = d.data;
        doc.getOverview().setName("Refs");

        doc.getMethods().createMethod("PowerOn", NEMethod::SmTrigger);
        doc.getMethods().createMethod("DoWork",  NEMethod::SmAction);
        doc.getEvents().createEvent("Started");
        doc.getTimers().createTimer("Tick");
        doc.getAttributes().createAttribute("Level");
        doc.getConstants().createConstant("MaxCount");

        SMStateData& root = doc.getStates();
        SMStateEntry* idle = root.createState("Idle", SMStateEntry::eStateKind::Start);
        SMStateEntry* run  = root.createState("Run",  SMStateEntry::eStateKind::Normal);
        d.runId = run->getId();

        // Idle: OnFinal event; entry EventSend(arg=Attribute) + ActionCall(arg=Constant).
        idle->setOnFinal("Started");

        SMEventSend* send = new SMEventSend();
        send->setEvent("Started");
        send->addArgument("p", eSrc::Attribute, "Level");
        idle->getEntryList().addOperation(send);

        SMActionCall* call = new SMActionCall();
        call->setAction("DoWork");
        call->addArgument("n", eSrc::Constant, "MaxCount");
        idle->getEntryList().addOperation(call);

        // Idle -> Run on Trigger PowerOn, with TimerStart + AttributeSet(source=Attribute).
        SMTransitionEntry* toRun = idle->getTransitions().createTransition(eKind::Trigger, "PowerOn", run->getId());
        SMTimerStart* tstart = new SMTimerStart();
        tstart->setTimer("Tick");
        toRun->getOperations().addOperation(tstart);
        SMAttributeSet* aset = new SMAttributeSet();
        aset->setAttribute("Level");
        aset->setSource(eSrc::Attribute);
        aset->setValue("Level");
        toRun->getOperations().addOperation(aset);

        // Run: internal transitions on Event and Timer, exit TimerStop.
        run->getTransitions().createTransition(eKind::Event, "Started", 0u, SMTransitionEntry::eTransitionKind::Internal);
        run->getTransitions().createTransition(eKind::Timer, "Tick", 0u, SMTransitionEntry::eTransitionKind::Internal);
        SMTimerStop* tstop = new SMTimerStop();
        tstop->setTimer("Tick");
        run->getExitList().addOperation(tstop);
    }

    int uses(const StateMachineData& doc, eTarget target, const QString& name, uint32_t id)
    {
        return SMReferences::whereUsed(doc, target, name, id).size();
    }

    //!< The ID of a state's first (highest-priority) transition, or 0 if it has none.
    uint32_t firstTransitionId(const StateMachineData& doc, const QString& stateName)
    {
        const SMStateEntry* state = doc.getStates().findState(stateName);
        if ((state == nullptr) || state->getTransitions().getElements().isEmpty())
            return 0u;
        return state->getTransitions().getElements().first()->getId();
    }

    //!< The number of inverse-walker references matching (target, name).
    int refCount(const QList<SMReferences::Ref>& refs, eTarget target, const QString& name)
    {
        int count = 0;
        for (const SMReferences::Ref& ref : refs)
            count += ((ref.target == target) && (ref.name == name)) ? 1 : 0;
        return count;
    }

    //!< The number of go-to-declaration targets matching (kind, name).
    int defCount(const QList<SMGoToDef::Target>& targets, eTarget kind, const QString& name)
    {
        int count = 0;
        for (const SMGoToDef::Target& target : targets)
            count += ((target.kind == kind) && (target.name == name)) ? 1 : 0;
        return count;
    }
}

//////////////////////////////////////////////////////////////////////////
// Test cases
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testWhereUsedCompleteness()
    {
        std::printf("[Test] where-used completeness (every reference kind)\n");
        Doc d;
        build(d);
        const StateMachineData& doc = d.data;

        CHECK(uses(doc, eTarget::Trigger,   "PowerOn",  0) == 1);
        CHECK(uses(doc, eTarget::Action,    "DoWork",   0) == 1);
        CHECK(uses(doc, eTarget::Event,     "Started",  0) == 3); // stimulus + EventSend + OnFinal
        CHECK(uses(doc, eTarget::Timer,     "Tick",     0) == 3); // stimulus + TimerStart + TimerStop
        CHECK(uses(doc, eTarget::Attribute, "Level",    0) == 3); // AttributeSet target + value + argument
        CHECK(uses(doc, eTarget::Constant,  "MaxCount", 0) == 1); // one argument
        CHECK(uses(doc, eTarget::State,     QString(),  d.runId) == 1); // one transition target

        // A name that matches nothing yields nothing; a wrong-kind lookup does too.
        CHECK(uses(doc, eTarget::Trigger,   "Started",  0) == 0);
        CHECK(uses(doc, eTarget::Timer,     "PowerOn",  0) == 0);
    }

    void testNavigationTargets()
    {
        std::printf("[Test] where-used navigation targets\n");
        Doc d;
        build(d);
        const StateMachineData& doc = d.data;

        // The trigger use points at a transition; the OnFinal use points at a state.
        bool sawTransition = false;
        for (const SMReferences::Use& u : SMReferences::whereUsed(doc, eTarget::Trigger, "PowerOn", 0))
            sawTransition = sawTransition || (u.isState == false);
        CHECK(sawTransition);

        bool sawStateOnFinal = false;
        for (const SMReferences::Use& u : SMReferences::whereUsed(doc, eTarget::Event, "Started", 0))
            sawStateOnFinal = sawStateOnFinal || (u.isState && (u.location.contains("OnFinal")));
        CHECK(sawStateOnFinal);
    }

    void testRewriteRoundTrip()
    {
        std::printf("[Test] rename rewrites references and round-trips\n");
        Doc d;
        build(d);
        StateMachineData& doc = d.data;

        // Timer: three references move together; the old name then resolves to nothing.
        CHECK(SMReferences::rewriteReferences(doc, eTarget::Timer, "Tick", "Tock") == 3);
        CHECK(uses(doc, eTarget::Timer, "Tick", 0) == 0);
        CHECK(uses(doc, eTarget::Timer, "Tock", 0) == 3);
        // Undo direction restores every site.
        CHECK(SMReferences::rewriteReferences(doc, eTarget::Timer, "Tock", "Tick") == 3);
        CHECK(uses(doc, eTarget::Timer, "Tick", 0) == 3);

        // Event and attribute rewrites cover their multi-field reference sets.
        CHECK(SMReferences::rewriteReferences(doc, eTarget::Event,     "Started", "Begin") == 3);
        CHECK(uses(doc, eTarget::Event, "Begin", 0) == 3);
        CHECK(SMReferences::rewriteReferences(doc, eTarget::Attribute, "Level",   "Depth") == 3);
        CHECK(uses(doc, eTarget::Attribute, "Depth", 0) == 3);

        // A state rename never rewrites a name field: the target is ID-based, so the count
        // is zero and the transition still resolves to the state by ID.
        CHECK(SMReferences::rewriteReferences(doc, eTarget::State, "Run", "Runner") == 0);
        CHECK(uses(doc, eTarget::State, QString(), d.runId) == 1);
    }

    void testCrossKindIsolation()
    {
        std::printf("[Test] rename does not touch a same-named different kind\n");
        Doc d;
        build(d);
        StateMachineData& doc = d.data;

        // Give an action the same name as the trigger, referenced by an ActionCall, then
        // rename only the trigger: the action's ActionCall must stay put.
        doc.getMethods().createMethod("Shared", NEMethod::SmAction);
        SMStateEntry* run = doc.getStates().findState("Run");
        SMActionCall* call = new SMActionCall();
        call->setAction("Shared");
        run->getEntryList().addOperation(call);

        // A trigger named "Shared" used by a transition stimulus.
        doc.getMethods().createMethod("Shared", NEMethod::SmTrigger);
        run->getTransitions().createTransition(eKind::Trigger, "Shared", 0u, SMTransitionEntry::eTransitionKind::Internal);

        CHECK(uses(doc, eTarget::Action,  "Shared", 0) == 1);
        CHECK(uses(doc, eTarget::Trigger, "Shared", 0) == 1);

        CHECK(SMReferences::rewriteReferences(doc, eTarget::Trigger, "Shared", "Fired") == 1);
        CHECK(uses(doc, eTarget::Trigger, "Fired", 0) == 1);
        CHECK(uses(doc, eTarget::Action, "Shared", 0) == 1); // untouched
    }

    void testDefinitionsOf()
    {
        std::printf("[Test] definitions-of (inverse walker) lists an element's references\n");
        Doc d;
        build(d);
        const StateMachineData& doc = d.data;

        // The Idle state's entry operations + OnFinal reference an event (twice: EventSend and
        // OnFinal), an action, an attribute (an argument), and a constant (an argument).
        const SMStateEntry* idle = doc.getStates().findState("Idle");
        CHECK(idle != nullptr);
        const QList<SMReferences::Ref> stateRefs = SMReferences::definitionsOf(doc, idle->getId(), true);
        CHECK(refCount(stateRefs, eTarget::Event,     "Started")  == 2);
        CHECK(refCount(stateRefs, eTarget::Action,    "DoWork")   == 1);
        CHECK(refCount(stateRefs, eTarget::Attribute, "Level")    == 1);
        CHECK(refCount(stateRefs, eTarget::Constant,  "MaxCount") == 1);

        // The Idle->Run transition references its trigger stimulus, a timer, and an attribute
        // (AttributeSet target + value); its target state is by ID -- navigation, not a
        // declaration -- so the inverse walker omits it.
        const uint32_t toRun = firstTransitionId(doc, "Idle");
        const QList<SMReferences::Ref> trRefs = SMReferences::definitionsOf(doc, toRun, false);
        CHECK(refCount(trRefs, eTarget::Trigger,   "PowerOn") == 1);
        CHECK(refCount(trRefs, eTarget::Timer,     "Tick")    == 1);
        CHECK(refCount(trRefs, eTarget::Attribute, "Level")   == 2);
        CHECK(refCount(trRefs, eTarget::State,     "Run")     == 0);
    }

    void testGoToDefResolution()
    {
        std::printf("[Test] go-to-declaration resolves and de-duplicates targets\n");
        Doc d;
        build(d);
        StateMachineData& doc = d.data;

        // Transition: PowerOn (trigger), Tick (timer), Level (attribute, referenced twice)
        // collapse to three navigable declarations, each resolved to a non-zero declaration ID.
        const uint32_t toRun = firstTransitionId(doc, "Idle");
        const QList<SMGoToDef::Target> trTargets = SMGoToDef::collect(doc, toRun, false);
        CHECK(trTargets.size() == 3);
        CHECK(defCount(trTargets, eTarget::Trigger,   "PowerOn") == 1);
        CHECK(defCount(trTargets, eTarget::Timer,     "Tick")    == 1);
        CHECK(defCount(trTargets, eTarget::Attribute, "Level")   == 1); // de-duplicated
        bool allResolved = true;
        for (const SMGoToDef::Target& target : trTargets)
            allResolved = allResolved && (target.declId != 0u);
        CHECK(allResolved);

        // State: Started (event, referenced twice) de-duplicates; DoWork/Level/MaxCount resolve.
        const SMStateEntry* idle = doc.getStates().findState("Idle");
        const QList<SMGoToDef::Target> stTargets = SMGoToDef::collect(doc, idle->getId(), true);
        CHECK(stTargets.size() == 4);
        CHECK(defCount(stTargets, eTarget::Event,     "Started")  == 1); // de-duplicated
        CHECK(defCount(stTargets, eTarget::Action,    "DoWork")   == 1);
        CHECK(defCount(stTargets, eTarget::Attribute, "Level")    == 1);
        CHECK(defCount(stTargets, eTarget::Constant,  "MaxCount") == 1);

        // A dangling reference (a name that resolves to no declaration) is skipped -- it has
        // nowhere to navigate. Add an ActionCall to an undeclared method on the Run state.
        SMStateEntry* run = doc.getStates().findState("Run");
        SMActionCall* ghost = new SMActionCall();
        ghost->setAction("Ghost");
        run->getExitList().addOperation(ghost);
        const QList<SMGoToDef::Target> ghostTargets = SMGoToDef::collect(doc, run->getId(), true);
        CHECK(defCount(ghostTargets, eTarget::Action, "Ghost") == 0);
    }
}

//////////////////////////////////////////////////////////////////////////
// Entry point
//////////////////////////////////////////////////////////////////////////

int main(int, char**)
{
    std::printf("SM-26 reference walker tests\n");
    testWhereUsedCompleteness();
    testNavigationTargets();
    testRewriteRoundTrip();
    testCrossKindIsolation();
    testDefinitionsOf();
    testGoToDefResolution();

    std::printf("\n%d checks, %d failure(s)\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
