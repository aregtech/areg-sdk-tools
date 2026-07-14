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
 *  \file        tests/si/SICommandTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Undo/redo command framework tests for the Service Interface (.siml) editor.
 *               The Service Interface has no undo/redo of its own; it drives the exact same
 *               shared command framework (model/common) as the FSM editor. This proves the
 *               framework is document-agnostic: identical invariants over ServiceInterfaceData.
 *
 *  Self-contained (no external test framework), matching SMModelTests.cpp.
 *
 ************************************************************************/

#include "lusan/data/si/ServiceInterfaceData.hpp"
#include "lusan/data/common/AttributeEntry.hpp"
#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/si/SICommand.hpp"

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

    QString serialize(const ServiceInterfaceData& doc)
    {
        QString out;
        QXmlStreamWriter writer(&out);
        writer.setAutoFormatting(true);
        doc.writeToXml(writer);
        return out;
    }

    AttributeEntry makeAttribute(const QString& name, ElementBase* parent)
    {
        return AttributeEntry(0u, name, AttributeEntry::eNotification::NotifyOnChange, parent);
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
        ServiceInterfaceData    doc;
        DocModelNotifier        notifier;
        QUndoStack              stack;

        int added = 0, removed = 0, changed = 0, reordered = 0;
        QObject::connect(&notifier, &DocModelNotifier::elementAdded,   [&](uint32_t, eDocElementKind){ ++added; });
        QObject::connect(&notifier, &DocModelNotifier::elementRemoved, [&](uint32_t, eDocElementKind){ ++removed; });
        QObject::connect(&notifier, &DocModelNotifier::elementChanged, [&](uint32_t, eDocElementKind){ ++changed; });
        QObject::connect(&notifier, &DocModelNotifier::listReordered,  [&](uint32_t, eDocElementKind){ ++reordered; });

        doc.getOverviewData().getId();
        doc.getAttributeData().getId();
        doc.getConstantData().getId();

        QStringList checkpoints;
        checkpoints << serialize(doc);
        auto pushAndSnap = [&](QUndoCommand* command)
        {
            stack.push(command);
            checkpoints << serialize(doc);
        };

        SIAttributeData& attrs = doc.getAttributeData();
        pushAndSnap(new TDocAddCommand<AttributeEntry, DocumentElem>(notifier, attrs, makeAttribute("speed", &attrs), eDocElementKind::Attribute, "Add speed"));
        pushAndSnap(new TDocAddCommand<AttributeEntry, DocumentElem>(notifier, attrs, makeAttribute("gear",  &attrs), eDocElementKind::Attribute, "Add gear"));

        SIConstantData& consts = doc.getConstantData();
        pushAndSnap(new TDocAddCommand<ConstantEntry, DocumentElem>(notifier, consts, ConstantEntry(0u, "MaxSpeed", &consts), eDocElementKind::Constant, "Add MaxSpeed"));

        {
            const uint32_t overviewId = doc.getOverviewData().getId();
            auto getter = [&doc]() -> QString { return doc.getOverviewData().getName(); };
            auto setter = [&doc](const QString& value) { doc.getOverviewData().setName(value); };
            pushAndSnap(new TDocSetPropertyCommand<QString>(notifier, overviewId, eDocElementKind::Overview, getter, setter, QString("Renamed"), "Rename service"));
        }

        pushAndSnap(new TDocReorderCommand<AttributeEntry, DocumentElem>(notifier, attrs, 0, 1, attrs.getId(), eDocElementKind::Attribute, "Reorder attributes"));

        const int steps = static_cast<int>(checkpoints.size()) - 1;
        const QString built = serialize(doc);

        bool undoExact = true;
        for (int k = steps; k >= 1; --k)
        {
            stack.undo();
            undoExact = undoExact && (serialize(doc) == checkpoints[k - 1]);
        }
        CHECK(undoExact);
        CHECK(serialize(doc) == checkpoints[0]);

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
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario B: composite (two removes as one undo step)
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testComposite()
    {
        ServiceInterfaceData    doc;
        DocModelNotifier        notifier;
        QUndoStack              stack;

        SIAttributeData& attrs = doc.getAttributeData();
        attrs.createAttribute("alpha");
        attrs.createAttribute("beta");
        // Read IDs from the container: createAttribute returns a pointer into the value
        // list, which the second insertion may reallocate.
        const uint32_t idA = attrs.getElements().at(0).getId();
        const uint32_t idB = attrs.getElements().at(1).getId();

        const QString before = serialize(doc);
        const int indexBefore = stack.index();

        SICompositeCommand* composite = new SICompositeCommand(doc, notifier, "Delete both");
        new TDocRemoveCommand<AttributeEntry, DocumentElem>(notifier, attrs, idA, eDocElementKind::Attribute, "Delete alpha", composite);
        new TDocRemoveCommand<AttributeEntry, DocumentElem>(notifier, attrs, idB, eDocElementKind::Attribute, "Delete beta", composite);
        stack.push(composite);

        CHECK(stack.index() == indexBefore + 1);            // both removals are one step
        CHECK(attrs.getElementCount() == 0);

        stack.undo();
        CHECK(serialize(doc) == before);                    // both restored with original IDs
        CHECK(attrs.getElementCount() == 2);

        stack.redo();
        CHECK(attrs.getElementCount() == 0);
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario C: deep history (100+ steps) and ID uniqueness
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testDeepHistory()
    {
        ServiceInterfaceData    doc;
        DocModelNotifier        notifier;
        QUndoStack              stack;

        SIAttributeData& attrs = doc.getAttributeData();
        const int count = 120;
        for (int i = 0; i < count; ++i)
        {
            stack.push(new TDocAddCommand<AttributeEntry, DocumentElem>(notifier, attrs, makeAttribute(QString("attr%1").arg(i), &attrs), eDocElementKind::Attribute, "Add"));
        }

        CHECK(stack.count() == count);
        const QString full = serialize(doc);

        for (int i = 0; i < count; ++i)
        {
            stack.undo();
        }
        CHECK(attrs.getElementCount() == 0);

        for (int i = 0; i < count; ++i)
        {
            stack.redo();
        }
        CHECK(attrs.getElementCount() == count);
        CHECK(serialize(doc) == full);

        QList<uint32_t> ids;
        for (const AttributeEntry& attr : attrs.getElements())
        {
            ids.append(attr.getId());
        }
        const QSet<uint32_t> unique(ids.begin(), ids.end());
        CHECK(unique.size() == ids.size());
    }
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char* /*argv*/[])
{
    std::printf("Service Interface command framework tests\n");
    testScriptedSequence();
    testComposite();
    testDeepHistory();

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
