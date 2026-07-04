/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        tests/sm/SMModelTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-01 unit tests: FSML in-memory document model construction, name-based
 *               lookups, the shared stimulus name space, document-order preservation, and
 *               document-wide monotonic, collision-free ID allocation (including notes).
 *
 *  This is a self-contained test program (no external test framework) so it introduces no
 *  new dependency. It builds a hierarchical machine mirroring the structure of the
 *  `TrafficLight.fsml` reference document and asserts the SM-01 acceptance criteria.
 *
 ************************************************************************/

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"

#include <QSet>
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
// ID-collection walk (verifies uniqueness across every section, incl. notes)
//////////////////////////////////////////////////////////////////////////

namespace
{
    void addId(QList<uint32_t>& ids, uint32_t id)
    {
        ids.append(id);
    }

    void collectArguments(QList<uint32_t>& ids, const QList<SMArgumentEntry>& args)
    {
        for (const SMArgumentEntry& arg : args)
        {
            addId(ids, arg.getId());
        }
    }

    void collectOperations(QList<uint32_t>& ids, const SMOperationList& ops)
    {
        for (const SMOperationBase* op : ops.getOperations())
        {
            addId(ids, op->getId());
            if (op->getOperationType() == SMOperationBase::eOperation::ActionCall)
            {
                collectArguments(ids, static_cast<const SMActionCall*>(op)->getArguments());
            }
            else if (op->getOperationType() == SMOperationBase::eOperation::EventSend)
            {
                collectArguments(ids, static_cast<const SMEventSend*>(op)->getArguments());
            }
        }
    }

    void collectStates(QList<uint32_t>& ids, const SMStateData& states)
    {
        for (const SMStateEntry* state : states.getElements())
        {
            addId(ids, state->getId());
            collectOperations(ids, state->getEntryList());
            collectOperations(ids, state->getExitList());

            for (const SMTransitionEntry* tr : state->getTransitions().getElements())
            {
                addId(ids, tr->getId());
                for (const SMConditionEntry* cond : tr->getConditions().getElements())
                {
                    addId(ids, cond->getId());
                    collectArguments(ids, cond->getArguments());
                }

                collectOperations(ids, tr->getOperations());
            }

            if (state->hasNestedStates())
            {
                collectStates(ids, *state->getNestedStates());
            }
        }
    }

    QList<uint32_t> collectAllIds(const StateMachineData& doc)
    {
        QList<uint32_t> ids;
        addId(ids, doc.getOverview().getId());

        for (const DataTypeCustom* dt : doc.getDataTypes().getCustomDataTypes())
        {
            addId(ids, dt->getId());
        }

        for (const SMAttributeEntry& a : doc.getAttributes().getElements())
        {
            addId(ids, a.getId());
        }

        for (const SMEventEntry* e : doc.getEvents().getElements())
        {
            addId(ids, e->getId());
            for (const MethodParameter& p : e->getElements())
            {
                addId(ids, p.getId());
            }
        }

        for (const SMTimerEntry& t : doc.getTimers().getElements())
        {
            addId(ids, t.getId());
        }

        for (const SMMethodEntry* m : doc.getMethods().getElements())
        {
            addId(ids, m->getId());
            for (const MethodParameter& p : m->getElements())
            {
                addId(ids, p.getId());
            }
        }

        for (const ConstantEntry& c : doc.getConstants().getElements())
        {
            addId(ids, c.getId());
        }

        for (const IncludeEntry& i : doc.getIncludes().getElements())
        {
            addId(ids, i.getId());
        }

        for (const SMImportEntry& im : doc.getImports().getElements())
        {
            addId(ids, im.getId());
        }

        collectStates(ids, doc.getStates());

        for (const SMLayoutNote& note : doc.getLayout().getNotes())
        {
            addId(ids, note.id);
        }

        return ids;
    }
}

//////////////////////////////////////////////////////////////////////////
// Test cases
//////////////////////////////////////////////////////////////////////////

