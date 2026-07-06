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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
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

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/model/sm/SMDataTypeModel.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/data/common/EnumEntry.hpp"

#include <QUndoStack>
#include <QXmlStreamWriter>
#include <QString>
#include <QStringList>
#include <QList>
#include <QSet>
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
        parent->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Timer, "T", QString());
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
// Scenario E: SM-07 Data Types page model — enumeration/structure/imported
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
// Scenario F (SM-07-02): container data types — direct creation and conversion-from-
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
// main
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char* /*argv*/[])
{
    std::printf("SM-04 command framework tests\n");
    testScriptedSequence();
    testCoalescedLayoutDrag();
    testCompositeDelete();
    testDeepHistory();
    testDataTypeLifecycle();
    testContainerLifecycle();

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
