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
 *  \file        tests/sm/SMCommandTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-04 unit tests: the undo/redo command framework. Verifies that scripted
 *               command sequences undo/redo to byte-identical model states (deep compare via
 *               serialization, so element IDs are compared too), that redo restores original
 *               IDs with no duplicates, that a composite delete is a single step, that layout
 *               drags coalesce, and that 100+ step histories navigate without corruption.
 *               Also covers SM-07 (SMDataTypeModel): the full data-type/field lifecycle
 *               (create/insert/rename/convert/reorder/delete) through the same undo stack.
 *               SM-07-02 extends this with container data types: basic-container object,
 *               key and value type mutations, keyed/non-keyed switching, through the same
 *               undo stack.
 *
 *  Self-contained (no external test framework), matching SMModelTests.cpp.
 *
 ************************************************************************/

#include "lusan/data/sm/SMClipboard.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/sm/SMPasteCommand.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/model/sm/SMDataTypeModel.hpp"
#include "lusan/model/sm/SMEventModel.hpp"
#include "lusan/model/sm/SMTimerModel.hpp"
#include "lusan/model/sm/SMIncludeModel.hpp"
#include "lusan/model/common/ConstantModel.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/sm/SMDocumentCache.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QUndoStack>
#include <QXmlStreamWriter>
#include <QString>
#include <QStringList>
#include <QList>
#include <QSet>
#include <cstdio>
#include <memory>

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

    //!< Deep comparison of two documents: the serialized XML is identical only when every
    //!< element, attribute and ID matches.
    QString serialize(const StateMachineData& doc)
    {
        QString out;
        QXmlStreamWriter writer(&out);
        writer.setAutoFormatting(true);
        doc.writeToXml(writer);
        return out;
    }
}

#define CHECK(cond)  check((cond), #cond)