namespace
{
    // Builds a machine mirroring the structure of TrafficLight.fsml: registries, a
    // recursive StateList with internal and external transitions and every operation
    // kind, plus a layout note. Every element type of spec 7.2 is represented.
    void buildTrafficLightLike(StateMachineData& doc)
    {
        SMOverviewData& ov = doc.getOverview();
        ov.setName("TrafficLight");
        ov.setThreading(SMOverviewData::eThreading::Local);
        ov.setDescription("Reference-like traffic light controller.");

        // Data types: one enumeration.
        doc.getDataTypes().addEnum("eDirection");

        // Events, timers.
        doc.getEvents().createEvent("StartTrafficLight");
        doc.getTimers().createTimer("Red");
        doc.getTimers().createTimer("YellowGreen");
        doc.getTimers().createTimer("VehicleWait");

        // Methods: triggers, actions (one with a parameter), a condition.
        doc.getMethods().createMethod("PowerOn", SMMethodEntry::eMethodType::Trigger);
        doc.getMethods().createMethod("PowerOff", SMMethodEntry::eMethodType::Trigger);
        doc.getMethods().createMethod("AllLightsOff", SMMethodEntry::eMethodType::Action);
        SMMethodEntry* setGreen = doc.getMethods().createMethod("SetVehicleGreen", SMMethodEntry::eMethodType::Action);
        setGreen->addParam("direction");
        SMMethodEntry* cond = doc.getMethods().createMethod("IsReady", SMMethodEntry::eMethodType::Condition);
        cond->setImplement(SMMethodEntry::eImplement::Embedded);
        cond->setBody("return true;");

        // Constants, includes, imports.
        doc.getConstants().createConstant("MaxCount");
        doc.getIncludes().createInclude("common/GlobalConst.hpp");
        doc.getImports().createImport("PedestrianCrossing");

        // States: root Start + a composite Normal state with a nested level.
        SMStateData& root = doc.getStates();
        SMStateEntry* lightOff = root.createState("LightOff", SMStateEntry::eStateKind::Start);
        SMStateEntry* lightOn  = root.createState("LightOn", SMStateEntry::eStateKind::Normal);

        // LightOff: entry action + external transition to LightOn.
        SMActionCall* offAction = new SMActionCall();
        offAction->setAction("AllLightsOff");
        lightOff->getEntryList().addOperation(offAction);
        lightOff->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, "PowerOn", "LightOn");

        // LightOn: external transition to LightOff (fires from any nested substate).
        lightOn->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, "PowerOff", "LightOff");

        // LightOn nested level: Start + a Normal state carrying operations and transitions.
        SMStateData* nested = lightOn->getOrCreateNestedStates();
        SMStateEntry* init = nested->createState("Initialize", SMStateEntry::eStateKind::Start);
        SMStateEntry* func = nested->createState("Function", SMStateEntry::eStateKind::Normal);

        // Initialize: EventSend on entry, external transition to Function with a TimerStart.
        SMEventSend* send = new SMEventSend();
        send->setEvent("StartTrafficLight");
        init->getEntryList().addOperation(send);
        SMTransitionEntry* toFunc = init->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, "PowerOn", "Function");
        SMTimerStart* tstart = new SMTimerStart();
        tstart->setTimer("YellowGreen");
        toFunc->getOperations().addOperation(tstart);

        // Function: exit TimerStop; an external transition to Initialize with a condition
        // row; an internal transition (no target) with an ActionCall + mapped argument.
        SMTimerStop* tstop = new SMTimerStop();
        tstop->setTimer("Red");
        func->getExitList().addOperation(tstop);

        SMTransitionEntry* toInit = func->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, "PowerOff", "Initialize");
        SMConditionEntry* row = toInit->getConditions().addCondition();
        row->setLhsKind(SMConditionEntry::eOperandKind::Condition);
        row->setLhs("IsReady");

        SMTransitionEntry* internal = func->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Timer, "VehicleWait");
        SMActionCall* act = new SMActionCall();
        act->setAction("SetVehicleGreen");
        internal->getOperations().addOperation(act);
        act->addArgument("direction", SMArgumentEntry::eValueSource::Value, "EastWest");

        // Layout: a view for the root level and a note attached to the LightOn level.
        doc.getLayout().addView(doc.getOverview().getId());
        SMLayoutNode& node = doc.getLayout().addNode(lightOff->getId());
        node.x = 80; node.y = 140; node.width = 160; node.height = 64;
        SMLayoutNote& note = doc.getLayout().addNote(lightOn->getId());
        note.text = "Shared yellow phase note.";
    }

    void testConstructionAndRepresentation(void)
    {
        std::printf("[Test] construction and representation\n");
        StateMachineData doc;
        buildTrafficLightLike(doc);

        CHECK(doc.getOverview().getName() == QString("TrafficLight"));
        CHECK(doc.getOverview().getThreading() == SMOverviewData::eThreading::Local);
        CHECK(doc.getFormatVersion() == VersionNumber(1, 0, 0));

        CHECK(doc.getDataTypes().getCustomDataTypes().size() == 1);
        CHECK(doc.getEvents().getElementCount() == 1);
        CHECK(doc.getTimers().getElementCount() == 3);
        CHECK(doc.getMethods().getElementCount() == 5);
        CHECK(doc.getConstants().getElementCount() == 1);
        CHECK(doc.getIncludes().getElementCount() == 1);
        CHECK(doc.getImports().getElementCount() == 1);

        // Recursive state structure: 2 root states + 2 nested = 4 total.
        CHECK(doc.getStates().getElementCount() == 2);
        CHECK(doc.getStateCount() == 4);

        const SMStateEntry* lightOn = doc.getStates().findState("LightOn");
        CHECK(lightOn != nullptr);
        CHECK(lightOn->hasNestedStates());
        CHECK(lightOn->getNestedStates()->getElementCount() == 2);

        // Layout note carries a document ID (collision-free requirement includes notes).
        CHECK(doc.getLayout().getNotes().size() == 1);
        CHECK(doc.getLayout().getNotes().first().id != 0);
    }

    void testRegistryLookups(void)
    {
        std::printf("[Test] name-based registry lookups\n");
        StateMachineData doc;
        buildTrafficLightLike(doc);

        CHECK(doc.getEvents().findEvent("StartTrafficLight") != nullptr);
        CHECK(doc.getEvents().findEvent("Nope") == nullptr);
        CHECK(doc.getTimers().findElement(QString("Red")) != nullptr);
        CHECK(doc.getMethods().findMethod("PowerOn") != nullptr);
        CHECK(doc.getMethods().findTrigger("PowerOn") != nullptr);
        CHECK(doc.getMethods().findTrigger("AllLightsOff") == nullptr);  // action, not a trigger
        CHECK(doc.getConstants().findElement(QString("MaxCount")) != nullptr);
        CHECK(doc.getIncludes().findElement(QString("common/GlobalConst.hpp")) != nullptr);
        CHECK(doc.getImports().findElement(QString("PedestrianCrossing")) != nullptr);
        CHECK(doc.getDataTypes().findCustomDataType("eDirection") != nullptr);

        // State lookup is document-wide, including nested levels (spec 6.2.3).
        CHECK(doc.findState("Function") != nullptr);
        CHECK(doc.findState("LightOff") != nullptr);
        CHECK(doc.findState("Ghost") == nullptr);
    }

    void testSharedStimulusNameSpace(void)
    {
        std::printf("[Test] shared stimulus name space\n");
        StateMachineData doc;
        buildTrafficLightLike(doc);

        StateMachineData::StimulusRef trig = doc.findStimulus("PowerOn");
        CHECK(trig.type == StateMachineData::eStimulusType::Trigger);
        CHECK(trig.element != nullptr);

        StateMachineData::StimulusRef evt = doc.findStimulus("StartTrafficLight");
        CHECK(evt.type == StateMachineData::eStimulusType::Event);

        StateMachineData::StimulusRef tmr = doc.findStimulus("Red");
        CHECK(tmr.type == StateMachineData::eStimulusType::Timer);

        // An action method is NOT part of the stimulus name space.
        CHECK(doc.findStimulus("AllLightsOff").type == StateMachineData::eStimulusType::None);
        CHECK(doc.isStimulusName("PowerOn"));
        CHECK(doc.isStimulusName("Unknown") == false);
    }

    void testOrderingInvariants(void)
    {
        std::printf("[Test] document-order preservation\n");
        StateMachineData doc;
        buildTrafficLightLike(doc);

        // States keep insertion (document) order.
        const QList<SMStateEntry*>& roots = doc.getStates().getElements();
        CHECK(roots.size() == 2);
        CHECK(roots.at(0)->getName() == QString("LightOff"));
        CHECK(roots.at(1)->getName() == QString("LightOn"));

        // Timers keep insertion order.
        const QList<SMTimerEntry>& timers = doc.getTimers().getElements();
        CHECK(timers.at(0).getName() == QString("Red"));
        CHECK(timers.at(1).getName() == QString("YellowGreen"));
        CHECK(timers.at(2).getName() == QString("VehicleWait"));

        // Transitions keep insertion order (document order = priority order, spec 6.6.7).
        const SMStateEntry* func = doc.findState("Function");
        CHECK(func != nullptr);
        const QList<SMTransitionEntry*>& trans = func->getTransitions().getElements();
        CHECK(trans.size() == 2);
        CHECK(trans.at(0)->getStimulus() == QString("PowerOff"));   // external, added first
        CHECK(trans.at(1)->getStimulus() == QString("VehicleWait")); // internal, added second
        CHECK(trans.at(0)->isExternal());
        CHECK(trans.at(1)->isExternal() == false);
    }

    void testIdInvariants(void)
    {
        std::printf("[Test] monotonic, collision-free IDs (all sections incl. notes)\n");
        StateMachineData doc;
        buildTrafficLightLike(doc);

        const QList<uint32_t> ids = collectAllIds(doc);
        CHECK(ids.isEmpty() == false);

        QSet<uint32_t> seen;
        bool allNonZero = true;
        bool allUnique = true;
        for (uint32_t id : ids)
        {
            if (id == 0)
            {
                allNonZero = false;
            }

            if (seen.contains(id))
            {
                allUnique = false;
            }

            seen.insert(id);
        }

        CHECK(allNonZero);
        CHECK(allUnique);
        std::printf("       collected %d element IDs, %d unique\n", int(ids.size()), int(seen.size()));

        // A freshly allocated ID must exceed every ID already handed out (monotonic counter).
        uint32_t maxId = 0;
        for (uint32_t id : ids)
        {
            maxId = (id > maxId) ? id : maxId;
        }

        SMTimerEntry* fresh = doc.getTimers().createTimer("BrandNewTimer");
        CHECK(fresh != nullptr);
        CHECK(fresh->getId() > maxId);
    }
}

int main(int /*argc*/, char* /*argv*/[])
{
    std::printf("=== SM-01 FSML in-memory document model tests ===\n");

    testConstructionAndRepresentation();
    testRegistryLookups();
    testSharedStimulusNameSpace();
    testOrderingInvariants();
    testIdInvariants();

    std::printf("=== %d checks, %d failure(s) ===\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