//////////////////////////////////////////////////////////////////////////
// Scenario A: scripted mixed sequence, full undo/redo round-trip per step
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testScriptedSequence()
    {
        StateMachineData    doc;
        DocModelNotifier    notifier;
        QUndoStack          stack;

        int added = 0, removed = 0, changed = 0, reordered = 0, layout = 0;
        QObject::connect(&notifier, &DocModelNotifier::elementAdded,   [&](uint32_t, eDocElementKind){ ++added; });
        QObject::connect(&notifier, &DocModelNotifier::elementRemoved, [&](uint32_t, eDocElementKind){ ++removed; });
        QObject::connect(&notifier, &DocModelNotifier::elementChanged, [&](uint32_t, eDocElementKind){ ++changed; });
        QObject::connect(&notifier, &DocModelNotifier::listReordered,  [&](uint32_t, eDocElementKind){ ++reordered; });
        QObject::connect(&notifier, &DocModelNotifier::layoutChanged,  [&](const QList<uint32_t>&){ ++layout; });

        // Warm up structural IDs so serialization is stable across the whole test.
        doc.getOverview().getId();
        doc.getTimers().getId();
        doc.getStates().getId();
        doc.getLayout().getId();

        QStringList checkpoints;
        checkpoints << serialize(doc);
        auto pushAndSnap = [&](QUndoCommand* command)
        {
            stack.push(command);
            checkpoints << serialize(doc);
        };

        pushAndSnap(new SMAddStateCommand(notifier, doc.getStates(), "Idle", SMStateEntry::eStateKind::Start,  "Add Idle"));
        pushAndSnap(new SMAddStateCommand(notifier, doc.getStates(), "Run",  SMStateEntry::eStateKind::Normal, "Add Run"));
        pushAndSnap(new SMAddStateCommand(notifier, doc.getStates(), "Stop", SMStateEntry::eStateKind::Normal, "Add Stop"));

        pushAndSnap(new TDocAddCommand<SMTimerEntry, DocumentElem>(notifier, doc.getTimers(), SMTimerEntry(0, "T1", 100, 1, &doc.getTimers()), eDocElementKind::Timer, "Add T1"));
        pushAndSnap(new TDocAddCommand<SMTimerEntry, DocumentElem>(notifier, doc.getTimers(), SMTimerEntry(0, "T2", 200, 1, &doc.getTimers()), eDocElementKind::Timer, "Add T2"));

        {
            const uint32_t overviewId = doc.getOverview().getId();
            auto getter = [&doc]() -> QString { return doc.getOverview().getName(); };
            auto setter = [&doc](const QString& value) { doc.getOverview().setName(value); };
            pushAndSnap(new TDocSetPropertyCommand<QString>(notifier, overviewId, eDocElementKind::Overview, getter, setter, QString("Machine2"), "Rename machine"));
        }

        pushAndSnap(new TDocReorderCommand<SMTimerEntry, DocumentElem>(notifier, doc.getTimers(), 0, 1, doc.getTimers().getId(), eDocElementKind::Timer, "Reorder timers"));

        {
            const uint32_t stopId = doc.getStates().findState("Stop")->getId();
            pushAndSnap(new SMRemoveStateCommand(doc, notifier, doc.getStates(), stopId, "Delete Stop"));
        }

        const int steps = static_cast<int>(checkpoints.size()) - 1;
        const QString built = serialize(doc);

        // Undo every step and confirm the model matches the recorded prior state exactly.
        bool undoExact = true;
        for (int k = steps; k >= 1; --k)
        {
            stack.undo();
            undoExact = undoExact && (serialize(doc) == checkpoints[k - 1]);
        }
        CHECK(undoExact);
        CHECK(serialize(doc) == checkpoints[0]);

        // Redo every step and confirm each state (including IDs) is restored exactly.
        bool redoExact = true;
        for (int k = 1; k <= steps; ++k)
        {
            stack.redo();
            redoExact = redoExact && (serialize(doc) == checkpoints[k]);
        }
        CHECK(redoExact);
        CHECK(serialize(doc) == built);

        CHECK(added > 0);
        CHECK(removed > 0);
        CHECK(changed > 0);
        CHECK(reordered > 0);
        CHECK(layout > 0);
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario B: coalesced layout drag
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testCoalescedLayoutDrag()
    {
        StateMachineData    doc;
        DocModelNotifier    notifier;
        QUndoStack          stack;

        stack.push(new SMAddStateCommand(notifier, doc.getStates(), "S", SMStateEntry::eStateKind::Start, "Add S"));
        const uint32_t stateId = doc.getStates().findState("S")->getId();

        SMLayoutNode& node = doc.getLayout().addNode(stateId);
        node.x = 1.0; node.y = 2.0; node.width = 10.0; node.height = 20.0;

        const QString atOrigin = serialize(doc);

        const uint32_t gesture = 777u;
        stack.push(new SMMoveNodeCommand(doc, notifier, stateId, gesture, 5.0, 6.0, 10.0, 20.0, "Move S"));
        stack.push(new SMMoveNodeCommand(doc, notifier, stateId, gesture, 7.0, 8.0, 10.0, 20.0, "Move S"));
        stack.push(new SMMoveNodeCommand(doc, notifier, stateId, gesture, 9.0, 9.0, 10.0, 20.0, "Move S"));

        // Add-state + one coalesced move gesture = 2 undo steps, not 4.
        CHECK(stack.count() == 2);

        const SMLayoutNode* moved = doc.getLayout().findNode(stateId);
        CHECK((moved != nullptr) && (moved->x == 9.0) && (moved->y == 9.0));

        stack.undo();   // reverts the whole drag in one step
        const SMLayoutNode* reverted = doc.getLayout().findNode(stateId);
        CHECK((reverted != nullptr) && (reverted->x == 1.0) && (reverted->y == 2.0));
        CHECK(serialize(doc) == atOrigin);

        stack.redo();
        const SMLayoutNode* redone = doc.getLayout().findNode(stateId);
        CHECK((redone != nullptr) && (redone->x == 9.0) && (redone->y == 9.0));
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario C: composite delete of a state with substate, transition and layout
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testCompositeDelete()
    {
        StateMachineData    doc;
        DocModelNotifier    notifier;
        QUndoStack          stack;

        SMStateEntry* parent = doc.getStates().createState("Parent", SMStateEntry::eStateKind::Normal);
        SMStateData*  nested = parent->getOrCreateNestedStates();
        nested->createState("Child", SMStateEntry::eStateKind::Start);
        parent->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Timer, "T", 0u, SMTransitionEntry::eTransitionKind::Internal);
        const uint32_t parentId = parent->getId();
        doc.getLayout().addNode(parentId).x = 3.0;

        const QString before = serialize(doc);
        const int indexBefore = stack.index();

        stack.push(new SMRemoveStateCommand(doc, notifier, doc.getStates(), parentId, "Delete Parent"));

        CHECK(stack.index() == indexBefore + 1);                    // one undo step
        CHECK(doc.getStates().findState("Parent") == nullptr);      // subtree gone
        CHECK(doc.getLayout().findNode(parentId) == nullptr);       // layout gone with it

        stack.undo();
        CHECK(serialize(doc) == before);                            // subtree + layout restored, IDs intact
        CHECK(doc.getStates().findState("Parent") != nullptr);

        stack.redo();
        CHECK(doc.getStates().findState("Parent") == nullptr);
    }

    //!< Renaming a state must keep every transition connected to it. Transitions reference their
    //!< target by ID, so the connection is intrinsically rename-proof: the target ID never changes,
    //!< and getTargetName() resolves the target's current name live (reflecting the rename).
    void testRenameKeepsTransitions()
    {
        StateMachineData    doc;
        DocModelNotifier    notifier;
        QUndoStack          stack;

        SMStateEntry* start  = doc.getStates().createState("Start", SMStateEntry::eStateKind::Start);
        SMStateEntry* worker = doc.getStates().createState("Worker", SMStateEntry::eStateKind::Normal);
        SMStateEntry* other  = doc.getStates().createState("Other", SMStateEntry::eStateKind::Normal);
        CHECK((start != nullptr) && (worker != nullptr) && (other != nullptr));

        // Start -> Worker, and a self-referencing loop Worker -> Worker; Other -> itself stays put.
        SMTransitionEntry* toWorker = start->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Timer, QString(), worker->getId(), SMTransitionEntry::eTransitionKind::Initial);
        SMTransitionEntry* loop     = worker->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Timer, "L", worker->getId());
        SMTransitionEntry* toOther  = other->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Timer, "O", other->getId());
        CHECK((toWorker != nullptr) && (loop != nullptr) && (toOther != nullptr));

        // The target IDs before the rename; they must be unchanged by it.
        const uint32_t workerId = worker->getId();
        const uint32_t otherId  = other->getId();

        stack.push(new SMRenameStateCommand(doc, notifier, worker->getId(), "Machine", "Rename"));

        // Renaming Worker to Machine leaves every target ID intact; the resolved names follow the rename.
        CHECK(worker->getName() == QStringLiteral("Machine"));
        CHECK((toWorker->getToId() == workerId) && (loop->getToId() == workerId) && (toOther->getToId() == otherId));
        CHECK(toWorker->getTargetName() == QStringLiteral("Machine"));
        CHECK(loop->getTargetName()     == QStringLiteral("Machine"));
        CHECK(toOther->getTargetName()  == QStringLiteral("Other"));

        stack.undo();
        CHECK(worker->getName() == QStringLiteral("Worker"));
        CHECK((toWorker->getToId() == workerId) && (loop->getToId() == workerId)); // connection intact through undo
        CHECK(toWorker->getTargetName() == QStringLiteral("Worker"));
        CHECK(loop->getTargetName()     == QStringLiteral("Worker"));
        CHECK(toOther->getTargetName()  == QStringLiteral("Other"));

        stack.redo();
        CHECK(toWorker->getTargetName() == QStringLiteral("Machine"));  // and reapplied on redo
        CHECK(loop->getTargetName()     == QStringLiteral("Machine"));
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario D: deep history (100+ steps) and ID uniqueness
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testDeepHistory()
    {
        StateMachineData    doc;
        DocModelNotifier    notifier;
        QUndoStack          stack;

        const int count = 120;
        for (int i = 0; i < count; ++i)
        {
            const SMStateEntry::eStateKind kind = (i == 0 ? SMStateEntry::eStateKind::Start : SMStateEntry::eStateKind::Normal);
            stack.push(new SMAddStateCommand(notifier, doc.getStates(), QString("St%1").arg(i), kind, "Add"));
        }

        CHECK(stack.count() == count);
        const QString full = serialize(doc);

        for (int i = 0; i < count; ++i)
        {
            stack.undo();
        }
        CHECK(doc.getStates().getElementCount() == 0);

        for (int i = 0; i < count; ++i)
        {
            stack.redo();
        }
        CHECK(doc.getStates().getElementCount() == count);
        CHECK(serialize(doc) == full);

        QList<uint32_t> ids;
        for (const SMStateEntry* state : doc.getStates().getElements())
        {
            ids.append(state->getId());
        }
        const QSet<uint32_t> unique(ids.begin(), ids.end());
        CHECK(unique.size() == ids.size());
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario E: SM-07 Data Types page model -- enumeration/structure/imported
// lifecycle through SMDataTypeModel exactly as SMDataType (the view) drives it,
// undo/redo round-trip, and a category conversion composite.
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testDataTypeLifecycle()
    {
        StateMachineModel model;
        SMDataTypeModel&  dt = model.getDataTypeModel();

        QStringList checkpoints;
        checkpoints << serialize(model.getData());
        auto snap = [&]() { checkpoints << serialize(model.getData()); };

        DataTypeCustom* point = dt.createDataType("Point", DataTypeBase::eCategory::Structure);
        CHECK(point != nullptr);
        snap();

        ElementBase* fx = dt.createField(point, "x");
        CHECK(fx != nullptr);
        const uint32_t fxId = fx->getId();
        snap();
        ElementBase* fy = dt.createField(point, "y");
        CHECK(fy != nullptr);
        const uint32_t fyId = fy->getId();
        snap();

        dt.setFieldType(static_cast<DataTypeStructure*>(point), fxId, QStringLiteral("uint32"));
        snap();
        dt.setFieldValue(point, fxId, "1");
        snap();
        dt.setDescription(point, "A 2D point");
        snap();
        dt.setDeprecated(point, true);
        snap();
        dt.setDeprecateHint(point, "use Point2D instead");
        snap();

        DataTypeCustom* color = dt.createDataType("Color", DataTypeBase::eCategory::Enumeration);
        CHECK(color != nullptr);
        snap();
        ElementBase* red = dt.createField(color, "Red");
        CHECK(red != nullptr);
        const uint32_t redId = red->getId();
        snap();
        dt.setFieldValue(color, redId, "0");
        snap();
        dt.setEnumDerived(static_cast<DataTypeEnum*>(color), "uint16");
        snap();

        DataTypeCustom* foo = dt.createDataType("Foo", DataTypeBase::eCategory::Imported);
        CHECK(foo != nullptr);
        snap();
        dt.setImportLocation(static_cast<DataTypeImported*>(foo), "foo.hpp");
        snap();
        dt.setImportNamespace(static_cast<DataTypeImported*>(foo), "ns");
        snap();
        dt.setImportObject(static_cast<DataTypeImported*>(foo), "Foo");
        snap();

        dt.renameDataType(point, "Point2D");
        CHECK(point->getName() == QStringLiteral("Point2D"));
        snap();

        dt.swapDataTypes(point->getId(), color->getId());
        CHECK(dt.findIndex(point) == 1);
        snap();

        DataTypeCustom* converted = dt.convertDataType(color, DataTypeBase::eCategory::Structure);
        CHECK((converted != nullptr) && (converted->getCategory() == DataTypeBase::eCategory::Structure));
        CHECK(converted->getName() == QStringLiteral("Color"));
        snap();

        dt.deleteField(point, fyId);
        CHECK(dt.getChildCount(point) == 1);
        snap();

        dt.deleteDataType(foo);
        CHECK(dt.findDataType(QStringLiteral("Foo")) == nullptr);
        snap();

        DataTypeCustom* first = dt.insertDataType(0, "First", DataTypeBase::eCategory::Structure);
        CHECK((first != nullptr) && (dt.findIndex(first) == 0));
        snap();

        const int steps = static_cast<int>(checkpoints.size()) - 1;
        const QString built = serialize(model.getData());

        bool undoExact = true;
        for (int k = steps; k >= 1; --k)
        {
            model.getUndoStack().undo();
            undoExact = undoExact && (serialize(model.getData()) == checkpoints[k - 1]);
        }
        CHECK(undoExact);
        CHECK(serialize(model.getData()) == checkpoints[0]);
        CHECK(dt.getDataTypeCount() == 0);

        bool redoExact = true;
        for (int k = 1; k <= steps; ++k)
        {
            model.getUndoStack().redo();
            redoExact = redoExact && (serialize(model.getData()) == checkpoints[k]);
        }
        CHECK(redoExact);
        CHECK(serialize(model.getData()) == built);
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario F (SM-07-02): container data types -- direct creation and conversion-from-
// existing both seed sensible defaults, switching basic container enables/disables and
// clears the key, key/value are set by name through SMDataTypeModel, undo/redo round-trip.
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testContainerLifecycle()
    {
        StateMachineModel model;
        SMDataTypeModel&  dt = model.getDataTypeModel();

        QStringList checkpoints;
        checkpoints << serialize(model.getData());
        auto snap = [&]() { checkpoints << serialize(model.getData()); };

        DataTypeCustom* itemType = dt.createDataType("Item", DataTypeBase::eCategory::Structure);
        CHECK(itemType != nullptr);
        snap();

        // Direct creation seeds the default basic container ("Array") and value ("bool").
        DataTypeCustom* listType = dt.createDataType("List", DataTypeBase::eCategory::Container);
        CHECK(listType != nullptr);
        DataTypeContainer* list = static_cast<DataTypeContainer*>(listType);
        CHECK(list->getContainer() == QStringLiteral("Array"));
        CHECK(list->getValue() == QStringLiteral("bool"));
        CHECK(list->canHaveKey() == false);
        snap();

        dt.setContainerValue(list, QStringLiteral("Item"));
        CHECK(list->getValue() == QStringLiteral("Item"));
        snap();

        // Switching to a keyed basic container enables the key with a non-empty default.
        dt.setContainerObject(list, QStringLiteral("HashMap"));
        CHECK(list->getContainer() == QStringLiteral("HashMap"));
        CHECK(list->canHaveKey());
        CHECK(list->getKey().isEmpty() == false);
        snap();

        dt.setContainerKey(list, QStringLiteral("uint32"));
        CHECK(list->getKey() == QStringLiteral("uint32"));
        snap();

        // Switching back to a non-keyed basic container clears the key.
        dt.setContainerObject(list, QStringLiteral("Array"));
        CHECK(list->canHaveKey() == false);
        CHECK(list->getKey().isEmpty());
        snap();

        // Conversion-from-existing (Structure -> Container) also seeds sensible defaults.
        DataTypeCustom* converted = dt.convertDataType(itemType, DataTypeBase::eCategory::Container);
        CHECK((converted != nullptr) && (converted->getCategory() == DataTypeBase::eCategory::Container));
        DataTypeContainer* dict = static_cast<DataTypeContainer*>(converted);
        CHECK(dict->getContainer() == QStringLiteral("Array"));
        snap();

        dt.setContainerObject(dict, QStringLiteral("Map"));
        snap();
        dt.setContainerKey(dict, QStringLiteral("int32"));
        snap();
        dt.setContainerValue(dict, QStringLiteral("List"));
        CHECK(dict->getValue() == QStringLiteral("List"));
        snap();

        const int steps = static_cast<int>(checkpoints.size()) - 1;
        const QString built = serialize(model.getData());

        bool undoExact = true;
        for (int k = steps; k >= 1; --k)
        {
            model.getUndoStack().undo();
            undoExact = undoExact && (serialize(model.getData()) == checkpoints[k - 1]);
        }
        CHECK(undoExact);
        CHECK(serialize(model.getData()) == checkpoints[0]);
        CHECK(dt.getDataTypeCount() == 0);

        bool redoExact = true;
        for (int k = 1; k <= steps; ++k)
        {
            model.getUndoStack().redo();
            redoExact = redoExact && (serialize(model.getData()) == checkpoints[k]);
        }
        CHECK(redoExact);
        CHECK(serialize(model.getData()) == built);
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario G (SM-09): Events page model (SMEventModel) -- event and payload-parameter
// lifecycle, and Timers page model (SMTimerModel) -- timer lifecycle, both through the same
// undo stack, plus the shared stimulus name-space collision check (StateMachineData::
// findStimulus) that the pages use for live "name already used" feedback.
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testEventTimerLifecycle()
    {
        StateMachineModel model;
        SMEventModel&     ev = model.getEventModel();
        SMTimerModel&     tm = model.getTimerModel();

        QStringList checkpoints;
        checkpoints << serialize(model.getData());
        auto snap = [&]() { checkpoints << serialize(model.getData()); };

        SMEventEntry* started = ev.createEvent("Started");
        CHECK(started != nullptr);
        snap();

        MethodParameter* speed = ev.createParam(started, "speed");
        CHECK(speed != nullptr);
        const uint32_t speedId = speed->getId();
        snap();

        ev.setParamType(started, speedId, "uint32");
        snap();
        ev.setParamDefault(started, speedId, true, "0");
        snap();
        MethodParameter* speedNow = ev.findParam(started, speedId);
        CHECK((speedNow != nullptr) && speedNow->hasDefault() && (speedNow->getValue() == QStringLiteral("0")));

        MethodParameter* code = ev.createParam(started, "code");
        CHECK(code != nullptr);
        const uint32_t codeId = code->getId();
        snap();

        ev.setDescription(started->getId(), "Machine started");
        snap();

        SMEventEntry* stopped = ev.createEvent("Stopped");
        CHECK(stopped != nullptr);
        snap();

        ev.renameEvent(stopped->getId(), "Halted");
        CHECK(ev.findEvent("Halted") == stopped);
        snap();

        ev.swapEvents(started->getId(), stopped->getId());
        CHECK(ev.findIndex(stopped) == 0);
        snap();

        ev.deleteParam(started, codeId);
        CHECK(ev.getParamCount(started) == 1);
        snap();

        // Timers are stored BY VALUE in a QList, and a reorder keeps IDs in list order (swapElements
        // swaps the two entries' data and their IDs, so index N always holds the Nth id). So a held
        // SMTimerEntry* dangles after any append/insert reallocates the buffer, and a captured id does
        // not follow a logical timer across a swap. The only stable handle to a logical timer is its
        // NAME: re-fetch by name at each use and read its current id right before passing it in.
        CHECK(tm.createTimer("T1") != nullptr);
        snap();
        tm.setTimeout(tm.findTimer("T1")->getId(), 500u);
        snap();
        tm.setRepeat(tm.findTimer("T1")->getId(), 3u);
        snap();

        CHECK(tm.createTimer("T2") != nullptr);
        snap();
        // The Continuous checkbox writes Repeat=0 (spec 6.10); 0xFFFFFFFF is also continuous.
        tm.setRepeat(tm.findTimer("T2")->getId(), 0u);
        CHECK(tm.findTimer("T2")->isContinuous());
        snap();
        tm.setDescription(tm.findTimer("T2")->getId(), "Heartbeat");
        snap();

        tm.swapTimers(tm.findTimer("T1")->getId(), tm.findTimer("T2")->getId());
        CHECK(tm.findIndex(tm.findTimer("T2")->getId()) == 0);   // T2 reordered to the front
        snap();

        CHECK(tm.insertTimer(0, "T0") != nullptr);
        CHECK(tm.findIndex(tm.findTimer("T0")->getId()) == 0);
        snap();

        tm.deleteTimer(tm.findTimer("T1")->getId());
        CHECK(tm.findTimer("T1") == nullptr);
        snap();

        // Shared stimulus name space (spec 6.10): an event and a timer must not share a name.
        CHECK(model.getData().isStimulusName("Halted"));
        CHECK(model.getData().isStimulusName("T2"));
        CHECK(model.getData().isStimulusName("NoSuchStimulus") == false);
        const StateMachineData::StimulusRef eventRef = model.getData().findStimulus("Halted");
        CHECK(eventRef.type == StateMachineData::eStimulusType::Event);
        const StateMachineData::StimulusRef timerRef = model.getData().findStimulus("T2");
        CHECK(timerRef.type == StateMachineData::eStimulusType::Timer);

        const int steps = static_cast<int>(checkpoints.size()) - 1;
        const QString built = serialize(model.getData());

        bool undoExact = true;
        for (int k = steps; k >= 1; --k)
        {
            model.getUndoStack().undo();
            undoExact = undoExact && (serialize(model.getData()) == checkpoints[k - 1]);
        }
        CHECK(undoExact);
        CHECK(serialize(model.getData()) == checkpoints[0]);
        CHECK(ev.getEventCount() == 0);
        CHECK(tm.getTimerCount() == 0);

        bool redoExact = true;
        for (int k = 1; k <= steps; ++k)
        {
            model.getUndoStack().redo();
            redoExact = redoExact && (serialize(model.getData()) == checkpoints[k]);
        }
        CHECK(redoExact);
        CHECK(serialize(model.getData()) == built);
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario H: SM-13 canvas state lifecycle -- create (state + node as one step),
// rename, and delete removing the transitions that target the deleted state.
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testCanvasStateLifecycle()
    {
        StateMachineData    doc;
        DocModelNotifier    notifier;
        QUndoStack          stack;

        const QString empty = serialize(doc);

        // Create: one undo step producing the state and its Node layout entry.
        SMCreateStateCommand* create = new SMCreateStateCommand(  doc, notifier, doc.getStates()
                                                                , "Idle", SMStateEntry::eStateKind::Start
                                                                , QRectF(32.0, 48.0, 160.0, 96.0), "Add Idle");
        stack.push(create);
        const uint32_t idleId = create->getStateId();
        CHECK(stack.count() == 1);
        CHECK(doc.getStates().findState("Idle") != nullptr);
        const SMLayoutNode* node = doc.getLayout().findNode(idleId);
        CHECK((node != nullptr) && (node->x == 32.0) && (node->height == 96.0));

        stack.undo();
        CHECK(serialize(doc) == empty);
        stack.redo();
        CHECK(doc.getLayout().findNode(idleId) != nullptr);

        // Rename: one undoable step broadcasting nameChanged.
        int renames = 0;
        QObject::connect(&notifier, &DocModelNotifier::nameChanged, [&](uint32_t, const QString&, const QString&){ ++renames; });
        stack.push(new SMRenameStateCommand(doc, notifier, idleId, "Ready", "Rename Idle"));
        CHECK(doc.getStates().findState("Ready") != nullptr);
        CHECK(renames == 1);
        stack.undo();
        CHECK(doc.getStates().findState("Idle") != nullptr);
        stack.redo();

        // Delete: a transition of a surviving state targeting the deleted one goes with it.
        SMStateEntry* run = doc.getStates().createState("Run", SMStateEntry::eStateKind::Normal);
        run->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Event, "evStop", idleId); // targets the (renamed) deleted state
        run->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Event, "evLoop", run->getId());
        const QString beforeDelete = serialize(doc);

        stack.push(new SMRemoveStateCommand(doc, notifier, doc.getStates(), idleId, "Delete Ready"));
        CHECK(doc.getStates().findState("Ready") == nullptr);
        CHECK(run->getTransitions().getElementCount() == 1);        // evStop -> Ready removed

        stack.undo();
        CHECK(serialize(doc) == beforeDelete);                      // transition restored with IDs
        CHECK(run->getTransitions().getElementCount() == 2);

        stack.redo();
        CHECK(run->getTransitions().getElementCount() == 1);
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario I: SM-20 copy/paste/duplicate -- subtree paste with ID re-allocation and
// name-collision suffixes, cross-document registry merge, one-step undo/redo.
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< Builds the SM-20 source document: registries, a composite state with nested
    //!< states, referencing transitions and operations, plus layout and a bound note.
    void buildPasteSource(StateMachineData& doc)
    {
        doc.getEvents().createEvent("evGo");
        SMTimerEntry* timer = doc.getTimers().createTimer("tmPoll");
        timer->setTimeout(500u);
        doc.getMethods().createMethod("doWork", SMMethodEntry::eMethodType::Action);

        SMStateData& root = doc.getStates();
        root.createState("Start", SMStateEntry::eStateKind::Start);
        SMStateEntry* worker = root.createState("Worker", SMStateEntry::eStateKind::Normal);
        SMStateEntry* idle   = root.createState("Idle", SMStateEntry::eStateKind::Normal);

        SMStateData* nested = worker->getOrCreateNestedStates();
        SMStateEntry* wstart = nested->createState("WStart", SMStateEntry::eStateKind::Start);
        SMStateEntry* inner  = nested->createState("Inner", SMStateEntry::eStateKind::Normal);

        worker->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Event, "evGo", idle->getId());
        wstart->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Timer, QString(), inner->getId(), SMTransitionEntry::eTransitionKind::Initial);
        worker->getEntryList().addOperation(new SMActionCall(0u, "doWork"));
        worker->getEntryList().addOperation(new SMEventSend(0u, "evGo"));
        worker->getExitList().addOperation(new SMTimerStart(0u, "tmPoll"));

        SMLayoutData& layout = doc.getLayout();
        SMLayoutNode& node = layout.addNode(worker->getId());
        node.x = 100.0; node.y = 100.0; node.width = 180.0; node.height = 120.0;
        SMLayoutEdge& edge = layout.addEdge(worker->getTransitions().getElements().first()->getId());
        edge.points = { QPointF(10.0, 10.0), QPointF(50.0, 50.0) };
        SMLayoutNote& note = layout.addNote(doc.getOverview().getId(), worker->getId());
        note.x = 40.0; note.y = 40.0; note.width = 80.0; note.height = 40.0;
        note.text = QStringLiteral("worker note");
    }

    //!< The IDs of every state and transition of a subtree, for uniqueness checks.
    QSet<uint32_t> ownedIdSet(const SMStateEntry& state)
    {
        QList<uint32_t> ids;
        SMClipboard::collectOwnedIds(state, ids);
        return QSet<uint32_t>(ids.constBegin(), ids.constEnd());
    }

    void testCopyPasteDuplicate()
    {
        StateMachineData    doc;
        DocModelNotifier    notifier;
        QUndoStack          stack;

        buildPasteSource(doc);
        SMStateEntry* worker = doc.findState("Worker");
        const uint32_t rootLevel = doc.getOverview().getId();

        // --- Same-document paste of a composite subtree (AC1/AC2/AC4). ---
        const QString xml = SMClipboard::serialize(doc, QList<uint32_t>{ worker->getId() });
        CHECK(xml.isEmpty() == false);
        std::unique_ptr<SMClipboardContent> content = SMClipboard::parse(xml);
        CHECK(content != nullptr);

        const QString before = serialize(doc);
        SMPasteCommand* paste = new SMPasteCommand(doc, notifier, std::move(content), rootLevel, QPointF(16.0, 16.0), "Paste");
        CHECK(paste->isEffective());
        stack.push(paste);
        const QString after = serialize(doc);
        CHECK(after != before);

        CHECK(paste->getPastedIds().size() == 1);
        const uint32_t copyId = paste->getPastedIds().first();
        SMStateEntry* copy = doc.findStateById(copyId);
        CHECK(copy != nullptr);
        CHECK(copy->getName() == (QStringLiteral("Worker_") + QString::number(copyId)));
        CHECK(copy->hasNestedStates() && (copy->getNestedStates()->getElementCount() == 2));

        // Fresh, non-overlapping IDs across the whole subtree.
        CHECK(ownedIdSet(*worker).intersects(ownedIdSet(*copy)) == false);

        // A target inside the copied set is remapped to the copy's new ID; a target leaving the
        // copied set (Idle) is dropped, while the stimulus (evGo) and the KIND are kept -- an
        // external transition that lost its target is an edge to reconnect, not an internal one.
        SMStateEntry* wstartCopy = copy->getNestedStates()->getStartState();
        CHECK((wstartCopy != nullptr) && (wstartCopy->getName() != QStringLiteral("WStart")));
        SMStateEntry* innerCopy = copy->getNestedStates()->getElements().last();
        CHECK(innerCopy->getName() != QStringLiteral("Inner"));
        CHECK(wstartCopy->getTransitions().getElements().first()->getToId() == innerCopy->getId());
        SMTransitionEntry* outTx = copy->getTransitions().getElements().first();
        CHECK(outTx->isExternal());                 // still external: only its target was lost
        CHECK(outTx->hasTarget() == false);         // target Idle lay outside the pasted set
        CHECK(outTx->getStimulus() == QStringLiteral("evGo"));

        // Registries merged in place: nothing duplicated within the same document.
        CHECK(doc.getEvents().getElementCount() == 1);
        CHECK(doc.getTimers().getElementCount() == 1);
        CHECK(doc.getMethods().getElementCount() == 1);

        // Layout pasted with the offset; the bound note followed its owner.
        const SMLayoutNode* nodeCopy = doc.getLayout().findNode(copyId);
        CHECK((nodeCopy != nullptr) && (nodeCopy->x == 116.0) && (nodeCopy->y == 116.0));
        CHECK(doc.getLayout().findNoteByOwner(copyId) != nullptr);

        // One undo step restores the exact prior state; redo restores identical IDs.
        stack.undo();
        CHECK(serialize(doc) == before);
        stack.redo();
        CHECK(serialize(doc) == after);

        // --- Cross-document paste: conflicting event copied+renamed, identical timer
        // reused, missing method added under its own name (AC3). ---
        StateMachineData    docB;
        DocModelNotifier    notifierB;
        QUndoStack          stackB;

        SMEventEntry* eventB = docB.getEvents().createEvent("evGo");
        eventB->addParam("payload");                            // different payload: conflict
        SMTimerEntry* timerB = docB.getTimers().createTimer("tmPoll");
        timerB->setTimeout(500u);                               // identical: reused

        std::unique_ptr<SMClipboardContent> contentB = SMClipboard::parse(xml);
        CHECK(contentB != nullptr);
        const QString beforeB = serialize(docB);
        SMPasteCommand* pasteB = new SMPasteCommand(  docB, notifierB, std::move(contentB)
                                                    , docB.getOverview().getId(), QPointF(16.0, 16.0), "Paste");
        CHECK(pasteB->isEffective());
        stackB.push(pasteB);
        const QString afterB = serialize(docB);

        CHECK(docB.getEvents().getElementCount() == 2);
        QString renamedEvent;
        for (const SMEventEntry* entry : docB.getEvents().getElements())
        {
            if (entry->getName() != QStringLiteral("evGo"))
            {
                renamedEvent = entry->getName();
            }
        }
        CHECK(renamedEvent.startsWith(QStringLiteral("evGo_")));

        SMStateEntry* copyB = docB.findState("Worker");         // no collision: name kept
        CHECK(copyB != nullptr);
        CHECK(copyB->getTransitions().getElements().first()->getStimulus() == renamedEvent);

        CHECK(docB.getTimers().getElementCount() == 1);         // reused, not copied
        CHECK(docB.getMethods().getElementCount() == 1);
        CHECK(docB.getMethods().findMethod(QString("doWork")) != nullptr);

        stackB.undo();
        CHECK(serialize(docB) == beforeB);
        stackB.redo();
        CHECK(serialize(docB) == afterB);

        // --- Explicit registry-entry copy: always a new, ID-suffixed entry. ---
        const uint32_t eventId = doc.getEvents().findEvent(QString("evGo"))->getId();
        const QString xmlEvent = SMClipboard::serialize(doc, QList<uint32_t>{ eventId });
        CHECK(xmlEvent.isEmpty() == false);
        std::unique_ptr<SMClipboardContent> contentE = SMClipboard::parse(xmlEvent);
        CHECK(contentE != nullptr);
        SMPasteCommand* pasteE = new SMPasteCommand(doc, notifier, std::move(contentE), rootLevel, QPointF(16.0, 16.0), "Paste");
        CHECK(pasteE->isEffective());
        stack.push(pasteE);
        CHECK(doc.getEvents().getElementCount() == 2);
        CHECK(pasteE->getPastedIds().size() == 1);
        const SMEventEntry* eventCopy = doc.getEvents().findEvent(pasteE->getPastedIds().first());
        CHECK((eventCopy != nullptr)
              && (eventCopy->getName() == (QStringLiteral("evGo_") + QString::number(eventCopy->getId()))));
        stack.undo();
        CHECK(doc.getEvents().getElementCount() == 1);
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario J: SM-28 history mode -- one undo step per change, and the attribute
// only survives a save while the state still has a submachine to remember.
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testHistoryMode()
    {
        std::printf("[SM-28] history mode: undo/redo and persistence\n");

        StateMachineData    doc;
        DocModelNotifier    notifier;
        QUndoStack          stack;

        doc.getStates().createState("Idle", SMStateEntry::eStateKind::Start);
        SMStateEntry* comp = doc.getStates().createState("Running", SMStateEntry::eStateKind::Normal);
        SMStateData* nested = comp->getOrCreateNestedStates();
        nested->createState("SubStart", SMStateEntry::eStateKind::Start);
        nested->createState("Work", SMStateEntry::eStateKind::Normal);
        const uint32_t compId = comp->getId();
        CHECK(comp->isComposite());

        const QString noHistory = serialize(doc);

        stack.push(new SMSetHistoryCommand(doc, notifier, compId, SMStateEntry::eHistory::Shallow, "Set history mode"));
        CHECK(stack.count() == 1);
        CHECK(doc.findStateById(compId)->getHistory() == SMStateEntry::eHistory::Shallow);
        CHECK(serialize(doc).contains("History=\"Shallow\""));

        // Switching modes is a second step, not an edit of the first.
        stack.push(new SMSetHistoryCommand(doc, notifier, compId, SMStateEntry::eHistory::Deep, "Set history mode"));
        CHECK(stack.count() == 2);
        CHECK(serialize(doc).contains("History=\"Deep\""));

        stack.undo();
        CHECK(doc.findStateById(compId)->getHistory() == SMStateEntry::eHistory::Shallow);
        stack.undo();
        CHECK(doc.findStateById(compId)->getHistory() == SMStateEntry::eHistory::None);
        CHECK(serialize(doc) == noHistory);         // clearing leaves no trace in the file

        stack.redo();
        stack.redo();
        CHECK(doc.findStateById(compId)->getHistory() == SMStateEntry::eHistory::Deep);

        // A submachine holding only its Start marker is not persisted, so the host reloads as a
        // plain state. Writing History anyway would hand the user a validation error on open.
        StateMachineData half;
        half.getStates().createState("Idle", SMStateEntry::eStateKind::Start);
        SMStateEntry* halfComp = half.getStates().createState("Running", SMStateEntry::eStateKind::Normal);
        halfComp->getOrCreateNestedStates()->createState("SubStart", SMStateEntry::eStateKind::Start);
        halfComp->setHistory(SMStateEntry::eHistory::Deep);
        halfComp->setOnFinal("evDone");
        const QString halfXml = serialize(half);
        CHECK(halfXml.contains("History=") == false);
        CHECK(halfXml.contains("OnFinal=") == false);
    }
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// SM-29: submachine hosting and the read-only document
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testSubmachineHosting()
    {
        std::printf("[SM-29] submachine link/unlink and read-only refusal\n");

        StateMachineData    doc;
        DocModelNotifier    notifier;
        QUndoStack          stack;

        doc.getStates().createState("Idle", SMStateEntry::eStateKind::Start);
        SMStateEntry* host = doc.getStates().createState("Phase", SMStateEntry::eStateKind::Normal);
        const uint32_t hostId = host->getId();
        if (IncludeEntry* import = doc.getIncludes().createInclude("./TurnCycle.fsml"))
        {
            import->setAlias("TurnCycle");
        }

        SMSetSubmachineCommand* link = new SMSetSubmachineCommand(doc, notifier, hostId, "TurnCycle", "Set submachine");
        CHECK(link->isEffective());
        stack.push(link);
        CHECK(doc.findStateById(hostId)->getSubmachine() == QStringLiteral("TurnCycle"));
        CHECK(doc.findStateById(hostId)->isComposite());

        // OnFinal and History only mean something while the state hosts a machine, so unlinking
        // takes them with it -- in the same undo step, and undo brings all three back.
        stack.push(new SMSetOnFinalCommand(doc, notifier, hostId, "evDone", "Set completion event"));
        stack.push(new SMSetHistoryCommand(doc, notifier, hostId, SMStateEntry::eHistory::Deep, "Set history mode"));
        stack.push(new SMSetSubmachineCommand(doc, notifier, hostId, QString(), "Remove submachine"));
        CHECK(doc.findStateById(hostId)->getSubmachine().isEmpty());
        CHECK(doc.findStateById(hostId)->getOnFinal().isEmpty());
        CHECK(doc.findStateById(hostId)->getHistory() == SMStateEntry::eHistory::None);

        stack.undo();
        CHECK(doc.findStateById(hostId)->getSubmachine() == QStringLiteral("TurnCycle"));
        CHECK(doc.findStateById(hostId)->getOnFinal() == QStringLiteral("evDone"));
        CHECK(doc.findStateById(hostId)->getHistory() == SMStateEntry::eHistory::Deep);

        // A painted composite refuses the swap: the subtree would be gone beyond undo's reach.
        SMStateEntry* painted = doc.getStates().createState("Painted", SMStateEntry::eStateKind::Normal);
        painted->getOrCreateNestedStates()->createState("SubStart", SMStateEntry::eStateKind::Start);
        SMSetSubmachineCommand* refused = new SMSetSubmachineCommand(doc, notifier, painted->getId(), "TurnCycle", "Set submachine");
        CHECK(refused->isEffective() == false);
        delete refused;

        // Renaming the import rewrites the alias on every hosting state as one step.
        StateMachineModel model;
        model.getData().getStates().createState("Idle", SMStateEntry::eStateKind::Start);
        SMStateEntry* a = model.getData().getStates().createState("A", SMStateEntry::eStateKind::Normal);
        SMStateEntry* b = model.getData().getStates().createState("B", SMStateEntry::eStateKind::Normal);
        IncludeEntry* imported = model.getIncludeModel().createInclude("./cycle.fsml");
        CHECK(imported != nullptr);
        CHECK(imported->getAlias() == QStringLiteral("cycle"));
        model.getIncludeModel().setAlias(imported->getId(), "Cycle");
        a->setSubmachine("Cycle");
        b->setSubmachine("Cycle");
        CHECK(model.getIncludeModel().whereUsed(imported->getId()).size() == 2);

        model.getIncludeModel().setAlias(imported->getId(), "Rotation");
        CHECK(a->getSubmachine() == QStringLiteral("Rotation"));
        CHECK(b->getSubmachine() == QStringLiteral("Rotation"));
        model.getUndoStack().undo();
        CHECK(a->getSubmachine() == QStringLiteral("Cycle"));
        CHECK(b->getSubmachine() == QStringLiteral("Cycle"));

        // Read-only: the stack is the chokepoint, so nothing a view builds can reach the model.
        const int before = model.getUndoStack().count();
        model.setReadOnly(true, QStringLiteral("Host : A"));
        CHECK(model.isReadOnly());
        CHECK(model.getUndoStack().count() == 0);           // history is dropped with the write right
        CHECK(model.getUndoStack().push(new SMSetOnFinalCommand(model.getData(), model.getNotifier(), a->getId(), "evX", "Set completion event")) == false);
        CHECK(a->getOnFinal().isEmpty());
        CHECK(model.saveToFile(QStringLiteral("C:/Temp/should-never-exist.fsml")) == false);
        CHECK(before >= 0);

        model.setReadOnly(false);
        CHECK(model.getUndoStack().push(new SMSetOnFinalCommand(model.getData(), model.getNotifier(), a->getId(), "evX", "Set completion event")));
        CHECK(a->getOnFinal() == QStringLiteral("evX"));
    }
}

namespace
{
    //!< Writes a minimal machine that imports \p importPath (empty for none), so the add-time
    //!< checks have real files on disk to walk.
    void writeMachineFile(const QString& path, const QString& name, const QString& importPath)
    {
        std::unique_ptr<StateMachineData> doc = StateMachineData::createNewDocument(name);
        if (importPath.isEmpty() == false)
        {
            IncludeEntry* entry = doc->getIncludes().createInclude(importPath);
            entry->setAlias(QStringLiteral("Nested"));
            entry->setVersion(VersionNumber(QStringLiteral("1.0.0")));
        }

        doc->writeToFile(path);
        SMDocumentCache::getInstance().clear();
    }

    void testImportPreChecks()
    {
        std::printf("[SM-29-EXT] add-time import refusals\n");

        QTemporaryDir dir;
        CHECK(dir.isValid());
        const QString root = dir.path();
        const auto at = [&root](const char* file) -> QString { return QDir(root).absoluteFilePath(QString::fromLatin1(file)); };

        StateMachineModel model;
        SMIncludeModel& includes = model.getIncludeModel();
        QStringList chain;

        // An unsaved host has no folder to resolve against and no identity anything can point
        // back at, so the cycle check would pass on any file. Refused outright rather than
        // silently unchecked.
        CHECK(includes.canImport(at("other.fsml"), chain) == SMIncludeModel::eImportRefusal::HostNotSaved);

        writeMachineFile(at("abc.fsml"), QStringLiteral("ABC"), QString());
        model.getData().setFilePath(at("abc.fsml"));
        CHECK(includes.canImport(at("abc.fsml"), chain) == SMIncludeModel::eImportRefusal::SelfImport);

        // DEF imports ABC; opening ABC and importing DEF would close the cycle.
        writeMachineFile(at("def.fsml"), QStringLiteral("DEF"), QStringLiteral("./abc.fsml"));
        SMDocumentCache::getInstance().clear();
        chain.clear();
        CHECK(includes.canImport(at("def.fsml"), chain) == SMIncludeModel::eImportRefusal::Cycle);
        CHECK(chain.isEmpty() == false);

        // A plain machine that points nowhere is accepted, and registering it a second time is
        // refused by the location guard rather than duplicated under another alias.
        writeMachineFile(at("plain.fsml"), QStringLiteral("Plain"), QString());
        SMDocumentCache::getInstance().clear();
        CHECK(includes.canImport(at("plain.fsml"), chain) == SMIncludeModel::eImportRefusal::None);
        const IncludeEntry* added = includes.createInclude(includes.storableLocation(at("plain.fsml")));
        CHECK(added != nullptr);
        CHECK((added != nullptr) && (added->getAlias() == QStringLiteral("plain")));
        CHECK(includes.createInclude(includes.storableLocation(at("plain.fsml"))) == nullptr);

        // A file that is not a machine at all is a warning, not a refusal: it may be repaired.
        QFile broken(at("broken.fsml"));
        CHECK(broken.open(QIODevice::WriteOnly));
        broken.write("not xml at all");
        broken.close();
        SMDocumentCache::getInstance().clear();
        CHECK(includes.canImport(at("broken.fsml"), chain) == SMIncludeModel::eImportRefusal::Unreadable);
    }

    void testPatchPinAutoFix()
    {
        std::printf("[SM-29-EXT] a patch-only pin drift is corrected silently on open\n");

        QTemporaryDir dir;
        CHECK(dir.isValid());
        const QString root = dir.path();
        const auto at = [&root](const char* file) -> QString { return QDir(root).absoluteFilePath(QString::fromLatin1(file)); };

        // The imported machine is actually at 1.2.7.
        std::unique_ptr<StateMachineData> imported = StateMachineData::createNewDocument(QStringLiteral("Imported"));
        imported->getOverview().setVersion(VersionNumber(QStringLiteral("1.2.7")));
        imported->writeToFile(at("imported.fsml"));

        // The host pins it at 1.2.1 -- same MAJOR.MINOR, drifted PATCH only.
        std::unique_ptr<StateMachineData> host = StateMachineData::createNewDocument(QStringLiteral("Host"));
        IncludeEntry* pin = host->getIncludes().createInclude(QStringLiteral("./imported.fsml"));
        pin->setAlias(QStringLiteral("Nested"));
        pin->setVersion(VersionNumber(QStringLiteral("1.2.1")));
        host->writeToFile(at("host.fsml"));
        SMDocumentCache::getInstance().clear();

        StateMachineModel model;
        CHECK(model.loadFromFile(at("host.fsml")));

        // The pin is corrected in memory immediately, no dialog.
        const IncludeEntry* fixed = model.getData().findImportByAlias(QStringLiteral("Nested"));
        CHECK(fixed != nullptr);
        CHECK((fixed != nullptr) && (fixed->getVersion().toString() == QStringLiteral("1.2.7")));

        // The document is marked modified, so the normal save path picks it up.
        CHECK(model.isDirty());

        // The very first validation pass says so, once, as Info.
        auto hasAutoFixNotice = [](const QList<SMIssue>& issues) -> bool
        {
            for (const SMIssue& issue : issues)
            {
                if ((issue.rule == (SMValidator::WARNING_RULE_BASE + 12)) && issue.message.contains(QStringLiteral("updated")))
                    return true;
            }
            return false;
        };
        CHECK(hasAutoFixNotice(model.getValidationController().issues()));

        // Re-running validation on the same, already-fixed document does not repeat the notice.
        model.getValidationController().validateNow();
        CHECK(hasAutoFixNotice(model.getValidationController().issues()) == false);

        // Saving persists the corrected pin, so a fresh open finds no drift at all.
        CHECK(model.saveToFile());
        SMDocumentCache::getInstance().clear();
        StateMachineModel reopened;
        CHECK(reopened.loadFromFile(at("host.fsml")));
        CHECK(reopened.isDirty() == false);
        bool anyPinIssue = false;
        for (const SMIssue& issue : reopened.getValidationController().issues())
            anyPinIssue = anyPinIssue || (issue.rule == 22) || (issue.rule == (SMValidator::WARNING_RULE_BASE + 12));
        CHECK(anyPinIssue == false);
    }

    void testRemoveComposite()
    {
        std::printf("[SM-29-EXT] remove a painted submachine, three levels deep\n");

        StateMachineData doc;
        DocModelNotifier notifier;
        QUndoStack       stack;

        doc.getStates().createState("Idle", SMStateEntry::eStateKind::Start);
        SMStateEntry* host = doc.getStates().createState("Host", SMStateEntry::eStateKind::Normal);
        SMStateEntry* peer = doc.getStates().createState("Peer", SMStateEntry::eStateKind::Normal);
        const uint32_t hostId = host->getId();

        // Level 1: Start + Work; level 2 under Work: Start + Deep; level 3 under Deep: Start.
        SMStateData* l1 = host->getOrCreateNestedStates();
        l1->createState("L1Start", SMStateEntry::eStateKind::Start);
        SMStateEntry* work = l1->createState("Work", SMStateEntry::eStateKind::Normal);
        SMStateData* l2 = work->getOrCreateNestedStates();
        l2->createState("L2Start", SMStateEntry::eStateKind::Start);
        SMStateEntry* deep = l2->createState("Deep", SMStateEntry::eStateKind::Normal);
        SMStateData* l3 = deep->getOrCreateNestedStates();
        SMStateEntry* leaf = l3->createState("L3Start", SMStateEntry::eStateKind::Start);
        const uint32_t leafId = leaf->getId();

        host->setHistory(SMStateEntry::eHistory::Deep);
        host->setOnFinal("evDone");

        // A transition of a surviving state that reaches into the subtree: exactly the reference
        // that dangles when the subtree goes and nobody sweeps for it.
        SMTransitionEntry* intruder = peer->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, "poke");
        intruder->setToId(leafId);

        doc.getLayout().addNode(hostId);
        doc.getLayout().addNode(leafId);
        doc.getLayout().addEdge(intruder->getId());

        SMRemoveCompositeCommand* command = new SMRemoveCompositeCommand(doc, notifier, hostId, "Remove submachine");
        CHECK(command->isEffective());
        CHECK(command->removedStateCount() == 5);           // L1Start, Work, L2Start, Deep, L3Start
        CHECK(command->removedTransitionCount() == 1);
        stack.push(command);

        CHECK(doc.findStateById(hostId)->hasNestedStates() == false);
        CHECK(doc.findStateById(hostId)->getHistory() == SMStateEntry::eHistory::None);
        CHECK(doc.findStateById(hostId)->getOnFinal().isEmpty());
        CHECK(doc.findStateById(leafId) == nullptr);
        CHECK(peer->getTransitions().getElementCount() == 0);
        CHECK(doc.getLayout().findNode(leafId) == nullptr);
        CHECK(doc.getLayout().findNode(hostId) != nullptr);  // the host survives, and so does its box

        stack.undo();
        CHECK(doc.findStateById(hostId)->hasNestedStates());
        CHECK(doc.findStateById(hostId)->getHistory() == SMStateEntry::eHistory::Deep);
        CHECK(doc.findStateById(hostId)->getOnFinal() == QStringLiteral("evDone"));
        CHECK(doc.findStateById(leafId) != nullptr);          // the same ID, not a fresh one
        CHECK(peer->getTransitions().getElementCount() == 1);
        CHECK(doc.getLayout().findNode(leafId) != nullptr);

        // A plain state has nothing to remove.
        SMRemoveCompositeCommand* nothing = new SMRemoveCompositeCommand(doc, notifier, peer->getId(), "Remove submachine");
        CHECK(nothing->isEffective() == false);
        delete nothing;
    }
}

//////////////////////////////////////////////////////////////////////////
// L3: loading a pre-L3 document resolves its free-text `Until` into a tree
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Scenario: re-pointing a transition re-binds a matching guard parameter silently
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testStimulusSilentRebind()
    {
        StateMachineData    doc;
        DocModelNotifier    notifier;
        QUndoStack          stack;

        // Two triggers, each declaring a 'count' : uint16 parameter.
        SMMethodEntry* walk = doc.getMethods().createMethod("RequestWalk", SMMethodEntry::eMethodType::Trigger);
        walk->addParam("count")->setType("uint16");
        SMMethodEntry* run = doc.getMethods().createMethod("RequestRun", SMMethodEntry::eMethodType::Trigger);
        run->addParam("count")->setType("uint16");

        stack.push(new SMAddStateCommand(notifier, doc.getStates(), "S", SMStateEntry::eStateKind::Start, "Add S"));
        SMStateEntry* s = doc.getStates().findState("S");
        SMTransitionEntry* tr = s->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger
                                                                    , "RequestWalk", 0u, SMTransitionEntry::eTransitionKind::Internal);
        const uint32_t tid = tr->getId();

        SMGuardParser::Result g = SMGuardParser::parse(doc, tid, "count > 3");
        CHECK(g.resolved());
        tr->getGuard().setTree(g.tree);

        const uint32_t walkCount = SMGuardSymbols::paramId(doc, tid, "count");
        const SMGuardNode* before = tr->getGuard().getTree()->childAt(0);
        CHECK((before != nullptr) && (before->getKind() == SMGuardNode::eKind::Param) && (before->getSymbolId() == walkCount));

        // Re-point the transition at the other stimulus, whose 'count' matches by name and type.
        stack.push(new SMSetStimulusCommand(doc, notifier, tid, SMTransitionEntry::eStimulusKind::Trigger, "RequestRun", "Set stimulus"));

        const uint32_t runCount = SMGuardSymbols::paramId(doc, tid, "count");
        CHECK(walkCount != runCount);   // two different declarations
        const SMGuardNode* after = tr->getGuard().getTree()->childAt(0);
        CHECK((after != nullptr) && (after->getSymbolId() == runCount));   // silently re-bound

        // Undo restores the original binding exactly.
        stack.undo();
        const SMGuardNode* undone = tr->getGuard().getTree()->childAt(0);
        CHECK((undone != nullptr) && (undone->getSymbolId() == walkCount));
    }
}

//////////////////////////////////////////////////////////////////////////
// The Constants page keeps working after the document object is replaced
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< Creating a new document, and opening a file, both swap the data object the model holds.
    //!< The Constants page model has to reach the section that is current then, not the one that
    //!< existed when the model was built.
    void testConstantsSurviveDocumentSwap()
    {
        StateMachineModel model;
        ConstantModel& constants = model.getConstantModel();

        CHECK(constants.createConstant(QStringLiteral("Before")) != nullptr);
        CHECK(constants.getConstantCount() == 1);

        // What "File / New" does.
        CHECK(model.createNewDocument(QStringLiteral("Swapped")));
        CHECK(constants.getConstantCount() == 0);
        CHECK(&model.getData().getConstants().getElements() == &constants.getConstants());

        ConstantEntry* added = constants.createConstant(QStringLiteral("MaxRetries"));
        CHECK(added != nullptr);
        CHECK(constants.getConstantCount() == 1);
        CHECK(model.getData().getConstants().getElementCount() == 1);

        // The edit landed in the new document, and undo takes it back out of that same one.
        constants.setValue(added->getId(), QStringLiteral("5"));
        CHECK(model.getData().getConstants().findElement(added->getId())->getValue() == QStringLiteral("5"));
        model.getUndoStack().undo();
        CHECK(model.getData().getConstants().findElement(added->getId())->getValue().isEmpty());

        // And again after a reload, which swaps the data object a second time.
        QTemporaryDir dir;
        CHECK(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("swap.fsml"));
        CHECK(model.saveToFile(path));
        CHECK(model.loadFromFile(path));
        CHECK(constants.getConstantCount() == 1);
        CHECK(constants.findConstant(QStringLiteral("MaxRetries")) != nullptr);
        CHECK(constants.createConstant(QStringLiteral("AfterReload")) != nullptr);
        CHECK(model.getData().getConstants().getElementCount() == 2);
    }
}

int main(int /*argc*/, char* /*argv*/[])
{
    std::printf("SM-04 command framework tests\n");
    testScriptedSequence();
    testCoalescedLayoutDrag();
    testCompositeDelete();
    testRenameKeepsTransitions();
    testDeepHistory();
    testDataTypeLifecycle();
    testContainerLifecycle();
    testEventTimerLifecycle();
    testCanvasStateLifecycle();
    testCopyPasteDuplicate();
    testHistoryMode();
    testSubmachineHosting();
    testImportPreChecks();
    testPatchPinAutoFix();
    testRemoveComposite();
    testStimulusSilentRebind();
    testConstantsSurviveDocumentSwap();

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
